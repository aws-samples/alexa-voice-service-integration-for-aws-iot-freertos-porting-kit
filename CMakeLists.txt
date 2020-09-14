cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
if(${CMAKE_VERSION} VERSION_LESS 3.18)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.18)
endif()

# Set project information
project(AiaClientSDK VERSION 0.0.0 LANGUAGES C)
set(PROJECT_BRIEF "A cross-platform SDK for interacting with the AVS for AWS IoT Core")

# Enable more verbose 'check' target
add_custom_target( check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure )

# Use C99.
set( CMAKE_C_STANDARD 99 )
set( CMAKE_C_STANDARD_REQUIRED ON )

# TODO: Move this to a separate CMake file specific encapsulating the build
if ( CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU" )
	SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -pedantic" )
endif()

# Do not allow in-source build.
if( ${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds are not allowed. Please build in a separate directory, such as ${PROJECT_SOURCE_DIR}/../build." )
endif()

if(NOT AIA_CLIENT_SDK_DIR)
    set(AIA_CLIENT_SDK_DIR "${PROJECT_SOURCE_DIR}/external/AIAClientSDK")
endif()

include(${AIA_CLIENT_SDK_DIR}/cmake/Logging.cmake)

# Build for Amazon FreeRTOS
set(aia_core_dir "${AIA_CLIENT_SDK_DIR}/AiaCore")
set(aia_util_dir "${AIA_CLIENT_SDK_DIR}/ApplicationUtilities")

set(aia_afr_dir "${PROJECT_SOURCE_DIR}")
set(aia_afr_ports_dir "${aia_afr_dir}/ports")
set(aia_afr_demo_dir "${aia_afr_dir}/demos/aia")
set(aia_afr_util_dir "${aia_afr_demo_dir}/utils")

set(AIA_AFR_PORTS_FOLDER "${aia_afr_ports_dir}")
set(AIA_BUTTON_FOLDER "${AIA_AFR_PORTS_FOLDER}/Button")
set(AIA_CLOCK_FOLDER "${AIA_AFR_PORTS_FOLDER}/Clock")
set(AIA_COMMON_FOLDER "${AIA_AFR_PORTS_FOLDER}/Common")
set(AIA_CRYPTO_FOLDER "${AIA_AFR_PORTS_FOLDER}/Crypto")
set(AIA_HTTP_FOLDER "${AIA_AFR_PORTS_FOLDER}/HTTP")
set(AIA_IOT_FOLDER "${AIA_AFR_PORTS_FOLDER}/IoT")
set(AIA_LWA_FOLDER "${AIA_AFR_PORTS_FOLDER}/LWA")
set(AIA_MEMORY_FOLDER "${AIA_AFR_PORTS_FOLDER}/Memory")
set(AIA_MICROPHONE_FOLDER "${AIA_AFR_PORTS_FOLDER}/Microphone")
set(AIA_REGISTRATION_FOLDER "${AIA_AFR_PORTS_FOLDER}/Registration")
set(AIA_STORAGE_FOLDER "${AIA_AFR_PORTS_FOLDER}/Storage")

set(AIA_DEMO_AFR_AUDIO false CACHE BOOL "Enable Audio Demo for AFR Windows Simulator")

# Module - aia_interface
afr_module(NAME aia_interface)

afr_set_lib_metadata(ID "aia_interface")
afr_set_lib_metadata(DESCRIPTION "This library provides the interface to the AIA, which allows devices to retrive Alexa via MQTT.")
afr_set_lib_metadata(DISPLAY_NAME "aia_interface")
afr_set_lib_metadata(CATEGORY "AIA")
afr_set_lib_metadata(VERSION "1.0.0")
afr_set_lib_metadata(IS_VISIBLE "true")

afr_module_include_dirs(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        "${aia_core_dir}/include"
        "${aia_afr_ports_dir}/include"
        "${AIA_BUTTON_FOLDER}/include"
        "${AIA_CLOCK_FOLDER}/include"
        "${AIA_CRYPTO_FOLDER}/include"
        "${AIA_IOT_FOLDER}/include"
        "${AIA_HTTP_FOLDER}/include"
        "${AIA_LWA_FOLDER}/include"
        "${AIA_MEMORY_FOLDER}/include"
        "${AIA_MICROPHONE_FOLDER}/include"
        "${AIA_REGISTRATION_FOLDER}/include"
        "${AIA_STORAGE_FOLDER}/include"
        "${AIA_COMMON_FOLDER}/include"
)

afr_module_sources(
    ${AFR_CURRENT_MODULE}
    PRIVATE
        "${AIA_CLOCK_FOLDER}/src/aia_clock_config.c"
        "${AIA_CRYPTO_FOLDER}/src/aia_crypto_config.c"
        "${AIA_STORAGE_FOLDER}/src/aia_storage_config.c"
        "${AIA_HTTP_FOLDER}/src/aia_http_config.c"
        "${AIA_IOT_FOLDER}/src/aia_iot_config.c"
        "${AIA_LWA_FOLDER}/src/aia_lwa_config.c"
        "${AIA_REGISTRATION_FOLDER}/src/aia_registration_config.c"
)

afr_module_dependencies(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        3rdparty::jsmn
        3rdparty::mbedtls
        AFR::mqtt
        AFR::serializer
        AFR::https
)

# Module - 3rdparty_aia_core
afr_module(NAME 3rdparty_aia_core)

afr_set_lib_metadata(ID "3rdparty_aia_core")
afr_set_lib_metadata(DESCRIPTION "This is the core implementation of AIA")
afr_set_lib_metadata(DISPLAY_NAME "3rdparty_aia_core")
afr_set_lib_metadata(CATEGORY "AIA")
afr_set_lib_metadata(VERSION "1.0.0")
afr_set_lib_metadata(IS_VISIBLE "true")

afr_module_include_dirs(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        "${aia_core_dir}/include"
        "${aia_afr_ports_dir}/include"
)

file(GLOB_RECURSE aia_core_src_files "${aia_core_dir}/source/*/*.c")

afr_module_sources(
    ${AFR_CURRENT_MODULE}
    PRIVATE
        ${aia_core_src_files}
)

afr_module_dependencies(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        AFR::aia_interface
)

# Module - aia_demo_utils
afr_module(NAME aia_demo_utils)

afr_set_lib_metadata(ID "aia_demo_utils")
afr_set_lib_metadata(DESCRIPTION "This is the application utilities of AIA")
afr_set_lib_metadata(DISPLAY_NAME "aia_demo_utils")
afr_set_lib_metadata(CATEGORY "AIA")
afr_set_lib_metadata(VERSION "1.0.0")
afr_set_lib_metadata(IS_VISIBLE "true")

afr_module_include_dirs(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        "${aia_util_dir}/aiaclient/include"
)

afr_module_sources(
    ${AFR_CURRENT_MODULE}
    PRIVATE
        "${aia_util_dir}/aiaclient/source/aia_client.c"
)

afr_module_dependencies(
    ${AFR_CURRENT_MODULE}
    PUBLIC
        AFR::3rdparty_aia_core
)

# Module - aia_util with audio functionalities
if ("${AIA_DEMO_AFR_AUDIO}")
    if (NOT LIBOPUS_INCLUDE_DIR OR NOT LIBOPUS_LIB_PATH)
        message( FATAL_ERROR "Missing libopus; Please specify -DLIBOPUS_INCLUDE_DIR and -DLIBOPUS_LIB_PATH" )
    endif()
    if (NOT PORTAUDIO_INCLUDE_DIR OR NOT PORTAUDIO_LIB_PATH)
        message( FATAL_ERROR "Missing portaudio; Please specify -DPORTAUDIO_INCLUDE_DIR and -DPORTAUDIO_LIB_PATH" )
    endif()

    afr_module_include_dirs(
        ${AFR_CURRENT_MODULE}
        PUBLIC
            "${aia_util_dir}/aiaalerttonesynthesizer/include"
            "${aia_util_dir}/aiaopusdecoder/include"
            "${aia_util_dir}/aiaportaudiospeaker/include"
            "${aia_afr_util_dir}/aiaportaudiomicrophone/include"
            "${LIBOPUS_INCLUDE_DIR}"
            "${PORTAUDIO_INCLUDE_DIR}"
    )

    afr_module_sources(
        ${AFR_CURRENT_MODULE}
        PRIVATE
            "${aia_util_dir}/aiaopusdecoder/source/aia_opus_decoder.c"
            "${aia_util_dir}/aiaportaudiospeaker/source/aia_portaudio_speaker.c"
            "${aia_afr_util_dir}/aiaportaudiomicrophone/source/aia_portaudio_microphone.c"
    )

    if(TARGET aws_demos)
        target_link_libraries( aws_demos PRIVATE "${LIBOPUS_LIB_PATH}" )
        target_link_libraries( aws_demos PRIVATE "${PORTAUDIO_LIB_PATH}" )
    endif()
endif()

# Module aia_demo
afr_demo_module(aia_demo)

afr_set_demo_metadata(ID "AIA_DEMO")
afr_set_demo_metadata(DESCRIPTION "An example on how to use Alexa voice service via AWS IoT")
afr_set_demo_metadata(DISPLAY_NAME "AIA for FreeRTOS")

afr_module_sources(
    ${AFR_CURRENT_MODULE}
    INTERFACE
        "${aia_afr_demo_dir}/aia_sample_main.c"
        "${aia_afr_demo_dir}/aia_sample_app.c"
)

afr_module_dependencies(
    ${AFR_CURRENT_MODULE}
    INTERFACE
        AFR::3rdparty_aia_core
        AFR::aia_demo_utils
)