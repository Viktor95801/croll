
/*

    croll.h library, to get your c rolling.

    Usage example:
    ```c
    // optional, used to strip prefixes from functions (except croll_init)
    // #define CROLL_STRIP_PREFIX
    #define CROLL_IMPLEMENTATION
    #include "../croll/croll.h"

    int main() {
        croll_init(); // used to init croll's functionalities

        croll_logInfo("Starting...\n");

        croll_logInfo("Enter a string: ");
        croll_StringBuilder sb = croll_HgetLine();

        croll_logInfo("You entered: %s\n", sb.data);

        croll_sbFree(&sb);

        croll_logInfo("Array test...\n");

        croll_daDecl(int,) arr = {0};

        for(int i = 0; i < 10; i++) {
            croll_daAppend(&arr, i);
        }

        croll_daForEach(int, it, &arr) {
            croll_logInfo("%lld.  %d\n", croll_daForEach_index(it, &arr), *it);
        }

        return 0;
    }
    ```

    Implementation macros:
        CROLL_IMPLEMENTATION, CROLL_HASHTABLE_IMPLEMENTATION
    Helper macros:
        CROLL_STRIP_PREFIX, CROLL_STATIC_FUNC, CROLL_MAX_TEXTFMT_BUFFERS, CROLL_TEXTFMT_BUFFER_SIZE

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
#include <string.h>

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

#ifdef CROLL_STATIC_FUNC
#define __STATIC_FUNCTION static
#else
#define __STATIC_FUNCTION
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
#define croll_daForEach_index(it, da) ((it) - (da)->data)

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
    struct croll_BumpAlloc *_next;
    croll_byte *data;
    size_t size;
    size_t offset;
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
typedef struct croll_PoolAlloc {
    struct croll_PoolAlloc *_next;
    void *free_chunks;
    void *chunks;
    size_t chunk_size;
    size_t size;
} croll_PoolAlloc;

// hashtable

struct croll_HtEntry {
    char *key;
    void *value; 
};

typedef struct {
    croll_PoolAlloc *str_allocator;
    size_t key_max_len;

    struct croll_HtEntry *entries;
    size_t cap;
    size_t len;
} croll_HashTable;

//======================================================================
// Strip prefix
//======================================================================

//======================================================================
// 1. Guards
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_GUARDS)
#define checkNullPtr croll_checkNullPtr
#define nullPtrGuard croll_nullPtrGuard
#endif

//======================================================================
// 2. Logging
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_LOG)
#define logInfo  croll_logInfo
#define logWarn  croll_logWarn
#define logError croll_logError
#endif

//======================================================================
// 3. I/O
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_IO)
#define HgetLine       croll_HgetLine
#define SgetLine       croll_SgetLine
#define readEntireFile croll_readEntireFile
#endif

//======================================================================
// 4. Text
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_TEXT)
#define textFmt       croll_textFmt
#define textSubString croll_textSubString
#endif

//======================================================================
// 5. Dynamic Array (DA)
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_DA)
#define daDecl(type,name)      croll_daDecl(type,name)
#define daFree(da)             croll_daFree(da)
#define daReserve(da,n)        croll_daReserve(da,n)
#define daAppend(da,x)         croll_daAppend(da,x)
#define daExtend(da,arr,count) croll_daExtend(da,arr,count)
#define daLast(da)             croll_daLast(da)
#define daForEach(type,it,da)  croll_daForEach(type,it,da)
#define daIndex(it,da)         croll_daForEach_index(it,da)
#endif

//======================================================================
// 6. String Builder
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_STRBUILDER)
#define sbDecl(name)         croll_sbDecl(name)
#define sbAppend(sb,ch)      croll_sbAppend(sb,ch)
#define sbExtend(sb,str,len) croll_sbExtend(sb,str,len)
#define sbFree(sb)           croll_sbFree(sb)
#endif

//======================================================================
// 7. Bump Allocator
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_BUMPALLOC)
#define bumpNew(cap)        croll_bumpNew(cap)
#define bumpAlloc(bump,n)   croll_bumpAlloc(bump,n)
#define bumpAllocOrExpand(bump,n) croll_bumpAllocOrExpand(bump,n)
#define bumpReset(bump)     croll_bumpReset(bump)
#define bumpDestroy(bump)   croll_bumpDestroy(bump)
#endif

//======================================================================
// 8. Pool Allocator
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_POOLALLOC)
#define poolNew(poolSz,chunkSz) croll_poolNew(poolSz,chunkSz)
#define poolAlloc(pool)         croll_poolAlloc(pool)
#define poolFree(pool,chunk)    croll_poolFree(pool,chunk)
#define poolDestroy(pool)       croll_poolDestroy(pool)
#endif

//======================================================================
// 9. Hash Table
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_HT)
#define htNew(keyMaxLen)    croll_htNew(keyMaxLen)
#define htDestroy(ht)       croll_htDestroy(ht)
#define htGet(ht,key)       croll_htGet(ht,key)
#define htSet(ht,key,val)   croll_htSet(ht,key,val)
#endif

//======================================================================
// 10. Hash Functions
//======================================================================
#if defined(CROLL_STRIP_PREFIX)
#define hashDjb2(str)       croll_hashDjb2(str)
#endif

//======================================================================
// 11. Types/Aliases
//======================================================================
#if defined(CROLL_STRIP_PREFIX) || defined(CROLL_STRIP_TYPES)
#define i8   croll_i8
#define u8   croll_u8
#define i16  croll_i16
#define u16  croll_u16
#define i32  croll_i32
#define u32  croll_u32
#define i64  croll_i64
#define u64  croll_u64
#define byte croll_byte
#endif 

//======================================================================
// Declaration of functions
//======================================================================
#ifdef __cplusplus
extern "C" {
#endif

//======================================================================
// Core Initialization
//======================================================================
/**
 * @brief Initializes the CRoll library.
 *
 * Must be called once before any other CRoll function.
 */
__STATIC_FUNCTION void croll_init(void);

//======================================================================
// Logging
//======================================================================
/**
 * @brief Logs an informational message to stdout.
 *
 * Prepends “[INFO]” in blue.
 *
 * @param format printf-style format string.
 * @param ...    format arguments.
 */
__STATIC_FUNCTION void croll_logInfo(const char *format, ...);

/**
 * @brief Logs a warning message to stderr.
 *
 * Prepends “[WARN]” in yellow.
 *
 * @param format printf-style format string.
 * @param ...    format arguments.
 */
__STATIC_FUNCTION void croll_logWarn(const char *format, ...);

/**
 * @brief Logs an error message to stderr.
 *
 * Prepends “[ERROR]” in red.
 *
 * @param format printf-style format string.
 * @param ...    format arguments.
 */
__STATIC_FUNCTION void croll_logError(const char *format, ...);

//======================================================================
// Text Formatting
//======================================================================
/**
 * @brief Formats a string into an internal rotating buffer.
 *
 * Not thread-safe.
 *
 * @param format printf-style format string.
 * @param ...    format arguments.
 * @return       Pointer to a null-terminated formatted string.
 */
__STATIC_FUNCTION char *croll_textFmt(const char *format, ...);

/**
 * @brief Returns a substring of an existing string.
 *
 * Uses croll_textFmt’s buffer.
 *
 * @param str   Source string.
 * @param start Index of first character (inclusive).
 * @param end   Index of last character (inclusive).
 * @return      Null-terminated substring.
 */
__STATIC_FUNCTION char *croll_textSubString(const char *str, size_t start, size_t end);

//======================================================================
// I/O
//======================================================================
/**
 * @brief Reads a line (until newline or EOF) into a StringBuilder.
 *
 * @return A croll_StringBuilder containing the line (null-terminated).
 */
__STATIC_FUNCTION croll_StringBuilder croll_HgetLine(void);

/**
 * @brief Reads a line into a user-provided buffer.
 *
 * Stops at newline or EOF, writes null terminator.
 *
 * @param buffer      User buffer.
 * @param buffer_size Size of buffer (must be >0).
 * @return            true on success, false if truncated.
 */
__STATIC_FUNCTION bool croll_SgetLine(char *buffer, size_t buffer_size);

/**
 * @brief Reads an entire file into a StringBuilder.
 *
 * @param path File path.
 * @return     croll_StringBuilder with file contents (null-terminated).
 */
__STATIC_FUNCTION croll_StringBuilder croll_readEntireFile(const char *path);

//======================================================================
// Memory Allocators
//======================================================================
/**
 * @brief Creates a new bump (arena) allocator.
 *
 * @param capacity Total size of the arena in bytes.
 * @return         Pointer to allocator or NULL on failure.
 */
__STATIC_FUNCTION croll_BumpAlloc *croll_bumpNew(size_t capacity);

/**
 * @brief Allocates memory from a bump allocator.
 *
 * @param bump Pointer to bump allocator.
 * @param size Number of bytes to allocate.
 * @return     Pointer to memory or NULL if out of space.
 */
__STATIC_FUNCTION void *croll_bumpAlloc(croll_BumpAlloc *bump, size_t size);

/**
 * @brief Resets a bump allocator (frees all in one go).
 *
 * @param bump Pointer to bump allocator.
 */
__STATIC_FUNCTION void croll_bumpReset(croll_BumpAlloc *bump);

/**
 * @brief Destroys a bump allocator and frees its memory.
 *
 * @param bump Pointer to bump allocator.
 */
__STATIC_FUNCTION void croll_bumpDestroy(croll_BumpAlloc *bump);

/**
 * @brief Creates a new pool allocator.
 *
 * @param pool_size  Number of chunks.
 * @param chunk_size Size of each chunk (>= sizeof(void*)).
 * @return           Pointer to allocator or NULL on failure.
 */
__STATIC_FUNCTION croll_PoolAlloc *croll_poolNew(size_t pool_size, size_t chunk_size);

/**
 * @brief Allocates one chunk from the pool.
 *
 * @param pool Pointer to pool allocator.
 * @return     Pointer to chunk or NULL if error.
 */
__STATIC_FUNCTION void *croll_poolAlloc(croll_PoolAlloc *pool);

/**
 * @brief Frees a chunk back to its pool.
 *
 * @param pool  Pointer to pool allocator.
 * @param chunk Pointer to previously allocated chunk.
 */
__STATIC_FUNCTION void croll_poolFree(croll_PoolAlloc *pool, void *chunk);

/**
 * @brief Destroys a pool allocator and frees all memory.
 *
 * @param pool Pointer to pool allocator.
 */
__STATIC_FUNCTION void croll_poolDestroy(croll_PoolAlloc *pool);

//======================================================================
// Hash Table
//======================================================================
/**
 * @brief Creates a new hash table.
 *
 * @param key_max_len Maximum key length (for internal pool).
 * @return            Pointer to table or NULL on failure.
 */
__STATIC_FUNCTION croll_HashTable *croll_htNew(size_t key_max_len);

/**
 * @brief Inserts or updates an entry in the hash table.
 *
 * @param ht    Pointer to hash table.
 * @param key   Null-terminated string key.
 * @param value Pointer to value.
 * @return      true on success, false on error.
 */
__STATIC_FUNCTION bool croll_htSet(croll_HashTable *ht, const char *key, void *value);

/**
 * @brief Retrieves a value by key.
 *
 * @param ht  Pointer to hash table.
 * @param key Null-terminated string key.
 * @return    Pointer to value or NULL if not found.
 */
__STATIC_FUNCTION void *croll_htGet(croll_HashTable *ht, const char *key);

/**
 * @brief Destroys a hash table and frees all memory.
 *
 * @param ht Pointer to hash table.
 */
__STATIC_FUNCTION void croll_htDestroy(croll_HashTable *ht);

//======================================================================
// Miscellaneous
//======================================================================
/**
 * @brief DJB2 string hash function.
 *
 * @param str Null-terminated string.
 * @return    Computed 32/64-bit hash.
 */
__STATIC_FUNCTION size_t croll_hashDjb2(const char *str);

#ifdef __cplusplus
}
#endif

//======================================================================
// Implementation
//======================================================================

#ifdef CROLL_IMPLEMENTATION

__STATIC_FUNCTION void croll_init() {
    croll_STDIN = stdin;
    croll_STDERR = stderr;
    croll_STDOUT = stdout;
}

// logging

__STATIC_FUNCTION void croll_logInfo(const char *format, ...) {
    fprintf(croll_STDOUT, "\033[94m[INFO]\033[0m ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

__STATIC_FUNCTION void croll_logWarn(const char *format, ...) {
    fprintf(croll_STDERR, "\033[93m[WARN]\033[0m ");
    va_list args;
    va_start(args, format);
    vfprintf(croll_STDERR, format, args);
    va_end(args);
}

__STATIC_FUNCTION void croll_logError(const char *format, ...) {
    fprintf(croll_STDERR, "\033[91m[ERROR]\033[0m ");
    va_list args;
    va_start(args, format);
    vfprintf(croll_STDERR, format, args);
    va_end(args);
}

// strings

__STATIC_FUNCTION char *croll_textFmt(const char *format, ...) {
    #ifndef CROLL_MAX_TEXTFMT_BUFFERS
    #define CROLL_MAX_TEXTFMT_BUFFERS 4
    #endif

    #ifndef CROLL_TEXTFMT_BUFFER_SIZE
    #define CROLL_TEXTFMT_BUFFER_SIZE 1024
    #endif

    static char buffers[CROLL_MAX_TEXTFMT_BUFFERS][CROLL_TEXTFMT_BUFFER_SIZE] = {0};
    static int index = 0;
    index = (index + 1) % CROLL_MAX_TEXTFMT_BUFFERS;
    
    memset(buffers[index], 0, CROLL_TEXTFMT_BUFFER_SIZE);

    va_list args;
    va_start(args, format);
    int size = vsnprintf(buffers[index], CROLL_TEXTFMT_BUFFER_SIZE, format, args);
    va_end(args);

    if(size >= CROLL_TEXTFMT_BUFFER_SIZE) {
        sprintf(buffers[index] + CROLL_TEXTFMT_BUFFER_SIZE - 4, "...");
    }

    return buffers[index];
}

__STATIC_FUNCTION char *croll_textSubString(const char *str, size_t start, size_t end) {
    char *nstr = croll_textFmt("%.*s", (int)((end + 1) - start), str + start);
    nstr[end + 1] = '\0';
    return nstr;
}

// IO

__STATIC_FUNCTION croll_StringBuilder croll_HgetLine() {
    croll_StringBuilder sb = {0};
    while (true) {
        char c = fgetc(croll_STDIN);
        if (c == EOF || c == '\n') break;
        croll_daAppend(&sb, c);
    }
    croll_daAppend(&sb, '\0'); // ensures null termination
    return sb;
}

__STATIC_FUNCTION bool croll_SgetLine(char *buffer, size_t buffer_size) {
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

__STATIC_FUNCTION croll_StringBuilder croll_readEntireFile(const char *path) {
    FILE *file = fopen(path, "r");
    croll_checkNullPtr(file) return (croll_StringBuilder){0};
    croll_StringBuilder sb = {0};

    while (true) {
        char c = fgetc(file);
        if (c == EOF) break;
        croll_sbAppend(&sb, c);
    }
    croll_sbAppend(&sb, '\0'); // ensures null termination
    fclose(file);
    return sb;
}

// memory

__STATIC_FUNCTION croll_BumpAlloc *croll_bumpNew(size_t cap) {
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

__STATIC_FUNCTION __CROLL_INLINE_ATTR void *croll_bumpAlloc(croll_BumpAlloc *bump, size_t size) {
    if(size == 0) return NULL;
    if(size & 0b111) size = croll_ALLOC_ALIGN(size);

    size_t cur_addr = (size_t)bump->data + bump->offset;
    if(cur_addr + size > (size_t)bump->data + bump->size)
        return NULL;
    
    bump->offset += size;
    return (void *)cur_addr;
}

__STATIC_FUNCTION __CROLL_INLINE_ATTR void *croll_bumpAllocOrExpand(croll_BumpAlloc *bump, size_t size) {
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

__STATIC_FUNCTION __CROLL_INLINE_ATTR void croll_bumpReset(croll_BumpAlloc *bump) {
    bump->offset = 0;
    while(bump->_next != NULL) {
        bump = bump->_next;
        bump->offset = 0;
    }
}

__STATIC_FUNCTION __CROLL_INLINE_ATTR void croll_bumpDestroy(croll_BumpAlloc *bump) {
    croll_checkNullPtr(bump) return;
    croll_checkNullPtr(bump->_next) {
        free(bump->data);
        free(bump);
        return;
    }
    while(bump->_next != NULL) {
        croll_BumpAlloc *next = bump->_next;
        free(bump->data);
        free(bump);
        bump = next;
    } 
}

// Djb2 hash function
__STATIC_FUNCTION size_t croll_hashDjb2(const char *str) {
    size_t hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

__STATIC_FUNCTION croll_PoolAlloc *croll_poolNew(size_t pool_size, size_t chunk_size) {
    if(pool_size == 0 || chunk_size < sizeof(void*))
    return NULL;
    chunk_size = croll_ALLOC_ALIGN(chunk_size);
    /*
    should somewhat loke like the following:
    struct chunk {
        union {
            void* next;
            void data[chunk_size];
        };
        croll_PoolAlloc *allocator;
    };
    */
    croll_PoolAlloc *pool = malloc(sizeof(croll_PoolAlloc));
    croll_checkNullPtr(pool) return NULL;

    pool->chunks = pool->free_chunks = malloc(pool_size * chunk_size);
    croll_checkNullPtr(pool->chunks) {
        free(pool);
        return NULL;
    }

    pool->_next = NULL;
    pool->chunk_size = chunk_size;
    pool->size = pool_size;

    // Reason why chunk_size >= sizeof(void*)
    croll_byte *chunks = pool->chunks;
    for(size_t i = 0; i < pool_size - 1; i++)
        // pool->chunks[i]->next = pool->chunks[i + 1];
        *(void**)(chunks + i * chunk_size) = chunks + (i + 1) * chunk_size;

    // pool->chunks[pool_size - 1]->next = NULL;
    *(void**)(chunks + (pool_size - 1) * chunk_size) = NULL;

    return pool;
}

__STATIC_FUNCTION __CROLL_INLINE_ATTR void croll_poolDestroy(croll_PoolAlloc *pool) {
    croll_checkNullPtr(pool) return;
    croll_checkNullPtr(pool->_next) {
        free(pool->chunks);
        free(pool);
        return;
    }

    while(pool->_next != NULL) {
        croll_PoolAlloc *next = pool->_next;
        free(pool->chunks);
        free(pool);
        pool = next;
    } 
}

__STATIC_FUNCTION void *croll_poolAlloc(croll_PoolAlloc *pool) {
    croll_checkNullPtr(pool) return NULL;
    croll_checkNullPtr(pool->free_chunks) {
        while(pool->_next != NULL) {
            pool = pool->_next;
            if(pool->free_chunks != NULL) return croll_poolAlloc(pool);
        }
        pool->_next = croll_poolNew(pool->size, pool->chunk_size - sizeof(croll_PoolAlloc *));
        pool = pool->_next;
    }

    void *chunk = pool->free_chunks;
    pool->free_chunks = *(void**)pool->free_chunks;

    return chunk;
}

__STATIC_FUNCTION __CROLL_INLINE_ATTR void croll_poolFree(croll_PoolAlloc *pool, void *chunk) {
    croll_checkNullPtr(pool) return;
    croll_checkNullPtr(chunk) return;

    *(void**)chunk = pool->free_chunks;
    pool->free_chunks = chunk;
}

#endif // CROLL_IMPLEMENTATION

#ifdef CROLL_HASHTABLE_IMPLEMENTATION
#define CROLL_HASHTABLE_INIT_CAP 64

__STATIC_FUNCTION croll_HashTable *croll_htNew(size_t key_max_len) {
    croll_HashTable *ht = (croll_HashTable *)malloc(sizeof(croll_HashTable));
    croll_checkNullPtr(ht)
        return NULL;
    memset(ht, 0, sizeof(croll_HashTable));

    ht->entries = calloc(CROLL_HASHTABLE_INIT_CAP, sizeof(struct croll_HtEntry));

    ht->key_max_len = key_max_len;
    ht->str_allocator = croll_poolNew(CROLL_HASHTABLE_INIT_CAP, key_max_len);
    croll_checkNullPtr(ht->str_allocator) {
        free(ht);
        return NULL;
    }

    ht->cap = CROLL_HASHTABLE_INIT_CAP;
    ht->len = 0;

    return ht;
}

__STATIC_FUNCTION void croll_htDestroy(croll_HashTable *ht) {
    for(size_t i = 0; i < ht->cap; i++) {
        croll_nullPtrGuard(ht->entries[i].key) {
            croll_poolFree(ht->str_allocator, ht->entries[i].key);
        }
    }

    croll_poolDestroy(ht->str_allocator);
    free(ht->entries);
    free(ht);
}

__STATIC_FUNCTION void *croll_htGet(croll_HashTable *ht, const char *key) {
    key = croll_textSubString(key, 0, ht->key_max_len);
    size_t hash = croll_hashDjb2(key);
    // same as index = hash % (ht->cap)
    size_t index = (size_t)(hash & (size_t)(ht->cap - 1));

    while(ht->entries[index].key != NULL) {
        if(strcmp(ht->entries[index].key, key) == 0)
            return ht->entries[index].value;

        index++;
        if(index >= ht->cap)
            // wrap around
            index = 0;
    }

    return NULL;
}

__STATIC_FUNCTION bool croll__htSetEntry(struct croll_HtEntry *entries, size_t cap, size_t *plen, const char *key, void *value, int key_max_len, croll_PoolAlloc *str_allocator) {
    key = croll_textSubString(key, 0, key_max_len);
    size_t hash = croll_hashDjb2(key);
    size_t index = (size_t)(hash & (size_t)(cap - 1));

    while(entries[index].key != NULL) {
        if(strcmp(entries[index].key, key) == 0) {
            entries[index].value = value;
            return true;
        }

        index++;
        if(index >= cap)
            index = 0;
    }

    if(plen != NULL) {
        char *nkey = croll_poolAlloc(str_allocator);
        croll_checkNullPtr(nkey) 
            return NULL;
        strcpy(nkey, key);
        key = nkey;

        (*plen)++;
    }

    entries[index].key = (char *)key;
    entries[index].value = value;

    return true;
}

__STATIC_FUNCTION bool croll__htExpand(croll_HashTable *ht) {
    size_t ncap = ht->cap * 2;
    if(ncap < ht->cap) // overflow check
        return false;
        
    struct croll_HtEntry *nentries = calloc(ncap, sizeof(struct croll_HtEntry));
    croll_checkNullPtr(nentries)
        return false;
    
    for(size_t i = 0; i < ht->cap; i++) {
        croll_nullPtrGuard(ht->entries[i].key)
            croll__htSetEntry(nentries, ncap, NULL, ht->entries[i].key, ht->entries[i].value, ht->key_max_len, ht->str_allocator);
    }

    free(ht->entries);
    ht->entries = nentries;
    ht->cap = ncap;

    return true;
}

__STATIC_FUNCTION bool croll_htSet(croll_HashTable *ht, const char *key, void *value) {
    if(value == NULL)
        return false;
    
    if(ht->len >= (ht->cap - (ht->cap >> 2)))
        if(!croll__htExpand(ht))
            return false;
    
    return croll__htSetEntry(ht->entries, ht->cap, &ht->len, key, value, ht->key_max_len, ht->str_allocator);
}
#endif // CROLL_HASHTABLE_IMPLEMENTATION

#endif // LIB_CROLLing_H