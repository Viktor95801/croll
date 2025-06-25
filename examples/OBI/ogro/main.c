#define CROLL_IMPLEMENTATION
#include "croll.h"

int main() {
    croll_logInfo("Left hand: ");
    int L; scanf("%d", &L);
    if(L < 0 || L > 5) return 1;
    croll_logInfo("Right hand: ");
    int R; scanf("%d", &R);
    if(R < 0 || R > 5) return 1;

    if(L > R)
        croll_logInfo("%d > %d. R: %d\n", L, R, L + R);
    else
        croll_logInfo("%d < %d. R: %d\n", L, R, 2 * (R - L));

    return 0;
}