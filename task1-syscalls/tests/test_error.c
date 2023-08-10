#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int fd;

	fd = open(argv[1], O_RDONLY);
	write(fd, "foo", 3);

	return errno;
}
