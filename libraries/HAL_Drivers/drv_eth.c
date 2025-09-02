/*
* Copyright (c) 2006-2023, RT-Thread Development Team
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date           Author       Notes
* 2018-11-19     SummerGift   first version
* 2018-12-25     zylx         fix some bugs
* 2019-06-10     SummerGift   optimize PHY state detection process
* 2019-09-03     xiaofan      optimize link change detection process
* 2025-02-11     kurisaW      adaptation for RZ Ethernet driver
*/

#include "drv_config.h"
#include "drv_eth.h"
#include <hal_data.h>
#include <netif/ethernetif.h>
#include <lwipopts.h>

/* debug option */
// #define ETH_RX_DUMP
// #define ETH_TX_DUMP
#define MINIMUM_ETHERNET_FRAME_SIZE (60U)
#define ETH_MAX_PACKET_SIZE (1514U)
#define ETHER_GMAC_INTERRUPT_FACTOR_RECEPTION (0x000000C0)
#define ETH_RX_BUF_SIZE ETH_MAX_PACKET_SIZE
#define ETH_TX_BUF_SIZE ETH_MAX_PACKET_SIZE
// #define DRV_DEBUG
#define LOG_TAG "drv.eth"
#ifdef DRV_DEBUG
#define DBG_LVL DBG_LOG
#else
#define DBG_LVL DBG_INFO
#endif /* DRV_DEBUG */
#include <rtdbg.h>

struct rt_eth_dev
{
    /* inherit from ethernet device */
    struct eth_device parent;
#ifndef PHY_USING_INTERRUPT_MODE
    rt_timer_t poll_link_timer;
#endif
};

static rt_uint8_t *Rx_Buff, *Tx_Buff;
// static  ETH_HandleTypeDef EthHandle;
static struct rt_eth_dev ra_eth_device;

static uint8_t g_link_change = 0; ///< Link change (bit0:port0, bit1:port1, bit2:port2)
static uint8_t g_link_status = 0; ///< Link status (bit0:port0, bit1:port1, bit2:port2)
static uint8_t previous_link_status = 0;

/* Multi-PHY support structures */
typedef struct {
    rmac_phy_instance_ctrl_t *p_ctrl;
    uint8_t port_bit;
    const char *name;
} phy_port_info_t;

static const phy_port_info_t phy_ports[] = {
    { &g_rmac_phy0_ctrl, 0x01, "PHY0" },
    { &g_rmac_phy1_ctrl, 0x02, "PHY1" }
};

#define PHY_PORTS_COUNT (sizeof(phy_ports) / sizeof(phy_ports[0]))

#if defined(SOC_SERIES_R9A07G0)

#define status_ecsr             status_link
#define ETHER_EVENT_INTERRUPT   ETHER_EVENT_SBD_INTERRUPT

#define R_ETHER_Open        R_GMAC_Open
#define R_ETHER_Write       R_GMAC_Write
#define R_ETHER_Read        R_GMAC_Read
#define R_ETHER_LinkProcess R_GMAC_LinkProcess

#elif defined(SOC_SERIES_R7KA8P1)

#define R_ETHER_Open        R_RMAC_Open
#define R_ETHER_Write       R_RMAC_Write
#define R_ETHER_Read        R_RMAC_Read
#define R_ETHER_LinkProcess R_RMAC_LinkProcess

#endif

#if defined(ETH_RX_DUMP) || defined(ETH_TX_DUMP)
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const rt_uint8_t *ptr, rt_size_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    for (i = 0; i < buflen; i += 16)
    {
        rt_kprintf("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%02X ", buf[i + j]);
            else
                rt_kprintf("   ");
        rt_kprintf(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                rt_kprintf("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        rt_kprintf("\n");
    }
}
#endif

/* Multi-PHY utility functions */
static rt_err_t phy_read_status_all(uint8_t *link_status)
{
    fsp_err_t res;
    uint32_t phy_data;
    uint8_t i;
    uint8_t status = 0;

    for (i = 0; i < PHY_PORTS_COUNT; i++)
    {
        res = R_RMAC_PHY_Read(phy_ports[i].p_ctrl, 0x1, &phy_data);
        if (res == FSP_SUCCESS)
        {
            if (phy_data & 0x04) /* PHY Basic Status Register Link Status bit */
            {
                status |= phy_ports[i].port_bit;
            }
        }
        else
        {
            LOG_W("%s PHY read failed, res = %d", phy_ports[i].name, res);
            return RT_ERROR;
        }
    }

    *link_status = status;
    return RT_EOK;
}

static void phy_print_status(uint8_t status)
{
    uint8_t i;

    LOG_I("PHY Link Status Summary:");
    for (i = 0; i < PHY_PORTS_COUNT; i++)
    {
        LOG_I("  %s: %s", phy_ports[i].name,
              (status & phy_ports[i].port_bit) ? "UP" : "DOWN");
    }
}

static rt_err_t phy_get_detailed_status(uint8_t phy_index, uint32_t *basic_status, uint32_t *control_reg)
{
    fsp_err_t res;

    if (phy_index >= PHY_PORTS_COUNT)
        return RT_EINVAL;

    /* Read Basic Status Register (0x01) */
    res = R_RMAC_PHY_Read(phy_ports[phy_index].p_ctrl, 0x1, basic_status);
    if (res != FSP_SUCCESS)
    {
        LOG_E("%s failed to read basic status register, res = %d", phy_ports[phy_index].name, res);
        return RT_ERROR;
    }

    /* Read Basic Control Register (0x00) */
    res = R_RMAC_PHY_Read(phy_ports[phy_index].p_ctrl, 0x0, control_reg);
    if (res != FSP_SUCCESS)
    {
        LOG_E("%s failed to read control register, res = %d", phy_ports[phy_index].name, res);
        return RT_ERROR;
    }

    return RT_EOK;
}

/* EMAC initialization function */
static rt_err_t rt_ra_eth_init(void)
{
    fsp_err_t res;
    uint8_t i;
    uint32_t basic_status, control_reg;

    res = R_ETHER_Open(&g_ether0_ctrl, &g_ether0_cfg);
    if (res != FSP_SUCCESS)
        LOG_W("R_ETHER_Open failed!, res = %d", res);

    /* Initialize and check all PHY ports */
    LOG_I("Initializing %d PHY ports...", PHY_PORTS_COUNT);
    for (i = 0; i < PHY_PORTS_COUNT; i++)
    {
        if (phy_get_detailed_status(i, &basic_status, &control_reg) == RT_EOK)
        {
            LOG_I("%s initialization:", phy_ports[i].name);
            LOG_I("  Basic Status: 0x%04X", basic_status);
            LOG_I("  Control Reg:  0x%04X", control_reg);
            LOG_I("  Link Status:  %s", (basic_status & 0x04) ? "UP" : "DOWN");
            LOG_I("  Auto-Neg:     %s", (basic_status & 0x20) ? "Complete" : "In Progress");
        }
        else
        {
            LOG_E("%s initialization failed!", phy_ports[i].name);
        }
    }

    /* Initialize link status tracking */
    phy_read_status_all(&g_link_status);
    previous_link_status = g_link_status;
    phy_print_status(g_link_status);

    return RT_EOK;
}

static rt_err_t rt_ra_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    LOG_D("emac open");
    return RT_EOK;
}

static rt_err_t rt_ra_eth_close(rt_device_t dev)
{
    LOG_D("emac close");
    return RT_EOK;
}

static rt_ssize_t rt_ra_eth_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    LOG_D("emac read");
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_ssize_t rt_ra_eth_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    LOG_D("emac write");
    rt_set_errno(-RT_ENOSYS);
    return 0;
}

static rt_err_t rt_ra_eth_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if (args)
        {
#if defined(SOC_SERIES_R9A07G0)
            SMEMCPY(args, g_ether0_ctrl.p_gmac_cfg->p_mac_address, 6);
#elif defined(SOC_SERIES_R7KA8P1)
            SMEMCPY(args, g_ether0_ctrl.p_cfg->p_mac_address, 6);
#else
            SMEMCPY(args, g_ether0_ctrl.p_ether_cfg->p_mac_address, 6);
#endif
        }
        else
        {
            return -RT_ERROR;
        }
        break;

    default:
        break;
    }

    return RT_EOK;
}

/* ethernet device interface */
/* transmit data*/
rt_err_t rt_ra_eth_tx(rt_device_t dev, struct pbuf *p)
{
    fsp_err_t res;
    struct pbuf *q;
    uint8_t *buffer = Tx_Buff;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;
    bufferoffset = 0;

    LOG_D("send frame len : %d", p->tot_len);

    /* copy frame from pbufs to driver buffers */
    for (q = p; q != NULL; q = q->next)
    {
        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE)
        {
            /* Copy data to Tx buffer*/
            SMEMCPY((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset), (ETH_TX_BUF_SIZE - bufferoffset));

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        SMEMCPY((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset), byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

#ifdef ETH_TX_DUMP
    dump_hex(buffer, p->tot_len);
#endif
#ifdef ETH_RX_DUMP
    if (p)
    {
        LOG_E("******p buf frame *********");
        for (q = p; q != NULL; q->next)
        {
            dump_hex(q->payload, q->len);
        }
    }
#endif
#if defined(__DCACHE_PRESENT)
    SCB_CleanInvalidateDCache();
#endif
    res = R_ETHER_Write(&g_ether0_ctrl, buffer, p->tot_len < MINIMUM_ETHERNET_FRAME_SIZE ?  MINIMUM_ETHERNET_FRAME_SIZE : p->tot_len);
    if (res != FSP_SUCCESS)
    {
        LOG_W("R_ETHER_Write failed!, res = %d", res);
        return (err_t)ERR_USE;
    }
    return RT_EOK;
}

/* receive data*/
struct pbuf *rt_ra_eth_rx(rt_device_t dev)
{
    struct pbuf *p = NULL;
    struct pbuf *q = NULL;
    uint32_t len = 0;
    uint8_t *buffer = Rx_Buff;
    fsp_err_t res;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t remaining_data;

    res = R_ETHER_Read(&g_ether0_ctrl, buffer, &len);
    if (res != FSP_SUCCESS)
    {
        LOG_D("R_ETHER_Read failed!, res = %d", res);
        return NULL;
    }

    LOG_D("receive frame len : %d", len);
    
#if defined(__DCACHE_PRESENT)
    SCB_CleanInvalidateDCache();
#endif

    if (len > 0)
    {
        /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
        if (p == NULL)
        {
            LOG_E("Failed to allocate pbuf for %d bytes", len);
            return NULL;
        }
    }

#ifdef ETH_RX_DUMP
    if (p)
    {
        dump_hex(buffer, p->tot_len);
    }
#endif

    if (p != NULL)
    {
        bufferoffset = 0;
        for (q = p; q != NULL; q = q->next)
        {
            byteslefttocopy = q->len;
            payloadoffset = 0;

            if (bufferoffset >= len)
            {
                LOG_W("Buffer offset exceeds received length");
                break;
            }

            remaining_data = len - bufferoffset;
            if (byteslefttocopy > remaining_data)
            {
                byteslefttocopy = remaining_data;
            }

            /* Copy remaining data in pbuf */
            SMEMCPY((uint8_t *)((uint8_t *)q->payload + payloadoffset), 
                   (uint8_t *)((uint8_t *)buffer + bufferoffset), 
                   byteslefttocopy);
            bufferoffset += byteslefttocopy;
        }
    }

#ifdef ETH_RX_DUMP
    if (p)
    {
        LOG_E("******p buf frame *********");
        for (q = p; q != NULL; q = q->next)
        {
            dump_hex(q->payload, q->len);
        }
    }
#endif

    return p;
}

static void phy_linkchange()
{
    fsp_err_t res;
    uint8_t port;
    uint8_t port_bit;
    uint8_t status_change;
    uint32_t phy_data;
    uint8_t current_link_status = 0;
    uint8_t i;

    LOG_D("phy_linkchange called, g_link_status: 0x%02x, g_link_change: 0x%02x", g_link_status, g_link_change);

    res = R_ETHER_LinkProcess(&g_ether0_ctrl);
    if (res != FSP_SUCCESS)
        LOG_D("R_ETHER_LinkProcess failed!, res = %d", res);
    
    /* Check link status for all PHY ports */
    for (i = 0; i < PHY_PORTS_COUNT; i++)
    {
        res = R_RMAC_PHY_Read(phy_ports[i].p_ctrl, 0x1, &phy_data);
        if (res == FSP_SUCCESS)
        {
            if (phy_data & 0x04) /* PHY Basic Status Register Link Status bit */
            {
                current_link_status |= phy_ports[i].port_bit; /* Port link up */
            }

            LOG_D("%s Status Register: 0x%08x, current_link: %d, previous_link: %d",
                  phy_ports[i].name, phy_data, 
                  (current_link_status & phy_ports[i].port_bit) ? 1 : 0,
                  (previous_link_status & phy_ports[i].port_bit) ? 1 : 0);

            status_change = previous_link_status ^ current_link_status;
            if (status_change & phy_ports[i].port_bit)
            {
                g_link_change |= phy_ports[i].port_bit;
                LOG_I("%s Manual Link status changed: %s", phy_ports[i].name,
                      (current_link_status & phy_ports[i].port_bit) ? "UP" : "DOWN");
            }
        }
        else
        {
            LOG_E("%s PHY_Read failed!, res = %d", phy_ports[i].name, res);
        }
    }

    /* Update global link status */
    g_link_status = current_link_status;

    /* Process link changes for all ports */
    for (port = 0; port < PHY_PORTS_COUNT; port++)
    {
        port_bit = phy_ports[port].port_bit;

        if (g_link_change & port_bit)
        {
            /* Link status changed */
            g_link_change &= (uint8_t)(~port_bit); /* change bit clear */

            if (g_link_status & port_bit)
            {
                /* Changed to Link-up */
                eth_device_linkchange(&ra_eth_device.parent, RT_TRUE);
                LOG_I("%s link up", phy_ports[port].name);
            }
            else
            {
                /* Changed to Link-down */
                eth_device_linkchange(&ra_eth_device.parent, RT_FALSE);
                LOG_I("%s link down", phy_ports[port].name);
            }
        }
    }

    previous_link_status = g_link_status;
}

void user_ether0_callback(ether_callback_args_t *p_args)
{
    rt_interrupt_enter();

    LOG_D("user_ether0_callback called, event: %d, status_ecsr: 0x%02x", p_args->event, p_args->status_ecsr);

    switch (p_args->event)
    {
    case ETHER_EVENT_LINK_ON:                          ///< Link up detection event/
        g_link_status |= (uint8_t)p_args->status_ecsr; ///< status up
        g_link_change |= (uint8_t)p_args->status_ecsr; ///< change bit set
        LOG_I("Interrupt: Link ON detected, status: 0x%02x", g_link_status);
        break;

    case ETHER_EVENT_LINK_OFF:                            ///< Link down detection event
        g_link_status &= (uint8_t)(~p_args->status_ecsr); ///< status down
        g_link_change |= (uint8_t)p_args->status_ecsr;    ///< change bit set
        LOG_I("Interrupt: Link OFF detected, status: 0x%02x", g_link_status);
        break;

    case ETHER_EVENT_WAKEON_LAN:    ///< Magic packet detection event
    /* If EDMAC FR (Frame Receive Event) or FDE (Receive Descriptor Empty Event)
        * interrupt occurs, send rx mailbox. */
    case ETHER_EVENT_RX_COMPLETE: ///< BSD Interrupt event
    {
        rt_err_t result;
        result = eth_device_ready(&(ra_eth_device.parent));
        if (result != RT_EOK)
            rt_kprintf("RX err =%d\n", result);
        break;
    }

    default:
        LOG_D("Unknown ethernet event: %d", p_args->event);
        break;
    }

    rt_interrupt_leave();
}

/* Register the EMAC device */
static int rt_hw_ra_eth_init(void)
{
    rt_err_t state = RT_EOK;

    /* Prepare receive and send buffers */
    Rx_Buff = (rt_uint8_t *)rt_calloc(1, ETH_MAX_PACKET_SIZE);
    if (Rx_Buff == RT_NULL)
    {
        LOG_E("No memory");
        state = -RT_ENOMEM;
        goto __exit;
    }

    Tx_Buff = (rt_uint8_t *)rt_calloc(1, ETH_MAX_PACKET_SIZE);
    if (Tx_Buff == RT_NULL)
    {
        LOG_E("No memory");
        state = -RT_ENOMEM;
        goto __exit;
    }

    ra_eth_device.parent.parent.init = NULL;
    ra_eth_device.parent.parent.open = rt_ra_eth_open;
    ra_eth_device.parent.parent.close = rt_ra_eth_close;
    ra_eth_device.parent.parent.read = rt_ra_eth_read;
    ra_eth_device.parent.parent.write = rt_ra_eth_write;
    ra_eth_device.parent.parent.control = rt_ra_eth_control;
    ra_eth_device.parent.parent.user_data = RT_NULL;

    ra_eth_device.parent.eth_rx = rt_ra_eth_rx;
    ra_eth_device.parent.eth_tx = rt_ra_eth_tx;

    rt_ra_eth_init();

    /* register eth device */
    state = eth_device_init(&(ra_eth_device.parent), "e0");
    if (RT_EOK == state)
    {
        LOG_D("emac device init success");
    }
    else
    {
        LOG_E("emac device init faild: %d", state);
        state = -RT_ERROR;
        goto __exit;
    }

    ra_eth_device.poll_link_timer = rt_timer_create("phylnk", (void (*)(void *))phy_linkchange,
                                                    NULL, RT_TICK_PER_SECOND, RT_TIMER_FLAG_PERIODIC);
    if (!ra_eth_device.poll_link_timer || rt_timer_start(ra_eth_device.poll_link_timer) != RT_EOK)
    {
        LOG_E("Start link change detection timer failed");
    }
__exit:
    if (state != RT_EOK)
    {
        if (Rx_Buff)
        {
            rt_free(Rx_Buff);
        }

        if (Tx_Buff)
        {
            rt_free(Tx_Buff);
        }
    }

    return state;
}
INIT_DEVICE_EXPORT(rt_hw_ra_eth_init);
