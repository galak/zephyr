/*
 * Copyright (c) 2018 Cypress
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr.h>
#include <misc/printk.h>

// Includes
// --------

#include <stdio.h>

#include "cy_gpio.h"
#include "cy_scb_uart.h"
#include "cy_sysint.h"
#include "cy_sysclk.h"
#include "cy_ipc_drv.h"
#include "ce216795_common.h"



//#include "cy_zephyr_rtx.h"

// Definitions
// -----------

#define SHARED_MEM_STARTD_ADDR  0x08023000


// Threads
// -------

IPC_STRUCT_Type             *ipc_channel_handle;
static IPC_INTR_STRUCT_Type *ipc_interrupt_ptr;
volatile uint32 responce = 1;

/*******************************************************************************
* Function Name: ipc_rx_queue_dispatcher
*******************************************************************************/
void ipc_rx_queue_dispatcher(void *arg1, void *arg2, void *arg3)
{
    while (true) {
     //   ipc_queue_drain(cons_queue, ipc_chan_handle);
    }
}


/*******************************************************************************
* Function Name: ipc_interrupt_handler
*******************************************************************************/
void ipc_interrupt_handler(void)
{
	printk("CM4: ipc_interrupt_handler INTR = 0x%x\n", (unsigned int)(ipc_interrupt_ptr->INTR));

    // Clear the interrupt and make a dummy read to avoid double interrupt occurrence:
    // - The double interrupt’s triggering is caused by buffered write operations on bus
    // - The dummy read of the status register is indeed required to make sure previous write completed before leaving ISR    
    // Note: This is a direct clear using the IPC interrupt register and not clear of an NVIC register
    Cy_IPC_Drv_ClearInterrupt(ipc_interrupt_ptr, CY_IPC_NO_NOTIFICATION, (1uL << SPM_IPC_CHANNEL));
	responce = 1;
}

void on_new_item(void *arg)
{
    CY_ASSERT(arg != NULL); // In this implementation, arg must not be NULL
    IPC_STRUCT_Type *ipc_chan_handle = (IPC_STRUCT_Type *)arg;

    Cy_IPC_Drv_AcquireNotify(ipc_chan_handle, (1uL << SPM_IPC_NOTIFY_CM0P_INTR));
}


void ipc_test()
{
	static int counter = 0;
	char *__shared_memory_start = (char *)SHARED_MEM_STARTD_ADDR;
	if(responce != 0)
	{
		sprintf(__shared_memory_start, "Shared Memory test %d", counter++);
		on_new_item(ipc_channel_handle);
		responce = 0;
	}
}


/*@external@*/ extern uint32_t interrupt_vector_table[];	/* To fix optimization */

/*******************************************************************************
* Function Name: main
*******************************************************************************/
void main(void)
{
    // Enable global interrupts
    __enable_irq();

    printf("CM4: Main\n");
    printk("IPC test app %s\n", CONFIG_ARCH);


    // Interrupts configuration for CM4
    // Configure interrupts ISR / MUX and priority
    cy_stc_sysint_t ipc_intr_Config;
    ipc_intr_Config.intrSrc = (IRQn_Type)cpuss_interrupts_ipc_0_IRQn + SPM_IPC_NOTIFY_CM4_INTR; 
    ipc_intr_Config.intrPriority = 1;
    (void)Cy_SysInt_Init(&ipc_intr_Config, ipc_interrupt_handler);
    // Set specific NOTIFY interrupt mask only.
    // Only the interrupt sources with their masks enabled can trigger the interrupt.
    ipc_interrupt_ptr = Cy_IPC_Drv_GetIntrBaseAddr(SPM_IPC_NOTIFY_CM4_INTR);
    CY_ASSERT(ipc_interrupt_ptr != NULL);
    Cy_IPC_Drv_SetInterruptMask(ipc_interrupt_ptr, 0x0, 1 << SPM_IPC_CHANNEL);
 
    // Enable the interrupt
    NVIC_EnableIRQ(ipc_intr_Config.intrSrc);
    
    ipc_channel_handle = Cy_IPC_Drv_GetIpcBaseAddress(SPM_IPC_CHANNEL);
    CY_ASSERT(ipc_channel_handle != NULL);

	/* Init interrupt in zephyr */
    IRQ_DIRECT_CONNECT((IRQn_Type)cpuss_interrupts_ipc_0_IRQn + SPM_IPC_NOTIFY_CM4_INTR, 1, ipc_interrupt_handler, 0);
    irq_enable((IRQn_Type)cpuss_interrupts_ipc_0_IRQn + SPM_IPC_NOTIFY_CM4_INTR);

    for (;;) {
    	k_sleep(1000);
		ipc_test();
    }
}
