ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

PYTHON3 ?= python3
CFLAGS += -g -w -pthread -fpermissive
CFLAGS += -O2
HEADERS := ${ROOT_DIR}/../
CFLAGS += -I$(HEADERS)
CPPFLAGS += $(CFLAGS)

PROG1 := ${ROOT_DIR}/test_mutual_exclusion
PROG2 := ${ROOT_DIR}/lock_hashmap
PROG3 := ${ROOT_DIR}/lockfree_hashmap

OBJS := ${SRCS:.c=.o}

LIBS = -Wl,-rpath,${ROOT_DIR}/.. -L ${ROOT_DIR}/.. -Wl,-rpath,${ROOT_DIR}/../target/release -L ${ROOT_DIR}/../target/release -Wl,-rpath,${ROOT_DIR}/../target/debug -L ${ROOT_DIR}/../target/debug

LIBS1 = $(LIBS) -lcspinlock
LIBS2 = $(LIBS) -llockhashmap
LIBS3 = $(LIBS) -llockfreehashmap

all: $(PROG1) $(PROG2) $(PROG3)

check: $(PROG1) $(PROG2) $(PROG3)
	$(PYTHON3) ${ROOT_DIR}/test_mutual_exclusion.py
	$(PYTHON3) ${ROOT_DIR}/test_lockhashmap.py
	$(PYTHON3) ${ROOT_DIR}/test_lockfreehashmap.py

$(PROG1): ${ROOT_DIR}/test_mutual_exclusion.c
	$(CC) $(CFLAGS) -Og -g -pthread -o $@ $< $(LIBS1)

SRCS = \
        ${ROOT_DIR}/hashmap-driver.c \
        ${ROOT_DIR}/thread.c

$(PROG2): $(SRCS)
	$(CC) $(CFLAGS) -Og -g -pthread -o $@ $(SRCS) $(LIBS2)

$(PROG3): $(SRCS)
	$(CC) $(CFLAGS) -Og -g -pthread -o $@ $(SRCS) $(LIBS3)

clean:
	$(RM) $(OBJS) $(PROG1) $(PROG2) $(PROG3)
