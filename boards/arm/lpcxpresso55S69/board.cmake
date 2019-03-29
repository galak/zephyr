#
# Copyright (c) 2017, NXP
#
# SPDX-License-Identifier: Apache-2.0
#

set_ifndef(LPCLINK_FW jlink)

if(LPCLINK_FW STREQUAL jlink)
  set_ifndef(BOARD_DEBUG_RUNNER jlink)
  set_ifndef(BOARD_FLASH_RUNNER jlink)
endif()

if(CONFIG_BOARD_LPCXPRESSO55S69_M33_CPU0)
#board_runner_args(jlink "--device=LPC54114J256_M4")
elseif(CONFIG_LPCXPRESSO55S69_M33_CPU1)
#board_runner_args(jlink "--device=LPC54114J256_M0")
endif()

include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
