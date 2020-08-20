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
 * @file aia_portaudio_microphone.c
 * @brief Reference implementation of a microphone recorder using PortAudio
 */

#ifndef AIA_SAMPLE_PORTAUDIO_MICROPHONE_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_SAMPLE_PORTAUDIO_MICROPHONE_H_

/* The config header is always included first. */
#include <aia_config.h>

#include <aiacore/data_stream_buffer/aia_data_stream_buffer_writer.h>

#include <portaudio.h>

/**
 * A thin wrapper around PortAudio used for recording microphone data. Methods
 * of this object are thread-safe.
 */
typedef struct AiaPortAudioMicrophoneRecorder AiaPortAudioMicrophoneRecorder_t;

/**
 * Creates an @c AiaPortAudioMicrophoneRecorder_t. The returned pointer should
 * be destroyed using @c AiaPortAudioMicrophoneRecorder_Destroy().
 *
 * @param bufferWriter The writer to use to write microphone data into an @c
 * AiaDataStreamBuffer_t.
 * @return A newly created @c AiaPortAudioMicrophoneRecorder_t if successful or
 * @c NULL otherwise.
 */
AiaPortAudioMicrophoneRecorder_t* AiaPortAudioMicrophoneRecorder_Create(
    AiaDataStreamWriter_t* bufferWriter );

/**
 * Uninitializes and deallocates an @c AiaPortAudioMicrophoneRecorder_t
 * previously created by a call to
 * @c AiaPortAudioMicrophoneRecorder_Create().
 *
 * @param recorder The @c AiaPortAudioMicrophoneRecorder_t to destroy.
 */
void AiaPortAudioMicrophoneRecorder_Destroy(
    AiaPortAudioMicrophoneRecorder_t* recorder );

/**
 * Starts streaming from the microphone.
 *
 * @param recorder The @c AiaPortAudioMicrophoneRecorder_t to act on.
 * @return Whether the start was successful.
 */
bool AiaPortAudioMicrophoneRecorder_StartStreamingMicrophoneData(
    AiaPortAudioMicrophoneRecorder_t* recorder );

/**
 * Stops streaming from the microphone.
 *
 * @param recorder The @c AiaPortAudioMicrophoneRecorder_t to act on.
 * @return Whether the stop was successful.
 */
bool AiaPortAudioMicrophoneRecorder_StopStreamingMicrophoneData(
    AiaPortAudioMicrophoneRecorder_t* recorder );

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_SAMPLE_PORTAUDIO_MICROPHONE_H_ */
