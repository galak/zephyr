/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __DT_BINDING_CY_MEM_H
#define __DT_BINDING_CY_MEM_H

#define __SIZE_K(x) (x * 1024)

#define DT_FLASH_ADDR		0x10000000
#define DT_FLASH_SIZE		__SIZE_K(1024)
#define DT_FLASH_M0P_ADDR		0x10000000
#define DT_FLASH_M0P_SIZE		__SIZE_K(384)
#define DT_FLASH_M4_ADDR		0x10060000
/* (DT_FLASH_M0P_ADDR + DT_FLASH_M0P_SIZE) */
#define DT_FLASH_M4_SIZE		__SIZE_K(640)

#define DT_RAM_ADDR			0x08000000
#define DT_SRAM_SIZE		__SIZE_K(256/2)
#define DT_SRAM_M0P_ADDR		0x08000000
#define DT_SRAM_M0P_SIZE		__SIZE_K(140)
#define DT_SRAM_SHAR_ADDR		0x08023000
/* (DT_SRAM_M0P_ADDR + DT_SRAM_M0P_SIZE) */
#define DT_SRAM_SHAR_SIZE		__SIZE_K(4)
#define DT_SRAM_M4_ADDR		0x08024000
/* (DT_SRAM_SHAR_ADDR + DT_SRAM_SHAR_SIZE) */
#define DT_SRAM_M4_SIZE		__SIZE_K(112)

#if 0
#error "Flash and RAM sizes not defined for this chip"
#endif

#endif /* __DT_BINDING_CY_MEM_H */
