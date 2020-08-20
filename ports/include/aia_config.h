/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * You may not use this file except in compliance with the terms and conditions
 * set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
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
