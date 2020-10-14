/*
 * Copyright (c) 2020, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_lpc_syscon
#include <errno.h>
#include <soc.h>
#include <drivers/clock_control.h>
#include <dt-bindings/clock/mcux_lpc_syscon_clock.h>
#include <fsl_clock.h>

#define LOG_LEVEL CONFIG_CLOCK_CONTROL_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(clock_control);

static int mcux_lpc_syscon_clock_control_on(const struct device *dev,
			      clock_control_subsys_t sub_system)
{
	return 0;
}

static int mcux_lpc_syscon_clock_control_off(const struct device *dev,
			       clock_control_subsys_t sub_system)
{
	return 0;
}

static int mcux_lpc_syscon_clock_control_get_subsys_rate(
					const struct device *dev,
				    clock_control_subsys_t sub_system,
				    uint32_t *rate)
{
#if defined(CONFIG_I2C_MCUX_FLEXCOMM) || \
		defined(CONFIG_SPI_MCUX_FLEXCOMM) || \
		defined(CONFIG_UART_MCUX_FLEXCOMM)

	uint32_t clock_name = (uint32_t) sub_system;

	switch (clock_name) {
	case kCLOCK_Flexcomm0:
		*rate = CLOCK_GetFlexCommClkFreq(0);
		break;
	case kCLOCK_Flexcomm1:
		*rate = CLOCK_GetFlexCommClkFreq(1);
		break;
	case kCLOCK_Flexcomm2:
		*rate = CLOCK_GetFlexCommClkFreq(2);
		break;
	case kCLOCK_Flexcomm3:
		*rate = CLOCK_GetFlexCommClkFreq(3);
		break;
	case kCLOCK_Flexcomm4:
		*rate = CLOCK_GetFlexCommClkFreq(4);
		break;
	case kCLOCK_Flexcomm5:
		*rate = CLOCK_GetFlexCommClkFreq(5);
		break;
	case kCLOCK_Flexcomm6:
		*rate = CLOCK_GetFlexCommClkFreq(6);
		break;
	case kCLOCK_Flexcomm7:
		*rate = CLOCK_GetFlexCommClkFreq(7);
		break;
	case kCLOCK_Hs_Lspi:
		*rate = CLOCK_GetHsLspiClkFreq();
		break;
	}
#endif

	return 0;
}

static int mcux_lpc_syscon_clock_control_init(const struct device *dev)
{
	return 0;
}

static const struct clock_control_driver_api mcux_lpc_syscon_api = {
	.on = mcux_lpc_syscon_clock_control_on,
	.off = mcux_lpc_syscon_clock_control_off,
	.get_rate = mcux_lpc_syscon_clock_control_get_subsys_rate,
};

#define LPC_CLOCK_INIT(n) \
	\
DEVICE_AND_API_INIT(mcux_lpc_syscon_##n, DT_INST_LABEL(n), \
		    &mcux_lpc_syscon_clock_control_init, \
		    NULL, NULL, \
		    PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEVICE, \
		    &mcux_lpc_syscon_api);

DT_INST_FOREACH_STATUS_OKAY(LPC_CLOCK_INIT)
