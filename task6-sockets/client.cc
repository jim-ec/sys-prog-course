#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#include "client_message.pb.h"
#include "server_message.pb.h"
#include "util.h"
#include "util_msg.h"


const char* host = nullptr;
int port = 0;
size_t request_count = 0;
std::mutex ioLock;



void client() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);
    if (0 > connect(fd, (struct sockaddr*) &addr, sizeof(addr))) {
        perror("Cannot connect");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < request_count; ++i) {
        size_t size;
        auto buffer = get_operation(size);
        if (0 > secure_send(fd, buffer.get(), size)) {
            perror("Cannot send");
            exit(EXIT_FAILURE);
        }
    }

    send_termination_message(fd);
    std::unique_ptr<char[]> buffer;
    secure_recv(fd, buffer);
    close(fd);
    sockets::server_msg msg;
    msg.ParseFromArray(buffer.get(), get_payload_size(buffer.get(), 0));
    auto result = msg.result();
    ioLock.lock();
    std::cout << result << std::endl;
    ioLock.unlock();
}



int main(int argc, char** argv) {
    if (argc < 4) {
        printf("usage: %s <thread_count> <host> <port> <request_count>\n", argv[0]);
        exit(-1);
    }
    size_t thread_count = atoi(argv[1]);
    host = (0 == strcmp(argv[2], "localhost")) ? "127.0.0.1" : argv[2];
    port = atoi(argv[3]);
    request_count = atoi(argv[4]);
    // fprintf(stderr, "Connecting %zu %s to %s:%d, each one making %zu requests ...\n",
    //     thread_count, thread_count > 1 ? "threads" : "thread", host, port, request_count);
    
    std::vector<std::thread> threads;
    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back(client);
    }
    for (auto& thread : threads) {
        thread.join();
    }
}
