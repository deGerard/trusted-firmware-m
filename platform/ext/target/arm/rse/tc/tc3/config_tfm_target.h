/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __CONFIG_TFM_TARGET_H__
#define __CONFIG_TFM_TARGET_H__

#include "config_tfm_target_common.h"

/* SCMI Comms SCP doorbell IRQ number and handler */
#define SCP_DOORBELL_IRQ         CMU_MHU2_Receiver_IRQn
#define SCP_DOORBELL_IRQ_HANDLER CMU_MHU2_Receiver_Handler

#endif /* __CONFIG_TFM_TARGET_H__ */
