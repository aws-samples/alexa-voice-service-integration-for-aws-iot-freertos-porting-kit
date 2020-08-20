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
 * @file aia_http_config.c
 * @brief Implements platform-specific Crypto functions which are not inlined in
 * @c aia_http_config.h.
 */

#include <aia_config.h>
#include <http/aia_http_config.h>
#include "iot_https_client.h"
#include "iot_https_utils.h"

#ifndef AIA_AFR_HTTPS_TRUSTED_ROOT_CA
#define AIA_AFR_HTTPS_TRUSTED_ROOT_CA                                    \
    "-----BEGIN CERTIFICATE-----\n"                                      \
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n" \
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n" \
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n" \
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n" \
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n" \
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n" \
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n" \
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n" \
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n" \
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n" \
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n" \
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n" \
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n" \
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n" \
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n" \
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n" \
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n" \
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"                                     \
    "-----END CERTIFICATE-----\n"
#endif /* ifndef AIA_AFR_HTTPS_TRUSTED_ROOT_CA */

#define AIA_AFR_HTTPS_BUFFER_SIZE ( (int)384 )
static const IotNetworkInterface_t* _pAiaNetIf;
static const void* _pAiaNetCredentialInfo;
static char* _reqHeader = "Content-Type";
static char* _reqHeaderVal = "application/json";

bool AiaHttpStoreNetworkInfo( const IotNetworkInterface_t* pNetworkInterface,
                              const void* pNetworkCredentialInfo )
{
    if( pNetworkInterface && pNetworkCredentialInfo )
    {
        _pAiaNetIf = pNetworkInterface;
        _pAiaNetCredentialInfo = pNetworkCredentialInfo;
        return true;
    }
    else
    {
        return false;
    }
}

bool AiaSendHttpsRequest( AiaHttpsRequest_t* httpsRequest,
                          AiaHttpsConnectionResponseCallback_t responseCallback,
                          void* responseCallbackUserData,
                          AiaHttpsConnectionFailureCallback_t failureCallback,
                          void* failureCallbackUserData )
{
    IotHttpsReturnCode_t httpsClientStatus = IOT_HTTPS_OK;
    IotHttpsConnectionInfo_t connConfig = { 0 };
    IotHttpsRequestInfo_t reqConfig = { 0 };
    IotHttpsResponseInfo_t respConfig = { 0 };
    IotHttpsConnectionHandle_t connHandle =
        IOT_HTTPS_CONNECTION_HANDLE_INITIALIZER;
    IotHttpsRequestHandle_t reqHandle = IOT_HTTPS_REQUEST_HANDLE_INITIALIZER;
    IotHttpsResponseHandle_t respHandle = IOT_HTTPS_RESPONSE_HANDLE_INITIALIZER;
    IotHttpsSyncInfo_t reqSyncInfo = { 0 };
    IotHttpsSyncInfo_t respSyncInfo = { 0 };
    const char* pPath = NULL;
    size_t pathLen = 0;
    const char* pAddress = NULL;
    size_t addressLen = 0;
    uint16_t respStatus = IOT_HTTPS_STATUS_OK;
    uint32_t retryNum = 0;
    size_t rspBodyLen = 0;
    uint8_t _pConnUserBuffer[ AIA_AFR_HTTPS_BUFFER_SIZE ] = { 0 };
    uint8_t _pReqUserBuffer[ AIA_AFR_HTTPS_BUFFER_SIZE ] = { 0 };
    uint8_t _pRespUserBuffer[ AIA_AFR_HTTPS_BUFFER_SIZE ] = { 0 };
    uint8_t _pRespBodyBuffer[ AIA_AFR_HTTPS_BUFFER_SIZE ] = { 0 };

    httpsClientStatus = IotHttpsClient_GetUrlPath(
        httpsRequest->url, strlen( httpsRequest->url ), &pPath, &pathLen );
    if( httpsClientStatus != IOT_HTTPS_OK )
    {
        AiaLogError(
            "An error occurred in IotHttpsClient_GetUrlPath() on URL %s. Error "
            "code: %d",
            httpsRequest->url, httpsClientStatus );
        return false;
    }
    httpsClientStatus = IotHttpsClient_GetUrlAddress(
        httpsRequest->url, strlen( httpsRequest->url ), &pAddress,
        &addressLen );
    if( httpsClientStatus != IOT_HTTPS_OK )
    {
        AiaLogError(
            "An error occurred in IotHttpsClient_GetUrlAddress() on URL "
            "%s\r\n. Error code %d",
            httpsRequest->url, httpsClientStatus );
        return false;
    }
    if( httpsRequest->method != AIA_HTTPS_METHOD_POST )
    {
        AiaLogError( "Unsupported AiaHttpsMethod_t, method=%d",
                     httpsRequest->method );
        return false;
    }
    httpsClientStatus = IotHttpsClient_Init();
    if( httpsClientStatus != IOT_HTTPS_OK )
    {
        AiaLogError(
            "An error occurred initializing the HTTPS library. Error code: %d",
            httpsClientStatus );
        return false;
    }

    connConfig.pAddress = pAddress;
    connConfig.addressLen = addressLen;
    connConfig.port = 443;
    connConfig.pCaCert = AIA_AFR_HTTPS_TRUSTED_ROOT_CA;
    connConfig.caCertLen = sizeof( AIA_AFR_HTTPS_TRUSTED_ROOT_CA );
    connConfig.userBuffer.pBuffer = _pConnUserBuffer;
    connConfig.userBuffer.bufferLen = sizeof( _pConnUserBuffer );
    connConfig.pClientCert =
        ( (IotNetworkCredentials_t*)_pAiaNetCredentialInfo )->pClientCert;
    connConfig.clientCertLen =
        ( (IotNetworkCredentials_t*)_pAiaNetCredentialInfo )->clientCertSize;
    connConfig.pPrivateKey =
        ( (IotNetworkCredentials_t*)_pAiaNetCredentialInfo )->pPrivateKey;
    connConfig.privateKeyLen =
        ( (IotNetworkCredentials_t*)_pAiaNetCredentialInfo )->privateKeySize;
    connConfig.pNetworkInterface = _pAiaNetIf;

    reqSyncInfo.pBody = (uint8_t*)( httpsRequest->body );
    reqSyncInfo.bodyLen = strlen( httpsRequest->body );
    respSyncInfo.pBody = (uint8_t*)_pRespBodyBuffer;
    respSyncInfo.bodyLen = sizeof( _pRespBodyBuffer );

    reqConfig.pPath = pPath;
    reqConfig.pathLen = strlen( pPath );
    reqConfig.pHost = pAddress;
    reqConfig.hostLen = addressLen;
    reqConfig.method = IOT_HTTPS_METHOD_POST;
    reqConfig.isNonPersistent = false;
    reqConfig.userBuffer.pBuffer = _pReqUserBuffer;
    reqConfig.userBuffer.bufferLen = sizeof( _pReqUserBuffer );
    reqConfig.isAsync = false;
    reqConfig.u.pSyncInfo = &reqSyncInfo;

    respConfig.userBuffer.pBuffer = _pRespUserBuffer;
    respConfig.userBuffer.bufferLen = sizeof( _pRespUserBuffer );
    respConfig.pSyncInfo = &respSyncInfo;

    httpsClientStatus =
        IotHttpsClient_InitializeRequest( &reqHandle, &reqConfig );
    if( httpsClientStatus != IOT_HTTPS_OK )
    {
        AiaLogError(
            "An error occurred in IotHttpsClient_InitializeRequest() with "
            "error code: %d",
            httpsClientStatus );
        IotHttpsClient_Cleanup();
        return false;
    }

    httpsClientStatus = IotHttpsClient_Connect( &connHandle, &connConfig );
    if( httpsClientStatus != IOT_HTTPS_OK )
    {
        AiaLogError( "Failed to connect to the server. Error code: %d.",
                     httpsClientStatus );
        IotHttpsClient_Cleanup();
        return false;
    }

    httpsClientStatus =
        IotHttpsClient_AddHeader( reqHandle, _reqHeader, strlen( _reqHeader ),
                                  _reqHeaderVal, strlen( _reqHeaderVal ) );
    if( httpsClientStatus == IOT_HTTPS_NETWORK_ERROR )
    {
        AiaLogError( "Failed to add header. Error code: %d.",
                     httpsClientStatus );
        IotHttpsClient_Disconnect( connHandle );
        IotHttpsClient_Cleanup();
        return false;
    }

    httpsClientStatus = IotHttpsClient_SendSync( connHandle, reqHandle,
                                                 &respHandle, &respConfig, 0 );
    if( httpsClientStatus != IOT_HTTPS_NETWORK_ERROR )
    {
        httpsClientStatus =
            IotHttpsClient_ReadResponseStatus( respHandle, &respStatus );
        if( httpsClientStatus != IOT_HTTPS_OK )
        {
            AiaLogError(
                "Error in retreiving the response status. Error code %d",
                httpsClientStatus );
            failureCallback( failureCallbackUserData );
        }
        else if( respStatus != IOT_HTTPS_STATUS_OK )
        {
            AiaLogError( "Failed to register to AIS. Response status: %d",
                         respStatus );
            failureCallback( failureCallbackUserData );
        }
        else
        {
            AiaHttpsResponse_t response;
            httpsClientStatus =
                IotHttpsClient_ReadContentLength( respHandle, &rspBodyLen );

            if( httpsClientStatus == IOT_HTTPS_OK )
            {
                response.body = _pRespBodyBuffer;
                response.bodyLen = rspBodyLen;
                response.status = respStatus;
                responseCallback( &response, responseCallbackUserData );
                AiaLogInfo( "AIS registration success." );
            }
            else
            {
                AiaLogError(
                    "Failed to read the Content-Length from the response. "
                    "Error code %d",
                    httpsClientStatus );
            }
        }
    }
    else
    {
        AiaLogError( "Failed to SendSync to the server. Error code: %d.",
                     httpsClientStatus );
    }

    if( connHandle != NULL )
    {
        IotHttpsClient_Disconnect( connHandle );
    }

    IotHttpsClient_Cleanup();

    return true;
}
