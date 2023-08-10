#include <string.h>
#include "allocator.h"
#include "common.h"
#include "buddy_block.h"
#include "direct_allocator.h"
#include "slot_allocator.h"


void* malloc_impl(size_t size) {
    if (size < SLOT_ALLOCATOR_THRESHOLD) {
        return buddy_block_allocate(size);
    }
    else if (size < DIRECT_ALLOCATION_THRESHOLD) {
        return slot_allocator_allocate(size);
    }
    else {
        return direct_allocator_allocate(size);
    }
}


void* calloc_impl(size_t count, size_t size) {
    void* pointer = malloc_impl(count * size);
    if (size < DIRECT_ALLOCATION_THRESHOLD) {
        memset(pointer, 0, count * size);
    }
    return pointer;
}


void free_impl(void* pointer) {
    if (direct_allocator_deallocate(pointer)) {
        return;
    }
    if (slot_allocator_deallocate(pointer)) {
        return;
    }
    if (buddy_block_deallocate(pointer)) {
        return;
    }
    return;
}


void* realloc_impl(void* pointer, size_t size) {
    size_t current_allocation_size = 0;
    void* insertion = buddy_block_reallocate(pointer, size, &current_allocation_size);
    if (insertion != NULL) {
        return insertion;
    }
    if (current_allocation_size == 0) {
        void* new_pointer;
        new_pointer = slot_allocator_reallocate(pointer, size);
        if (new_pointer != NULL) {
            return new_pointer;
        }

        // if (size < ALLOCATOR_DIRECT_ALLOCATION_THRESHOLD) {
        //     // Move out of the direct allocator
        //     new_pointer = buddy_block_allocate(size);
        //     if (new_pointer == NULL) {
        //         return NULL;
        //     }
        //     memcpy(new_pointer, pointer, size);
        //     direct_allocator_deallocate(pointer);
        //     return new_pointer;
        // }
        // else {
            return direct_allocator_reallocate(pointer, size);
        // }
    }
    else {
        // Move allocation to the direct allocator.
        insertion = direct_allocator_allocate(size);
        if (insertion == NULL) {
            return NULL;
        }
        memcpy(insertion, pointer, current_allocation_size);
        buddy_block_deallocate(pointer);
        return insertion;
    }
    return NULL;
}
