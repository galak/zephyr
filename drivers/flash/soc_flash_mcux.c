/*
 * Copyright (c) 2016 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <device.h>
#include <string.h>
#include <drivers/flash.h>
#include <errno.h>
#include <init.h>
#include <soc.h>
#include "flash_priv.h"

#include "fsl_common.h"
#ifdef CONFIG_HAS_MCUX_IAP
#include "fsl_iap.h"
#else
#include "fsl_flash.h"
#endif

#if DT_HAS_NODE(DT_INST(0, nxp_kinetis_ftfa))
#define DT_DRV_COMPAT nxp_kinetis_ftfa
#elif DT_HAS_NODE(DT_INST(0, nxp_kinetis_ftfe))
#define DT_DRV_COMPAT nxp_kinetis_ftfe
#elif DT_HAS_NODE(DT_INST(0, nxp_kinetis_ftfl))
#define DT_DRV_COMPAT nxp_kinetis_ftfl
#elif DT_HAS_NODE(DT_INST(0, nxp_lpc_iap))
#define DT_DRV_COMPAT nxp_lpc_iap
#else
#error No matching compatible for soc_flash_mcux.c
#endif

#define SOC_NV_FLASH_NODE DT_INST(0, soc_nv_flash)

struct flash_priv {
	flash_config_t config;
	/*
	 * HACK: flash write protection is managed in software.
	 */
	struct k_sem write_lock;
	u32_t pflash_block_base;
};

#define ALIGN_DOWN(x, a)    ((x) & -(a))

#define FLASH_CMD_INIT             0
#define FLASH_CMD_POWERDOWN        1
#define FLASH_CMD_SET_READ_MODE    2
#define FLASH_CMD_READ_SINGLE_WORD 3
#define FLASH_CMD_ERASE_RANGE      4
#define FLASH_CMD_BLANK_CHECK      5
#define FLASH_CMD_MARGIN_CHECK     6
#define FLASH_CMD_CHECKSUM         7
#define FLASH_CMD_WRITE            8
#define FLASH_CMD_WRITE_PROG       10
#define FLASH_CMD_PROGRAM          12
#define FLASH_CMD_REPORT_ECC       13
/* Below CMDs apply to C040HDATFC flash only */
#define FLASH_CMD_SET_WRITE_MODE               14
#define FLASH_CMD_PATTERN_CHECK                16  /* Test mode only */
#define FLASH_CMD_MASS_ERASE_PROG              17  /* Test mode only */
#define FLASH_CMD_MASS_WRITE                   18  /* Test mode only */
#define FLASH_CMD_READ_TRIM_DATA               19  /* Test mode only */
#define FLASH_CMD_PAGEREGISTER_UPLOAD          20  /* Test mode only */
#define FLASH_CMD_SET_INDIVIDUAL_TRIM_REGISTER 21  /* Test mode only */
#define FLASH_CMD_TRIM_READ_PHASE              22  /* Test mode only */

#define FLASH_READPARAM_REG                      (FLASH->DATAW[0])
#define FLASH_READPARAM_WAIT_STATE_MASK          (0xFU)
#define FLASH_READPARAM_WAIT_STATE_SHIFT         (0U)
#define FLASH_READPARAM_WAIT_STATE(x)            (((uint32_t)(((uint32_t)(x)) << FLASH_READPARAM_WAIT_STATE_SHIFT)) & FLASH_READPARAM_WAIT_STATE_MASK)
#define FLASH_READPARAM_INTERFACE_TRIM_MASK      (0xFFF0U)
#define FLASH_READPARAM_INTERFACE_TRIM_SHIFT     (4U)
#define FLASH_READPARAM_INTERFACE_TRIM(x)        (((uint32_t)(((uint32_t)(x)) << FLASH_READPARAM_INTERFACE_TRIM_SHIFT)) & FLASH_READPARAM_INTERFACE_TRIM_MASK)
#define FLASH_READPARAM_CONTROLLER_TRIM_MASK     (0xFFF0000U)
#define FLASH_READPARAM_CONTROLLER_TRIM_SHIFT    (16U)
#define FLASH_READPARAM_CONTROLLER_TRIM(x)       (((uint32_t)(((uint32_t)(x)) << FLASH_READPARAM_CONTROLLER_TRIM_SHIFT)) & FLASH_READPARAM_CONTROLLER_TRIM_MASK)

#define FLASH_WRITEPARAM_REG                     (FLASH->DATAW[0])
#define FLASH_WRITEPARAM_ERASE_RAMP_CTRL_MASK    (0x3U)
#define FLASH_WRITEPARAM_ERASE_RAMP_CTRL_SHIFT   (0U)
#define FLASH_WRITEPARAM_ERASE_RAMP_CTRL(x)      (((uint32_t)(((uint32_t)(x)) << FLASH_WRITEPARAM_ERASE_RAMP_CTRL_SHIFT)) & FLASH_WRITEPARAM_ERASE_RAMP_CTRL_MASK)
#define FLASH_WRITEPARAM_PROGRAM_RAMP_CTRL_MASK  (0xCU)
#define FLASH_WRITEPARAM_PROGRAM_RAMP_CTRL_SHIFT (2U)
#define FLASH_WRITEPARAM_PROGRAM_RAMP_CTRL(x)    (((uint32_t)(((uint32_t)(x)) << FLASH_WRITEPARAM_PROGRAM_RAMP_CTRL_SHIFT)) & FLASH_WRITEPARAM_PROGRAM_RAMP_CTRL_MASK)

#define FLASH_READMODE_REG                 (FLASH->DATAW[0])
#define FLASH_READMODE_ECC_MASK            (0x4U)
#define FLASH_READMODE_ECC_SHIFT           (2U)
#define FLASH_READMODE_ECC(x)              (((uint32_t)(((uint32_t)(x)) << FLASH_READMODE_ECC_SHIFT)) & FLASH_READMODE_ECC_MASK)
#define FLASH_READMODE_MARGIN_MASK         (0xC00U)
#define FLASH_READMODE_MARGIN_SHIFT        (10U)
#define FLASH_READMODE_MARGIN(x)           (((uint32_t)(((uint32_t)(x)) << FLASH_READMODE_MARGIN_SHIFT)) & FLASH_READMODE_MARGIN_MASK)
#define FLASH_READMODE_DMACC_MASK          (0x8000U)
#define FLASH_READMODE_DMACC_SHIFT         (15U)
#define FLASH_READMODE_DMACC(x)            (((uint32_t)(((uint32_t)(x)) << FLASH_READMODE_DMACC_SHIFT)) & FLASH_READMODE_DMACC_MASK)

#define FLASH_DATAW_IDX_MAX  3

static status_t flash_command_sequence(flash_config_t *config)
{
    status_t status = kStatus_Fail;
    uint32_t registerValue;

    while (!(FLASH->INT_STATUS & FLASH_INT_STATUS_DONE_MASK));

    /* Check error bits */
    /* Get flash status register value */
    registerValue = FLASH->INT_STATUS;

    /* Checking access error */
    if(registerValue & FLASH_INT_STATUS_FAIL_MASK)
    {
        status = kStatus_FLASH_CommandFailure;
    }
    else if(registerValue & FLASH_INT_STATUS_ERR_MASK)
    {
        status = kStatus_FLASH_CommandNotSupported;
    }
    else if(registerValue & FLASH_INT_STATUS_ECC_ERR_MASK)
    {
        status = kStatus_FLASH_EccError;
    }
    else
    {
        status = kStatus_FLASH_Success;
    }

    return status;
}

/*! @brief Validates the range and alignment of the given address range.*/
static status_t flash_check_range(flash_config_t *config,
                                  uint32_t startAddress,
                                  uint32_t lengthInBytes,
                                  uint32_t alignmentBaseline)
{
    if (config == NULL)
    {
        return kStatus_FLASH_InvalidArgument;
    }

    /* Verify the start and length are alignmentBaseline aligned. */
    if ((startAddress & (alignmentBaseline - 1)) ||
        (lengthInBytes & (alignmentBaseline - 1)))
    {
        return kStatus_FLASH_AlignmentError;
    }

    /* check for valid range of the target addresses */
    if (((startAddress >= config->PFlashBlockBase) &&
         ((startAddress + lengthInBytes) <= (config->PFlashBlockBase + config->PFlashTotalSize))))
    {
        return kStatus_FLASH_Success;
    }
    
    if (((startAddress >= config->ffrConfig.ffrBlockBase) &&
         ((startAddress + lengthInBytes) <= (config->ffrConfig.ffrBlockBase + config->ffrConfig.ffrTotalSize))))
    {
        return kStatus_FLASH_Success;
    }

    return kStatus_FLASH_AddressError;
}

status_t FLASH_ReadSingleWord(flash_config_t *config,
                              uint32_t start,
                              uint32_t *readbackData)
{
    status_t status = kStatus_Fail;
    uint32_t byteSizes = sizeof(uint32_t) * (FLASH_DATAW_IDX_MAX + 1);

    if (readbackData == NULL)
    {
        return kStatus_FLASH_InvalidArgument;
    }

    status = flash_check_range(config, start, byteSizes, kFLASH_AlignementUnitSingleWordRead);
    if (kStatus_FLASH_Success != status)
    {
        return status;
    }

    FLASH->INT_CLR_STATUS = FLASH_INT_CLR_STATUS_FAIL_MASK | FLASH_INT_CLR_STATUS_ERR_MASK |
                            FLASH_INT_CLR_STATUS_DONE_MASK | FLASH_INT_CLR_STATUS_ECC_ERR_MASK;

    /* Set start address */
    FLASH->STARTA = start >> 4;

    /* ReadSingleWord notes:
        Flash contains one DMACC word per page. Such words are not readable through
          the read interface. DMACC words are managed internally by the controller in
          order to store a flag (all1), which can be used to verify whether a programming
          operation was prematurely terminated.
        DMACC words are all_0 for an erased page, all_1 for a programmed page
    */

    /* Set read modes */
    FLASH_READMODE_REG = FLASH_READMODE_ECC(config->modeConfig.readSingleWord.readWithEccOff) |
                         FLASH_READMODE_MARGIN(config->modeConfig.readSingleWord.readMarginLevel) |
                         FLASH_READMODE_DMACC(config->modeConfig.readSingleWord.readDmaccWord);

     /* Calling flash command sequence function to execute the command */
    FLASH->CMD = FLASH_CMD_READ_SINGLE_WORD;
    status = flash_command_sequence(config);

    if (kStatus_FLASH_Success == status)
    {
        for (uint32_t datawIndex = 0; datawIndex <= FLASH_DATAW_IDX_MAX; datawIndex++)
        {
            *readbackData++ = FLASH->DATAW[datawIndex];
        }
    }

    return status;
}

status_t FLASH_Read(flash_config_t *config, uint32_t start, uint8_t *dest, uint32_t lengthInBytes)
{
    status_t status = kStatus_Fail;

    status = flash_check_range(config, start, lengthInBytes, 1);
    if (kStatus_FLASH_Success != status)
    {
        return status;
    }

    uint32_t readbackData[FLASH_DATAW_IDX_MAX + 1];
    while (lengthInBytes)
    {
        uint32_t alignedStart = ALIGN_DOWN(start, kFLASH_AlignementUnitSingleWordRead);
        status = FLASH_ReadSingleWord(config, alignedStart, readbackData);
        if (status != kStatus_FLASH_Success)
        {
            break;
        }
        for (uint32_t i = 0; i < sizeof(readbackData); i++)
        {
            if ((alignedStart == start) && lengthInBytes)
            {
                *dest = *((uint8_t *)readbackData + i);
                dest++;
                start++;
                lengthInBytes--;
            }
            alignedStart++;
        }
    }

    return status;
}

status_t FLASH_SetReadModes(flash_config_t *config)
{
    if (config == NULL)
    {
        return kStatus_FLASH_InvalidArgument;
    }

    /* Set read parameters*/
    FLASH_READPARAM_REG = (FLASH_READPARAM_REG & (~FLASH_READPARAM_WAIT_STATE_MASK)) | FLASH_READPARAM_WAIT_STATE(config->modeConfig.setReadMode.readWaitStates);

    /* This starts the Set read mode command, no need to wait until command is
        completed: further accesses are stalled until the command is completed. */
    FLASH->CMD = FLASH_CMD_SET_READ_MODE;

    return kStatus_FLASH_Success;
}

/*
 * Interrupt vectors could be executed from flash hence the need for locking.
 * The underlying MCUX driver takes care of copying the functions to SRAM.
 *
 * For more information, see the application note below on Read-While-Write
 * http://cache.freescale.com/files/32bit/doc/app_note/AN4695.pdf
 *
 */

static int flash_mcux_erase(struct device *dev, off_t offset, size_t len)
{
	struct flash_priv *priv = dev->driver_data;
	u32_t addr;
	status_t rc;
	unsigned int key;

	if (k_sem_take(&priv->write_lock, K_NO_WAIT)) {
		return -EACCES;
	}

	addr = offset + priv->pflash_block_base;

	key = irq_lock();
	rc = FLASH_Erase(&priv->config, addr, len, kFLASH_ApiEraseKey);
	irq_unlock(key);

	k_sem_give(&priv->write_lock);

	return (rc == kStatus_Success) ? 0 : -EINVAL;
}

static int flash_mcux_read(struct device *dev, off_t offset,
				void *data, size_t len)
{
	struct flash_priv *priv = dev->driver_data;
	u32_t addr;

	/*
	 * The MCUX supports different flash chips whose valid ranges are
	 * hidden below the API: until the API export these ranges, we can not
	 * do any generic validation
	 */
	addr = offset + priv->pflash_block_base;

	memcpy(data, (void *) addr, len);

	FLASH_SetReadModes(&priv->config);
	FLASH_Read(&priv->config, addr, data, len);

	return 0;
}

static int flash_mcux_write(struct device *dev, off_t offset,
				const void *data, size_t len)
{
	struct flash_priv *priv = dev->driver_data;
	u32_t addr;
	status_t rc;
	unsigned int key;

	if (k_sem_take(&priv->write_lock, K_NO_WAIT)) {
		return -EACCES;
	}

	addr = offset + priv->pflash_block_base;

	key = irq_lock();
	rc = FLASH_Program(&priv->config, addr, (uint8_t *) data, len);
	irq_unlock(key);

	k_sem_give(&priv->write_lock);

	return (rc == kStatus_Success) ? 0 : -EINVAL;
}

static int flash_mcux_write_protection(struct device *dev, bool enable)
{
	struct flash_priv *priv = dev->driver_data;
	int rc = 0;

	if (enable) {
		rc = k_sem_take(&priv->write_lock, K_FOREVER);
	} else {
		k_sem_give(&priv->write_lock);
	}

	return rc;
}

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
static const struct flash_pages_layout dev_layout = {
	.pages_count = DT_REG_SIZE(SOC_NV_FLASH_NODE) /
				DT_PROP(SOC_NV_FLASH_NODE, erase_block_size),
	.pages_size = DT_PROP(SOC_NV_FLASH_NODE, erase_block_size),
};

static void flash_mcux_pages_layout(struct device *dev,
									const struct flash_pages_layout **layout,
									size_t *layout_size)
{
	*layout = &dev_layout;
	*layout_size = 1;
}
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

static struct flash_priv flash_data;

static const struct flash_driver_api flash_mcux_api = {
	.write_protection = flash_mcux_write_protection,
	.erase = flash_mcux_erase,
	.write = flash_mcux_write,
	.read = flash_mcux_read,
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	.page_layout = flash_mcux_pages_layout,
#endif
#if DT_NODE_HAS_PROP(SOC_NV_FLASH_NODE, write_block_size)
	.write_block_size = DT_PROP(SOC_NV_FLASH_NODE, write_block_size),
#else
	.write_block_size = FSL_FEATURE_FLASH_PFLASH_BLOCK_WRITE_UNIT_SIZE,
#endif
};

static int flash_mcux_init(struct device *dev)
{
	struct flash_priv *priv = dev->driver_data;
	uint32_t pflash_block_base;
	status_t rc;

	k_sem_init(&priv->write_lock, 0, 1);

	rc = FLASH_Init(&priv->config);

#ifdef CONFIG_HAS_MCUX_IAP
	FLASH_GetProperty(&priv->config, kFLASH_PropertyPflashBlockBaseAddr,
			  &pflash_block_base);
#else
	FLASH_GetProperty(&priv->config, kFLASH_PropertyPflash0BlockBaseAddr,
			  &pflash_block_base);
#endif
	priv->pflash_block_base = (u32_t) pflash_block_base;

	return (rc == kStatus_Success) ? 0 : -EIO;
}

DEVICE_AND_API_INIT(flash_mcux, DT_INST_LABEL(0),
			flash_mcux_init, &flash_data, NULL, POST_KERNEL,
			CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &flash_mcux_api);
