#ifndef __CGC_H__
#define __CGC_H__

#include <stdint.h>

// CGC

// CGC (Typedef)

typedef void (*cgc_dealloc_t)(void*);

typedef struct {
    struct cgc_obj_info_t* prev;
    cgc_dealloc_t dealloc;
    uint16_t ref_count;
    uint16_t field_count;
    uint8_t gc_mark;
    // uint8_t reserved; // alignment free space
    uint8_t debug_left : 4;
    uint8_t debug_right : 4;
} cgc_obj_info_t;

// CGC (GC Utils)

extern cgc_obj_info_t* cgc_last_obj;
extern uint8_t cgc_gc_age;

void cgc_mark(cgc_obj_info_t* ptr);
void cgc_gc_marking();
void cgc_gc_collecting();
void cgc_gc();
void cgc_init();

// CGC (Reference Utils)

void cgc_ref(cgc_obj_info_t* ptr);
void cgc_unref(cgc_obj_info_t* ptr);

// CGC (defines)

#define CGC_DEFINE_ALLOC(TYPE, FIELD_COUNT, ARGS, BODY) \
    TYPE* TYPE##_alloc ARGS { \
        TYPE* ptr = malloc(sizeof(TYPE)); \
        CGC_INFO_INIT(TYPE, ptr, (FIELD_COUNT)); \
        (BODY); \
        return ptr; \
    }

#define CGC_INFO_INIT(TYPE, PTR, FIELD_COUNT) \
    { \
        cgc_obj_info_t* __cgc_info = (void*) (PTR); \
        __cgc_info->prev = (void*) cgc_last_obj->prev; \
        cgc_last_obj->prev = (void*) __cgc_info; \
        __cgc_info->dealloc = (void*) TYPE##_dealloc; \
        __cgc_info->ref_count = 0; \
        __cgc_info->field_count = (FIELD_COUNT); \
        __cgc_info->gc_mark = 0; \
    }

#define CGC_DBG_INFO_INIT(PTR, LEFT, RIGHT) \
    { \
        cgc_obj_info_t* __cgc_info = (void*) (PTR); \
        __cgc_info->debug_left = (LEFT); \
        __cgc_info->debug_right = (RIGHT); \
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
#define CGC_PTR_ENTER(PTR) cgc_ref((cgc_obj_info_t*) (void*) (PTR))
#define CGC_PTR_EXIT(PTR) cgc_unref((cgc_obj_info_t*) (void*) (PTR))

#endif // __CGC_H__
