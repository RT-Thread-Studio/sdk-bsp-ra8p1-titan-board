/*
 * Copyright (c) 2025 Renesas Electronics Corporation and/or its affiliates
 * Copyright (c) 2022-2023 NXP (original implementation)
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "mcmgr.h"
#include "mcmgr_internal_core_api.h"
#include "bsp_api.h"
#include "r_ipc.h"
#include "rpmsg_platform.h"

/* Count of cores in the system */
#define MCMGR_CORECOUNT 2

/* Count of memory regions in the system */
#define MCMGR_MEMREGCOUNT 2

/* IPC channel configuration for Core0 and Core1 */
#if (BSP_CFG_CPU_CORE == 0)
    #define MCMGR_IPC_SEND_UNIT    IPC1
    #define MCMGR_IPC_RECV_UNIT    IPC0
    #define IPC_CTRL               g_ipc1_ctrl
#else
    #define MCMGR_IPC_SEND_UNIT    IPC0
    #define MCMGR_IPC_RECV_UNIT    IPC1
    #define IPC_CTRL               g_ipc0_ctrl
#endif

/* Core context for MCMGR */
volatile mcmgr_core_context_t s_mcmgrCoresContext[MCMGR_CORECOUNT] = {
    {.state = kMCMGR_ResetCoreState, .startupData = 0},
    {.state = kMCMGR_ResetCoreState, .startupData = 0}
};

/* Initialize structure with information of all cores */
static const mcmgr_core_info_t s_mcmgrCores[MCMGR_CORECOUNT] = {
    {.coreType = kMCMGR_CoreTypeCortexM85, .coreName = "Main"},
    {.coreType = kMCMGR_CoreTypeCortexM33, .coreName = "Secondary"}
};

/* System information structure */
const mcmgr_system_info_t g_mcmgrSystem = {
    .coreCount = MCMGR_CORECOUNT,
    .memRegCount = MCMGR_MEMREGCOUNT,
    .cores = s_mcmgrCores
};

/* Forward declaration of callback function */
void mcmgr_ipc_callback(ipc_callback_args_t *p_args);

/* Early initialization of MCMGR for a core */
mcmgr_status_t mcmgr_early_init_internal(mcmgr_core_t coreNum)
{
    if ((uint32_t)coreNum >= g_mcmgrSystem.coreCount)
    {
        return kStatus_MCMGR_Error;
    }

    /* Initialize IPC channels for both cores */
    fsp_err_t err = R_IPC_Open(&g_ipc0_ctrl, &g_ipc0_cfg);
    if (err != FSP_SUCCESS)
    {
        return kStatus_MCMGR_Error;
    }
    err = R_IPC_Open(&g_ipc1_ctrl, &g_ipc1_cfg);
    if (err != FSP_SUCCESS)
    {
        return kStatus_MCMGR_Error;
    }

    /* Trigger core up event */
    return MCMGR_TriggerEvent(kMCMGR_RemoteCoreUpEvent, 0);
}

/* Late initialization of MCMGR for a core */
mcmgr_status_t mcmgr_late_init_internal(mcmgr_core_t coreNum)
{
    if ((uint32_t)coreNum >= g_mcmgrSystem.coreCount)
    {
        return kStatus_MCMGR_Error;
    }

    /* IPC interrupt is already enabled in R_IPC_Open */
    return kStatus_MCMGR_Success;
}

/* Start a core with a boot address */
mcmgr_status_t mcmgr_start_core_internal(mcmgr_core_t coreNum, void *bootAddress)
{
    if (coreNum != kMCMGR_Core1)
    {
        return kStatus_MCMGR_Error;
    }

    /* Send core start event using message */
    uint32_t message = (uint32_t)(kMCMGR_RemoteCoreUpEvent << 16) | ((uint32_t)bootAddress & 0xFFFF);
    fsp_err_t err = R_IPC_MessageSend(&IPC_CTRL, message);
    if (err != FSP_SUCCESS)
    {
        return kStatus_MCMGR_Error;
    }
#if (BSP_CFG_CPU_CORE == CPU0) && defined(SOC_SERIES_R7KA8P1)
    R_BSP_SecondaryCoreStart();
#endif

    s_mcmgrCoresContext[coreNum].state = kMCMGR_RunningCoreState;
    return kStatus_MCMGR_Success;
}

/* Retrieve startup data for a core */
mcmgr_status_t mcmgr_get_startup_data_internal(mcmgr_core_t coreNum, uint32_t *startupData)
{
    if (coreNum != kMCMGR_Core1 || startupData == NULL)
    {
        return kStatus_MCMGR_Error;
    }

    if (s_mcmgrCoresContext[coreNum].state >= kMCMGR_RunningCoreState)
    {
        *startupData = s_mcmgrCoresContext[coreNum].startupData;
        return kStatus_MCMGR_Success;
    }

    return kStatus_MCMGR_NotReady;
}

/* Stop a core */
mcmgr_status_t mcmgr_stop_core_internal(mcmgr_core_t coreNum)
{
    if (coreNum != kMCMGR_Core1)
    {
        return kStatus_MCMGR_Error;
    }

    /* Send core stop event using message */
    uint32_t message = (uint32_t)(kMCMGR_RemoteCoreDownEvent << 16);
    fsp_err_t err = R_IPC_MessageSend(&IPC_CTRL, message);
    if (err != FSP_SUCCESS)
    {
        return kStatus_MCMGR_Error;
    }

    s_mcmgrCoresContext[coreNum].state = kMCMGR_ResetCoreState;
    return kStatus_MCMGR_Success;
}

/* Get core properties (not implemented) */
mcmgr_status_t mcmgr_get_core_property_internal(mcmgr_core_t coreNum,
                                               mcmgr_core_property_t property,
                                               void *value,
                                               uint32_t *length)
{
    return kStatus_MCMGR_NotImplemented;
}

/* Get the current core */
mcmgr_core_t mcmgr_get_current_core_internal(void)
{
    return (BSP_CFG_CPU_CORE == 0) ? kMCMGR_Core0 : kMCMGR_Core1;
}

/* Trigger an event to another core */
mcmgr_status_t mcmgr_trigger_event_internal(uint32_t remoteData, bool forcedWrite)
{
    fsp_err_t err = R_IPC_MessageSend(&IPC_CTRL, remoteData);
    if (err != FSP_SUCCESS)
    {
        return kStatus_MCMGR_Error;
    }

    return kStatus_MCMGR_Success;
}

/* IPC callback for MCMGR events */
void mcmgr_ipc_callback(ipc_callback_args_t *p_args)
{
    rt_interrupt_enter();
    if (p_args->event == IPC_EVENT_MESSAGE_RECEIVED)
    {
        uint32_t data = p_args->message;
        uint16_t eventType = (uint16_t)(data >> 16u);
        uint16_t eventData = (uint16_t)(data & 0xFFFFu);

        if (eventType >= kMCMGR_RemoteCoreUpEvent && eventType < kMCMGR_EventTableLength)
        {
            if (MCMGR_eventTable[eventType].callback != NULL)
            {
                MCMGR_eventTable[eventType].callback(eventData, MCMGR_eventTable[eventType].callbackData);
            }
        }
    }
    rt_interrupt_leave();
}

#if defined(MCMGR_HANDLE_EXCEPTIONS) && (MCMGR_HANDLE_EXCEPTIONS == 1)
/* Exception handlers */
void DefaultISR(void)
{
    uint32_t exceptionNumber = __get_IPSR();
    (void)MCMGR_TriggerEvent(kMCMGR_RemoteExceptionEvent, (uint16_t)exceptionNumber);
    for (;;)
    {
    }
}

void HardFault_Handler(void)
{
    DefaultISR();
}

void NMI_Handler(void)
{
    DefaultISR();
}

void MemManage_Handler(void)
{
    DefaultISR();
}

void BusFault_Handler(void)
{
    DefaultISR();
}

void UsageFault_Handler(void)
{
    DefaultISR();
}
#endif /* MCMGR_HANDLE_EXCEPTIONS */
