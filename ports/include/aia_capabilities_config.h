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
