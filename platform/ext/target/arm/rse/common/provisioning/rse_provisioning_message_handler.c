/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include "rse_provisioning_message_handler.h"

#include "rse_provisioning_message.h"
#include "rse_provisioning_config.h"
#include "cc3xx_drv.h"
#include "device_definition.h"
#include "tfm_log.h"
#include "region_defs.h"
#include "fatal_error.h"
#include "tfm_hal_platform.h"
#include "crypto.h"
#include "tfm_plat_otp.h"
#include "rse_otp_dev.h"

#include <string.h>

static bool rse_debug_is_disabled(void)
{
    enum lcm_error_t lcm_err;
    uint32_t dcu_values[LCM_DCU_WIDTH_IN_BYTES / sizeof(uint32_t)];

    lcm_err = lcm_dcu_get_enabled(&LCM_DEV_S, (uint8_t *)dcu_values);
    if (lcm_err != LCM_ERROR_NONE) {
        FATAL_ERR(false);
        /* Assume the least secure case if there is an error */
        return false;
    }

    /* FIXME remove this once the FVP is fixed */
    if (dcu_values[0] == 0) {
        return false;
    }

    return (dcu_values[0] & 0x55555555) == 0;
}

static bool rse_disable_debug_and_reset(void)
{
    /* FixMe: Use the reset debug disable mechanism */
    return false;
}

static enum tfm_plat_err_t is_blob_valid_for_lcs(const struct rse_provisioning_message_blob_t *blob,
                                                 enum lcm_lcs_t lcs)
{
    uint32_t purpose_mask = 0;

    switch(lcs) {
    case LCM_LCS_CM:
       purpose_mask = RSE_PROVISIONING_BLOB_VALID_IN_CM_LCS
                      << RSE_PROVISIONING_BLOB_PURPOSE_LCS_MASK_OFFSET;
       break;
    case LCM_LCS_DM:
       purpose_mask = RSE_PROVISIONING_BLOB_VALID_IN_DM_LCS
                      << RSE_PROVISIONING_BLOB_PURPOSE_LCS_MASK_OFFSET;
       break;
    case LCM_LCS_SE:
       purpose_mask = RSE_PROVISIONING_BLOB_VALID_IN_SE_LCS
                      << RSE_PROVISIONING_BLOB_PURPOSE_LCS_MASK_OFFSET;
       break;
    case LCM_LCS_RMA:
       purpose_mask = RSE_PROVISIONING_BLOB_VALID_IN_RMA_LCS
                      << RSE_PROVISIONING_BLOB_PURPOSE_LCS_MASK_OFFSET;
       break;
    default:
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_LCS);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_LCS;
    }

    if (blob->purpose & purpose_mask) {
        return TFM_PLAT_ERR_SUCCESS;
    }

    FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_PURPOSE);
    return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_PURPOSE;
}

static enum tfm_plat_err_t get_tp_mode_for_blob(const struct rse_provisioning_message_blob_t *blob,
                                                enum lcm_tp_mode_t *tp_mode)
{
    enum rse_provisioning_blob_required_tp_mode_config_t tp_mode_config;

    tp_mode_config = (blob->purpose >> RSE_PROVISIONING_BLOB_PURPOSE_TP_MODE_OFFSET)
                     & RSE_PROVISIONING_BLOB_PURPOSE_TP_MODE_MASK;
    switch(tp_mode_config) {
    case RSE_PROVISIONING_BLOB_REQUIRES_TP_MODE_VIRGIN:
        *tp_mode = LCM_TP_MODE_VIRGIN;
        return TFM_PLAT_ERR_SUCCESS;
    case RSE_PROVISIONING_BLOB_REQUIRES_TP_MODE_PCI:
        *tp_mode = LCM_TP_MODE_PCI;
        return TFM_PLAT_ERR_SUCCESS;
    case RSE_PROVISIONING_BLOB_REQUIRES_TP_MODE_TCI:
        *tp_mode = LCM_TP_MODE_TCI;
        return TFM_PLAT_ERR_SUCCESS;
    default:
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_TP_MODE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_TP_MODE;
    }
}

static bool is_sp_mode_required_for_blob(const struct rse_provisioning_message_blob_t *blob)
{
    return (((blob->purpose >> RSE_PROVISIONING_BLOB_PURPOSE_SP_MODE_OFFSET)
            & RSE_PROVISIONING_BLOB_PURPOSE_SP_MODE_MASK)
            == RSE_PROVISIONING_BLOB_REQUIRES_SP_MODE_ENABLED);
}

static bool is_blob_chainloaded(const struct rse_provisioning_message_blob_t *blob)
{
    return (((blob->purpose >> RSE_PROVISIONING_BLOB_PURPOSE_SEQUENCING_OFFSET)
            & RSE_PROVISIONING_BLOB_PURPOSE_SEQUENCING_MASK)
            == RSE_PROVISIONING_BLOB_CHAINED);
}

static inline bool is_blob_personalized(const struct rse_provisioning_message_blob_t *blob)
{
    return (((blob->metadata >> RSE_PROVISIONING_BLOB_DETAILS_PERSONALIZATION_OFFSET)
            & RSE_PROVISIONING_BLOB_DETAILS_PERSONALIZATION_MASK)
            == RSE_PROVISIONING_BLOB_TYPE_PERSONALIZED);
}


static enum tfm_plat_err_t disable_debug_before_running_blob(
                            const struct rse_provisioning_message_blob_t *blob,
                            enum lcm_lcs_t lcs, enum lcm_tp_mode_t tp_mode)
{
    if ((tp_mode != LCM_TP_MODE_VIRGIN) &&
	((lcs == LCM_LCS_CM) || (lcs == LCM_LCS_DM)) &&
	is_sp_mode_required_for_blob(blob)) {
        return tfm_plat_otp_secure_provisioning_start();
    } else {
        /* FixMe: this has a bool return but the function returns tfm_plat_err_t */
        return rse_disable_debug_and_reset();
    }
}

static inline bool blob_needs_code_data_decryption(
        const struct rse_provisioning_message_blob_t *blob)
{
    enum rse_provisioning_blob_code_and_data_decryption_config_t decryption_config =
        (blob->metadata >> RSE_PROVISIONING_BLOB_DETAILS_CODE_DATA_DECRYPTION_OFFSET)
        & RSE_PROVISIONING_BLOB_DETAILS_CODE_DATA_DECRYPTION_MASK;

    return (decryption_config == RSE_PROVISIONING_BLOB_CODE_DATA_DECRYPTION_AES);
}

static inline bool blob_needs_secret_decryption(
        const struct rse_provisioning_message_blob_t *blob)
{
    enum rse_provisioning_blob_secret_values_decryption_config_t decryption_config =
        (blob->metadata >> RSE_PROVISIONING_BLOB_DETAILS_SECRET_VALUES_DECRYPTION_OFFSET)
        & RSE_PROVISIONING_BLOB_DETAILS_SECRET_VALUES_DECRYPTION_MASK;

    return decryption_config == RSE_PROVISIONING_BLOB_SECRET_VALUES_DECRYPTION_AES;
}

static enum tfm_plat_err_t copy_auth_code_data(const struct rse_provisioning_message_blob_t *blob,
                                               void *code_output, size_t code_output_size,
                                               void *data_output, size_t data_output_size)
{
    if (blob->code_size > code_output_size) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_CODE_SIZE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_CODE_SIZE;
    }

    if (blob->data_size > data_output_size) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_DATA_SIZE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_DATA_SIZE;
    }

    memcpy(code_output, blob->code_and_data_and_secret_values, blob->code_size);
    cc3xx_lowlevel_aes_update_authed_data(code_output, blob->code_size);

    memcpy(data_output, blob->code_and_data_and_secret_values + blob->code_size, blob->data_size);
    cc3xx_lowlevel_aes_update_authed_data(data_output, blob->data_size);

    return TFM_PLAT_ERR_SUCCESS;
}

static enum tfm_plat_err_t decrypt_code_data(const struct rse_provisioning_message_blob_t *blob,
                                             void *code_output, size_t code_output_size,
                                             void *data_output, size_t data_output_size)
{
    enum cc3xx_error cc_err;

    /* This catches if the buffer would overflow */
    cc3xx_lowlevel_aes_set_output_buffer(code_output, code_output_size);
    cc_err = cc3xx_lowlevel_aes_update(blob->code_and_data_and_secret_values,
                                       blob->code_size);
    if (cc_err != CC3XX_ERR_SUCCESS) {
        return (enum tfm_plat_err_t)cc_err;
    }

    cc3xx_lowlevel_aes_set_output_buffer(data_output, data_output_size);
    cc_err = cc3xx_lowlevel_aes_update(blob->code_and_data_and_secret_values + blob->code_size,
                                       blob->data_size);
    if (cc_err != CC3XX_ERR_SUCCESS) {
        return (enum tfm_plat_err_t)cc_err;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

static enum tfm_plat_err_t copy_auth_secret_values(const struct rse_provisioning_message_blob_t *blob,
                                                   void *values_output, size_t values_output_size)
{
    if (blob->secret_values_size > values_output_size) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_VALUES_SIZE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_VALUES_SIZE;
    }

    memcpy(values_output, blob->code_and_data_and_secret_values + blob->code_size + blob->data_size,
           blob->secret_values_size);
    cc3xx_lowlevel_aes_update_authed_data(values_output, blob->secret_values_size);

    return TFM_PLAT_ERR_SUCCESS;
}

static enum tfm_plat_err_t decrypt_secret_values(const struct rse_provisioning_message_blob_t *blob,
                                                   void *values_output, size_t values_output_size)
{
    enum cc3xx_error cc_err;

    cc3xx_lowlevel_aes_set_output_buffer(values_output, values_output_size);
    cc_err = cc3xx_lowlevel_aes_update(blob->code_and_data_and_secret_values
                                       + blob->code_size + blob->data_size,
                                       blob->secret_values_size);
    if (cc_err != CC3XX_ERR_SUCCESS) {
        return (enum tfm_plat_err_t)cc_err;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

typedef enum tfm_plat_err_t (*setup_aes_key_t)(const struct rse_provisioning_message_blob_t *, uint32_t *);

static enum tfm_plat_err_t aes_generic_blob_operation(cc3xx_aes_mode_t mode,
                                                      const struct rse_provisioning_message_blob_t *blob,
                                                      uint8_t *iv, size_t iv_len,
                                                      void *code_output, size_t code_output_size,
                                                      void *data_output, size_t data_output_size,
                                                      void *values_output, size_t values_output_size,
                                                      setup_aes_key_t setup_aes_key)
{
    enum tfm_plat_err_t err;
    enum cc3xx_error cc_err;
    size_t actual_blob_size = sizeof(*blob) + blob->code_size + blob->data_size + blob->secret_values_size;
    size_t data_size_to_decrypt = 0;
    size_t data_size_to_auth = 0;
    uint32_t authed_header_offset = offsetof(struct rse_provisioning_message_blob_t, metadata);
    size_t authed_header_size = offsetof(struct rse_provisioning_message_blob_t, code_and_data_and_secret_values) - authed_header_offset;
    cc3xx_aes_key_id_t kmu_slot;

    if (blob_needs_code_data_decryption(blob)) {
        data_size_to_decrypt += blob->code_size;
        data_size_to_decrypt += blob->data_size;
    }

    if (blob_needs_secret_decryption(blob)) {
        data_size_to_decrypt += blob->secret_values_size;
    }

    err = setup_aes_key(blob, (uint32_t *)&kmu_slot);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        ERROR("Provisioning key setup failed\n");
        return err;
    }

    if ((mode == CC3XX_AES_MODE_CCM) || (data_size_to_decrypt != 0)) {
        cc_err = cc3xx_lowlevel_aes_init(CC3XX_AES_DIRECTION_DECRYPT, mode,
                                         kmu_slot, NULL,
                                         CC3XX_AES_KEYSIZE_256,
                                         (uint32_t*)iv,
                                         iv_len);
        if (cc_err != CC3XX_ERR_SUCCESS) {
            ERROR("CC3XX setup failed\n");
            return (enum tfm_plat_err_t)cc_err;
        }
    }

    if (blob->signature_size < AES_TAG_MAX_LEN) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_TAG_LEN);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_TAG_LEN;
    }

    data_size_to_auth = actual_blob_size - data_size_to_decrypt
                        - authed_header_offset;

    cc3xx_lowlevel_aes_set_tag_len(AES_TAG_MAX_LEN);
    cc3xx_lowlevel_aes_set_data_len(data_size_to_decrypt, data_size_to_auth);

    cc3xx_lowlevel_aes_update_authed_data(((uint8_t *)blob) + authed_header_offset,
                                          authed_header_size);

    /* If the values are decrypted by the blob but the code is decrypted here,
     * then the ordering would have changed so that we don't have an
     * _update_auth_data operation after an _update operation. Instead, this is
     * an invalid configuration.
     */
    if (!blob_needs_secret_decryption(blob) && blob_needs_code_data_decryption(blob)) {
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_DECRYPTION_CONFIG;
    }

    if (blob_needs_code_data_decryption(blob)) {
        err = decrypt_code_data(blob,
                                code_output, code_output_size,
                                data_output, data_output_size);
    } else {
        err = copy_auth_code_data(blob,
                                  code_output, code_output_size,
                                  data_output, data_output_size);
    }
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (blob_needs_secret_decryption(blob)) {
        err = decrypt_secret_values(blob, values_output, values_output_size);
    } else {
        err = copy_auth_secret_values(blob, values_output, values_output_size);
    }
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if ((mode == CC3XX_AES_MODE_CCM) || (data_size_to_decrypt != 0)) {
        cc_err = cc3xx_lowlevel_aes_finish((uint32_t *)blob->signature, NULL);
        if (cc_err != CC3XX_ERR_SUCCESS) {
            ERROR("bundle decryption failed\n");
            return (enum tfm_plat_err_t)cc_err;
        }
    }

    return TFM_PLAT_ERR_SUCCESS;
}

static enum tfm_plat_err_t aes_decrypt_and_unpack_blob(const struct rse_provisioning_message_blob_t *blob,
                                                        void *code_output, size_t code_output_size,
                                                        void *data_output, size_t data_output_size,
                                                        void *values_output, size_t values_output_size,
                                                        setup_aes_key_t setup_aes_key)
{
    uint8_t iv[AES_IV_LEN] = {0};

    memcpy(iv, (uint8_t*)blob->iv, sizeof(blob->iv));

    return aes_generic_blob_operation(CC3XX_AES_MODE_CTR, blob, iv, AES_IV_LEN,
                                      code_output, code_output_size,
                                      data_output, data_output_size,
                                      values_output, values_output_size,
                                      setup_aes_key);
}

static enum tfm_plat_err_t aes_validate_and_unpack_blob(const struct rse_provisioning_message_blob_t *blob,
                                                        void *code_output, size_t code_output_size,
                                                        void *data_output, size_t data_output_size,
                                                        void *values_output, size_t values_output_size,
                                                        setup_aes_key_t setup_aes_key)
{
    uint8_t iv[AES_IV_LEN] = {0};

    memcpy(iv, (uint8_t*)blob->iv, sizeof(blob->iv));

    return aes_generic_blob_operation(CC3XX_AES_MODE_CCM, blob,
                                      iv, 8,
                                      code_output, code_output_size,
                                      data_output, data_output_size,
                                      values_output, values_output_size,
                                      setup_aes_key);
}

static enum tfm_plat_err_t hash_blob(const struct rse_provisioning_message_blob_t *blob,
                                     void *code_output, size_t code_output_size,
                                     void *data_output, size_t data_output_size,
                                     void *values_output, size_t values_output_size,
                                     uint8_t *hash, size_t hash_len, size_t *hash_size)
{
    int32_t bl1_err;
    const uint32_t authed_header_offset =
        offsetof(struct rse_provisioning_message_blob_t, metadata);
    const size_t authed_header_size =
        offsetof(struct rse_provisioning_message_blob_t, code_and_data_and_secret_values) - authed_header_offset;

    bl1_err = fih_int_decode(bl1_hash_init(RSE_PROVISIONING_HASH_ALG));
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    bl1_err = bl1_hash_update(((uint8_t *)blob) + authed_header_offset,
                                        authed_header_size);
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    bl1_err = bl1_hash_update(code_output, blob->code_size);
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    bl1_err = bl1_hash_update(data_output, blob->data_size);
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    bl1_err = bl1_hash_update(values_output, blob->secret_values_size);
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    bl1_err = bl1_hash_finish(hash, hash_len, hash_size);
    if (bl1_err != 0) {
        return (enum tfm_plat_err_t)bl1_err;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

typedef enum tfm_plat_err_t (*get_rotpk_t)(uint32_t **, size_t *, uint32_t **, size_t *);

static enum tfm_plat_err_t ecdsa_validate_and_unpack_blob(const struct rse_provisioning_message_blob_t *blob,
                                                          void *code_output, size_t code_output_size,
                                                          void *data_output, size_t data_output_size,
                                                          void *values_output, size_t values_output_size,
                                                          setup_aes_key_t setup_aes_key,
                                                          get_rotpk_t get_rotpk)
{
    enum tfm_plat_err_t err;
    cc3xx_err_t cc_err;
    uint32_t blob_hash[RSE_PROVISIONING_HASH_MAX_SIZE / sizeof(uint32_t)];
    uint32_t *public_key_x;
    uint32_t *public_key_y;
    size_t public_key_x_size;
    size_t public_key_y_size;
    size_t sig_point_len = blob->signature_size / 2;
    size_t hash_size;

    /* FixMe: Check the usage of AES in the ECDSA signed blob */
    err = aes_decrypt_and_unpack_blob(blob,
                                      code_output, code_output_size,
                                      data_output, data_output_size,
                                      values_output, values_output_size,
                                      setup_aes_key);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = hash_blob(blob,
                    code_output, code_output_size,
                    data_output, data_output_size,
                    values_output, values_output_size,
                    (uint8_t *)blob_hash, sizeof(blob_hash), &hash_size);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = get_rotpk(&public_key_x, &public_key_x_size, &public_key_y, &public_key_y_size);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    cc_err = cc3xx_lowlevel_ecdsa_verify(RSE_PROVISIONING_CURVE,
                                         public_key_x, public_key_x_size,
                                         public_key_y, public_key_y_size,
                                         blob_hash, hash_size,
                                         (uint32_t *)blob->signature, sig_point_len,
                                         (uint32_t *)(blob->signature + sig_point_len), sig_point_len);
    return cc_err;
}

static enum tfm_plat_err_t ecdsa_validate_blob_without_unpacking(const struct rse_provisioning_message_blob_t *blob,
                                                                 get_rotpk_t get_rotpk)
{
    enum tfm_plat_err_t err;
    cc3xx_err_t cc_err;
    uint32_t blob_hash[RSE_PROVISIONING_HASH_MAX_SIZE / sizeof(uint32_t)];
    uint32_t *public_key_x;
    uint32_t *public_key_y;
    size_t public_key_x_size;
    size_t public_key_y_size;
    size_t sig_point_len = blob->signature_size / 2;
    size_t hash_size;

    err = hash_blob(blob,
                    (uint8_t *)blob->code_and_data_and_secret_values, blob->code_size,
                    (uint8_t *)blob->code_and_data_and_secret_values + blob->code_size, blob->data_size,
                    (uint8_t *)blob->code_and_data_and_secret_values + blob->code_size + blob->data_size,
                    blob->secret_values_size,
                    (uint8_t *)blob_hash, sizeof(blob_hash), &hash_size);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = get_rotpk(&public_key_x, &public_key_x_size, &public_key_y, &public_key_y_size);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    cc_err = cc3xx_lowlevel_ecdsa_verify(RSE_PROVISIONING_CURVE,
                                         public_key_x, public_key_x_size,
                                         public_key_y, public_key_y_size,
                                         blob_hash, hash_size,
                                         (uint32_t *)blob->signature, sig_point_len,
                                         (uint32_t *)(blob->signature + sig_point_len), sig_point_len);
    return cc_err;
}

static enum tfm_plat_err_t validate_and_unpack_blob(const struct rse_provisioning_message_blob_t *blob,
                                                    size_t msg_size,
                                                    void *code_output, size_t code_output_size,
                                                    void *data_output, size_t data_output_size,
                                                    void *values_output, size_t values_output_size,
                                                    setup_aes_key_t setup_aes_key,
                                                    get_rotpk_t get_rotpk)
{
    size_t blob_payload_size = blob->code_size + blob->data_size + blob->secret_values_size;
    enum rse_provisioning_blob_signature_config_t sig_config;

    if (blob_payload_size > msg_size) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_SIZE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_INVALID_SIZE;
    }

    if (blob->code_size == 0) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_BLOB_ZERO_CODE_SIZE);
        return TFM_PLAT_ERR_PROVISIONING_BLOB_ZERO_CODE_SIZE;
    }

    sig_config = (blob->metadata >> RSE_PROVISIONING_BLOB_DETAILS_SIGNATURE_OFFSET)
                 & RSE_PROVISIONING_BLOB_DETAILS_SIGNATURE_MASK;
    switch(sig_config) {
#ifdef RSE_PROVISIONING_ENABLE_AES_SIGNATURES
    case RSE_PROVISIONING_BLOB_SIGNATURE_KRTL_DERIVATIVE:
        return aes_validate_and_unpack_blob(blob,
                                            code_output, code_output_size,
                                            data_output, data_output_size,
                                            values_output, values_output_size,
                                            setup_aes_key);
#endif
#ifdef RSE_PROVISIONING_ENABLE_ECDSA_SIGNATURES
    case RSE_PROVISIONING_BLOB_SIGNATURE_ROTPK_IN_ROM:
    case RSE_PROVISIONING_BLOB_SIGNATURE_ROTPK_NOT_IN_ROM:
        return ecdsa_validate_and_unpack_blob(blob,
                                              code_output, code_output_size,
                                              data_output, data_output_size,
                                              values_output, values_output_size,
                                              setup_aes_key, get_rotpk);
#endif
    default:
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_SIGNATURE_TYPE_NOT_SUPPORTED);
        return TFM_PLAT_ERR_PROVISIONING_SIGNATURE_TYPE_NOT_SUPPORTED;
    }
}

static enum tfm_plat_err_t run_blob(void *code_ptr)
{
    enum tfm_plat_err_t err;

    INFO("Running blob\n");

    err = ((enum tfm_plat_err_t (*)(void))((uintptr_t)code_ptr | 0b1))();
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t default_blob_handler(const struct rse_provisioning_message_blob_t *blob,
                                         size_t msg_size, const void *ctx)
{
    enum tfm_plat_err_t err;
    enum lcm_lcs_t lcs;
    enum lcm_tp_mode_t tp_mode;
    enum lcm_tp_mode_t blob_tp_mode;
    enum lcm_error_t lcm_err;
    enum lcm_bool_t sp_mode_enabled;

    struct default_blob_handler_ctx_t *blob_ctx = (struct default_blob_handler_ctx_t *)ctx;

    lcm_get_tp_mode(&LCM_DEV_S, &tp_mode);

    lcm_err = lcm_get_lcs(&LCM_DEV_S, &lcs);
    if (lcm_err != LCM_ERROR_NONE) {
        return (enum tfm_plat_err_t)lcm_err;
    }

    err = get_tp_mode_for_blob(blob, &blob_tp_mode);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (blob_tp_mode != tp_mode) {
        if (tp_mode == LCM_TP_MODE_VIRGIN && blob_tp_mode == LCM_TP_MODE_PCI) {
            lcm_err = lcm_set_tp_mode(&LCM_DEV_S, LCM_TP_MODE_PCI);
            if (lcm_err != LCM_ERROR_NONE) {
                return (enum tfm_plat_err_t)lcm_err;
            }
            tfm_hal_system_reset();
        }

#if !(defined(RSE_PROVISIONING_CM_DEBUG_CLOSED) && defined(RSE_PROVISIONING_REQUIRE_AUTHENTICATION_FOR_TCI))
        if (tp_mode == LCM_TP_MODE_VIRGIN && blob_tp_mode == LCM_TP_MODE_TCI) {
            lcm_err = lcm_set_tp_mode(&LCM_DEV_S, LCM_TP_MODE_TCI);
            if (lcm_err != LCM_ERROR_NONE) {
                return (enum tfm_plat_err_t)lcm_err;
            }
            tfm_hal_system_reset();
        }
#endif

        FATAL_ERR(TFM_PLAT_ERR_PROVISINING_INVALID_TP_MODE);
        return TFM_PLAT_ERR_PROVISINING_INVALID_TP_MODE;
    }

    err = is_blob_valid_for_lcs(blob, lcs);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    if (!rse_debug_is_disabled()) {
        err = disable_debug_before_running_blob(blob, lcs, tp_mode);
        if (err != TFM_PLAT_ERR_SUCCESS) {
            return err;
        }

        tfm_hal_system_reset();
    }

    lcm_get_sp_enabled(&LCM_DEV_S, &sp_mode_enabled);
    if (is_sp_mode_required_for_blob(blob) != (sp_mode_enabled == LCM_TRUE)) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_INVALID_SP_MODE);
        return TFM_PLAT_ERR_PROVISIONING_INVALID_SP_MODE;
    }

    if (is_blob_chainloaded(blob) != blob_ctx->blob_is_chainloaded) {
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_SEQUENCING);
        return TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_SEQUENCING;
    }

    if (is_blob_personalized(blob)) {
#ifdef RSE_OTP_HAS_SOC_AREA
        if (memcmp(blob->soc_uid, (void *)P_RSE_OTP_SOC->unique_id, sizeof(P_RSE_OTP_SOC->unique_id)) != 0) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_PERSONALIZATION);
            return TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_PERSONALIZATION;
        }
#else
        /* Cannot accept personalized blobs for OTP without SoC Area*/
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_PERSONALIZATION);
        return TFM_PLAT_ERR_PROVISIONING_INVALID_BLOB_PERSONALIZATION;
#endif /* RSE_OTP_HAS_SOC_AREA */

    } else {
#if RSE_CM_REQUIRES_PERSONALIZED_BLOBS
        if (lcs == LCM_LCS_CM) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_CM_DERIVATION_NOT_PERSONALIZED);
            return TFM_PLAT_ERR_PROVISIONING_CM_DERIVATION_NOT_PERSONALIZED;
        }
#endif
#ifdef RSE_DM_REQUIRES_PERSONALIZED_BLOBS
        if (lcs == LCM_LCS_DM) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_DM_DERIVATION_NOT_PERSONALIZED);
            return TFM_PLAT_ERR_PROVISIONING_DM_DERIVATION_NOT_PERSONALIZED;
        }
#endif
    }

    err = validate_and_unpack_blob(blob, msg_size,
                                   (void * )PROVISIONING_BUNDLE_CODE_START,
                                   PROVISIONING_BUNDLE_CODE_SIZE,
                                   (void * )PROVISIONING_BUNDLE_DATA_START,
                                   PROVISIONING_BUNDLE_DATA_SIZE,
                                   (void * )PROVISIONING_BUNDLE_VALUES_START,
                                   PROVISIONING_BUNDLE_VALUES_SIZE,
                                   blob_ctx->setup_aes_key, blob_ctx->get_rotpk);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    err = run_blob((void *)PROVISIONING_BUNDLE_CODE_START);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return err;
    }

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t handle_provisioning_message(const struct rse_provisioning_message_t *msg, size_t msg_size,
                                                struct provisioning_message_handler_config *config, void *ctx)
{
    if (msg_size < sizeof(msg->header) || msg->header.data_length > msg_size - sizeof(msg->header)) {
        return TFM_PLAT_ERR_PROVISIONING_MESSAGE_INVALID_SIZE;
    }

    switch(msg->header.type) {
    case RSE_PROVISIONING_MESSAGE_TYPE_BLOB:
        if (config->blob_handler == NULL) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_BLOB_HANDLER);
            return TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_BLOB_HANDLER;
        }
        return config->blob_handler(&msg->blob, msg->header.data_length, ctx);
    case RSE_PROVISIONING_MESSAGE_TYPE_PLAIN_DATA:
        if (config->plain_data_handler == NULL) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_PLAIN_DATA_HANDLER);
            return TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_PLAIN_DATA_HANDLER;
        }
        return config->plain_data_handler(&msg->plain, msg->header.data_length, ctx);
    case RSE_PROVISIONING_MESSAGE_TYPE_CERTIFICATE:
        if (config->cert_handler == NULL) {
            FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_CERT_HANDLER);
            return TFM_PLAT_ERR_PROVISIONING_MESSAGE_NO_CERT_HANDLER;
        }
        return config->cert_handler(&msg->cert, msg->header.data_length, ctx);
    default:
        FATAL_ERR(TFM_PLAT_ERR_PROVISIONING_MESSAGE_INVALID_TYPE);
        return TFM_PLAT_ERR_PROVISIONING_MESSAGE_INVALID_TYPE;
    }
}
