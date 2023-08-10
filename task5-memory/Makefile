# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CC ?= cc
CCFLAGS ?= -g -Wall -Wextra -Wpedantic -O0
FLAGS = $(CCFLAGS) -std=c11
DYLIB_FLAGS = -shared -fPIC -ldl

#CC = /opt/homebrew/opt/gcc/bin/gcc-11

.PHONY: all clean check

all: libmymalloc.so

# $(var:old_prefix%old_suffix=new_prefix%new_suffix)
MODULES = allocator buddy_allocator buddy_block direct_allocator slot_allocator common
SOURCES = $(MODULES:%=src/%.c)
HEADERS = $(SOURCES:.c=.h)

libmymalloc.so: task5-memory.c $(SOURCES) $(HEADERS)
	$(CC) -o $@ task5-memory.c $(SOURCES) $(FLAGS) $(DYLIB_FLAGS)

# -target x86_64-apple-darwin-elf -stdlib=libc++
main: src/main.c $(SOURCES) $(HEADERS)
	$(CC) -o $@ src/main.c $(SOURCES) $(FLAGS)

check: all
	$(MAKE) -C tests check

clean:
	rm -rf *.so *.so.* *.o *.a *.dSYM main
