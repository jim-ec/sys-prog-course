#include <stdlib.h>
#include "src/allocator.h"


void* malloc(size_t size) {
    return malloc_impl(size);
}

void* calloc(size_t count, size_t size) {
    return calloc_impl(count, size);
}

void free(void* pointer) {
    free_impl(pointer);
}

void* realloc(void* pointer, size_t size) {
    return realloc_impl(pointer, size);
}
