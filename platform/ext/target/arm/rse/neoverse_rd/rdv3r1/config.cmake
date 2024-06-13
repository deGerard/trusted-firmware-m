#-------------------------------------------------------------------------------
# SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Note: Cached varaibles always take the first value set, while normal
# variables always take the last.

set(PLAT_NEOVERSE_HOST_VERSION_VERSION  4        CACHE STRING  "Version of Neoverse-RD host mapping to use - coresponds to nrdX directories.")
set(PLAT_LCP_COUNT                      7        CACHE STRING  "Number of LCPs to load")
set(RSE_AMOUNT                          2        CACHE STRING  "Amount of RSEs in the system")
set(MULTI_RSE_TOPOLOGY_FILE             ${CMAKE_CURRENT_LIST_DIR}/bl1/rdv3r1.tgf CACHE STRING "Topology file for RSE to RSE BL1 communication")

# Once all cache options are set, set common options as fallback
include(${CMAKE_CURRENT_LIST_DIR}/../common/config.cmake)
