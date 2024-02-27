#ifndef _LOADER_H
#define _LOADER_H

#define LINE_LEN 100

/*
 * H | name(6) | base_address(6) | length(6)
 * T | address(6) | length(2) | code(length)
 * E | .text_address(6)
 */

void load_section(char *pathname);

#endif
