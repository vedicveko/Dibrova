/******************************************************************************
*  spec_names.h								      *
* 									      *
*  defines etc for spec_names.c						      *
******************************************************************************/
 
 
#ifndef SPECIAL
#error structs.h must be included!
#else
typedef SPECIAL(*proctype);
char *get_spec_name(SPECIAL(func));
proctype get_spec_proc(char *name, int type);
void list_spec_procs(struct char_data *ch);
#endif

