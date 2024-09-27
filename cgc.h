#ifndef __CGC_H__
#define __CGC_H__

#include <stdint.h>

// CGC (Constants)

#ifndef CGC_MARK_STACK_SIZE
#define CGC_MARK_STACK_SIZE (256*4)
#endif

// CGC (Typedef)

typedef void (*cgc_dealloc_t)(void*);

typedef struct {
    struct cgc_ptr_t* prev;
    cgc_dealloc_t dealloc;
    uint16_t ref_count;
    uint16_t field_count;
    uint16_t gc_mark;
    // // DEBUG // //
    uint32_t debug_left;
    uint32_t debug_right;
} cgc_ptr_t;

typedef struct {
    cgc_ptr_t* head_obj;
    uint16_t gc_age;
    uint16_t obj_stack_size;
} cgc_t;

// CGC (GC Utils)

cgc_t* cgc_alloc();
void cgc_free(cgc_t*);
void cgc_gc(cgc_t*);

// CGC (Reference Utils)

void cgc_ref(cgc_ptr_t* ptr);
void cgc_unref(cgc_ptr_t* ptr);

// CGC (Defines)

#define CGC_DEFINE_ALLOC(TYPE, FIELD_COUNT, ARGS, BODY) \
    TYPE* TYPE##_alloc ARGS { \
        TYPE* ptr = malloc(sizeof(TYPE)); \
        CGC_INFO_INIT(gc, TYPE, ptr, (FIELD_COUNT)); \
        (BODY); \
        return ptr; \
    }

#define CGC_INFO_INIT(GC, TYPE, PTR, FIELD_COUNT) \
    { \
        cgc_ptr_t* __cgc_ptr = (void*) (PTR); \
        __cgc_ptr->prev = (void*) gc->head_obj->prev; \
        gc->head_obj->prev = (void*) __cgc_ptr; \
        __cgc_ptr->dealloc = (void*) TYPE##_dealloc; \
        __cgc_ptr->ref_count = 0; \
        __cgc_ptr->field_count = (FIELD_COUNT); \
        __cgc_ptr->gc_mark = 0; \
    }

#define CGC_DBG_INFO_INIT(PTR, LEFT, RIGHT) \
    { \
        cgc_ptr_t* __cgc_ptr = (void*) (PTR); \
        __cgc_ptr->debug_left = (LEFT); \
        __cgc_ptr->debug_right = (RIGHT); \
    }

#define CGC_DEFINE_DEALLOC(TYPE, BODY) \
    void TYPE##_dealloc(TYPE* ptr) { \
        (BODY); \
        free(ptr); \
    }

#define CGC_VAR_ALLOC(TYPE, NAME, ...) \
    TYPE* NAME = CGC_PTR_ALLOC(TYPE, __VA_ARGS__); \
    CGC_PTR_ENTER(NAME);

#define CGC_PTR_ALLOC(TYPE, ...) TYPE##_alloc(__VA_ARGS__)
#define CGC_PTR_ENTER(PTR) cgc_ref((void*) (PTR))
#define CGC_PTR_EXIT(PTR) cgc_unref((void*) (PTR))

#endif // __CGC_H__
