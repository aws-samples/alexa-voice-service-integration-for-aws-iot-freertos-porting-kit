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
 * @file aia_registration_config.c
 * @brief Implements platform-specific Registration functions which are not
 * inlined in
 * @c aia_registration_config.h.
 */

#include <registration/aia_registration_config.h>

#include <aia_config.h>

const char* g_aiaIotEndpoint;
const char* g_aiaClientId;
const char* g_aiaAwsAccountId;

bool AiaGetIotClientId( char* iotClientId, size_t* len )
{
    if( !len )
    {
        AiaLogError( "Null len." );
        return false;
    }

    size_t iotClientIdLen = strlen( g_aiaClientId ) + 1;
    if( !iotClientIdLen )
    {
        AiaLogError( "Failed to retrieve IoT Client Id length" );
        return false;
    }
    if( iotClientId )
    {
        if( *len < iotClientIdLen )
        {
            AiaLogError(
                "iotClientId buffer too small to hold IoT Client Id." );
            return false;
        }
        strncpy( iotClientId, g_aiaClientId, iotClientIdLen );
    }
    *len = iotClientIdLen;
    return true;
}

bool AiaGetAwsAccountId( char* awsAccountId, size_t* len )
{
    if( !len )
    {
        AiaLogError( "Null len." );
        return false;
    }

    size_t awsAccountIdLen = strlen( g_aiaAwsAccountId ) + 1;
    if( !awsAccountIdLen )
    {
        AiaLogError( "Failed to retrieve AWS Account Id length" );
        return false;
    }
    if( awsAccountId )
    {
        if( *len < awsAccountIdLen )
        {
            AiaLogError(
                "awsAccountId buffer too small to hold AWS Account Id." );
            return false;
        }
        strncpy( awsAccountId, g_aiaAwsAccountId, awsAccountIdLen );
    }
    *len = awsAccountIdLen;
    return true;
}

bool AiaGetIotEndpoint( char* iotEndpoint, size_t* len )
{
    if( !len )
    {
        AiaLogError( "Null len." );
        return false;
    }

    size_t iotEndpointLen = strlen( g_aiaIotEndpoint ) + 1;
    if( !iotEndpointLen )
    {
        AiaLogError( "Failed to retrieve IoT endpoint length" );
        return false;
    }
    if( iotEndpoint )
    {
        if( *len < iotEndpointLen )
        {
            AiaLogError( "iotEndpoint buffer too small to hold IoT endpoint." );
            return false;
        }
        strncpy( iotEndpoint, g_aiaIotEndpoint, iotEndpointLen );
    }
    *len = iotEndpointLen;
    return true;
}
