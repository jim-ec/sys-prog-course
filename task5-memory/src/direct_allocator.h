#pragma once

#include <stdlib.h>
#include <stdbool.h>


void* direct_allocator_allocate(size_t size);
bool direct_allocator_deallocate(void* pointer);
void* direct_allocator_reallocate(void* pointer, size_t size);
void direct_allocator_print();
