#pragma once

#include"hmap.h"
#include "ahash.h"
#include "test_helpers.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define TIMES (3*UINT16_MAX)
// bench insertion, lookup, at least.
// Benching deletion doesn't make too much sense.
int main(){

    // init hmap to minimum size with a reasonable sized payload type
    uint32_t *hmap = NULL;
    hm_init(hmap, 16, realloc, ahash_buf);

    clock_t start = clock();
    for (uint32_t i = 0; i < TIMES; ++i){
        hm_set(hmap, i, i);
        if (hm_is_err_set(hmap)){
            printf("Insert failed!\n");
            exit(1);
        }
    }
    clock_t end = clock();
    printf("%u inserts took %g sec\n",TIMES, (double)(end-start)/CLOCKS_PER_SEC);
    hm_free(hmap);

    return 0;
}