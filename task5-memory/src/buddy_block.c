#include <stdio.h>
#include <string.h>
#include <stdalign.h>
#include "buddy_block.h"
#include "buddy_allocator.h"
#include "common.h"


struct BuddyBlock {
    void* next;
};


enum {
    CAPACITY = (PAGE_SIZE - sizeof(struct BuddyBlock)) / (sizeof(struct BuddyAllocator) + BUDDY_ALLOCATOR_BITMAP_SIZE)
};


static struct BuddyBlock* buddy_block() {
    static struct BuddyBlock* initial_buddy_block = NULL;
    if (initial_buddy_block == NULL) {
        initial_buddy_block = (struct BuddyBlock*) request_page(PAGE_SIZE);
    }
    return initial_buddy_block;
}


static struct BuddyAllocator* ith_allocator(struct BuddyBlock* block, size_t i) {
    return (struct BuddyAllocator*) ((uint8_t*) (block + 1) + i * (sizeof(struct BuddyAllocator) + BUDDY_ALLOCATOR_BITMAP_SIZE));
}


void* buddy_block_allocate(size_t size) {
    for (struct BuddyBlock* block = buddy_block();; block = block->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct BuddyAllocator* allocator = ith_allocator(block, i);
            if (allocator->page != NULL) {
                void* allocation = buddy_allocator_allocate(allocator, size);
                if (allocation != NULL) {
                    return allocation;
                }
                else {
                    // Try to allocate from next allocator.
                    continue;
                }
            }
            else {
                allocator->page = request_page(BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION);
                if (allocator->page == NULL) {
                    return NULL;
                }
                return buddy_allocator_allocate(allocator, size);
            }
        }
        if (block->next == NULL) {
            block->next = request_page(PAGE_SIZE);
            if (block->next == NULL) {
                return NULL;
            }
        }
    }
    return NULL;
}


bool buddy_block_deallocate(void* pointer) {
    for (struct BuddyBlock* block = buddy_block(); block != NULL; block = block->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct BuddyAllocator* allocator = ith_allocator(block, i);
            if (allocator->page == NULL ||
                (void*) allocator->page > pointer ||
                (void*) (allocator->page + BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION) <= pointer
            ) {
                continue;
            }
            else {
                buddy_allocator_deallocate(allocator, pointer);
                return true;
            }
        }
    }
    return false;
}


/// Reallocation strategy:
/// - try to expand the block
/// - try to move allocation within the same allocator
/// - move the allocation to another allocator
void* buddy_block_reallocate(void* pointer, size_t size, size_t* current_allocation_size) {
    for (struct BuddyBlock* block = buddy_block(); block != NULL; block = block->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct BuddyAllocator* allocator = ith_allocator(block, i);
            if (allocator->page == NULL ||
                (void*) allocator->page > pointer ||
                (void*) (allocator->page + BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION) <= pointer
            ) {
                continue;
            }
            else {
                void* insertion = buddy_allocator_reallocate(allocator, pointer, size, current_allocation_size);
                if (insertion != NULL) {
                    return insertion;
                }
                if (size > DIRECT_ALLOCATION_THRESHOLD) {
                    // current_allocation_size has been written at this point
                    return NULL;
                }
                insertion = buddy_block_allocate(size);
                if (insertion == NULL) {
                    return NULL;
                }
                memcpy(insertion, pointer, *current_allocation_size);
                buddy_block_deallocate(pointer);
                return insertion;
            }
        }
    }
    *current_allocation_size = 0;
    return NULL;
}


void buddy_block_print() {
    size_t i_block = 0;
    for (struct BuddyBlock* block = buddy_block(); block != NULL; block = block->next) {
        bool printed_current_block = false;
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct BuddyAllocator* allocator = ith_allocator(block, i);
            if (allocator->page != NULL) {
                if (!printed_current_block) {
                    printf("Block #%zu\n", i_block++);
                    printed_current_block = true;
                }
                printf("  Allocator #%zu:  ", i);
                buddy_allocator_print(allocator);
                printf("\n");
            }
        }
    }
}
