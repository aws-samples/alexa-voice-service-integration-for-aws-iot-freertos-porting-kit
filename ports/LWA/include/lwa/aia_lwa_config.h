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

#ifndef AIA_LWA_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_LWA_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * Retrieves the LWA refresh token. The LWA refresh token returned is
 * null-terminated. If @c NULL is passed for @c refreshToken, this function will
 * store the buffer length needed to store the LWA refresh token into @c len and
 * return true if successful.
 *
 * @param[out] refreshToken A user provided buffer.
 * @param[in, out] len Pointer to the length of @c refreshToken. If LWA refresh
 * token is successfully retrieved this will be set to the length of the token
 * that will be copied into @c buffer. This includes space for a
 * trailing @c '\0'.
 * @return @c true if successful or @c false otherwise.
 */
bool AiaGetRefreshToken( char* refreshToken, size_t* len );

/**
 * Retrieves the LWA Client Id. The LWA Client Id returned is
 * null-terminated. If @c NULL is passed for @c lwaClientId, this function
 * will store the buffer length needed to store the LWA Client Id into @c
 * len and return true if successful.
 *
 * @param[out] lwaClientId A user provided buffer.
 * @param[in, out] len Pointer to the length of @c lwaClientId. If LWA
 * Client Id is successfully retrieved this will be set to the length of the
 * token that will be copied into @c buffer. This includes space for
 * a trailing @c '\0'.
 * @return @c true if entire LWA Client Id is copied to the @c lwaClientId,
 * else @c false.
 */
bool AiaGetLwaClientId( char* lwaClientId, size_t* len );

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_LWA_CONFIG_H_ */
