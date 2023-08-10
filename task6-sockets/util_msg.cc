#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <memory>
#include <atomic>
#include "util.h"
#include "util_msg.h"
#include "client_message.pb.h"
#include "server_message.pb.h"


#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>


/**
 * It takes as arguments one char[] array of 4 or bigger size and an integer.
 * It converts the integer into a byte array.
 */
void _convertIntToByteArray(char* dst, int sz) {
    auto tmp = dst;
    tmp[0] = (sz >> 24) & 0xFF;
    tmp[1] = (sz >> 16) & 0xFF;
    tmp[2] = (sz >> 8) & 0xFF;
    tmp[3] = sz & 0xFF;
}

/**
 * It takes as an argument a ptr to an array of size 4 or bigger and 
 * converts the char array into an integer.
 */
int _convertByteArrayToInt(char* b) {
    return (b[0] << 24)
        + ((b[1] & 0xFF) << 16)
        + ((b[2] & 0xFF) << 8)
        + (b[3] & 0xFF);
}

std::unique_ptr<char[]> get_operation(size_t& size) {
    static std::atomic<int> i{0};
    sockets::client_msg msg;

    if (i.fetch_add(1) % 2 == 0) {
        // create an ADD request that adds +5
        // that can be either 5 repeated add requests
        // or one add request with 5 as an argument
        auto* op1 = msg.add_ops();
        op1->set_type(sockets::client_msg::ADD);
        op1->set_argument(2);
        auto* op2 = msg.add_ops();
        op2->set_type(sockets::client_msg::ADD);
        op2->set_argument(3);
    }
    else {
        // create a SUB request with 2 as an argument
        // that can be either 2 repeated sub requests
        // or one add request with 2 as an argument
        auto* op = msg.add_ops();
        op->set_type(sockets::client_msg::SUB);
        op->set_argument(2);
    }
    
    // Serialize the protobuf object into a char buf and return it.
    size = msg.ByteSizeLong();
    std::unique_ptr<char[]> pointer;
    pointer.reset(new char[size]);
    if (!msg.SerializeToArray(pointer.get(), size)) {
        std::cerr << "Cannot serialize message" << std::endl;
        exit(-1);
    }
    return pointer;
}

void send_termination_message(int socket) {
    sockets::client_msg msg;
    auto* op = msg.add_ops();
    op->set_type(sockets::client_msg::TERMINATION);
    std::vector<char> buffer(msg.ByteSizeLong());
    if (!msg.SerializeToArray(buffer.data(), buffer.size())) {
        std::cerr << "Cannot serialize message" << std::endl;
        exit(-1);
    }
    if (0 > secure_send(socket, buffer.data(), buffer.size())) {
        perror("Cannot send termination message");
        exit(EXIT_FAILURE);
    }
}
