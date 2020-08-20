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
 * @file aia_application_config.h
 * @brief APIs required to be implemented by the end application.
 */

#ifndef AIA_APPLICATION_CONFIG_H_
#define AIA_APPLICATION_CONFIG_H_

#include <aia_config.h>

/* AIA Capabilities Config */
#include <aia_capabilities_config.h>

#include <aiaconnectionmanager/aia_connection_constants.h>
#include <aiacore/capabilities_sender/aia_capabilities_sender_state.h>
#include <aiacore/data_stream_buffer/aia_data_stream_buffer.h>
#include <aiacore/data_stream_buffer/aia_data_stream_buffer_writer.h>
#include <aiaexceptionmanager/aia_exception_code.h>
#include <aiauxmanager/aia_ux_state.h>

/* TODO: ADSER-1920 Move all of these into aia_config.h into simple functions
 * rather than callbacks such that failures to define them will result in a
 * compilation error. */
/**
 * The following are functions expected to be implemented by the end
 * application. Most are required parameters for creation of the @c AiaClient
 * object.
 */
/** @{ */

/**
 * This callback is invoked after the client is successfully connected to
 * the service upon a call to @c AiaClient_Connect. Implementations are
 * expected to be non-blocking and are not required to be thread-safe.
 *
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 */
typedef void ( *AiaConnectionManageronConnectionSuccessCallback_t )(
    void* userData );

/**
 * This callback function is invoked after the client is disconnected from the
 * service. This can occur upon reception of a server initiated disconnection or
 * upon a call to @c AiaClient_Disconnect(). Implementations are expected to be
 * non-blocking and are not required to be thread-safe.
 *
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 * @param code The Connection Disconnect Message code received by the client.
 */
typedef void ( *AiaConnectionManagerOnDisconnectedCallback_t )(
    void* userData, AiaConnectionOnDisconnectCode_t code );

/**
 * This callback function is used after the client receives a unsuccessful
 * Connection Acknowledgement message upon a call to @c AiaClient_Connect.
 * Implementations are expected to be non-blocking and are not required to be
 * thread-safe.
 *
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 * @param code The Connection Acknowledge Message code received by the client.
 */
typedef void ( *AiaConnectionManagerOnConnectionRejectionCallback_t )(
    void* userData, AiaConnectionOnConnectionRejectionCode_t code );

/**
 * This callback is invoked after an Exception directive is received from the
 * service. Implementations are expected to be non-blocking and are not required
 * to be thread-safe.
 * @note This may be helpful for applications that want to implement some sort
 * of watchdog logic.
 *
 * @param userData Optional user data pointer which was provided alongside the
 * callback.
 * @param code The Exception message code received by the client.
 */
typedef void ( *AiaExceptionManagerOnExceptionCallback_t )(
    void* userData, AiaExceptionCode_t code );

/**
 * This function allows clients to observe the state of the @c
 * AiaCapabilitiesSender_t and published capabilities upon a call to @c
 * AiaClient_PublishCapabilities.
 *
 * @param state The state of capabilities publishing.
 * @param description The description message provided by the Aia service
 * when acknowledging published capabilities. Note that this field will only be
 * valid when @c state is @c AIA_CAPABILITIES_STATE_REJECTED and that this field
 * is optional and may be @c NULL.
 * @param descriptionLen The length of @c description, not including a null
 * terminating character.
 * @param userData Context associated with this callback.
 * @note Calling back into the @c AiaClient_t from within the
 * same execution context of this callback will result in a deadlock.
 */
typedef void ( *AiaCapabilitiesObserver_t )( AiaCapabilitiesSenderState_t state,
                                             const char* description,
                                             size_t descriptionLen,
                                             void* userData );

/**
 * This function will be invoked to notify of @c AiaUXState_t changes.
 * Implementations are expected to be non-blocking and are not required to be
 * thread-safe.
 *
 * @param state The new UX state to display to the end user.
 * @param userData Context to be passed with this callback.
 * @note Calling back into the @c AiaClient_t from within the same
 * execution context of this callback will result in a deadlock.
 */
typedef void ( *AiaUXStateObserverCb_t )( AiaUXState_t state, void* userData );

/**
 * This function is used to push data from an internal stream to the platform
 * for playback every @c AIA_SPEAKER_FRAME_PUSH_CADENCE_MS. Implementations are
 * expected to be non-blocking and are not required to be thread-safe.
 *
 * @param buf A buffer containing the data to play to be copied.
 * @param size The number of bytes to play.
 * @param userData User data associated with this callback.
 *
 * @return @c true if the frame was accepted or @c false if the frame could not
 * be buffered.
 * @note In the current implementation, a single frame is pushed per
 * invocation.
 * @note If @c false is returned, this callback will not be called again until
 * notified to again via @c AiaClient_OnSpeakerReady() is called.
 * @note Calling back into the @c AiaClient_t from within the same
 * execution context of this callback will result in a deadlock.
 */
typedef bool ( *AiaPlaySpeakerData_t )( const void* buf, size_t size,
                                        void* userData );

/**
 * This function is used to change the speaker's volume. Implementations are
 * expected to be non-blocking and are not required to be thread-safe.
 *
 * @param volume The new absolute volume. This value will fall between @c
 * AIA_MIN_VOLUME and @c AIA_MAX_VOLUME, inclusive.
 * @param userData User data associated with this callback.
 * @warning Calling back into the @c AiaClient_t from within the same
 * execution context of this callback will result in a deadlock.
 * @note Implementations are expected to scale the volume level logarithmically
 * to account for human sound level perception of loudness (i.e. decibel).
 */
typedef void ( *AiaSetVolume_t )( uint8_t volume, void* userData );

/**
 * This is the shared buffer from which the SDK will consume microphone data
 * captured by the device. The device is expected to capture and write
 * microphone data into this buffer using an @c AiaMicrophoneBufferWriter_t.
 * Data contained in the underlying buffer must be in 16-bit linear PCM, 16-kHz
 * sample rate, single channel, little-endian byte order format.
 */
typedef AiaDataStreamBuffer_t AiaMicrophoneBuffer_t;

/**
 * This is the writer object that applications are expected to use to write data
 * into the @c AiaMicrophoneBuffer_t.
 */
typedef AiaDataStreamWriter_t AiaMicrophoneBufferWriter_t;

/** @} */

#endif /* ifndef AIA_APPLICATION_CONFIG_H_ */
