/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __RSE_OTP_CONFIG_H__
#define __RSE_OTP_CONFIG_H__

#include "rse_nv_counter_config.h"

#include <stdint.h>

#define RSE_OTP_SIZE 16384

/* The following options control the ROM view of the OTP. They must remain
 * constant for a given ROM
 */
#define RSE_OTP_HAS_CM_AREA
#define RSE_OTP_HAS_BL1_2
#define RSE_OTP_HAS_DM_AREA
#define RSE_OTP_HAS_DYNAMIC_AREA
/* Enable the below definition if specific RSE platform OTP has SoC area */
/* #define RSE_OTP_HAS_SOC_AREA */
#define RSE_OTP_HAS_KRTL_USAGE_COUNTER
#define RSE_OTP_HAS_LFT_COUNTER

/* #define RSE_OTP_DM_SETS_DM_AND_DYNAMIC_AREA_SIZE */

#define RSE_OTP_KRTL_COUNTER_MAX_VALUE 16
#define RSE_OTP_LFT_COUNTER_MAX_VALUE 512

#define RSE_OTP_DMA_ICS_SIZE 0x108

/* The following options control the CM area, and can be changed without
 * altering the ROM.
 */
#define RSE_OTP_HAS_CCA_ITEMS
/* #define RSE_OTP_HAS_RSE_ID */
/* #define RSE_OTP_HAS_ROUTING_TABLES */

#define RSE_OTP_CM_ROTPK_SIZE 32
#define RSE_OTP_CM_ROTPK_AMOUNT 4
#define RSE_OTP_CM_ROTPK_IS_HASH_NOT_KEY
#define RSE_OTP_CM_ROTPK_MAX_REVOCATIONS 0
#define RSE_ROUTING_TABLES_SIZE 0
#define RSE_OTP_CM_RESERVED_SIZE 4

/* The following options control the BL1_2 area, and can be changed without
 * altering the ROM.
 */

#define RSE_OTP_BL1_2_MAX_SIZE 8192
#define RSE_OTP_BL1_2_HASH_SIZE 48

/* The following options control the DM area, and can be changed without
 * altering the ROM.
 */
#define RSE_OTP_DM_ROTPK_SIZE 32
#define RSE_OTP_DM_ROTPK_AMOUNT 3
#define RSE_OTP_DM_ROTPK_IS_HASH_NOT_KEY
#define RSE_OTP_DM_ROTPK_MAX_REVOCATIONS 0
#define RSE_OTP_DM_RESERVED_SIZE 0

/* The following options control the Dynamic area, and can be changed without
 * altering the ROM.
 */

#define RSE_OTP_MAX_REPROVISIONINGS 8
/* #define RSE_OTP_HAS_IAK_ENDORSEMENT_CERTIFICATE */
/* #define RSE_OTP_ENDORSEMENT_CERTIFICATE_SIZE 0x60 */
/* #define RSE_OTP_ENDORSEMENT_CERTIFICATE_METADATA_SIZE 0x30 */

/* 4 banks of NV counters are provided. Each bank must be a fixed size, so there
 * can be up to 4 different sizes of NV counters in the unlocked area, but with
 * unlimited amounts of each size of counter.
 */
#define RSE_OTP_NV_COUNTERS_BANK_0_MAX_VALUE 256
#define RSE_OTP_NV_COUNTERS_BANK_0_AMOUNT    RSE_NV_COUNTER_AMOUNT

#define RSE_OTP_NV_COUNTERS_BANK_1_MAX_VALUE 0
#define RSE_OTP_NV_COUNTERS_BANK_1_AMOUNT    0

#define RSE_OTP_NV_COUNTERS_BANK_2_MAX_VALUE 0
#define RSE_OTP_NV_COUNTERS_BANK_2_AMOUNT    0

#define RSE_OTP_NV_COUNTERS_BANK_3_MAX_VALUE 0
#define RSE_OTP_NV_COUNTERS_BANK_3_AMOUNT    0

/* The following options control the SoC area, and cannot be changed without
 * altering the ROM.
 */

#define RSE_OTP_SOC_RESERVED_SIZE 0x12D4

/* #define RSE_OTP_HEADER_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_header_area_t { */
/* }; */

/* #define RSE_OTP_CM_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_cm_area_t { */
/* }; */

/* #define RSE_OTP_BL1_2_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_bl1_2_area_t { */
/* }; */

/* #define RSE_OTP_DM_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_dm_area_t { */
/* }; */

/* #define RSE_OTP_DYNAMIC_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_dynamic_area_t { */
/* }; */

/* #define RSE_OTP_SOC_SUBPLATFORM_ITEMS */
/* struct __attribute__((packed)) rse_otp_subplatform_soc_area_t { */
/* }; */

#include "rse_otp_check_config.h"

#endif /* __RSE_OTP_CONFIG_H__ */
