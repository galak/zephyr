# Copyright (c) 2018 Cypress Semiconductor
#
# SPDX-License-Identifier: Apache-2.0
#

set(BOARD cy8ckit_062_wifi_bt_m4)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

enable_language(C ASM)

# Location of external dependencies:
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
set(AUTH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../auth_mgr")

target_sources(app PRIVATE src/main.c
)

target_include_directories(app PRIVATE 
               ${AUTH_DIR}/
			   ./src)
