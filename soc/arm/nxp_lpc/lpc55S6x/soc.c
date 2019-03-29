/*
 * Copyright (c) 2017, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief System/hardware module for nxp_lpc54114 platform
 *
 * This module provides routines to initialize and support board-level
 * hardware for the nxp_lpc54114 platform.
 */

#include <kernel.h>
#include <device.h>
#include <init.h>
#include <soc.h>
#include <uart.h>
#include <linker/sections.h>
#include <arch/cpu.h>
#include <cortex_m/exc.h>
#include <fsl_power.h>
#include <fsl_clock.h>
#include <fsl_common.h>
#include <fsl_device_registers.h>

/**
 *
 * @brief Initialize the system clock
 *
 * @return N/A
 *
 */

static ALWAYS_INLINE void clkInit(void)
{

#ifdef CONFIG_SOC_LPC55S69_M33_CPU0
    /*!< Set up the clock sources */
    /*!< Configure FRO192M */
    POWER_DisablePD(kPDRUNCFG_PD_FRO192M);               /*!< Ensure FRO is on  */
    CLOCK_SetupFROClocking(12000000U);                   /*!< Set up FRO to the 12 MHz, just for sure */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);                /*!< Switch to FRO 12MHz first to ensure we can change the clock setting */

    CLOCK_SetupFROClocking(96000000U);                   /* Enable FRO HF(96MHz) output */

    CLOCK_SetFLASHAccessCyclesForFreq(12000000U);          /*!< Set FLASH wait states for core */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U, false);         /*!< Set AHBCLKDIV divider to value 1 */

    /*!< Set up clock selectors - Attach clocks to the peripheries */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);                 /*!< Switch MAIN_CLK to FRO12M */

	/* Attach 12 MHz clock to FLEXCOMM0 */
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
#endif /* CONFIG_SOC_LPC55S69_M33_CPU0 */
}

/**
 *
 * @brief Perform basic hardware initialization
 *
 * Initialize the interrupt controller device drivers.
 * Also initialize the timer device driver, if required.
 *
 * @return 0
 */

static int nxp_lpc54114_init(struct device *arg)
{
	ARG_UNUSED(arg);

	/* old interrupt lock level */
	unsigned int oldLevel;

	/* disable interrupts */
	oldLevel = irq_lock();

	_ClearFaults();

	/* Initialize FRO/system clock to 48 MHz */
	clkInit();

	/*
	 * install default handler that simply resets the CPU if configured in
	 * the kernel, NOP otherwise
	 */
	NMI_INIT();

	/* restore interrupt state */
	irq_unlock(oldLevel);

	return 0;
}

SYS_INIT(nxp_lpc54114_init, PRE_KERNEL_1, 0);
