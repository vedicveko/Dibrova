/**************************************************************************
*  File: triggers.c                                                       *
*                                                                         *
*  Usage: contains all the trigger functions for scripts.                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Death's Gate MUD is based on CircleMUD, Copyright (C) 1993, 94.        *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*                                                                         *
*  $Author: dibrova $
*  $Date: 2000/06/12 22:09:30 $
*  $Revision: 1.1 $
**************************************************************************/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "dg_scripts.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "olc.h"

extern struct index_data **trig_index;
extern struct room_data *world;
extern char *dirs[];

#ifndef LVL_BUILDER
#define LVL_BUILDER LVL_GOD
#endif


/* external functions from scripts.c */
void add_var(struct trig_var_data **var_list, char *name, char *value, long id);
int script_driver(const void *go, trig_data *trig, int type, int mode);
char *matching_quote(char *p);
char *str_str(char *cs, char *ct);


/* mob trigger types */
char *trig_types[] = {
  "Global", 
  "Random",
  "Command",
  "Speech",
  "Act",
  "Death",
  "Greet",
  "Greet-All",
  "Entry",
  "Receive",
  "Fight",
  "HitPrcnt",
  "Bribe",
  "Load",
  "Memory",
  "\n"
};


/* obj trigger types */
char *otrig_types[] = {
  "Global",
  "Random",
  "Command",
  "UNUSED",
  "UNUSED",
  "Timer",
  "Get",
  "Drop",
  "Give",
  "Wear",
  "UNUSED",
  "Remove",
  "UNUSED",
  "Load",
  "UNUSED",
  "\n"
};


/* wld trigger types */
char *wtrig_types[] = {
  "Global",
  "Random",
  "Command",
  "Speech",
  "UNUSED",
  "Zone Reset",
  "Enter",
  "Drop",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "\n"
};


/*
 *  General functions used by several triggers
 */


/*
 * Copy first phrase into first_arg, returns rest of string
 */
char *one_phrase(char *arg, char *first_arg)
{
    skip_spaces(&arg);

    if (!*arg)
	*first_arg = '\0';

    else if (*arg == '"')
    {
	char *p, c;

	p = matching_quote(arg);
	c = *p;
	*p = '\0';
	strcpy(first_arg, arg + 1);
	if (c == '\0')
	    return p;
	else
	    return p + 1;
    }

    else
    {
	char *s, *p;

	s = first_arg;
	p = arg;
	
	while (*p && !isspace(*p) && *p != '"')
	    *s++ = *p++;

	*s = '\0';
	return p;
    }

    return arg;
}


int is_substring(char *sub, char *string)
{
    char *s;

    if ((s = str_str(string, sub)))
    {
	int len = strlen(string);
	int sublen = strlen(sub);
	
	/* check front */
	if ((s == string || isspace(*(s - 1)) || ispunct(*(s - 1))) &&

	    /* check end */
	    ((s + sublen == string + len) || isspace(s[sublen]) ||
	     ispunct(s[sublen])))
	    return 1;
    }

    return 0;
}


/*
 * return 1 if str contains a word or phrase from wordlist.
 * phrases are in double quotes (").
 * if wrdlist is NULL, then return 1, if str is NULL, return 0.
 */
int word_check(char *str, char *wordlist)
{
    char words[MAX_INPUT_LENGTH], phrase[MAX_INPUT_LENGTH], *s;

    if (*wordlist=='*') return 1;

    strcpy(words, wordlist);
    
    for (s = one_phrase(words, phrase); *phrase; s = one_phrase(s, phrase))
	if (is_substring(phrase, str))
	    return 1;

    return 0;
}

	

/*
 *  mob triggers
 */

void random_mtrigger(char_data *ch)
{
  trig_data *t;

  if (!SCRIPT_CHECK(ch, MTRIG_RANDOM) || AFF_FLAGGED(ch, AFF_CHARM))
    return;

  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_RANDOM) && 
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }
}

void bribe_mtrigger(char_data *ch, char_data *actor, int amount)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (!SCRIPT_CHECK(ch, MTRIG_BRIBE) || AFF_FLAGGED(ch, AFF_CHARM))
    return;

  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_BRIBE) && (amount >= GET_TRIG_NARG(t))) {

      sprintf(buf, "%d", amount);
      add_var(&GET_TRIG_VARS(t), "amount", buf, 0);
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }
}

void greet_memory_mtrigger(char_data *actor)
{
  trig_data *t;
  char_data *ch;
  struct script_memory *mem;
  char buf[MAX_INPUT_LENGTH];
  int command_performed = 0;

  for (ch = world[IN_ROOM(actor)].people; ch; ch = ch->next_in_room) {
    if (!SCRIPT_MEM(ch) || !AWAKE(ch) || FIGHTING(ch) || (ch == actor) || 
	AFF_FLAGGED(ch, AFF_CHARM))
      continue;

    /* find memory line with command only */
    for (mem = SCRIPT_MEM(ch); mem && SCRIPT_MEM(ch); mem=mem->next) {
      if (GET_ID(actor)!=mem->id) continue;
      if (mem->cmd) {
        command_interpreter(ch, mem->cmd); /* no script */
        command_performed = 1;
        break;
      }
    }

    /* if a command was not performed execute the memory script */
    if (mem && !command_performed) {
      for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
        if (IS_SET(GET_TRIG_TYPE(t), MTRIG_MEMORY) &&
            CAN_SEE(ch, actor) &&
            !GET_TRIG_DEPTH(t) &&
            number(1, 100) <= GET_TRIG_NARG(t)) {
              ADD_UID_VAR(buf, t, actor, "actor", 0);
              script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
              break;
        }
      }
    }

    /* delete the memory */
    if (mem) {
      if (SCRIPT_MEM(ch)==mem) {
        SCRIPT_MEM(ch) = mem->next;
      } else {
        struct script_memory *prev;
        prev = SCRIPT_MEM(ch);
        while (prev->next != mem) prev = prev->next;
        prev->next = mem->next;
      }
      if (mem->cmd) free(mem->cmd);
      free(mem);
    }
  }
}


int greet_mtrigger(char_data *actor, int dir)
{
  trig_data *t;
  char_data *ch;
  char buf[MAX_INPUT_LENGTH];
  int rev_dir[] = { SOUTH, WEST, NORTH, EAST, DOWN, UP };
  int intermediate, final=TRUE;
 
  for (ch = world[IN_ROOM(actor)].people; ch; ch = ch->next_in_room) {
    if (!SCRIPT_CHECK(ch, MTRIG_GREET | MTRIG_GREET_ALL) || 
	!AWAKE(ch) || FIGHTING(ch) || (ch == actor) || 
	AFF_FLAGGED(ch, AFF_CHARM))
      continue;
    
    for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
      if (((IS_SET(GET_TRIG_TYPE(t), MTRIG_GREET) && CAN_SEE(ch, actor)) ||
	   IS_SET(GET_TRIG_TYPE(t), MTRIG_GREET_ALL)) && 
	  !GET_TRIG_DEPTH(t) && (number(1, 100) <= GET_TRIG_NARG(t))) {
        if (dir>=0)
          add_var(&GET_TRIG_VARS(t), "direction", dirs[rev_dir[dir]], 0);
	ADD_UID_VAR(buf, t, actor, "actor", 0);
	intermediate =  script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
        if (!intermediate) final = FALSE;
	continue;
      }
    }
  }
  return final;
}


void entry_memory_mtrigger(char_data *ch)
{
  trig_data *t;
  char_data *actor;
  struct script_memory *mem;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_MEM(ch) || AFF_FLAGGED(ch, AFF_CHARM))
    return;


  for (actor = world[IN_ROOM(ch)].people; actor && SCRIPT_MEM(ch);
       actor = actor->next_in_room) {
    if (actor!=ch && SCRIPT_MEM(ch)) {
      for (mem = SCRIPT_MEM(ch); mem && SCRIPT_MEM(ch); mem = mem->next) {
        if (GET_ID(actor)==mem->id) {
          struct script_memory *prev;
          if (mem->cmd) command_interpreter(ch, mem->cmd);
          else {
            for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
              if (TRIGGER_CHECK(t, MTRIG_MEMORY) && (number(1, 100) <=
                  GET_TRIG_NARG(t))){
                ADD_UID_VAR(buf, t, actor, "actor", 0);
                script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
                break;
              }
            }
          }
          /* delete the memory */
          if (SCRIPT_MEM(ch)==mem) {
            SCRIPT_MEM(ch) = mem->next;
          } else {
            prev = SCRIPT_MEM(ch);
            while (prev->next != mem) prev = prev->next;
            prev->next = mem->next;
          }
          if (mem->cmd) free(mem->cmd);
          free(mem);
        }
      } /* for (mem =..... */
    }
  }
}

int entry_mtrigger(char_data *ch)
{
  trig_data *t;
  
  if (!SCRIPT_CHECK(ch, MTRIG_ENTRY) || AFF_FLAGGED(ch, AFF_CHARM))
    return 1;
  
  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_ENTRY) && (number(1, 100) <= GET_TRIG_NARG(t))){
      return script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }

  return 1;
}

int command_mtrigger(char_data *actor, char *cmd, char *argument)
{
  char_data *ch, *ch_next;
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  for (ch = world[IN_ROOM(actor)].people; ch; ch = ch_next) {
    ch_next = ch->next_in_room;

    if (SCRIPT_CHECK(ch, MTRIG_COMMAND) && !AFF_FLAGGED(ch, AFF_CHARM) &&
        (actor!=ch)) {
      for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
        if (!TRIGGER_CHECK(t, MTRIG_COMMAND))
          continue;
        
        if (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t)) {
          sprintf(buf,"SYSERR: Command Trigger #%d has no text argument!",
            GET_TRIG_VNUM(t));
          mudlog(buf, NRM, LVL_BUILDER, TRUE);
          continue;
        }

        if (*GET_TRIG_ARG(t)=='*' ||
            !strn_cmp(GET_TRIG_ARG(t), cmd, strlen(GET_TRIG_ARG(t)))) {
	  ADD_UID_VAR(buf, t, actor, "actor", 0);
	  skip_spaces(&argument);
	  add_var(&GET_TRIG_VARS(t), "arg", argument, 0);
	  skip_spaces(&cmd);
	  add_var(&GET_TRIG_VARS(t), "cmd", cmd, 0);
	  
	  if (script_driver(ch, t, MOB_TRIGGER, TRIG_NEW))
	    return 1;
	}
      }
    }
  }

  return 0;
}
  

void speech_mtrigger(char_data *actor, char *str)
{
  char_data *ch, *ch_next;
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  for (ch = world[IN_ROOM(actor)].people; ch; ch = ch_next)
  {
    ch_next = ch->next_in_room;

    if (SCRIPT_CHECK(ch, MTRIG_SPEECH) && AWAKE(ch) &&
        !AFF_FLAGGED(ch, AFF_CHARM) && (actor!=ch))
      for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
        if (!TRIGGER_CHECK(t, MTRIG_SPEECH))
          continue;

        if (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t)) {
          sprintf(buf,"SYSERR: Speech Trigger #%d has no text argument!",
            GET_TRIG_VNUM(t));
          mudlog(buf, NRM, LVL_BUILDER, TRUE);
          continue;
        }

        if (((GET_TRIG_NARG(t) && word_check(str, GET_TRIG_ARG(t))) ||
             (!GET_TRIG_NARG(t) && is_substring(GET_TRIG_ARG(t), str)))) {
          ADD_UID_VAR(buf, t, actor, "actor", 0);
          add_var(&GET_TRIG_VARS(t), "speech", str, 0);
          script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
          break;
        }
      }
  }
}


void act_mtrigger(const char_data *ch, char *str, char_data *actor, 
		  char_data *victim, obj_data *object,
		  obj_data *target, char *arg)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (SCRIPT_CHECK(ch, MTRIG_ACT) && !AFF_FLAGGED(ch, AFF_CHARM) &&
      (actor!=ch))
    for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next)  {
      if (!TRIGGER_CHECK(t, MTRIG_ACT))
        continue;

      if (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t)) {
        sprintf(buf,"SYSERR: Act Trigger #%d has no text argument!",
          GET_TRIG_VNUM(t));
        mudlog(buf, NRM, LVL_BUILDER, TRUE);
        continue;
      }

      if (((GET_TRIG_NARG(t) && word_check(str, GET_TRIG_ARG(t))) ||
           (!GET_TRIG_NARG(t) && is_substring(GET_TRIG_ARG(t), str)))) {
	
	if (actor)
	  ADD_UID_VAR(buf, t, actor, "actor", 0);
	if (victim)
	  ADD_UID_VAR(buf, t, victim, "victim", 0);
	if (object)
	  ADD_UID_VAR(buf, t, object, "object", 0);
	if (target)
	  ADD_UID_VAR(buf, t, target, "target", 0);
	if (arg) {
	  skip_spaces(&arg);
	  add_var(&GET_TRIG_VARS(t), "arg", arg, 0);
	}	  
	script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
	break;
      }	
    }
}


void fight_mtrigger(char_data *ch)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (!SCRIPT_CHECK(ch, MTRIG_FIGHT) || !FIGHTING(ch) || 
      AFF_FLAGGED(ch, AFF_CHARM))
    return;
  
  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_FIGHT) &&
	(number(1, 100) <= GET_TRIG_NARG(t))){

      ADD_UID_VAR(buf, t, FIGHTING(ch), "actor", 0)
      script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }
}


void hitprcnt_mtrigger(char_data *ch)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (!SCRIPT_CHECK(ch, MTRIG_HITPRCNT) || !FIGHTING(ch) ||
      AFF_FLAGGED(ch, AFF_CHARM))
    return;
  
  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_HITPRCNT) && GET_MAX_HIT(ch) &&
	(((GET_HIT(ch) * 100) / GET_MAX_HIT(ch)) <= GET_TRIG_NARG(t))) {

      ADD_UID_VAR(buf, t, FIGHTING(ch), "actor", 0)
      script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }
}


int receive_mtrigger(char_data *ch, char_data *actor, obj_data *obj)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(ch, MTRIG_RECEIVE) || AFF_FLAGGED(ch, AFF_CHARM))
    return 1;
  
  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_RECEIVE) &&
	(number(1, 100) <= GET_TRIG_NARG(t))){

      ADD_UID_VAR(buf, t, actor, "actor", 0);
      ADD_UID_VAR(buf, t, obj, "object", 0);
      return script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


int death_mtrigger(char_data *ch, char_data *actor)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (!SCRIPT_CHECK(ch, MTRIG_DEATH) || AFF_FLAGGED(ch, AFF_CHARM))
    return 1;
  
  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_DEATH)) {

      if (actor)
	ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}

void load_mtrigger(char_data *ch)
{
  trig_data *t;

  if (!SCRIPT_CHECK(ch, MTRIG_LOAD))
    return;

  for (t = TRIGGERS(SCRIPT(ch)); t; t = t->next) {
    if (TRIGGER_CHECK(t, MTRIG_LOAD) && 
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(ch, t, MOB_TRIGGER, TRIG_NEW);
      break;
    }
  }
}


/*
 *  object triggers
 */

void random_otrigger(obj_data *obj)
{
  trig_data *t;

  if (!SCRIPT_CHECK(obj, OTRIG_RANDOM))
    return;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_RANDOM) && 
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
      break;
    }
  }
}


void timer_otrigger(struct obj_data *obj)
{
  trig_data *t;
      
  if (!SCRIPT_CHECK(obj, OTRIG_TIMER))
    return;
  
  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_TIMER))
      script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);       
  }  
     
  return;
}


int get_otrigger(obj_data *obj, char_data *actor)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(obj, OTRIG_GET))
    return 1;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_GET) && (number(1, 100) <= GET_TRIG_NARG(t))) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


/* checks for command trigger on specific object. assumes obj has cmd trig */
int cmd_otrig(obj_data *obj, char_data *actor, char *cmd,
	      char *argument, int type)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  
  if (obj && SCRIPT_CHECK(obj, OTRIG_COMMAND))
    for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
      if (!TRIGGER_CHECK(t, OTRIG_COMMAND))
        continue;
 
      if (IS_SET(GET_TRIG_NARG(t), type) &&
          (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t))) {
        sprintf(buf,"SYSERR: O-Command Trigger #%d has no text argument!",
          GET_TRIG_VNUM(t));
        mudlog(buf, NRM, LVL_BUILDER, TRUE);
        continue;
      }
       
      if (IS_SET(GET_TRIG_NARG(t), type) &&
          (*GET_TRIG_ARG(t)=='*' ||
          !strn_cmp(GET_TRIG_ARG(t), cmd, strlen(GET_TRIG_ARG(t))))) {
	
	ADD_UID_VAR(buf, t, actor, "actor", 0);
	skip_spaces(&argument);
	add_var(&GET_TRIG_VARS(t), "arg", argument, 0);
	skip_spaces(&cmd);
	add_var(&GET_TRIG_VARS(t), "cmd", cmd, 0);
	
	if (script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW))
	  return 1;
      }
    }

  return 0;
}


int command_otrigger(char_data *actor, char *cmd, char *argument)
{
  obj_data *obj;
  int i;

  for (i = 0; i < NUM_WEARS; i++)
    if (cmd_otrig(GET_EQ(actor, i), actor, cmd, argument, OCMD_EQUIP))
      return 1;
  
  for (obj = actor->carrying; obj; obj = obj->next_content)
    if (cmd_otrig(obj, actor, cmd, argument, OCMD_INVEN))
      return 1;

  for (obj = world[IN_ROOM(actor)].contents; obj; obj = obj->next_content)
    if (cmd_otrig(obj, actor, cmd, argument, OCMD_ROOM))
      return 1;

  return 0;
}


int wear_otrigger(obj_data *obj, char_data *actor, int where)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(obj, OTRIG_WEAR))
    return 1;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_WEAR)) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


int remove_otrigger(obj_data *obj, char_data *actor)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(obj, OTRIG_REMOVE))
    return 1;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_REMOVE)) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


int drop_otrigger(obj_data *obj, char_data *actor)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(obj, OTRIG_DROP))
    return 1;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_DROP) && (number(1, 100) <= GET_TRIG_NARG(t))) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


int give_otrigger(obj_data *obj, char_data *actor, char_data *victim)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!SCRIPT_CHECK(obj, OTRIG_GIVE))
    return 1;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_GIVE) && (number(1, 100) <= GET_TRIG_NARG(t))) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      ADD_UID_VAR(buf, t, victim, "victim", 0);
      return script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}

void load_otrigger(obj_data *obj)
{
  trig_data *t;

  if (!SCRIPT_CHECK(obj, OTRIG_LOAD))
    return;

  for (t = TRIGGERS(SCRIPT(obj)); t; t = t->next) {
    if (TRIGGER_CHECK(t, OTRIG_LOAD) && 
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(obj, t, OBJ_TRIGGER, TRIG_NEW);
      break;
    }
  }
}



/*
 *  world triggers
 */

void reset_wtrigger(struct room_data *room)
{
  trig_data *t;

  if (!SCRIPT_CHECK(room, WTRIG_RESET))
    return;

  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) {
    if (TRIGGER_CHECK(t, WTRIG_RESET) &&
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
      break;
    }
  }
}

void random_wtrigger(struct room_data *room)
{
  trig_data *t;

  if (!SCRIPT_CHECK(room, WTRIG_RANDOM))
    return;

  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) {
    if (TRIGGER_CHECK(t, WTRIG_RANDOM) &&
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
      break;
    }
  }
}


int enter_wtrigger(struct room_data *room, char_data *actor, int dir)
{
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];
  int rev_dir[] = { SOUTH, WEST, NORTH, EAST, DOWN, UP };

  if (!SCRIPT_CHECK(room, WTRIG_ENTER))
    return 1;

  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) {
    if (TRIGGER_CHECK(t, WTRIG_ENTER) && 
	(number(1, 100) <= GET_TRIG_NARG(t))) {
      add_var(&GET_TRIG_VARS(t), "direction", dirs[rev_dir[dir]], 0);
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      return script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
    }
  }

  return 1;
}


int command_wtrigger(char_data *actor, char *cmd, char *argument)
{
  struct room_data *room;
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!actor || !SCRIPT_CHECK(&world[IN_ROOM(actor)], WTRIG_COMMAND))
    return 0;

  room = &world[IN_ROOM(actor)];
  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) {
    if (!TRIGGER_CHECK(t, WTRIG_COMMAND))
      continue;

    if (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t)) {
      sprintf(buf,"SYSERR: W-Command Trigger #%d has no text argument!",
        GET_TRIG_VNUM(t));
      mudlog(buf, NRM, LVL_BUILDER, TRUE);
      continue;
    }

    if (*GET_TRIG_ARG(t)=='*' ||
        !strn_cmp(GET_TRIG_ARG(t), cmd, strlen(GET_TRIG_ARG(t)))) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      skip_spaces(&argument);
      add_var(&GET_TRIG_VARS(t), "arg", argument, 0);
      skip_spaces(&cmd);
      add_var(&GET_TRIG_VARS(t), "cmd", cmd, 0);
      
      return script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
    }
  }
  
  return 0;
}


void speech_wtrigger(char_data *actor, char *str)
{
  struct room_data *room;
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!actor || !SCRIPT_CHECK(&world[IN_ROOM(actor)], WTRIG_SPEECH))
    return;

  room = &world[IN_ROOM(actor)];
  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) {
    if (!TRIGGER_CHECK(t, WTRIG_SPEECH))
      continue;

    if (!GET_TRIG_ARG(t) || !*GET_TRIG_ARG(t)) {
      sprintf(buf,"SYSERR: W-Speech Trigger #%d has no text argument!",
        GET_TRIG_VNUM(t));
      mudlog(buf, NRM, LVL_BUILDER, TRUE);
      continue;
    }

    if (((GET_TRIG_NARG(t) && word_check(str, GET_TRIG_ARG(t))) ||
         (!GET_TRIG_NARG(t) && is_substring(GET_TRIG_ARG(t), str)))) {
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      add_var(&GET_TRIG_VARS(t), "speech", str, 0);
      script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
      break;
    }
  }
}

int drop_wtrigger(obj_data *obj, char_data *actor)
{
  struct room_data *room;
  trig_data *t;
  char buf[MAX_INPUT_LENGTH];

  if (!actor || !SCRIPT_CHECK(&world[IN_ROOM(actor)], WTRIG_DROP))
    return 1;

  room = &world[IN_ROOM(actor)];
  for (t = TRIGGERS(SCRIPT(room)); t; t = t->next) 
    if (TRIGGER_CHECK(t, WTRIG_DROP) &&
	(number(1, 100) <= GET_TRIG_NARG(t))) {	
      
      ADD_UID_VAR(buf, t, actor, "actor", 0);
      ADD_UID_VAR(buf, t, obj, "object", 0);
      return script_driver(room, t, WLD_TRIGGER, TRIG_NEW);
      break;
    }	

  return 1;
}
