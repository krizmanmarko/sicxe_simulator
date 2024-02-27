#ifndef _MACHINE_H
#define _MACHINE_H

#include <stdint.h>
#define MAX_ADDRESS 0xfffff

enum {A, X, L, B, S, T, PC = 8, SW};

uint32_t get_reg(int id);
void set_reg(int id, uint32_t v);
uint8_t get_byte(uint32_t addr);
void set_byte(uint32_t addr, uint8_t v);
uint32_t get_word(uint32_t addr);
void set_word(uint32_t addr, uint32_t v);

uint8_t fetch();
uint32_t execute(); // returns opcode executed

void load_memory(uint32_t address, uint8_t *buf, int count);

#endif
