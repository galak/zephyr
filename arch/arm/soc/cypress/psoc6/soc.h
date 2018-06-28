/*
 * Copyright (c) 2018, CYPRESS
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Board configuration macros
 *
 * This header file is used to specify and describe board-level aspects
 */

#ifndef _ASMLANGUAGE
#include <cy_device_headers.h>
#endif /* !_ASMLANGUAGE */

#ifndef _SOC__H_
#define _SOC__H_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS               CONFIG_NUM_IRQ_PRIO_BITS
#endif
#define __Vendor_SysTickConfig         0 /* Default to standard SysTick */

#ifdef __cplusplus
}
#endif

#endif /* _SOC__H_ */
