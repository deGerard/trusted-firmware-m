/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __NOC_S3_DISCOVERY_DRV_H__
#define __NOC_S3_DISCOVERY_DRV_H__

#include "noc_s3_drv.h"

#include <stdint.h>

/**
 * \brief NoC S3 Discovery node structure.
 */
struct noc_s3_discovery_node {
    enum noc_s3_node_type_value node_type;
    uint32_t node_id;
    uint32_t node_off_addr;
};

/**
 * \brief Does a discovery flow search and returns subfeature offset address
 *        based on the parent component ID and type.
 *
 * \param[in]   dev                  NoC S3 struct \ref noc_s3_dev.
 * \param[in]   component_node_type  Specify the parent component type of the
 *                                   subfeature enum \ref
 *                                   noc_s3_node_type_value.
 * \param[in]   component_node_id    Specify the parent component id of the
 *                                   subfeature.
 * \param[in]   subfeature_node_type Specify the type of the subfeature node
 *                                   enum \ref noc_s3_subfeature_type_value.
 * \param[out]  ret_address          Fetched offset address for the specified
 *                                   node.
 *
 * \return Returns error code as specified in enum \ref noc_s3_err.
 */
enum noc_s3_err noc_s3_fetch_subfeature_offset(
    const struct noc_s3_dev *dev,
    const enum noc_s3_node_type_value component_node_type,
    const uint32_t component_node_id,
    const enum noc_s3_subfeature_type_value subfeature_node_type,
    uint32_t *ret_off_addr);

#endif /* __NOC_S3_DISCOVERY_DRV_H__ */
