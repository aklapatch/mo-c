#include "hmap.h"
#include "test_helpers.h"

#define NUM_KEYS (31)
int main(){
    
    uint16_t *hmap = NULL;
    hm_init(hmap, 32, ant_hash);

    TEST_GROUP("Basic init");
    TEST_INT_EQ(hm_num(hmap), 0);
    TEST_INT_EQ(hm_err(hmap), ds_success);
    TEST_INT_EQ(hm_cap(hmap), 32);
    TEST_GROUP_OK();

    // insert a lot of values and see how this goes.
    TEST_GROUP("Insert a few keys");
    for (uint16_t i = 0; i < NUM_KEYS; ++i){
        // insert the value
        hm_set(hmap, i, i);
        TEST_INT_EQ(hm_err(hmap), ds_success);

        uint16_t out_val = UINT16_MAX;
        hm_get(hmap, i, out_val);
        TEST_INT_EQ(hm_err(hmap), ds_success);
        TEST_INT_EQ(out_val, i);
    }

    // make sure all values didn't get overwritten
    for (uint16_t i = 0; i < NUM_KEYS; ++i){
        uint16_t out_val = UINT16_MAX;
        hm_get(hmap, i, out_val);
        TEST_INT_EQ(hm_err(hmap), ds_success);
        TEST_INT_EQ(out_val, i);
    }
    TEST_GROUP_OK();

    TEST_GROUP("Realloc key preservation");
    hm_grow(hmap, 64);
    // try resizing the hmap and see if all the key are still there
    for (uint16_t i = 0; i < NUM_KEYS; ++i){
        uint16_t out_val = UINT16_MAX;
        hm_info_ptr(hmap)->tmp_val_i = hm_find_val_i(hmap, i);
        if (hm_info_ptr(hmap)->tmp_val_i != UINTPTR_MAX){\
            out_val= hmap[hm_info_ptr(hmap)->tmp_val_i];\
        }
        TEST_INT_EQ(hm_err(hmap), ds_success);
        TEST_INT_EQ(out_val, i);
    }
    TEST_GROUP_OK();

    // try deleting all the keys and make sure they're gone
    TEST_GROUP("Ensure deletion");
    for (uint16_t i = 0; i < NUM_KEYS; ++i){
        hm_del(hmap, i);
        TEST_INT_EQ(hm_err(hmap), ds_success);

        uint16_t out_val = UINT16_MAX;
        hm_get(hmap, i, out_val);
        TEST_INT_EQ(hm_err(hmap), ds_not_found);
    }
    TEST_GROUP_OK();

    TEST_GROUP("hmap free");
    hm_free(hmap);
    TEST_PTR_EQ(hmap, NULL);
    TEST_GROUP_OK();

    TEST_GROUP("Bulk insert");
    hm_init(hmap, 32, ant_hash);
    // insert a stupid number of keys and see if it still works
    for (uint16_t i = 0; i < UINT16_MAX; ++i){
        printf("key=%u\n", i);
        hm_set(hmap, i, i);
        TEST_INT_EQ(hm_err(hmap), ds_success);

        uint16_t out_val = UINT16_MAX;
        hm_get(hmap, i, out_val);
        TEST_INT_EQ(hm_err(hmap), ds_success);
        TEST_INT_EQ(out_val, i);
    }
    for (uint16_t i = 0; i < UINT16_MAX; ++i){
        uint16_t out_val = UINT16_MAX;
        printf("key=%u\n", i);
        hm_get(hmap, i, out_val);
        TEST_INT_EQ(hm_err(hmap), ds_success);
        TEST_INT_EQ(out_val, i);
    }
    TEST_GROUP_OK();

    hm_free(hmap);

    return 0;
}
