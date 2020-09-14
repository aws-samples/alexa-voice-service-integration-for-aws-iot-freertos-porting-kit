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

#ifndef AIA_MICROPHONE_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_MICROPHONE_CONFIG_H_

#include <stdlib.h>

/** How often data will be published on the /microphone topic. */
#define MICROPHONE_PUBLISH_RATE ( (AiaDurationMs_t)50 )

/**
 * The amount of microphone samples that will be published when the microphone
 * is open per @c MICROPHONE_PUBLISH_RATE iteration.
 *
 * @note 800 samples represents the real time rate for 50 ms. However, in
 * situations involving preroll (wakeword), UPL will benefit from reading faster
 * than the real-time rate. Ideally, this should be set to the largest value
 * possible not exceeded the device's maximum MQTT message size that the device
 * is capable of publishing.
 */
static const size_t AIA_MICROPHONE_CHUNK_SIZE_SAMPLES = 1600;

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_MICROPHONE_CONFIG_H_ */
