/*
 * Copyright (c) 2021-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __MMIO_DEFS_H__
#define __MMIO_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "tfm_peripherals_def.h"

/* Boundary handle binding macros. */
#define HANDLE_ATTR_PRIV_POS            1U
#define HANDLE_ATTR_PRIV_MASK           (0x1UL << HANDLE_ATTR_PRIV_POS)
#define HANDLE_ATTR_NS_POS              0U
#define HANDLE_ATTR_NS_MASK             (0x1UL << HANDLE_ATTR_NS_POS)
#if TFM_ISOLATION_LEVEL == 3
#define HANDLE_PER_ATTR_BITS            (0x4)
#define HANDLE_ATTR_RW_POS              (1 << (HANDLE_PER_ATTR_BITS - 1))
#define HANDLE_ATTR_INDEX_MASK          (HANDLE_ATTR_RW_POS - 1)
#define HANDLE_INDEX_BITS               (0x8)
#define HANDLE_INDEX_MASK               (((1 << HANDLE_INDEX_BITS) -1) << 24)
#define HANDLE_ENCODE_INDEX(attr, idx)                              \
    do {                                                            \
        (attr) |= (((idx) << 24) & HANDLE_INDEX_MASK);              \
        (idx)++;                                                    \
    } while (0)
#endif

/* Allowed named MMIO of this platform, maximum number is 5. */
const uintptr_t partition_named_mmio_list[] = {
//    (uintptr_t)TFM_PERIPHERAL_FPGA_IO,
//    (uintptr_t)TFM_PERIPHERAL_TIMER0,
    (uintptr_t)TFM_PERIPHERAL_STD_UART,
};

#ifdef __cplusplus
}
#endif

#endif /* __MMIO_DEFS_H__ */
