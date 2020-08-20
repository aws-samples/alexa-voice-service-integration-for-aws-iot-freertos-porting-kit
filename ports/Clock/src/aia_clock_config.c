/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
 * @file aia_clock_config.c
 * @brief Implements platform-specific clock functions which are not inlined
 * in
 * @c aia_clock_config.h.
 */

#include <clock/aia_clock_config.h>
#include <iot/aia_iot_config.h>

#include <aiacore/aia_utils.h>

#include AiaClock( HEADER )

#include <time.h>

#define AIA_CLOCK_YEARS_BEWEEN_NTC_AND_UTC_EPOCH 70
#define AIA_CLOCK_DAYS_PER_YEAR 365
#define AIA_CLOCK_NUM_EXTRA_DAYS_BETWEEN_NTC_AND_UTC_EPOCH 17
#define AIA_CLOCK_SECONDS_PER_DAY 86400UL

/** @name Variables synchronized by g_spinLock. */
/** @{ */

/* Holds last synchronized, or initial time if synchronization has not occurred
 * yet. */
static AiaTimepointSeconds_t g_lastRetrievedTimeSinceNTPEpoch;

/* Default last synchronization time to initialization time, expressed in terms
 * of a monotonically increasing clock. */
static AiaTimepointSeconds_t g_lastSynchronization;

/* State that tracks whether the system clock is synchronized with the AIA
 * server or not. */
static bool g_serverSynchronized = false;

/** @} */

/** Simple spin lock boolean flag to guard against asynchronous reads/writes to
 * the below clock functions. */
static AiaAtomicBool_t g_spinLock = false;

AiaTimepointSeconds_t AiaClock_GetTimeSinceNTPEpoch()
{
    while( !Atomic_CompareAndSwap_u32( &g_spinLock, 1, 0 ) )
    {
        /* spin lock, inefficient */
    }
    if( !g_serverSynchronized )
    {
        /* RFC 868 */
        static const AiaTimepointSeconds_t UNIX_NTP_EPOCH_DELTA_SECONDS =
            2208988800;
        /* Initial retrieval of time from system. */
        /* Although not defined by the C standard, this is almost always an
         * integral value holding the number of seconds (not counting leap
         * seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time. */
        g_lastRetrievedTimeSinceNTPEpoch =
            time( NULL ) + UNIX_NTP_EPOCH_DELTA_SECONDS;
        g_lastSynchronization = AiaClock( GetTimeMs )() / AIA_MS_PER_SECOND;
    }
    AiaTimepointSeconds_t ret =
        g_lastRetrievedTimeSinceNTPEpoch +
        ( ( AiaClock( GetTimeMs )() / AIA_MS_PER_SECOND ) -
          g_lastSynchronization );
    AiaAtomicBool_Clear( &g_spinLock );
    return ret;
}

void AiaClock_SetTimeSinceNTPEpoch( AiaTimepointSeconds_t secondsSinceNTPEpoch )
{
    time_t secondsSinceUtcEpoch =
        secondsSinceNTPEpoch -
        ( AIA_CLOCK_YEARS_BEWEEN_NTC_AND_UTC_EPOCH * AIA_CLOCK_DAYS_PER_YEAR +
          AIA_CLOCK_NUM_EXTRA_DAYS_BETWEEN_NTC_AND_UTC_EPOCH ) *
            AIA_CLOCK_SECONDS_PER_DAY;
    AiaLogInfo( "UTC time = %s", asctime( gmtime( &secondsSinceUtcEpoch ) ) );

    while( !Atomic_CompareAndSwap_u32( &g_spinLock, 1, 0 ) )
    {
        /* spin lock, inefficient */
    }
    g_serverSynchronized = true;
    g_lastSynchronization = AiaClock( GetTimeMs )() / AIA_MS_PER_SECOND;
    g_lastRetrievedTimeSinceNTPEpoch = secondsSinceNTPEpoch;
    AiaAtomicBool_Clear( &g_spinLock );
}
