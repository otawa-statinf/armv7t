/*** This file was generated automatically by generate_interface_code.py. ***/
#ifndef INTERFACE_CODE_DUMP_REGS_C
#define INTERFACE_CODE_DUMP_REGS_C

#include "interface_code.h"
#include <stdio.h>

/**
 * Dump compared values of registers to stdout.
 */
void dump_regs(void) {
	int i;
	char buf[100];

	printf("\033[1m        GLISS              ||  REMOTE\033[0m\n");
	printf("\033[1m        before  |after     ||  before  |after\033[0m\n");
	for(i = 0; i < NUM_REG; i++) {
		if (reg_infos[i].size == 64) {
			printf("%-8s%016llX|%016llX  ||  %016llX|%016llX\n", reg_infos[i].name, reg_infos[i].gliss_last, reg_infos[i].gliss, reg_infos[i].gdb_last, reg_infos[i].gdb);
		}
		else {
			if(reg_infos[i].gliss != reg_infos[i].gdb)
				printf("\033[1m\033[31m");
			printf("%-8s%08llX|%08llX  ||  %08llX|%08llX\n", reg_infos[i].name, reg_infos[i].gliss_last, reg_infos[i].gliss, reg_infos[i].gdb_last, reg_infos[i].gdb);
			if(reg_infos[i].gliss != reg_infos[i].gdb)
				printf("\033[0m");
		}
	}
}

#endif /* INTERFACE_CODE_DUMP_REGS */

