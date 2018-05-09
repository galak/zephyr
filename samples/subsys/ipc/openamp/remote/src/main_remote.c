/*
 * Copyright (c) 2018, NXP
 * Copyright (c) 2018, Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <ipm.h>
#include <misc/printk.h>
#include <device.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openamp/open_amp.h>
#include <metal/device.h>

#include "common.h"

#define APP_TASK_STACK_SIZE (1024)
K_THREAD_STACK_DEFINE(thread_stack, APP_TASK_STACK_SIZE);
static struct k_thread thread_data;

static struct device *ipm_handle = NULL;

static metal_phys_addr_t shm_physmap[] = { SHM_START_ADDR };
static struct metal_device shm_device = {
	.name = SHM_DEVICE_NAME,
	.bus = NULL,
	.num_regions = 1,
	{
		{
			.virt       = (void *) SHM_START_ADDR,
			.physmap    = shm_physmap,
			.size       = SHM_SIZE,
			.page_shift = 0xffffffff,
			.page_mask  = 0xffffffff,
			.mem_flags  = 0,
			.ops        = { NULL },
		},
	},
	.node = { NULL },
	.irq_num = 0,
	.irq_info = NULL
};

static volatile unsigned int received_data;

static struct virtio_vring_info rvrings[2] = {
	[0] = {
		.info.align = VRING_ALIGNMENT,
	},
	[1] = {
		.info.align = VRING_ALIGNMENT,
	},
};
static struct virtio_device vdev;
static struct rpmsg_virtio_device rvdev;
static struct metal_io_region *io;
static struct virtqueue * vq[2];

static unsigned char virtio_get_status(struct virtio_device *vdev)
{
	unsigned char *status_byte = (void *)RSC_TABLE_ADDRESS;

	return *status_byte;
}

static void virtio_set_status(struct virtio_device *vdev, unsigned char status)
{
	uint32_t dummy_data = 0x00220022; /* Some data must be provided */

	ipm_send(ipm_handle, 0, 0, &dummy_data, sizeof(dummy_data));

	return ;
}

static uint32_t virtio_get_features(struct virtio_device *vdev)
{
	return 1 << VIRTIO_RPMSG_F_NS;
}

static void virtio_set_features(struct virtio_device *vdev,
                                      uint32_t features)
{
	return ;
}

static void virtio_notify(struct virtqueue *vq)
{
	uint32_t dummy_data = 0x00110011; /* Some data must be provided */

	ipm_send(ipm_handle, 0, 0, &dummy_data, sizeof(dummy_data));
}

struct virtio_dispatch dispatch = {
	.get_status = virtio_get_status,
	.set_status = virtio_set_status,
	.get_features = virtio_get_features,
	.set_features = virtio_set_features,
	.notify = virtio_notify,
};

static K_SEM_DEFINE(data_sem, 0, 1);
static K_SEM_DEFINE(data_rx_sem, 0, 1);

static void platform_ipm_callback(void *context, u32_t id, volatile void *data)
{
	k_sem_give(&data_sem);
}

int endpoint_cb(struct rpmsg_endpoint *ept, void *data,
                             size_t len, uint32_t src, void *priv)
{
	received_data = *((unsigned int *) data);

	k_sem_give(&data_rx_sem);

	return RPMSG_SUCCESS;
}

static K_SEM_DEFINE(ept_sem, 0, 1);

struct rpmsg_endpoint my_ept;
struct rpmsg_endpoint *ep = &my_ept;

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
	(void)ept;
	rpmsg_destroy_ept(ep);
}

static unsigned int receive_message(void)
{
        while (k_sem_take(&data_rx_sem, K_NO_WAIT) != 0) {
                int status = k_sem_take(&data_sem, K_FOREVER);

                if (status == 0) {
			virtqueue_notification(vq[1]);
                }
        }
        return received_data;
}

static int send_message(unsigned int message)
{
	return rpmsg_send(ep, &message, sizeof(message));
}

void app_task(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);
	int status = 0;
	struct metal_device *device;
	struct rpmsg_device *rdev;

	printk("\r\nOpenAMP demo started\r\n");

	struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
	metal_init(&metal_params);

	status = metal_register_generic_device(&shm_device);
	if (status != 0) {
		printk("metal_register_generic_device(): could not register "
		       "shared memory device: error code %d\n", status);
	}

        status = metal_device_open("generic", SHM_DEVICE_NAME, &device);

        if (status != 0) {
                printk("metal_device_open failed %d\n", status);
        }

        io = metal_device_io_region(device, 0);

	/* setup IPM */
	ipm_handle = device_get_binding("MAILBOX_0");

	ipm_register_callback(ipm_handle, platform_ipm_callback, NULL);

	ipm_set_enabled(ipm_handle, 1);

	/* setup vdev */
	vq[0] = virtqueue_allocate(VRING_SIZE);
	vq[1] = virtqueue_allocate(VRING_SIZE);

	vdev.role = RPMSG_REMOTE;
	vdev.vrings_num = VRING_COUNT;
	vdev.func = &dispatch;
	rvrings[0].io = io;
	rvrings[0].info.vaddr = (void *)VRING_TX_ADDRESS;
	rvrings[0].info.num_descs = VRING_SIZE;
	rvrings[0].info.align = VRING_ALIGNMENT;
	rvrings[0].vq = vq[0];

	rvrings[1].io = io;
	rvrings[1].info.vaddr = (void *)VRING_RX_ADDRESS;
	rvrings[1].info.num_descs = VRING_SIZE;
	rvrings[1].info.align = VRING_ALIGNMENT;
	rvrings[1].vq = vq[1];

	vdev.vrings_info = &rvrings[0];

	/* setup rvdev */
	rpmsg_init_vdev(&rvdev, &vdev, NULL, io, NULL);
	rdev = rpmsg_virtio_get_rpmsg_device(&rvdev);

	rpmsg_create_ept(ep, rdev, "k", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY,
			 endpoint_cb, rpmsg_service_unbind);

	unsigned int message = 0;

	while (message < 100) {
		message = receive_message();
		printk("Primary core received a message: %d\n", message);

		message++;
		status = send_message(message);
		if (status < 0) {
			printk("send_message(%d) failed with status %d\n",
			       message, status);
			goto _cleanup;
		}
	}

_cleanup:
	rpmsg_deinit_vdev(&rvdev);
	metal_finish();

	printk("OpenAMP demo ended.\n");
}

void main(void)
{
	printk("Starting application thread!\n");
	k_thread_create(&thread_data, thread_stack, APP_TASK_STACK_SIZE,
			(k_thread_entry_t)app_task,
			NULL, NULL, NULL, K_PRIO_COOP(7), 0, 0);
}
