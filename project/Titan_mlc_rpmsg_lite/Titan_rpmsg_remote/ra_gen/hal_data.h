/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_ipc.h"
#include "r_sci_b_uart.h"
            #include "r_uart_api.h"
FSP_HEADER
/** IPC Instance. */
            extern const ipc_instance_t g_ipc1;

            /** Access the IPC instance using these structures when calling API functions directly
            (::p_api is not used). */
            extern ipc_instance_ctrl_t g_ipc1_ctrl;
            extern const ipc_cfg_t g_ipc1_cfg;

#ifndef NULL
            void NULL(ipc_callback_args_t * p_args);
#endif
/** IPC Instance. */
            extern const ipc_instance_t g_ipc0;

            /** Access the IPC instance using these structures when calling API functions directly
            (::p_api is not used). */
            extern ipc_instance_ctrl_t g_ipc0_ctrl;
            extern const ipc_cfg_t g_ipc0_cfg;

#ifndef g_ipc0_callback
            void g_ipc0_callback(ipc_callback_args_t * p_args);
#endif
/** UART on SCI Instance. */
            extern const uart_instance_t      g_uart5;

            /** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
            extern sci_b_uart_instance_ctrl_t     g_uart5_ctrl;
            extern const uart_cfg_t g_uart5_cfg;
            extern const sci_b_uart_extended_cfg_t g_uart5_cfg_extend;

            #ifndef user_uart5_callback
            void user_uart5_callback(uart_callback_args_t * p_args);
            #endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
