#include <string.h>
#include <stdio.h>
#include "machine.h"
#include "opcode.h"
#include "device.h"

uint32_t regs[9];

// BEWARE: endiannes in SIC is opposite to mine
uint8_t mem[MAX_ADDRESS + 1];

off_t read_device_offsets[255 - 3] = {0}; // not including 0, 1, 2

uint32_t get_reg(int id)
{
	if (0 <= id && id <= 9 && id != 7) {
		if (regs[id] > 0x00ffffff)
			regs[id] &= ~(0xff << 24);
		return regs[id];
	}
	return 0;
}

void set_reg(int id, uint32_t v)
{
	if (0 <= id && id <= 9 && id != 7) {
		regs[id] = v;
		regs[id] &= ~(0xff << 24);
	}
}

static uint8_t peek(uint32_t addr)
{
	if (addr > MAX_ADDRESS)
		return 0;
	return mem[addr];
}

static void poke(uint32_t addr, uint8_t v)
{
	if (addr <= MAX_ADDRESS)
		mem[addr] = v;
}

static uint8_t get_reg_lsb(uint32_t regval)
{
	return (uint8_t) (regval & 0xff);
}

static uint8_t get_lsb(uint32_t word)
{
	return (uint8_t) (word >> 16);
}

static void set_cc(int comp)
{
	uint32_t sw = get_reg(SW);
	if (comp < 0) {
		sw |= (1 << 6);
		sw &= ~(1 << 7);
	} else if (comp == 0) {
		sw &= ~(1 << 6);
		sw &= ~(1 << 7);
	} else {
		sw &= ~(1 << 6);
		sw |= (1 << 7);
	}
	set_reg(SW, sw);
}

static int get_cc()
{
	uint32_t sw = get_reg(SW);
	if (sw & (1 << 6))
		return -1;
	else if (sw & (1 << 7))
		return 1;
	return 0;
}

// just for debugging purposes
static void not_implemented()
{
	while (1);
}

static uint32_t try_exec_format_1(uint32_t opcode)
{
	switch (opcode) {
		// none of these instructions need to be implemented
		case FIX:
		case FLOAT:
		case HIO:
		case NORM:
		case SIO:
		case TIO:
			not_implemented();
			return opcode;
		default:
			return 0xffffffff;
	}
}

static void addr(int r1, int r2)
{
	set_reg(r2, get_reg(r2) + get_reg(r1));
}

static void clear(int r1)
{
	set_reg(r1, 0);
}

static void divr(int r1, int r2)
{
	if (get_reg(r1) != 0)
		set_reg(r2, get_reg(r2) / get_reg(r1));
}

static void mulr(int r1, int r2)
{
	set_reg(r2, get_reg(r2) * get_reg(r1));
}

static void rmo(int r1, int r2)
{
	set_reg(r2, get_reg(r1));
}

static void shiftl(int r1, int r2)
{
	int n = r2 + 1;
	uint32_t tmp_reg = get_reg(r1);
	uint32_t a = (tmp_reg << n) & ~(0xff << 24);
	uint32_t b = tmp_reg >> (24 - n);
	set_reg(r1, a | b);
}

static void shiftr(int r1, int r2)
{
	int n = r2 + 1;
	uint32_t tmp_reg = get_reg(r1);
	int bit = tmp_reg & (1 << 23);
	for (int i = 0; i < n; i++) {
		tmp_reg >>= 1;
		tmp_reg &= ~(1 << 23);
		tmp_reg |= bit;
	}
	set_reg(r1, tmp_reg);
}

static int compare(int a, int b)
{
	return a - b;
}

// not tested yet
static void tixr(int r1)
{
	set_reg(X, get_reg(X) + 1);
	set_cc(get_reg(X) - r1);
}

static uint32_t try_exec_format_2(uint32_t opcode)
{
	uint8_t just_opcode = opcode >> 8;
	int r1 = (opcode & 0xf0) >> 4;
	int r2 = opcode & 0x0f;
	switch (just_opcode) {
		case ADDR:
			addr(r1, r2);
			return opcode;
		case CLEAR:
			clear(r1);
			return opcode;
		case DIVR:
			divr(r1, r2);
			return opcode;
		case MULR:
			mulr(r1, r2);
			return opcode;
		case RMO:
			rmo(r1, r2);
			return opcode;
		case SHIFTL:
			shiftl(r1, r2);
			return opcode;
		case SHIFTR:
			shiftr(r1, r2);
			return opcode;
		case SVC:
			not_implemented();
			return opcode;
		case TIXR:
			tixr(r1);
			return opcode;
		default:
			return 0xffffffff;
	}
}

uint8_t get_byte(uint32_t addr)
{
	if (addr > MAX_ADDRESS)
		return 0xff;
	return peek(addr);
}

void set_byte(uint32_t addr, uint8_t v)
{
	if (addr < MAX_ADDRESS)
		poke(addr, v);
}

// addr + 2 = lsb
// addr + 1 = ...
// addr + 0 = msb
uint32_t get_word(uint32_t addr)
{
	uint32_t word = 0;
	word |= peek(addr) << 16;
	word |= peek(addr + 1) << 8;
	word |= peek(addr + 2);
	return word;
}

// addr + 2 = lsb
// addr + 1 = ...
// addr + 0 = msb
void set_word(uint32_t addr, uint32_t v)
{
	uint32_t mask = 0x00ff0000;
	uint32_t tmp = (v & mask) >> 16;
	poke(addr, tmp);

	mask = 0x0000ff00;
	tmp = (v & mask) >> 8;
	poke(addr + 1, tmp);

	mask = 0x000000ff;
	tmp = v & mask;
	poke(addr + 2, tmp);
}

static void rd(uint8_t operand)
{
	int dev, retval;
	uint8_t buf;
	uint32_t big_buf;
	dev = operand - 3;
	if (operand > 2) {
		retval = read_from_device(operand, read_device_offsets[dev], &buf);
		if (retval > 0)
			read_device_offsets[dev] += retval;
	} else {
		retval = read_from_device(operand, 0, &buf);
	}
	big_buf = 0 | buf;
	set_reg(A, big_buf);
}

static void tix(uint32_t operand)
{
	set_reg(X, get_reg(X) + 1);
	set_cc(get_reg(X) - operand);
}

static void wd(uint8_t operand)
{
	write_to_device(operand, get_reg_lsb(get_reg(A)));
}

static uint32_t try_exec_format_34(uint32_t opcode)
{
	int retval, n, i, x, b, p, e;
	uint32_t un, operand;
	uint8_t just_opcode;

	un = 0x41414141; // poison value
	retval = opcode;
	n = (opcode & (1 << 17)) && 1;
	i = (opcode & (1 << 16)) && 1;
	x = (opcode & (1 << 15)) && 1;
	b = (opcode & (1 << 14)) && 1;
	p = (opcode & (1 << 13)) && 1;
	e = (opcode & (1 << 12)) && 1;
	just_opcode = opcode >> 16;
	just_opcode &= ~(1 << 1); // clear n
	just_opcode &= ~(1 << 0); // clear i

	if (n == 0 && i == 0) {
		// SIC format [ opcode(6) | n | i | x | address(15)]

		uint32_t address = opcode & 0x7fff;
		un = address;
	} else if (e == 0) {
		// format 3 [ opcode(6) | n | i | x | b | p | 0 | offset(12) ]

		int16_t offset = opcode & 0xfff;
		// [0, 4095] -> [-2048, 2047]
		if (offset > 0x7ff)
			offset -= 4096;
		if (b == 0 && p == 0)
			un = offset;
		else if (b == 1 && p == 0)
			un = get_reg(B) + offset;
		else if (b == 0 && p == 1)
			un = get_reg(PC) + offset;
	} else if (e == 1) {
		// format 4 [ opcode(6) | n | i | x | 001 | address(20) ]

		opcode <<= 8;
		opcode += fetch();
		uint32_t address = opcode & 0xfffff;
		un = address;
	}

	if (x == 1)
		un += get_reg(X);

	if (n == 0 && i == 0)
		operand = get_word(un);
	else if (n == 0 && i == 1)
		operand = un;
	else if (n == 1 && i == 0)
		operand = get_word(get_word(un));
	else if (n == 1 && i == 1)
		operand = get_word(un);


	// halt	J halt
	if (just_opcode == J && get_reg(PC) == un + 3) return 0xffffffff;

	switch (just_opcode) {
		case ADD:
			set_reg(A, get_reg(A) + operand);
			return retval;
		case ADDF:
			not_implemented();
			return retval;
		case AND:
			set_reg(A, get_reg(A) & operand);
			return retval;
		case COMP:
			set_cc(compare(get_reg(A), operand));
			return retval;
		case COMPF:
			not_implemented();
			return retval;
		case DIV:
			set_reg(A, get_reg(A) / operand);
			return retval;
		case DIVF:
			not_implemented();
			return retval;
		case J:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_reg(PC, operand);
			return retval;
		case JEQ:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			if (get_cc() == 0)
				set_reg(PC, operand);
			return retval;
		case JGT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			if (get_cc() > 0)
				set_reg(PC, operand);
			return retval;
		case JLT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			if (get_cc() < 0)
				set_reg(PC, operand);
			return retval;
		case JSUB:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_reg(L, get_reg(PC));
			set_reg(PC, operand);
			return retval;
		case LDA:
			set_reg(A, operand);
			return retval;
		case LDB:
			set_reg(B, operand);
			return retval;
		case LDCH:
			set_reg(A, get_lsb(operand));
			return retval;
		case LDF:
			not_implemented();
			return retval;
		case LDL:
			set_reg(L, operand);
			return retval;
		case LDS:
			set_reg(S, operand);
			return retval;
		case LDT:
			set_reg(T, operand);
			return retval;
		case LDX:
			set_reg(X, operand);
			return retval;
		case LPS:
			not_implemented();
			return retval;
		case MUL:
			set_reg(A, get_reg(A) * operand);
			return retval;
		case MULF:
			not_implemented();
			return retval;
		case OR:
			set_reg(A, get_reg(A) | operand);
			return retval;
		case RD:
			rd(get_lsb(operand));
			return retval;
		case RSUB:
			set_reg(PC, get_reg(L));
			return retval;
		case STA:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(A));
			return retval;
		case STB:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(B));
			return retval;
		case STCH:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_byte(operand, get_reg_lsb(get_reg(A)));
			return retval;
		case STF:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			not_implemented();
			return retval;
		case STL:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(L));
			return retval;
		case STS:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(S));
			return retval;
		case STT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(T));
			return retval;
		case STX:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			set_word(operand, get_reg(X));
			return retval;
		case SUB:
			set_reg(A, get_reg(A) - operand);
			return retval;
		case SUBF:
			not_implemented();
			return retval;
		case TD:
			set_cc(test_device(get_lsb(operand)));
			return retval;
		case TIX:
			tix(operand);
			return retval;
		case WD:
			wd(get_lsb(operand));
			return retval;
		default:
			return 0xffffffff;
	}
}

void load_memory(uint32_t address, uint8_t *buf, int count)
{
	memcpy(&mem[address], buf, count);
}

uint8_t fetch()
{
	int pc = get_reg(PC);
	uint8_t tmp = get_byte(pc);
	set_reg(PC, pc + 1);
	return tmp;
}

uint32_t execute()
{
	int retval;
	uint32_t opcode;

	opcode = fetch();
	retval = try_exec_format_1(opcode);
	if (retval != 0xffffffff)
		return retval;

	opcode <<= 8;
	opcode += fetch();
	retval = try_exec_format_2(opcode);
	if (retval != 0xffffffff)
		return retval;

	opcode <<= 8;
	opcode += fetch();
	retval = try_exec_format_34(opcode);
	if (retval != 0xffffffff)
		return retval;

	return 0xffffffff;
}
