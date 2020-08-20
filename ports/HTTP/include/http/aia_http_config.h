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

#ifndef AIA_HTTP_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_HTTP_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>
#include "iot_config.h"
#include "platform/iot_network.h"

/**
 * @name Sending HTTPS requests to a URL using HTTP/1.1. AIA only requires
 * sending POST HTTPS messages for registration with support for redirects. */
/** @{ */

/**
 * HTTP methods from RFC2616. AIA only requires sending POST HTTPS messages for
 * registration.
 */
typedef enum AiaHttpsMethod
{
    AIA_HTTPS_METHOD_POST
} AiaHttpsMethod_t;

/* Contain information needed to make a HTTPS request. */
typedef struct AiaHttpsRequest
{
    /** The HTTP method to use in the request. */
    AiaHttpsMethod_t method;

    /** Array of C-strings to be sent as headers in the request. */
    const char** headers;

    /** Size of @c headers. */
    size_t headersLen;

    /** The URL to use in the request, as a C-string.*/
    const char* url;

    /** Data to send as the body of the request to the server, as a C-string. */
    const char* body;
} AiaHttpsRequest_t;

/* Contains HTTPS response information */
typedef struct AiaHttpsResponse
{
    /** The response code received. */
    size_t status;

    /** The body of the response. */
    char* body;

    /** Length of @c body. */
    size_t bodyLen;
} AiaHttpsResponse_t;

/**
 * This callback function is used when a failure is encountered in making the
 * request.
 * @note Implementations are not required to be thread-safe.
 *
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 */
typedef void ( *AiaHttpsConnectionFailureCallback_t )( void* userData );

/**
 * This callback function is used after a response is received from the server
 * following a request.
 * @note Implementations are not required to be thread-safe.
 *
 * @param httpsResponse The HTTPS response received from the server.
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 */
typedef void ( *AiaHttpsConnectionResponseCallback_t )(
    AiaHttpsResponse_t* httpsResponse, void* userData );

/**
 * Store AFR network information for later use.
 *
 * @param pNetworkInterface AFR demo network interfaces.
 * @param pNetworkCredentialInfo AFR demo network Credential
 * @return @c true if the request was able to be performed successfully or @c
 * false otherwise.
 */
bool AiaHttpStoreNetworkInfo(const IotNetworkInterface_t* pNetworkInterface,
                             const void* pNetworkCredentialInfo );

/**
 * Sends a HTTPS request. Implementation must be using HTTP/1.1 and follow
 * redirects.
 * @note A callback to @c responseCallback or @c failureCallback will only be
 * made if @c true is returned.
 * @note Implementations are not required to be thread-safe.
 *
 * @param httpsRequest Information used for sending the HTTPS request.
 * @param responseCallback A callback for when a response is received from the
 * server.
 * @param responseCallbackUserData User data to pass to @c responseCallback.
 * @param failureCallback A callback for when a failure in encountered making
 * the request.
 * @param failureCallbackUserData User data to pass to @c failureCallback.
 * @return @c true if the request was able to be performed successfully or @c
 * false otherwise.
 */
bool AiaSendHttpsRequest( AiaHttpsRequest_t* httpsRequest,
                          AiaHttpsConnectionResponseCallback_t responseCallback,
                          void* responseCallbackUserData,
                          AiaHttpsConnectionFailureCallback_t failureCallback,
                          void* failureCallbackUserData );
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_HTTP_CONFIG_H_ */
