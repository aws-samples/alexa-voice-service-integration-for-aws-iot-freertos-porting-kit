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

#ifndef AIA_CLOCK_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_CLOCK_CONFIG_H_

#include <stddef.h>
#include <stdint.h>

/** Macros and typedefs for clock functions. We require the typedefs to be
 * fixed-size POD types. */
/** @{ */

typedef uint64_t AiaTimepointMs_t;
typedef uint32_t AiaDurationMs_t;
typedef uint64_t AiaTimepointSeconds_t;
typedef uint32_t AiaDurationSeconds_t;

/**
 * @name AIA clock-capability related functionality. Implementations are
 * expected to be thread-safe.
 */
/** @{ */

/**
 * Retrieves the time since the NTP epoch (Jan 1, 1900) expressed in seconds.
 *
 * @return seconds since the NTP epoch.
 */
AiaTimepointSeconds_t AiaClock_GetTimeSinceNTPEpoch();

/**
 * Synchronizes the device clock with a server provided time. This function will
 * only be called if the application attempts to synchronize its clock with the
 * service.
 *
 * @param secondsSinceNTPEpoch seconds since the NTP epoch. Note that this value
 * is received from a remote service and a networking hops are not accounted
 * for.
 */
void AiaClock_SetTimeSinceNTPEpoch(
    AiaTimepointSeconds_t secondsSinceNTPEpoch );

/** @} */
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_CLOCK_CONFIG_H_ */
