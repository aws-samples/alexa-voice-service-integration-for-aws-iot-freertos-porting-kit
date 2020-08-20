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
