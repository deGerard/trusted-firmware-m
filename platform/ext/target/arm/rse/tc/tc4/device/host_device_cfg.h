/*
 * Copyright (c) 2019-2024, Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HOST_DEVICE_CFG_H__
#define __HOST_DEVICE_CFG_H__

#include "host_device_cfg_common.h"

#define MHU5_S

#define MHU_RSE_TO_SCP_DEV        MHU5_SENDER_DEV_S
#define MHU_SCP_TO_RSE_DEV        MHU5_RECEIVER_DEV_S

#endif  /* __HOST_DEVICE_CFG_H__ */
