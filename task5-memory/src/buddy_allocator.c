/// This implementation of the buddy allocator algorithm takes much inspiration from
/// here: https://github.com/evanw/buddy-malloc/blob/master/buddy-malloc.c

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "buddy_allocator.h"
#include "common.h"


#define PRINT_NODES false
#define PRINT_BUCKETS false


static bool is_marked(uint8_t* nodes, size_t i) {
    uint8_t byte = nodes[i / 8];
    uint8_t place = (1 << (i % 8));
    return byte & place;
}


static void mark(uint8_t* nodes, size_t i) {
    uint8_t place = (1 << (i % 8));
    nodes[i / 8] |= place;
}


static void unmark(uint8_t* nodes, size_t i) {
    uint8_t place = (1 << (i % 8));
    nodes[i / 8] &= ~place;
}


static size_t bucket_size(size_t bucket) {
    if (bucket >= BUDDY_ALLOCATOR_DEPTH) {
        return 0;
    }
    return ALLOCATION_GRANULARITY * (1 << (BUDDY_ALLOCATOR_DEPTH - 1 - bucket));
}


/// Left child node.
static size_t left(size_t index) {
    return 2 * index + 1;
}


/// Right child node.
static size_t right(size_t index) {
    return 2 * index + 2;
}


/// Sibling node. Sibling of root is `~0u`.
static size_t sibling(size_t index) {
    return ((index - 1) ^ 1) + 1;
}


/// Parent node.
static size_t parent(size_t index) {
    return (index - 1) / 2;
}


/// True if the sibling is right of this node. Also true for the root node.
static bool is_left(size_t index) {
    return index < sibling(index);
}


static bool is_split(uint8_t* nodes, size_t i, size_t bucket) {
    // if both children are unavailable, then this node is unavailable, not split
    return  bucket < BUDDY_ALLOCATOR_DEPTH - 1 && (is_marked(nodes, left(i)) || is_marked(nodes, right(i)));
}


static void* node_to_pointer(struct BuddyAllocator* allocator, size_t i, size_t bucket) {
    size_t offset = ((i - (1 << bucket) + 1) << (BUDDY_ALLOCATOR_DEPTH - bucket - 1));
    return allocator->page + ALLOCATION_GRANULARITY * offset;
}


void* buddy_allocator_allocate(struct BuddyAllocator* allocator, size_t size) {
    if (size > BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION) {
        return NULL;
    }

    size_t bucket = 0;
    size_t page_offset = 0;

    for (size_t i = 0;;) {
        if (is_marked(allocator->nodes, i) && (size > bucket_size(bucket + 1) || !is_split(allocator->nodes, i, bucket))) {
            if (!is_left(i)) {
                do {
                    i = parent(i);
                    page_offset -= bucket_size(bucket);
                    --bucket;
                } while (!is_left(i));
            }
            if (i == 0) {
                return NULL;
            }

            // Switch to right child on the same bucket
            i = sibling(i);
            page_offset += bucket_size(bucket);

            continue;
        }

        if (size <= bucket_size(bucket + 1)) {
            // try to descend as much as possible
            i = left(i);
            ++bucket;
            // printf("descend to left, bucket=%zu\n", bucket);
            continue;
        }

        mark(allocator->nodes, i);
        while (i != 0) {
            i = parent(i);
            mark(allocator->nodes, i);
        };
        return allocator->page + page_offset;
    }

    return NULL;
}


void find_buddy(struct BuddyAllocator* allocator, void* requested_pointer, size_t* out_i, size_t* out_level) {
    size_t i = 0;
    size_t bucket = 0;
    for (uint8_t* pointer = allocator->page;;) {
        if (pointer == requested_pointer && !is_split(allocator->nodes, i, bucket)) {
            break;
        }
        if (requested_pointer < (void*) (pointer + bucket_size(bucket + 1))) {
            i = left(i);
        }
        else {
            pointer += bucket_size(bucket + 1);
            i = right(i);
        }
        ++bucket;
    }
    *out_i = i;
    *out_level = bucket;
}


void buddy_allocator_deallocate(struct BuddyAllocator* allocator, void* requested_pointer) {
    size_t i, bucket;
    find_buddy(allocator, requested_pointer, &i, &bucket);

    // Try to merge buddies
    for (;;) {
        unmark(allocator->nodes, i);

        if (i == 0 || is_marked(allocator->nodes, sibling(i))) {
            break;
        }

        // Merge `i` and `sibling(i)`:
        i = parent(i);
        --bucket;
    }

    return;
}


void* buddy_allocator_reallocate(struct BuddyAllocator* allocator, void* requested_pointer, size_t size, size_t* current_allocation_size) {
    // Find allocation size
    size_t i, bucket;
    find_buddy(allocator, requested_pointer, &i, &bucket);
    *current_allocation_size = bucket_size(bucket);

    // Try to shrink allocation, if the requested size is smaller then the current size by a bucket.
    bool shrunk = false;
    while (size <= bucket_size(bucket + 1) && bucket < BUDDY_ALLOCATOR_DEPTH - 1) {
        shrunk = true;
        // Descend into left child node, so we dont even have to move the data.
        i = left(i);
        mark(allocator->nodes, i);
        ++bucket;
    }
    if (shrunk) {
        return requested_pointer;
    }

    // Do not do anything if requested size did not grow
    if (size <= *current_allocation_size) {
        return requested_pointer;
    }

    // Do nothing if new size exceed the direct allocation threshold.
    if (size > BUDDY_ALLOCATOR_MAXIMAL_ALLOCATION) {
        return NULL;
    }

    // Try to expand the allocation size by merging with unmarked siblings.
    // This automatically de-allocates the current allocation.
    for (;;) {
        if (bucket_size(bucket) >= size) {
            // Successfully expanded the allocation.
            // It is possible that the memory range has a different starting pointer
            // when expanded to the left. In this case, existing data has to be moved.
            void* new_pointer = node_to_pointer(allocator, i, bucket);
            if (new_pointer != requested_pointer) {
                memcpy(new_pointer, requested_pointer, *current_allocation_size);
            }
            return new_pointer;
        }

        // Deallocate current node.
        unmark(allocator->nodes, i);
        
        if (bucket == 0) {
            // Cannot extent root buddy.
            break;
        }
        if (is_marked(allocator->nodes, sibling(i))) {
            // Sibling is already used.
            // Otherwise, the unmarking above already merged both siblings.
            break;
        }

        i = parent(i);
        --bucket;
    }

    // Expansion was not successfull. Try to find a disjoint memory range.
    // While the de-allocation took already place, the fact that merging failed, means
    // that this new allocation will never overlap with the old allocation.
    void* new_pointer = buddy_allocator_allocate(allocator, size);
    if (new_pointer == NULL) {
        return NULL;
    }
    memcpy(new_pointer, requested_pointer, *current_allocation_size);
    return new_pointer;
}


static size_t usage_recurse(uint8_t* nodes, size_t i, size_t bucket) {
    if (is_split(nodes, i, bucket)) {
        size_t usage_left = usage_recurse(nodes, left(i), bucket + 1);
        size_t usage_right = usage_recurse(nodes, right(i), bucket + 1);
        return usage_left + usage_right;
    }
    return is_marked(nodes, i) ? bucket_size(bucket) : 0;
}


static size_t usage(uint8_t* nodes) {
    return usage_recurse(nodes, 0, 0);
}


double buddy_allocator_utilization(struct BuddyAllocator* allocator) {
    return (double) usage(allocator->nodes) / (double) BUDDY_ALLOCATOR_BITMAP_SIZE;
}


#if PRINT_BUCKETS
static void print_nodes_recurse(struct Allocator* allocator, size_t bucket, size_t i) {
    uint8_t* nodes = allocator_nodes(allocator);
    if (!is_marked(nodes, i)) {
        printf("%zu", bucket_size(bucket));
    }
    else if (is_marked(nodes, i) && !is_split(nodes, i, bucket)) {
        printf("*%zu", bucket_size(bucket));
    }
    else {
        printf("(");
        print_nodes_recurse(allocator, bucket + 1, left(i));
        printf(", ");
        print_nodes_recurse(allocator, bucket + 1, right(i));
        printf(")");
    }
}
#endif


void buddy_allocator_print(struct BuddyAllocator* allocator) {
    printf("    %.2lf%%", 100.0f * buddy_allocator_utilization(allocator));
#if PRINT_NODES
    uint8_t* nodes = allocator_nodes(allocator);
    printf("    %hhu ", is_marked(nodes, 0));
    for (size_t i = 1; i < ALLOCATOR_NODE_COUNT; i += 2) {
        printf("%hhu%hhu ", is_marked(nodes, i), is_marked(nodes, i + 1));
    }
#endif
#if PRINT_BUCKETS
    printf("    [");
    print_nodes_recurse(allocator, 0, 0);
    printf("]");
#endif
}
