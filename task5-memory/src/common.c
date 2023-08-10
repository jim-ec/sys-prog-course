#define _DEFAULT_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"


size_t align_up(size_t address, size_t alignment) {
    return (address + (alignment - 1)) & -alignment;
}


uint8_t* request_page(size_t size) {
    void* pointer = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // Page is assumed to be zero-initialized by the kernel.
    return pointer != (void*) -1 ? pointer : NULL;
}


void release_page(void* pointer, size_t size) {
    munmap(pointer, size);
}


void debug_trap() {
}
