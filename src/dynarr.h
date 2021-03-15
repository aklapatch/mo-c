#pragma once

#include<stdint.h>
#include<stddef.h>
#include<string.h>
#include <stdio.h>
#include <stdbool.h>

// allow replacement of the allocator by the programmer
#ifndef C_DS_REALLOC
#include <stdlib.h>
#define C_DS_REALLOC realloc
#endif

typedef enum ds_error_e {
    ds_success = 0,
    ds_alloc_fail,
    ds_out_of_bounds,
    ds_null_ptr,
    ds_bad_param,
    ds_fail, // That one error code you hate getting because you don't know what went wrong
    ds_num_errors,
} ds_error_e;

#define RET_SWITCH_STR(x) case (x): return (#x);

char * ds_get_err_str(ds_error_e err){
    switch (err){
        RET_SWITCH_STR(ds_success);
        RET_SWITCH_STR(ds_alloc_fail);
        RET_SWITCH_STR(ds_out_of_bounds);
        RET_SWITCH_STR(ds_null_ptr);
        RET_SWITCH_STR(ds_bad_param);
        RET_SWITCH_STR(ds_fail);
        RET_SWITCH_STR(ds_num_errors);
        default:
            return "No matching error found!\n";
    }
}

// General strategy:
// - Use functions to typecheck as much as possible (pointers mostly)
//   this means that every macro should point to at least one function.

// TODO:
// - make dynarr:
//   - revise so that macros only handle code that mutates the host pointer.
//   - add a feature to init over a static buffer and transition to heap-allocated memory.
//   - set append apis to use insert apis X
//      - api list:
    //      - alloc x
    //      - realloc x
    //      - free X
    //      - addn x
    //      - append X
    //      - pop X
    //      - maybegrow X
    //      - arrdel X
    //      - arrdeln X
    //      - insertn x
    //      - insert x
// - then make hashmap
// - then make string hashmap

// The dynamic array has a info struct that is allocated behind the pointer the user provides.
typedef struct dynarr_info {
    uintptr_t len,cap;
    ds_error_e err;
} dynarr_info;

dynarr_info * get_dynarr_info(void * ptr){
    return (ptr == NULL) ? NULL : ((dynarr_info*)ptr) - 1;
}

uintptr_t get_dynarr_len(void *ptr){
    return (ptr == NULL) ? 0 : get_dynarr_info(ptr)->len;
}

uintptr_t get_dynarr_cap(void *ptr){
    return (ptr == NULL) ? 0 : get_dynarr_info(ptr)->cap;
}

void dynarr_set_err(void * ptr, ds_error_e err){
    if (ptr != NULL){
        get_dynarr_info(ptr)->err = err;
    }
}

ds_error_e get_dynarr_err(void* ptr){
    return (ptr == NULL) ? ds_null_ptr : get_dynarr_info(ptr)->err;
}

bool dynarr_is_err_set(void * ptr){
    return get_dynarr_err(ptr) != ds_success;
}

char * dynarr_get_err_str(void* ptr){
    return ds_get_err_str(get_dynarr_err(ptr));
}

void dynarr_print(void *ptr){
    printf("dynarr: ptr=%p, base=%p, len=%u, cap=%u\n",ptr,get_dynarr_info(ptr), get_dynarr_len(ptr),get_dynarr_cap(ptr));
}

// This should never be used to free anything because it automatically adds the size of the info struct,
// to the allocated amount.
void* bare_dynarr_realloc(void * ptr,uintptr_t item_count, uintptr_t item_size){

    dynarr_info *base_ptr = NULL;
    if (ptr != NULL){
        base_ptr = get_dynarr_info(ptr);
    }

    // only allocate if we need to
    if (get_dynarr_cap(ptr) < item_count){
        dynarr_info *new_ptr = C_DS_REALLOC(base_ptr, item_count*item_size + sizeof(dynarr_info));
        if (new_ptr != NULL){
            new_ptr->cap = item_count;
            if (base_ptr == NULL){
                new_ptr->len = 0;
            }
            new_ptr->err = ds_success;
            ++new_ptr;
            return new_ptr;
        }
        else {
            // return the old pointer if we don't get memory
            dynarr_set_err(ptr, ds_alloc_fail);
            return ptr;
        }
    }
    else {
        // no allocation was needed, but set success anyway.
        dynarr_set_err(ptr, ds_success);
        return ptr;
    }
}

// allocates memory, sets capacity and nothing else
#define dynarr_alloc(type, item_count) bare_dynarr_realloc(NULL, item_count, sizeof(type));

// absorb the pointer normally emitted by reallocing
#define dynarr_free(ptr)\
    do{\
        void * __absorb__realloc__ret__ptr = C_DS_REALLOC(get_dynarr_info(ptr), 0);\
    } while (0)

#define dynarr_set_cap(ptr, new_cap) (ptr) = bare_dynarr_realloc((ptr), (new_cap), sizeof(*(ptr)));

#define dynarr_set_len(ptr, new_len) \
    do {\
        if (new_len > get_dynarr_cap(ptr)){\
            dynarr_set_cap(ptr, new_len);\
            if (get_dynarr_cap(ptr) >= new_len){\
                get_dynarr_info(ptr)->len = new_len;\
                dynarr_set_err(ptr, ds_success);\
            } \
        } else { \
            get_dynarr_info(ptr)->len = new_len; \
            dynarr_set_err(ptr, ds_success);\
        }\
    }while (0)

void* bare_dynarr_maybe_grow(void * ptr, uintptr_t new_count, uintptr_t item_size){
    uintptr_t cap = get_dynarr_cap(ptr);
    if (cap < new_count){
        // use a growth factor of around 1.5
        while (cap < new_count){
            cap += (cap >> 1) + 8;
        }
        return bare_dynarr_realloc(ptr, cap, item_size);
    }
    else {
        dynarr_set_err(ptr, ds_success);
        return ptr;
    }
}

#define dynarr_maybe_grow(ptr, new_count) (ptr) = bare_dynarr_maybe_grow((ptr), (new_count), sizeof(*(ptr)))


void bare_dyarr_deln(void* ptr, uintptr_t del_i, uintptr_t n, uintptr_t item_size){
    if (del_i + n <= get_dynarr_len(ptr)){
        uint8_t *start_ptr = (uint8_t*)(ptr  + item_size*del_i);
        uint8_t *end_ptr = start_ptr + item_size*n;
        memmove(start_ptr, end_ptr, item_size*(get_dynarr_len(ptr) - del_i - n));
        get_dynarr_info(ptr)->len -= n;
        dynarr_set_err(ptr, ds_success);
    }
    else {
        dynarr_set_err(ptr, ds_out_of_bounds);
    }
}

#define dynarr_deln(ptr, del_i, n) bare_dyarr_deln(ptr, del_i, n, sizeof(*ptr))

#define dynarr_del(ptr, del_i) dynarr_deln(ptr, del_i, 1)

#define dynarr_pop(ptr) dynarr_del(ptr, get_dynarr_len(ptr) - 1)

void bare_dyarr_insertn(void* ptr, void* items, uintptr_t start_i, uintptr_t n, uintptr_t item_size){
    if (get_dynarr_cap(ptr) >= get_dynarr_len(ptr) + n && start_i <= get_dynarr_len(ptr)){
        uint8_t *cpy_start = ((uint8_t*)ptr) + start_i*item_size;
        uint8_t * move_end = cpy_start + n*item_size;
        memmove(move_end, cpy_start, (get_dynarr_len(ptr) - start_i)*item_size );
        memcpy(cpy_start, items, n*item_size);
        get_dynarr_info(ptr)->len += n;
        dynarr_set_err(ptr, ds_success);
    }
    else if (start_i >= get_dynarr_len(ptr) && get_dynarr_err(ptr) != ds_alloc_fail) {
        dynarr_set_err(ptr, ds_out_of_bounds);
    }
}
#define dynarr_insertn(ptr, items, start_i, n)\
    do{\
        dynarr_maybe_grow(ptr, get_dynarr_len(ptr) + n);\
        bare_dyarr_insertn(ptr, items, start_i, n, sizeof(*ptr));\
    }while(0)

// I could make this a function, but then it would not be able to take integer literals
#define dynarr_insert(ptr, item, start_i)\
    do{\
        dynarr_maybe_grow(ptr, get_dynarr_len(ptr) + 1);\
        if (get_dynarr_cap(ptr) >= get_dynarr_len(ptr) + 1 && start_i <= get_dynarr_len(ptr)){\
            memmove(&ptr[start_i + 1], &ptr[start_i], sizeof(*ptr)*(get_dynarr_len(ptr) - start_i));\
            ptr[start_i] = item;\
            ++get_dynarr_info(ptr)->len;\
            dynarr_set_err(ptr, ds_success);\
        }else if (start_i >= get_dynarr_len(ptr) && get_dynarr_err(ptr) != ds_alloc_fail){\
            dynarr_set_err(ptr, ds_out_of_bounds);\
        }\
    }while(0)

#define dynarr_appendn(ptr, items, n) dynarr_insertn(ptr, items, get_dynarr_len(ptr), n)

#define dynarr_append(ptr, item) dynarr_insert(ptr, item, get_dynarr_len(ptr))
