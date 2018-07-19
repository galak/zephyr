/*
 * Copyright (c) 2018 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <init.h>
#include <kernel.h>
#include <pinmux.h>
#include <soc.h>
#include <sys_io.h>
#include <gpio/gpio_cmsdk_ahb.h>

#include "pinmux/pinmux.h"

/**
 * @brief Pinmux driver for ARM MPS2 AN521 Board
 *
 * The ARM MPS2 AN521 Board has 4 GPIO controllers. These controllers
 * are responsible for pin muxing, input/output, pull-up, etc.
 *
 * All GPIO controller pins are exposed via the following sequence of pin
 * numbers:
 *   Pins  0 -  15 are for GPIO0
 *   Pins 16 -  31 are for GPIO1
 *   Pins 32 -  47 are for GPIO2
 *   Pins 48 -  51 are for GPIO3
 *
 * For the GPIO controllers configuration ARM MPS2 AN521 Board follows the
 * Arduino compliant pin out.
 */

#define CMSDK_AHB_GPIO0_DEV \
	((volatile struct gpio_cmsdk_ahb *)CMSDK_AHB_GPIO0)
#define CMSDK_AHB_GPIO1_DEV \
	((volatile struct gpio_cmsdk_ahb *)CMSDK_AHB_GPIO1)
#define CMSDK_AHB_GPIO2_DEV \
	((volatile struct gpio_cmsdk_ahb *)CMSDK_AHB_GPIO2)
#define CMSDK_AHB_GPIO3_DEV \
	((volatile struct gpio_cmsdk_ahb *)CMSDK_AHB_GPIO3)

/*
 * This is the mapping from the ARM MPS2 AN521 Board pins to GPIO
 * controllers.
 *
 * D0 : EXT_0
 * D1 : EXT_4
 * D2 : EXT_2
 * D3 : EXT_3
 * D4 : EXT_1
 * D5 : EXT_6
 * D6 : EXT_7
 * D7 : EXT_8
 * D8 : EXT_9
 * D9 : EXT_10
 * D10 : EXT_12
 * D11 : EXT_13
 * D12 : EXT_14
 * D13 : EXT_11
 * D14 : EXT_15
 * D15 : EXT_5
 * D16 : EXT_16
 * D17 : EXT_17
 * D18 : EXT_18
 * D19 : EXT_19
 * D20 : EXT_20
 * D21 : EXT_21
 * D22 : EXT_22
 * D23 : EXT_23
 * D24 : EXT_24
 * D25 : EXT_25
 * D26 : EXT_26
 * D27 : EXT_30
 * D28 : EXT_28
 * D29 : EXT_29
 * D30 : EXT_27
 * D31 : EXT_32
 * D32 : EXT_33
 * D33 : EXT_34
 * D34 : EXT_35
 * D35 : EXT_36
 * D36 : EXT_38
 * D37 : EXT_39
 * D38 : EXT_40
 * D39 : EXT_44
 * D40 : EXT_41
 * D41 : EXT_31
 * D42 : EXT_37
 * D43 : EXT_42
 * D44 : EXT_43
 * D45 : EXT_45
 * D46 : EXT_46
 * D47 : EXT_47
 * D48 : EXT_48
 * D49 : EXT_49
 * D50 : EXT_50
 * D51 : EXT_51
 *
 * UART_3_RX : D0
 * UART_3_TX : D1
 * SPI_3_CS : D10
 * SPI_3_MOSI : D11
 * SPI_3_MISO : D12
 * SPI_3_SCLK : D13
 * I2C_3_SDA : D14
 * I2C_3_SCL : D15
 * UART_4_RX : D26
 * UART_4_TX : D30
 * SPI_4_CS : D36
 * SPI_4_MOSI : D37
 * SPI_4_MISO : D38
 * SPI_4_SCK : D39
 * I2C_4_SDA : D40
 * I2C_4_SCL : D41
 *
 */

struct arm_scc_reg_map_t {
    volatile uint32_t reset_ctrl;             /* 0x00 RW Reset Control Register */
    volatile uint32_t clk_ctrl;               /* 0x04 RW Clock Control Register*/
    volatile uint32_t pwr_ctrl;               /* 0x08 RW Power Control Register*/
    volatile uint32_t pll_ctrl;               /* 0x0C RW Power Control Register */
    volatile uint32_t dbg_ctrl;               /* 0x10 RW Debug Control Register */
    volatile uint32_t sram_ctrl;              /* 0x14 RW SRAM Control Register */
    volatile uint32_t intr_ctrl;              /* 0x18 RW Interupt Control Register */
    volatile uint32_t reserved1;              /* 0x1C RW reserved */
    volatile uint32_t cpu0_vtor_sram;         /* 0x20 RW Reset vector for CPU0 Secure Mode */
    volatile uint32_t cpu0_vtor_flash;        /* 0x24 RW Reset vector for CPU0 Secure Mode */
    volatile uint32_t cpu1_vtor_sram;         /* 0x28 RW Reset vector for CPU1 Secure Mode */
    volatile uint32_t cpu1_vtor_flash;        /* 0x2C RW Reset vector for CPU0 Secure Mode */
    volatile uint32_t iomux_main_insel;       /* 0x30 RW Main function in data select */
    volatile uint32_t iomux_main_outse;       /* 0x34 RW Main function out data select */
    volatile uint32_t iomux_main_oense;       /* 0x38 RW Main function out enable select */
    volatile uint32_t iomux_main_default_in;  /* 0x3C RW Main function default in select */
    volatile uint32_t iomux_altf1_insel;      /* 0x40 RW Alt function 1 in data select */
    volatile uint32_t iomux_altf1_outse;      /* 0x44 RW Alt function 1 out data select */
    volatile uint32_t iomux_altf1_oense;      /* 0x48 RW Alt function 1 out enable select */
    volatile uint32_t iomux_altf1_default_in; /* 0x4C RW Alt function 1 default in select */
    volatile uint32_t iomux_altf2_insel;      /* 0x50 RW Alt function 2 in data select */
    volatile uint32_t iomux_altf2_outse;      /* 0x54 RW Alt function 2 out data select */
    volatile uint32_t iomux_altf2_oense;      /* 0x58 RW Alt function 2 out enable select */
    volatile uint32_t iomux_altf2_default_in; /* 0x5C RW Alt function 2 default in select */
    volatile uint32_t pvt_ctrl;               /* 0x60 RW PVT control register */
    volatile uint32_t spare0;                 /* 0x64 RW reserved */
    volatile uint32_t iopad_ds0;              /* 0x68 RW Drive Select 0 */
    volatile uint32_t iopad_ds1;              /* 0x6C RW Drive Select 1 */
    volatile uint32_t iopad_pe;               /* 0x70 RW Pull Enable */
    volatile uint32_t iopad_ps;               /* 0x74 RW Pull Select */
    volatile uint32_t iopad_sr;               /* 0x78 RW Slew Select */
    volatile uint32_t iopad_is;               /* 0x7C RW Input Select */
    volatile uint32_t sram_rw_margine;        /* 0x80 RW reserved */
    volatile uint32_t static_conf_sig0;       /* 0x84 RW Static configuration */
    volatile uint32_t static_conf_sig1;       /* 0x88 RW Static configuration */
    volatile uint32_t req_set;                /* 0x8C RW External Event Enable */
    volatile uint32_t req_clear;              /* 0x90 RW External Event Clear */
    volatile uint32_t iomux_altf3_insel;      /* 0x94 RW Alt function 3 in data select */
    volatile uint32_t iomux_altf3_outse;      /* 0x98 RW Alt function 3 out data select */
    volatile uint32_t iomux_altf3_oense;      /* 0x9C RW Alt function 3 out enable select */
    volatile uint32_t iomux_altf3_default_in; /* 0xA0 RW Alt function 3 default in select */
    volatile uint32_t pcsm_ctrl_override;     /* 0xA4 RW Q-Channels QACTIVE Override */
    volatile uint32_t pd_cpu0_iso_override;   /* 0xA8 RW CPU0 Isolation Override */
    volatile uint32_t pd_cpu1_iso_override;   /* 0xAC RW CPU1 Isolation Override */
    volatile uint32_t sys_sram_rw_assist0;    /* 0xB0 RW CPU0 icache sram ldata */
    volatile uint32_t sys_sram_rw_assist1;    /* 0xB4 RW CPU0 icache sram tag */
    volatile uint32_t sys_sram_rw_assist2;    /* 0xB8 RW CPU1 icache sram ldata */
    volatile uint32_t sys_sram_rw_assist3;    /* 0xBC RW CPU1 icache sram tag */
    volatile uint32_t sys_sram_rw_assist4;    /* 0xC0 RW System sram */
    volatile uint32_t sys_sram_rw_assist5;    /* 0xC4 RW System sram */
    volatile uint32_t reserved2[3];           /* reserved */
    volatile uint32_t crypto_sram_rw_assist0; /* 0xD4 RW Crypto ram */
    volatile uint32_t crypto_sram_rw_assist1; /* 0xD8 RW Crypto sec sram */
    volatile uint32_t crypto_sram_rw_assist2; /* 0xDC RW Reserved */
    volatile uint32_t req_edge_sel;           /* 0xC0 RW Power clock request edge select */
    volatile uint32_t req_enable;             /* 0xC4 RW Power clock request enable */
    volatile uint32_t reserved3[28];          /* reserved */
    volatile uint32_t chip_id;                /* 0x100 RO Chip ID 0x0797_0477 */
    volatile uint32_t clock_status;           /* 0x104 RO Clock status */
    volatile uint32_t io_in_status;           /* 0x108 RO I/O in status */
};


#define MUSCA_SCC_DEV \
	((volatile struct arm_scc_reg_map_t *)0x5010C000)

static void arm_mps2_pinmux_defaults(void)
{
#if 0
	u32_t gpio_0 = 0;
	u32_t gpio_1 = 0;
	u32_t gpio_2 = 0;

	/* Set GPIO Alternate Functions */

	gpio_0 = (1<<0)   /* Shield 0 UART 3 RXD */
	       | (1<<4)   /* Shield 0 UART 3 TXD */
	       | (1<<5)   /* Shield 0 I2C SCL SBCON2 */
	       | (1<<15)  /* Shield 0 I2C SDA SBCON2 */
	       | (1<<11)  /* Shield 0 SPI 3 SCK */
	       | (1<<12)  /* Shield 0 SPI 3 SS */
	       | (1<<13)  /* Shield 0 SPI 3 MOSI */
	       | (1<<14); /* Shield 0 SPI 3 MISO */

	CMSDK_AHB_GPIO0_DEV->altfuncset = gpio_0;

	gpio_1 = (1<<10) /* Shield 1 UART 4 RXD */
	       | (1<<14) /* Shield 1 UART 4 TXD */
	       | (1<<15) /* Shield 1 I2C SCL SBCON3 */
	       | (1<<0)  /* ADC SPI 2 SS */
	       | (1<<1)  /* ADC SPI 2 MISO */
	       | (1<<2)  /* ADC SPI 2 MOSI */
	       | (1<<3)  /* ADC SPI 2 SCK */
	       | (1<<5)  /* USER BUTTON 0 */
	       | (1<<6); /* USER BUTTON 1 */

	CMSDK_AHB_GPIO1_DEV->altfuncset = gpio_1;

	gpio_2 = (1<<9)   /* Shield 1 I2C SDA SBCON3 */
	       | (1<<6)   /* Shield 1 SPI 4 SS */
	       | (1<<7)   /* Shield 1 SPI 4 MOSI */
	       | (1<<8)   /* Shield 1 SPI 4 MISO */
	       | (1<<12); /* Shield 1 SPI 4 SCK */

	CMSDK_AHB_GPIO2_DEV->altfuncset = gpio_2;
#endif

#if 1
	/* Enable GPIO0/1 for UART 0 */
	MUSCA_SCC_DEV->iomux_main_insel = 0xfffffffc;
	MUSCA_SCC_DEV->iomux_main_outse = 0xfffffffc;
	MUSCA_SCC_DEV->iomux_main_oense = 0xfffffffc;
#endif
}

void kumar(void)
{
	printk("in kumar\n");
	printk("MUSCA_SCC_DEV %p\n", &(MUSCA_SCC_DEV->iomux_main_insel));
	printk("MUSCA_SCC_DEV->iomux_main_insel %x\n", MUSCA_SCC_DEV->iomux_main_insel);
	printk("MUSCA_SCC_DEV->iomux_main_outse %x\n", MUSCA_SCC_DEV->iomux_main_outse);
	printk("MUSCA_SCC_DEV->iomux_main_oense %x\n", MUSCA_SCC_DEV->iomux_main_oense);
}

static int arm_mps2_pinmux_init(struct device *port)
{
	ARG_UNUSED(port);

	arm_mps2_pinmux_defaults();

	return 0;
}

SYS_INIT(arm_mps2_pinmux_init, PRE_KERNEL_1,
	 CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
