/* pinmux_board_arduino_due.c - Arduino Due pinmux driver */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <kernel.h>
#include <pinmux.h>
#include <soc.h>
#include <sys_io.h>

#include "pinmux/pinmux.h"

static void __pinmux_defaults(void)
{

}

static int pinmux_init(struct device *port)
{
	ARG_UNUSED(port);
	port = NULL;

	__pinmux_defaults();

	return 0;
}

SYS_INIT(pinmux_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
