/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>

#include "main.h"
#include "cy_syslib.h"
#include "cy_gpio.h"
#include "cycfg_pins.h"
#include "cy_scb_uart.h"
#include <string.h>

#define CY_CORTEX_M4_APPL_ADDR_SPM  0x10060000
#define SHARED_MEM_STARTD_ADDR      0x08023000
#define SHARED_MEM_END_ADDR         0x08024000

// Forward Declarations
// --------------------

IPC_STRUCT_Type             *ipc_channel_handle;
static IPC_INTR_STRUCT_Type *ipc_interrupt_ptr;

void on_vacancy(void *arg)
{
    CY_ASSERT(arg != NULL); // In this implementation, arg must not be NULL
    IPC_STRUCT_Type *ipc_chan_handle = (IPC_STRUCT_Type *)arg;
    
    printf("--->CB: CM0+ TX Q became NOT FULL!\n");
    
    Cy_IPC_Drv_AcquireNotify(ipc_chan_handle, (1uL << SPM_IPC_NOTIFY_CM4_INTR));
};

/*******************************************************************************
* Function Name: ipc_interrupt_handler
*******************************************************************************/
void ipc_interrupt_handler(void)
{
	char *__shared_memory_start = (char *)SHARED_MEM_STARTD_ADDR;
    printk("CM0+: ipc_interrupt_handler INTR = 0x%x\n", (unsigned int)(ipc_interrupt_ptr->INTR));
	printk("CM0+: message: %s\n", __shared_memory_start);
	
	on_vacancy(ipc_channel_handle);

    // Clear the interrupt and make a dummy read to avoid double interrupt occurrence:
    // - The double interrupt’s triggering is caused by buffered write operations on bus
    // - The dummy read of the status register is indeed required to make sure previous write completed before leaving ISR
    // Note: This is a direct clear using the IPC interrupt register and not clear of an NVIC register
    Cy_IPC_Drv_ClearInterrupt(ipc_interrupt_ptr, CY_IPC_NO_NOTIFICATION, (1uL << SPM_IPC_CHANNEL));
}

void main(void)
{
	printk("CM0+: Hello World! %s\n", CONFIG_ARCH);
	
	// Interrupts configuration for CM0+
    // * See ce216795_common.h for occupied interrupts
    // -----------------------------------------------

    // Configure interrupts ISR / MUX and priority
    cy_stc_sysint_t ipc_intr_Config;
    ipc_intr_Config.intrSrc = (IRQn_Type)NvicMux3_IRQn; // Can be any Mux we choose
    ipc_intr_Config.cm0pSrc = (cy_en_intr_t)cpuss_interrupts_ipc_0_IRQn + SPM_IPC_NOTIFY_CM0P_INTR;    // Must match the interrupt we trigger using NOTIFY on CM4
    ipc_intr_Config.intrPriority = 1;
    (void)Cy_SysInt_Init(&ipc_intr_Config, ipc_interrupt_handler);

    // Set specific NOTIFY interrupt mask only.
    // Only the interrupt sources with their masks enabled can trigger the interrupt.
    ipc_interrupt_ptr = Cy_IPC_Drv_GetIntrBaseAddr(SPM_IPC_NOTIFY_CM0P_INTR);
    Cy_IPC_Drv_SetInterruptMask(ipc_interrupt_ptr, 0x0, 1 << SPM_IPC_CHANNEL);

    // Enable the interrupt
    NVIC_EnableIRQ(ipc_intr_Config.intrSrc);

    ipc_channel_handle = Cy_IPC_Drv_GetIpcBaseAddress(SPM_IPC_CHANNEL);

	/* Init interrupt in zephyr */
    IRQ_DIRECT_CONNECT(NvicMux3_IRQn, 1, ipc_interrupt_handler, 0);
    irq_enable(NvicMux3_IRQn);

    // Enable CM4
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR_SPM);
	
	while(1)
	{
        Cy_GPIO_Inv(LED_RED_PORT, LED_RED_PIN); /* toggle the pin */
        Cy_SysLib_Delay(3000);
	}
}
 