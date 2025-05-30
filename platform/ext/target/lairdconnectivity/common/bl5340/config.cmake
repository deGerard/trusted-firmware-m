#-------------------------------------------------------------------------------
# Copyright (c) 2020, Nordic Semiconductor ASA.
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
# Copyright (c) 2021, Laird Connectivity.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(${PLATFORM_PATH}/common/core/${NRF_PLATFORM_PATH}/common/core/config.cmake)

set(SECURE_UART1                        ON                   CACHE BOOL      "Enable secure UART1")
set(SECURE_QSPI                         ON                   CACHE BOOL      "Enable secure QSPI")
set(MCUBOOT_UPGRADE_STRATEGY            "SWAP_USING_SCRATCH" CACHE STRING    "Enable using scratch flash section for swapping images")
if(NOT BL2)
    set(BL2_TRAILER_SIZE                0x800                CACHE STRING    "Trailer size")
endif()
set(NRF_NS_STORAGE                      OFF                  CACHE BOOL      "Enable non-secure storage partition")
set(NRF_NS_SECONDARY                    OFF                  CACHE BOOL      "Enable non-secure secondary partition")
set(TFM_BL2_LOG_LEVEL                   LOG_LEVEL_NONE       CACHE STRING    "Level of logging to use for MCUboot [OFF, ERROR, WARNING, INFO, VERBOSE]" FORCE)
