#include "cgc.h"
#include <stdlib.h>
#include <string.h>

// CGC (GC Utils)

cgc_obj_info_t* cgc_last_obj;
uint8_t cgc_gc_age;

void cgc_mark(cgc_obj_info_t* ptr) {
    cgc_obj_info_t** stack = calloc(CGC_MARK_STACK_SIZE, sizeof(cgc_obj_info_t*));
    uint16_t stack_top = 0;
    uint16_t stack_ptr = 0;
    //
    while (ptr) {
        ptr->gc_mark = cgc_gc_age;
        cgc_obj_info_t** fields = (void*) ((uintptr_t) ptr + sizeof(cgc_obj_info_t));
        //
        if (stack_top + ptr->field_count >= CGC_MARK_STACK_SIZE) {
            uint16_t count = stack_top - stack_ptr;
            memcpy(stack, stack + stack_ptr, count * sizeof(cgc_obj_info_t*));
            stack_top = count;
            stack_ptr = 0;
            stack[stack_top + 1] = nullptr;
        }
        //
        for (int i = 0; i < ptr->field_count; ++i) {
            cgc_obj_info_t* field = fields[i];
            if (field && field->gc_mark != cgc_gc_age) {
                stack[stack_top++] = field;
            }
        }
        //
        ptr = stack[stack_ptr++];
    }
    //
    free(stack);
}

void cgc_gc_marking() {
    cgc_gc_age *= 3;
    cgc_obj_info_t* last = cgc_last_obj;
    while (last) {
        if (last->ref_count)
            cgc_mark(last);
        last = (void*) last->prev;
    }
}

void cgc_gc_collecting() {
    cgc_obj_info_t* last = cgc_last_obj;
    cgc_obj_info_t* prev = nullptr;
    while (last) {
        if (last->gc_mark != cgc_gc_age) {
            prev->prev = last->prev;
            cgc_obj_info_t* deleting = last;
            last = (void*) last->prev;
            deleting->dealloc(deleting);
            continue;
        }
        prev = last;
        last = (void*) last->prev;
    }
}

void cgc_gc() {
    cgc_gc_marking();
    cgc_gc_collecting();
}

void cgc_init() {
    // root object init
    cgc_obj_info_t* root_obj = malloc(sizeof(cgc_obj_info_t));
    root_obj->prev = nullptr;
    root_obj->dealloc = nullptr; // no deallocating
    root_obj->ref_count = 1; // no deallocating
    root_obj->field_count = 0;
    root_obj->gc_mark = 0;
    cgc_last_obj = root_obj;
    // gc age init
    cgc_gc_age = 127;
}

// CGC (Reference Utils)

void cgc_ref(cgc_obj_info_t* ptr) {
    ptr->ref_count++;
}

void cgc_unref(cgc_obj_info_t* ptr) {
    ptr->ref_count--;
}