#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "loader.h"
#include "machine.h"

static char *read_line(int fd)
{
	int retval;
	char *line = (char *)malloc(LINE_LEN);
	for (int i = 0; i < LINE_LEN; i++) {
		char c;
		retval = read(fd, &c, 1);
		assert(retval >= 0);
		if (retval == 0) {
			c = '\0';
		}

		if (c == '\n') {
			line[i] = '\0';
			return line;
		}
		line[i] = c;
	}
	free(line);
	return NULL;
}

static uint8_t __hexascii_to_number(char hexascii)
{
	char lower = hexascii | (1 << 5);
	switch (lower) {
		case '0':
			return 0x00;
		case '1':
			return 0x01;
		case '2':
			return 0x02;
		case '3':
			return 0x03;
		case '4':
			return 0x04;
		case '5':
			return 0x05;
		case '6':
			return 0x06;
		case '7':
			return 0x07;
		case '8':
			return 0x08;
		case '9':
			return 0x09;
		case 'a':
			return 0x0a;
		case 'b':
			return 0x0b;
		case 'c':
			return 0x0c;
		case 'd':
			return 0x0d;
		case 'e':
			return 0x0e;
		case 'f':
			return 0x0f;
		default:
			assert(0);
	}
}

static uint32_t hexascii_to_number(char *line, int begin, int length)
{
	if (length > 6)
		return 0;

	uint32_t number = 0;
	for (int i = 0; i < length; i++) {
		char c = line[begin + i];
		number += __hexascii_to_number(c);
		if (i < length - 1) {
			number <<= 4;
		}
	}
	return number;
}

void load_section(char *pathname)
{
	int fd, initialized;
	char record_type;
	char *line;

	// H
	char name[7] = {'\0'};
	uint32_t base_address, length;
	// T
	uint32_t address;
	uint8_t tcode_length;
	uint8_t *tcode;
	// E
	uint32_t code_begin_address;

	fd = open(pathname, O_RDONLY);
	assert(fd >= 0);
	initialized = 0;

	line = read_line(fd);
	while (line != NULL) {
		record_type = line[0];
		if (record_type == 'H') {
			for (int i = 0; i < 6; i++) {
				name[i] = line[i + 1];
			}
			base_address = hexascii_to_number(line, 7, 6);
			length = hexascii_to_number(line, 13, 6);
			initialized = 1;
			printf("[+] Loading %s (base: 0x%06x, len: %d)\n",
			       name, base_address, length);
		} else if (record_type == 'T' && initialized) {
			address = hexascii_to_number(line, 1, 6);
			tcode_length = hexascii_to_number(line, 7, 2);
			tcode = (uint8_t *)malloc(tcode_length * sizeof(uint8_t));
			for (int i = 0; i < tcode_length; i++) {
				uint8_t a = hexascii_to_number(line, 9 + i * 2, 1);
				uint8_t b = hexascii_to_number(line, 9 + i * 2 + 1, 1);
				tcode[i] = (a << 4) + b;
			}
			load_memory(address, tcode, tcode_length);
			printf("[+] 0x%06x: written %d bytes\n",
			       address, tcode_length);
			free(tcode);
		} else if (record_type == 'E' && initialized) {
			code_begin_address = hexascii_to_number(line, 1, 6);
			set_reg(PC, code_begin_address);
			printf("[+] entry point set to 0x%06x\n",
			       code_begin_address);
		}
		free(line);
		line = read_line(fd);
	}
	assert(close(fd) >= 0);
}
