#pragma once

#include <stdlib.h>
#include <stdbool.h>


void* buddy_block_allocate(size_t size);
bool buddy_block_deallocate(void* pointer);
void* buddy_block_reallocate(void* pointer, size_t size, size_t* current_allocation_size);
void buddy_block_print();
