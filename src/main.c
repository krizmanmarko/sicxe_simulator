#include <stdio.h>
#include "machine.h"
#include "loader.h"

int main(int argc, char **argv)
{
	load_section("/tmp/echo.obj");

	while (execute() >= 0) {}
	return 0;
}
