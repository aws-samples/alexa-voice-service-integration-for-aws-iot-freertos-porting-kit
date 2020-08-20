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
 * @brief Reference implementation of a microphone recorder using PortAudio.
 */

/* The config header is always included first. */
#include <aia_config.h>

#include <aiaportaudiomicrophone/aia_portaudio_microphone.h>

#include <aiamicrophonemanager/aia_microphone_constants.h>

#include AiaMutex( HEADER )
#include AiaTimer( HEADER )

/** Number of channels format of microphone data. */
static const int NUM_INPUT_CHANNELS = 1;

/** Not playing back anything in this component, so no output channels. */
static const int NUM_OUTPUT_CHANNELS = 0;

/** Sample rate to record at. */
static const double SAMPLE_RATE = 16000;

/**
 * Called by a @c AiaPortAudioMicrophoneRecorder_t's @c captureMicrophoneTimer
 * to capture and write available microphone data to the underlying buffer.
 * This task runs at a @c MICROPHONE_PUBLISH_RATE / 4 cadence (arbitrary / needs
 * tuning).
 *
 * @param userData Pointer to the @c AiaPortAudioMicrophoneRecorder_t to act on.
 */
static void AiaPortAudioMicrophoneRecorder_MicrophoneCaptureTask(
    void* userData );

/**
 * A thin wrapper around PortAudio used for recording microphone data.
 */
struct AiaPortAudioMicrophoneRecorder
{
    /** The writer that will be used to write audio data into an @c
     * AiaDataStreamBuffer_t. */
    AiaDataStreamWriter_t* bufferWriter;

    /** Mutex used to guard against asynchronous calls in threaded
     * environments. */
    AiaMutex_t mutex;

    /** @name Variables synchronized by mutex. */
    /** @{ */

    /** The PortAudio stream. */
    PaStream* paStream;

    /** @} */

    /** Timer used to periodically capture and write available microphone data.
     */
    AiaTimer_t captureMicrophoneTimer;

    /** Flag used check if timer is created.
     */
    bool captureMicrophoneTimerExisted;
};

AiaPortAudioMicrophoneRecorder_t* AiaPortAudioMicrophoneRecorder_Create(
    AiaDataStreamWriter_t* bufferWriter )
{
    if( !bufferWriter )
    {
        AiaLogError( "Null bufferWriter" );
        return NULL;
    }

    size_t wordSize = AiaDataStreamWriter_GetWordSize( bufferWriter );

    if( wordSize != AIA_MICROPHONE_BUFFER_WORD_SIZE )
    {
        AiaLogError( "Invalid word size, expected=%zu, received=%zu",
                     AIA_MICROPHONE_BUFFER_WORD_SIZE, wordSize );
        return NULL;
    }

    PaError err;
    err = Pa_Initialize();
    if( err != paNoError )
    {
        AiaLogError( "Failed to initialize PortAudio, errorCode=%s",
                     Pa_GetErrorText( err ) );
        return NULL;
    }

    AiaPortAudioMicrophoneRecorder_t* recorder =
        AiaCalloc( 1, sizeof( AiaPortAudioMicrophoneRecorder_t ) );
    if( !recorder )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu",
                     sizeof( AiaPortAudioMicrophoneRecorder_t ) );
        Pa_Terminate();
        return NULL;
    }

    if( !AiaMutex( Create )( &recorder->mutex, false ) )
    {
        AiaLogError( "AiaMutex( Create ) failed." );
        AiaFree( recorder );
        Pa_Terminate();
        return NULL;
    }

    recorder->bufferWriter = bufferWriter;

    err = Pa_OpenDefaultStream( &recorder->paStream, NUM_INPUT_CHANNELS,
                                NUM_OUTPUT_CHANNELS, paInt16, SAMPLE_RATE,
                                paFramesPerBufferUnspecified, NULL, NULL );
    if( err != paNoError )
    {
        AiaLogError( "Failed to open PortAudio default stream, errorCode=%s",
                     Pa_GetErrorText( err ) );
        AiaMutex( Destroy )( &recorder->mutex );
        AiaFree( recorder );
        Pa_Terminate();
        return NULL;
    }

    recorder->captureMicrophoneTimerExisted = false;

    return recorder;
}

void AiaPortAudioMicrophoneRecorder_Destroy(
    AiaPortAudioMicrophoneRecorder_t* recorder )
{
    AiaAssert( recorder );
    if( !recorder )
    {
        AiaLogError( "Null recorder" );
        return;
    }

    if( recorder->captureMicrophoneTimerExisted )
    {
        AiaTimer( Destroy )( &recorder->captureMicrophoneTimer );
        recorder->captureMicrophoneTimerExisted = false;
    }

    AiaMutex( Lock )( &recorder->mutex );
    Pa_CloseStream( recorder->paStream );
    AiaMutex( Unlock )( &recorder->mutex );

    Pa_Terminate();
    AiaMutex( Destroy )( &recorder->mutex );
    AiaFree( recorder );
}

void AiaPortAudioMicrophoneRecorder_MicrophoneCaptureTask( void* userData )
{
    AiaPortAudioMicrophoneRecorder_t* recorder =
        (AiaPortAudioMicrophoneRecorder_t*)userData;
    AiaAssert( recorder );
    if( !recorder )
    {
        AiaLogError( "Null recorder" );
        return;
    }

    AiaMutex( Lock )( &recorder->mutex );

    /* Get number of frames that can be read without blocking. */
    signed long numFramesAvailableToRead =
        Pa_GetStreamReadAvailable( recorder->paStream );
    if( numFramesAvailableToRead <= 0 )
    {
        if( numFramesAvailableToRead < 0 )
        {
            AiaLogError( "Pa_GetStreamReadAvailable failed, error=%ld",
                         numFramesAvailableToRead );
        }
        AiaMutex( Unlock )( &recorder->mutex );
        return;
    }

    if( Pa_IsStreamActive(recorder->paStream) <= 0)
    {
        AiaLogWarn( "Pa_ReadStream is not active" );
        AiaMutex( Unlock )( &recorder->mutex );
        return;
    }

    /* TODO: ADSER-1692 Double copy here, optimize. */
    int16_t buf[ numFramesAvailableToRead ];
    PaError err =
        Pa_ReadStream( recorder->paStream, buf, numFramesAvailableToRead );
    AiaMutex( Unlock )( &recorder->mutex );
    if( err != paNoError )
    {
        if( err != paInputOverflowed )
        {
            AiaLogError( "Pa_ReadStream failed, errorCode=%s",
                         Pa_GetErrorText( err ) );
            return;
        }
        else
        {
            AiaLogWarn( "Input overflowed and discarded from previous call" );
        }
    }

    ssize_t writeReturnCode = AiaDataStreamWriter_Write(
        recorder->bufferWriter, buf, numFramesAvailableToRead );
    if( writeReturnCode <= 0 )
    {
        AiaLogError( "Failed to write to stream, error=%s",
                     AiaDataStreamWriter_ErrorToString( writeReturnCode ) );
        return;
    }
}

bool AiaPortAudioMicrophoneRecorder_StartStreamingMicrophoneData(
    AiaPortAudioMicrophoneRecorder_t* recorder )
{
    AiaAssert( recorder );
    if( !recorder )
    {
        AiaLogError( "Null recorder" );
        return false;
    }

    if( !AiaTimer( Create )(
            &recorder->captureMicrophoneTimer,
            AiaPortAudioMicrophoneRecorder_MicrophoneCaptureTask, recorder ) )
    {
        AiaLogError( "Failed to create captureMicrophoneTimer" );
        return false;
    }
    recorder->captureMicrophoneTimerExisted = true;

    AiaMutex( Lock )( &recorder->mutex );
    PaError err = Pa_StartStream( recorder->paStream );
    AiaMutex( Unlock )( &recorder->mutex );
    if( err != paNoError )
    {
        AiaLogError( "Failed to start PortAudio stream" );
        AiaTimer( Destroy )( &recorder->captureMicrophoneTimer );
        recorder->captureMicrophoneTimerExisted = false;
        return false;
    }

    if( !AiaTimer( Arm )( &recorder->captureMicrophoneTimer, 0,
                          MICROPHONE_PUBLISH_RATE / 4 ) )
    {
        AiaLogError( "Failed to arm captureMicrophoneTimer" );
        AiaMutex( Lock )( &recorder->mutex );
        Pa_CloseStream( recorder->paStream );
        AiaMutex( Unlock )( &recorder->mutex );
        AiaTimer( Destroy )( &recorder->captureMicrophoneTimer );
        recorder->captureMicrophoneTimerExisted = false;
        return false;
    }

    return true;
}

bool AiaPortAudioMicrophoneRecorder_StopStreamingMicrophoneData(
    AiaPortAudioMicrophoneRecorder_t* recorder )
{
    AiaAssert( recorder );
    if( !recorder )
    {
        AiaLogError( "Null recorder" );
        return false;
    }

    AiaTimer(Destroy)(&recorder->captureMicrophoneTimer);
    recorder->captureMicrophoneTimerExisted = false;

    AiaMutex( Lock )( &recorder->mutex );
    PaError err = Pa_StopStream( recorder->paStream );
    AiaMutex( Unlock )( &recorder->mutex );
    if( err != paNoError )
    {
        AiaLogError( "Failed to stop PortAudio stream" );
        return false;
    }
    return true;
}
