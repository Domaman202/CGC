#include "cgc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CGC (GC Utils)

cgc_obj_info_t* cgc_last_obj;
uint8_t cgc_gc_age;

void cgc_mark_old(cgc_obj_info_t* ptr) {
    ptr->gc_mark = cgc_gc_age;
    cgc_obj_info_t** fields = (void*) ((uintptr_t) ptr + sizeof(cgc_obj_info_t));
    for (int i = 0; i < ptr->field_count; ++i) {
        cgc_obj_info_t* field = fields[i];
        if (field && field->gc_mark != cgc_gc_age) {
            cgc_mark_old(field);
        }
    }
}

#define CGC_MARK_STACK_SIZE 1024

void cgc_mark_new_dbg_print(cgc_obj_info_t** stack, uint16_t stack_top, uint16_t stack_ptr, const char* text) {
    printf(text);
    for (int i = 0; i < stack_top; ++i) {
        printf("[%d] (%d|%d) [%d|%d]\n", i, stack[i]->debug_left, stack[i]->debug_right, stack_top, stack_ptr);
    }
    printf("\n");
}

void cgc_mark_new(cgc_obj_info_t* ptr) {
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

#define CGC_MARK(PTR) cgc_mark_new((PTR))

void cgc_gc_marking() {
    cgc_gc_age *= 3;
    cgc_obj_info_t* last = cgc_last_obj;
    while (last) {
        if (last->ref_count) {
            CGC_MARK(last);
        }
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