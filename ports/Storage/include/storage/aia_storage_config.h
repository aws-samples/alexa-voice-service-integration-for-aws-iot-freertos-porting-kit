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

#ifndef AIA_STORAGE_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_STORAGE_CONFIG_H_

#include <clock/aia_clock_config.h>

#include <stdbool.h>
#include <stddef.h>

/**
 * @name Retrieval and persistent storage of generic blobs as key-value pairs.
 * Implementations are not required to be thread-safe. These functions are
 * required to be implemented. Implementations should be defined in @file
 * aia_storage_config.c
 */
/** @{ */

/**
 * Persists the given blob.
 *
 * @param key Null-terminated string to use as the key for this blob.
 * @param blob The buffer to persist.
 * @param size Size of @c blob.
 * @return @c true on success or @c false otherwise.
 */
bool AiaStoreBlob( const char* key, const uint8_t* blob, size_t size );

/**
 * Loads blob out of persistent storage.
 *
 * @param key Null-terminated string representing the key for the blob to load.
 * @param[out] blob The buffer to load.
 * @param size Size of @c blob.
 * @return @c true on success or @c false otherwise.
 */
bool AiaLoadBlob( const char* key, uint8_t* const blob, size_t size );

/**
 * Gets the size of the blob in persistent storage.
 *
 * @param key Null-terminated string to use as the key for the blob.
 * @return the length of data in bytes. @c 0 will be returned on failures.
 */
size_t AiaGetBlobSize( const char* key );

/**
 * Checks if a blob exists in persistent storage.
 *
 * @param key Null-terminated string to use as the key for the blob.
 * @return @c true if the blob exists of @c false otherwise.
 */
bool AiaBlobExists( const char* key );

/** @} */

/**
 * @name Retrieval and persistent storage of shared secret used for end-to-end
 * encryption. Storage will be used during registration and secret rotation and
 * loading will be used during AIA SDK startup. Implementations are not required
 * to be thread-safe. These functions are required to be implemented.
 * Implementations should be defined in @file aia_config.c
 */
/** @{ */

/**
 * Persists the given secret.
 *
 * @param sharedSecret The buffer to persist.
 * @param size Size of @c sharedSecret. Note that this should be @c
 * AiaEncryptionAlgorithm_GetKeySize() of the encryption algorithm defined in
 * @file aia_registration_config.h.
 * @return @c true on success or @c false otherwise.
 */
bool AiaStoreSecret( const uint8_t* sharedSecret, size_t size );

/**
 * Loads shared secret out of persistent storage.
 *
 * @param[out] sharedSecret The buffer to load into.
 * @param size Size of the @c sharedSecret buffer.
 * @return @c true on success or @c false otherwise.
 */
bool AiaLoadSecret( uint8_t* sharedSecret, size_t size );

/** @} */

/**
 * @name Retrieval of local volume. Implementations are not required to
 * be thread-safe. These functions are optional and only required if the device
 * persists volume locally or allows for local offline changes to volume. AIA
 * will use a cached server-side volume if @c AIA_LOAD_VOLUME is not defined. If
 * not desired, delete the below portion.
 */
/** @{ */

/**
 * This function is used to load persisted volume for use during
 * synchronizing of device state upon a fresh AIA connection.
 *
 * @return @c The loaded volume. Any failure shall return @c AIA_DEFAULT_VOLUME.
 */
uint8_t AiaLoadVolume();

#define AIA_LOAD_VOLUME AiaLoadVolume

/** @} */

/**
 * @name Retrieval and persistent storage of AIA alerts (timers, reminders,
 * alarms). Storage will be used when an alert is created, updated or deleted.
 * Loading will be used before adding, updating or deleting an alert or during
 * AIA SDK startup. Implementations are not required to be thread-safe. These
 * functions are required to be implemented. Implementations should be defined
 * in @file aia_config.c.
 */
/** @{ */

/**
 * Removes a given alert from persistent storage.
 *
 * @param alertToken The alert token to remove not including the null
 * terminator.
 * @param alertTokenLen Length of @c alertToken not including the null
 * terminator.
 * @note @c alertTokenLen is expected to be equal to @c AIA_ALERT_TOKEN_CHARS
 * for persistent storage consistency.
 * @return @c true on success of @c false otherwise.
 */
bool AiaDeleteAlert( const char* alertToken, size_t alertTokenLen );

/**
 * Serializes the given alert fields and persists the serialized blob.
 *
 * @param alertToken The alert token to persist not including the null
 * terminator.
 * @param alertTokenLen Length of @c alertToken not including the null
 * terminator.
 * @param scheduledTime The alert scheduled time to persist.
 * @param duration The alert duration to persist.
 * @param alertType The alert type to persist.
 * @return @c true on success or @c false otherwise.
 *
 * @note The format the alerts tokens are stored in the persistent storage (i.e.
 * little endian byte ordering) needs to match with the format the alerts are
 * retrieved from the persistent storage (@c AiaLoadAlert()).
 * @note @c alertTokenLen is expected to be equal to @c AIA_ALERT_TOKEN_CHARS
 * for persistent storage consistency.
 */
bool AiaStoreAlert( const char* alertToken, size_t alertTokenLen,
                    AiaTimepointSeconds_t scheduledTime,
                    AiaDurationMs_t duration, uint8_t alertType );

/**
 * Deserializes the alert fields starting from the given buffer location.
 *
 * @param[out] alertToken Pointer to the buffer to hold the alert token not
 * including the null terminator.
 * @param alertTokenLen Length of @c alertToken not including the null
 * terminator.
 * @param[out] scheduledTime Pointer to the buffer to hold the scheduled time.
 * @param[out] duration Pointer to the buffer to hold the duration.
 * @param[out] alertType Pointer to the buffer to hold the alertType.
 * @param allAlertsBuffer Pointer to the buffer to start reading alert
 * information from.
 * @return @c true on success or @c false otherwise.
 *
 * @note The format the alerts tokens are loaded from the persistent storage
 * (i.e. little endian byte ordering) needs to match with the format the alerts
 * are stored in the persistent storage (@c AiaStoreAlert()).
 * @note The @c allAlertsBuffer parameter points to a section of the alerts blob
 * loaded via the @c AiaLoadAlerts() function which splits the alerts blob into
 * sections of @c AIA_SIZE_OF_ALERT_IN_BYTES bytes.
 * @note @c alertTokenLen is expected to be equal to @c AIA_ALERT_TOKEN_CHARS
 * for persistent storage consistency.
 */
bool AiaLoadAlert( char* alertToken, size_t alertTokenLen,
                   AiaTimepointSeconds_t* scheduledTime,
                   AiaDurationMs_t* duration, uint8_t* alertType,
                   const uint8_t* allAlertsBuffer );

/**
 * Loads alerts out of persistent storage if an alerts blob exists in persistent
 * storage.
 *
 * @param[out] allAlerts Buffer to hold the loaded alerts.
 * @param size Size of the @c allAlerts buffer.
 * @return @c true on success or if the alerts blob does not exist,
 * and @c false otherwise.
 */
bool AiaLoadAlerts( uint8_t* allAlerts, size_t size );

/**
 * Gets the size of the alerts blob in persistent storage.
 *
 * @return the length of alerts blob in bytes. @c 0 will be returned on
 * failures or if the alerts blob does not exist.
 */
size_t AiaGetAlertsSize();

/**
 * Checks if an alerts blob exists in the persistent storage or not.
 *
 * @return @c true if the blob exists or @c false otherwise.
 */
bool AiaAlertsBlobExists();

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_STORAGE_CONFIG_H_ */
