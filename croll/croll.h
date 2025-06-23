
/*

    croll.h library, to get your c rolling.

    Usage example:
    ```c
    // optional, used to strip prefixes from functions (except croll_init)
    // #define CROLL_STRIP_PREFIX
    #define CROLL_IMPLEMENTATION
    #include "../croll/croll.h"

    croll_daDecl(int, int_arr);

    int main() {
        croll_init(); // used to init croll's functionalities

        croll_logInfo("Starting...\n");

        croll_logInfo("Enter a string: ");
        croll_StringBuilder sb = croll_HgetLine();

        croll_logInfo("You entered: %s\n", sb.data);

        croll_sbFree(&sb);

        croll_logInfo("Array test...\n");

        struct int_arr arr = {0};

        for(int i = 0; i < 10; i++) {
            croll_daAppend(&arr, i);
        }

        croll_daForEach(&arr, it) {
            croll_logInfo("  %d\n", *it);
        }

        return 0;
    }
    ```

    Implementation macros:
        CROLL_IMPLEMENTATION, CROLL_POOL_ALLOC_IMPLEMENTATION
    Helper macros:
        CROLL_STRIP_PREFIX

    License: MIT
    
Copyright 2025 Viktor Hugo C.M.G.
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
    associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
    and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
    THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef LIB_CROLLing_H
#define LIB_CROLLing_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

//======================================================================
// CRoll specific parts
//======================================================================

#if __has_attribute(format)
#define __CROLL_FORMAT_ATTR __attribute__((format(printf, 1, 2)))
#else
#define __CROLL_FORMAT_ATTR
#endif

#if __has_attribute(always_inline)
#define __CROLL_INLINE_ATTR inline __attribute__((always_inline))
#else
#define __CROLL_INLINE_ATTR inline
#endif

static FILE *croll_STDIN;
static FILE *croll_STDERR;
static FILE *croll_STDOUT;

//======================================================================
// Macros
//======================================================================

#define croll_ASSERT(expr, msg) assert((expr) && (msg))
#define croll_UNUSED(x) (void)(x)

#define croll_checkNullPtr(ptr) if ((ptr) == NULL)
#define croll_nullPtrGuard(ptr) if ((ptr) != NULL)

#define croll_ALLOC_ALIGN(size) (((size) + 7) & ~7)


// dynamic array macros

// Example use:
// croll_daDeclStart(int, )
//     int a;
//     int b;
// croll_daDeclEnd()
#define croll_daDeclStart(type, name) struct name { type *data; size_t len, cap;
#define croll_daDeclEnd() }
#define croll_daDecl(type, name) croll_daDeclStart(type, name) croll_daDeclEnd()

#define croll_daFree(da) do {free((da)->data); (da)->data = NULL; (da)->len = 0; (da)->cap = 0;} while(0)

#define croll_daInitCap 64
#define croll_daReserve(da, expect)                                                                   \
    do {                                                                                              \
        if ((expect) > (da)->cap) {                                                                   \
            if ((da)->cap == 0)                                                                       \
                (da)->cap = croll_daInitCap;                                                          \
            while ((da)->cap < (expect))                                                              \
                (da)->cap *= 2;                                                                       \
            (da)->data = realloc((da)->data, (da)->cap * sizeof(*(da)->data));    \
            croll_ASSERT((da)->data != NULL, "Failed to reallocate data array at file: "__FILE__);    \
        }                                                                                             \
    } while(0)

#define croll_daAppend(da, value)              \
    do {                                       \
        croll_daReserve((da), (da)->len+1);    \
        (da)->data[(da)->len++] = (value);     \
    } while(0)

#define croll_daExtend(da, values, value_count)                                           \
    do {                                                                                  \
        croll_daReserve((da), (da)->len+(value_count));                                   \
        memcpy((da)->data + (da)->len, (values), (value_count) * sizeof(*(da)->data));    \
        (da)->len += (value_count);                                                       \
    } while(0)

#define croll_daLast(da) ((da)->data[(croll_ASSERT((da)->len > 0, "Data array is empty at file: "__FILE__), (da)->len-1)])

#define croll_daForEach(type, it, da) for(type *it = (da)->data; it < (da)->data + (da)->len; it++)
#define croll_daForEach_elemIndex(da, it) ((it) - (da)->data)

// hash table defines

// #define croll_htDeclStart(type, name) struct name { struct {char *key; type value;} *entries; int cap; int len;}
// #define croll_htDeclEnd() }
// #define croll_htDecl(type, name) croll_htDeclStart(type, name) croll_htDeclEnd()

// #define croll_htFree(ht) do {free((ht)->entries); (ht)->entries = NULL; (ht)->len = 0; (ht)->cap = 0;} while(0)

// #define croll_htInitCap 16

// #define croll_htFunctionDecl(ht_type_name, type) \
//     static inline ht_type_name *ht_##type##_new(void) {\
//         ht_type_name *table = malloc(sizeof(type));\
//         croll_checkNullPtr(table) return NULL;\
//         table->len = 0;\
//         table->cap = croll_htInitCap;\
//         \
//         /* allocate memory for entries */\
//         table->entries = (struct {char *key; type value;} *)calloc(croll_htInitCap, sizeof(struct {char *key; type value;}));\
//         croll_checkNullPtr(table->entries) { free(table); return NULL; }\
//     \
//         return table;\
//     }\
//     \
//     static inline void ht_##type##_destroy(ht_type_name *table) {\
//         croll_checkNullPtr(table) return;\
//         \
//         for(int i = 0; i < table->len; i++)\
//             free(table->entries[i].key);\
//         \
//         free(table->entries);\
//         free(table);\
//     }\
//     \
//     static inline type *ht_##type##_get(ht_type_name *table, char *key) {\
//         size_t hash = croll_hashDjb2(key);\
//         size_t index = (size_t)(hash & table->cap) /* hash % cap */\
//         \
//         /* loop until empty entry */ \
//         while(table->entries[index].key != NULL) {\
//             if(strcmp(table->entries[index].key, key) == 0)\
//                 return &table->entries[index].value;\ /* found entry */\
//             \
//             index++;\
//             if(index => table->cap)\
//                 index = 0;\
//         }\
//         \
//         return NULL; /* not found */\
//     }\
//     \
//     static inline void ht_##type##_set()

//======================================================================
// Types
//======================================================================

// helper types

typedef int8_t croll_i8;
typedef uint8_t croll_u8;
typedef int16_t croll_i16;
typedef uint16_t croll_u16;
typedef int32_t croll_i32;
typedef uint32_t croll_u32;
typedef int64_t croll_i64;
typedef uint64_t croll_u64;

// helper aliases

typedef croll_u8 croll_byte;

// string builder

typedef croll_daDecl(char, ) croll_StringBuilder;

#define croll_sbAppend(sb, c) croll_daAppend(sb, c)
#define croll_sbExtend(sb, str, len) croll_daExtend(sb, str, len)
#define croll_sbFree(sb) croll_daFree(sb)

// bump allocator

// Aka arena allocator, used as essentially a garbage collector
typedef struct croll_BumpAlloc {
    croll_byte *data;
    size_t size;
    size_t offset;
    struct croll_BumpAlloc *_next;
} croll_BumpAlloc;

// A simple pool allocator.

/**
 * @struct croll_PoolAlloc
 * @brief Pool allocator
 * 
 * A simple pool allocator.
 * 
 * The allocation is done in O(1) time due to the next free chunk being always available in the `free_chunks` field.
 */
typedef struct {
    void *free_chunks;
    void *chunks;
    size_t chunk_size;
    size_t pool_size;

    int allocated_chunks; // debug
} croll_PoolAlloc;
#define croll_poolChunkDecl(size)

// Won't be implemented yet
// // A simpler version of croll_BumpAlloc for use with all sorts of memories, from heap to a simple char arr[1024]; heap like array. Used mostly on croll internal string functions.
// typedef struct croll_SBumpAlloc {
//     croll_u8 *data;
//     size_t size;
//     size_t offset;
// } croll_SBumpAlloc;

//======================================================================
// Strip prefix
//======================================================================

#ifdef CROLL_STRIP_PREFIX

#define logInfo     croll_logInfo
#define logWarn     croll_logWarn
#define logError    croll_logError

#define HgetLine    croll_HgetLine
#define SgetLine    croll_SgetLine

#endif // CROLL_STRIP_PREFIX

//======================================================================
// Declaration of functions
//======================================================================

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Initializes the CRoll library.
 *
 * This function should be called before using any other functions in the library.
 */
static void croll_init();

// logging

/**
 * @brief Prints a formatted string to stdout with the info format.
 *
 * The info format is blue and has [INFO] before the message.
 *
 * @param format The format string to use. Must be a valid format string for vprintf.
 * @param ... The arguments to use in the format string.
 */
static void croll_logInfo(const char *format, ...) __CROLL_FORMAT_ATTR;
/*
 * @brief Prints a formatted string to stderr with the warning format.
 *
 * The warning format is yellow and has [WARN] before the message.
 *
 * @param format The format string to use. Must be a valid format string for vprintf.
 * @param ... The arguments to use in the format string.
 */
static void croll_logWarn(const char *format, ...) __CROLL_FORMAT_ATTR;
/**
 * @brief Prints a formatted string to stderr with the error format.
 *
 * The error format is red and has [RED] before the message.
 *
 * @param format The format string to use. Must be a valid format string for vprintf.
 * @param ... The arguments to use in the format string.
 */
static void croll_logError(const char *format, ...) __CROLL_FORMAT_ATTR;

// IO

/**
 * @brief Reads a line from stdin and stores it in a croll_StringBuilder.
 *
 * This function reads a line from stdin and stores it in a croll_StringBuilder.
 * The line is read until a newline or EOF is encountered.
 *
 * @return The croll_StringBuilder containing the line.
 */
static croll_StringBuilder croll_HgetLine();
/**
 * @brief Reads a line from stdin and stores it in a char array.
 *
 * This function reads a line from stdin and stores it in the given char array.
 * The line is read until a newline or EOF is encountered.
 *
 * @param buffer The char array to store the line in. Must be at least buffer_size+1 in size.
 * @param buffer_size The maximum size of the buffer. Must be greater than 0.
 *
 * @return True if the line was successfully read, false if the buffer was too small.
 */
static bool croll_SgetLine(char *buffer, size_t buffer_size);

// memory

// bump/arena allocator

/**
 * @brief Creates a new bump allocator.
 *
 * Creates a new bump allocator with the given capacity.
 * @param cap The capacity of the bump allocator.
 *
 * @return A pointer to the new bump allocator, or NULL if allocation failed.
 */
static croll_BumpAlloc *croll_bumpNew(size_t capacity);

/**
 * @brief Allocates memory using a bump allocator.
 *
 * This function allocates memory using a bump allocator.
 *
 * @param bump The bump allocator to use.
 * @param size The size of the memory to allocate.
 *
 * @return A pointer to the allocated memory.
 */
static void *croll_bumpAlloc(croll_BumpAlloc *bump, size_t size);

/**
 * @brief Resets a bump allocator.
 *
 * This function resets a bump allocator. The offset is just set to 0, so the next allocation will start at the beginning.
 *
 * @param bump The bump allocator to reset.
 */
static void croll_bumpReset(croll_BumpAlloc *bump);

/**
 * @brief Destroys a bump allocator.
 *
 * This function destroys a bump allocator, freeing it's content and nullifying it.
 *
 * @param bump The bump allocator to destroy.
 */
static void croll_bumpDestroy(croll_BumpAlloc *bump);

// pool allocator

/**
 * @brief Creates a new pool allocator.
 *
 * Creates a new pool allocator with the given pool size and chunk size.
 * `chunk_size` must be greater than or qual to `sizeof(void*)`
 * @param pool_size The size of the pool.
 * @param chunk_size The size of the chunks.
 *
 * @return A pointer to the new pool allocator, or NULL if allocation failed.
 */
static croll_PoolAlloc *croll_poolNew(size_t pool_size, size_t chunk_size);

/**
 * @brief Destroys a pool allocator.
 *
 * Destroys a pool allocator, freeing it's content and nullifying it.
 *
 * @param pool The pool allocator to destroy.
 */
static void croll_poolDestroy(croll_PoolAlloc *pool);

/**
 * @brief Allocates memory using a pool allocator.
 *
 *  This function allocates memory using a pool allocator.
 * 
 * @param pool The pool allocator to use.
 *
 * @return A pointer to the allocated memory.
 */
static void *croll_poolAlloc(croll_PoolAlloc *pool);

/**
 * @brief Frees memory using a pool allocator.
 *
 * This function frees memory using a pool allocator. `pool` and `chunk` must not be NULL
 *
 * @param pool The pool allocator to use.
 * @param chunk The chunk to free.
 */
static void croll_poolFree(croll_PoolAlloc *pool, void *chunk);

// hash table

/**
 * @brief Computes the hash of a string using the Djb2 algorithm.
 * 
 * @param str The string to hash.
 * 
 * @return The hash of the string.
 * 
 * @see https://en.wikipedia.org/wiki/Daniel_J._Bernstein#Software
 */
static size_t croll_hashDjb2(char *str);

#ifdef __cplusplus
}
#endif // __cplusplus

//======================================================================
// Implementation
//======================================================================

#ifdef CROLL_IMPLEMENTATION

static void croll_init() {
    croll_STDIN = stdin;
    croll_STDERR = stderr;
    croll_STDOUT = stdout;
}

// logging

static void croll_logInfo(const char *format, ...) {
    printf("\033[94m[INFO]\033[0m ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

static void croll_logWarn(const char *format, ...) {
    fprintf(croll_STDERR, "\033[93m[WARN]\033[0m ");
    va_list args;
    va_start(args, format);
    vfprintf(croll_STDERR, format, args);
    va_end(args);
}

static void croll_logError(const char *format, ...) {
    printf("\033[91m[ERROR]\033[0m ");
    va_list args;
    va_start(args, format);
    vfprintf(croll_STDERR, format, args);
    va_end(args);
}

// IO

static croll_StringBuilder croll_HgetLine() {
    croll_StringBuilder sb = {0};
    while (true) {
        char c = fgetc(croll_STDIN);
        if (c == EOF || c == '\n') break;
        croll_daAppend(&sb, c);
    }
    croll_daAppend(&sb, '\0'); // ensures null termination
    return sb;
}

static bool croll_SgetLine(char *buffer, size_t buffer_size) {
    croll_ASSERT(buffer != NULL, "Buffer must not be NULL at file: "__FILE__);
    croll_ASSERT(buffer_size > 0, "Buffer size must be greater than 0 at file: "__FILE__);
    size_t buffer_length = 1;
    while (true) {
        char c = fgetc(croll_STDIN);
        if (c == EOF || c == '\n') break;
        *buffer++ = c;
        buffer_length++;
        if (buffer_length >= buffer_size) {
            *buffer = '\0';
            return false;
        }
    }
    *buffer = '\0';
    return true;
}

// memory

static croll_BumpAlloc *croll_bumpNew(size_t cap) {
    croll_BumpAlloc *bump = malloc(sizeof(croll_BumpAlloc));
    croll_checkNullPtr(bump) return NULL;
    
    bump->data = malloc(cap);
    croll_checkNullPtr(bump->data) {
        free(bump);
        return NULL;
    }
    bump->_next = NULL;
    bump->size = cap;
    bump->offset = 0;
    
    return bump;
}

static __CROLL_INLINE_ATTR void *croll_bumpAlloc(croll_BumpAlloc *bump, size_t size) {
    if(size == 0) return NULL;
    if(size & 0b111) size = croll_ALLOC_ALIGN(size);

    size_t cur_addr = (size_t)bump->data + bump->offset;
    if(cur_addr + size > (size_t)bump->data + bump->size)
        return NULL;
    
    bump->offset += size;
    return (void *)cur_addr;
}

static __CROLL_INLINE_ATTR void *croll_bumpAllocOrExpand(croll_BumpAlloc *bump, size_t size) {
    if(size == 0) return NULL;
    if(size & 0b111) size = croll_ALLOC_ALIGN(size);

    size_t cur_addr = (size_t)bump->data + bump->offset;
    if(cur_addr + size > (size_t)bump->data + bump->size) {
        while(cur_addr + size > (size_t)bump->data + bump->size) {
            if(bump->_next == NULL) {
                bump->_next = croll_bumpNew(bump->size);
                if(bump->_next == NULL) return NULL;
            }
            bump = bump->_next;
            cur_addr = (size_t)bump->data + bump->offset;
        }
    }

    bump->offset += size;
    return (void *)cur_addr;
} 

static __CROLL_INLINE_ATTR void croll_bumpReset(croll_BumpAlloc *bump) {
    bump->offset = 0;
    while(bump->_next != NULL) {
        bump = bump->_next;
        bump->offset = 0;
    }
}

static __CROLL_INLINE_ATTR void croll_bumpDestroy(croll_BumpAlloc *bump) {
    do {
        croll_BumpAlloc *next = bump->_next;
        free(bump->data);
        bump->data = NULL;
        bump->size = 0;
        bump->offset = 0;
        free(bump);
        bump = NULL;
        bump = next;
    } while(bump->_next != NULL);
}

// hash table

// Djb2 hash function
static size_t croll_hashDjb2(char *str) {
    size_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

#endif // CROLL_IMPLEMENTATION

#ifdef CROLL_POOL_ALLOC_IMPLEMENTATION
static croll_PoolAlloc *croll_poolNew(size_t pool_size, size_t chunk_size) {
    chunk_size = croll_ALLOC_ALIGN(chunk_size);
    if(pool_size == 0 || chunk_size < sizeof(void*))
        return NULL;
    croll_PoolAlloc *pool = malloc(sizeof(croll_PoolAlloc));
    croll_checkNullPtr(pool) return NULL;

    pool->chunks = pool->free_chunks = malloc(pool_size * chunk_size);
    croll_checkNullPtr(pool->chunks) {
        free(pool);
        return NULL;
    }

    pool->chunk_size = chunk_size;
    pool->pool_size = pool_size;
    pool->allocated_chunks = 0;

    // Reason why chunk_size >= sizeof(void*)
    croll_byte *chunks = pool->chunks;
    for(size_t i = 0; i < pool_size - 1; i++)
        // pool->chunks[i]->next = pool->chunks[i + 1];
        *(void**)(chunks + i * chunk_size) = chunks + (i + 1) * chunk_size;
    // pool->chunks[pool_size - 1]->next = NULL;
    *(void**)(chunks + (pool_size - 1) * chunk_size) = NULL;

    return pool;
}

static __CROLL_INLINE_ATTR void croll_poolDestroy(croll_PoolAlloc *pool) {
    croll_checkNullPtr(pool) return;
    free(pool->chunks);
    free(pool);
}

static __CROLL_INLINE_ATTR void *croll_poolAlloc(croll_PoolAlloc *pool) {
    croll_checkNullPtr(pool) return NULL;
    croll_checkNullPtr(pool->free_chunks) return NULL;

    void *chunk = pool->free_chunks;
    pool->free_chunks = *(void**)pool->free_chunks;

    pool->allocated_chunks++;
    return chunk;
}

static __CROLL_INLINE_ATTR void croll_poolFree(croll_PoolAlloc *pool, void *chunk) {
    croll_checkNullPtr(pool) return;
    croll_checkNullPtr(chunk) return;
    
//    if(!(chunk >= pool->chunks && (pool->chunks + (pool->pool_size-1) * pool->chunk_size) <= chunk))
//        return;

    *(void**)chunk = pool->free_chunks;
    pool->free_chunks = chunk;

    pool->allocated_chunks--;
}
#endif // CROLL_POOL_ALLOC_IMPLEMENTATION

#endif // LIB_CROLLing_H