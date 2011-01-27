/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"

/* external structs */
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];
extern int npc_steal(struct char_data *ch, struct char_data *victim);
extern struct time_info_data time_info;
extern char *rank[][3];

ACMD(do_say);
ACMD(do_assist);
void GreetPeople(struct char_data *ch);
void send_to_zone_outdoor(int zone_rnum, char *messg);
void send_to_zone_indoor(int zone_rnum, char *messg);
bool perform_wear(struct char_data * ch, struct obj_data * obj, int where, int wearall);
void perform_get_from_container(struct char_data * ch, struct obj_data * obj,
                                struct obj_data * cont, int mode, int subcmd);
SPECIAL(shop_keeper);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
int perform_remove(struct char_data *ch, int pos);

void mobaction(struct char_data *ch, char *string, char *action)
{
  sprintf(buf, "%s %s", action, string);
  command_interpreter(ch, buf);
}

void mobsay(struct char_data *ch, const char *msg)
{
  char Gbuf[MAX_INPUT_LENGTH];

  if (!msg || !*msg) {
    log("SYSERR: No text in mobsay()");
    return;
  }
  if (strlen(msg) > (MAX_INPUT_LENGTH - 1)) {
    log("SYSERR: text too long in mobsay()");
    return;
  }
  strcpy(Gbuf, msg);
  do_say(ch, Gbuf, 0, 0);
}


#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

void mobile_activity(void)
{
  register struct char_data *ch, *next_ch, *vict;
  struct obj_data *obj, *best_obj;
  struct follow_type *f;	/* Follower. */
  int door, found, max, i;
  memory_rec *names;

  int hit_roll = 0, to_hit = 0;
 
  extern int no_specials;

  ACMD(do_get);

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (!IS_MOB(ch))
      continue;

    /* Examine call for special procedure */
    if (MOB_FLAGGED(ch, MOB_SPEC) && !no_specials) {
      if (mob_index[GET_MOB_RNUM(ch)].func == NULL) {
	sprintf(buf, "SYSERR: %s (#%d): Attempting to call non-existing mob func",
		GET_NAME(ch), GET_MOB_VNUM(ch));
	log(buf);
	REMOVE_BIT_AR(MOB_FLAGS(ch), MOB_SPEC);
      } else {
	if ((mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, ""))
	  continue;		/* go to next char */
      }
    }

    /* If the mob has no specproc, do the default actions */
    if (FIGHTING(ch) || !AWAKE(ch))
      continue;

    /* Scavenger (picking up objects) */
    if (MOB_FLAGGED(ch, MOB_SCAVENGER) && !FIGHTING(ch) && AWAKE(ch))
      if (world[ch->in_room].contents && !number(0, 3)) {
	max = 1;
	best_obj = NULL;

	for (obj = world[ch->in_room].contents; obj; obj = obj->next_content) {
	  if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max &&
              GET_OBJ_EXP(obj) >= 0 ) {
	    best_obj = obj;
	    max = GET_OBJ_COST(obj);
	  }
        }

	if (best_obj != NULL) {
	  obj_from_room(best_obj);
	  obj_to_char(best_obj, ch);
	  act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);

        if(GET_RACE(ch) != RACE_NPC_ANIMAL) {
          if (GET_OBJ_TYPE(best_obj) == ITEM_LIGHT)
            perform_wear(ch, best_obj, WEAR_LIGHT, 0);
          else if (CAN_WEAR(best_obj, ITEM_WEAR_HOLD))
            perform_wear(ch, best_obj, WEAR_HOLD, 0);
          else if (CAN_WEAR(best_obj, ITEM_WEAR_WIELD))
            perform_wear(ch, best_obj, WEAR_WIELD, 0);
          else {
            for (i = 1; i < NUM_WEARS; i++) {   
               if (i == WEAR_HOLD)
                 continue;
               if (perform_wear(ch, best_obj, i, 0))
                 break;
            }
          }
         }
	}


      }

    /* Mob Movement */
    if (!MOB_FLAGGED(ch, MOB_SENTINEL) && (GET_POS(ch) == POS_STANDING) &&
	((door = number(0, 18)) < NUM_OF_DIRS) && CAN_GO(ch, door) &&
	(!ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_NOMOB) &&
	 !ROOM_FLAGGED(EXIT(ch, door)->to_room, ROOM_DEATH)) &&
	(!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
	 (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone))) {
      perform_move(ch, door, 1);
    }

    /* Aggressive Mobs */
    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) || MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN)) {
      found = FALSE;

      for(vict=world[ch->in_room].people;vict && !found;vict=vict->next_in_room) {

        if(vict == NULL) { continue; }

	if (IS_NPC(vict) || !CAN_SEE(ch, vict) ||
            PRF_FLAGGED(vict, PRF_NOHASSLE) ||
            AFF_FLAGGED(vict, AFF_INCOGNITO))
	  continue;
        if (AFF_FLAGGED(vict, AFF_DIVINE_SHIELD))
          continue;
	if (MOB_FLAGGED(ch, MOB_WIMPY) && AWAKE(vict))
	  continue;
	if (!MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_EVIL) && IS_EVIL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL) && IS_NEUTRAL(vict)) ||
	    (MOB_FLAGGED(ch, MOB_AGGR_GOOD) && IS_GOOD(vict))) {

          if (number(15, 30) <= ((GET_CHA(vict) + GET_LUCK(vict)) / 2)) {
            act("$n looks at $N with an indifference.",
                FALSE, ch, 0, vict, TO_NOTVICT);
            act("$N looks at you with an indifference.",
                FALSE, vict, 0, ch, TO_CHAR);
          } else
          /* INTIMIDATE */
          if (GET_SKILL(vict, SKILL_INTIMIDATE)) {
            act("$n cowers before $N, unwilling to attack.",
                FALSE, ch, 0, vict, TO_NOTVICT);
            act("$N cowers before you, unwilling to attack.",
                FALSE, vict, 0, ch, TO_CHAR);
          } else
          /* QUICKDRAW */
          if (GET_SKILL(vict, SKILL_QUICKDRAW) > number(0, 101))  {
            perform_quickdraw(vict, ch, SCMD_REAL_QUICKDRAW);
          } else          

	  if (GET_CLASS(ch) == CLASS_NPC_THIEF) {
	    /* Backstab the bitch! */
	    hit_roll = number (1,100) + GET_STR(ch);
	    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
	    if (GET_LEVEL(vict) >= LVL_IMMORT)  
	      hit_roll = 0;

	    if (GET_EQ(ch, WEAR_WIELD)){
	      if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) == (TYPE_PIERCE - TYPE_HIT)) {

		if (AWAKE(vict) && (hit_roll < to_hit)) {
		  damage(ch, vict, 0, SKILL_BACKSTAB, ABT_SKILL);
		} else {
		  damage(ch, vict, GET_LEVEL(ch) * 3, SKILL_BACKSTAB, ABT_SKILL);
		}	  
		
		found = TRUE;
	      } 
	    }  
	    else {
		hit(ch, vict, TYPE_UNDEFINED);
		found = TRUE;
	    }
	  } 
          else if (GET_CLASS(ch) == CLASS_NPC_MONK) {
             do_generic_skill(ch, vict, SKILL_KI, 0);
          } else {
	  hit(ch, vict, TYPE_UNDEFINED);
	  found = TRUE;
	  }
	}
      }
    }

    /* Mob Memory */
    if (MOB_FLAGGED(ch, MOB_MEMORY) && MEMORY(ch)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
	if (IS_NPC(vict) || !CAN_SEE(ch, vict) || PRF_FLAGGED(vict, PRF_NOHASSLE))
	  continue;
	for (names = MEMORY(ch); names && !found; names = names->next)
	  if (names->id == GET_IDNUM(vict) && !AFF_FLAGGED(vict, AFF_INCOGNITO)) {
	    found = TRUE;
	    act("'Hey!  You're the fiend that attacked me!!!', exclaims $n.",
		FALSE, ch, 0, 0, TO_ROOM);
	    hit(ch, vict, TYPE_UNDEFINED);
	  }
      }
    }

    /* Helper Mobs */
    if (MOB_FLAGGED(ch, MOB_HELPER)) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {
        if (ch != vict && !IS_NPC(vict) && FIGHTING(vict) &&
            IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict)) {
          if (GET_SKILL(vict, SKILL_DARK_ALLEGIANCE) &&
              GET_ALIGNMENT(ch) <= -300) {
            do_assist(ch, GET_NAME(vict), 0, 0);
            found = TRUE;
          }
        }
	if (ch != vict && IS_NPC(vict) && FIGHTING(vict) &&
            !IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict)) {
	    act("$n jumps to the aid of $N!", FALSE, ch, 0, vict, TO_ROOM);
  	    hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
	    found = TRUE;
	}
      }
      if (MOB_FLAGGED(ch, MOB_PC_ASSIST)) {
        found = FALSE;
    }
        for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room)
			if (ch != vict && !IS_NPC(vict) && FIGHTING(vict) && IS_NPC(FIGHTING(vict)) && ch != FIGHTING(vict))
			{
				for (f = vict->followers; f; f = f->next)
				{
					if (IS_AFFECTED(f->follower, AFF_CHARM) && IS_NPC(f->follower))
					{
						act("/cCWith a a great war cry, $n leaps into the fight at $N's side!\r\n/c0", FALSE, ch, 0, f->follower, TO_NOTVICT);
						act("$n leaps to your side, joining in the fray!", FALSE, ch, 0, 0, TO_VICT);
						hit(ch, vict, TYPE_UNDEFINED);
						found = TRUE;
					}
				}
				if (found != TRUE)
				{
					act("/cCWith a a great war cry, $n leaps into the fight at $N's side!\r\n/c0", FALSE, ch, 0, vict, TO_NOTVICT);
					act("$n leaps to your side, joining in the fray!", FALSE, ch, 0, 0, TO_VICT);
					hit(ch, FIGHTING(vict), TYPE_UNDEFINED);
					found = TRUE;
				}
			}
    }
    /* Add new mobile actions here */
    if (GET_CLASS(ch) == CLASS_NPC_THIEF) {
      found = FALSE;
      for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {

	if (IS_NPC(vict) || !CAN_SEE(ch, vict)) { continue; }

	switch(number(0, 8)) {
          case 1:
	    mob_steal_eq(ch, vict);
	    found = TRUE;
	    break;
	  case 2:
	    mob_steal_from_inv(ch, vict);
	    found = TRUE;
	    break;
	  case 3:
	    npc_steal(ch, vict);
	    found = TRUE;
	    break;
          default:
	    break;
	}
      }
    }


   if(GET_CLASS(ch) == CLASS_NPC_CITIZEN) {
    if(number(0, 10) == 5) {
      GreetPeople(ch);
    }
   }
   if (GET_RACE(ch) == RACE_NPC_EQUINE) {
     found = FALSE;
     for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) { 
       if (IS_NPC(vict) || !CAN_SEE(ch, vict)) { 
         continue;    
       }
       if (number(1, 100) <= 15) {
         mobaction(ch, GET_NAME(vict), "whinny");
       }
       found = TRUE;
     }
   }

   if(GET_RACE(ch) == RACE_NPC_MAMAL) {
    found = FALSE;
  for (vict = world[ch->in_room].people; vict && !found; vict = vict->next_in_room) {

        if (IS_NPC(vict) || !CAN_SEE(ch, vict)) { continue; }
         if(GET_CHA(vict) < 10 && number(1, 10) <= 3) {
           mobaction(ch, GET_NAME(vict), "growl");
         }   
        found = TRUE;
        }
   }

/*  if((GET_CLASS(ch) == CLASS_NPC_HUMANOID) && (MOB_FLAGGED(ch, MOB_SCAVENGER))) {

    list = vict->carrying;
    for (obj = list; obj && !found; obj = obj->next_content) {
      if (CAN_SEE_OBJ(vict, obj)) {
        command_interpreter(ch, "wear all");
        found = 1;
      }
    }
  } */
 
  }				/* end for() */
}



/* Mob Memory Routines */

/* make ch remember victim */
void remember(struct char_data * ch, struct char_data * victim)
{
  memory_rec *tmp;
  bool present = FALSE;

  if (!IS_NPC(ch) || IS_NPC(victim) || (GET_LEVEL(victim) >= LVL_IMMORT))
    return;

  for (tmp = MEMORY(ch); tmp && !present; tmp = tmp->next)
    if (tmp->id == GET_IDNUM(victim))
      present = TRUE;

  if (!present) {
    CREATE(tmp, memory_rec, 1);
    tmp->next = MEMORY(ch);
    tmp->id = GET_IDNUM(victim);
    MEMORY(ch) = tmp;
  }
}


/* make ch forget victim */
void forget(struct char_data * ch, struct char_data * victim)
{
  memory_rec *curr, *prev = NULL;

  if (!(curr = MEMORY(ch)))
    return;

  while (curr && curr->id != GET_IDNUM(victim)) {
    prev = curr;
    curr = curr->next;
  }

  if (!curr)
    return;			/* person wasn't there at all. */

  if (curr == MEMORY(ch))
    MEMORY(ch) = curr->next;
  else
    prev->next = curr->next;

  free(curr);
}


/* erase ch's memory */
void clearMemory(struct char_data * ch)
{
  memory_rec *curr, *next;

  curr = MEMORY(ch);

  while (curr) {
    next = curr->next;
    free(curr);
    curr = next;
  }

  MEMORY(ch) = NULL;
}

/* called only by GreetPeople() */
void Submit(struct char_data *ch, struct char_data *t)
{
  char Gbuf[MAX_STRING_LENGTH];

  switch (number(1, 5)) {
  case 1:
    sprintf(Gbuf, "bow %s", GET_NAME(t));
    command_interpreter(ch, Gbuf);
    break;
  case 2:
    sprintf(Gbuf, "smile %s", GET_NAME(t));
    command_interpreter(ch, Gbuf);
    break;
  case 3:
    if(GET_SEX(ch) != GET_SEX(t)) {
    mobsay(ch, "Hey, you're kinda cute.");
    sprintf(Gbuf, "flirt %s", GET_NAME(t));
    command_interpreter(ch, Gbuf);
    }
    else {
    sprintf(Gbuf, "wink %s", GET_NAME(t));
    command_interpreter(ch, Gbuf);
    }
    break;
  case 4:
    sprintf(Gbuf, "wave %s", GET_NAME(t));
    command_interpreter(ch, Gbuf);
    break;
  default:
    act("$n nods $s head at you", 0, ch, 0, t, TO_VICT);
    act("$n nods $s head at $N", 0, ch, 0, t, TO_NOTVICT);
    break;
  }
}

void PkillokHello(struct char_data *ch, struct char_data *t)
{
  switch (number(1, 10)) {
  case 1:
    act("$n looks at you, then quickly averts $s eyes.", 0, ch, 0, t, TO_VICT);
    act("$n looks at $N, then quickly averts $s eyes, as if afraid.", 
         0, ch, 0, t, TO_NOTVICT);
    break;
  case 2:
    act("$n moves quickly aside to let you pass.", 0, ch, 0, t, TO_VICT);
    act("$n moves quickly aside to let $N pass, giving $M plenty of room.",
         0, ch, 0, t, TO_NOTVICT);
    break;
  case 3:
    act("$n's eyes widen in alarm as $e spies you.", 0, ch, 0, t, TO_VICT);
    act("$n's eyes widen in alarm as $e spies $N.", 0, ch, 0, t, TO_NOTVICT);
    break;
  default:
    return;
  }
}


/* called only by GreetPeople() */
void SayNiceHello(struct char_data *ch, struct char_data *t)
{
  char Gbuf[MAX_STRING_LENGTH];

  switch (number(1, 10)) {
  case 1:
    mobsay(ch, "Greetings, adventurer");
    break;
  case 2:
    if (GET_RANK(t) <= 1) {
      mobsay(ch, "Good day"); 
      
    }
    else {
      sprintf(buf, "Good day, %s %s", rank[(int)GET_RANK(t)][(int)GET_SEX(t)], GET_NAME(t)); 
      mobsay(ch, buf);
    
   }
  break;
  case 3:
    if (t->player.sex == SEX_FEMALE)
      mobsay(ch, "Pleasant journey, mistress");
    else
      mobsay(ch, "Pleasant journey, mister");
    break;
  case 4:
    if (GET_RANK(t) > 2) {
      sprintf(Gbuf, "Make way!  Make way for %s %s!", rank[(int)GET_RANK(t)][(int)GET_SEX(t)], GET_NAME(t));
    mobsay(ch, Gbuf);
    }
    break;
  case 5:
    mobsay(ch, "May the prophets smile upon you.");
    break;
  case 6:
    mobsay(ch, "It is a pleasure to see you again.");
    break;
  case 7:
    mobsay(ch, "You are always welcome here.");
    break;
  case 8:
    mobsay(ch, "My lord bids you greetings.");
    break;
  case 9:
    if (time_info.hours > 6 && time_info.hours < 12)
      sprintf(Gbuf, "Good morning, %s", GET_NAME(t));
    else if (time_info.hours >= 12 && time_info.hours < 20)
      sprintf(Gbuf, "Good afternoon, %s", GET_NAME(t));
    else if (time_info.hours >= 20 && time_info.hours <= 24)
      sprintf(Gbuf, "Good evening, %s", GET_NAME(t));
    else
      sprintf(Gbuf, "Up for a midnight stroll, %s?\n", GET_NAME(t));
    mobsay(ch, Gbuf);
    break;
  default:
    return;
  }
}

/* called only by GreetPeople() */
void SayBlehHello(struct char_data *ch, struct char_data *t)
{

  switch (number(1, 10)) {
  case 1:
  act("$n looks for a second, then looks away quickly.", TRUE, ch, 0, 0, TO_ROOM);
    break;
  case 2:
    mobsay(ch, "Hello.");
    break;
  case 3:
    act("$n purposefully averts his gaze.", TRUE, ch, 0, 0, TO_ROOM);
    break;
  case 4:
    act("$n whistles softly to himself.", TRUE, ch, 0, 0, TO_ROOM);
    break;
  case 5:
    act("$n smiles warmly at you.", TRUE, ch, 0, 0, TO_ROOM);
    break;
  case 6:
    mobsay(ch, "Good day to you.");
    break;
  case 7:
    act("$n looks around for something or someone.", TRUE, ch, 0, 0, TO_ROOM);
    break;
  default:
    break;
  }
}

void GreetPeople(struct char_data *ch)
{
  struct char_data *tch, *tch_next;

    for (tch = world[ch->in_room].people; tch; tch = tch_next) {
      tch_next = tch->next_in_room;
      if (!IS_NPC(tch)) {
	if (tch && GET_LEVEL(tch) > GET_LEVEL(ch) && number(0, 3)) {
	  if (CAN_SEE(ch, tch)) {
            if (GET_CHA(tch) > 10) {
	       Submit(ch, tch);
            }
            if (PLR_FLAGGED(tch, PLR_PKILL_OK)) {
               PkillokHello(ch, tch);
            }
	    SayNiceHello(ch, tch);
	  }
	  return;
	} else {
	  SayBlehHello(ch, tch);
	}
      }
    }
}
