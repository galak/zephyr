/*
 * Copyright (c) 2018, CYPRESS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <arch/cpu.h>
#include <cortex_m/exc.h>
#include <cycfg_platform.h>

#include "cy_syslib.h"
#include "cy_gpio.h"
#include "cycfg_pins.h"
#include "cy_scb_uart.h"


/*******************************************************************************
* Function Name: InitHardware()
********************************************************************************
*
*  Peforms hardware initialization: LEDs.
*
*******************************************************************************/
void InitHardware(void)
{
    /* Initialize LEDs */
    Cy_GPIO_Pin_FastInit(LED_RED_PORT, LED_RED_PIN, CY_GPIO_DM_STRONG_IN_OFF, 1u, HSIOM_SEL_GPIO);
}

static int init_cycfg_platform_wraper(struct device *arg)
{
	ARG_UNUSED(arg);
	SystemInit();
	InitHardware();
	return 0;
}

SYS_INIT(init_cycfg_platform_wraper, PRE_KERNEL_1, 0);
