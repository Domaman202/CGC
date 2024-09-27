#include "cgc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// TEST

typedef struct {
    cgc_ptr_t __cgc;
    char* text;
} test_obj_a;

typedef struct {
    cgc_ptr_t __cgc;
    void* f_ref;
} test_obj_b;

typedef struct {
    cgc_ptr_t __cgc;
    void* f_left;
    void* f_right;
} test_obj_c;

CGC_DEFINE_DEALLOC(test_obj_a, {
    printf("[Dealloc](%d)(%d) (Test Object A) %s\n", ptr->__cgc.debug_left, ptr->__cgc.debug_right, ptr->text);
    free(ptr->text);
})

CGC_DEFINE_DEALLOC(test_obj_b, {
    printf("[Dealloc](%d)(%d) (Test Object B) %lu\n", ptr->__cgc.debug_left, ptr->__cgc.debug_right, (uintptr_t) ptr);
})

CGC_DEFINE_DEALLOC(test_obj_c, {
    printf("[Dealloc](%d)(%d) (Test Object C) %lu\n", ptr->__cgc.debug_left, ptr->__cgc.debug_right, (uintptr_t) ptr);
})

CGC_DEFINE_ALLOC(test_obj_a, 0, (const cgc_t* gc, const char* text), {
    ptr->text = strdup(text);
})

CGC_DEFINE_ALLOC(test_obj_b, 1, (const cgc_t* gc,  void* ref), {
    ptr->f_ref = ref;
})

CGC_DEFINE_ALLOC(test_obj_c, 2, (const cgc_t* gc, void* left, void* right), {
    ptr->f_left = left;
    ptr->f_right = right;
})

int main(void) {
    // cgc init
    cgc_t* gc = malloc(sizeof(cgc_t));
    cgc_init(gc);

    // TEST BEGIN

    // init left
    test_obj_a* o11 = CGC_PTR_ALLOC(test_obj_a, gc, "left first");
    CGC_DBG_INFO_INIT(o11, 1, 1);
    test_obj_a* o12 = CGC_PTR_ALLOC(test_obj_a, gc, "left second");
    CGC_DBG_INFO_INIT(o12, 1, 2);
    test_obj_c* o13 = CGC_PTR_ALLOC(test_obj_c, gc, o11, o12);
    CGC_DBG_INFO_INIT(o13, 1, 3);
    // init right
    test_obj_a* o21 = CGC_PTR_ALLOC(test_obj_a, gc, "right first");
    CGC_DBG_INFO_INIT(o21, 2, 1);
    test_obj_a* o22 = CGC_PTR_ALLOC(test_obj_a, gc, "right second");
    CGC_DBG_INFO_INIT(o22, 2, 2);
    test_obj_c* o23 = CGC_PTR_ALLOC(test_obj_c, gc, o21, o22);
    CGC_DBG_INFO_INIT(o23, 2, 3);
    // init center
    CGC_VAR_ALLOC(test_obj_c, ptr, gc, o13, o23);

    // ref
    // CGC_PTR_ENTER(ptr);
    // collect #1
    printf("Collection #1\n");
    cgc_gc(gc);
    // unref
    CGC_PTR_EXIT(ptr);
    // collect #2
    printf("Collection #2\n");
    cgc_gc(gc);

    // TEST END

    // cgc deinit
    cgc_deinit(gc);
    free(gc);
    return 0;
}
