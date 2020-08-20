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
 * @file aia_storage_config.c
 * @brief Implements platform-specific Storage functions which are not inlined
 * in
 * @c aia_storage_config.h.
 */

#include <storage/aia_storage_config.h>

#include <aia_config.h>

#include <aiaalertmanager/aia_alert_constants.h>
#include <aiacore/aia_volume_constants.h>

#include <errno.h>
#include <stdio.h>

const char* g_aiaClientId;
const char* g_aiaAwsAccountId;
const char* g_aiaStorageFolder;

#ifdef AIA_LOAD_VOLUME

uint8_t AiaLoadVolume()
{
    /* TODO: ADSER-1741 Provide an actual reference implementation of persisting
     * device volume. */
    return AIA_DEFAULT_VOLUME;
}

#endif

#define AIA_SHARED_SECRET_STORAGE_KEY "AiaSharedSecretStorageKey"
#define AIA_ALL_ALERTS_STORAGE_KEY_V0 "AiaAllAlertsStorageKey"

bool AiaStoreSecret( const uint8_t* sharedSecret, size_t size )
{
    return AiaStoreBlob( AIA_SHARED_SECRET_STORAGE_KEY, sharedSecret, size );
}

bool AiaLoadSecret( uint8_t* sharedSecret, size_t size )
{
    return AiaLoadBlob( AIA_SHARED_SECRET_STORAGE_KEY, sharedSecret, size );
}

/** If using the provided sample storage implementation, this is the memory
 * spaces that the SDK will read/write from/to. 
 * Vendors should store/load blob to/from NVRAM/persistant storage by their platform
 */

/**
 * blob storage key
 */
typedef enum _blobstorage_key_e
{
    AIA_BLOB_STORAGE_KEY_START = 0,
    AIA_BLOB_SHARED_SECRET_STORAGE_KEY = AIA_BLOB_STORAGE_KEY_START,
    AIA_BLOB_ALL_ALERTS_STORAGE_KEY_V0,
    AIA_BLOB_TOPIC_ROOT_KEY,
    AIA_BLOB_STORAGE_KEY_MAX,
} blobstorage_key_e;

#define BLOBSTORAGE_SHAREDKEY_SIZE 32
#define BLOBSTORAGE_ALERTKEY_SIZE 64
#define BLOBSTORAGE_TOPICROOT_SIZE 16

typedef struct _blobstorage_t {
    const char* key;
    void* const storage;
    const size_t capacity;
    size_t used_len;
} blobstorage_t;

uint8_t blobstorage_sharedkey[ BLOBSTORAGE_SHAREDKEY_SIZE ];
uint8_t blobstorage_alertkey[ BLOBSTORAGE_ALERTKEY_SIZE ];
uint8_t blobstorage_topicroot[ BLOBSTORAGE_TOPICROOT_SIZE ];
blobstorage_t blobstorage[] = {
    { AIA_SHARED_SECRET_STORAGE_KEY, blobstorage_sharedkey, sizeof(blobstorage_sharedkey), 0 }, // AIA_BLOB_SHARED_SECRET_STORAGE_KEY
    { AIA_ALL_ALERTS_STORAGE_KEY_V0, blobstorage_alertkey, sizeof(blobstorage_alertkey), 1 },   // AIA_BLOB_ALL_ALERTS_STORAGE_KEY_V0
    { "AiaTopicRootKey", blobstorage_topicroot, sizeof(blobstorage_topicroot), 0 },             // AIA_BLOB_TOPIC_ROOT_KEY
};

bool AiaStoreBlob( const char* key, const uint8_t* blob, size_t size )
{
    bool process = false;
    blobstorage_key_e k = AIA_BLOB_STORAGE_KEY_MAX;

    for( k = AIA_BLOB_STORAGE_KEY_START; blob && k < AIA_BLOB_STORAGE_KEY_MAX; ++k )
    {
        if( strncmp(key, blobstorage[ k ].key, strlen(key)) == 0 )
        {
            if( blobstorage[ k ].capacity >= size )
            {
                memcpy( blobstorage[ k ].storage, blob, size );
                blobstorage[ k ].used_len = size;
                process = true;
            }
            else
            {
                AiaLogError( "blob Store size error: key(%s), capacity(%u), size(%u)", key, blobstorage[k].capacity, size );
            }

            break;
        }
    }

    if( !blob || k >= AIA_BLOB_STORAGE_KEY_MAX )
    {
        AiaLogError( "Invalid input: blob(%u) key(%s)", blob, key);
    }

    return process;
}

bool AiaLoadBlob( const char* key, uint8_t* const blob, size_t size )
{
    bool process = false;
    blobstorage_key_e k = AIA_BLOB_STORAGE_KEY_MAX;

    for( k = AIA_BLOB_STORAGE_KEY_START; blob && k < AIA_BLOB_STORAGE_KEY_MAX; ++k )
    {
        if( strncmp(key, blobstorage[ k ].key, strlen(key)) == 0 )
        {
            if( blobstorage[ k ].used_len <= size )
            {
                memcpy( blob, blobstorage[ k ].storage, blobstorage[k].used_len );
                process = true;
            }
            else
            {
                AiaLogError( "blob load size error: key(%s), used(%u), size(%u)", key, blobstorage[k].used_len, size );
            }

            break;
        }
    }

    if( !blob || k >= AIA_BLOB_STORAGE_KEY_MAX )
    {
        AiaLogError( "Invalid input: blob(%u) key(%s)", blob, key);
    }

    return process;
}

bool AiaBlobExists( const char* key )
{
	bool existed = false;
    blobstorage_key_e k = AIA_BLOB_STORAGE_KEY_MAX;

    for( k = AIA_BLOB_STORAGE_KEY_START;k < AIA_BLOB_STORAGE_KEY_MAX; ++k )
    {
        if( strncmp(key, blobstorage[ k ].key, strlen(key)) == 0 )
        {
            existed = true;
            break;
        }
    }

    return existed;
}

size_t AiaGetBlobSize( const char* key )
{
    size_t blob_size = 0;
    blobstorage_key_e k = AIA_BLOB_STORAGE_KEY_MAX;

    for( k = AIA_BLOB_STORAGE_KEY_START; k < AIA_BLOB_STORAGE_KEY_MAX; ++k )
    {
        if( strncmp( key, blobstorage[ k ].key, strlen(key) ) == 0 )
        {
            blob_size = blobstorage[ k ].used_len;
            break;
        }
    }

    if( k >= AIA_BLOB_STORAGE_KEY_MAX )
    {
        AiaLogError( "blob key error: %s", key );
    }

    return blob_size;
}

bool AiaStoreAlert( const char* alertToken, size_t alertTokenLen,
                    AiaTimepointSeconds_t scheduledTime,
                    AiaDurationMs_t duration, uint8_t alertType )
{
    if( !alertToken )
    {
        AiaLogError( "Null alertToken" );
        return false;
    }
    if( alertTokenLen != AIA_ALERT_TOKEN_CHARS )
    {
        AiaLogError( "Invalid alert token length" );
        return false;
    }

    size_t allAlertsBytes = AiaGetAlertsSize();
    size_t alertsBytesWithNewAlert =
        allAlertsBytes + AIA_SIZE_OF_ALERT_IN_BYTES;
    size_t startingOffset = allAlertsBytes;
    size_t bytePosition;
    bool updatingExistingAlert = false;

    /**
     * Load all alerts from persistent storage. Allocated additional space
     * for a new alert though in case the token we are trying to insert does
     * not exist in persistent storage yet.
     */
    uint8_t* allAlertsBuffer = AiaCalloc( 1, alertsBytesWithNewAlert );
    if( !allAlertsBuffer )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu.", alertsBytesWithNewAlert );
        return false;
    }

    if( !AiaLoadAlerts( allAlertsBuffer, allAlertsBytes ) )
    {
        AiaLogError( "AiaLoadBlob failed" );
        AiaFree( allAlertsBuffer );
        return false;
    }

    /** Go through the tokens to find the first empty or matching one */
    for( bytePosition = 0; bytePosition < alertsBytesWithNewAlert;
         bytePosition += AIA_SIZE_OF_ALERT_IN_BYTES )
    {
        startingOffset = bytePosition;
        if( '\0' == allAlertsBuffer[ bytePosition ] )
        {
            /** Found an empty token */
            break;
        }
        else
        {
            /** Check if this token matches with what we are trying to insert */
            if( !strncmp( (const char*)allAlertsBuffer + bytePosition,
                          alertToken, alertTokenLen ) )
            {
                updatingExistingAlert = true;
                break;
            }
        }
    }

    /* Check if we have reached the alerts storage limit */
    if( startingOffset == alertsBytesWithNewAlert )
    {
        AiaLogError(
            "AiaStoreAlert failed: Maximum number of local alerts to store "
            "reached." );
        AiaFree( allAlertsBuffer );
        return false;
    }

    /** Write the new alert token */
    uint8_t* newAlertOffset = allAlertsBuffer + startingOffset;
    memcpy( newAlertOffset, alertToken, alertTokenLen );

    /** Write the other fields: scheduledTime, duration, alertType */
    bytePosition = alertTokenLen;
    for( size_t i = 0; i < sizeof( AiaTimepointSeconds_t );
         ++i, bytePosition++ )
    {
        newAlertOffset[ bytePosition ] = ( scheduledTime >> ( i * 8 ) );
    }
    for( size_t i = 0; i < sizeof( AiaDurationMs_t ); ++i, bytePosition++ )
    {
        newAlertOffset[ bytePosition ] = ( duration >> ( i * 8 ) );
    }
    for( size_t i = 0; i < sizeof( uint8_t ); ++i, bytePosition++ )
    {
        newAlertOffset[ bytePosition ] = ( alertType >> ( i * 8 ) );
    }

    /** Store the new blob in persistent storage */
    size_t storeSize =
        ( updatingExistingAlert ? allAlertsBytes : alertsBytesWithNewAlert );
    /** Store the new blob in persistent storage */
    if( !AiaStoreBlob( AIA_ALL_ALERTS_STORAGE_KEY_V0, allAlertsBuffer,
                       storeSize ) )
    {
        AiaLogError( "AiaStoreBlob failed" );
        AiaFree( allAlertsBuffer );
        return false;
    }

    AiaFree( allAlertsBuffer );
    return true;
}

bool AiaDeleteAlert( const char* alertToken, size_t alertTokenLen )
{
    if( !alertToken )
    {
        AiaLogError( "Null alertToken" );
        return false;
    }
    if( alertTokenLen != AIA_ALERT_TOKEN_CHARS )
    {
        AiaLogError( "Invalid alert token length" );
        return false;
    }

    size_t allAlertsBytes = AiaGetAlertsSize();
    size_t bytePosition;
    bool deletingExistingAlert = false;

    /**
     * Load all alerts from persistent storage.
     */
    uint8_t* allAlertsBuffer = AiaCalloc( 1, allAlertsBytes );
    if( !allAlertsBuffer )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu.", allAlertsBytes );
        return false;
    }

    if( !AiaLoadAlerts( allAlertsBuffer, allAlertsBytes ) )
    {
        AiaLogError( "AiaLoadBlob failed" );
        AiaFree( allAlertsBuffer );
        return false;
    }

    /** Go through the tokens to find the first empty or matching one */
    for( bytePosition = 0; bytePosition < allAlertsBytes;
         bytePosition += AIA_SIZE_OF_ALERT_IN_BYTES )
    {
        if( '\0' == allAlertsBuffer[ bytePosition ] )
        {
            /** Found an empty token */
            break;
        }
        else
        {
            /** Check if this token matches with what we are trying to delete */
            if( !strncmp( (const char*)allAlertsBuffer + bytePosition,
                          alertToken, alertTokenLen ) )
            {
                uint8_t* moveDst = allAlertsBuffer + bytePosition;
                uint8_t* moveSrc =
                    allAlertsBuffer + bytePosition + AIA_SIZE_OF_ALERT_IN_BYTES;
                size_t moveBytes =
                    allAlertsBytes -
                    ( bytePosition + AIA_SIZE_OF_ALERT_IN_BYTES );
                memmove( moveDst, moveSrc, moveBytes );
                deletingExistingAlert = true;
                break;
            }
        }
    }

    /** Store the new blob in persistent storage */
    size_t storeSize =
        ( deletingExistingAlert ? allAlertsBytes - AIA_SIZE_OF_ALERT_IN_BYTES
                                : allAlertsBytes );
    /** Store the new blob */
    if( !AiaStoreBlob( AIA_ALL_ALERTS_STORAGE_KEY_V0, allAlertsBuffer,
                       storeSize ) )
    {
        AiaLogError( "AiaStoreBlob failed" );
        AiaFree( allAlertsBuffer );
        return false;
    }

    AiaFree( allAlertsBuffer );
    return true;
}

bool AiaLoadAlert( char* alertToken, size_t alertTokenLen,
                   AiaTimepointSeconds_t* scheduledTime,
                   AiaDurationMs_t* duration, uint8_t* alertType,
                   const uint8_t* allAlertsBuffer )
{
    if( !alertToken )
    {
        AiaLogError( "Null alertToken" );
        return false;
    }
    if( !scheduledTime )
    {
        AiaLogError( "Null scheduledTime" );
        return false;
    }
    if( !duration )
    {
        AiaLogError( "Null duration" );
        return false;
    }
    if( !alertType )
    {
        AiaLogError( "Null alertType" );
        return false;
    }
    if( !allAlertsBuffer )
    {
        AiaLogError( "Null allAlertsBuffer" );
        return false;
    }
    if( alertTokenLen != AIA_ALERT_TOKEN_CHARS )
    {
        AiaLogError( "Invalid alert token length" );
        return false;
    }

    size_t bytePosition = 0;
    *scheduledTime = 0;
    *duration = 0;
    *alertType = 0;

    memcpy( alertToken, allAlertsBuffer, alertTokenLen );
    bytePosition += alertTokenLen;

    for( size_t i = 0; i < sizeof( AiaTimepointSeconds_t );
         ++i, ++bytePosition )
    {
        *scheduledTime |= (unsigned)allAlertsBuffer[ bytePosition ]
                          << ( i * 8 );
    }

    for( size_t i = 0; i < sizeof( AiaDurationMs_t ); ++i, ++bytePosition )
    {
        *duration |= (unsigned)allAlertsBuffer[ bytePosition ] << ( i * 8 );
    }

    for( size_t i = 0; i < sizeof( uint8_t ); ++i, ++bytePosition )
    {
        *alertType |= (unsigned)allAlertsBuffer[ bytePosition ] << ( i * 8 );
    }

    return true;
}

bool AiaLoadAlerts( uint8_t* allAlerts, size_t size )
{
    if( !AiaAlertsBlobExists() )
    {
        if( size != 0 )
        {
            AiaLogError( "Alerts blob with size %zu does not exist", size );
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return AiaLoadBlob( AIA_ALL_ALERTS_STORAGE_KEY_V0, allAlerts, size );
    }
}

size_t AiaGetAlertsSize()
{
    return AiaGetBlobSize( AIA_ALL_ALERTS_STORAGE_KEY_V0 );
}

bool AiaAlertsBlobExists()
{
    return AiaBlobExists( AIA_ALL_ALERTS_STORAGE_KEY_V0 );
}
