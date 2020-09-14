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
 * @file aia_sample_app.c
 * @brief Aia sample app.
 */

/* The config header is always included first. */
#include <aia_config.h>

#include "aia_sample_app.h"

#include <aia_application_config.h>
#include <aia_capabilities_config.h>
#include <aiaalertmanager/aia_alert_slot.h>
#include <aiaclient/aia_client.h>
#include <aiaconnectionmanager/aia_connection_constants.h>
#include <aiacore/aia_mbedtls_threading.h>
#include <aiacore/aia_random_mbedtls.h>
#include <aiacore/aia_utils.h>
#include <aiacore/capabilities_sender/aia_capabilities_sender.h>
#include <aiacore/data_stream_buffer/aia_data_stream_buffer.h>
#include <aiacore/data_stream_buffer/aia_data_stream_buffer_reader.h>
#include <aiacore/data_stream_buffer/aia_data_stream_buffer_writer.h>
#include <aiaexceptionmanager/aia_exception_code.h>
#include <aiamicrophonemanager/aia_microphone_constants.h>
#include <aiaregistrationmanager/aia_registration_manager.h>
#include <aiauxmanager/aia_ux_state.h>

//#define AIA_DEMO_AUDIO_ENABLE
#ifdef AIA_DEMO_AUDIO_ENABLE
#include <aiaalerttonesynthesizer/aia_alert_tone.h>
#include <aiaopusdecoder/aia_opus_decoder.h>
#include <aiaportaudiomicrophone/aia_portaudio_microphone.h>
#include <aiaportaudiospeaker/aia_portaudio_speaker.h>
#endif

#include AiaTaskPool( HEADER )
#include AiaTimer( HEADER )

/* Platform layer includes. */
#include <inttypes.h>
#include "event_groups.h"
#include "platform/iot_clock.h"
#include "platform/iot_threads.h"

/** Information for registration */
#define AIA_REG_HTTPS_AWS_ACCOUNT_ID "012345678901"
#define AIA_REG_HTTPS_LWA_CLIENT_ID "xxxxxxxxxxxx"
#define AIA_REG_HTTPS_LWA_REFRESH_TOKEN "xxxxxxxxxxx"

extern const char *g_aiaIotEndpoint;
extern const char *g_aiaClientId;
extern const char *g_aiaAwsAccountId;
extern const char *g_aiaStorageFolder;
extern char *g_aiaLwaRefreshToken;
extern char *g_aiaLwaClientId;

/** The amount of audio data to keep in the microphone buffer. */
#define AMOUNT_OF_AUDIO_DATA_IN_MIC_BUFFER ( (AiaDurationMs_t)10000 )

/** Buffer size in samples. */
#define MIC_BUFFER_SIZE_IN_SAMPLES \
    ( ( size_t )( AMOUNT_OF_AUDIO_DATA_IN_MIC_BUFFER * AIA_MICROPHONE_SAMPLE_RATE_HZ / AIA_MS_PER_SECOND ) )

#ifdef AIA_DEMO_AUDIO_ENABLE
/**
 * Sample app pushes speaker data through PortAudio speaker which publishes
 * @c SAMPLE_RATE samples per second. We calculate here how long it takes in
 * milliseconds to publish all the samples in a given alert tone frame.
 */
#define AIA_ALERT_TONE_FRAME_LENGTH_IN_MS \
    ( ( AiaDurationMs_t )( ( AIA_ALERT_TONE_FRAME_LENGTH * AIA_MS_PER_SECOND ) / SAMPLE_RATE ) )
#endif

/** AIA Demo Cases **/
typedef enum _AIA_DEMO_CASES_E
{
    AIA_DEMO_CASE_CONNECT = 0,
    AIA_DEMO_CASE_CAPABILITY,
    AIA_DEMO_CASE_SYNC_ST,
    AIA_DEMO_CASE_SYNC_TIME,
    AIA_DEMO_CASE_HOLD_TO_TALK_START,
    AIA_DEMO_CASE_HOLD_TO_TALK_STOP,
    AIA_DEMO_CASE_DISCONNECT,
} AIA_DEMO_CASES_E;

char *AIA_DEMO_CASE_TO_STRING[] = {
    "AIA_DEMO_CASE_CONNECT",   "AIA_DEMO_CASE_CAPABILITY",         "AIA_DEMO_CASE_SYNC_ST",
    "AIA_DEMO_CASE_SYNC_TIME", "AIA_DEMO_CASE_HOLD_TO_TALK_START", "AIA_DEMO_CASE_HOLD_TO_TALK_STOP",
    "AIA_DEMO_CASE_DISCONNECT"
};

/** AIA Demo Events **/
#define AIA_EVENT_CONNECTED ( 0x1 << 0 )
#define AIA_EVENT_DISCONNECTED ( 0x1 << 1 )
#define AIA_EVENT_CAP_CHANGED ( 0x1 << 2 )
#define AIA_EVENT_REGISTERED ( 0x1 << 3 )
#define AIA_EVENT_DUMMY ( 0x1 << 4 )

static EventGroupHandle_t aia_eg;

#define AIA_DEMO_EG_WAIT( _event, _ms ) \
    xEventGroupWaitBits( aia_eg, _event, true, false, _ms == 0 ? portMAX_DELAY : pdMS_TO_TICKS( _ms ) )
#define AIA_DEMO_EG_DUMMY_WAIT_WAIT( _ms ) AIA_DEMO_EG_WAIT( AIA_EVENT_DUMMY | AIA_EVENT_DISCONNECTED, _ms )
#define AIA_DEMO_EG_SET( _event ) xEventGroupSetBits( aia_eg, _event )
#define AIA_DEMO_EG_GET() xEventGroupGetBits( aia_eg )

/** Buffer size in bytes; */
static const size_t MIC_BUFFER_SIZE_IN_BYTES = MIC_BUFFER_SIZE_IN_SAMPLES * AIA_MICROPHONE_BUFFER_WORD_SIZE;

/** Arbitrary. */
static const size_t MIC_NUM_READERS = 2;

/** @name Methods that simply print states to stdout. */
/** @{ */
static void onAiaConnectionSuccessfulSimpleUI( void *userData );
static void onAiaDisconnectedSimpleUI( void *userData, AiaConnectionOnDisconnectCode_t code );
static void onAiaConnectionRejectedSimpleUI( void *userData, AiaConnectionOnConnectionRejectionCode_t code );
static void onAiaExceptionReceivedSimpleUI( void *userData, AiaExceptionCode_t code );
static void onAiaCapabilitiesStateChangedSimpleUI( AiaCapabilitiesSenderState_t state, const char *description,
                                                   size_t descriptionLen, void *userData );
static void onUXStateChangedSimpleUI( AiaUXState_t state, void *userData );
static void onAiaRegistrationSuccess( void *userData );
static void onAiaRegistrationFailed( void *userData, AiaRegistrationFailureCode_t code );
/** @} */

/**
 * Callback that allows the PortAudio speaker to indicate it is ready for
 * audio data again.
 *
 * @param userData Context to be associated with the callback.
 */
static void onPortAudioSpeakerReadyAgain( void *userData );

#ifdef AIA_ENABLE_SPEAKER
/**
 * Callback from the underlying @c AiaSpeakerManager_t to push frames for
 * playback.
 *
 * @param buf The buffer to play.
 * @param size Size of @c buf
 * @param userData Context for this callback.
 */
static bool onSpeakerFramePushedForPlayback( const void *buf, size_t size, void *userData );

/**
 * Callback from the underlying @c AiaSpeakerManager_t to change the volume.
 *
 * @param newVolume The new volume, between @c AIA_MIN_VOLUME and @c
 * AIA_MAX_VOLUME, inclusive.
 * @param userData Context for this callback. This should point to an @c
 * AiaPortAudioSpeaker_t.
 */
static void onSpeakerVolumeChanged( uint8_t newVolume, void *userData );

/**
 * Callback from the underlying @c AiaSpeakerManager_t to start offline alert
 * playback.
 *
 * @param offlineAlert The offline alert to play the alert tone for.
 * @param userData Context for this callback.
 * @return @c true If offline alert playback has successfully started or @c
 * false otherwise.
 */
static bool onStartOfflineAlertTone( const AiaAlertSlot_t* offlineAlert,
                                     void* userData );

/**
 * This is a periodic function controlled by the @c offlineAlertPlaybackTimer.
 * It is responsible for pushing a frame to the speaker for the offline alert
 * tone every time it is invoked.
 *
 * @param userData Context for this function.
 */
static void onPlayOfflineAlertTone( void* userData );

/**
 * Callback from the underlying @c AiaSpeakerManager_t to stop offline alert
 * playback.
 *
 * @param userData Context for this callback.
 * @return @c true If offline alert playback has been successfully stopped or @c
 * false otherwise.
 */
static bool onStopOfflineAlertTone( void* userData );
#endif

/**
 * Processes inputted command from a user.
 *
 * @param c The command inputted as a character to process.
 * @param[out] exit This is set to @c true if a user wishes to quit the
 * application.
 * @param sampleApp The @c AiaSampleApp_t that will process received commands.
 */
static void processDemoCase( AIA_DEMO_CASES_E c, AiaSampleApp_t *sampleApp );

/**
 * Initialize the AIA Client.
 *
 * @param sampleApp The @c AiaSampleApp_t to initialize the client for.
 */
static bool initAiaClient( AiaSampleApp_t *sampleApp );

/**
 * Register with AIA.
 *
 * @param sampleApp The @c AiaSampleApp_t to initialize the client for.
 */
static bool registerAia( AiaSampleApp_t *sampleApp );

/**
 * Container of all components necessary for the client to run.
 */
struct AiaSampleApp
{
    /** Handle to a connected MQTT connection. */
    AiaMqttConnectionPointer_t mqttConnection;

    /** Raw underlying buffer that will be used to hold microphone data. */
    void *rawMicrophoneBuffer;

    /** This buffer will be used to hold microphone data. */
    AiaMicrophoneBuffer_t *microphoneBuffer;

    /** This will be used by the underlying @c aiaClient to stream microphone
     * data to Aia when needed. */
    AiaDataStreamReader_t *microphoneBufferReader;

    /** Used to capture and write microphone data to the @c microphoneBuffer. */
    AiaMicrophoneBufferWriter_t *microphoneBufferWriter;

    /** The Aia client. */
    AiaClient_t *aiaClient;

    /** The registration manager used to register the Aia client. */
    AiaRegistrationManager_t *registrationManager;
    
    /** Whether to skip publishing events to Aia service */
    AiaAtomicBool_t shouldPublishEvent;

#ifdef AIA_ENABLE_SPEAKER
    /** Whether speaker is ready to accept new frames */
    AiaAtomicBool_t isSpeakerReady;

    /** Whether the offline alert being currently played should be removed
     * from persistent storage */
    AiaAtomicBool_t shouldDeleteOfflineAlert;

    /** Timer that controls how often a frame should be pushed to the speaker
     * while playing an offline alert. */
    AiaTimer_t offlineAlertPlaybackTimer;

    /** Pointer to the offline alert that is currently being played */
    AiaAlertSlot_t* offlineAlertInProgress;

    /** Keeps track of the time offline alert playback started at */
    AiaTimepointSeconds_t offlineAlertPlaybackStartTime;
#endif

#ifdef AIA_DEMO_AUDIO_ENABLE
    /** Whether microphone is currently being recorded locally. */
    bool isMicrophoneActive;

    /** Whether an interaction (hold-to-talk) is currently occurring. */
    bool isMicrophoneOpen;

    /** The PortAudio microphone wrapper. */
    AiaPortAudioMicrophoneRecorder_t *portAudioMicrophoneRecorder;

    /** Opus decoder to decode speaker frames into PCM data. */
    AiaOpusDecoder_t *opusDecoder;

    /** PortAudio based speaker to play PCM data. */
    AiaPortAudioSpeaker_t *portAudioSpeaker;
#endif

    /** A flag is used to store whether this client connects to Aia */
    bool isAiaClientConnected;

    /** A flag is used to check whether this demo should keep running */
    bool toRunDemo;

    /** Capability state of this client */
    AiaCapabilitiesSenderState_t capState;
};

AiaSampleApp_t *AiaSampleApp_Create( AiaMqttConnectionPointer_t mqttConnection, const char *iotClientId )
{
    /* Enabled mbed TLS threading layer. */
    AiaMbedtlsThreading_Init();

    /**
     * Initialize mbed TLS contexts needed for random number generation.
     * Required if AiaRandom_Rand() and AiaRandom_Seed() are using the
     * AiaRandomMbedtls implementation.
     */
    AiaRandomMbedtls_Init();

    /**
     * Initialize Aia MbedTLS Crypto usage.
     */
    if( !AiaCryptoMbedtls_Init() )
    {
        AiaLogError( "AiaCryptoMbedtls_Init failed" );
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    if( !AiaRandom_Seed( iotClientId, strlen( iotClientId ) ) )
    {
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        AiaLogError( "AiaRandom_Seed failed" );
        return NULL;
    }

    /* TODO: ADSER-1690 Simplify cleanup. */
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)AiaCalloc( 1, sizeof( AiaSampleApp_t ) );
    if( !sampleApp )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu.", sizeof( AiaSampleApp_t ) );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->mqttConnection = mqttConnection;

    sampleApp->rawMicrophoneBuffer = AiaCalloc( MIC_BUFFER_SIZE_IN_BYTES, 1 );
    if( !sampleApp->rawMicrophoneBuffer )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu", MIC_BUFFER_SIZE_IN_BYTES );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->microphoneBuffer = AiaDataStreamBuffer_Create( sampleApp->rawMicrophoneBuffer, MIC_BUFFER_SIZE_IN_BYTES,
                                                              AIA_MICROPHONE_BUFFER_WORD_SIZE, MIC_NUM_READERS );
    if( !sampleApp->microphoneBuffer )
    {
        AiaLogError( "AiaDataStreamBuffer_Create failed" );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->microphoneBufferReader = AiaDataStreamBuffer_CreateReader(
        sampleApp->microphoneBuffer, AIA_DATA_STREAM_BUFFER_READER_NONBLOCKING, true );
    if( !sampleApp->microphoneBufferReader )
    {
        AiaLogError( "AiaDataStreamBuffer_CreateReader failed" );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->microphoneBufferWriter = AiaDataStreamBuffer_CreateWriter(
        sampleApp->microphoneBuffer, AIA_DATA_STREAM_BUFFER_WRITER_NONBLOCKABLE, false );
    if( !sampleApp->microphoneBufferWriter )
    {
        AiaLogError( "AiaDataStreamBuffer_CreateWriter failed" );
        AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

#ifdef AIA_DEMO_AUDIO_ENABLE
    sampleApp->portAudioMicrophoneRecorder = AiaPortAudioMicrophoneRecorder_Create( sampleApp->microphoneBufferWriter );
    if( !sampleApp->portAudioMicrophoneRecorder )
    {
        AiaLogError( "AiaPortAudioMicrophoneRecorder_Create failed" );
        AiaDataStreamWriter_Destroy( sampleApp->microphoneBufferWriter );
        AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->isMicrophoneActive = false;

    sampleApp->opusDecoder = AiaOpusDecoder_Create();
    if( !sampleApp->opusDecoder )
    {
        AiaLogError( "AiaOpusDecoder_Create failed" );
        AiaPortAudioMicrophoneRecorder_Destroy( sampleApp->portAudioMicrophoneRecorder );
        AiaDataStreamWriter_Destroy( sampleApp->microphoneBufferWriter );
        AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }

    sampleApp->portAudioSpeaker = AiaPortAudioSpeaker_Create( onPortAudioSpeakerReadyAgain, sampleApp );
    if( !sampleApp->portAudioSpeaker )
    {
        AiaLogError( "AiaPortAudioSpeakerPlayer_Create failed" );
        AiaOpusDecoder_Destroy( sampleApp->opusDecoder );
        AiaPortAudioMicrophoneRecorder_Destroy( sampleApp->portAudioMicrophoneRecorder );
        AiaDataStreamWriter_Destroy( sampleApp->microphoneBufferWriter );
        AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }
#endif

#ifdef AIA_ENABLE_SPEAKER
    if( !AiaTimer( Create )( &sampleApp->offlineAlertPlaybackTimer,
                             onPlayOfflineAlertTone, sampleApp ) )
    {
        AiaLogError( "AiaTimer( Create ) failed" );
#ifdef AIA_DEMO_AUDIO_ENABLE
        AiaPortAudioSpeaker_Destroy( sampleApp->portAudioSpeaker );
        AiaOpusDecoder_Destroy( sampleApp->opusDecoder );
        AiaPortAudioMicrophoneRecorder_Destroy( sampleApp->portAudioMicrophoneRecorder );
#endif
        AiaDataStreamWriter_Destroy( sampleApp->microphoneBufferWriter );
        AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
        AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
        AiaFree( sampleApp->rawMicrophoneBuffer );
        AiaFree( sampleApp );
        AiaCryptoMbedtls_Cleanup();
        AiaRandomMbedtls_Cleanup();
        AiaMbedtlsThreading_Cleanup();
        return NULL;
    }
#endif

    AiaAtomicBool_Clear( &sampleApp->shouldPublishEvent );

#ifdef AIA_ENABLE_SPEAKER
    AiaAtomicBool_Set( &sampleApp->isSpeakerReady );
    AiaAtomicBool_Clear( &sampleApp->shouldDeleteOfflineAlert );
#endif

    g_aiaIotEndpoint = clientcredentialMQTT_BROKER_ENDPOINT;
    g_aiaClientId = clientcredentialIOT_THING_NAME;
    g_aiaAwsAccountId = AIA_REG_HTTPS_AWS_ACCOUNT_ID;
    g_aiaLwaRefreshToken = AIA_REG_HTTPS_LWA_REFRESH_TOKEN;
    g_aiaLwaClientId = AIA_REG_HTTPS_LWA_CLIENT_ID;

    aia_eg = xEventGroupCreate();
    sampleApp->isAiaClientConnected = false;
    sampleApp->toRunDemo = true;
    sampleApp->capState = AIA_CAPABILITIES_STATE_NONE;

    return sampleApp;
}

void AiaSampleApp_Destroy( AiaSampleApp_t *sampleApp )
{
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }

    AiaAtomicBool_Clear( &sampleApp->shouldPublishEvent );

#ifdef AIA_ENABLE_SPEAKER
    AiaAtomicBool_Clear( &sampleApp->isSpeakerReady );
    AiaAtomicBool_Clear( &sampleApp->shouldDeleteOfflineAlert );

    AiaTimer( Destroy )( &sampleApp->offlineAlertPlaybackTimer );
    if( sampleApp->offlineAlertInProgress )
    {
        AiaFree( sampleApp->offlineAlertInProgress );
        sampleApp->offlineAlertInProgress = NULL;
    }
#endif

#ifdef AIA_DEMO_AUDIO_ENABLE
    AiaPortAudioSpeaker_Destroy( sampleApp->portAudioSpeaker );
    AiaOpusDecoder_Destroy( sampleApp->opusDecoder );
    AiaPortAudioMicrophoneRecorder_Destroy( sampleApp->portAudioMicrophoneRecorder );
#endif
    if( sampleApp->aiaClient )
    {
        AiaClient_Destroy( sampleApp->aiaClient );
        sampleApp->aiaClient = NULL;
    }

    AiaDataStreamWriter_Destroy( sampleApp->microphoneBufferWriter );
    AiaDataStreamReader_Destroy( sampleApp->microphoneBufferReader );
    AiaDataStreamBuffer_Destroy( sampleApp->microphoneBuffer );
    AiaFree( sampleApp->rawMicrophoneBuffer );
    sampleApp->isAiaClientConnected = false;
    sampleApp->toRunDemo = false;
    sampleApp->capState = AIA_CAPABILITIES_STATE_NONE;
    AiaFree( sampleApp );

    AiaCryptoMbedtls_Cleanup();
    AiaRandomMbedtls_Cleanup();
    AiaMbedtlsThreading_Cleanup();

    vEventGroupDelete( aia_eg );
}

void AiaSampleApp_Run( AiaSampleApp_t *sampleApp )
{
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }

#ifdef AIA_DEMO_AUDIO_ENABLE
    const AIA_DEMO_CASES_E cmd[] = { AIA_DEMO_CASE_CONNECT,           AIA_DEMO_CASE_CAPABILITY,
                                     AIA_DEMO_CASE_SYNC_ST,           AIA_DEMO_CASE_HOLD_TO_TALK_START,
                                     AIA_DEMO_CASE_HOLD_TO_TALK_STOP, AIA_DEMO_CASE_DISCONNECT };
#else
    const AIA_DEMO_CASES_E cmd[] = { AIA_DEMO_CASE_CONNECT, AIA_DEMO_CASE_CAPABILITY, AIA_DEMO_CASE_SYNC_ST,
                                     AIA_DEMO_CASE_SYNC_TIME, AIA_DEMO_CASE_DISCONNECT };
#endif

    if( !registerAia( sampleApp ) )
    {
        AiaLogError( "Registration Failed" );
        sampleApp->toRunDemo = false;
        return;
    }
    AiaLogInfo( "Aia Registered." );

    if( !initAiaClient( sampleApp ) )
    {
        AiaLogError( "Client initialization failed" );
        sampleApp->toRunDemo = false;
        return;
    }
    AiaLogInfo( "Client initialized." );

    for( int i = 0; i < sizeof( cmd ) / sizeof( AIA_DEMO_CASES_E ) && sampleApp->toRunDemo; ++i )
    {
        AIA_DEMO_CASES_E command = cmd[ i ];
        AiaLogInfo( "---- Current demo case = %s ----", AIA_DEMO_CASE_TO_STRING[ command ] );
        processDemoCase( command, sampleApp );
    }

    if( sampleApp->aiaClient )
    {
        if( sampleApp->isAiaClientConnected )
        {
            AiaClient_Disconnect( sampleApp->aiaClient, AIA_CONNECTION_ON_DISCONNECTED_GOING_OFFLINE, NULL );
            AIA_DEMO_EG_WAIT( AIA_EVENT_DISCONNECTED, 5000 );
            if( 0 == ( AIA_DEMO_EG_GET() & AIA_EVENT_DISCONNECTED ) )
            {
                AiaLogError( "Client disconnect timeout." );
            }
        }
        AiaClient_Destroy( sampleApp->aiaClient );
        sampleApp->aiaClient = NULL;
        sampleApp->toRunDemo = false;
        AiaLogInfo( "Client deinitialize." );
    }
}

static bool initAiaClient( AiaSampleApp_t *sampleApp )
{
    AiaLogInfo( "Initializing client." );
    sampleApp->aiaClient =
        AiaClient_Create( sampleApp->mqttConnection, onAiaConnectionSuccessfulSimpleUI, onAiaConnectionRejectedSimpleUI,
                          onAiaDisconnectedSimpleUI, sampleApp, AiaTaskPool( GetSystemTaskPool )(),
                          onAiaExceptionReceivedSimpleUI, NULL, onAiaCapabilitiesStateChangedSimpleUI, sampleApp
#ifdef AIA_ENABLE_SPEAKER
                          ,
                          onSpeakerFramePushedForPlayback, sampleApp, onSpeakerVolumeChanged, sampleApp, onStartOfflineAlertTone, sampleApp, onStopOfflineAlertTone, sampleApp
#endif
                          ,
                          onUXStateChangedSimpleUI, NULL
#ifdef AIA_ENABLE_MICROPHONE
                          ,
                          sampleApp->microphoneBufferReader
#endif
        );

    if( !sampleApp->aiaClient )
    {
        AiaLogError( "AiaClient_Create failed" );
        return false;
    }

    return true;
}

static bool registerAia( AiaSampleApp_t *sampleApp )
{
    AiaLogInfo( "Registering with Aia" );
    sampleApp->registrationManager =
        AiaRegistrationManager_Create( onAiaRegistrationSuccess, sampleApp, onAiaRegistrationFailed, sampleApp );
    if( !sampleApp->registrationManager )
    {
        AiaLogError( "AiaRegistrationManager_Create failed" );
        return false;
    }

    if( !AiaRegistrationManager_Register( sampleApp->registrationManager ) )
    {
        AiaLogError( "AiaRegistrationManager_Register Failed" );
        AiaRegistrationManager_Destroy( sampleApp->registrationManager );
        sampleApp->registrationManager = NULL;
        return false;
    }
    return true;
}

static void onAiaRegistrationSuccess( void *userData )
{
    if( !userData )
    {
        AiaLogError( "Null userData." );
        return;
    }
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaLogInfo( "Registration Succeeded" );
    AiaRegistrationManager_Destroy( sampleApp->registrationManager );
    sampleApp->registrationManager = NULL;
}

static void onAiaRegistrationFailed( void *userData, AiaRegistrationFailureCode_t code )
{
    if( !userData )
    {
        AiaLogError( "Null userData." );
        return;
    }
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaLogInfo( "Registration Failed, code=%d", code );
    AiaRegistrationManager_Destroy( sampleApp->registrationManager );
    sampleApp->registrationManager = NULL;
}

void processDemoCase( AIA_DEMO_CASES_E c, AiaSampleApp_t *sampleApp )
{
    switch( c )
    {
        case AIA_DEMO_CASE_CONNECT:
            AiaLogInfo( "Connecting to Aia" );
            if( !AiaClient_Connect( sampleApp->aiaClient ) )
            {
                AiaLogError( "Failed to connect to Aia" );
                sampleApp->toRunDemo = false;
                return;
            }
            AIA_DEMO_EG_WAIT( AIA_EVENT_CONNECTED, 5000 );
            sampleApp->toRunDemo = sampleApp->isAiaClientConnected;
            return;
        case AIA_DEMO_CASE_DISCONNECT:
            AiaLogInfo( "Disconnecting from Aia" );
            if( !AiaClient_Disconnect( sampleApp->aiaClient, AIA_CONNECTION_ON_DISCONNECTED_GOING_OFFLINE, NULL ) )
            {
                AiaLogError( "Failed to disconnect from Aia" );
            }
            AIA_DEMO_EG_WAIT( AIA_EVENT_DISCONNECTED, 5000 );
            return;
#ifdef AIA_DEMO_AUDIO_ENABLE
        case AIA_DEMO_CASE_HOLD_TO_TALK_START:
            AiaLogInfo( "Hold to talk" );
            if( !sampleApp->isMicrophoneOpen )
            {
                sampleApp->isMicrophoneOpen = !sampleApp->isMicrophoneOpen;
#ifdef AIA_ENABLE_SPEAKER
                AiaClient_StopSpeaker( sampleApp->aiaClient );
                AiaLogInfo( "\n\n-------- Start to talk --------\n\n" );
                AIA_DEMO_EG_DUMMY_WAIT_WAIT( 500 );
                if( 0 == ( AIA_DEMO_EG_GET() & AIA_EVENT_DISCONNECTED ) )
                {
                    if( !AiaClient_HoldToTalkStart( sampleApp->aiaClient, AiaDataStreamWriter_Tell( sampleApp->microphoneBufferWriter ) ) )
                    {
                        AiaLogError( "\n\n-------- Failed to initiate hold-to-talk. --------\n\n" );
                        sampleApp->toRunDemo = false;
                        return;
                    }
                    if( !sampleApp->isMicrophoneActive )
                    {
                        AiaPortAudioMicrophoneRecorder_StartStreamingMicrophoneData( sampleApp->portAudioMicrophoneRecorder );
                        sampleApp->isMicrophoneActive = true;
                        AiaLogInfo( "New microphone state: %s", sampleApp->isMicrophoneActive ? "on" : "off" );
                    }
                    AIA_DEMO_EG_DUMMY_WAIT_WAIT( 2500 );
                }
#endif
            }
            return;
        case AIA_DEMO_CASE_HOLD_TO_TALK_STOP:
            if( sampleApp->isMicrophoneOpen )
            {
                if( sampleApp->isMicrophoneActive )
                {
                    AiaPortAudioMicrophoneRecorder_StopStreamingMicrophoneData(
                        sampleApp->portAudioMicrophoneRecorder );
                    sampleApp->isMicrophoneActive = false;
                    AiaLogInfo( "New microphone state: %s", sampleApp->isMicrophoneActive ? "on" : "off" );
                }
                AiaLogInfo( "\n\n-------- Stop talking --------\n\n" );
                AIA_DEMO_EG_DUMMY_WAIT_WAIT( 2500 );
                AiaClient_CloseMicrophone( sampleApp->aiaClient );
                if( 0 == ( AIA_DEMO_EG_GET() & AIA_EVENT_DISCONNECTED ) )
                {
                    AiaLogInfo( "\n\n-------- Waiting for hold-to-talk AIS response... --------\n\n" );
                    AIA_DEMO_EG_DUMMY_WAIT_WAIT( 7000 );
                    sampleApp->isMicrophoneOpen = !sampleApp->isMicrophoneOpen;
                    AiaClient_StopSpeaker( sampleApp->aiaClient );
                }
                else
                {
                    AiaLogWarn( "\n\n-------- Skip hold-to-talk demo because of disconnection. --------\n\n" );
                    return;
                }
            }
            return;
#endif
        case AIA_DEMO_CASE_CAPABILITY:
            AiaLogInfo( "Publishing capabilities" );
            if( !AiaClient_PublishCapabilities( sampleApp->aiaClient ) )
            {
                AiaLogError( "Failed to publish capabilities." );
                sampleApp->toRunDemo = false;
                return;
            }
            AIA_DEMO_EG_WAIT( AIA_EVENT_CAP_CHANGED, 5000 );
            sampleApp->toRunDemo = sampleApp->capState == AIA_CAPABILITIES_STATE_PUBLISHED;
            if( sampleApp->toRunDemo )
            {
                AIA_DEMO_EG_WAIT( AIA_EVENT_CAP_CHANGED, 5000 );
                sampleApp->toRunDemo = sampleApp->capState == AIA_CAPABILITIES_STATE_ACCEPTED;
            }
            return;
        case AIA_DEMO_CASE_SYNC_ST:
            AiaLogInfo( "Publishing SynchronizeState event" );
            if( !AiaClient_SynchronizeState( sampleApp->aiaClient ) )
            {
                AiaLogError( "Failed to synchronize state." );
                sampleApp->toRunDemo = false;
                return;
            }
            return;
#ifdef AIA_ENABLE_CLOCK
        case AIA_DEMO_CASE_SYNC_TIME:
            AiaLogInfo( "Initiating NTP clock synchronization" );
            if( !AiaClient_SynchronizeClock( sampleApp->aiaClient ) )
            {
                AiaLogError( "Failed to synchronize clock." );
                sampleApp->toRunDemo = false;
                return;
            }
            return;
#endif
        default:
            AiaLogInfo( "Unsupported test case" );
            sampleApp->toRunDemo = false;
            return;
    }
}

static void onAiaConnectionSuccessfulSimpleUI( void *userData )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }
    AiaAtomicBool_Set( &sampleApp->shouldPublishEvent );
    AiaLogInfo( "Aia connection successful" );

    sampleApp->isAiaClientConnected = true;
    AIA_DEMO_EG_SET( AIA_EVENT_CONNECTED );
}

static void onAiaDisconnectedSimpleUI( void *userData, AiaConnectionOnDisconnectCode_t code )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }
    AiaAtomicBool_Clear( &sampleApp->shouldPublishEvent );
    AiaLogWarn( "Disconnected from Aia, code=%d", code );

    sampleApp->isAiaClientConnected = false;
    sampleApp->toRunDemo = false;
    AiaLogWarn( "Stop demo because client is disconnected now" );
    AIA_DEMO_EG_SET( AIA_EVENT_DISCONNECTED );
}

static void onAiaConnectionRejectedSimpleUI( void *userData, AiaConnectionOnConnectionRejectionCode_t code )
{
    AiaSampleApp_t* sampleApp = (AiaSampleApp_t*)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }

    AiaAtomicBool_Clear( &sampleApp->shouldPublishEvent );
    AiaLogInfo( "Aia connection rejected, code=%d", code );
}

static void onAiaExceptionReceivedSimpleUI( void *userData, AiaExceptionCode_t code )
{
    (void)userData;
    AiaLogInfo( "Aia exception received, code=%d", code );
}

static void onAiaCapabilitiesStateChangedSimpleUI( AiaCapabilitiesSenderState_t state, const char *description,
                                                   size_t descriptionLen, void *userData )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaLogInfo( "Aia capabilities state changed, state=%s, description=%.*s",
                AiaCapabilitiesSenderState_ToString( state ), descriptionLen, description );
    sampleApp->capState = state;
    AIA_DEMO_EG_SET( AIA_EVENT_CAP_CHANGED );
}

static void onPortAudioSpeakerReadyAgain( void *userData )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }
    AiaAtomicBool_Set( &sampleApp->isSpeakerReady );
#ifdef AIA_ENABLE_SPEAKER
    AiaClient_OnSpeakerReady( sampleApp->aiaClient );
#endif
}

#ifdef AIA_ENABLE_SPEAKER
static bool onSpeakerFramePushedForPlayback( const void *buf, size_t size, void *userData )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return false;
    }
    AiaLogDebug( "Received speaker frame for playback, size=%zu", size );

    bool ret = true;
    int16_t *pcmSamples = NULL;
#ifdef AIA_DEMO_AUDIO_ENABLE
    int decodedSamples = 0;
    pcmSamples = AiaOpusDecoder_DecodeFrame( sampleApp->opusDecoder, buf, size, &decodedSamples );
    if( !pcmSamples )
    {
        AiaLogError( "AiaOpusDecoder_DecodeFrame failed" );
        return false;
    }

    ret = AiaPortAudioSpeaker_PlaySpeakerData( sampleApp->portAudioSpeaker, pcmSamples, decodedSamples );
#endif
    if( pcmSamples )
    {
        AiaFree( pcmSamples );
    }
    return ret;
}

static void onSpeakerVolumeChanged( uint8_t newVolume, void *userData )
{
    AiaSampleApp_t *sampleApp = (AiaSampleApp_t *)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }
    AiaLogDebug( "Received volume change, volume=%" PRIu8, newVolume );
#ifdef AIA_DEMO_AUDIO_ENABLE
    if( sampleApp->portAudioSpeaker )
    {
        AiaPortAudioSpeaker_SetNewVolume( sampleApp->portAudioSpeaker, newVolume );
    }
#endif
}

static bool onStopOfflineAlertTone( void* userData )
{
    AiaSampleApp_t* sampleApp = (AiaSampleApp_t*)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return false;
    }

    /* Destroy offline alert playback timer */
    AiaTimer( Destroy )( &sampleApp->offlineAlertPlaybackTimer );
    if( !AiaTimer( Create )( &sampleApp->offlineAlertPlaybackTimer,
                             onPlayOfflineAlertTone, sampleApp ) )
    {
        AiaLogError( "AiaTimer( Create ) failed" );
        return false;
    }

    /* Clear offline alert in progress, clear conditional flags */
    if( sampleApp->offlineAlertInProgress )
    {
        AiaFree( sampleApp->offlineAlertInProgress );
        sampleApp->offlineAlertInProgress = NULL;
    }
    sampleApp->offlineAlertPlaybackStartTime = 0;
    AiaAtomicBool_Clear( &sampleApp->shouldDeleteOfflineAlert );

    return true;
}

static bool onStartOfflineAlertTone( const AiaAlertSlot_t* offlineAlert,
                                     void* userData )
{
    AiaSampleApp_t* sampleApp = (AiaSampleApp_t*)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return false;
    }
    if( !offlineAlert )
    {
        AiaLogError( "Null offlineAlert" );
        return false;
    }

    /* Set the offline alert to play, initialize conditional variables */
    sampleApp->offlineAlertInProgress =
        AiaCalloc( 1, sizeof( AiaAlertSlot_t ) );
    if( !sampleApp->offlineAlertInProgress )
    {
        AiaLogError( "AiaCalloc failed, bytes=%zu.", sizeof( AiaAlertSlot_t ) );
        return false;
    }
    *sampleApp->offlineAlertInProgress = *offlineAlert;

    /* Set the time we started offline alert playback */
    sampleApp->offlineAlertPlaybackStartTime = AiaClock_GetTimeSinceNTPEpoch();

    AiaLogDebug( "Playing offline alert tone for %s",
                 AiaAlertType_ToString( offlineAlert->alertType ) );

    /* Arm the offline alert playback timer to go off every @c
     * AIA_SPEAKER_FRAME_PUSH_CADENCE_MS milliseconds */
    if( !AiaTimer( Arm )( &sampleApp->offlineAlertPlaybackTimer, 0,
                          AIA_SPEAKER_FRAME_PUSH_CADENCE_MS ) )
    {
        AiaLogError( "AiaTimer( Arm ) failed" );
        if( sampleApp->offlineAlertInProgress )
        {
            AiaFree( sampleApp->offlineAlertInProgress );
            sampleApp->offlineAlertInProgress = NULL;
        }
        sampleApp->offlineAlertPlaybackStartTime = 0;
        AiaTimer( Destroy )( &sampleApp->offlineAlertPlaybackTimer );
        return false;
    }

    AiaLogInfo( "Started offline alert playback timer" );

    return true;
}

static void onPlayOfflineAlertTone( void* userData )
{
    AiaSampleApp_t* sampleApp = (AiaSampleApp_t*)userData;
    AiaAssert( sampleApp );
    if( !sampleApp )
    {
        AiaLogError( "Null sampleApp" );
        return;
    }

    /* Quit early if speaker is not ready or if offline alert playback is
     * disabled */
    if( !AiaAtomicBool_Load( &sampleApp->isSpeakerReady ) )
    {
        return;
    }

    /* Pointer to the offline alert that should be played */
    AiaAlertSlot_t* offlineAlert = sampleApp->offlineAlertInProgress;

    /* Check if this alert exceeded its duration */
    AiaTimepointSeconds_t now = AiaClock_GetTimeSinceNTPEpoch();
    if( now >= offlineAlert->scheduledTime )
    {
        AiaDurationSeconds_t timeSinceAlertStart =
            now - offlineAlert->scheduledTime;
        if( timeSinceAlertStart * AIA_MS_PER_SECOND > offlineAlert->duration )
        {
            AiaLogDebug( "Offline alert expired, deleting it" );
            AiaAtomicBool_Set( &sampleApp->shouldDeleteOfflineAlert );
        }
    }

    /* Invoke the callback to delete the alert from local storage if it is
     * supposed to be deleted */
    if( AiaAtomicBool_Load( &sampleApp->shouldDeleteOfflineAlert ) )
    {
        if( !AiaClient_DeleteAlert( sampleApp->aiaClient,
                                    offlineAlert->alertToken ) )
        {
            AiaLogWarn( "Failed to delete offline alert" );
        }

        AiaTimer( Destroy )( &sampleApp->offlineAlertPlaybackTimer );
        if( !AiaTimer( Create )( &sampleApp->offlineAlertPlaybackTimer,
                                 onPlayOfflineAlertTone, sampleApp ) )
        {
            AiaLogError( "AiaTimer( Create ) failed" );
        }

        if( sampleApp->offlineAlertInProgress )
        {
            AiaFree( sampleApp->offlineAlertInProgress );
            sampleApp->offlineAlertInProgress = NULL;
        }
        sampleApp->offlineAlertPlaybackStartTime = 0;
        AiaAtomicBool_Clear( &sampleApp->shouldDeleteOfflineAlert );

        return;
    }

    /* Variables needed to play the offline alert tone */
    size_t numFrames = 0;
    size_t frameLength = 0;

    /* Check for correct alert type */
    switch( offlineAlert->alertType )
    {
        case AIA_ALERT_TYPE_ALARM:
        case AIA_ALERT_TYPE_REMINDER:
        case AIA_ALERT_TYPE_TIMER:
#ifdef AIA_DEMO_AUDIO_ENABLE
            numFrames = AIA_SPEAKER_FRAME_PUSH_CADENCE_MS /
                        AIA_ALERT_TONE_FRAME_LENGTH_IN_MS;
            /* Make sure we are pushing at least one frame */
            numFrames = numFrames > 0 ? numFrames : 1;
            frameLength = AIA_ALERT_TONE_FRAME_LENGTH;
#endif
            break;
        default:
            AiaLogError( "Unknown alert type:%" PRIu32,
                         offlineAlert->alertType );
            if( sampleApp->offlineAlertInProgress )
            {
                AiaFree( sampleApp->offlineAlertInProgress );
                sampleApp->offlineAlertInProgress = NULL;
            }
            sampleApp->offlineAlertPlaybackStartTime = 0;
            return;
    }

    for( size_t i = 0; i < numFrames; ++i )
    {
#ifdef AIA_DEMO_AUDIO_ENABLE
        if( !AiaPortAudioSpeaker_PlaySpeakerData(
                sampleApp->portAudioSpeaker, AIA_ALERT_TONE, frameLength ) )
        {
            AiaLogDebug( "Failed to play offline alert buffer" );
            AiaAtomicBool_Clear( &sampleApp->isSpeakerReady );
            return;
        }
#endif
    }

    return;
}
#endif

static void onUXStateChangedSimpleUI( AiaUXState_t state, void *userData )
{
    (void)userData;
    AiaLogInfo( "**** UX state changed, state=%s ****", AiaUXState_ToString( state ) );
}
