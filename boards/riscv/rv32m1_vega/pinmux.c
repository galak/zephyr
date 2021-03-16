/*
 * Copyright 2018 Foundries.io Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <init.h>
#include <drivers/pinmux.h>
#include <drivers/gpio.h>
#include <fsl_port.h>

#ifdef CONFIG_BT_CTLR_DEBUG_PINS
const struct device *vega_debug_portb;
const struct device *vega_debug_portc;
const struct device *vega_debug_portd;
#endif

static int rv32m1_vega_pinmux_init(const struct device *dev)
{
	ARG_UNUSED(dev);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(porta), okay)
	__unused const struct device *porta =
		DEVICE_DT_GET(DT_NODELABEL(porta));
	__ASSERT_NO_MSG(device_is_ready(porta));
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(portb), okay)
	__unused const struct device *portb =
		DEVICE_DT_GET(DT_NODELABEL(portb));
	__ASSERT_NO_MSG(device_is_ready(portb));
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(portc), okay)
	__unused const struct device *portc =
		DEVICE_DT_GET(DT_NODELABEL(portc));
	__ASSERT_NO_MSG(device_is_ready(portc));
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(portd), okay)
	__unused const struct device *portd =
		DEVICE_DT_GET(DT_NODELABEL(portd));
	__ASSERT_NO_MSG(device_is_ready(portd));
#endif
#if DT_NODE_HAS_STATUS(DT_NODELABEL(porte), okay)
	__unused const struct device *porte =
		DEVICE_DT_GET(DT_NODELABEL(porte));
	__ASSERT_NO_MSG(device_is_ready(porte));
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpuart0), okay) && CONFIG_SERIAL
	/* LPUART0 RX, TX */
	pinmux_pin_set(portc, 7, kPORT_MuxAlt3);
	pinmux_pin_set(portc, 8, kPORT_MuxAlt3);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpuart1), okay) && CONFIG_SERIAL
	/* LPUART1 RX, TX */
	pinmux_pin_set(portc, 29, kPORT_MuxAlt2);
	pinmux_pin_set(portc, 30, kPORT_MuxAlt2);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpi2c0), okay) && CONFIG_I2C
	/* LPI2C0 SCL, SDA - Arduino header */
	pinmux_pin_set(portc, 10, kPORT_MuxAlt4);
	pinmux_pin_set(portc, 9, kPORT_MuxAlt4);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpi2c3), okay) && CONFIG_I2C
	/* LPI2C3 SCL, SDA - FXOS8700 */
	pinmux_pin_set(porte, 30, kPORT_MuxAlt3);
	pinmux_pin_set(porte, 29, kPORT_MuxAlt3);
#endif

	/* FXOS8700 INT1, INT2, RST */
	pinmux_pin_set(porte, 1, kPORT_MuxAsGpio);
	pinmux_pin_set(porte, 22, kPORT_MuxAsGpio);
	pinmux_pin_set(porte, 27, kPORT_MuxAsGpio);

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpspi0), okay) && CONFIG_SPI
	/* LPSPI0 SCK, SOUT, PCS2, SIN */
	pinmux_pin_set(portb,  4, kPORT_MuxAlt2);
	pinmux_pin_set(portb,  5, kPORT_MuxAlt2);
	pinmux_pin_set(portb,  6, kPORT_MuxAlt2);
	pinmux_pin_set(portb,  7, kPORT_MuxAlt2);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(lpspi1), okay) && CONFIG_SPI
	/* LPSPI1 SCK, SIN, SOUT, CS */
	pinmux_pin_set(portb, 20, kPORT_MuxAlt2);
	pinmux_pin_set(portb, 21, kPORT_MuxAlt2);
	pinmux_pin_set(portb, 24, kPORT_MuxAlt2);
	pinmux_pin_set(portb, 22, kPORT_MuxAlt2);
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(tpm2), okay) && CONFIG_PWM
	/* RGB LEDs as PWM */
	pinmux_pin_set(porta, 22, kPORT_MuxAlt6);
	pinmux_pin_set(porta, 23, kPORT_MuxAlt6);
	pinmux_pin_set(porta, 24, kPORT_MuxAlt6);
#else
	/* RGB LEDs as GPIO */
	pinmux_pin_set(porta, 22, kPORT_MuxAsGpio);
	pinmux_pin_set(porta, 23, kPORT_MuxAsGpio);
	pinmux_pin_set(porta, 24, kPORT_MuxAsGpio);
#endif

#ifdef CONFIG_BT_CTLR_DEBUG_PINS

	pinmux_pin_set(portb, 29, kPORT_MuxAsGpio);

	pinmux_pin_set(portc, 28, kPORT_MuxAsGpio);
	pinmux_pin_set(portc, 29, kPORT_MuxAsGpio);
	pinmux_pin_set(portc, 30, kPORT_MuxAsGpio);

	pinmux_pin_set(portd, 0, kPORT_MuxAsGpio);
	pinmux_pin_set(portd, 1, kPORT_MuxAsGpio);
	pinmux_pin_set(portd, 2, kPORT_MuxAsGpio);
	pinmux_pin_set(portd, 3, kPORT_MuxAsGpio);
	pinmux_pin_set(portd, 4, kPORT_MuxAsGpio);
	pinmux_pin_set(portd, 5, kPORT_MuxAsGpio);

	const struct device *gpio_dev =
		device_get_binding(DT_LABEL(DT_NODELABEL(gpiob)));

	gpio_pin_configure(gpio_dev, 29, GPIO_OUTPUT);

	gpio_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpioc)));
	gpio_pin_configure(gpio_dev, 28, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 29, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 30, GPIO_OUTPUT);

	gpio_dev = device_get_binding(DT_LABEL(DT_NODELABEL(gpiod)));
	gpio_pin_configure(gpio_dev, 0, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 1, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 2, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 3, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 4, GPIO_OUTPUT);
	gpio_pin_configure(gpio_dev, 5, GPIO_OUTPUT);
#endif

	return 0;
}

SYS_INIT(rv32m1_vega_pinmux_init, PRE_KERNEL_1, CONFIG_PINMUX_INIT_PRIORITY);
