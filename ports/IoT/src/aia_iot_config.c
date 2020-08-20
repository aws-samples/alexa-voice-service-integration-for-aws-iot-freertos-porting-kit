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
 * @file aia_iot_config.c
 */

#include <iot/aia_iot_config.h>

bool AiaMqttPublish( AiaMqttConnectionPointer_t connection, AiaMqttQos_t qos,
                     const char* topic, size_t topicLength, const void* message,
                     size_t messageLength )
{
    if( !connection )
    {
        AiaLogError( "Null connection." );
        return false;
    }
    if( !topic )
    {
        AiaLogError( "Null topic." );
        return false;
    }
    if( !topicLength )
    {
        topicLength = strlen( topic );
    }
    if( !message )
    {
        AiaLogError( "Null message." );
        return false;
    }
    if( !messageLength )
    {
        messageLength = strlen( message );
    }
    IotMqttPublishInfo_t topicPublish = { .qos = qos,
                                          .pTopicName = topic,
                                          .topicNameLength = topicLength,
                                          .pPayload = message,
                                          .payloadLength = messageLength,
                                          .retryMs = MQTT_RETRY_TIMEOUT_MS,
                                          .retryLimit = MQTT_RETRY_LIMIT };

    AiaLogDebug( "[AiaMqttPublish] %.*s", topicLength, topic );

    /** TODO: ADSER-1400 Use non-blocking MQTT publish. */
    return IOT_MQTT_SUCCESS == IotMqtt_TimedPublish( connection, &topicPublish,
                                                     0, MQTT_TIMEOUT_MS );
}
