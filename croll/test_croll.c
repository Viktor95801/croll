// dev_test_croll.c
#define CROLL_STRIP_PREFIX
#define CROLL_IMPLEMENTATION
#define CROLL_POOL_ALLOC_IMPLEMENTATION
#include "croll.h"

#include <string.h>
#include <stdio.h>

int total_tests = 0;
int failed_tests = 0;

#define TEST(name) do { \
    printf("[TEST] %s\n", #name); \
    total_tests++; \
    if (!(name())) { \
        printf("   X Failed: %s\n", #name); \
        failed_tests++; \
    } else printf("   V Passed: %s\n", #name); \
} while(0)

#define ASSERT(cond) \
    do { if (!(cond)) { printf("      Assertion failed: %s\n", #cond); return false; } } while (0)

// Test logging functions (visually)
bool test_logging() {
    croll_logInfo("This is an info message\n");
    croll_logWarn("This is a warning message\n");
    croll_logError("This is an error message\n");
    return true; // Just visual
}

bool test_da_append() {
    croll_daDecl(int,) arr = {0};

    for (int i = 0; i < 10; i++)
        croll_daAppend(&arr, i);

    ASSERT(arr.len == 10);
    for (int i = 0; i < 10; i++)
        ASSERT(arr.data[i] == i);

    ASSERT(croll_daLast(&arr) == 9);

    croll_daFree(&arr);
    return true;
}

bool test_da_extend() {
    croll_daDecl(int, ) arr = {0};
    int values[] = {42, 43, 44};
    croll_daExtend(&arr, values, 3);

    ASSERT(arr.len == 3);
    ASSERT(arr.data[0] == 42 && arr.data[2] == 44);

    croll_daFree(&arr);
    return true;
}

bool test_da_foreach() {
    croll_daDecl(int, ) arr = {0};
    for (int i = 0; i < 5; i++)
        croll_daAppend(&arr, i);

    int sum = 0;
    croll_daForEach(int, it, &arr) {
        sum += *it;
    }

    ASSERT(sum == 0 + 1 + 2 + 3 + 4);
    croll_daFree(&arr);
    return true;
}

bool test_stringbuilder() {
    croll_StringBuilder sb = {0};
    const char *hello = "hello";
    croll_sbExtend(&sb, hello, strlen(hello));
    croll_sbAppend(&sb, '\0');

    ASSERT(strcmp(sb.data, "hello") == 0);

    croll_sbFree(&sb);
    return true;
}

bool test_sgetline() {
    char input[] = "hello";

    printf("The key is: %s\n", input);
    printf("Enter the key: ");
    char buffer[10];
    bool success = croll_SgetLine(buffer, sizeof(buffer));
    ASSERT(success);
    ASSERT(strcmp(buffer, "hello") == 0);
    return true;
}

bool test_hgetline() {
    char input[] = "world";

    printf("The key is: %s\n", input);
    printf("Enter the key: ");
    croll_StringBuilder sb = croll_HgetLine();
    croll_sbAppend(&sb, '\0');

    ASSERT(strcmp(sb.data, "world") == 0);

    croll_sbFree(&sb);
    return true;
}

bool test_bumpallocator() {
    croll_BumpAlloc *arena = croll_bumpNew(30);
    ASSERT(arena != NULL);

    char *ptr1 = (char *)croll_bumpAllocOrExpand(arena, 10);
    ASSERT(ptr1 != NULL);

    ptr1 = "fizzbuzz";

    char *ptr2 = (char *)croll_bumpAllocOrExpand(arena, 20);
    ASSERT(ptr2 != NULL);

    ptr2 = "foobar";

    ASSERT(arena->_next != NULL);

    printf("%s %s\n", ptr1, ptr2);

    croll_BumpAlloc *a = arena;
    int i;
    for(;;) {
        printf("%d. Arena {.data=%p, .size=%lld, .offset=%lld, .next=%p}\n", i++, a->data, a->size, a->offset, a->_next);
        a = a->_next;
        if(a->_next == NULL) {
            printf("%d. Arena {.data=%p, .size=%lld, .offset=%lld, .next=%p}\n", i++, a->data, a->size, a->offset, a->_next);
            break;
        }
    }

    croll_bumpReset(arena);
    ASSERT(arena->_next->offset == 0);
    ASSERT(arena->offset == 0);

    croll_bumpDestroy(arena);
    return true;
}

bool test_poolallocator() {
    croll_PoolAlloc *pool = croll_poolNew(1, 16);

    char *hello = croll_poolAlloc(pool);
    ASSERT(hello != NULL);
    strcpy(hello, "Hello, ");

    char *world = croll_poolAlloc(pool);
    ASSERT(world != NULL);
    strcpy(world, "World!");

    logInfo("%s %s\n", hello, world);

    croll_poolFree(pool, hello);
    croll_poolFree(pool, world);   
    croll_poolDestroy(pool);

    return true;
}

bool test_hash() {
    size_t hash = croll_hashDjb2("hello");
    croll_UNUSED(hash);
    return true;
}

void test_all() {
    TEST(test_logging);
    TEST(test_da_append);
    TEST(test_da_extend);
    TEST(test_da_foreach);
    TEST(test_stringbuilder);
    TEST(test_sgetline);
    TEST(test_hgetline);
    TEST(test_bumpallocator);
    TEST(test_poolallocator);
    TEST(test_hash);
}

int main(void) {
    croll_init();
    logInfo("Running tests...\n");
    
    test_all();

    printf("\nTest summary: %d total, %d failed.\n", total_tests, failed_tests);

    return failed_tests ? 1 : 0;
}
