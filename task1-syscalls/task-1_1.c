#include <unistd.h>
#include <sys/syscall.h>
#include <stddef.h>
#include <sys/types.h>


ssize_t read(int fd, void *buf, size_t count) {
    if (count == 0) return 0;
    return syscall(SYS_read, fd, buf, count);
}


ssize_t write(int fd, const void *buf, size_t count) {
    if (count == 0) return 0;
    return syscall(SYS_write, fd, buf, count);
}
