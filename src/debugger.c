#include <stdio.h>
#include "debugger.h"
#include "disassembler.h"

int finished = 0;

static void print_menu()
{
	puts("===============================================================================");
	puts("1. continue");
	puts("2. step");
	puts("3. print registers");
	puts("4. quit");
	printf("> ");
}

static void print_registers()
{
	printf("A: 0x%06x	X: 0x%06x	L: 0x%06x	B: 0x%06x\n",
	       get_reg(A), get_reg(X), get_reg(L), get_reg(B));

	printf("S: 0x%06x	T: 0x%06x	PC: 0x%06x	SW: 0x%06x\n",
	       get_reg(S), get_reg(T), get_reg(PC), get_reg(SW));
}

static void step()
{
	print_registers();
	uint32_t opcode = execute();
	if (opcode == 0xffffffff) {
		finished = 1;
		puts("[+] Execution finished");
		return;
	}
	disassemble(opcode);
}

int main(int argc, char **argv)
{
	int choice = 0;
	if (argc < 2) {
		printf("Usage: %s <file.obj>\n", argv[0]);
		return 1;
	}
	load_section(argv[1]);
	while (choice != 4 || finished) {
		print_menu();
		scanf("%d", &choice);
		if (choice == 1) {
			if (!finished) {
				while (execute() != 0xffffffff) ;
				finished = 1;
			} else {
				puts("[+] Execution finished");
			}
		} else if (choice == 2)
			if (!finished)
				step();
			else
				puts("[+] Execution finished");
		else if (choice == 3)
			print_registers();
		else
			break;
	}
	return 0;
}
