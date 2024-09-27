#include "cgc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// TEST

typedef struct {
    cgc_obj_info_t cgc_info;
    char* text;
} test_obj_a;

typedef struct {
    cgc_obj_info_t cgc_info;
    void* f_ref;
} test_obj_b;

typedef struct {
    cgc_obj_info_t cgc_info;
    void* f_left;
    void* f_right;
} test_obj_c;

CGC_DEFINE_DEALLOC(test_obj_a, {
    printf("[Dealloc](%d)(%d) (Test Object A) %s\n", ptr->cgc_info.debug_left, ptr->cgc_info.debug_right, ptr->text);
    free(ptr->text);
})

CGC_DEFINE_DEALLOC(test_obj_b, {
    printf("[Dealloc](%d)(%d) (Test Object B) %lu\n", ptr->cgc_info.debug_left, ptr->cgc_info.debug_right, (uintptr_t) ptr);
})

CGC_DEFINE_DEALLOC(test_obj_c, {
    printf("[Dealloc](%d)(%d) (Test Object C) %lu\n", ptr->cgc_info.debug_left, ptr->cgc_info.debug_right, (uintptr_t) ptr);
})

CGC_DEFINE_ALLOC(test_obj_a, 0, (const char* text), {
    ptr->text = strdup(text);
})

CGC_DEFINE_ALLOC(test_obj_b, 1, (void* ref), {
    ptr->f_ref = ref;
})

CGC_DEFINE_ALLOC(test_obj_c, 2, (void* left, void* right), {
    ptr->f_left = left;
    ptr->f_right = right;
})

int main(void) {
    // cgc init
    cgc_init();

    // TEST BEGIN

    // init left
    test_obj_a* o11 = CGC_PTR_ALLOC(test_obj_a, "left first");
    CGC_DBG_INFO_INIT(o11, 1, 1);
    test_obj_a* o12 = CGC_PTR_ALLOC(test_obj_a, "left second");
    CGC_DBG_INFO_INIT(o12, 1, 2);
    test_obj_c* o13 = CGC_PTR_ALLOC(test_obj_c, o11, o12);
    CGC_DBG_INFO_INIT(o13, 1, 3);
    // init right
    test_obj_a* o21 = CGC_PTR_ALLOC(test_obj_a, "right first");
    CGC_DBG_INFO_INIT(o21, 2, 1);
    test_obj_a* o22 = CGC_PTR_ALLOC(test_obj_a, "right second");
    CGC_DBG_INFO_INIT(o22, 2, 2);
    test_obj_c* o23 = CGC_PTR_ALLOC(test_obj_c, o21, o22);
    CGC_DBG_INFO_INIT(o23, 2, 3);
    // init center
    CGC_VAR_ALLOC(test_obj_c, ptr, o13, o23);

    // ref
    // CGC_PTR_ENTER(ptr);
    // collect #1
    printf("Collection #1\n");
    cgc_gc();
    // unref
    CGC_PTR_EXIT(ptr);
    // collect #2
    printf("Collection #2\n");
    cgc_gc();

    // TEST END

    // return
    return 0;
}
