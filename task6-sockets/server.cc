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
#include <algorithm>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <map>

#include <mutex>
#include <condition_variable>
#include <algorithm>
#include "util.h"
#include "client_message.pb.h"
#include "server_message.pb.h"


int nb_server_threads = 2;
int port = 1025;
constexpr int backlog = 1024; // how many pending connections the queue will hold


// the atomic number that is increased/decreased according to the received requests
std::atomic<int64_t> number = 0;
std::mutex ioLock;


// helper function to pass argument threads
struct ThreadArgs {
    std::vector<int> sockets;
    std::mutex mtx;
    // std::condition_variable cv;
    // std::map<int, uint64_t> total_bytes;
};


bool process_request(std::unique_ptr<char[]>& buffer, int buf_size, int fd) {
    // Note: buffer should be serialized into a protobuf object. It might contain more than one (repeated) requests.
    for (size_t offset = 0; offset < buf_size;) {
        sockets::client_msg message;
        if (!message.ParseFromArray(buffer.get() + offset, buf_size - offset)) {
            std::cerr << "Cannot parse Protobuf array" << std::endl;
            exit(-1);
        }
        offset += message.ByteSizeLong();

        for (size_t i = 0; i < message.ops_size(); ++i) {
            const auto& op = message.ops(i);
            if (op.type() == sockets::client_msg::ADD) {
                number += op.argument();
            }
            else if (op.type() == sockets::client_msg::SUB) {
                number -= op.argument();
            }
            else if (op.type() == sockets::client_msg::TERMINATION) {
                sockets::server_msg response;
                int64_t snapshot = number;
                response.set_result(static_cast<int32_t>(snapshot));
                ioLock.lock();
                std::cout << snapshot << std::endl;
                ioLock.unlock();
                size_t len = response.ByteSizeLong();
                char* buffer = new char[len];
                response.SerializeToArray(buffer, len);
                secure_send(fd, buffer, len);
                close(fd);
                return false;
            }
        }
    }

    return true;
}

void server(ThreadArgs& args) {
    auto id = std::this_thread::get_id();
    std::vector<int> sockets;
    for (;;) {
        // TODO:
        // wait until there are connections
        for (;;) {
            std::lock_guard _(args.mtx);
            if (!args.sockets.empty()) {
                sockets = args.sockets; // copy queue so we don't have to lock later
                break;
            }
        }

        // receive and process requests
        for (size_t i = 0; i < sockets.size(); ++i) {
            int fd = sockets[i];
            if (fd < 0) { continue; }
            struct pollfd pollfd = {.fd = fd, .events = POLLIN};
            switch (poll(&pollfd, 1, 5)) {
            case -1:
                perror("poll");
                exit(EXIT_FAILURE);
            case 0:
                continue;
            default:
                std::unique_ptr<char[]> buffer;
                int len = secure_recv(fd, buffer);
                if (!process_request(buffer, len, fd)) {
                    std::lock_guard _(args.mtx);
                    args.sockets[i] = -1;
                }
            }
        }

        {
            std::lock_guard _(args.mtx);
            auto it = std::remove_if(args.sockets.begin(), args.sockets.end(), [](int fd) { return fd == -1; });
            args.sockets.erase(it, args.sockets.end());
        }
    }
}



int main(int args, char* argv[]) {
    if (args < 3) {
        std::cerr << "usage: ./server <nb_server_threads> <port>\n";
        return EXIT_FAILURE;
    }

    nb_server_threads = std::atoi(argv[1]);
    port = std::atoi(argv[2]);

    std::vector<std::thread> threads;
    std::vector<ThreadArgs> threadArgs(nb_server_threads);

    for (int i = 0; i < nb_server_threads; i++) {
        threads.emplace_back(std::thread(server, std::ref(threadArgs[i])));
    }

    // create the socket on the listening address
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        perror("Cannot create server listening socket");
    }

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);


    try_bind:
    if (0 != bind(fd, (struct sockaddr*) &server_addr, sizeof(server_addr))) {
        if (errno == EADDRINUSE) {
            goto try_bind;
        }
        perror("Cannot bind");
        exit(EXIT_FAILURE);
    }

    if (0 != listen(fd, backlog)) {
        perror("Cannot listen");
        exit(EXIT_FAILURE);
    }

    // wait for incomming connections and assign them the threads
    for (size_t i = 0;; i = (i + 1) % nb_server_threads) {
        int connection_socket = accept(fd, 0, 0);
        if (connection_socket < 0) {
            perror("Cannot accept");
            exit(EXIT_FAILURE);
        }
        if (fcntl(connection_socket, F_SETFL, O_NONBLOCK) < 0) {
            perror("Cannot set socket to be non-blocking");
        }
        std::lock_guard _(threadArgs[i].mtx);
        threadArgs[i].sockets.push_back(connection_socket);
    }

    return EXIT_SUCCESS;
}
