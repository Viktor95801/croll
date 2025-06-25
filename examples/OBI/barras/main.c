#define CROLL_IMPLEMENTATION
#include "croll.h"

int main(void) {
    croll_init();
    
    croll_logInfo("Starting...\n");

    char buf[32];
    croll_logInfo("Enter the number of cols: ");
    croll_SgetLine(buf, sizeof(buf));
    croll_i32 cols = strtol(buf, NULL, 10);

    croll_logInfo("Enter the data: ");
    croll_StringBuilder data = croll_HgetLine();

    char *token = strtok(data.data, " ");
    croll_daDecl(int,) bars = {0};
    {
        int i = 0;
        while(token != NULL) {
            croll_i32 number = strtol(token, NULL, 10);
            croll_daAppend(&bars, number);
            token = strtok(NULL, " ");
            i++;
        }
        if(i != cols) {
            croll_logError("Invalid input\n");
            croll_sbFree(&data);
            croll_daFree(&bars);
            return 1;
        }
    }

    int max = 0;
    croll_daForEach(int, it, &bars) {
        int tmp = *it;
        if(tmp > max) max = tmp;
    }

    int **matrix = calloc(cols, sizeof(char*));
    for(int i = 0; i < cols; i++)
        matrix[i] = calloc(max, sizeof(char));

    for(int col = 0; col < cols; col++) {
        for(int bar = 0; bar < max; bar++) {
            char one_or_zero = bar < bars.data[col] ? '1' : '0';
            matrix[col][max - (bar + 1)] = one_or_zero;
        }
    }

    for(int bar = 0; bar < max; bar++) {
        for(int col = 0; col < cols; col++) {
            printf("%c", matrix[col][bar]);
        }
        printf("\n");
    }

    for(int i = 0; i < cols; i++)
        free(matrix[i]);
    free(matrix);
    croll_sbFree(&data);
    croll_daFree(&bars);
    return 0;
}
