/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "crypto.h"

#include <stdint.h>
#include <string.h>

#include "region_defs.h"
#include "device_definition.h"
#include "otp.h"
#include "fih.h"
#include "cc3xx_drv.h"

#define KEY_DERIVATION_MAX_BUF_SIZE 128

static enum tfm_bl1_hash_alg_t multipart_alg = 0;
static mbedtls_sha512_context multipart_ctx;

fih_int sha256_init()
{
    fih_int fih_rc;

    multipart_alg = TFM_BL1_HASH_ALG_SHA256;

    /* This is only used by TFM_BL1_2 which currently only uses SHA256 */
    fih_rc = fih_int_encode_zero_equality(cc3xx_lowlevel_hash_init(CC3XX_HASH_ALG_SHA256));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    FIH_RET(FIH_SUCCESS);
}

fih_int sha256_finish(uint8_t *hash,
                        size_t hash_length,
                        size_t *hash_size)
{
    cc3xx_lowlevel_hash_finish((uint32_t *)hash, hash_length);

    if (hash_size != NULL) {
        *hash_size = 32;
    }

    multipart_alg = 0;

    FIH_RET(FIH_SUCCESS);
}

fih_int sha256_update(const uint8_t *data,
                        size_t data_length)
{
    fih_int fih_rc;

    fih_rc = fih_int_encode_zero_equality(cc3xx_lowlevel_hash_update(data, data_length));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    FIH_RET(FIH_SUCCESS);
}

static fih_int sha256_compute(const uint8_t *data,
                              size_t data_length,
                              uint8_t *hash, size_t hash_len, size_t *hash_size)
{
    fih_int fih_rc;

    if ((data == NULL) || (hash == NULL)) {
        FIH_RET(FIH_FAILURE);
    }

    fih_rc = fih_int_encode_zero_equality(cc3xx_lowlevel_hash_init(CC3XX_HASH_ALG_SHA256));
    if(fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    fih_rc = fih_int_encode_zero_equality(cc3xx_lowlevel_hash_update(data,
                                                                     data_length));
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }
    cc3xx_lowlevel_hash_finish((uint32_t *)hash, hash_len);

    if (hash_size != NULL) {
        *hash_size = 32;
    }

    FIH_RET(FIH_SUCCESS);
}


fih_int sha384_init()
{
    fih_int fih_rc;
    int rc;

    multipart_alg = TFM_BL1_HASH_ALG_SHA384;

    mbedtls_sha512_init(&multipart_ctx);

    rc = mbedtls_sha512_starts(&multipart_ctx, 1);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    FIH_RET(FIH_SUCCESS);

out:
    mbedtls_sha512_free(&multipart_ctx);
    multipart_alg = 0;
    FIH_RET(fih_rc);

}

fih_int sha384_finish(uint8_t *hash,
                        size_t hash_length,
                        size_t *hash_size)
{
    fih_int fih_rc;
    int rc;

    if (hash_length < 48) {
        fih_rc = FIH_FAILURE;
        goto out;
    }

    rc = mbedtls_sha512_finish(&multipart_ctx, hash);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    if (hash_size != NULL) {
        *hash_size = 48;
    }

out:
    mbedtls_sha512_free(&multipart_ctx);
    multipart_alg = 0;
    FIH_RET(fih_rc);
}

fih_int sha384_update(const uint8_t *data,
                        size_t data_length)
{
    int rc;
    fih_int fih_rc;

    rc = mbedtls_sha512_update(&multipart_ctx, data, data_length);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    FIH_RET(FIH_SUCCESS);

out:
    mbedtls_sha512_free(&multipart_ctx);
    multipart_alg = 0;
    FIH_RET(fih_rc);
}

static fih_int sha384_compute(const uint8_t *data,
                           size_t data_length,
                           uint8_t *hash, size_t hash_len, size_t *hash_size)
{
    int rc = 0;
    fih_int fih_rc;
    mbedtls_sha512_context ctx;

    if (hash_len < 48) {
        FIH_RET(FIH_FAILURE);
    }

    mbedtls_sha512_init(&ctx);

    rc = mbedtls_sha512_starts(&ctx, 1);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    rc = mbedtls_sha512_update(&ctx, data, data_length);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    rc = mbedtls_sha512_finish(&ctx, hash);
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        goto out;
    }

    if (hash_size != NULL) {
        *hash_size = 48;
    }

out:
    mbedtls_sha512_free(&ctx);
    FIH_RET(fih_rc);
}

fih_int bl1_hash_init(enum tfm_bl1_hash_alg_t alg)
{
    fih_int fih_rc;

    switch(alg) {
    case TFM_BL1_HASH_ALG_SHA256:
        FIH_CALL(sha256_init, fih_rc);
        break;
    case TFM_BL1_HASH_ALG_SHA384:
        FIH_CALL(sha384_init, fih_rc);
        break;
    default:
        fih_rc = FIH_FAILURE;
    }

    FIH_RET(fih_rc);
}

fih_int bl1_hash_finish(uint8_t *hash,
                        size_t hash_length,
                        size_t *hash_size)
{
    fih_int fih_rc;

    switch(multipart_alg) {
    case TFM_BL1_HASH_ALG_SHA256:
        FIH_CALL(sha256_finish, fih_rc, hash, hash_length, hash_size);
        break;
    case TFM_BL1_HASH_ALG_SHA384:
        FIH_CALL(sha384_finish, fih_rc, hash, hash_length, hash_size);
        break;
    default:
        fih_rc = FIH_FAILURE;
    }

    FIH_RET(fih_rc);
}

fih_int bl1_hash_update(const uint8_t *data,
                        size_t data_length)
{
    fih_int fih_rc;

    switch(multipart_alg) {
    case TFM_BL1_HASH_ALG_SHA256:
        FIH_CALL(sha256_update, fih_rc, data, data_length);
        break;
    case TFM_BL1_HASH_ALG_SHA384:
        FIH_CALL(sha384_update, fih_rc, data, data_length);
        break;
    default:
        fih_rc = FIH_FAILURE;
    }

    FIH_RET(fih_rc);
}

fih_int bl1_hash_compute(enum tfm_bl1_hash_alg_t alg,
                         const uint8_t *data,
                         size_t data_length,
                         uint8_t *hash,
                         size_t hash_length,
                         size_t *hash_size)
{
    fih_int fih_rc;

    switch(alg) {
    case TFM_BL1_HASH_ALG_SHA256:
        FIH_CALL(sha256_compute, fih_rc, data, data_length, hash, hash_length, hash_size);
        break;
    case TFM_BL1_HASH_ALG_SHA384:
        FIH_CALL(sha384_compute, fih_rc, data, data_length, hash, hash_length, hash_size);
        break;
    default:
        fih_rc = FIH_FAILURE;
    }

    FIH_RET(fih_rc);
}

static int32_t bl1_key_to_cc3xx_key(enum tfm_bl1_key_id_t key_id,
                                    cc3xx_aes_key_id_t *cc3xx_key_type,
                                    uint8_t *key_buf, size_t key_buf_size)
{
    int32_t rc;
    fih_int fih_rc;

    switch(key_id) {
    case TFM_BL1_KEY_HUK:
        *cc3xx_key_type = CC3XX_AES_KEY_ID_HUK;
        break;
    case TFM_BL1_KEY_GUK:
        *cc3xx_key_type = CC3XX_AES_KEY_ID_GUK;
        break;
    case TFM_BL1_KEY_USER:
        return -1;

    default:
        *cc3xx_key_type = CC3XX_AES_KEY_ID_USER_KEY;
        FIH_CALL(bl1_otp_read_key, fih_rc, key_id, key_buf, key_buf_size, NULL);
        if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
            memset(key_buf, 0, key_buf_size);
            return fih_int_decode(fih_rc);
        }
        break;
    }

    return 0;
}

fih_int bl1_aes_256_ctr_decrypt(enum tfm_bl1_key_id_t key_id,
                                const uint8_t *key_material,
                                uint8_t *counter,
                                const uint8_t *ciphertext,
                                size_t ciphertext_length,
                                uint8_t *plaintext)
{
    cc3xx_aes_key_id_t cc3xx_key_type;
    uint32_t key_buf[32 / sizeof(uint32_t)];
    fih_int fih_rc;
    int32_t rc = 0;
    const uint8_t *input_key = key_buf;
    cc3xx_err_t err;

    if (ciphertext_length == 0) {
        FIH_RET(FIH_SUCCESS);
    }

    if ((counter == NULL) || (ciphertext == NULL) || (plaintext == NULL)) {
        FIH_RET(FIH_FAILURE);
    }

    if ((uintptr_t)counter & 0x3) {
        FIH_RET(FIH_FAILURE);
    }

    if (key_material == NULL) {
        rc = bl1_key_to_cc3xx_key(key_id, &cc3xx_key_type, (uint8_t *)key_buf,
                                  sizeof(key_buf));
        fih_rc = fih_int_encode_zero_equality(rc);
        if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
            FIH_RET(fih_rc);
        }
    } else {
        cc3xx_key_type = CC3XX_AES_KEY_ID_USER_KEY;
        input_key = key_material;
    }

    err = cc3xx_lowlevel_aes_init(CC3XX_AES_DIRECTION_DECRYPT, CC3XX_AES_MODE_CTR,
                                  cc3xx_key_type, input_key, CC3XX_AES_KEYSIZE_256,
                                  (uint32_t *)counter, 16);
    fih_rc = fih_int_encode_zero_equality(err);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    cc3xx_lowlevel_aes_set_output_buffer(plaintext, ciphertext_length);
    cc3xx_lowlevel_aes_update(ciphertext, ciphertext_length);
    cc3xx_lowlevel_aes_finish(NULL, NULL);

    FIH_RET(FIH_SUCCESS);
}

static int32_t aes_256_ecb_encrypt(enum tfm_bl1_key_id_t key_id,
                                   const uint8_t *plaintext,
                                   size_t ciphertext_length,
                                   uint8_t *ciphertext)
{
    cc3xx_aes_key_id_t cc3xx_key_type;
    uint32_t key_buf[32 / sizeof(uint32_t)];
    int32_t rc = 0;
    cc3xx_err_t err;

    if (ciphertext_length == 0) {
        return 0;
    }

    if (ciphertext == NULL || plaintext == NULL) {
        return -1;
    }

    rc = bl1_key_to_cc3xx_key(key_id, &cc3xx_key_type, key_buf, sizeof(key_buf));
    if (rc) {
        return rc;
    }

    err = cc3xx_lowlevel_aes_init(CC3XX_AES_DIRECTION_ENCRYPT, CC3XX_AES_MODE_ECB,
                                  cc3xx_key_type, (uint32_t *)key_buf,
                                  CC3XX_AES_KEYSIZE_256,
                                  NULL, 0);
    if (err != CC3XX_ERR_SUCCESS) {
        return 1;
    }

    cc3xx_lowlevel_aes_set_output_buffer(ciphertext, ciphertext_length);
    cc3xx_lowlevel_aes_update(plaintext, ciphertext_length);
    cc3xx_lowlevel_aes_finish(NULL, NULL);
}

/* This is a counter-mode KDF complying with NIST SP800-108 where the PRF is a
 * combined sha256 hash and an ECB-mode AES encryption. ECB is acceptable here
 * since the input to the PRF is a hash, and the hash input is different every
 * time because of the counter being part of the input.
 */
fih_int bl1_derive_key(enum tfm_bl1_key_id_t key_id, const uint8_t *label,
                       size_t label_length, const uint8_t *context,
                       size_t context_length, uint32_t *output_key,
                       size_t output_length)
{
    cc3xx_aes_key_id_t key_type;
    uint32_t key_buf[32 / sizeof(uint32_t)];
    uint8_t *input_key = (uint8_t *)key_buf;
    fih_int fih_rc;
    int32_t rc = 0;
    cc3xx_err_t err;

    rc = bl1_key_to_cc3xx_key(key_id, &key_type, input_key, sizeof(key_buf));
    fih_rc = fih_int_encode_zero_equality(rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
        FIH_RET(fih_rc);
    }

    err = cc3xx_lowlevel_kdf_cmac(key_type, (uint32_t *)input_key,
                                  CC3XX_AES_KEYSIZE_256, label, label_length, context,
                                  context_length, output_key, output_length);

    fih_rc = fih_int_encode_zero_equality(err);
    FIH_RET(fih_rc);
}
