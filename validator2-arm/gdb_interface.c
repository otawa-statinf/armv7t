/*********************
Gliss CPU simulator validator
gdb_interface.c : gdb-related functions
**********************/

#define GDB_INTERFACE_C
#include "all_inc.h"
#include "internal.h"


/**
 * Send the given command and wait for reply.
 * @param cmd			Command to send.
 * @param replybuf		Buffer for reply.
 * @param printreply	1 to display the reply, 0 for quiet mode.
 */
void send_gdb_cmd(char *cmd, char *replybuf, int printreply) {
	if(cmd[strlen(cmd)-1] != '\n')
		fprintf(stderr, "WARNING: no newline at end of command %s\n", cmd);
	write(to_gdb_pipe[1], cmd, strlen(cmd));
	log_msg("CMD: %s", cmd);
	wait_for_gdb_output(replybuf, printreply);
}


/**
 * Wait for a reply.
 * @param replybuf		Reply buffer to use.
 * @param printreply	True to print GDB replies.
 * @return				Ever 0 (???).
 * TODO		Pass replybuf size as parameter to avoid ugly 3999 size.
 */
int wait_for_gdb_output(char *replybuf, int printreply) {
	fgets(replybuf, 3999, from_gdb);
	/*log_msg("GDB: %s", replybuf);*/

	// GDB is used to add its own command prompt everywhere: it's a pain to remove it!
	if(*replybuf=='(' && *(replybuf + 1) =='g') {

		// copy paste instead of recursion for performance reasons
		fgets(replybuf, 3999, from_gdb);
		log_msg("GDB: %s", replybuf);
		if(*replybuf=='(' && *(replybuf+1) =='g') {
			wait_for_gdb_output(replybuf, 0);
		}

		// not interesting: just ignore
		if(*replybuf != '*' && *replybuf != '^')
			wait_for_gdb_output(replybuf, printreply);
	}

	// not interesting: just ignore
	if(*replybuf != '*' && *replybuf != '^') {
		log_msg("GDB: %s", replybuf);
		wait_for_gdb_output(replybuf, printreply);
	}
	return 0;
}


/**
 * Try to match output with the given pattern.
 * @param replybuf		Current reply content.
 * @param pattern		Pattern to match.
 * @param is_fatal		Error mode.
 * @param error_label	Error message.
 * @return				0 for success, non-zero else.
 */
int match_gdb_output(char * replybuf, char * pattern, int is_fatal, char * error_label) {
	if(strstr(replybuf, "^error") || !strstr(replybuf, pattern))
		switch(is_fatal) {
		case IS_ERROR:
			fprintf(stderr, "ERROR: %s: ", error_label);
			fprintf(stderr, "%s\n", replybuf);
			exit(1);
			break;
		case IS_WARNING:
			fprintf(stderr, "WARNING: %s: ", error_label);
			fprintf(stderr, "%s\n I will attempt to continue.", replybuf);
			return 1;
			break;
		case IS_HANDLED_ELSEWHERE:
			return 1;
			break;
		}
	return 0;
}



void test_reply(char * replybuf)
{
	if ( *replybuf != '^' || *(replybuf + 1) != 'd')
	{
		fprintf(stderr, "ERROR: Unable to read register value: ");
		fprintf(stderr, "%s\n", replybuf);
		exit(1);
	}
}

void read_gdb_output_register_value_i64(char * replybuf, int64_t * regval)
{
	test_reply(replybuf);

	char * runptr = replybuf + 13;//on saute sur le premier chiffre
// printf("output_lli, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtoll(runptr, &endptr, 0);
// printf("output_lli, val=[%016llX]\n", *regval);
}

void read_gdb_output_register_value_i32(char * replybuf, int32_t * regval)
{
	test_reply(replybuf);

	char * runptr = replybuf + 13;//on saute sur le premier chiffre
// printf("output_li, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtol(runptr, &endptr, 0);
// printf("output_li, val=[%08X]\n", *regval);
}

void read_gdb_output_register_value_f64(char * replybuf, double * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
// printf("output_lf, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtod(runptr, &endptr);
// printf("output_lf, val=[%08X]\n", *regval);
}

void read_gdb_output_register_value_f32(char * replybuf, float * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
// printf("output_f, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtof(runptr, &endptr);
// printf("output_f, val=[%08X]\n", *regval);
}

void read_gdb_output_register_value_u64(char * replybuf, uint64_t * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
// printf("output_llu, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtoull(runptr, &endptr, 0);
// printf("output_llu, val=[%08X]\n", *regval);
}

void read_gdb_output_register_value_u32(char * replybuf, uint32_t * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
// printf("output_llu, runptr=[%s]\n", runptr);
	char *endptr = runptr;
	*regval = strtoul(runptr, &endptr, 0);
// printf("output_llu, val=[%08X]\n", *regval);
}

void read_gdb_output_register_value_16(char * replybuf, uint16_t * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
	char *endptr = runptr;
	*regval = (strtoul(runptr, &endptr, 0) & 0xFFFF) ;
}

void read_gdb_output_register_value_8(char * replybuf, uint8_t * regval)
{
	test_reply(replybuf);
	char * runptr = replybuf+13; //on saute sur le premier chiffre
	char *endptr = runptr;
	*regval = (strtoul(runptr, &endptr, 0) & 0xFF) ;
}

void read_gdb_output_pc(char * replybuf, uint32_t * pc)
{
	read_gdb_output_register_value_u32(replybuf, pc);
}
#undef GDB_INTERFACE_C
