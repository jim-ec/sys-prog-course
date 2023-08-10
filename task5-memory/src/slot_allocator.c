// hands out allocations aligned to a specifc slot size
// this allocator is used for a very constraint size class,
// thus it makes sense to simply manage an array of statically sized blocks
// which can be assigned to an allocation or not. they are called slots
// an allocation spans one ore more slots continuously, so do free spans


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "common.h"
#include "slot_allocator.h"


struct SlotAllocator {
    struct SlotAllocator* next;
    size_t cursor;
};


// TODO: explicitely return an aligned pointer when serving memory to user
struct Slot {
    size_t length;
    bool used;
};


enum {
    SLOT_SIZE = SLOT_ALLOCATOR_SLOT_SIZE + sizeof(struct Slot),
    SLOT_COUNT = (SLOT_ALLOCATOR_SIZE - sizeof(struct SlotAllocator)) / SLOT_SIZE
};


// If the `i`th slot is not the begin of a span, it will not contain valid data.
static struct Slot* get_slot(struct SlotAllocator* allocator, size_t i) {
    return (struct Slot*) ((uint8_t*) (allocator + 1) + i * SLOT_SIZE);
}


static struct Slot* get_successor_slot(struct SlotAllocator* allocator, size_t i) {
    return get_slot(allocator, i + get_slot(allocator, i)->length);
}

static struct SlotAllocator* create() {
    struct SlotAllocator* allocator = (struct SlotAllocator*) request_page(SLOT_ALLOCATOR_SIZE);
    if (allocator == NULL) {
        return NULL;
    }
    allocator->next = NULL;
    allocator->cursor = 0;
    get_slot(allocator, 0)->used = false;
    get_slot(allocator, 0)->length = SLOT_COUNT;
    return allocator;
}


static struct SlotAllocator* slot_allocator() {
    static struct SlotAllocator* initial_slot_allocator = NULL;
    if (initial_slot_allocator == NULL) {
        initial_slot_allocator = create();
    }
    return initial_slot_allocator;
}


static size_t slot_to_index(struct SlotAllocator* allocator, struct Slot* slot) {
    size_t offset = (size_t) slot - (size_t) allocator - sizeof(struct SlotAllocator);
    return offset / SLOT_SIZE;
}


void* slot_allocator_allocate(size_t size) {
    size_t required_slots = (sizeof(struct Slot) + size) / SLOT_SIZE;
    if (required_slots * SLOT_SIZE < (sizeof(struct Slot) + size)) {
        ++required_slots;
    }

    struct SlotAllocator* allocator = slot_allocator();
    if (allocator == NULL) {
        return NULL;
    }
    for (;; allocator = allocator->next) {
        for (size_t i = allocator->cursor; i < SLOT_COUNT;) {
            struct Slot* slot = get_slot(allocator, i);

            if (slot->used) {
                // skip used spans
                i += slot->length;
                continue;
            }

            // merge with unused successor spans
            while ((i + slot->length < SLOT_COUNT) && !get_successor_slot(allocator, i)->used) {
                slot->length += get_successor_slot(allocator, i)->length;
            }

            if (slot->length < required_slots) {
                // skip unused spans which are too small
                i += slot->length;
                continue;
            }

            // found big enough span
            slot->used = true;
            if (slot->length > required_slots) {
                // split slot into used and unused spans
                // get_successor_slot(allocator, i)->used = false;
                // get_successor_slot(allocator, i)->length = slot->length - required_slots;
                get_slot(allocator, i + required_slots)->used = false;
                get_slot(allocator, i + required_slots)->length = slot->length - required_slots;
                slot->length = required_slots;
            }
            // update cursor
            //allocator->cursor = (i + required_slots) % SLOT_COUNT; // TODO: next search must start *after* this span
            allocator->cursor = i + required_slots;

            // return data pointer
            return (slot + 1);
        }

        if (allocator->next == NULL) {
            allocator->next = create();
            if (allocator->next == NULL) {
                return NULL;
            }
        }
    }
}


bool slot_allocator_deallocate(void* pointer) {
    struct SlotAllocator* allocator = slot_allocator();
    for (;;) {
        if (pointer < (void*) allocator ||
            pointer >= (void*) ((uint8_t*) allocator + SLOT_ALLOCATOR_SIZE)
        ) {
            if (allocator->next != NULL) {
                allocator = allocator->next;
                continue;
            }
            else {
                return false;
            }
        }

        struct Slot* slot = (struct Slot*) pointer - 1;
        size_t i = slot_to_index(allocator, slot);
        if (i < allocator->cursor) {
            allocator->cursor = i;
        }

        // free spans are merged in the allocation function, because here we don't know where the previous span starts
        slot->used = false;
        return true;
    }
}


void* slot_allocator_reallocate(void* pointer, size_t size) {
    size_t required_slots = (sizeof(struct Slot) + size) / SLOT_SIZE;
    if (required_slots * SLOT_SIZE < (sizeof(struct Slot) + size)) {
        ++required_slots;
    }
    struct SlotAllocator* allocator = slot_allocator();
    for (;;) {
        if (pointer < (void*) allocator ||
            pointer >= (void*) ((uint8_t*) allocator + SLOT_ALLOCATOR_SIZE)
        ) {
            if (allocator->next != NULL) {
                allocator = allocator->next;
                continue;
            }
            else {
                return false;
            }
        }

        struct Slot* slot = (struct Slot*) pointer - 1;
        size_t i = slot_to_index(allocator, slot);

        // try to expand the span by merging with unused spans
        while ((i + slot->length < SLOT_COUNT) && !get_successor_slot(allocator, i)->used && slot->length <= required_slots) {
            slot->length += get_successor_slot(allocator, i)->length;
        }

        // expansion overshooted or requested a smaller allocation, split into used and unused spans again
        if (slot->length > required_slots) {
            get_slot(allocator, i + required_slots)->used = false;
            get_slot(allocator, i + required_slots)->length = slot->length - required_slots;
            slot->length = required_slots;
        }

        if (slot->length == required_slots) {
            // update cursor, maybe it does not point to a span begin anymore
            allocator->cursor = i + required_slots;

            // successfully expanded span
            // no need to move the data, because span can only expand to the right
            return pointer;
        }

        // span is not big enough even after merging with free spans
        // simply allocate another span from somewhere and move the data there
        void* new_pointer = slot_allocator_allocate(size);
        if (new_pointer == NULL) {
            return NULL;
        }
        memcpy(new_pointer, pointer, slot->length * SLOT_SIZE);

        // deallocate current span
        slot->used = false;

        return new_pointer;
    }
}


void slot_allocator_print() {
    size_t i_allocator = 0;
    for (struct SlotAllocator* allocator = slot_allocator(); allocator != NULL; allocator = allocator->next) {
        printf("Slot allocator #%zu:  ", i_allocator);
        for (size_t i = 0; i < SLOT_COUNT;) {
            struct Slot* slot = get_slot(allocator, i);
            if (slot->used) {
                printf("*");
            }
            printf("%zu,", slot->length);
            i += slot->length;
        }
        printf("\n");
        ++i_allocator;
    }
}


void slot_allocator_check_integrity() {
    for (struct SlotAllocator* allocator = slot_allocator(); allocator != NULL; allocator = allocator->next) {
        for (size_t i = 0;;) {
            struct Slot* slot = get_slot(allocator, i);
            if (slot->used != true && slot->used != false) {
                debug_trap();
            }
            if (slot->length == 0) {
                debug_trap();
            }
            if (i + slot->length > SLOT_COUNT) {
                debug_trap();
            }
            if (i + slot->length == SLOT_COUNT) {
                break;
            }
            i += slot->length;
        }
    }
}
