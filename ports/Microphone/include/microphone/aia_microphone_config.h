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
