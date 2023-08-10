#pragma once

#include <stdlib.h>
#include <stdint.h>


// Allocator configuration
enum {
    PAGE_SIZE = 4096,
    ALLOCATION_GRANULARITY = 16,

    BUDDY_ALLOCATOR_DEPTH = 9,
    BUDDY_ALLOCATOR_NODE_COUNT = (1 << BUDDY_ALLOCATOR_DEPTH) - 1,
    BUDDY_ALLOCATOR_BITMAP_SIZE = (BUDDY_ALLOCATOR_NODE_COUNT + 7) / 8,
    BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION = ALLOCATION_GRANULARITY * (1 << (BUDDY_ALLOCATOR_DEPTH - 1)),

    SLOT_ALLOCATOR_SIZE = 32 * PAGE_SIZE,
    SLOT_ALLOCATOR_THRESHOLD = 500,
    SLOT_ALLOCATOR_SLOT_SIZE = 512,

    DIRECT_ALLOCATION_THRESHOLD = 2 * PAGE_SIZE
};


size_t align_up(size_t address, size_t alignment);
uint8_t* request_page(size_t size);
void release_page(void* pointer, size_t size);
void debug_trap();
