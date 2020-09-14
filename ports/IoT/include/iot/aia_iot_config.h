/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AIA_IOT_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_IOT_CONFIG_H_

#include <clock/aia_clock_config.h>

/* The IoT config header */
#include <iot_config.h>

/* IoT SDK headers */
#include <iot_json_utils.h>
#include <iot_atomic.h>
#include <iot_logging.h>
#include <iot_mqtt.h>
#include <platform/iot_threads.h>
#include <types/iot_mqtt_types.h>
#include <types/iot_taskpool_types.h>

#include <string.h>

/** Simple macro to stringify a parameter. */
#define AIA_STRINGIFY( x ) #x

/**
 * Second layer of macro to ensure @c AIA_CONFIG_STRINGIFY() is expanded at the
 * caller's location rather than here.
 */
#define AIA_TOSTRING( x ) AIA_STRINGIFY( x )

/**
 * Simple printf-style logging macros which automatically prepends the file
 * name and line number.
 */
/** @{ */
#define IOT_LOG_LEVEL_AIA IOT_LOG_INFO
#define AiaLog( Level, ... ) \
    IotLog_Generic( IOT_LOG_LEVEL_AIA, "AIA", Level, NULL, __VA_ARGS__ )
#define AiaLogDebug( ... ) AiaLog( IOT_LOG_DEBUG, __VA_ARGS__ )
#define AiaLogError( ... ) AiaLog( IOT_LOG_ERROR, __VA_ARGS__ )
#define AiaLogInfo( ... ) AiaLog( IOT_LOG_INFO, __VA_ARGS__ )
#define AiaLogWarn( ... ) AiaLog( IOT_LOG_WARN, __VA_ARGS__ )

#ifdef AIA_EMIT_SENSITIVE_LOGS

#define AIA_SENSITIVE_TAG "AIA_SENSITIVE"
#define AiaLogSensitive( ... )                                             \
    IotLog_Generic( IOT_LOG_DEBUG, AIA_SENSITIVE_TAG, IOT_LOG_DEBUG, NULL, \
                    __VA_ARGS__ )

#else
#define AiaLogSensitive( ... )

#endif
/** @} */

/** Macros and typedefs for task pool. */
/** @{ */
typedef IotTaskPoolInfo_t AiaTaskPoolInfo_t;
typedef IotTaskPoolError_t AiaTaskPoolError_t;
typedef IotTaskPoolJobStatus_t AiaTaskPoolJobStatus_t;
typedef IotTaskPoolJobStorage_t AiaTaskPoolJobStorage_t;
typedef IotTaskPoolJob_t AiaTaskPoolJob_t;
#define AiaTaskPool( MEMBER ) IotTaskPool_##MEMBER
#define AiaTaskPool_t AiaTaskPool( t )
#define IotTaskPool_HEADER <iot_taskpool.h>
#define IotTaskPool_INITIALIZER IOT_TASKPOOL_INITIALIZER
#define IotTaskPool_INFO_INITIALIZER IOT_TASKPOOL_INFO_INITIALIZER
#define IotTaskPool_JOB_STORAGE_INITIALIZER IOT_TASKPOOL_JOB_STORAGE_INITIALIZER
#define IotTaskPool_JOB_INITIALIZER IOT_TASKPOOL_JOB_INITIALIZER
/** @} */

/**
 * Determine if the AiaTaskPool return error is success.
 *
 * @param error The return error to evaluate.
 * @return @c true if @c error indicates success, else @c false.
 */
static inline bool AiaTaskPoolSucceeded( AiaTaskPoolError_t error )
{
    return error == IOT_TASKPOOL_SUCCESS;
}

/**
 * This function searches for the specified @c jsonkey in the json @c
 * jsonDocument string provided.  If found, @c jsonValue and @c
 * jsonValueLength are updated to indicate where the value for that key starts
 * and ends.
 * @note In a complex nested JSON @c jsonDocument, calling this function with
 * the @c jsonkey for a complex object will return the entire complex object as
 * the @c jsonValue.  This allows a recursive decent pattern where this
 * function can subsequently be called with the complex object that was found as
 * the new @c jsonDocument, allowing the application to search for a @c
 * jsonkey within that object.
 *
 * @param jsonDocument A buffer containing the JSON document text to search in
 * (does not need to be '\0' terminated).
 * @param jsonDocumentLength The length of @c jsonDocument (does not need to
 * include a terminating '\0').
 * @param jsonkey A buffer containing the key to search for (does not need to
 * be '\0' terminated).
 * @param jsonkeyLength The length of @c jsonkey (does not need to include a
 * terminating '\0').
 * @param jsonValue A pointer to a @c const @c char pointer that can be used to
 * return the location @ jsonkey's value starts at.
 * @param jsonValueLength A pointer to a @c size_t that can be used to return
 * the length of @c jsonkey's value (not including a terminating '\0').
 * @return @c true if @c jsonkey was found successfully, else @c false.
 * TODO: Handle arrays in AiaFindJsonValue (ADSER-1494)
 */
inline bool AiaFindJsonValue( const char* jsonDocument,
                              size_t jsonDocumentLength, const char* jsonKey,
                              size_t jsonKeyLength, const char** jsonValue,
                              size_t* jsonValueLength )
{
    return jsonDocument && jsonKey && jsonDocumentLength && jsonKeyLength &&
            IotJsonUtils_FindJsonValue( jsonDocument, jsonDocumentLength, jsonKey,
                                      jsonKeyLength, jsonValue,
                                      jsonValueLength );
}

/**
 * Implements an atomic load function by performing a bitwise OR operation with
 * all zeroes.
 * TODO: ADSER-1517 implement generic and compiler-specific version using actual
 * atomic load instructions
 */
static inline uint32_t AiaAtomic_Load_u32( uint32_t* operand )
{
    return Atomic_OR_u32( operand, 0 );
}

/**
 * Implements an atomic store operation by continuously performing a
 * compare-and-swap operation until the value is successfully written.
 * TODO: ADSER-1517 implement generic and compiler-specific version using actual
 * atomic store instructions
 */
static inline void AiaAtomic_Store_u32( uint32_t* operand, uint32_t val )
{
    while( !Atomic_CompareAndSwap_u32( operand, val, *operand ) )
    {
    }
}

/**
 * A function which performs an arithmetic atomic addition operation.
 * TODO: ADSER-1517 implement generic and compiler-specific version using actual
 * atomic add instructions
 *
 * @param[in,out] operand Pointer to the augend and where the sum is stored.
 * @param[in] addendum Value to add to the augend.
 *
 * @return The initial value at `operand`.
 */
static inline uint32_t AiaAtomic_Add_u32( uint32_t* operand, uint32_t addendum )
{
    return Atomic_Add_u32( operand, addendum );
}

/**
 * An unsigned, integral type used to represent indexes in the stream. Note that
 * @c AiaDataStreamIndex_t wrapping is not checked for,so this type must
 * be large enough to guarantee that no wrapping will occur over the practical
 * lifespan of the AiaDataStreamBuffer_t instance.
 */
typedef uint32_t AiaDataStreamIndex_t;

static const AiaDataStreamIndex_t AIA_DATA_STREAM_INDEX_MAX = UINT32_MAX;

/**
 * An atomic version of @c AiaDataStreamIndex_t. Note that this type is
 * not inherently atomic and must be accessed using the @c
 * AiaDataStreamAtomicIndex_Load() and @c
 * AiaDataStreamAtomicIndex_Store() functions.
 */
typedef AiaDataStreamIndex_t AiaDataStreamAtomicIndex_t;

/** A function used to atomically load the value of an @c
 * AiaDataStreamIndex_t. */
static inline AiaDataStreamIndex_t AiaDataStreamAtomicIndex_Load(
    AiaDataStreamIndex_t* operand )
{
    return AiaAtomic_Load_u32( operand );
}

/**
 * A function used to atomically store a value into an @c
 * AiaDataStreamIndex_t.
 */
static inline void AiaDataStreamAtomicIndex_Store(
    AiaDataStreamIndex_t* operand, AiaDataStreamIndex_t newValue )
{
    AiaAtomic_Store_u32( operand, newValue );
}

/**
 * A function which performs an arithmetic atomic addition operation.
 *
 * @param[in,out] operand Pointer to the augend and where the sum is stored.
 * @param[in] addendum Value to add to the augend.
 *
 * @return The initial value at `operand`.
 */
static inline AiaDataStreamIndex_t AiaDataStreamAtomicIndex_Add(
    AiaDataStreamIndex_t* operand, AiaDataStreamIndex_t addendum )
{
    return AiaAtomic_Add_u32( operand, addendum );
}

/**
 * An atomic boolean abstraction. Note that this type is not inherently atomic
 * and must be accessed using the @c AiaAtomicBool_Set, @c
 * AiaAtomicBool_Clear and @c AiaAtomicBool_Load functions.
 */
typedef uint32_t AiaAtomicBool_t;

/** A function used to set the value of @c AiaAtomicBool_t. */
static inline void AiaAtomicBool_Set( AiaAtomicBool_t* operand )
{
    AiaAtomic_Store_u32( operand, 1 );
}

/** A function used to clear the value of an @c AiaAtomicBool_t. */
static inline void AiaAtomicBool_Clear( AiaAtomicBool_t* operand )
{
    AiaAtomic_Store_u32( operand, 0 );
}

/** A function used to load the value of an @c AiaAtomicBool_t. */
static inline AiaAtomicBool_t AiaAtomicBool_Load( AiaAtomicBool_t* operand )
{
    return AiaAtomic_Load_u32( operand );
}

/** Macros for mutexes. */
/** @{ */
#define AiaMutex( MEMBER ) IotMutex_##MEMBER
#define IotMutex_HEADER <platform/iot_threads.h>
#define AiaMutex_t AiaMutex( t )
/** @} */

/** Macros for Semaphores. */
/** @{ */
#define AiaSemaphore( MEMBER ) IotSemaphore_##MEMBER
#define IotSemaphore_HEADER <platform/iot_threads.h>
#define AiaSemaphore_t AiaSemaphore( t )
/** @} */

/**
 * Macros for doubly-linked lists.
 *
 * Example Usage:
 * @code{c}
 * #include AiaListDouble( HEADER )
 *
 * void foo( void)
 * {
 *     AiaListDouble_t list;
 *     AiaListDouble( Link_t ) link = AiaListDouble( LINK_INITIALIZER );
 *     AiaListDouble( InsertTail )( &list, &link );
 *     AiaAssert(!AiaListDouble( IsEmpty )( &list );
 *     AiaListDouble( RemoveAll )( &list, NULL, 0 );
 * }
 * @endcode
 */
/** @{ */
#define AiaContainerConcat( TYPE, MEMBER ) TYPE##_##MEMBER
#define AiaContainer( TYPE, MEMBER ) AiaContainerConcat( TYPE, MEMBER )
#define IotListDouble_HEADER <iot_linear_containers.h>
#define AiaListDouble( MEMBER ) AiaContainer( IotListDouble, MEMBER )
#define AiaListDouble_t AiaListDouble( t )
#define IotListDouble_Link_t IotLink_t
#define IotListDouble_LINK_INITIALIZER IOT_LINK_INITIALIZER
#define IotListDouble_ForEach IotContainers_ForEach
/** @} */

#define AiaClock( MEMBER ) IotClock_##MEMBER
#define IotClock_HEADER <platform/iot_clock.h>

/** Macros and typedefs for timers. */
/** @{ */
/** IotClock_ARM minimal starting interval **/
#define AIA_CLOCK_ARM_MS_MIN    10
#define AIA_CLOCK_ARM_START_MIN(_start_ms) (AIA_CLOCK_ARM_MS_MIN > _start_ms ? AIA_CLOCK_ARM_MS_MIN : _start_ms)
#define AiaClock_Timer_Create IotClock_TimerCreate
#define AiaClock_Timer_Destroy IotClock_TimerDestroy
#define AiaClock_Timer_Arm(_pt, _start, _duration) IotClock_TimerArm(_pt, AIA_CLOCK_ARM_START_MIN(_start), _duration)
#define AiaClock_Timer_HEADER IotClock_HEADER
#define AiaTimer( MEMBER ) AiaClock_Timer_##MEMBER
typedef IotTimer_t AiaTimer_t;
/** @} */

/**
 * Typedefs to handle MQTT communications.
 */
/** @{ */
typedef IotMqttConnection_t AiaMqttConnectionPointer_t;
typedef IotMqttQos_t AiaMqttQos_t;
typedef IotMqttCallbackParam_t AiaMqttCallbackParam_t;
static const AiaMqttQos_t AIA_MQTT_QOS0 = IOT_MQTT_QOS_0;
/** @} */

/**
 * The timeout for MQTT operations.
 */
#define MQTT_TIMEOUT_MS 5000

/**
 * Retry for QOS 1 MQTT operations.
 */
/** @{ */
static const AiaDurationMs_t MQTT_RETRY_TIMEOUT_MS = 2000;
static const size_t MQTT_RETRY_LIMIT = 10;
/** @} */

/**
 * Following type is used to handle MQTT callbacks.
 */
typedef void ( *AiaMqttTopicHandler_t )( void*, AiaMqttCallbackParam_t* );

/**
 * Subscribes to a given MQTT connection topic with the provided parameters.
 *
 * @param connection Pointer to the MQTT connection to use for the subscription.
 * @param qos Quality of Service during subscription.
 * @param topic The topic to subscribe to.
 * @param handler The callback function to invoke after subscription is done.
 * @param userData Context of the callback function.
 * @return @c true if subscription is successful, else @ false.
 */
inline bool AiaMqttSubscribe( AiaMqttConnectionPointer_t connection,
                              AiaMqttQos_t qos, const char* topic,
                              AiaMqttTopicHandler_t handler, void* userData )
{
    IotMqttSubscription_t topicSubscription;
    topicSubscription.qos = qos;
    topicSubscription.pTopicFilter = topic;
    topicSubscription.topicFilterLength = (uint16_t)strlen( topic );
    topicSubscription.callback.function = handler;
    topicSubscription.callback.pCallbackContext = userData;

    return IOT_MQTT_SUCCESS ==
           IotMqtt_TimedSubscribe( connection, &topicSubscription,
                                   1, /* Subscribe to one topic at once */
                                   0, /* No flags */
                                   MQTT_TIMEOUT_MS ); /* Timeout - 5 seconds */
}

/**
 * Unsubscribes from a given MQTT connection topic with the provided parameters.
 *
 * @param connection Pointer to the MQTT connection to use for the
 * unsubscription.
 * @param qos Quality of Service during unsubscription.
 * @param topic The topic to unsubscribe from.
 * @param handler The callback function to invoke after unsubscription is done.
 * @param userData Context of the callback function.
 * @return @c true if unsubscription is successful, else @ false.
 */
inline bool AiaMqttUnsubscribe( AiaMqttConnectionPointer_t connection,
                                AiaMqttQos_t qos, const char* topic,
                                AiaMqttTopicHandler_t handler, void* userData )
{
    IotMqttSubscription_t topicSubscription;
    topicSubscription.qos = qos;
    topicSubscription.pTopicFilter = topic;
    topicSubscription.topicFilterLength = (uint16_t)strlen( topic );
    topicSubscription.callback.function = handler;
    topicSubscription.callback.pCallbackContext = userData;

    return IOT_MQTT_SUCCESS == IotMqtt_TimedUnsubscribe(
                                   connection, &topicSubscription,
                                   1, /* Unsubscribe from one topic at once */
                                   0, /* No flags */
                                   MQTT_TIMEOUT_MS ); /* Timeout - 5 seconds */
}

/**
 * Publishes to a given MQTT topic with the provided parameters.
 * @note Uses a blocking variant of publish when using QOS 1.
 *
 * @param connection Pointer to the MQTT connection to use for the publish.
 * @param qos Quality of Service for publish.
 * @param topic The topic to publish to.
 * @param topicLength The length of @c topic, or 0 if @c topic is
 *     null-terminated.
 * @param message The message to publish.
 * @param messageLength The length of @c message, or 0 if @c message is
 *     null-terminated.
 * @return @c true if publish is successful, else @c false.
 */
bool AiaMqttPublish( AiaMqttConnectionPointer_t connection, AiaMqttQos_t qos,
                     const char* topic, size_t topicLength, const void* message,
                     size_t messageLength );

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_IOT_CONFIG_H_ */
