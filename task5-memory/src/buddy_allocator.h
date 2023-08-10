#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "common.h"


// TODO: Store page pointer in block
struct BuddyAllocator {
    uint8_t* page;
    uint8_t nodes[BUDDY_ALLOCATOR_BITMAP_SIZE];
};


void* buddy_allocator_allocate(struct BuddyAllocator* allocator, size_t size);
void* buddy_allocator_reallocate(struct BuddyAllocator* allocator, void* pointer, size_t size, size_t* current_allocation_size);
void buddy_allocator_deallocate(struct BuddyAllocator* allocator, void* pointer);
void buddy_allocator_print(struct BuddyAllocator* allocator);
double buddy_allocator_utilization(struct BuddyAllocator* allocator);
