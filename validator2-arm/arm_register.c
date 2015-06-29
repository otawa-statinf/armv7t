#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "arm_register.h"
#include "interface_code.h"

/* rendering of bankable ARM registers */


/**
 * Compute real index for multiplexed GPR by mode.
 * @param st	Current state.
 * @param idx	Opcode index.
 * @return		Actual index in GPR.
 */
static unsigned int real_idx(arm_state_t * st, unsigned int idx) {
	if (idx <= 7)
		return idx;
	else if (idx <= 12) {
		if ((st->APSR & 0x1F) == 17) /* mode == FIQ */
			return idx + 8;
		else
			return idx;
	} else if (idx <= 14) {
		switch (st->APSR & 0x1F) {
			case 19:	return idx + 10;
			case 23:	return idx + 12;
			case 17:	return idx + 8;
			case 18:	return idx + 16;
			case 27:	return idx + 14;
			default:	return idx;
		}
	} else
		return 15;

}


/**
 * Get register value by index.
 * @param st	Current state.
 * @param idx	Register index.
 * @return		Register value.
 */
uint32_t get_arm_reg(arm_state_t* st, unsigned int idx) {
	return st->GPR[real_idx(st, idx)];
}


/**
 * Set the value of register.
 * @param st	Current state.
 * @param idx	Register index.
 * @param val	Value to set.
 */
void set_arm_reg(arm_state_t* st, unsigned int idx, uint32_t val) {
	st->GPR[real_idx(st, idx)] = val;
}



#define REG_R		0
#define REG_UCPSR	1


/**
 * Find bank and register from textual representation.
 * If the register cannot be found, perform assertion failure.
 * @param desc	Register description.
 * @param st	Current state.
 * @param bank	Found bank (result).
 * @param idx	Found index (result).
 */
void get_gliss_reg_addr(char *desc, arm_state_t * st, int *bank, int *idx) {
	/*  let's hope we have only simple reg name or indexed by integer */

	/* search an index */
	char *idx_ptr = desc;
	while (*idx_ptr && (*idx_ptr != '['))
		idx_ptr++;
	if (*idx_ptr)
		*idx = strtoul(idx_ptr + 1, 0, 0);
	else
		*idx = 0;

	/* scan the type of register */
	if (strncmp("R", desc, 1) == 0)
		*bank = REG_R;
	else if (strncmp("APSR", desc, 4) == 0)
		*bank = REG_UCPSR;
	else {
		fprintf(stderr, "FATAL: cannot find register %s\n", desc);
		assert(0);
	}
}


/**
 * Get register value.
 * @param st	Current state.
 * @param idx	Register index.
 * @return		Value of register.
 */
uint64_t get_gliss_reg(arm_state_t* st, int idx) {
	switch (reg_infos[idx].gliss_reg) {
	case REG_R:
		return get_arm_reg(st, reg_infos[idx].gliss_idx);
	case REG_UCPSR:
		return st->APSR;
	default:
		assert(0);
		return 0;
	}
}
