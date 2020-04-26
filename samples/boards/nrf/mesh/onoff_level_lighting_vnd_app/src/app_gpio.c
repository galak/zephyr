/* Bluetooth: Mesh Generic OnOff, Generic Level, Lighting & Vendor Models
 *
 * Copyright (c) 2018 Vikrant More
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/gpio.h>

#include "app_gpio.h"
#include "publisher.h"

struct device *led_device[4];
struct device *button_device[4];

K_WORK_DEFINE(button_work, publish);

static void button_pressed(struct device *dev,
			   struct gpio_callback *cb, u32_t pins)
{
	k_work_submit(&button_work);
}

void app_gpio_init(void)
{
	static struct gpio_callback button_cb[4];

	/* LEDs configuration & setting */

	led_device[0] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(led0), gpios));
	gpio_pin_configure(led_device[0], DT_GPIO_PIN(DT_ALIAS(led0), gpios),
			   DT_GPIO_FLAGS(DT_ALIAS(led0), gpios) | GPIO_OUTPUT_INACTIVE);

#ifndef ONE_LED_ONE_BUTTON_BOARD
	led_device[1] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(led1), gpios));
	gpio_pin_configure(led_device[1], DT_GPIO_PIN(DT_ALIAS(led1), gpios),
			   DT_GPIO_FLAGS(DT_ALIAS(led1), gpios) | GPIO_OUTPUT_INACTIVE);

	led_device[2] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(led2), gpios));
	gpio_pin_configure(led_device[2], DT_GPIO_PIN(DT_ALIAS(led2), gpios),
			   DT_GPIO_FLAGS(DT_ALIAS(led2), gpios) | GPIO_OUTPUT_INACTIVE);

	led_device[3] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(led3), gpios));
	gpio_pin_configure(led_device[3], DT_GPIO_PIN(DT_ALIAS(led3), gpios),
			   DT_GPIO_FLAGS(DT_ALIAS(led3), gpios) | GPIO_OUTPUT_INACTIVE);
#endif
	/* Buttons configuration & setting */

	k_work_init(&button_work, publish);

	button_device[0] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw0), gpios));
	gpio_pin_configure(button_device[0], DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
			   GPIO_INPUT | GPIO_INT_DEBOUNCE |
			   DT_GPIO_FLAGS(DT_ALIAS(sw0), gpios));
	gpio_pin_interrupt_configure(button_device[0], DT_GPIO_PIN(DT_ALIAS(sw0), gpios),
				     GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb[0], button_pressed,
			   BIT(DT_GPIO_PIN(DT_ALIAS(sw0), gpios)));
	gpio_add_callback(button_device[0], &button_cb[0]);

#ifndef ONE_LED_ONE_BUTTON_BOARD
	button_device[1] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw1), gpios));
	gpio_pin_configure(button_device[1], DT_GPIO_PIN(DT_ALIAS(sw1), gpios),
			   GPIO_INPUT | GPIO_INT_DEBOUNCE |
			   DT_GPIO_FLAGS(DT_ALIAS(sw1), gpios));
	gpio_pin_interrupt_configure(button_device[1], DT_GPIO_PIN(DT_ALIAS(sw1), gpios),
				     GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb[1], button_pressed,
			   BIT(DT_GPIO_PIN(DT_ALIAS(sw1), gpios)));
	gpio_add_callback(button_device[1], &button_cb[1]);

	button_device[2] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw2), gpios));
	gpio_pin_configure(button_device[2], DT_GPIO_PIN(DT_ALIAS(sw2), gpios),
			   GPIO_INPUT | GPIO_INT_DEBOUNCE |
			   DT_GPIO_FLAGS(DT_ALIAS(sw2), gpios));
	gpio_pin_interrupt_configure(button_device[2], DT_GPIO_PIN(DT_ALIAS(sw2), gpios),
				     GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb[2], button_pressed,
			   BIT(DT_GPIO_PIN(DT_ALIAS(sw2), gpios)));
	gpio_add_callback(button_device[2], &button_cb[2]);

	button_device[3] = device_get_binding(DT_GPIO_LABEL(DT_ALIAS(sw3), gpios));
	gpio_pin_configure(button_device[3], DT_GPIO_PIN(DT_ALIAS(sw3), gpios),
			   GPIO_INPUT | GPIO_INT_DEBOUNCE |
			   DT_GPIO_FLAGS(DT_ALIAS(sw3), gpios));
	gpio_pin_interrupt_configure(button_device[3], DT_GPIO_PIN(DT_ALIAS(sw3), gpios),
				     GPIO_INT_EDGE_TO_ACTIVE);
	gpio_init_callback(&button_cb[3], button_pressed,
			   BIT(DT_GPIO_PIN(DT_ALIAS(sw3), gpios)));
	gpio_add_callback(button_device[3], &button_cb[3]);
#endif
}
