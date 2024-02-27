#include "disassembler.h"
#include <stdio.h>
#include <stdint.h>

void print_reg(int id)
{
	switch (id) {
		case 0:
			printf("A");
			return;
		case 1:
			printf("X");
			return;
		case 2:
			printf("L");
			return;
		case 3:
			printf("B");
			return;
		case 4:
			printf("S");
			return;
		case 5:
			printf("T");
			return;
		case 6:
			printf("F");
			return;
		case 8:
			printf("PC");
			return;
		case 9:
			printf("SW");
			return;
	}
}

static uint32_t try_disassemble_format_1(uint32_t opcode)
{
	switch (opcode) {
		// none of these instructions need to be implemented
		case FIX:
			puts("FIX");
			return opcode;
		case FLOAT:
			puts("FLOAT");
			return opcode;
		case HIO:
			puts("HIO");
			return opcode;
		case NORM:
			puts("NORM");
			return opcode;
		case SIO:
			puts("SIO");
			return opcode;
		case TIO:
			puts("TIO");
			return opcode;
		default:
			return 0xffffffff;
	}
}

static uint32_t try_disassemble_format_2(uint32_t opcode)
{
	uint8_t just_opcode = opcode >> 8;
	int r1 = (opcode & 0xf0) >> 4;
	int r2 = opcode & 0x0f;
	switch (just_opcode) {
		case ADDR:
			printf("ADDR ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case CLEAR:
			printf("CLEAR ");
			print_reg(r1);
			puts("");
			return opcode;
		case DIVR:
			printf("DIVR ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case MULR:
			printf("MULR ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case RMO:
			printf("RMO ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case SHIFTL:
			printf("SHIFTL ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case SHIFTR:
			printf("SHIFTR ");
			print_reg(r1);
			printf(" ");
			print_reg(r2);
			puts("");
			return opcode;
		case SVC:
			printf("SVC ");
			print_reg(r1);
			puts("");
			return opcode;
		case TIXR:
			printf("SHIFTR ");
			print_reg(r1);
			puts("");
			return opcode;
		default:
			return 0xffffffff;
	}
}

static uint32_t try_disassemble_format_34(uint32_t opcode)
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

	char *c = "";
	char *xstr = "";
	char *plus = "";
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
		plus = "+";
	}

	if (x == 1) {
		un += get_reg(X);
		xstr = ", X";
	}

	if (n == 0 && i == 0) {
		operand = get_word(un);
	} else if (n == 0 && i == 1) {
		operand = un;
		c = "#";
	} else if (n == 1 && i == 0) {
		operand = get_word(get_word(un));
		c = "@";
	} else if (n == 1 && i == 1) {
		operand = get_word(un);
	}


	// halt	J halt
	if (just_opcode == J && get_reg(PC) == un + 3) return 0xffffffff;

	switch (just_opcode) {
		case ADD:
			printf("%sADD %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case ADDF:
			printf("%sADDF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case AND:
			printf("%sAND %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case COMP:
			printf("%sCOMP %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case COMPF:
			printf("%sCOMPF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case DIV:
			printf("%sDIV %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case DIVF:
			printf("%sDIVF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case J:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sJ %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case JEQ:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sJEQ %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case JGT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sJGT %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case JLT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sJLT %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case JSUB:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sJSUB %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDA:
			printf("%sLDA %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDB:
			printf("%sLDB %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDCH:
			printf("%sLDCH %s%d%s\n", plus, c, operand >> 16, xstr);
			return retval;
		case LDF:
			printf("%sLDF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDL:
			printf("%sLDL %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDS:
			printf("%sLDS %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDT:
			printf("%sLDT %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LDX:
			printf("%sLDX %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case LPS:
			printf("%sLPS %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case MUL:
			printf("%sMUL %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case MULF:
			printf("%sMULF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case OR:
			printf("%sOR %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case RD:
			printf("%sRD %s%d%s\n", plus, c, operand >> 16, xstr);
			return retval;
		case RSUB:
			printf("RSUB\n");
			return retval;
		case STA:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTA %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STB:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTB %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STCH:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTCH %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STF:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STL:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTL %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STS:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTS %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STT:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTT %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case STX:
			if (n == 1 && i == 0)
				operand = get_word(un);
			else
				operand = un;
			printf("%sSTX %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case SUB:
			printf("%sSUB %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case SUBF:
			printf("%sSUBF %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case TD:
			printf("%sTD %s%d%s\n", plus, c, operand >> 16, xstr);
			return retval;
		case TIX:
			printf("%sTIX %s%d%s\n", plus, c, operand, xstr);
			return retval;
		case WD:
			printf("\n%sWD %s%d%s\n", plus, c, operand >> 16, xstr);
			return retval;
		default:
			return 0xffffffff;
	}
}

void disassemble(uint32_t opcode)
{
	// 0xff is invalid opcode
	if (try_disassemble_format_1(opcode) != 0xffffffff)
		return;
	if (try_disassemble_format_2(opcode) != 0xffffffff)
		return;
	if (try_disassemble_format_34(opcode) != 0xffffffff)
		return;
}
