#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define TESTGROUP(label) printf("[Test Group]: %s\n", label);

// We only want the expression (val1 or val2) to be evaluated once, but
// we still want to get the value of the result of the function.
// We also want the stringified version of the expression, which 
// We can only get from a macro.
#define TESTINTEQ(val1, val2)\
    do{\
        uintptr_t _val1 = (val1), _val2 = (val2);\
        if (_val1 != _val2){\
            printf("[Test]: %s @ %u FAIL! %s = %lx, %s = %lx \n", __FILE__, __LINE__, #val1, _val1, #val2, _val2);\
            exit(1);\
        }\
    }while(0)

// ahhhh C, reducing code duplication by casting pointers to integers
// (sarcastic)
#define TESTPTREQ(ptr1, ptr2) TESTINTEQ((uintptr_t)ptr1, (uintptr_t)ptr2)

#define TESTINTNEQ(val1, val2)\
    do{\
        uintptr_t _val1 = (val1), _val2 = (val2);\
        if (_val1 == _val2){\
            printf("[Test]: %s @ %u FAIL! %s = %lx, %s = %lx \n", __FILE__, __LINE__, #val1, _val1, #val2, _val2);\
            exit(1);\
        }\
    }while(0)

#define TESTPTRNEQ(ptr1, ptr2) TESTINTNEQ((uintptr_t)ptr1, (uintptr_t)ptr2)
