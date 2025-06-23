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
