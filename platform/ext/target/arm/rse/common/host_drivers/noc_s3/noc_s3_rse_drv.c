/*
 * Copyright (c) 2023-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "noc_s3_rse_drv.h"

#include <stddef.h>

enum noc_s3_err noc_s3_program_psam_table(
    const struct noc_s3_dev *dev,
    const struct noc_s3_psam_cfgs psam_table[],
    const uint32_t psam_table_count)
{
    enum noc_s3_err err;
    struct noc_s3_psam_dev psam_dev = {0};
    uint32_t p_idx, r_idx;

    if (dev == NULL || dev->periphbase == (uintptr_t)NULL) {
        return NOC_S3_ERR_INVALID_ARG;
    }

    if (psam_table_count != 0 && psam_table == NULL) {
        return NOC_S3_ERR_INVALID_ARG;
    }

    for (p_idx = 0; p_idx < psam_table_count; ++p_idx) {
        err = noc_s3_psam_dev_init(
            dev, psam_table[p_idx].component,
            psam_table[p_idx].add_chip_addr_offset ? dev->chip_addr_offset : 0,
            &psam_dev);
        if (err != NOC_S3_SUCCESS) {
            return err;
        }

        /* Set region fields */
        for (r_idx = 0; r_idx < psam_table[p_idx].nh_region_count; ++r_idx) {
            err = noc_s3_psam_configure_next_available_nhregion(
                &psam_dev, &psam_table[p_idx].regions[r_idx]);
            if (err != NOC_S3_SUCCESS) {
                return err;
            }
        }

        /* Enable the PSAM region */
        err = noc_s3_psam_enable(&psam_dev);
        if (err != NOC_S3_SUCCESS) {
            return err;
        }
    }

    return NOC_S3_SUCCESS;
}

enum noc_s3_err noc_s3_program_apu_table(
    const struct noc_s3_dev *dev,
    const struct noc_s3_apu_cfgs apu_table[],
    const uint32_t apu_table_count)
{
    enum noc_s3_err err;
    struct noc_s3_apu_dev apu_dev = {0};
    uint32_t a_idx, r_idx;

    if (dev == NULL || dev->periphbase == (uintptr_t)NULL) {
        return NOC_S3_ERR_INVALID_ARG;
    }

    if (apu_table_count != 0 && apu_table == NULL) {
        return NOC_S3_ERR_INVALID_ARG;
    }

    for (a_idx = 0; a_idx < apu_table_count; ++a_idx) {
        err = noc_s3_apu_dev_init(
            dev, apu_table[a_idx].component,
            apu_table[a_idx].add_chip_addr_offset ? dev->chip_addr_offset : 0,
            &apu_dev);
        if (err != NOC_S3_SUCCESS) {
            return err;
        }

        /* Set region fields */
        for (r_idx = 0; r_idx < apu_table[a_idx].region_count; ++r_idx) {
            err = noc_s3_apu_configure_next_available_region(
                &apu_dev, &apu_table[a_idx].regions[r_idx]);
            if (err != NOC_S3_SUCCESS) {
                return err;
            }
        }

        err = noc_s3_apu_sync_err_enable(&apu_dev);
        if (err != NOC_S3_SUCCESS) {
            return err;
        }

        err = noc_s3_apu_enable(&apu_dev);
        if (err != NOC_S3_SUCCESS) {
            return err;
        }
    }

    return NOC_S3_SUCCESS;
}
