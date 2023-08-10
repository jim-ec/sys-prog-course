#pragma once

#include <stdlib.h>


void* malloc_impl(size_t size);
void* calloc_impl(size_t count, size_t size);
void free_impl(void* pointer);
void* realloc_impl(void* pointer, size_t size);
