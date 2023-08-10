#include <sys/types.h>
#include <fcntl.h>

#include <errno.h>
#include <stddef.h>


ssize_t read_syscall(int fd, void *buffer, size_t count);
asm("read_syscall:\n\t"
    "movq $0, %rax\n\t"
    "syscall\n\t"
    "ret\n\t"
);


ssize_t read(int fd, void *buffer, size_t count) {
    if (count == 0) {
        return 0;
    }

    if (fd < 0) {
        errno = EBADF;
        return -1;
    }

    if (buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    int flags = fcntl(fd, F_GETFL);
    if (flags & O_WRONLY) {
        errno = EBADF;
        return -1;
    }

    return read_syscall(fd, buffer, count);
}


ssize_t write_syscall(int fd, const void *buffer, size_t count);
asm("write_syscall:\n\t"
    "movq $1, %rax\n\t"
    "syscall\n\t"
    "ret\n\t"
);


ssize_t write(int fd, const void *buffer, size_t count) {
    if (count == 0) {
        return 0;
    }

    if (fd < 0) {
        errno = EBADF;
        return -1;
    }

    if (buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    int flags = fcntl(fd, F_GETFL);
    if (!(flags & O_RDWR) && !(flags & O_WRONLY)) {
        errno = EBADF;
        return -1;
    }

    return write_syscall(fd, buffer, count);
}
