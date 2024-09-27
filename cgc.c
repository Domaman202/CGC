#include "cgc.h"
#include <stdlib.h>
#include <string.h>

// CGC (GC Utils)

void cgc_mark(cgc_t* gc, cgc_ptr_t* ptr) {
    cgc_ptr_t** stack = calloc(CGC_MARK_STACK_SIZE, sizeof(cgc_ptr_t*));
    uint16_t stack_top = 0;
    uint16_t stack_ptr = 0;
    //
    while (ptr) {
        ptr->gc_mark = gc->gc_age;
        cgc_ptr_t** fields = (void*) ((uintptr_t) ptr + sizeof(cgc_ptr_t));
        //
        if (stack_top + ptr->field_count >= CGC_MARK_STACK_SIZE) {
            uint16_t count = stack_top - stack_ptr;
            memcpy(stack, stack + stack_ptr, count * sizeof(cgc_ptr_t*));
            stack_top = count;
            stack_ptr = 0;
            stack[stack_top + 1] = nullptr;
        }
        //
        for (int i = 0; i < ptr->field_count; ++i) {
            cgc_ptr_t* field = fields[i];
            if (field && field->gc_mark != gc->gc_age) {
                stack[stack_top++] = field;
            }
        }
        //
        ptr = stack[stack_ptr++];
    }
    //
    free(stack);
}

void cgc_gc_marking(cgc_t* gc) {
    gc->gc_age *= 3;
    cgc_ptr_t* last = gc->head_obj;
    while (last) {
        if (last->ref_count)
            cgc_mark(gc, last);
        last = (void*) last->prev;
    }
}

void cgc_gc_collecting(cgc_t* gc) {
    cgc_ptr_t* last = gc->head_obj;
    cgc_ptr_t* prev = nullptr;
    while (last) {
        if (last->gc_mark != gc->gc_age) {
            prev->prev = last->prev;
            cgc_ptr_t* deleting = last;
            last = (void*) last->prev;
            deleting->dealloc(deleting);
            continue;
        }
        prev = last;
        last = (void*) last->prev;
    }
}

void cgc_gc(cgc_t* gc) {
    cgc_gc_marking(gc);
    cgc_gc_collecting(gc);
}

cgc_t* cgc_alloc() {
    // alloc struct
    cgc_t* gc = malloc(sizeof(cgc_t));
    // alloc & init root object
    cgc_ptr_t* root_obj = malloc(sizeof(cgc_ptr_t));
    root_obj->prev = nullptr;
    root_obj->dealloc = nullptr; // no deallocating
    root_obj->ref_count = 1; // no deallocating
    root_obj->field_count = 0;
    root_obj->gc_mark = 0;
    root_obj->debug_left = 666;
    root_obj->debug_right = 999;
    gc->head_obj = root_obj;
    // gc age init
    gc->gc_age = 127;
    // return struct
    return gc;
}

void cgc_free(cgc_t* gc) {
    cgc_ptr_t* last = gc->head_obj;
    while (last) {
        void* ptr = last;
        last = (void*) last->prev;
        free(ptr);
    }
    free(gc);
}

// CGC (Reference Utils)

void cgc_ref(cgc_ptr_t* ptr) {
    ptr->ref_count++;
}

void cgc_unref(cgc_ptr_t* ptr) {
    ptr->ref_count--;
}