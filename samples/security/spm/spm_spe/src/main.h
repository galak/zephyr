/*******************************************************************************
* \file main.h
*
* \version  1.0
*
* \brief Function prototypes and constants available to the example project.
*
********************************************************************************
* \copyright
*
* © 2018, Cypress Semiconductor Corporation
* or a subsidiary of Cypress Semiconductor Corporation. All rights
* reserved.
*
* This software, including source code, documentation and related
* materials (“Software”), is owned by Cypress Semiconductor
* Corporation or one of its subsidiaries (“Cypress”) and is protected by
* and subject to worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, non-
* exclusive, non-transferable license to copy, modify, and compile the
* Software source code solely for use in connection with Cypress’s
* integrated circuit products. Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE. Cypress reserves the right to make
* changes to the Software without notice. Cypress does not assume any
* liability arising out of the application or use of the Software or any
* product or circuit described in the Software. Cypress does not
* authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
******************************************************************************/
#ifndef MAIN_H
#define MAIN_H


#include "cmsis_compiler.h"
#include <stdio.h>
#include "cy_scb_uart.h"
#include "cy_sysint.h"
#include "cy_sysclk.h"
#include "cy_ipc_config.h"
#include "cy_ipc_drv.h"

// IPC channel for SPM IPC
#define SPM_IPC_CHANNEL 8u

// Interrupts for SPM IPC
#define SPM_IPC_NOTIFY_CM4_INTR     (CY_IPC_INTR_SPARE + 1) // CM0+ to CM4 notify interrupt number
#define SPM_IPC_NOTIFY_CM0P_INTR    (CY_IPC_INTR_SPARE + 2) // CM4 to CM0+ notify interrupt number


#define UART_DEB_PUT_CHAR(...)                  while(1UL != Cy_SCB_UART_Put(SCB6, __VA_ARGS__))
#define DBG_PRINTF(...)                         (printf(__VA_ARGS__))

/* Assign pins for UART on SCB6: P12[0], P12[1] */
#define UART_PORT       P12_0_PORT
#define UART_RX_NUM     P12_0_NUM
#define UART_TX_NUM     P12_1_NUM

/* Assign divider type and number for UART */
#define UART_CLK_DIV_TYPE     (CY_SYSCLK_DIV_8_BIT)
#define UART_CLK_DIV_NUMBER   (PERI_DIV_8_NR - 1u)

#endif /* MAIN_H */

/* [] END OF FILE */
