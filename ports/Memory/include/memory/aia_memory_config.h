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
