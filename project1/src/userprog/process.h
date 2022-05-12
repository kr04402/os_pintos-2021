#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

int parseline(char*buf,char**argv,int*argc);		/* parse */
bool stack_pass(char**argv,int argc,void** esp);	/* pass arguments to stack */
#endif /* userprog/process.h */
