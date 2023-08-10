// DO NOT EDIT
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
#include <atomic>

#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <map>

#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "util.h"


int nb_server_threads   = 1;
int port                = 1025;
constexpr int backlog   = 1024; // how many pending connections the queue will hold


int main(int args, char* argv[]) {
    if (args < 2) {
        std::cerr << "usage: ./server <port>\n";
        exit(1);
    }

    port = std::atoi(argv[1]);

    /* listen on sock_fd, new connection on new_fd */
    int sockfd, new_fd; 

    /* my address information */
    struct sockaddr_in my_addr; 

    /* connector.s address information */
    struct sockaddr_in their_addr; 
    socklen_t sin_size;

    int yes=1;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        perror("setsockopt");
        exit(1);
    }

    my_addr.sin_family      = AF_INET;          // host byte order
    my_addr.sin_port        = htons(port);      // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;       // automatically fill with my IP
    memset(&(my_addr.sin_zero), 0, 8);          // zero the rest of the struct

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }


    sin_size = sizeof(struct sockaddr_in);
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
        std::cerr << "accept() failed .. " << std::strerror(errno) << "\n";
    }

    // set the socket to non-blocking mode
    fcntl(new_fd, F_SETFL, O_NONBLOCK);


    int64_t bytecount = -1;
    while (1) {
        std::unique_ptr<char[]> buffer;
        if ((bytecount = secure_recv(new_fd, buffer))  <= 0) {
            if (bytecount == 0) {
                    break;
            }
        }
        std::cout << bytecount << "\n";
    }

    std::cout.flush();
    close(new_fd);
    return 0;
}








