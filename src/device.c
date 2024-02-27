#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/stat.h>

#include "device.h"

int read_from_device(uint8_t n, off_t offset, uint8_t *buf)
{
	int fd, retval;
	char pathname[7]; // XX.dev

	if (n == 0)
		return read(STDIN_FILENO, buf, 1);

	sprintf(pathname, "%hhX.dev", n);

	fd = open(pathname, O_RDONLY);
	printf("%s\n", pathname);
	assert(fd >= 0);
	assert(lseek(fd, offset, SEEK_SET) >= (off_t) 0);
	retval = read(fd, buf, 1);
	assert(close(fd) == 0);

	return retval;
}

int write_to_device(uint8_t n, uint8_t v)
{
	int fd, retval;
	char pathname[7];

	if (n == 1)
		return write(STDOUT_FILENO, &v, 1);
	else if (n == 2)
		return write(STDERR_FILENO, &v, 1);

	sprintf(pathname, "%hhX.dev", n);

	fd = open(pathname, O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR);
	assert(fd >= 0);
	retval = write(fd, &v, 1);
	assert(close(fd) == 0);

	return retval;
}

int test_device(uint8_t n)
{
	int fd;
	char pathname[7];

	if (n == 0 || n == 1 || n == 2)
		return 1;

	sprintf(pathname, "%02hhX.dev", n);
	fd = open(pathname, O_RDWR);
	if (fd < 0)
		return 0;
	close(fd);
	return 1;
}
