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

/**
 * @file aia_capabilities_config.h
 * @brief Configurable client capabilities.
 */

#ifndef AIA_CAPABILITIES_CONFIG_H_
#define AIA_CAPABILITIES_CONFIG_H_

#include <aiacore/aia_json_constants.h>

#include <stdint.h>

/**
 * @name "Speaker" interface capabilities.
 *
 * If this interface is supported on
 * this device, all below configurations must defined. If this interface is not
 * supported on this device, all below configurations must be deleted entirely.
 */
/** @{ */
#define AIA_ENABLE_SPEAKER
#define AIA_AUDIO_BUFFER_SIZE UINT64_C( 128000 )
static const AiaJsonLongType AIA_AUDIO_BUFFER_OVERRUN_WARN_THRESHOLD =
    8 * AIA_AUDIO_BUFFER_SIZE / 10;
static const AiaJsonLongType AIA_AUDIO_BUFFER_UNDERRUN_WARN_THRESHOLD =
    2 * AIA_AUDIO_BUFFER_SIZE / 10;
#define AIA_SPEAKER_AUDIO_DECODER_FORMAT "OPUS"
#define AIA_SPEAKER_AUDIO_DECODER_BITRATE_TYPE "CONSTANT"
static const AiaJsonLongType AIA_SPEAKER_AUDIO_DECODER_BITS_PER_SECOND = 64000;
#define AIA_SPEAKER_AUDIO_DECODER_NUM_CHANNELS UINT64_C( 1 )
/** @} */

/**
 * @name "Microphone" interface capabilities.
 *
 * If this interface is supported on
 * this device, all below configurations must defined. If this interface is not
 * supported on this device, all below configurations must be deleted entirely.
 */
/** @{ */
#define AIA_ENABLE_MICROPHONE
#define AIA_MICROPHONE_AUDIO_ENCODER_FORMAT "AUDIO_L16_RATE_16000_CHANNELS_1"
/** @} */

/**
 * @name "Alerts" interface capabilities.
 *
 * If this interface is supported on
 * this device, all below configurations must defined. If this interface is not
 * supported on this device, all below configurations must be deleted entirely.
 */
/** @{ */
#define AIA_ENABLE_ALERTS
static const AiaJsonLongType AIA_ALERTS_MAX_ALERT_COUNT = 20;
/** @} */

/**
 * @name "Alerts" interface capabilities.
 *
 * If this interface is supported on
 * this device, all below configurations must defined. If this interface is not
 * supported on this device, all below configurations must be deleted entirely.
 */
/** @{ */
#define AIA_ENABLE_CLOCK
/** @} */

/**
 * @name "System" interface capabilities.
 */
/** @{ */
#define AIA_ENABLE_SYSTEM
static const AiaJsonLongType AIA_SYSTEM_MQTT_MESSAGE_MAX_SIZE = 8000;
#define AIA_SYSTEM_FIRMWARE_VERSION "42"
#define AIA_SYSTEM_LOCALE "en-US"
/** @} */

#endif /* ifndef AIA_CAPABILITIES_CONFIG_H_ */
