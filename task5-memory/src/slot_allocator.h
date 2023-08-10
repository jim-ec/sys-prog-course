#pragma once

#include <stdlib.h>
#include <stdbool.h>


void* slot_allocator_allocate(size_t size);
bool slot_allocator_deallocate(void* pointer);
void* slot_allocator_reallocate(void* pointer, size_t size);
void slot_allocator_print();
void slot_allocator_check_integrity();
