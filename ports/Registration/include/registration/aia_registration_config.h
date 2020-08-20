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

#ifndef AIA_REGISTRATION_CONFIG_H_
#define AIA_REGISTRATION_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>

#define AIA_REGISTRATION_ENDPOINT \
    "https://api.amazonalexa.com/v1/ais/registration"

/**
 * Retrieves the IoT Client Id. The IoT Client Id returned is
 * null-terminated. If @c NULL is passed for @c iotClientId, this function
 * will store the buffer length needed to store the IoT Client Id into @c
 * len and return true if successful.
 *
 * @param[out] iotClientId A user provided buffer.
 * @param[in, out] len Pointer to the length of @c iotClientId. If IoT
 * Client Id is successfully retrieved this will be set to the length of the
 * token that will be copied into @c buffer. This includes space for
 * a trailing @c '\0'.
 * @return @c true if entire Iot Client Id is copied to the @c iotClientId,
 * else @c false.
 */
bool AiaGetIotClientId( char* iotClientId, size_t* len );

/**
 * Retrieves the AWS Account Id. The AWS Account Id returned is
 * null-terminated. If @c NULL is passed for @c awsAccountId, this function
 * will store the buffer length needed to store the AWS Account Id into @c
 * len and return true if successful.
 *
 * @param[out] awsAccountId A user provided buffer.
 * @param[in, out] len Pointer to the length of @c awsAccountId. If AWS
 * Account Id is successfully retrieved this will be set to the length of
 * the token that will be copied into @c buffer. This includes space for a
 * trailing @c '\0'.
 * @return @c true if entire AWS Account Id is copied into @c awsAccountId,
 * else @c false.
 */
bool AiaGetAwsAccountId( char* awsAccountId, size_t* len );

/**
 * Retrieves the IoT endpoint. The IoT endpoint returned is
 * null-terminated. If @c NULL is passed for @c iotEndpoint, this function
 * will store the buffer length needed to store the IoT endpoint into @c
 * len and return true if successful.
 *
 * @param[out] iotEndpoint A user provided buffer.
 * @param[in, out] len Pointer to the length of @c iotEndpoint. If IoT
 * endpoint is successfully retrieved this will be set to the length of
 * the token that will be copied into @c buffer. This includes space for a
 * trailing @c '\0'.
 * @return @c true if entire IoT endpoint is copied into @c iotEndpoint,
 * else @c false.
 */
bool AiaGetIotEndpoint( char* iotEndpoint, size_t* len );

#endif /* ifndef AIA_REGISTRATION_CONFIG_H_ */
