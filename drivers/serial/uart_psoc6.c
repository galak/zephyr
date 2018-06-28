/*
 * Copyright (c) 2018 Cypress
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file
 * @brief UART driver for Cypress PSoC6 MCU family.
 *
 * Note:
 * - Error handling is not implemented.
 * - The driver works only in polling mode, interrupt mode is not implemented.
 * - Only port 12 is supported yet
 */
#include <device.h>
#include <errno.h>
#include <init.h>
#include <misc/__assert.h>
#include <soc.h>
#include <uart.h>
#include <board.h>

#include "cy_syslib.h"
#include "cy_sysclk.h"
#include "cy_gpio.h"
#include "cycfg_pins.h"
#include "cy_scb_uart.h"

/*
 * Verify Kconfig configuration
 */

#ifdef CONFIG_UART_PSOC6_PORT_12
/* Assign pins for UART on SCB6: P12[0], P12[1] */
#define UART_PORT       P12_0_PORT
#define UART_RX_NUM     P12_0_NUM
#define UART_TX_NUM     P12_1_NUM
#define UART_RX_VAL     P12_0_SCB6_UART_RX
#define UART_TX_VAL     P12_1_SCB6_UART_TX
#define UART_CLOCK      PCLK_SCB6_CLOCK
#define UART_SCB        SCB6
#endif /* CONFIG_UART_PSOC6_PORT_12 */

/* Assign divider type and number for UART */
#define UART_CLK_DIV_TYPE     (CY_SYSCLK_DIV_8_BIT)
#define UART_CLK_DIV_NUMBER   (PERI_DIV_8_NR - 1u)

//#endif

#ifdef CONFIG_UART_PSOC6_PORT_5
/* Assign pins for UART on SCB5: P5[0], P5[1] */
#define UART_PORT       P5_0_PORT
#define UART_RX_NUM     P5_0_NUM
#define UART_TX_NUM     P5_1_NUM
#define UART_RX_VAL     P5_0_SCB5_UART_RX
#define UART_TX_VAL     P5_1_SCB5_UART_TX
#define UART_CLOCK      PCLK_SCB5_CLOCK
#define UART_SCB        SCB5

/* Assign divider type and number for UART */
#define UART_CLK_DIV_TYPE     (CY_SYSCLK_DIV_8_BIT)
#define UART_CLK_DIV_NUMBER   (PERI_DIV_8_NR - 1u)

#endif /* CONFIG_UART_PSOC6_PORT_5 */

/* Allocate context for UART operation */
cy_stc_scb_uart_context_t uartContext;

/* Populate configuration structure */
const cy_stc_scb_uart_config_t uartConfig =
{
	.uartMode                   = CY_SCB_UART_STANDARD,
	.enableMutliProcessorMode   = false,
	.smartCardRetryOnNack       = false,
	.irdaInvertRx               = false,
	.irdaEnableLowPowerReceiver = false,

	.oversample                 = 12UL,

	.enableMsbFirst             = false,
	.dataWidth                  = 8UL,
	.parity                     = CY_SCB_UART_PARITY_NONE,
	.stopBits                   = CY_SCB_UART_STOP_BITS_1,
	.enableInputFilter          = false,
	.breakWidth                 = 11UL,
	.dropOnFrameError           = false,
	.dropOnParityError          = false,

	.receiverAddress            = 0UL,
	.receiverAddressMask        = 0UL,
	.acceptAddrInFifo           = false,

	.enableCts                  = false,
	.ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
	.rtsRxFifoLevel             = 0UL,
	.rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,

	.rxFifoTriggerLevel  = 0UL,
	.rxFifoIntEnableMask = 0UL,
	.txFifoTriggerLevel  = 0UL,
	.txFifoIntEnableMask = 0UL,
};

/*******************************************************************************
* Function Name: uart_psoc6_init()
********************************************************************************
*
*  Peforms hardware initialization: debug UART.
*
*******************************************************************************/
static int uart_psoc6_init(struct device *dev)
{
	/* Connect SCB5 UART function to pins */
	Cy_GPIO_SetHSIOM(UART_PORT, UART_RX_NUM, UART_RX_VAL);
	Cy_GPIO_SetHSIOM(UART_PORT, UART_TX_NUM, UART_TX_VAL);

	/* Configure pins for UART operation */
	Cy_GPIO_SetDrivemode(UART_PORT, UART_RX_NUM, CY_GPIO_DM_HIGHZ);
	Cy_GPIO_SetDrivemode(UART_PORT, UART_TX_NUM, CY_GPIO_DM_STRONG_IN_OFF);

	/* Connect assigned divider to be a clock source for UART */
	Cy_SysClk_PeriphAssignDivider(UART_CLOCK, UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);

	/* UART desired baud rate is 115200 bps (Standard mode).
	* The UART baud rate = (SCB clock frequency / Oversample).
	* For PeriClk = 50 MHz, select divider value 36 and get SCB clock = (50 MHz / 36) = 1,389 MHz.
	* Select Oversample = 12. These setting results UART data rate = 1,389 MHz / 12 = 115750 bps.
	*/
	Cy_SysClk_PeriphSetDivider   (UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER, 35UL);
	Cy_SysClk_PeriphEnableDivider(UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);

	/* Configure UART to operate */
	(void) Cy_SCB_UART_Init(UART_SCB, &uartConfig, &uartContext);
	Cy_SCB_UART_Enable(UART_SCB);

	return(0);
}

static int uart_psoc6_poll_in(struct device *dev, unsigned char *c)
{
	uint32_t rec;

	//Uart *const uart = DEV_CFG(dev)->regs;

	rec = Cy_SCB_UART_Get(UART_SCB);
	*c = (unsigned char)(rec & 0xff);

	return((rec == CY_SCB_UART_RX_NO_DATA) ? -1 : 0);
}

static unsigned char uart_psoc6_poll_out(struct device *dev, unsigned char c)
{
	//Uart *const uart = DEV_CFG(dev)->regs;

	while(1UL != Cy_SCB_UART_Put(UART_SCB, (uint32_t)c));

	return c;
}


static const struct uart_driver_api uart_psoc6_driver_api = {
	.poll_in = uart_psoc6_poll_in,
	.poll_out = uart_psoc6_poll_out,
};

#ifdef CONFIG_UART_PSOC6_PORT_12
DEVICE_AND_API_INIT(uart_psoc6_12, "uart_port12",
		    uart_psoc6_init, NULL,
		    &uartConfig,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    (void *)&uart_psoc6_driver_api);
#endif /* CONFIG_UART_PSOC6_PORT_12 */

#ifdef CONFIG_UART_PSOC6_PORT_5
DEVICE_AND_API_INIT(uart_psoc6_5, "uart_port5",
		    uart_psoc6_init, NULL,
		    &uartConfig,
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
		    (void *)&uart_psoc6_driver_api);
#endif /* CONFIG_UART_PSOC6_PORT_5 */
