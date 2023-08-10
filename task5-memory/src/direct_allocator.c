#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "common.h"
#include "direct_allocator.h"


struct DirectAllocator {
    struct DirectAllocator* next;
    // TODO: DirectAllocation candidate
};

struct DirectAllocation {
    size_t size;
    void* pages;
};


enum {
    CAPACITY = (PAGE_SIZE - sizeof(struct DirectAllocator)) / sizeof(struct DirectAllocation)
};


static struct DirectAllocator* direct_allocator() {
    static struct DirectAllocator* initial_direct_allocator = NULL;
    if (initial_direct_allocator == NULL) {
        initial_direct_allocator = (struct DirectAllocator*) request_page(PAGE_SIZE);
    }
    return initial_direct_allocator;
}


static struct DirectAllocation* ith_allocation(struct DirectAllocator* allocator, size_t i) {
    return (struct DirectAllocation*) ((uint8_t*) (allocator + 1) + i * sizeof(struct DirectAllocation));
}


void* direct_allocator_allocate(size_t size) {
    for (struct DirectAllocator* allocator = direct_allocator();; allocator = allocator->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct DirectAllocation* allocation = ith_allocation(allocator, i);
            if (allocation->pages == NULL) {
                allocation->pages = request_page(size);
                allocation->size = size;
                return allocation->pages;
            }
        }
        if (allocator->next == NULL) {
            allocator->next = (struct DirectAllocator*) request_page(PAGE_SIZE);
            if (allocator->next == NULL) {
                return NULL;
            }
        }
    }
}


bool direct_allocator_deallocate(void* pointer) {
    for (struct DirectAllocator* allocator = direct_allocator(); allocator != NULL; allocator = allocator->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct DirectAllocation* allocation = ith_allocation(allocator, i);
            if (allocation->pages == pointer) {
                release_page(allocation->pages, allocation->size);
                allocation->pages = NULL;
                return true;
            }
        }
    }
    return false;
}


void* direct_allocator_reallocate(void* pointer, size_t size) {
    for (struct DirectAllocator* allocator = direct_allocator(); allocator != NULL; allocator = allocator->next) {
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct DirectAllocation* allocation = ith_allocation(allocator, i);
            if (allocation->pages == pointer) {
                // TODO: This could also be implemented with mremap, but it's not POSIX
                // and I'm mainly developing on a non-Linux machine.
                if (allocation->size >= size) {
                    // Shrink allocation
                    if (allocation->size - size < PAGE_SIZE) {
                        // Size difference is not measurable in pages.
                        return pointer;
                    }
                    size_t release_start = align_up(size, PAGE_SIZE);
                    release_page((uint8_t*) allocation->pages + release_start, allocation->size - release_start);
                }
                else {
                    // Grow allocation
                    void* insertion = request_page(size);
                    if (insertion == NULL) {
                        return NULL;
                    }
                    memcpy(insertion, allocation->pages, allocation->size);
                    release_page(allocation->pages, allocation->size);
                    allocation->pages = insertion;
                }
                allocation->size = size;
                return allocation->pages;
            }
        }
    }
    // Not found
    return NULL;
}


void direct_allocator_print() {
    size_t i_allocator = 0;
    for (struct DirectAllocator* allocator = direct_allocator(); allocator != NULL; allocator = allocator->next) {
        bool printed_current_allocator = false;
        for (size_t i = 0; i < CAPACITY; ++i) {
            struct DirectAllocation* allocation = ith_allocation(allocator, i);
            if (allocation->pages != NULL) {
                if (!printed_current_allocator) {
                    printf("Direct allocator #%zu\n", i_allocator);
                    printed_current_allocator = true;
                }
                printf("  Allocation #%zu:  size=%zu\n", i, allocation->size);
            }
        }
        ++i_allocator;
    }
}
