/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "psa/client.h"
#include "psa/internal_trusted_storage.h"
#include "psa_manifest/sid.h"
#include "tfm_its_defs.h"

struct rot_psa_its_storage_info_t {
    rot_size_t capacity;
    rot_size_t size;
    psa_storage_create_flags_t flags;
};

psa_status_t psa_its_set(psa_storage_uid_t uid,
                         size_t data_length,
                         const void *p_data,
                         psa_storage_create_flags_t create_flags)
{
    psa_status_t status;

    psa_invec in_vec[] = {
        { .base = &uid, .len = sizeof(uid) },
        { .base = p_data, .len = data_length },
        { .base = &create_flags, .len = sizeof(create_flags) }
    };

    status = psa_call(TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE, TFM_ITS_SET,
                      in_vec, IOVEC_LEN(in_vec), NULL, 0);

    return status;
}

psa_status_t psa_its_get(psa_storage_uid_t uid,
                         size_t data_offset,
                         size_t data_size,
                         void *p_data,
                         size_t *p_data_length)
{
    psa_status_t status;
    rot_size_t data_offset_param;

    psa_invec in_vec[] = {
        { .base = &uid, .len = sizeof(uid) },
        { .base = &data_offset_param, .len = sizeof(data_offset_param) }
    };

    psa_outvec out_vec[] = {
        { .base = p_data, .len = data_size }
    };

    if (data_offset > ROT_SIZE_MAX) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }
    data_offset_param = (rot_size_t)data_offset;

    if (p_data_length == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    status = psa_call(TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE, TFM_ITS_GET,
                      in_vec, IOVEC_LEN(in_vec), out_vec, IOVEC_LEN(out_vec));

    *p_data_length = out_vec[0].len;

    return status;
}

psa_status_t psa_its_get_info(psa_storage_uid_t uid,
                              struct psa_storage_info_t *p_info)
{
    psa_status_t status;
    struct rot_psa_its_storage_info_t info_param = {0};

    psa_invec in_vec[] = {
        { .base = &uid, .len = sizeof(uid) }
    };

    psa_outvec out_vec[] = {
        { .base = &info_param, .len = sizeof(info_param) }
    };

    if (p_info == NULL) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    status = psa_call(TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE,
                      TFM_ITS_GET_INFO, in_vec, IOVEC_LEN(in_vec), out_vec,
                      IOVEC_LEN(out_vec));

    p_info->capacity = info_param.capacity;
    p_info->size = info_param.size;
    p_info->flags = info_param.flags;

    return status;
}

psa_status_t psa_its_remove(psa_storage_uid_t uid)
{
    psa_status_t status;

    psa_invec in_vec[] = {
        { .base = &uid, .len = sizeof(uid) }
    };

    status = psa_call(TFM_INTERNAL_TRUSTED_STORAGE_SERVICE_HANDLE,
                      TFM_ITS_REMOVE, in_vec, IOVEC_LEN(in_vec), NULL, 0);

    return status;
}
