#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>


ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);


int main() {
    int fd = open("bla.txt", O_RDWR);

    char buffer[32] = {0};
    ssize_t count;
    
    count = read(fd, buffer, sizeof(buffer));
    if (count >= 0) {
        printf("Read %d bytes\n", (int) count);
        printf("Buffer: `%s`\n", buffer);
    }
    else {
        printf("Could not read: %s\n", strerror(errno));
        exit(2);
    }

    lseek(fd, 0, SEEK_SET);

    count = write(fd, "Ciao", sizeof("Ciao") - 1);
    if (count >= 0) {
        printf("Wrote %d bytes\n", (int) count);
    }
    else {
        printf("Could not write: %s\n", strerror(errno));
        exit(2);
    }

    close(fd);
}
