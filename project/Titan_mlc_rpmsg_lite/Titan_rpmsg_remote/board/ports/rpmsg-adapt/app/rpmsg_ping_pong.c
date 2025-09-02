#include <stdio.h>
#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include "hal_data.h"

#include <rtthread.h>

#define LOCAL_EPT_ADDR             0x00

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

static THE_MESSAGE msg = {0};

static struct rpmsg_lite_instance *my_rpmsg;

#define RPMSG_LITE_SHMEM_BASE 0x22174000

#define RPMSG_LITE_NS_ANNOUNCE_STRING "rpmsg-m33"

static void app_nameservice_isr_cb(uint32_t new_ept, const char *new_ept_name, uint32_t flags, void *user_data)
{
    uint32_t *data = (uint32_t *)user_data;

    *data = new_ept;
}

void rp_ping_pong(void *param)
{
    volatile uint32_t remote_addr;
    volatile rpmsg_ns_handle ns_handle;

    struct rpmsg_lite_endpoint *my_ept;
    rpmsg_queue_handle my_queue;

    /* Print the initial banner */
    rt_kprintf("\r\nRPMSG Ping-Pong RT-Thread RTOS API Demo...\r\n");

    rt_kprintf("RPMSG Share Base Addr is 0x%x\r\n", RPMSG_LITE_SHMEM_BASE);
    my_rpmsg = rpmsg_lite_remote_init((void *)RPMSG_LITE_SHMEM_BASE, RL_PLATFORM_RA8P1_M33_LINK_ID, RL_NO_FLAGS);

    rt_kprintf("remote init!\r\n");

    rpmsg_lite_wait_for_link_up(my_rpmsg, RL_BLOCK);
    rt_kprintf("Link is up!\r\n");

    my_queue  = rpmsg_queue_create(my_rpmsg);
    my_ept    = rpmsg_lite_create_ept(my_rpmsg, LOCAL_EPT_ADDR, rpmsg_queue_rx_cb, my_queue);
    ns_handle = rpmsg_ns_bind(my_rpmsg, app_nameservice_isr_cb, ((void *)0));

    rt_thread_mdelay(100);

    while (msg.DATA <= 100U)
    {
        rt_kprintf("Waiting for ping...\r\n");
        (void)rpmsg_queue_recv(my_rpmsg, my_queue, (uint32_t *)&remote_addr, (char *)&msg, sizeof(THE_MESSAGE),
                               ((void *)0), RL_BLOCK);
        msg.DATA++;
        rt_kprintf("Sending pong...\r\n");
        (void)rpmsg_lite_send(my_rpmsg, my_ept, remote_addr, (char *)&msg, sizeof(THE_MESSAGE), RL_BLOCK);
    }

    rt_kprintf("Ping pong done, deinitializing...\r\n");

    (void)rpmsg_lite_destroy_ept(my_rpmsg, my_ept);
    my_ept = ((void *)0);
    (void)rpmsg_queue_destroy(my_rpmsg, my_queue);
    my_queue = ((void *)0);
    (void)rpmsg_ns_unbind(my_rpmsg, ns_handle);
    (void)rpmsg_lite_deinit(my_rpmsg);
    my_rpmsg = ((void *)0);
    msg.DATA = 0U;

    rt_kprintf("\n\nAll test procedures have been completed.\n");
}
MSH_CMD_EXPORT(rp_ping_pong, rp_ping_pong);
