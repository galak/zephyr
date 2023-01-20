/*
 * Copyright (c) 2022 Vestas Wind Systems A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/drivers/can.h>
#include <zephyr/drivers/can/can_fake.h>
#include <zephyr/fff.h>
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_dummy.h>
#include <zephyr/ztest.h>

/**
 * @addtogroup t_can_driver
 * @{
 * @defgroup t_can_api test_can_shell
 * @}
 */

#define FAKE_CAN_NAME DEVICE_DT_NAME(DT_NODELABEL(fake_can))

/* Global variables */
static struct can_timing timing_capture;
static struct can_filter filter_capture;
static struct can_frame frame_capture;
DEFINE_FFF_GLOBALS;

static void assert_can_frame_equal(const struct can_frame *f1, const struct can_frame *f2)
{
	zassert_equal(f1->flags, f2->flags, "flags mismatch");
	zassert_equal(f1->id, f2->id, "id mismatch");
	zassert_equal(f1->dlc, f2->dlc, "dlc mismatch");
	zassert_mem_equal(f1->data, f2->data, can_dlc_to_bytes(f1->dlc), "data mismatch");
}

static int can_shell_test_capture_frame(const struct device *dev, const struct can_frame *frame,
					k_timeout_t timeout, can_tx_callback_t callback,
					void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(timeout);
	ARG_UNUSED(callback);
	ARG_UNUSED(user_data);

	memcpy(&frame_capture, frame, sizeof(frame_capture));

	return 0;
}

void can_shell_test_send(const char *cmd, const struct can_frame *expected)
{
	const struct shell *sh = shell_backend_dummy_get_ptr();
	int err;

	fake_can_send_fake.custom_fake = can_shell_test_capture_frame;

	err = shell_execute_cmd(sh, cmd);
	assert_can_frame_equal(expected, &frame_capture);
}

ZTEST(can_shell, test_can_send_data_all_options)
{
	const struct can_frame expected = {
		.flags = CAN_FRAME_IDE | CAN_FRAME_FDF | CAN_FRAME_BRS | CAN_FRAME_RTR,
		.id = 0x1024,
		.dlc = can_bytes_to_dlc(0),
		.data = { },
	};

	can_shell_test_send("can send " FAKE_CAN_NAME " -r -e -f -b 1024", &expected);
}

static void can_shell_before(void *fixture)
{
	ARG_UNUSED(fixture);

	memset(&timing_capture, 0, sizeof(timing_capture));
	memset(&filter_capture, 0, sizeof(filter_capture));
	memset(&frame_capture, 0, sizeof(frame_capture));
}

static void *can_shell_setup(void)
{
	const struct shell *sh = shell_backend_dummy_get_ptr();

	/* Wait for the initialization of the shell dummy backend. */
	WAIT_FOR(shell_ready(sh), 20000, k_msleep(1));
	zassert_true(shell_ready(sh), "timed out waiting for dummy shell backend");

	return NULL;
}

ZTEST_SUITE(can_shell, NULL, can_shell_setup, can_shell_before, NULL, NULL);
