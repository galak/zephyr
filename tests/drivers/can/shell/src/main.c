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
static const struct device *const fake_can_dev = DEVICE_DT_GET(DT_NODELABEL(fake_can));
static struct can_frame frame_capture;
DEFINE_FFF_GLOBALS;

static void assert_can_frame_equal(const struct can_frame *f1, const struct can_frame *f2)
{
	zassert_equal(f1->flags, f2->flags, "flags mismatch");
	zassert_equal(f1->id, f2->id, "id mismatch");
	zassert_equal(f1->dlc, f2->dlc, "dlc mismatch");
	zassert_mem_equal(f1->data, f2->data, can_dlc_to_bytes(f1->dlc), "data mismatch");
}

static void can_shell_test_send(const char *cmd, const struct can_frame *expected)
{
	const struct shell *sh = shell_backend_dummy_get_ptr();
	int err;


	err = shell_execute_cmd(sh, cmd);
	zassert_ok(err, "failed to execute shell command (err %d)", err);
	zassert_equal(fake_can_send_fake.call_count, 1, "send function not called");
	zassert_equal(fake_can_send_fake.arg0_val, fake_can_dev, "wrong device pointer");
	assert_can_frame_equal(expected, &frame_capture);
}

ZTEST(can_shell, test_can_send_std_id)
{
	const struct can_frame expected = {
		.flags = 0,
		.id = 0x010,
		.dlc = can_bytes_to_dlc(2),
		.data = { 0xaa, 0x55 },
	};

	can_shell_test_send("can send " FAKE_CAN_NAME " 010 aa 55", &expected);
}

ZTEST(can_shell, test_can_send_fd)
{
	const struct can_frame expected = {
		.flags = CAN_FRAME_FDF,
		.id = 0x123,
		.dlc = can_bytes_to_dlc(8),
		.data = { 0xaa, 0x55, 0xaa, 0x55, 0x11, 0x22, 0x33, 0x44 },
	};

	can_shell_test_send("can send " FAKE_CAN_NAME " -f 123 aa 55 aa 55 11 22 33 44", &expected);
}

static void *can_shell_setup(void)
{
	const struct shell *sh = shell_backend_dummy_get_ptr();

	/* Wait for the initialization of the shell dummy backend. */
	WAIT_FOR(shell_ready(sh), 20000, k_msleep(1));
	zassert_true(shell_ready(sh), "timed out waiting for dummy shell backend");

	return NULL;
}

ZTEST_SUITE(can_shell, NULL, can_shell_setup, NULL, NULL, NULL);
