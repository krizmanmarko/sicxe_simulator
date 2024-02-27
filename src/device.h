#ifndef _DEVICE_H
#define _DEVICE_H

#include <stdint.h>
#include <sys/types.h>

int read_from_device(uint8_t n, off_t offset, uint8_t *buf); // returns -1 on fail or exits
int write_to_device(uint8_t n, uint8_t v); // returns -1 on fail or exits
int test_device(uint8_t n); // returns 1 on success 0 on fail

#endif
