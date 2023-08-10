// DO NOT EDIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <thread>
#include <vector>
#include <iostream>
#include <cstring>
#include <atomic>
#include <iostream>
#include <fstream>
#include "util.h"


int nb_clients  = -1;
int port        = -1;
int nb_messages = -1;


// returns a random std::string of size 'size'
std::string getstring(size_t size) {
    std::string st;
    for (size_t i = 0; i < size; i++) {
        st += "d";
    }
    return st;
}

// returns a unique_ptr to a char[] of 'random' size
std::unique_ptr<char[]> get_rand_data(size_t& size) {
    static int i = 1;
    i*= 2;

    // do not sent bigger messages
    if (i > 65536)
        i = 1;
    size = i;
    auto  st = getstring(i);
    char* data = const_cast<char*>(st.c_str());

    std::unique_ptr<char[]> ptr = std::make_unique<char[]>(size);
    ::memcpy(ptr.get(), data, size);

    // return std::move(ptr);
    return ptr;
}



int main (int args, char* argv[]) {
    if (args < 4) {
        std::cerr << "usage: ./client <hostname> <port> <nb_messages>\n";
    }


    port        = std::atoi(argv[2]);
    nb_messages = std::atoi(argv[3]);

    struct hostent *he;
    if ((he = gethostbyname(argv[1])) == NULL) {
        exit(1);
    }

    int sockfd, numbytes;
    struct sockaddr_in their_addr; 

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family   = AF_INET;
    their_addr.sin_port     = htons(port);
    their_addr.sin_addr     = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    int iterations = nb_messages;
    while (iterations > 0) {
        size_t size = 0;
        std::unique_ptr<char[]> buf = get_rand_data(size);
        if ((numbytes = secure_send(sockfd, buf.get(), size)) == -1) {
            std::cout << std::strerror(errno) << "\n";
            exit(1);
        }
        std::cout << numbytes << "\n";
        iterations--;
    }

    close(sockfd);
}
