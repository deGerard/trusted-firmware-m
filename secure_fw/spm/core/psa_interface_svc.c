/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include "compiler_ext_defs.h"
#include "config_spm.h"
#include "runtime_defs.h"
#include "svc_num.h"
#include "tfm_psa_call_pack.h"
#include "utilities.h"
#include "psa/client.h"
#include "psa/lifecycle.h"
#include "psa/service.h"

__naked uint32_t psa_framework_version_svc(void)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_FRAMEWORK_VERSION)"\n"
                   "bx      lr                                  \n");
}

__naked uint32_t psa_version_svc(uint32_t sid)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_VERSION)"         \n"
                   "bx      lr                                 \n");
}

__naked psa_status_t tfm_psa_call_pack_svc(psa_handle_t handle,
                                           uint32_t ctrl_param,
                                           const psa_invec *in_vec,
                                           psa_outvec *out_vec)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_CALL)"            \n"
                   "bx      lr                                 \n");
}

__naked psa_signal_t psa_wait_svc(psa_signal_t signal_mask, uint32_t timeout)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_WAIT)"            \n"
                   "bx      lr                                 \n");
}

__naked psa_status_t psa_get_svc(psa_signal_t signal, psa_msg_t *msg)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_GET)"             \n"
                   "bx      lr                                 \n");
}

__naked size_t psa_read_svc(psa_handle_t msg_handle, uint32_t invec_idx,
                            void *buffer, size_t num_bytes)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_READ)"            \n"
                   "bx      lr                                 \n");
}

__naked size_t psa_skip_svc(psa_handle_t msg_handle,
                            uint32_t invec_idx, size_t num_bytes)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_SKIP)"            \n"
                   "bx      lr                                 \n");
}

__naked void psa_write_svc(psa_handle_t msg_handle, uint32_t outvec_idx,
                           const void *buffer, size_t num_bytes)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_WRITE)"           \n"
                   "bx      lr                                 \n");
}

__naked void psa_reply_svc(psa_handle_t msg_handle, psa_status_t retval)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_REPLY)"           \n"
                   "bx      lr                                 \n");
}

#if CONFIG_TFM_DOORBELL_API == 1
__naked void psa_notify_svc(int32_t partition_id)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_NOTIFY)"          \n"
                   "bx      lr                                 \n");
}

__naked void psa_clear_svc(void)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_CLEAR)"           \n"
                   "bx      lr                                 \n");
}
#endif /* CONFIG_TFM_DOORBELL_API == 1 */

__naked void psa_panic_svc(void)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_PANIC)"           \n"
                   "bx      lr                                 \n");
}

__naked uint32_t psa_rot_lifecycle_state_svc(void)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_LIFECYCLE)"       \n"
                   "bx      lr                                 \n");
}

/* Following PSA APIs are only needed by connection-based services */
#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1

__naked psa_handle_t psa_connect_svc(uint32_t sid, uint32_t version)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_CONNECT)"         \n"
                   "bx      lr                                 \n");
}

__naked void psa_close_svc(psa_handle_t handle)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_CLOSE)"           \n"
                   "bx      lr                                 \n");
}

__naked void psa_set_rhandle_svc(psa_handle_t msg_handle, void *rhandle)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_SET_RHANDLE)"     \n"
                   "bx      lr                                 \n");
}

#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API */

#if CONFIG_TFM_FLIH_API == 1 || CONFIG_TFM_SLIH_API == 1

__naked void psa_irq_enable_svc(psa_signal_t irq_signal)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_IRQ_ENABLE)"      \n"
                   "bx      lr                                 \n");
}

__naked psa_irq_status_t psa_irq_disable_svc(psa_signal_t irq_signal)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_IRQ_DISABLE)"     \n"
                   "bx      lr                                 \n");
}

/* This API is only used for FLIH. */
#if CONFIG_TFM_FLIH_API == 1
__naked void psa_reset_signal_svc(psa_signal_t irq_signal)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_RESET_SIGNAL)"    \n"
                   "bx      lr                                 \n");
}
#endif

/* This API is only used for SLIH. */
#if CONFIG_TFM_SLIH_API == 1
__naked void psa_eoi_svc(psa_signal_t irq_signal)
{
    __asm volatile("svc     "M2S(TFM_SVC_PSA_EOI)"             \n"
                   "bx      lr                                 \n");
}
#endif

#endif /* CONFIG_TFM_FLIH_API == 1 || CONFIG_TFM_SLIH_API == 1 */

#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
__naked psa_status_t agent_psa_call_svc(psa_handle_t handle,
                                        uint32_t control,
                                        const struct client_params_t *params,
                                        const void *client_data_stateless)
{
    __asm volatile("svc     "M2S(TFM_SVC_AGENT_PSA_CALL)"      \n"
                   "bx      lr                                 \n");
}

#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1
__naked psa_handle_t agent_psa_connect_svc(uint32_t sid, uint32_t version,
                                           int32_t ns_client_id,
                                           const void *client_data)
{
    __asm volatile("svc     "M2S(TFM_SVC_AGENT_PSA_CONNECT)"   \n"
                   "bx      lr                                 \n");
}

__naked void agent_psa_close_svc(psa_handle_t handle, int32_t ns_client_id)
{
    __asm volatile("svc     "M2S(TFM_SVC_AGENT_PSA_CLOSE)"     \n"
                   "bx      lr                                 \n");
}

#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1 */
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */

const struct psa_api_tbl_t psa_api_svc = {
                                tfm_psa_call_pack_svc,
                                psa_version_svc,
                                psa_framework_version_svc,
                                psa_wait_svc,
                                psa_get_svc,
                                psa_read_svc,
                                psa_skip_svc,
                                psa_write_svc,
                                psa_reply_svc,
                                psa_panic_svc,
                                psa_rot_lifecycle_state_svc,
#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1
                                psa_connect_svc,
                                psa_close_svc,
                                psa_set_rhandle_svc,
#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1 */
#if CONFIG_TFM_DOORBELL_API == 1
                                psa_notify_svc,
                                psa_clear_svc,
#endif /* CONFIG_TFM_DOORBELL_API == 1 */
#if CONFIG_TFM_FLIH_API == 1 || CONFIG_TFM_SLIH_API == 1
                                psa_irq_enable_svc,
                                psa_irq_disable_svc,
#if CONFIG_TFM_FLIH_API == 1
                                psa_reset_signal_svc,
#endif /* CONFIG_TFM_FLIH_API == 1 */
#if CONFIG_TFM_SLIH_API == 1
                                psa_eoi_svc,
#endif /* CONFIG_TFM_SLIH_API == 1 */
#endif /* CONFIG_TFM_FLIH_API == 1 || CONFIG_TFM_SLIH_API == 1 */
#ifdef TFM_PARTITION_NS_AGENT_MAILBOX
                                agent_psa_call_svc,
#if CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1
                                agent_psa_connect_svc,
                                agent_psa_close_svc,
#endif /* CONFIG_TFM_CONNECTION_BASED_SERVICE_API == 1 */
#endif /* TFM_PARTITION_NS_AGENT_MAILBOX */
                            };
