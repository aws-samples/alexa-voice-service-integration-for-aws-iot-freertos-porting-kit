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

#ifndef AIA_COMMON_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_COMMON_CONFIG_H_

#include <microphone/aia_microphone_config.h>

#include <assert.h>
#include <stddef.h>

/**
 * Tests whether @c EXPRESSION is true, and terminates the application if it is
 * not.
 * @warning Applications may stub this out such that EXPRESSION is never
 * evaluated, so application code should not depend on it.  For example, if @c
 * AiaAssert() is implemented using @c assert(), the asserts will all be
 * disabled uf @c NDEBUG is defined.
 *
 * @param EXPRESSION A boolean expression to evaluate and assert is true.
 */
#define AiaAssert( EXPRESSION ) assert( ( EXPRESSION ) )

/**
 * For those platform do not define ssize_t type
 */
typedef long ssize_t;

/**
 * This function will be called by the SDK when critical un-recoverable failures
 * occur. Examples include memory allocation failures.
 */
static inline void AiaCriticalFailure()
{
    AiaLogError( "AiaCriticalFailure" );
    AiaAssert(0);   // or use iot_reset_reboot(True) by vendors' implementation
}

/** How often data will be published on the /event topic. */
static const AiaDurationMs_t EVENT_PUBLISH_RATE = MICROPHONE_PUBLISH_RATE;

/** How many slots to be used in a sequencing buffer. */
static const size_t AIA_SEQUENCER_SLOTS = 4;

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_COMMON_CONFIG_H_ */
