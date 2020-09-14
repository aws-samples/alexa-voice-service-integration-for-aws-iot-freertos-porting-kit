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

#ifndef AIA_COMMON_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_COMMON_CONFIG_H_

#include <microphone/aia_microphone_config.h>

#include <assert.h>
#include <stddef.h>

/**
 * Tests whether @c EXPRESSION is true, and terminates the application if it is
 * not.
 * @warning Applications may stub this out such that EXPRESSION is never
 * evaluated, so application code should not depend on it.  For example, if @c
 * AiaAssert() is implemented using @c assert(), the asserts will all be
 * disabled uf @c NDEBUG is defined.
 *
 * @param EXPRESSION A boolean expression to evaluate and assert is true.
 */
#define AiaAssert( EXPRESSION ) assert( ( EXPRESSION ) )

/**
 * For those platform do not define ssize_t type
 */
typedef long ssize_t;

/**
 * This function will be called by the SDK when critical un-recoverable failures
 * occur. Examples include memory allocation failures.
 */
static inline void AiaCriticalFailure()
{
    AiaLogError( "AiaCriticalFailure" );
    AiaAssert(0);   // or use iot_reset_reboot(True) by vendors' implementation
}

/** How often data will be published on the /event topic. */
static const AiaDurationMs_t EVENT_PUBLISH_RATE = MICROPHONE_PUBLISH_RATE;

/** How many slots to be used in a sequencing buffer. */
static const size_t AIA_SEQUENCER_SLOTS = 4;

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_COMMON_CONFIG_H_ */
