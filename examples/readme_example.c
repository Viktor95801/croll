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
