/***************************************************************************//**
* \file cycfg_init.c
* \version 1.0
*
* \brief
* Performs a basic PSoC 6 device initialization.
*
********************************************************************************
* \copyright
* Copyright 2017-2018, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdbool.h>
#include "cy_syslib.h"
#include "cy_syspm.h"
#include "cycfg_platform.h"
#include "cycfg_clocks.h"

/*******************************************************************************
* Function Name: Cy_SystemInit
****************************************************************************//**
*
* \brief This function is called by the start-up code for the selected device.
* It performs all of the necessary device configuration based on the design
* settings. This includes settings for the platform resources and peripheral
* clock.
*
*******************************************************************************/

void Cy_SystemInit(void)
{
    /* Configure platform resources */
    init_cycfg_platform();

    /* Configure peripheral clocks */
    init_cycfg_clocks();
}
