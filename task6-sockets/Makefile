# Set you prefererred CFLAGS/compiler compiler here.
# Our github runner provides gcc-10 by default.
CC ?= cc
CFLAGS ?= -g -Wall -O2
CXX ?= c++
CXXFLAGS ?= -g -Wall -O2
CARGO ?= cargo
RUSTFLAGS ?= -g

LINKING_FLAGS = -lpthread  #-fsanitize=address
PROTO_FILES = client_message.pb.cc server_message.pb.cc
SRC_FILES = util_msg.cc
EXTRA_LFLAGS = $(LINKING_FLAGS) `pkg-config --cflags --libs protobuf`
COMPILE_FLAGS = -std=c++17

.PHONY = check clean all

all: build
# this target should build all executables for all tests
# C example:
#all:
#	$(CC) $(CFLAGS) -o task-name task-name.c

# C++ example:
#all:
#	$(CXX) $(CXXFLAGS) -o task-name task-name.cpp

# Rust example:
#all:
#	$(CARGO) build --release

# Usually there is no need to modify this
build: proto_files server client
	$(MAKE) -C tests build



proto_files:
	protoc --cpp_out=. client_message.proto
	protoc --cpp_out=. server_message.proto


server:
	$(CXX) $(COMPILE_FLAGS) $@.cc ${PROTO_FILES} ${SRC_FILES} -o $@ ${EXTRA_LFLAGS}

client:
	$(CXX) $(COMPILE_FLAGS) $@.cc ${PROTO_FILES} ${SRC_FILES} -o $@ ${EXTRA_LFLAGS}

check: proto_files
	$(MAKE) -C tests check

clean:
	$(MAKE) -C tests clean
	-rm -f server client
	-rm -f *.txt
	-rm -f ${PROTO_FILES} client_message.pb.h server_message.pb.h
