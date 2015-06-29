#ifndef LEON_REGISTER_H
#define LEON_REGISTER_H

#include <stdint.h>
#include <arm/api.h>

uint32_t get_arm_reg(arm_state_t* st, unsigned int idx);
void set_arm_reg(arm_state_t * st, unsigned int idx, uint32_t val);
void get_gliss_reg_addr(char *desc, arm_state_t* st, int *bank, int *idx);
uint64_t get_gliss_reg(arm_state_t * st, int idx);

#endif /* LEON_REGISTER_H */
