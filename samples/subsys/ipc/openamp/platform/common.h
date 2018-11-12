/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef COMMON_H__
#define COMMON_H__

#define SHM_START_ADDR		0x04000400
#define SHM_SIZE		0x7c00
#define SHM_DEVICE_NAME		"sramx.shm"

#define VRING_COUNT		2
#define VRING_RX_ADDRESS	0x04007800
#define VRING_TX_ADDRESS	0x04007C00
#define VRING_ALIGNMENT		4
#define VRING_SIZE		16

#define RSC_TABLE_ADDRESS	0x04000000

static uint32_t virtio_get_features(struct virtio_device *vdev)
{
	return 1 << VIRTIO_RPMSG_F_NS;
}

static void virtio_set_features(struct virtio_device *vdev,
                                      uint32_t features)
{
	return ;
}

#endif
