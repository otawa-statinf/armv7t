/*
 * Copyright (c) 2010, IRIT - UPS <casse@irit.fr>
 *
 * This file is part of GLISS V2.
 *
 * GLISS V2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GLISS V2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GLISS V2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <arm/api.h>
#include <arm/loader.h>
#include <arm/config.h>

/**
 * Data structure for storing labels.
 */
typedef struct list_entry_t {
	const char *name;
	arm_address_t addr;
	uint32_t size;
	struct list_entry_t *next;
} list_entry_t;


/**
 * List of labels.
 */
static list_entry_t *labels = 0;

static list_entry_t *extrasyms = 0;

/**
 * Print list of labels.
 */
void print_list(list_entry_t *l) {
	fprintf(stderr, "printing list\n");
	list_entry_t *e = l;
	while(e) {
		fprintf(stderr, "\t\"%s\"\t%08X\n", e->name, e->addr);
		e = e->next;
	}
	fprintf(stderr, "end list.\n");
}


/**
 * Add a symbol to the label list.
 * @param m		List header.
 * @param n		Name of label.
 * @param a		Address of label.
 */
void add_to_list(list_entry_t **m, const char *n, arm_address_t a, uint32_t s) {

#	ifdef ARM_PROCESS_CODE_LABEL
		{ ARM_PROCESS_CODE_LABEL(a); }
#	endif

	/* build the node */
	list_entry_t *e = (list_entry_t *)malloc(sizeof(list_entry_t));
	if(e == 0) {
		fprintf(stderr, "ERROR: malloc failed\n");
		return;
	}
	e->name = n;
	e->addr = a;
	e->size = s;
	e->next = 0;

	/* find the position */
	list_entry_t *cur = *m;
	list_entry_t **prev = m;
	while(cur && cur->addr < a) {
		prev = &cur->next;
		cur = cur->next;
	}

	/* insert the node */
	*prev = e;
	if(cur)
		e->next = cur;
}


/**
 * Get the label name associated with an address
 * @param	m	the sorted list to search within
 * @para	addr	the address whose label (if any) is wanted
 * @param	name	will point to the name if a label exists, NULL otherwise
 * @return	0 if no label exists for the given address, non zero otherwise
*/
int get_label_from_list(list_entry_t *m, arm_address_t addr, const char **name) {

	/* find the entry */
	list_entry_t *e = m;
	while(e && e->addr < addr)
		e = e->next;

	/* found ? */
	if(e && e->addr == addr) {
		*name = e->name;
		return 1;
	}

	/* not found */
	else {
		*name = 0;
		return 0;
	}
}

uint32_t get_item_from_list(list_entry_t *m, arm_address_t addr, list_entry_t **r) {

	/* find the entry */
	list_entry_t *e = m;
	while(e && e->addr < addr)
		e = e->next;

	/* found ? */
	if(e && e->addr == addr) {
		*r = e;
		return 1;
	}

	/* not found */
	else {
		return 0;
	}
}

/**
 * Get the label name associated with an address
 * @param	m	the sorted list to search within
 * @para	addr	the address whose label (if any) is wanted
 * @param	name	will point to the name if a label exists, NULL otherwise
 * @return	0 if no label exists for the given address, non zero otherwise
*/
uint32_t get_size_from_list(list_entry_t *m, arm_address_t addr) {

	/* find the entry */
	list_entry_t *e = m;
	while(e && e->addr < addr)
		e = e->next;

	/* found ? */
	if(e && e->addr == addr) {
		return e->size;
	}

	/* not found */
	else {
		return 0;
	}
}


/**
 * Get the closer name associated with an address
 * @param	m	the sorted list to search within
 * @para	addr	the address whose label (if any) is wanted
 * @return	Closer entry associated with address or null.
*/
list_entry_t *get_closer_label_from_list(list_entry_t *m, arm_address_t addr) {
	list_entry_t *e = m, *prev = 0;
	while(e && e->addr <= addr) {
		prev = e;
		e = e->next;
	}
	return prev;
}


/**
 * Destroy the label list.
 * @param m		Label list header.
 */
void destroy_list(list_entry_t *m) {
	list_entry_t *cur = m;
	while(cur) {
		list_entry_t *next = cur->next;
		free(cur);
		cur = next;
	}
}


/**
 * Called when the option parsing fails.
 * @param msg	Formatted string of the message.
 * @param ...	Free arguments.
 */
static void fail_with_help(const char *msg, ...) {
	va_list args;

	/* display syntax */
	fprintf(stderr, "SYNTAX: disasm ");
	if(arm_modes[0].name)
		fprintf(stderr, "[-m MODE] ");
	fprintf(stderr, "EXECUTABLE\n");

	/* display modes */
	if(arm_modes[0].name) {
		int i, fst = 1;
		fprintf(stderr, "MODE may be one of ");
		for(i = 0; arm_modes[i].name; i++) {
			if(!fst)
				fprintf(stderr, ", ");
			else
				fst = 0;
			fputs(arm_modes[i].name, stderr);
		}
		fprintf(stderr, "\n");
	}

	/* display error message */
	fprintf(stderr, "\nERROR: ");
	va_start(args, msg);
	vfprintf(stderr, msg, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(1);
}


/**
 * Convert an address to a label if available.
 * @param address	Address to get label for.
 * @return			Result string of conversion.
 */
char *arm_solve_label_disasm(arm_address_t address) {
	static char buf[256];
	list_entry_t *lab = get_closer_label_from_list(labels, address);
	if(!lab)
		sprintf(buf, "%08x", address);
	else if(lab->addr == address)
		snprintf(buf, sizeof(buf), "%08x <%s>", address, lab->name);
	else
		snprintf(buf, sizeof(buf), "%08x <%s+0x%x>", address, lab->name, address - lab->addr);
	return buf;
}


/**
 * Disassembly entry point.
 */
int main(int argc, char **argv) {
	arm_platform_t *pf;
	int s_it;
	arm_loader_sect_t *s_tab;
	int sym_it;
	int nb_sect_disasm = 0;
	arm_loader_t *loader;
	int max_size = 0, i, j;
	int min_size = 42;
	char *exe_path = 0;
	arm_inst_t *(*decode)(arm_decoder_t *decoder, arm_address_t address) = arm_decode;
	int i_sect;
	arm_decoder_t *d;

	/* test arguments */
	for(i = 1; i < argc; i++) {
		if(arm_modes[0].name && strcmp(argv[i], "-m") == 0) {
			i++;
			if(i >= argc)
				fail_with_help("no argument for -m option");
			for(j = 0; arm_modes[j].name; j++)
				if(strcmp(argv[i], arm_modes[j].name) == 0) {
					decode = arm_modes[j].decode;
					break;
				}
			if(!arm_modes[j].name)
				fail_with_help("no mode named %s", argv[i]);
		}
		else if(argv[i][0] == '-')
			fail_with_help("unknown option %s", argv[i]);
		else if(exe_path)
			fail_with_help("several executable paths given");
		else
			exe_path = argv[i];
	}
	if(!exe_path)
		fail_with_help("no executable path given!");

	/* we need a loader alone for sections */
	loader = arm_loader_open(exe_path);
	if (loader == NULL) {
		fprintf(stderr, "ERROR: cannot load the given executable : %s.\n", exe_path);
		return 2;
	}

	/* display sections */
	printf("found %d sections in the executable %s\n", arm_loader_count_sects(loader)-1, exe_path);
	s_tab = (arm_loader_sect_t *)malloc(arm_loader_count_sects(loader) * sizeof(arm_loader_sect_t));
	for(s_it = 0; s_it < arm_loader_count_sects(loader); s_it++) {
		arm_loader_sect_t data;
		arm_loader_sect(loader, s_it, &data);
		if(data.type == ARM_LOADER_SECT_TEXT) {
			s_tab[nb_sect_disasm++] = data;
			printf("[X]");
		}
		printf("\t%20s\ttype:%08x\taddr:%08x\tsize:%08x\n", data.name, data.type, data.addr, data.size);
	}
	printf("found %d sections to disasemble\n", nb_sect_disasm);

	/* display symbols */
	printf("\nfound %d symbols in the executable %s\n", arm_loader_count_syms(loader)-1, exe_path);
	for(sym_it = 0; sym_it < arm_loader_count_syms(loader); sym_it++) {
		arm_loader_sym_t data;
		arm_loader_sym(loader, sym_it, &data);
		if(data.sect != 1)
			continue;

		if(data.type == ARM_LOADER_SYM_CODE) {
			printf("[L]");
			add_to_list(&labels, data.name, data.value, data.size);
		}
		else if(data.type == ARM_LOADER_SYM_DATA)
			add_to_list(&labels, data.name, data.value, data.size);
		else if(data.type == ARM_LOADER_SYM_NO_TYPE) {
			if(strncmp(data.name, "$", 1) == 0) {
				add_to_list(&extrasyms, data.name, data.value, data.size);
			}
			else if(strcmp(data.name, "") != 0)
				add_to_list(&labels, data.name, data.value, data.size);
		}
		printf("\t%20s\tvalue:%08X\tsize:%08X\tinfo:%08X\tshndx:%08X\n", data.name, data.value, data.size, data.type, data.sect);
	}

	/* configure disassembly */
	arm_solve_label = arm_solve_label_disasm;

	/* create the platform */
	pf = arm_new_platform();
	if(pf == NULL) {
		fprintf(stderr, "ERROR: cannot create the platform.");
		destroy_list(labels);
		return 1;
	}

	/* load it */
	arm_loader_load(loader, pf);

	d = arm_new_decoder(pf);
	/* multi iss part, TODO: improve */
	arm_state_t *state = arm_new_state(pf);
	/* not really useful as select condition for instr set will never change as we don't execute here,
	 * changing instr set should be done manually by manipulating state */
	arm_set_cond_state(d, state);

	/* compute instruction max size */
	for(i = 1; i < ARM_TOP; i++) {
		int size = arm_get_inst_size_from_id(i) / 8;
		if(size > max_size)
			max_size = size;
		if(size < min_size)
			min_size = size;
	}

	/* disassemble the sections */
	for(i_sect = 0; i_sect<nb_sect_disasm; i_sect++) {

		/* display new section */
		arm_address_t adr_start = s_tab[i_sect].addr;
		arm_address_t adr_end = s_tab[i_sect].addr + s_tab[i_sect].size;
		arm_address_t prev_addr = 0;
		printf("\ndisasm new section, addr=%08x, size=%08x\n", s_tab[i_sect].addr, s_tab[i_sect].size);

		/* traverse all instructions */
		list_entry_t *cur_sym=NULL;
		while (adr_start < adr_end) {
			list_entry_t *le=NULL, *ee=NULL;
			get_item_from_list(labels, adr_start, &le);
			get_item_from_list(extrasyms, adr_start, &ee);

			/* display label */
			if(le) {
				cur_sym = le;
				printf("\n%08x <%s> (size: %u, last addr: %8x)\n", adr_start, le->name, cur_sym->size, adr_start+cur_sym->size);
			}

			if(ee) {
				if(strncmp(ee->name, "$t", 2) == 0) {
					//This is the beginning of a Thumb mode.
					state->APSR = state->APSR | 0x00000020; //put 6th bit to 1
					arm_set_cond_state(d, state);
				}
				else if(strncmp(ee->name, "$d", 2) == 0) {
					//This is data stuffs
					//consume data up to the end of the symbol, or up to the next symbol, or up to the next extra symbol
					arm_address_t stop_data_at = adr_start;
					if(cur_sym && cur_sym->next && cur_sym->next->addr < stop_data_at)
						stop_data_at = cur_sym->next->addr;
					if(ee->next && ee->next->addr < stop_data_at)
						stop_data_at = ee->next->addr;
					if(cur_sym && cur_sym->size > 0)
						stop_data_at = cur_sym->addr+cur_sym->size;

					if(adr_start >= stop_data_at)
						stop_data_at = adr_start+4; //avoid looping, it happened that the cur_sym->size was actually a bit short

					while(adr_start < stop_data_at) {
						printf(" %8x:\t", adr_start);
						printf(".word %8x\n", arm_mem_read32(arm_get_memory(pf, 0), adr_start));
						adr_start += 4;
					}
					adr_start = stop_data_at; //in case this was not aligned
					
					continue;
				}
			}

			if(adr_start >= cur_sym->addr+cur_sym->size) {
				//some weird/unused remaining bits after the symbols
				adr_start += min_size;
				continue;
			}

			/* disassemble instruction */
			int size;
			char buff[100];
			arm_inst_t *inst = decode(d, adr_start);
			arm_disasm(buff, inst);

			printf(" %8x:\t", adr_start);

			/* display the instruction bytes */
			size = arm_get_inst_size(inst) / 8;
			for(i = 0; i < max_size; i++) {
				if(i < size)
					printf("%02x", arm_mem_read8(arm_get_memory(pf, 0), adr_start + i));
				else
					fputs("  ", stdout);
			}

			/* displat the instruction */
			printf("\t%s\n", buff);
			/* inst size is given in bit, we want it in byte */
			adr_start += size;
		}
	}

	/* cleanup */
	arm_delete_decoder(d);
	arm_unlock_platform(pf);
	destroy_list(labels);

	return 0;
}
