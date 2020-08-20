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
