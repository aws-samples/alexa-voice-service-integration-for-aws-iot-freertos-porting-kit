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
 * @file aia_lwa_config.c
 * @brief Implements platform-specific LWA functions which are not inlined
 * in
 * @c aia_lwa_config.h.
 */

#include <iot/aia_iot_config.h>
#include <lwa/aia_lwa_config.h>

char* g_aiaLwaRefreshToken;
char* g_aiaLwaClientId;

bool AiaGetRefreshToken( char* refreshToken, size_t* len )
{
    if( !len )
    {
        AiaLogError( "Null len." );
        return false;
    }
    size_t refreshTokenLen = strlen( g_aiaLwaRefreshToken ) + 1;
    if( refreshToken )
    {
        /* TODO: ADSER-1938 Implement LWA calls. */
        if( *len < refreshTokenLen )
        {
            AiaLogError(
                "refreshToken buffer too small to hold refresh token." );
            return false;
        }
        strncpy( refreshToken, g_aiaLwaRefreshToken, refreshTokenLen );
    }
    *len = refreshTokenLen;
    return true;
}

bool AiaGetLwaClientId( char* lwaClientId, size_t* len )
{
    if( !len )
    {
        AiaLogError( "Null len." );
        return false;
    }
    size_t lwaClientIdLen = strlen( g_aiaLwaClientId ) + 1;
    if( lwaClientId )
    {
        /* TODO: ADSER-1938 Implement LWA calls. */
        if( *len < lwaClientIdLen )
        {
            AiaLogError(
                "lwaClientId buffer too small to hold LWA Client Id." );
            return false;
        }
        strncpy( lwaClientId, g_aiaLwaClientId, lwaClientIdLen );
    }
    *len = lwaClientIdLen;
    return true;
}
