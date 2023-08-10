#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
	int val, fd;
	char *buf;
	size_t nbytes;

	srand(time(NULL));
	val = rand() % 4096;
	buf = malloc(val);

	// Open file and read val bytes in buf
	fd = open(argv[1], O_RDWR);
	nbytes = read(fd, buf, val);
	printf("read(%d, %p, %d) = %ld\n",
	       fd, buf, val, nbytes);
	// Write zeroes in buf
	memset(buf, '0', val);
	// write the cleared buf in the file at the beginning
	lseek(fd, 0, SEEK_SET);
	nbytes = write(fd, buf, val);
	printf("write(%d, %p, %d) = %ld\n",
	       fd, buf, val, nbytes);

	close(fd);

	return 0;
}
