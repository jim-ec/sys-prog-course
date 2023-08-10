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
#include "util.h"

/**
 * returns a std::string of size len with random data (e.g. only 'd' chars.)
 */
std::string random_alpharithmetic(size_t len) {
    std::string st;
    for (size_t i = 0; i < len; i++) 
        st += "d";
    return st;
}




int main(int args, char** argv) {
    // TEST 1
    auto st_payload = random_alpharithmetic(2);
    int payload_size = st_payload.size();
    char* payload = const_cast<char*>(st_payload.c_str());
    std::unique_ptr<char[]> dst = std::make_unique<char[]>(payload_size+4);
    construct_message(dst.get(), payload, payload_size);

    if (payload_size != get_payload_size(dst.get(), 0)) {
        exit(2);
    }

    if (::memcmp((dst.get()+4), payload, payload_size) != 0)
        exit(1);




    // TEST 2
    st_payload = random_alpharithmetic(1024);
    payload_size = st_payload.size();
    payload = const_cast<char*>(st_payload.c_str());
    dst = std::make_unique<char[]>(payload_size+4);
    construct_message(dst.get(), payload, payload_size);

    if (payload_size != get_payload_size(dst.get(), 0)) {
        exit(4);
    }

    if (::memcmp((dst.get()+4), payload, payload_size) != 0)
        exit(1);

    // TEST 3
    st_payload = random_alpharithmetic(2096);
    payload_size = st_payload.size();
    payload = const_cast<char*>(st_payload.c_str());
    dst = std::make_unique<char[]>(payload_size+4);
    construct_message(dst.get(), payload, payload_size);

    if (payload_size != get_payload_size(dst.get(), 0)) {
        exit(4);
    }

    if (::memcmp((dst.get()+4), payload, payload_size) != 0)
        exit(1);

    // TEST 4
    st_payload = random_alpharithmetic(8192);
    payload_size = st_payload.size();
    payload = const_cast<char*>(st_payload.c_str());
    dst = std::make_unique<char[]>(payload_size+4);
    construct_message(dst.get(), payload, payload_size);

    if (payload_size != get_payload_size(dst.get(), 0)) {
        exit(4);
    }

    if (::memcmp((dst.get()+4), payload, payload_size) != 0)
        exit(1);


    // TEST 4
    st_payload = random_alpharithmetic(10000);
    payload_size = st_payload.size();
    payload = const_cast<char*>(st_payload.c_str());
    dst = std::make_unique<char[]>(payload_size+4);
    construct_message(dst.get(), payload, payload_size);

    if (payload_size != get_payload_size(dst.get(), 0)) {
        exit(4);
    }

    if (::memcmp((dst.get()+4), payload, payload_size) != 0)
        exit(1);
}
