CXX = g++
COMPILE_FLAGS = -O3 -std=c++17 -Wall -g

.PHONY : all build check clean
all : build check

check: clean client_test server_test message_formatting_tests run
build: clean client_test server_test message_formatting_tests

LINKING_FLAGS = -lpthread  #-fsanitize=address
PROTO_FILES = client_message.pb.cc server_message.pb.cc
SRC_FILES = util_msg.cc
EXTRA_LFLAGS = $(LINKING_FLAGS) -lprotobuf
CXXFLAGS ?= -I../

proto_files:
	protoc --cpp_out=. client_message.proto
	protoc --cpp_out=. server_message.proto

message_formatting_tests:
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $@.cc -o $@ ${LINKING_FLAGS}
	mv message_formatting_tests ../

client_test: 
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $@.cc -o $@ ${LINKING_FLAGS}
	mv client_test ../

server_test:
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $@.cc -o $@ ${LINKING_FLAGS}
	mv server_test ../

server:
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $@.cc ${PROTO_FILES} ${SRC_FILES} -o $@ ${EXTRA_LFLAGS}
	mv server ../

client:
	$(CXX) $(COMPILE_FLAGS) $(CXXFLAGS) $@.cc ${PROTO_FILES} ${SRC_FILES} -o $@ ${EXTRA_LFLAGS}
	mv client ../


clean:
	rm -f ../message_formatting_tests ../server_test ../client_test *.txt *.txt *.txt
	#rm -f client_message.pb.h client_message.pb.cc server_message.pb.h server_message.pb.cc


run:
	sh ./run.sh
