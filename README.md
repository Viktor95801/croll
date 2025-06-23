# What is this?

This is a header-only stb-style library for higger-level C interfaces and utility functions for working with strings, arrays, and memory, made to get your C rolling.
This is a work in progress and highly experimental, so the API changes will be constant.

## Resources

- [Documentation](./DOCUMENTATION.md)
- [Examples](./examples)

Highly inspired by [Tsoding's nob](https://github.com/tsoding/nob.h/blob/master/nob.h), [Tsoding's video about arenas](https://www.youtube.com/watch?v=UYLHA_Ey8Ys&pp=ugMICgJwdBABGAHKBRdhcmVuYSBhbGxvY2F0b3IgdHNvZGluZw%3D%3D) and [base.h](https://github.com/TomasBorquez/base.h/blob/master/base.h), motivated by the sheer amount of code I've written in C without utility libs.

## Usage

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
