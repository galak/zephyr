/* ST Microelectronics IIS2DH 3-axis accelerometer driver
 *
 * Copyright (c) 2020 STMicroelectronics
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Datasheet:
 * https://www.st.com/resource/en/datasheet/iis2dh.pdf
 */

#define DT_DRV_COMPAT st_iis2dh

#include <string.h>
#include "iis2dh.h"
#include <zephyr/logging/log.h>

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)

#define IIS2DH_SPI_READM	(3 << 6)	/* 0xC0 */
#define IIS2DH_SPI_WRITEM	(1 << 6)	/* 0x40 */

LOG_MODULE_DECLARE(IIS2DH, CONFIG_SENSOR_LOG_LEVEL);

static struct spi_config iis2dh_spi_conf = SPI_CONFIG_DT_INST(0,
	(SPI_OP_MODE_MASTER | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8)),
	0);

static int iis2dh_spi_read(struct iis2dh_data *ctx, uint8_t reg,
			   uint8_t *data, uint16_t len)
{
	struct spi_config *spi_cfg = &iis2dh_spi_conf;
	uint8_t buffer_tx[2] = { reg | IIS2DH_SPI_READM, 0 };
	const struct spi_buf tx_buf = {
			.buf = buffer_tx,
			.len = 2,
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};
	const struct spi_buf rx_buf[2] = {
		{
			.buf = NULL,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set rx = {
		.buffers = rx_buf,
		.count = 2
	};

	if (spi_transceive(ctx->bus, spi_cfg, &tx, &rx)) {
		return -EIO;
	}

	return 0;
}

static int iis2dh_spi_write(struct iis2dh_data *ctx, uint8_t reg,
			    uint8_t *data, uint16_t len)
{
	struct spi_config *spi_cfg = &iis2dh_spi_conf;
	uint8_t buffer_tx[1] = { reg | IIS2DH_SPI_WRITEM };
	const struct spi_buf tx_buf[2] = {
		{
			.buf = buffer_tx,
			.len = 1,
		},
		{
			.buf = data,
			.len = len,
		}
	};
	const struct spi_buf_set tx = {
		.buffers = tx_buf,
		.count = 2
	};


	if (spi_write(ctx->bus, spi_cfg, &tx)) {
		return -EIO;
	}

	return 0;
}

stmdev_ctx_t iis2dh_spi_ctx = {
	.read_reg = (stmdev_read_ptr) iis2dh_spi_read,
	.write_reg = (stmdev_write_ptr) iis2dh_spi_write,
};

int iis2dh_spi_init(const struct device *dev)
{
	struct iis2dh_data *data = dev->data;

	data->ctx = &iis2dh_spi_ctx;
	data->ctx->handle = data;

#if DT_INST_SPI_DEV_HAS_CS_GPIOS(0)
	struct spi_config *spi_cfg = &iis2dh_spi_conf;
	const struct spi_cs_control *cs_ctrl = spi_cfg->cs;

	if (!device_is_ready(cs_ctrl->gpio.port)) {
		LOG_ERR("Unable to get GPIO SPI CS device");
		return -ENODEV;
	}
#endif

	return 0;
}
#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */
