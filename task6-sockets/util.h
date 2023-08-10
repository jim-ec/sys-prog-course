#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstdarg>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
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


/**
 * It takes as arguments one char[] array of 4 or bigger size and an integer.
 * It converts the integer into a byte array.
 */
inline void convertIntToByteArray(char* dst, int sz) {
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
inline int convertByteArrayToInt(char* b) {
    return (b[0] << 24)
        + ((b[1] & 0xFF) << 16)
        + ((b[2] & 0xFF) << 8)
        + (b[3] & 0xFF);
}


/**
 * It constructs the message to be sent. 
 * It takes as arguments a destination char ptr, the payload (data to be sent)
 * and the payload size.
 * It returns the expected message format at dst ptr;
 *
 *  |<---msg size (4 bytes)--->|<---payload (msg size bytes)--->|
 *
 *
 */
inline void construct_message(char* dst, char* payload, size_t payload_size) {
    uint32_t length_field = htonl(static_cast<uint32_t>(payload_size));
    memcpy(dst, &length_field, 4);
    memcpy(dst + 4, payload, payload_size);
}

/**
 * It returns the actual size of msg.
 * Not that msg might not contain all payload data. 
 * The function expects at least that the msg contains the first 4 bytes that
 * indicate the actual size of the payload.
 */
inline int get_payload_size(char* msg, size_t bytes) {
    auto payload_size = ntohl(*reinterpret_cast<uint32_t*>(msg));
    return payload_size;
}


inline void trace(const char* message, ...) {
    static std::mutex ioLock;
    std::lock_guard _(ioLock);
    va_list ap;
    va_start(ap, message);
    vfprintf(stderr, message, ap);
    fprintf(stderr, "\n");
    fflush(stderr);
    va_end(ap);
}


inline void send_all(int fd, char* data, size_t count) {
    size_t charsSent = 0;
    while (charsSent < count) {
        int result = send(fd, data + charsSent, count - charsSent, 0);
        if (result == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || errno == ECONNRESET) {
                continue;
            }
            if (errno != 0) {
                perror("Cannot send");
                exit(EXIT_FAILURE);
            }
        }
        charsSent += result;
    }
}


/**
 * Sends to the connection defined by the fd, a message with a payload (data) of size len bytes.
 * The fd should be non-blocking socket.
 */
inline int secure_send(int fd, char* data, size_t length) {
    char* buf = new char[4 + length];
    construct_message(buf, data, length);
    send_all(fd, buf, 4 + length);
    return length;
}

inline bool recv_all(int fd, char* data, size_t count) {
    size_t charsRecved = 0;
    while (charsRecved < count) {
        int result = recv(fd, data + charsRecved, count - charsRecved, 0);
        if (result == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ECONNRESET) {
                // connection still valid, but no or not enough data sent yet
                // return 0;
                continue;
            }
            else {
                perror("recv_secure");
            }
        }
        if (result == 0) {
            // connection closed
            return false;
        }
        charsRecved += result;
    }
    return true;
}


/**
 * Receives a message from the fd (non-blocking) and stores it in buf.
 */
inline int secure_recv(int fd, std::unique_ptr<char[]>& buf) {
    uint32_t length;
    if (!recv_all(fd, (char*) &length, 4)) {
        return 0;
    }
    length = ntohl(length);
    buf.reset(new char[length]);
    if (!recv_all(fd, buf.get(), length)) {
        return 0;
    }
    return length;
}
