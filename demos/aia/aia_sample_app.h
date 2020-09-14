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

/**
 * @file aia_sample_app.h
 * @brief User-facing functions of the @c AiaSampleApp_t type.
 */

#ifndef AIA_SAMPLE_APP_H_
#define AIA_SAMPLE_APP_H_

/* The config header is always included first. */
#include <aia_config.h>

/**
 * A sample application type that creates and runs the application via keyboard
 * inputs. The returned pointer should be destroyed using @c
 * AiaSampleApp_Destroy().
 */
typedef struct AiaSampleApp AiaSampleApp_t;

/**
 * Creates all underlying Aia components.
 *
 * @param mqttConnection Handle to a connected MQTT connection.
 * @param iotClientId Null-terminated IoT Client Id string for the device.
 * @return A newly created @c AiaSampleApp_t if successful or @c NULL otherwise.
 */
AiaSampleApp_t* AiaSampleApp_Create( AiaMqttConnectionPointer_t mqttConnection,
                                     const char* iotClientId );

/**
 * Uninitializes and deallocates an @c AiaSampleApp_t previously created
 * by a call to @c AiaSampleApp_Create().
 *
 * @param sampleApp The @c AiaSampleApp_t to destroy.
 */
void AiaSampleApp_Destroy( AiaSampleApp_t* sampleApp );

/* TODO: ADSER-1689 Allow for running on single-threaded machines. Make
 * non-blocking. */
/**
 * This is a blocking function that processes user inputs via @c stdin. This
 * function will return when a user inputs a command to quit.
 *
 * @param sampleApp The @c AiaSampleApp_t to act on.
 */
void AiaSampleApp_Run( AiaSampleApp_t* sampleApp );

#endif /* ifndef AIA_SAMPLE_APP_H_ */
