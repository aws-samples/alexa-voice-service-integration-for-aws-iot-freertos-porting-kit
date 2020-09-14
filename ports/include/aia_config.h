/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AIA_CONFIG_H_
#ifdef __cplusplus
extern "C"
{
#endif
#define AIA_CONFIG_H_

/* The IoT config header */
#include <iot_config.h>
#include "threading_alt.h"

#include <button/aia_button_config.h>
#include <crypto/aia_crypto_config.h>
#include <http/aia_http_config.h>
#include <iot/aia_iot_config.h>
#include <lwa/aia_lwa_config.h>
#include <memory/aia_memory_config.h>
#include <microphone/aia_microphone_config.h>
#include <registration/aia_registration_config.h>
#include <storage/aia_storage_config.h>

#include <common/aia_common_config.h>

/* AIA Capabilities Config */
#include <aia_capabilities_config.h>

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_CONFIG_H_ */
