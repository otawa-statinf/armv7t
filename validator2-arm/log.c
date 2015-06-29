/*********************
Gliss CPU simulator validator
log.c : logging subsystem
**********************/


#include "internal.h"
#include "all_inc.h"


FILE * logfile;
int do_logging;

int open_log_file(char * fpath)
	{
	logfile = fopen(fpath, "w");
	if ( logfile == NULL ) 
		{
		perror("Opening log file");
		exit(1);
		}
	else return 0;	
	}
	
void close_log_file()
	{
	fclose(logfile);
	}

extern int verbose;

void log_msg(char * fmt, ...) {

	// logging
	if(do_logging) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(logfile, fmt, ap);
		va_end(ap);
	}

	// just verbose
	if(verbose) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}
