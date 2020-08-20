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

#ifndef AIA_MEMORY_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define AIA_MEMORY_CONFIG_H_

#include <stdlib.h>

/**
 * Allocates zero'd memory.  Memory allocated using this function should be
 * released using a call to @c AiaFree().
 *
 * @param count The number of elements to allocate.
 * @param size The size (in bytes) of each element.
 * @return A @c void pointer to the allocated memory, or @c NULL if the memory
 * cannot be allocated.
 */
static inline void* AiaCalloc( size_t count, size_t size )
{
    void* ptr = pvPortMalloc( count * size );

    if (ptr)
    {
        memset(ptr, 0, count * size);
    }
    return ptr;
}

/** Releases memory allocated by a call to @c AiaCalloc(). */
static inline void AiaFree( void* ptr )
{
    vPortFree( ptr );
}

#ifdef __cplusplus
}
#endif
#endif /* ifndef AIA_MEMORY_CONFIG_H_ */
