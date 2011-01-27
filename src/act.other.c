/* ********************************************************************** * 
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __ACT_OTHER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "house.h"
#include "quest.h"
#include "dg_scripts.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct zone_data *zone_table;
//extern struct spell_info_type spell_info[];
extern struct index_data *mob_index;
extern char *class_abbrevs[];
extern const int rev_dir[];
extern char *dirs[];
extern int pkill_ok(struct char_data *ch, struct char_data *vict);
extern int find_door(struct char_data *ch, char *type, 
                     char *dir, char *cmdname);
extern struct char_data *ch_selling;
extern struct char_data *ch_buying;
extern int pk_allowed;
extern struct char_data *character_list;

/* extern procedures */
SPECIAL(shop_keeper);
void add_follower(struct char_data * ch, struct char_data * leader);
void save_aliases(struct char_data *ch);
void list_abilities(struct char_data * ch);
void die(struct char_data * ch, struct char_data * killer);
void Crash_rentsave(struct char_data * ch, int cost);
void stop_auction(int type, struct char_data * ch);
void perform_put(struct char_data * ch, struct obj_data * obj, 
                 struct obj_data * cont);
void set_descrip(struct char_data * ch, char *descrip);
ACMD(do_flee);
void save_corpses(void);
 
ACMD(do_quit)
{
  extern int free_rent;
  int save_room;
  struct descriptor_data *d, *next_d;

  if (IS_NPC(ch) || !ch->desc)
    return;

  if (subcmd != SCMD_QUIT && GET_LEVEL(ch) < LVL_IMMORT)
    send_to_char("You will lose all your stuff if you quit.  To save "
                 "your gear, you must rent at an inn.  If you are sure "
                 "you want to quit, type /cWquit!/c0.", ch);
    // send_to_char("You have to type quit--no less, to quit!\r\n", ch);
  else if (GET_POS(ch) == POS_FIGHTING)
    send_to_char("No way!  You're fighting for your life!\r\n", ch);
  else if (GET_POS(ch) < POS_STUNNED) {
    send_to_char("You die before your time...\r\n", ch);
    die(ch, NULL);
  } else {

   if (GET_LEVEL(ch) < LVL_IMMORT) {
      char_from_room(ch);
      char_to_room(ch, real_room(1));
   }
    if (!GET_INVIS_LEV(ch))
      act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);

    sprintf(buf, "%s has quit the game.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Goodbye, friend.. Come back soon!\r\n", ch);
/*
	if (ch == ch_selling)
		stop_auction(AUC_QUIT_CANCEL, NULL);
*/


    /*
     * kill off all sockets connected to the same player as the one who is
     * trying to quit.  Helps to maintain sanity as well as prevent duping.
     */
    save_corpses();
    
    for (d = descriptor_list; d; d = next_d) {
      next_d = d->next;
      if (d == ch->desc)
        continue;
      if (d->character && (GET_IDNUM(d->character) == GET_IDNUM(ch)))
        close_socket(d);
    }

    save_aliases(ch);

   save_room = ch->in_room;
   if (free_rent)
      Crash_rentsave(ch, 0);
    extract_char(ch);		/* Char is saved in extract char */

    /* If someone is quitting in their house, let them load back here */
    if (ROOM_FLAGGED(save_room, ROOM_HOUSE))
      save_char(ch, save_room);
  }
}



void save_aliases(struct char_data *ch);
ACMD(do_save)
{
  if (IS_NPC(ch) || !ch->desc)
    return;

  /* Only tell the char we're saving if they actually typed "save" */
  if (cmd) {
    sprintf(buf, "Saving %s.\r\n", GET_NAME(ch));
    send_to_char(buf, ch);
  }

  save_char(ch, NOWHERE);
  Crash_crashsave(ch);
  if(ROOM_FLAGGED(ch->in_room, ROOM_VAULT)) {
    vault_crashsave(world[ch->in_room].number);
    send_to_char("Saving Vault\r\n", ch);
  
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
    House_crashsave(world[ch->in_room].number);
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
  send_to_char("Sorry, but you cannot do that.\r\n", ch);
}



ACMD(do_sneak)
{
  struct affected_type af;
  byte percent;

  send_to_char("Okay, you'll try to move silently for a while.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK, ABT_SKILL);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SNEAK) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}



ACMD(do_hide)
{
  byte percent;

  send_to_char("You attempt to hide yourself.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HIDE) + dex_app_skill[GET_DEX(ch)].hide)
    return;

  SET_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
}




ACMD(do_steal)
{
  struct char_data *vict;
  struct obj_data *obj;
  char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];
  int percent, gold, eq_pos, pcsteal = 0, ohoh = 0;
//  extern int pt_allowed;


  ACMD(do_gen_comm);

  argument = one_argument(argument, obj_name);
  one_argument(argument, vict_name);


  if(ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("You have to much of a peaceful feeling to do that.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, vict_name))) {
    send_to_char("Steal what from who?\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("Come on now, that's rather stupid!\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to steal from that player.\r\n", ch);
    return;
  } 

  /* 101% is a complete failure */
  percent = number(1, 125) - GET_DEX(ch);

  if (GET_POS(vict) < POS_SLEEPING)
    percent = -1;		/* ALWAYS SUCCESS */

  if(!IS_NPC(vict) && !pkill_ok(ch, vict)){
    send_to_char("You can't steal from players yet!\r\n", ch);
    return;
  }
  /* NO NO With Imp's and Shopkeepers, and if player thieving is not allowed */
  if (GET_LEVEL(vict) >= LVL_IMMORT || pcsteal ||
      GET_MOB_SPEC(vict) == shop_keeper)
    percent = 101;		/* Failure */

  if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {

    if (!(obj = get_obj_in_list_vis(vict, obj_name, vict->carrying))) {

      for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
	if (GET_EQ(vict, eq_pos) &&
	    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
	    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
	  obj = GET_EQ(vict, eq_pos);
	  break;
	}
      if (!obj) {
	act("$E hasn't got that item.", FALSE, ch, 0, vict, TO_CHAR); return;
      } else {			/* It is equipment */
	if ((GET_POS(vict) > POS_STUNNED)) {
	  send_to_char("Steal the equipment now?  Impossible!\r\n", ch);
	  return;
	} else {
if(IS_OBJ_STAT(GET_EQ(vict, eq_pos), ITEM_NOTRANSFER)) {
  act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_ROOM);
  act("$n tries to steal something from you.", TRUE, ch, 0, vict, TO_VICT);
  send_to_char("You fail.\r\n", ch);
  return;
}
	  act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
	  obj_to_char(unequip_char(vict, eq_pos), ch);
	}
      }
    } else {			/* obj found in inventory */

      percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */

      if ((IS_OBJ_STAT(obj, ITEM_NOTRANSFER)) || (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL)))) {
	ohoh = TRUE;
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
        improve_abil(ch, SKILL_STEAL, ABT_SKILL);
      } else {			/* Steal the item */
	if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
	  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
	    obj_from_char(obj);
	    obj_to_char(obj, ch);
	    send_to_char("Got it!\r\n", ch);
	  }
	} else
	  send_to_char("You cannot carry that much.\r\n", ch);
      }
    }
  } else {			/* Steal some coins */
    if (AWAKE(vict) && (percent > GET_SKILL(ch, SKILL_STEAL))) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
      act("You discover that $n has $s hands in your wallet.", FALSE, ch, 0, vict, TO_VICT);
      act("$n tries to steal gold from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
        improve_abil(ch, SKILL_STEAL, ABT_SKILL);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
      gold = MIN(1782, gold);
      if (gold > 0) {
	GET_GOLD(ch) += gold;
	GET_GOLD(vict) -= gold;
        if (gold > 1) {
	  sprintf(buf, "Bingo!  You got %d gold coins.\r\n", gold);
	  send_to_char(buf, ch);
	} else {
	  send_to_char("You manage to swipe a solitary gold coin.\r\n", ch);
	}
      } else {
	send_to_char("You couldn't get any gold...\r\n", ch);
      }
    }
  }

  if (ohoh && IS_NPC(vict) && AWAKE(vict))
    hit(vict, ch, TYPE_UNDEFINED);
}



ACMD(do_practice)
{
  one_argument(argument, arg);

  if (*arg)
    send_to_char("You can only practice skills in your guild.\r\n", ch);
  else
    list_abilities(ch);
}



ACMD(do_visible)
{
  void appear(struct char_data * ch);
  void perform_immort_vis(struct char_data *ch);

  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    perform_immort_vis(ch);
    return;
  }

  if(IS_AFFECTED(ch, AFF_SNEAK)) {
    appear(ch);
    send_to_char("You stop sneaking.\r\n", ch);
  }
     
  else if IS_AFFECTED(ch, AFF_INVISIBLE) {
    appear(ch);
    send_to_char("You break the spell of invisibility.\r\n", ch);
  } else
    send_to_char("You are already visible.\r\n", ch);
}



ACMD(do_title)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your title is fine... go away.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_NOTITLE))
    send_to_char("You can't title yourself -- you shouldn't have abused it!\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Titles can't contain the ( or ) characters.\r\n", ch);
//  else if (strlen(argument) > MAX_TITLE_LENGTH) {
  else if (strlen(argument) > 25) {
//    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
//	    MAX_TITLE_LENGTH);
    send_to_char("Sorry, titles can't be longer than 25 characters.\r\n", ch);
  } else {
    set_title(ch, argument);
    sprintf(buf, "Okay, you're now %s %s.\r\n", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
  }
}


int perform_group(struct char_data *ch, struct char_data *vict)
{
	struct follow_type *f = NULL;
	bool group = TRUE;

	if (AFF_FLAGGED(vict, AFF_GROUP))
		return (1);

	if (IS_IMMORT(ch))
	{
		if (IS_IMMORT(vict) || (IS_REMORT(vict) && GET_LEVEL(vict) >= 75) || (GET_LEVEL(vict) >= 100))
			group = TRUE;
		else
			group = FALSE;
	}
	else if (IS_REMORT(ch))
	{
		if (IS_IMMORT(vict) && GET_LEVEL(ch) >= 75)
		{
			for (f = ch->followers; f; f = f->next)
			{
				if (group == FALSE)
					continue;
				else {
					if (ch != f->follower && (!IS_IMMORT(f->follower)) && (!IS_REMORT(f->follower)) && (GET_LEVEL(f->follower) < 100))
						group = FALSE;
					else
						group = TRUE;
				}
			}
		}
		else if (IS_REMORT(ch))
			group = TRUE;
		else if (GET_LEVEL(vict) >= (GET_LEVEL(ch)-50))
			group = TRUE;
		else
			group = FALSE;
	} else if ((GET_LEVEL(vict) >= (GET_LEVEL(ch)-50) && GET_LEVEL(vict) <= (GET_LEVEL(ch)+50) && !IS_IMMORT(vict)) || (IS_REMORT(vict) && GET_LEVEL(ch) >= (GET_LEVEL(vict)-50)))
		group = TRUE;
	else {
		group = FALSE;
	}

	if (group == TRUE)
	{
		SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
		if (ch != vict)
			act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
		act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
		act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
		return (1);
	} else {
		act("$N can not join your group.", FALSE, ch, 0, vict, TO_CHAR);
		act("You can not join $n's group.", FALSE, ch, 0, vict, TO_VICT);
		act("$N can not join $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
		return (1);
	}
}
/*
  int lo_lev, hi_lev;
  struct follow_type *f;

  if (IS_AFFECTED(vict, AFF_GROUP) || !CAN_SEE(ch, vict))
    return 0;

  lo_lev = hi_lev = GET_LEVEL(ch);
  for (f = ch->followers; f; f = f->next)
    if (ch != f->follower && AFF_FLAGGED(vict, AFF_GROUP)) {
      hi_lev=MAX(hi_lev, GET_LEVEL(f->follower));
      lo_lev=MIN(hi_lev, GET_LEVEL(f->follower));
    }
  if (!IS_NPC(vict)) {
    if (GET_LEVEL(vict) > lo_lev+50 && GET_CLASS(ch) < CLASS_VAMPIRE) {
      act("$N's level is too high to join your group.",
          FALSE, ch, 0, vict, TO_CHAR);
      act("Your level is too high to join $n's group.",
          FALSE, ch, 0, vict, TO_VICT);
      return 1;
    }

    if (GET_LEVEL(vict) < hi_lev-50 && GET_CLASS(vict) < CLASS_VAMPIRE) {
      act("$N's level is too low to join your group.",
          FALSE, ch, 0, vict, TO_CHAR);
      act("Your level is too low to join $n's group.",
          FALSE, ch, 0, vict, TO_VICT);
      return 1;
    }
	if ((GET_CLASS(vict) >= CLASS_VAMPIRE) && (hi_lev < 100) && (!GET_LEVEL(ch) >= CLASS_VAMPIRE)) {
      act("$N's is too powerfull to join your group.",
          FALSE, ch, 0, vict, TO_CHAR);
      act("You are too powerfull to join $n's group.",
          FALSE, ch, 0, vict, TO_VICT);
      return 1;
    }
  
    if (GET_LEVEL(vict) < 100  && !IS_REMORT(vict) && GET_CLASS(ch) >= CLASS_VAMPIRE) {
      act("Only mortals of level 100 or above my group with you.\r\n$N's level is too low to join your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("Only mortals of level 100 or above may group with immorts.\r\nYour level is too low to join $n's group.", FALSE, ch, 0, vict, TO_VICT);
      return 1;
    }
  }
  SET_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
  if (ch != vict)
    act("$N is now a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
  act("You are now a member of $n's group.", FALSE, ch, 0, vict, TO_VICT);
  act("$N is now a member of $n's group.", FALSE, ch, 0, vict, TO_NOTVICT);
  return 1;
}
*/

void print_group(struct char_data *ch)
{
  struct char_data *k;
  struct follow_type *f;
  int perc_hit, perc_mana, perc_move;
  

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    
    sprintf(buf, "%sBut you are not the member of a group!%s\r\n", buf, CCNRM(ch, C_NRM));
   
     send_to_char(buf, ch);
  } 
  else {
    sprintf(buf, "%sYour group consists of:\r\n%s",
            buf, CCNRM(ch, C_NRM));
    send_to_char(buf, ch); 

    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP)) {

      *buf = '\0';

      perc_hit = MIN(100, 100*GET_HIT(k)/ MAX(1, GET_MAX_HIT(k)));
      perc_mana = MIN(100, 100*GET_MANA(k)/ MAX(1, GET_MAX_MANA(k)));
      perc_move = MIN(100, 100*GET_MOVE(k)/MAX(1, GET_MAX_MOVE(k)));
      

      if ((perc_hit < 25) || (perc_mana < 25) || (perc_move < 25))
        strcpy(buf, CCRED(ch, C_NRM));
      else if ((perc_hit > 90) && (perc_mana > 90) && (perc_move > 90))
        strcpy(buf, CCGRN(ch, C_NRM));
      else
        strcpy(buf, CCYEL(ch, C_NRM));

      *buf2 = '\0';
      if (IS_AFFECTED(k, AFF_SANCTUARY))
        sprintf(buf2, "%s%s*%s", buf2, CCWHT(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(k, AFF_HASTE))
        sprintf(buf2, "%s%s*%s", buf2, CCRED(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(k, AFF_BREATHE))
        sprintf(buf2, "%s%s*%s", buf2, CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(k, AFF_FLY))
        sprintf(buf2, "%s%s*%s", buf2, CCYEL(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
     
    switch(GET_CLASS(k)){
     case CLASS_DRUID:
      sprintf(buf, "%s     [%4dHP %4dVM %4dMV] %s[%3d %3s] %s (Head of group) %s[%s]%s\n\r",
       buf,  GET_HIT(k), GET_VIM(k), GET_MOVE(k),  CCNRM(ch, C_NRM),
        GET_LEVEL(k), CLASS_ABBR(k), GET_NAME(k), CCNRM(ch, C_NRM), buf2,
        CCNRM(ch, C_NRM));
      break; 
     case CLASS_MONK:
      sprintf(buf, "%s     [%4dHP %4dQI %4dMV] %s[%3d %3s] %s (Head of group) %s[%s]%s\n\r",
       buf,  GET_HIT(k), GET_QI(k), GET_MOVE(k),  CCNRM(ch, C_NRM),
        GET_LEVEL(k), CLASS_ABBR(k), GET_NAME(k), CCNRM(ch, C_NRM), buf2,
        CCNRM(ch, C_NRM));
      break; 
     default:
      sprintf(buf, "%s     [%4dHP %4dMP %4dMV] %s[%3d %3s] %s (Head of group) %s[%s]%s\n\r",
       buf,  GET_HIT(k), GET_MANA(k), GET_MOVE(k),  CCNRM(ch, C_NRM),
        GET_LEVEL(k), CLASS_ABBR(k), GET_NAME(k), CCNRM(ch, C_NRM), buf2,
        CCNRM(ch, C_NRM));
      break; 
    }
      if (CAN_SEE(ch, k))
        send_to_char(buf, ch);
    }

    for (f = k->followers; f; f = f->next) {
      if (!IS_AFFECTED(f->follower, AFF_GROUP))
	continue;

      *buf = '\0';

      perc_hit = MIN(100, 100*GET_HIT(f->follower)/ MAX(1, GET_MAX_HIT(f->follower)));
      perc_mana = MIN(100, 100*GET_MANA(f->follower)/ MAX(1, GET_MAX_MANA(f->follower)));
      perc_move = MIN(100, 100*GET_MOVE(f->follower)/MAX(1, GET_MAX_MOVE(f->follower)));

      if ((perc_hit < 25) || (perc_mana < 25) || (perc_move < 25))
        strcpy(buf, CCRED(ch, C_NRM));
      else if ((perc_hit > 90) && (perc_mana > 90) && (perc_move > 90))
        strcpy(buf, CCGRN(ch, C_NRM));
      else
        strcpy(buf, CCYEL(ch, C_NRM));
      
      *buf2 = '\0';
      if (IS_AFFECTED(f->follower, AFF_SANCTUARY))
        sprintf(buf2, "%s%s*%s", buf2, CCWHT(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(f->follower, AFF_HASTE))
        sprintf(buf2, "%s%s*%s", buf2, CCRED(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(f->follower, AFF_BREATHE))
        sprintf(buf2, "%s%s*%s", buf2, CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      if (IS_AFFECTED(f->follower, AFF_FLY))
        sprintf(buf2, "%s%s*%s", buf2, CCYEL(ch, C_NRM), CCNRM(ch, C_NRM));
      else
        sprintf(buf2, "%s ", buf2);
      
      switch(GET_CLASS(f->follower)){
       case CLASS_DRUID:
         sprintf(buf, "%s     [%4dHP %4dVM %4dMV] %s[%3d %3s] %s %s[%s]%s\n\r",
           buf, GET_HIT(f->follower), GET_VIM(f->follower), GET_MOVE(f->follower),
           CCNRM(ch, C_NRM), GET_LEVEL(f->follower), CLASS_ABBR(f->follower),
           GET_NAME(f->follower), CCNRM(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
         break;
       case CLASS_MONK:
         sprintf(buf, "%s     [%4dHP %4dQI %4dMV] %s[%3d %3s] %s %s[%s]%s\n\r",
           buf, GET_HIT(f->follower), GET_QI(f->follower), GET_MOVE(f->follower),
           CCNRM(ch, C_NRM), GET_LEVEL(f->follower), CLASS_ABBR(f->follower),
           GET_NAME(f->follower), CCNRM(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
         break;
       default:
         sprintf(buf, "%s     [%4dHP %4dMP %4dMV] %s[%3d %3s] %s %s[%s]%s\n\r",
           buf, GET_HIT(f->follower), GET_MANA(f->follower), GET_MOVE(f->follower),
           CCNRM(ch, C_NRM), GET_LEVEL(f->follower), CLASS_ABBR(f->follower),
           GET_NAME(f->follower), CCNRM(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
         break;
      }
      if (CAN_SEE(ch, f->follower))
        send_to_char(buf, ch);
    }

    *buf = '\0';
    sprintf(buf, "%s\r\n%s",
            CCBLU(ch, C_NRM), CCNRM(ch, C_NRM));
    send_to_char(buf, ch); 

  }
}



ACMD(do_group)
{
  struct char_data *vict;
  struct follow_type *f;
  int found;

  one_argument(argument, buf);

  if (!*buf) {
    print_group(ch);
    return;
  }

  if (ch->master) {
    act("You can not enroll group members without being head of a group.",
	FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if (!str_cmp(buf, "all")) {
    perform_group(ch, ch);
    for (found = 0, f = ch->followers; f; f = f->next)
      found += perform_group(ch, f->follower);
    if (!found)
      send_to_char("Everyone following you is already in your group.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if ((vict->master != ch) && (vict != ch))
    act("$N must follow you to enter your group.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!IS_AFFECTED(vict, AFF_GROUP))
      perform_group(ch, vict);
    else {
      if (ch != vict)
	act("$N is no longer a member of your group.", FALSE, ch, 0, vict, TO_CHAR);
      act("You have been kicked out of $n's group!", FALSE, ch, 0, vict, TO_VICT);
      act("$N has been kicked out of $n's group!", FALSE, ch, 0, vict, TO_NOTVICT);
      REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_GROUP);
    }
  }
}



ACMD(do_ungroup)
{
  struct follow_type *f, *next_fol;
  struct char_data *tch;
  void stop_follower(struct char_data * ch);

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->master || !(IS_AFFECTED(ch, AFF_GROUP))) {
      send_to_char("But you lead no group!\r\n", ch);
      return;
    }
    sprintf(buf2, "%s has disbanded the group.\r\n", GET_NAME(ch));
    for (f = ch->followers; f; f = next_fol) {
      next_fol = f->next;
      if (IS_AFFECTED(f->follower, AFF_GROUP)) {
	REMOVE_BIT_AR(AFF_FLAGS(f->follower), AFF_GROUP);
	send_to_char(buf2, f->follower);
        if (!IS_AFFECTED(f->follower, AFF_CHARM))
	  stop_follower(f->follower);
      }
    }

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
    send_to_char("You disband the group.\r\n", ch);
    return;
  }
  if (!(tch = get_char_room_vis(ch, buf))) {
    send_to_char("There is no such person!\r\n", ch);
    return;
  }
  if (tch->master != ch) {
    send_to_char("That person is not following you!\r\n", ch);
    return;
  }

  if (!IS_AFFECTED(tch, AFF_GROUP)) {
    send_to_char("That person isn't in your group.\r\n", ch);
    return;
  }

  REMOVE_BIT_AR(AFF_FLAGS(tch), AFF_GROUP);

  act("$N is no longer a member of your group.", FALSE, ch, 0, tch, TO_CHAR);
  act("You have been kicked out of $n's group!", FALSE, ch, 0, tch, TO_VICT);
  act("$N has been kicked out of $n's group!", FALSE, ch, 0, tch, TO_NOTVICT);
 
  if (!IS_AFFECTED(tch, AFF_CHARM))
    stop_follower(tch);
}

const char *hit_rept_msgs[] = {
  "I think I'm going to pass out... ", /* 0 */
  "I have many grevious wounds! ", /* 10% */
  "I seem to be bleeding all over the place, ", /* 20% */
  "I've lost track of all the places I've been hit, ", /* 30% */
  "Some of these wounds look pretty bad, ", /* 40% */
  "I could use a healer, ", /* 50% */
  "Am I supposed to be bleeding this much? ", /* 60% */
  "My body aches all over, ", /* 70% */
  "I seem to have a few scratches, ", /* 80% */
  "I am feeling quite well, ", /* 90% */
  "I am in excellent health, " /* 100% */
};

const char *mana_rept_msgs[] = {
  "I have no mystical energy to speak of, ", /* 0 */
  "my mystical reserves are almost depleted, ", /* 10% */
  "my mystical energies feel extremely weak, ", /* 20% */
  "I need to channel my reserves, ", /* 30% */
  "I have less mystical energy left than I thought, ", /* 40% */
  "I could use a chance to restore my reserves, ", /* 50% */
  "I'm fine so quit asking, ", /* 60% */
  "I'm feeling the strain on my powers a bit, ", /* 70% */
  "I have a good deal of reserve left, ", /* 80% */
  "I have a quite a lot of reserve left, ", /* 90% */
  "my reserves are full, " /* 100% */
};

const char *move_rept_msgs[] = {
  "and I have almost no energy left.'\r\n", /* 0 */
  "I really could use a rest.'\r\n", /* 10% */
  "I could stumble a short way.'\r\n", /* 20% */
  "I think I could hike a short distance.'\r\n", /* 30% */
  "I'm feeling quite winded.'\r\n", /* 40% */
  "I could walk for a while a way.'\r\n", /* 50% */
  "My feet are a bit to weary.'\r\n", /* 60% */
  "I could walk for quite a while.'\r\n", /* 70% */
  "I am good to go.'\r\n", /* 80% */
  "I have a lot of energy.'\r\n", /* 90% */
  "I could walk forever.'\r\n" /* 100% */
};


/* This is the altered do_report command. Insert the lines marked with the + */
ACMD(do_report)
{
  struct char_data *k;
  struct follow_type *f;
  int percent;

  if (!AFF_FLAGGED(ch, AFF_GROUP)) {
    send_to_char("But you are not a member of any group!\r\n", ch);
    return;
  }

  sprintf(buf, "%s reports, '", GET_NAME(ch));

  /* Health */
  if (GET_HIT(ch) <= 0)
     sprintf(buf + strlen(buf), "I should already be dead! ");
  else if (GET_HIT(ch) < 3)
     sprintf(buf + strlen(buf), "I am barely holding on to life, ");
  else if (GET_HIT(ch) >= GET_MAX_HIT(ch))
     sprintf(buf + strlen(buf), "I am in perfect health, ");
  else {
     percent = MIN(10, ((float)((float)GET_HIT(ch) / (float)GET_MAX_HIT(ch)) * 10));
     sprintf(buf + strlen(buf), "%s", hit_rept_msgs[percent]);
  }

 switch(GET_CLASS(ch)) {

   case CLASS_DRUID:
  /* Mana */
  if (GET_VIM(ch) <= 0)
     sprintf(buf + strlen(buf), "my vim has been fully depleted, ");
  else if (GET_VIM(ch) < 3)
     sprintf(buf + strlen(buf), "I have a teensy bit of vim left, ");
  else if (GET_VIM(ch) >= GET_MAX_VIM(ch))
     sprintf(buf + strlen(buf), "I have plenty of magical energy, ");
  else {
     percent = MIN(10, ((float)((float)GET_VIM(ch) / (float)GET_MAX_VIM(ch)) *
 10));
     sprintf(buf + strlen(buf), "%s", mana_rept_msgs[percent]);
  }
  break;
  case CLASS_MONK:
  /* Mana */
  if (GET_QI(ch) <= 0)
     sprintf(buf + strlen(buf), "my qi has been fully depleted, ");
  else if (GET_QI(ch) < 3)
     sprintf(buf + strlen(buf), "I have a teensy bit of qi left, ");
  else if (GET_QI(ch) >= GET_MAX_QI(ch))
     sprintf(buf + strlen(buf), "I have plenty of qi energy, ");
  else {
     percent = MIN(10, ((float)((float)GET_QI(ch) / (float)GET_MAX_QI(ch)) *
 10));
     sprintf(buf + strlen(buf), "%s", mana_rept_msgs[percent]);
  }
  break;
  default:
  /* Mana */
  if (GET_MANA(ch) <= 0)
     sprintf(buf + strlen(buf), "my mana has been fully depleted, ");
  else if (GET_MANA(ch) < 3)
     sprintf(buf + strlen(buf), "I have a teensy bit of mana left, ");
  else if (GET_MANA(ch) >= GET_MAX_MANA(ch))
     sprintf(buf + strlen(buf), "I have plenty of magical energy, ");
  else {
     percent = MIN(10, ((float)((float)GET_MANA(ch) / (float)GET_MAX_MANA(ch)) *
 10));
     sprintf(buf + strlen(buf), "%s", mana_rept_msgs[percent]);
  }
  break;

}

  /* Move */
  if (GET_MOVE(ch) <= 0)
     sprintf(buf + strlen(buf), "and I couldn't walk anywhere if my life depended on it.'\r\n");
  else if (GET_MOVE(ch) < 3)
     sprintf(buf + strlen(buf), "and I think I could stumble another step.'\r\n");
  else if (GET_MOVE(ch) >= GET_MAX_MOVE(ch))
     sprintf(buf + strlen(buf), "and I am fully rested.'\r\n");
  else {
     percent = MIN(10, ((float)((float)GET_MOVE(ch) / (float)GET_MAX_MOVE(ch)) * 10));
     sprintf(buf + strlen(buf), "%s", move_rept_msgs[percent]);
  }

  CAP(buf);

  k = (ch->master ? ch->master : ch);

  for (f = k->followers; f; f = f->next)
    if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower != ch)
      send_to_char(buf, f->follower);
  if (k != ch)
    send_to_char(buf, k);
  send_to_char("You report to the group.\r\n", ch);
}


ACMD(do_split)
{
  int amount, num, share;
  struct char_data *k;
  struct follow_type *f;

  if (IS_NPC(ch))
    return;

  one_argument(argument, buf);

  if (is_number(buf)) {
    amount = atoi(buf);
    if (amount <= 0) {
      send_to_char("Sorry, you can't do that.\r\n", ch);
      return;
    }
    if (amount > GET_GOLD(ch)) {
      send_to_char("You don't seem to have that much gold to split.\r\n", ch);
      return;
    }
    k = (ch->master ? ch->master : ch);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
      num = 1;
    else
      num = 0;

    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room))
	num++;

    if (num && IS_AFFECTED(ch, AFF_GROUP))
      share = amount / num;
    else {
      send_to_char("With whom do you wish to share your gold?\r\n", ch);
      return;
    }

    GET_GOLD(ch) -= share * (num - 1);

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)
	&& !(IS_NPC(k)) && k != ch) {
      GET_GOLD(k) += share;
      sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
	      amount, share);
      send_to_char(buf, k);
    }
    for (f = k->followers; f; f = f->next) {
      if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	  (!IS_NPC(f->follower)) &&
	  (f->follower->in_room == ch->in_room) &&
	  f->follower != ch) {
	GET_GOLD(f->follower) += share;
	sprintf(buf, "%s splits %d coins; you receive %d.\r\n", GET_NAME(ch),
		amount, share);
	send_to_char(buf, f->follower);
      }
    }
    sprintf(buf, "You split %d coins among %d members -- %d coins each.\r\n",
	    amount, num, share);
    send_to_char(buf, ch);
  } else {
    send_to_char("How many coins do you wish to split with your group?\r\n", ch);
    return;
  }
}



ACMD(do_use)
{
  struct obj_data *mag_item;
  int equipped = 1;

  half_chop(argument, arg, buf);
  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;
  }
  mag_item = GET_EQ(ch, WEAR_HOLD);

  if (!mag_item || !isname(arg, mag_item->name)) {
    switch (subcmd) {
    case SCMD_RECITE:
    case SCMD_QUAFF:
      equipped = 0;
      if (!(mag_item = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf2, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf2, ch);
	return;
      }
      break;
    case SCMD_USE:
      sprintf(buf2, "You don't seem to be holding %s %s.\r\n", AN(arg), arg);
      send_to_char(buf2, ch);
      return;
      break;
    default:
      log("SYSERR: Unknown subcmd passed to do_use");
      return;
      break;
    }
  }
  switch (subcmd) {
  case SCMD_QUAFF:
    if (GET_CLASS(ch) == CLASS_VAMPIRE && 
        mag_item->item_number == real_object(81)) {
   send_to_char("You quaff the blood, renewing your body with health.\r\n", ch);
      act("$n quaffs a vial of blood.", TRUE, ch, 0, 0, TO_ROOM);
      extract_obj(mag_item);
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch), number(250, 500) + GET_HIT(ch));
      return;
    }
    if (GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
      send_to_char("You can only quaff potions.", ch);
      return;
    }
    break;
  case SCMD_RECITE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) && (GET_OBJ_TYPE(mag_item) != ITEM_RUNE)) {
      send_to_char("You can only recite scrolls and runes.\r\n", ch);
      return;
    }
    if(GET_OBJ_TYPE(mag_item) == ITEM_RUNE && GET_OBJ_VAL(mag_item, 1) == 0) {
	send_to_char("This rune has yet to be inscribed.\r\n", ch);
	return;
    }
    break;
  case SCMD_USE:
    if ((GET_OBJ_TYPE(mag_item) != ITEM_WAND) &&
	(GET_OBJ_TYPE(mag_item) != ITEM_STAFF)) {
      send_to_char("You can't seem to figure out how to use it.\r\n", ch);
      return;
    }
    break;
  }

  mag_objectmagic(ch, mag_item, buf, ABT_SPELL);
}



ACMD(do_wimpy)
{
  int wimp_lev;

  one_argument(argument, arg);

  if (GET_CLASS(ch) == CLASS_VAMPIRE) {
    send_to_char("Flee from a fight? A vampire? Never.\r\n", ch);
    return;

  } else

  if (!*arg) {
    if (GET_WIMP_LEV(ch)) {
      sprintf(buf, "Your current wimp level is %d hit points.\r\n",
	      GET_WIMP_LEV(ch));
      send_to_char(buf, ch);
      return;
    } else {
      send_to_char("At the moment, you're not a wimp.  (sure, sure...)\r\n", ch);
      return;
    }
  }
  if (isdigit(*arg)) {
    if ((wimp_lev = atoi(arg))) {
      if (wimp_lev < 0)
	send_to_char("Heh, heh, heh.. we are jolly funny today, eh?\r\n", ch);
      else if (wimp_lev > GET_MAX_HIT(ch))
	send_to_char("That doesn't make much sense, now does it?\r\n", ch);
      else if (wimp_lev > (GET_MAX_HIT(ch) >> 1))
	send_to_char("You can't set your wimp level above half your hit points.\r\n", ch);
      else {
	sprintf(buf, "Okay, you'll wimp out if you drop below %d hit points.\r\n",
		wimp_lev);
	send_to_char(buf, ch);
	GET_WIMP_LEV(ch) = wimp_lev;
      }
    } else {
      send_to_char("Okay, you'll now tough out fights to the bitter end.\r\n", ch);
      GET_WIMP_LEV(ch) = 0;
    }
  } else
    send_to_char("Specify at how many hit points you want to wimp out at.  (0 to disable)\r\n", ch);

  return;

}


ACMD(do_display)
{
  size_t i;

  if (IS_NPC(ch)) {
    send_to_char("Mosters don't need displays.  Go away.\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Usage: prompt { { H | M | V | Q | I | A | G | P | B | T | X | W } | all | none }\r\n", ch);
    return;
  }
  if ((!str_cmp(argument, "on")) || (!str_cmp(argument, "all"))) {
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPQI);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPVIM);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPARIA);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPGOLD);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPPCT);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOBP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPXTOL);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPSHORT);
  } else {
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPGOLD);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPQI);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPVIM);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPARIA);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPPCT);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOBP);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPXTOL);
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_DISPSHORT);

    if (!str_cmp(argument, "none")) {
      send_to_char(OK, ch);
      return;
    }
    for (i = 0; i < strlen(argument); i++) {
      switch (LOWER(argument[i])) {
      case 'h':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
	break;
      case 'm':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
	break;
      case 'v':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
	break;
      case 'q':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPQI);
      case 'c':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPARIA);
	break;
      case 'i':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPVIM);
	break;
      case 'a':
        SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPARIA);
	break;
      case 'g':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPGOLD);
	break;
      case 'p':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPPCT);
	break;
      case 'b':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOBP);
	break;
      case 't':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
	break;
      case 'x':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPXTOL);
	break;
      case 'w':
	SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPSHORT);
	break;
      default:
    send_to_char("Usage: prompt { { H | M | V | Q | I | A | G | P | B | T | X | W } | all | none }\r\n", ch);
	return;
	break;
      }
    }
  }

  send_to_char(OK, ch);
}



ACMD(do_gen_write)
{
  FILE *fl;
  char *tmp, *filename, buf[MAX_STRING_LENGTH];
  struct stat fbuf;
  extern int max_filesize;
  time_t ct;

  switch (subcmd) {
  case SCMD_BUG:
    filename = BUG_FILE;
    break;
  case SCMD_TYPO:
    filename = TYPO_FILE;
    break;
  case SCMD_IDEA:
    filename = IDEA_FILE;
    break;
  default:
    return;
  }

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (IS_NPC(ch)) {
    send_to_char("Monsters can't have ideas - Go away.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("That must be a mistake...\r\n", ch);
    return;
  }
  sprintf(buf, "%s %s: %s", GET_NAME(ch), CMD_NAME, argument);
  mudlog(buf, NRM, LVL_IMMORT, TRUE);

  if (stat(filename, &fbuf) < 0) {
    perror("Error statting file");
    return;
  }
  if (fbuf.st_size >= max_filesize) {
    send_to_char("Sorry, the file is full right now.. try again later.\r\n", ch);
    return;
  }
  if (!(fl = fopen(filename, "a"))) {
    perror("do_gen_write");
    send_to_char("Could not open the file.  Sorry.\r\n", ch);
    return;
  }
  fprintf(fl, "%-8s (%6.6s) [%5d] %s\n", GET_NAME(ch), (tmp + 4),
	  world[ch->in_room].number, argument);
  fclose(fl);
  send_to_char("Okay.  Thanks!\r\n", ch);
}



#define TOG_OFF 0
#define TOG_ON  1

ACMD(do_gen_tog)
{
  long result;
  extern int nameserver_is_slow;
  char tmp[128];

  char *tog_messages[][2] = {
    {"You are now safe from summoning by other players.\r\n",
    "You may now be summoned by other players.\r\n"},
    {"Nohassle disabled.\r\n",
    "Nohassle enabled.\r\n"},
    {"Brief mode off.\r\n",
    "Brief mode on.\r\n"},
    {"Compact mode off.\r\n",
    "Compact mode on.\r\n"},
    {"You can now hear tells.\r\n",
    "You are now deaf to tells.\r\n"},
    {"You can now hear auctions.\r\n",
    "You are now deaf to auctions.\r\n"},
    {"You can now hear shouts.\r\n",
    "You are now deaf to shouts.\r\n"},
    {"You can now hear gossip.\r\n",
    "You are now deaf to gossip.\r\n"},
    {"You can now hear the congratulation messages.\r\n",
    "You are now deaf to the congratulation messages.\r\n"},
    {"You can now hear the Wiz-channel.\r\n",
    "You are now deaf to the Wiz-channel.\r\n"},
    {"/cGYou are no longer part of the Quest./c0\r\n",
    "/cGOkay, you are part of the Quest!/c0\r\n"},
    {"You will no longer see the room flags.\r\n",
    "You will now see the room flags.\r\n"},
    {"You will now have your communication repeated.\r\n",
    "You will no longer have your communication repeated.\r\n"},
    {"HolyLight mode off.\r\n",
    "HolyLight mode on.\r\n"},
    {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
    "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
    {"Autoexits disabled.\r\n",
    "Autoexits enabled.\r\n"},
    {"Autoassist disabled.\r\n",
    "Autoassist enabled.\r\n"},
    {"Autosplit disabled.\r\n",
    "Autosplit enabled.\r\n"},
    {"Newbie tips disabled.\r\n",
    "Newbie tips enabled.\r\n"},
    {"You are no longer away from your keyboard.\r\n",
    "You are away from your keyboard.\r\n"},
    {"You will no longer see arena messages.\r\n",
    "You will now see arena messages.\r\n"},
    {"You will no longer automatically loot corpses.\r\n",
    "You will now automatically loot corpses.\r\n"},
    {"You will now receive informational messages.\r\n",
    "You will no longer receive informational messages.\r\n"},
    {"You will no longer see condition messages during fights.\r\n",
     "You will now see condition messages during fights.\r\n"},
    {"You will no longer automatically sacrifice corpses.\r\n",
    "You will now automatically sacrifice corpses.\r\n"},
    {"Autotitle disabled.\r\n",
    "Autotitle enabled.\r\n"},
    {"You will no longer automatically see surrounding map.\r\n",
    "You will now automatically see surrounding map.\r\n"},
    {"You will now see your rank displayed on the who list.\r\n",
    "You will no longer see your rank displayed on the who list.\r\n"},
    {"You will now go to the Grid when you level.\r\n",
    "You will no longer go to the Grid when you level.\r\n"},
    {"Autogold disabled.\r\n",
     "Autogold enabled.\r\n"},
    {"You will no longer see mob flags.\r\n",
     "You will now see mob flags.\r\n"},
    {"Others can now follow you.\r\n",
    "You will no longer be followed.\r\n"},
  };


  if (IS_NPC(ch))
    return;

  switch (subcmd) {
  case SCMD_NOSUMMON:
    result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
    break;
  case SCMD_NOHASSLE:
    result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
    break;
  case SCMD_BRIEF:
    result = PRF_TOG_CHK(ch, PRF_BRIEF);
    break;
  case SCMD_COMPACT:
    result = PRF_TOG_CHK(ch, PRF_COMPACT);
    break;
  case SCMD_NOTELL:
    result = PRF_TOG_CHK(ch, PRF_NOTELL);
    break;
  case SCMD_NOAUCTION:
    result = PRF_TOG_CHK(ch, PRF_NOAUCT);
    break;
  case SCMD_DEAF:
    result = PRF_TOG_CHK(ch, PRF_DEAF);
    break;
  case SCMD_NOGOSSIP:
    result = PRF_TOG_CHK(ch, PRF_NOGOSS);
    break;
  case SCMD_NOGRATZ:
    result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
    break;
  case SCMD_NOWIZ:
    result = PRF_TOG_CHK(ch, PRF_NOWIZ);
    break;
  case SCMD_QUEST:
    if (!PRF_FLAGGED(ch, PRF_QUEST)) {
      if (can_join_quest(ch)) {
        result = PRF_TOG_CHK(ch, PRF_QUEST); /* Join quest */
        sprintf(tmp, "/cG%s joins the quest./c0\r\n", GET_NAME(ch));
        send_to_all(tmp);
      } else {
        send_to_char("You can't join the quest at the moment.\r\n", ch);
        return;
      }
    } else {
      result = PRF_TOG_CHK(ch, PRF_QUEST); /* quit quest */
    }
    break;
  case SCMD_ROOMFLAGS:
    result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
    break;
  case SCMD_MOBFLAGS:
    result = PRF_TOG_CHK(ch, PRF_MOBFLAGS);
    break;
  case SCMD_NOREPEAT:
    result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
    break;
  case SCMD_HOLYLIGHT:
    result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
    break;
  case SCMD_SLOWNS:
    result = (nameserver_is_slow = !nameserver_is_slow);
    break;
  case SCMD_AUTOEXIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
    break;
  case SCMD_ARENA:
    result = PRF_TOG_CHK(ch, PRF_ARENA);
    break;
  case SCMD_AUTOASSIST:
    result = PRF_TOG_CHK(ch, PRF_AUTOASSIST);
    break;
  case SCMD_AUTOSPLIT:
    result = PRF_TOG_CHK(ch, PRF_AUTOSPLIT);
    break;
  case SCMD_SHOWTIPS:
    result = PRF_TOG_CHK(ch, PRF_SHOWTIPS);
    break;
  case SCMD_AFK:
    result = PRF_TOG_CHK(ch, PRF_AFK);
    if (result)
      act("$n goes away from $s keyboard.", FALSE, ch, 0, 0, TO_ROOM);
    else
      act("$n returns to $s keyboard.", FALSE, ch, 0, 0, TO_ROOM);
    break;
/* Start the bits on the next vector! */
  case SCMD_AUTOLOOT:
    result = PRF_TOG_CHK(ch, PRF_AUTOLOOT);
    break;
  case SCMD_AUTOGOLD:
    result = PRF_TOG_CHK(ch, PRF_AUTOGOLD);
    break;
  case SCMD_NOINFO:
    result = PRF_TOG_CHK(ch, PRF_NOINFO);
    break;
  case SCMD_AUTODIAG:
    result = PRF_TOG_CHK(ch, PRF_AUTODIAG);
    break;
  case SCMD_AUTOSAC:
    result = PRF_TOG_CHK(ch, PRF_AUTOSAC);
    break;
  case SCMD_AUTOTITLE:
    result = PRF_TOG_CHK(ch, PRF_AUTOTITLE);
    break;
  case SCMD_AUTOMAP:
    result = PRF_TOG_CHK(ch, PRF_AUTOMAP);
    break;
  case SCMD_NORANK:
if (PLR_FLAGGED(ch, PLR_KEEPRANK)) {
send_to_char("You can't do that.\r\n", ch);
return;
}
    result = PRF_TOG_CHK(ch, PRF_NORANK);
    break; 
  case SCMD_NOGRID:
    result = PRF_TOG_CHK(ch, PRF_NOGRID);
    break; 
  case SCMD_NOFOLLOW:
    result = PRF_TOG_CHK(ch, PRF_NOFOLLOW);
    break;
  default:
    log("SYSERR: Unknown subcmd in do_gen_toggle");
    return;
    break;
  }

  if (result)
    send_to_char(tog_messages[subcmd][TOG_ON], ch);
  else
    send_to_char(tog_messages[subcmd][TOG_OFF], ch);

  return;
}


ACMD(do_spy)
{
  int dir, return_room;
  extern char *dirs[];

  if (!GET_SKILL(ch, SKILL_SPY)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  dir = search_block(argument + 1, dirs, FALSE);
  if (dir < 0 || !EXIT(ch, dir) ||
      EXIT(ch, dir)->to_room == NOWHERE) {
    send_to_char("Spy where?\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_SPY) < number(1, 101))
    send_to_char("You don't see anything.\r\n", ch);
  else {
    if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)&&EXIT(ch, dir)->keyword){
      sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    } else {
      return_room = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, world[return_room].dir_option[dir]->to_room);
      send_to_char("You spy into the next room and see: \r\n\r\n", ch);
      look_at_room(ch, 1);
      char_from_room(ch);
      char_to_room(ch, return_room);
      act("$n peeks into the next room.", TRUE, ch, 0, 0, TO_NOTVICT);
    }
  }
}

/* Forage skill */
#define FORAGE_COST 10
ACMD(do_forage)
{
  int percent;
  int prob;
  struct obj_data *obj;

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_FORAGE);

  if (!prob) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (GET_MOVE(ch)<FORAGE_COST) {
    send_to_char("You are too tired.\r\n", ch);
    return;
  }

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("But you can't see!\r\n", ch);
    return;
  }
  /*MJ*/
  if (IS_IMPEN(ch->in_room)) {
    send_to_char("You can't see a thing.\r\n", ch);
    return;
  }

  if (SECT(ch->in_room)!=SECT_FOREST && SECT(ch->in_room)!=SECT_FIELD  &&
      SECT(ch->in_room)!=SECT_HILLS  && SECT(ch->in_room)!=SECT_MOUNTAIN) {
    send_to_char("You can't forage here.\r\n", ch);
    return;
  }

  if (prob > percent) {
    obj = read_object(1091, VIRTUAL);
    obj_to_char(obj, ch);
    send_to_char("You find some food!\r\n", ch);
    act("$n finds some food!", FALSE, ch, 0, 0, TO_ROOM);
  } else {
    send_to_char("You forage for a while but come up empty handed.\r\n", ch);
    act("$n forages but doesn't seem to find anything.",
        FALSE, ch, 0, 0, TO_ROOM);
  }
  GET_MOVE(ch) -= FORAGE_COST;

}

/* Bandage skill for Druids */
#define BANDAGE_MOVE_COST 20
ACMD(do_bandage)
{

  struct char_data *vict;
  int percent;
  int prob;

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_BANDAGE);

  if (!prob) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this skill in the Grid.", ch);
    return;
  } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("This protective magic does not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }

  if (GET_MOVE(ch)<BANDAGE_MOVE_COST) {
    send_to_char("You are too tired.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg)
    vict = ch;
  else if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
    
  if (percent > prob) {
    send_to_char("You try but it doesn't seem to help.\r\n", ch);
    improve_abil(ch, SKILL_BANDAGE, ABT_SKILL);
    return;
  }

  if (vict==ch) {
    send_to_char("You dress your wounds.\r\n", ch);
    act("$n dresses $s wounds.", FALSE, ch, 0, 0, TO_ROOM);
  } else {
    act("$n dresses your wounds.", FALSE, ch, 0, vict, TO_VICT);
    act("You dress $N's wounds.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n dresses $N's wounds.", FALSE, ch, 0, vict, TO_NOTVICT);
  }

  GET_MOVE(ch)-=BANDAGE_MOVE_COST;
  GET_HIT(vict) += number((GET_LEVEL(ch) / 2), (GET_LEVEL(ch) / 4));
  GET_HIT(vict) = MIN(GET_HIT(vict), GET_MAX_HIT(vict));

}

#define AURA_VIM_COST 100
ACMD(do_aura_of_nature)
{
  if (GET_HIT(ch) >= (GET_MAX_HIT(ch) + 1)) {
     send_to_char("You still have nature's aura about you.\r\n", ch);
     return;
  }
  if (!GET_SKILL(ch, SKILL_AURA_OF_NATURE)) {
    send_to_char("You do not possess this skill.\r\n", ch);
    return;
  }
  if (GET_VIM(ch) <= AURA_VIM_COST) {
    send_to_char("You have not the vim for this skill.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this skill in the Grid.\r\n", ch);
    return;
  }
  act("You focus the aura of nature about yourself.", 
      FALSE, ch, 0, 0, TO_CHAR);
  act("$n closes $s eyes and focuses the aura of nature upon $mself.", 
      FALSE, ch, 0, 0, TO_ROOM); 
  if (GET_LEVEL(ch) <= 75) {
    GET_HIT(ch) += dice(25, 4);
    GET_VIM(ch) -= AURA_VIM_COST;
  } else if (GET_LEVEL(ch) <= 100 && GET_LEVEL(ch) >= 76) {
    if (GET_VIM(ch) <= AURA_VIM_COST + 25) {
      send_to_char("You have not the vim for this skill.\r\n", ch);
      return;
    } else
    GET_HIT(ch) += dice(25, 6);
    GET_VIM(ch) -= AURA_VIM_COST + 25;
  } else if (GET_LEVEL(ch) >= 101) {
    if (GET_VIM(ch) <= AURA_VIM_COST + 50) {
      send_to_char("You have not the vim for this skill.\r\n", ch);
      return;
    } else 
    GET_HIT(ch) += dice(25, 8);
    GET_VIM(ch) -= AURA_VIM_COST + 50;
  }
}


#define CALL_VIM_COST 35
#define CALL_HAWK 25
#define CALL_MONKEY 26
#define CALL_TIGER 27
#define CALL_BEAR 28
ACMD(do_call) {

  struct char_data *pet;
  struct follow_type *f;
  int chance;
  int penalty=0;

  if (!GET_SKILL(ch, SKILL_CALL_WILD)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this skill in the Grid.", ch);
    return;
  }  

  if (GET_VIM(ch) <= CALL_VIM_COST) {
    send_to_char("Your vim is too low.\r\n", ch);
    return;
  }

  GET_VIM(ch) -= CALL_VIM_COST;

  for (f = ch->followers; f; f = f->next)
    if (IN_ROOM(ch) == IN_ROOM(f->follower)) {
      if (IS_MOB(f->follower))
        penalty+=5;
      else
        penalty+=2;
    }

  if (GET_CHA(ch) < (penalty + 5)) {
    send_to_char("You can not attract any more followers now.\r\n", ch);
    return;
  }

  chance = number(0, GET_SKILL(ch, SKILL_CALL_WILD) + GET_LEVEL(ch));

  if (chance < 90) {
    act("Your call fails.  No wildlife was attracted.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n belts out an impressive call to the wild and looks a bit dissapointed as it remains unanswered.", FALSE, ch, 0, 0, TO_NOTVICT);
    return;
  } else if (chance  < 100)
    pet = read_mobile(CALL_HAWK, VIRTUAL);
  else if (chance  < 140)
    pet = read_mobile(CALL_MONKEY, VIRTUAL);
  else if (chance  < 180)
    pet = read_mobile(CALL_TIGER, VIRTUAL);
  else
    pet = read_mobile(CALL_BEAR, VIRTUAL);

  act("Your call of the wild is answered by $N.", FALSE, ch, 0, pet, TO_CHAR);
  act("$ns call of the wild is answered by $N.", FALSE, ch, 0, pet, TO_ROOM);

  IS_CARRYING_W(pet) = 1000;
  IS_CARRYING_N(pet) = 100;
  SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);  

  char_to_room(pet, IN_ROOM(ch));
  add_follower(pet, ch);
}


ACMD(do_holdbreath) {

  struct affected_type af;

  if (!GET_SKILL(ch, SKILL_HOLD_BREATH)) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  if (SECT(ch->in_room)==SECT_UNDERWATER) {
    send_to_char("You can't do that while you're underwater!\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(ch, AFF_BREATHE)) {
    send_to_char("You release your breath.\r\n", ch);
  }

  act("You take a deep breath!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n takes a deep breath!", FALSE, ch, 0, 0, TO_ROOM);

  af.type = SKILL_HOLD_BREATH;
  af.duration = (int)(GET_SKILL(ch, SKILL_HOLD_BREATH)/number(8, 12));
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_BREATHE;
  affect_to_char(ch, &af);

}


void catch_plague(struct char_data *ch)
{
    struct affected_type af;
 
    if (IS_AFFECTED(ch, AFF_PLAGUE)) {
      return;
    }

    /* well they are now. */
    else {
    send_to_char("\r\n", ch);
    send_to_char("You sneeze all over the place.\r\n", ch);
    act("$n sneezes.", FALSE, ch, 0, 0,TO_ROOM);
    send_to_char("You don't feel so good.\r\n", ch);
    act("$n doesn't look so good.", FALSE, ch, 0, 0,TO_ROOM);

    af.location = APPLY_HITROLL;
    af.modifier = -5;
    af.duration = 48;  
    af.bitvector = AFF_PLAGUE;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);

    }
}


ACMD(do_run)
{
  struct affected_type af;
  byte percent;

  if (GET_SKILL(ch, SKILL_RUN) <= 0) {
    send_to_char("You do not know that skill.\r\n", ch);
    return;
  }
  if (GET_VIM(ch) <= 99) {
    send_to_char("You haven't enough vim to use this skill.\r\n", ch);
    return;
  }

  send_to_char("Okay, you'll try to pick up the pace.\r\n", ch);

  if (IS_AFFECTED(ch, AFF_RUN))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_RUN);

  percent = number(1, 151);

  if (percent > GET_SKILL(ch, SKILL_RUN) + GET_CON(ch) + GET_DEX(ch))
    return;

  af.type = SKILL_RUN;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_RUN;
  affect_to_char(ch, &af);
  GET_VIM(ch) -= 100;
}

ACMD(do_shadow)
{
  struct affected_type af;
  byte percent;

  if (GET_QI(ch) < 35 - GET_WIS(ch)) {
    send_to_char("Your Qi is too weak to do that.\r\n", ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_SHADOW)) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  send_to_char("You blend with the shadows.\r\n", ch);
  if (IS_AFFECTED(ch, AFF_SHADOW))
    affect_from_char(ch, SKILL_SHADOW, ABT_SKILL);

  percent = number(1, 101);	/* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_SHADOW) + dex_app_skill[GET_DEX(ch)].sneak)
    return;

  af.type = SKILL_SHADOW;
  af.duration = (GET_LEVEL(ch) >> 2);
  af.modifier = -4;
  af.location = APPLY_AC;
  af.bitvector = AFF_SHADOW;
  affect_to_char(ch, &af);

}

bool is_animal(struct char_data *ch)
{
	if(GET_RACE(ch) == RACE_NPC_ANIMAL || GET_RACE(ch) == RACE_NPC_MAMAL || GET_RACE(ch) == RACE_NPC_REPTILE || GET_RACE(ch) == RACE_NPC_FISH || GET_RACE(ch) == RACE_NPC_AVIAN || GET_RACE(ch) == RACE_NPC_INSECT)
		return TRUE;
	else
		return FALSE;
}

ACMD(do_befriend)
{
  struct char_data *vict = NULL;
  struct affected_type af;
  int percent, prob;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Befriend what?\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("They're not hear...\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("How sad that you have no other friends...\r\n", ch);
    return;
  }

  if (!IS_NPC(vict)) {
    send_to_char("I don't think so...\r\n", ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_BEFRIEND)) {
    send_to_char("Nice doggie....\r\n", ch);
    return;
  }

  if (!is_animal(vict)) {
    send_to_char("That is not an animal!\r\n", ch);
    return;
  }

  if (GET_LEVEL(vict) > GET_LEVEL(ch)>>1) {
    send_to_char("That animal is too independent-minded to join the likes of you!\r\n", ch);
    return;
  }

  percent = dice(1, 100);
  prob = GET_SKILL(ch, SKILL_BEFRIEND) + GET_CHA(ch);

  if (percent > prob) {
    sprintf(buf, "%s snaps at your hand!\r\n", vict->player.short_descr);
    send_to_char(buf, ch);
    return;
  }


  if (vict->master)
    stop_follower(vict);

  add_follower(vict, ch);

  af.type = SPELL_CHARM;

  if (GET_INT(vict))
    af.duration = 24 * 18 / GET_INT(vict);
  else
    af.duration = 24 * 18;

  af.modifier = 0;
  af.location = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char(vict, &af);

  act("Isn't $n just such a nice fellow?", FALSE, ch, 0, vict, TO_VICT);
  if (IS_NPC(vict)) {
    REMOVE_BIT_AR(MOB_FLAGS(vict), MOB_AGGRESSIVE);
    REMOVE_BIT_AR(MOB_FLAGS(vict), MOB_SPEC);
  }
}

char* msgs[][2] = {

{"You start to break some floor boards when you dig.\r\n",

 "$n starts to break some floor boards as $e starts digging.\r\n" },

{"You wonder if this is a good place after all, with all the gravel.\r\n",

 NULL },

{"You make a nice hole while digging up a lot of dirt.\r\n",

 "$n digs a hole and goes about $s business.\r\n" },

{"You seem to be hitting alot of roots when you dig.\r\n",

 "$n look like $e is trying to dig up a tree!\r\n" },

{"You dig up more clay than dirt here.\r\n",

 "$n seems to be digging up alot of clay.\r\n" },

{"You start to chip away at the rock here.\r\n",

 "$n bangs away at the side of the mountain.\r\n" },

{"You can't dig in the water!\r\n",

 NULL },

{"You can't dig in the water!\r\n",

 NULL },

{"You can't dig in the water!\r\n",

 NULL },

{"You can't dig up air!\r\n",

 NULL },

{"You can't dig up the road!\r\n",

 NULL },

{"You start to scoop away the sand.\r\n",

 "$n starts diging in the sand.\r\n" },

{"You start to scoop away the sand.\r\n",

 "$n starts diging in the sand.\r\n" },

{"You start to scoop away the snow.\r\n",

 "$n starts diging in the snow.\r\n" },

{"You can't dig in a swamp!\r\n",

 NULL },

{"You can't dig in the water!\r\n",

 NULL },

/* always keep this as the last message */

{ "If you see this please tell a god.\r\n", NULL }
};

/* #pragma argsused */
ACMD(do_bury)
{
  
  struct obj_data *obj;
  half_chop(argument, arg, buf);

if (!GET_SKILL(ch, SKILL_BURY)) {
  send_to_char("You don't know how!\r\n", ch);
  return;
}

  if (!*arg) {
    sprintf(buf2, "What do you want to %s?\r\n", CMD_NAME);
    send_to_char(buf2, ch);
    return;

  }


  if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return;

  }


  /*
  ** find the sector types that you don't want people
  ** to be able to dig or bury in.
  */

  if((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
     (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
     (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
     (world[IN_ROOM(ch)].sector_type == SECT_ROAD) ||
     (world[IN_ROOM(ch)].sector_type == SECT_SWAMP) ||
     (world[IN_ROOM(ch)].sector_type == SECT_OCEAN) ||
     (world[IN_ROOM(ch)].sector_type == SECT_CITY) ||
     (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {

    /* display the messages if available */
    if(msgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
      send_to_char(msgs[world[IN_ROOM(ch)].sector_type][0], ch);

      if(msgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
	act(msgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL,TO_ROOM);
      return;
  }


  if(msgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
    send_to_char(msgs[world[IN_ROOM(ch)].sector_type][0], ch);

  if(msgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
    act(msgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL,TO_ROOM);

  act("You bury $a $o here.\r\n", TRUE, ch, obj, NULL, TO_CHAR);
  act("$n buries $a $o here.\r\n", TRUE, ch, obj, NULL, TO_ROOM);
  obj_from_char(obj);

  SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BURIED);
  obj_to_room(obj, IN_ROOM(ch));
};

/* #pragma argsused */
ACMD(do_digup)
{

struct obj_data *obj;
int found_item = 0;

if (!GET_SKILL(ch, SKILL_DIG)) {
  send_to_char("You don't know how!\r\n", ch);
  return;
}

/*
** find the sector types that you don't want people
** to be able to dig or bury in.
*/

if ((world[IN_ROOM(ch)].sector_type == SECT_WATER_SWIM) ||
   (world[IN_ROOM(ch)].sector_type == SECT_WATER_NOSWIM) ||
   (world[IN_ROOM(ch)].sector_type == SECT_UNDERWATER) ||
     (world[IN_ROOM(ch)].sector_type == SECT_ROAD) ||
     (world[IN_ROOM(ch)].sector_type == SECT_SWAMP) ||
     (world[IN_ROOM(ch)].sector_type == SECT_OCEAN) ||
   (world[IN_ROOM(ch)].sector_type == SECT_FLYING)) {

  /* display the messages if available */
 
  if(msgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
    send_to_char(msgs[world[IN_ROOM(ch)].sector_type][0], ch);

  if(msgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
    act(msgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL,TO_ROOM);

  return;
}




/*
** Now that we have established that we can dig lets go
** ahead and do it!
*/

if(msgs[world[IN_ROOM(ch)].sector_type][0] != NULL)
  send_to_char(msgs[world[IN_ROOM(ch)].sector_type][0], ch);

if(msgs[world[IN_ROOM(ch)].sector_type][1] != NULL)
  act(msgs[world[IN_ROOM(ch)].sector_type][1], TRUE, ch, NULL, NULL,TO_ROOM);


/*
** search for an object in the room that has a ITEM_BURIED flag
*/

obj = world[ch->in_room].contents;

while(obj != NULL) {
  if(IS_BURIED(obj))    {

/* Remove the buried bit so the player can see it. */
    REMOVE_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BURIED);

    if(CAN_SEE_OBJ(ch, obj)) {
      found_item = 1;     /* player found something */
      act("You found $a $o buried here.\r\n", TRUE, ch, obj, NULL, TO_CHAR);
      act("$n has found $a $o buried here.\r\n", TRUE, ch, obj, NULL,TO_ROOM);
            obj_from_room(obj);
	    obj_to_char(obj, ch);
    }



    else {
/* add the bit back becuase the player can't unbury what
** what he can't find...  */
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BURIED);
    }
  }


/* go to the next obj */


obj = obj->next;

}


if(!found_item)   /* if the player didn't find anything */
  send_to_char("Sorry! You didn't find anything.\r\n", ch);
}

ACMD(do_assign_player)
{
  struct char_data *vict;
  int i = 0, c = 0, found = FALSE;

  one_argument(argument, arg);

  if (IS_NPC(ch))
    return;
  if (GET_LEVEL(ch) >= LVL_IMMORT) {
    send_to_char("You don't need to assign anyone; You're an imm!\r\n", ch);
    return;
  }
  
  while((i < MAX_ASSIGNED) && (CORPSE_RETRIEVAL(ch)[i] != NOBODY)) {
    if (get_player_idnum(CORPSE_RETRIEVAL(ch)[i]))
      CORPSE_RETRIEVAL(ch)[c++] = CORPSE_RETRIEVAL(ch)[i++];
    else
      i++;
  }
  while(c < MAX_ASSIGNED)
    CORPSE_RETRIEVAL(ch)[c++] = NOBODY;
 
  i = 0;
  if (*arg) {
    if ((strcmp(arg, "clear") == 0) ) {
      while((i < MAX_ASSIGNED) && (CORPSE_RETRIEVAL(ch)[i] > NOBODY))
        CORPSE_RETRIEVAL(ch)[i++] = NOBODY;
      send_to_char("You are the only one who may get your corpse now.\r\n", ch);
      return;
    } else if (!(vict = get_player(ch,arg))) {
      send_to_char(NOPERSON, ch);
      return;
    } else if (vict == ch) {
      send_to_char("Assign yourself? You can already touch your corpse!\r\n", ch);
      return;
    } else if (GET_LEVEL(vict) >= LVL_IMMORT) {
      send_to_char("You can't assign an immortal to get your corpse!\r\n", ch);
      return;
    } else {
      while((i < MAX_ASSIGNED) && (CORPSE_RETRIEVAL(ch)[i] > NOBODY)) {
        if (GET_IDNUM(vict) == CORPSE_RETRIEVAL(ch)[i]) {
          sprintf(buf, "You have already assigned %s to touch your corpse!\r\n", GET_NAME(vict));
          send_to_char(buf, ch);
          return;
        }
        i++;
    
      }
      if (i >= MAX_ASSIGNED) {
        for (i = 0; i < (MAX_ASSIGNED - 1); i++) 
          CORPSE_RETRIEVAL(ch)[i] = CORPSE_RETRIEVAL(ch)[i+1];
      }
      CORPSE_RETRIEVAL(ch)[i] = GET_IDNUM(vict);
      sprintf(buf, "You are now able to touch %s's corpse.\r\n", GET_NAME(ch));
      send_to_char(buf, vict);
    }
  }

  send_to_char("The following people are able to get your corpse:\r\n", ch);
  for(i = 0; i < MAX_ASSIGNED; i++) {
    if (CORPSE_RETRIEVAL(ch)[i] != NOBODY) {
      if ((vict = get_player_idnum(CORPSE_RETRIEVAL(ch)[i]))) {
        sprintf(buf, "   %s\r\n", GET_NAME(vict));
        send_to_char(buf, ch);
        found = TRUE;
      }
    }
  }
  if(!found)
    send_to_char("   None.\r\n", ch);

}  

/*
ACMD(do_climb)
{
  int percent, prob;

  one_argument(argument, arg);

  if(!*arg) {
    send_to_char("Which way?\r\n", ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_CLIMB)) {
    send_to_char("Mayhaps you should ask a druid or thief for help.\r\n", ch);
    return;
  }

  if(AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("Maybe you should try this when you can see...\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("But you're in the middle of a fight!\r\n", ch);
    return;
  } 

    percent = number(1, 101);
    prob = GET_SKILL(ch, SKILL_CLIMB);

    if (percent > prob) {
      send_to_char("You attempt the climb but instead end up on your ass 
          "look quite the fool.\r\n", ch);
      send_to_room("$n attempts to make a climb but instead falls on $s ass "
          "looking quite the fool.\r\n", ch->in_room);
      GET_POS(ch) = POS_SITTING;
      return;

    } else

    if ((percent - prob) >= 80) {
      send_to_char("You scrabble and claw, trying to make the climb but end "
          "up slipping and falling on the ground.\r\n", ch);
      send_to_room("$n scrabbles and claws, trying to make a climb but ends "
          "up slipping and falling on the ground.\r\n", ch->in_room);
      GET_POS(ch) = POS_SITTING;
      return;

    } else

    if ((percent - prob) >= 60) {
      send_to_char("You try, you really try - but you just can't seem to make "
          "the climb.\r\n", ch);
      send_to_room("$n makes one hell of an attempt at making a climb, but $e "
          "just can't seem to make it.\r\n", ch->in_room);
      return;

    } else
 
    send_to_char("You successfully make the climb!\r\n", ch);
      act("$n climbs away, making it look all too easy.\r\n",
          TRUE, 0, 0, ch, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, EXIT(ch,arg)->to_room); 
    sprintf(buf2, "%s climbs into the room with a satisfied grunt.\r\n",
                   GET_NAME(ch));
    act(buf2, TRUE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);
  }
  else
    send_to_char("There is no exit in that direction!\r\n", ch);

}
*/

ACMD(do_aid)
{
  struct char_data *vict;
        
  one_argument(argument, buf);
        
  if (!*buf) {
    send_to_char("Aid whom?\r\n", ch);
    return;
  }
        
  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("You do not see such a person!\r\n", ch);
    return;
  }

  if (GET_HIT(vict) < 1) {
    GET_HIT(vict) = 1;
    act("You rush to staunch $N's bleeding!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n rushes to your aid as you lie bleeding to death!", FALSE, ch, 0, vict, TO_VICT);
    act("$n jumps to $N's aid!", FALSE, ch, 0, vict, TO_NOTVICT);
    send_to_char("Your vision slowly returns!\r\n", vict);
  } else {
    send_to_char("Aid only works on incapacitated folks!\r\n", ch);
  }
}

ACMD(do_kiss)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (!*buf)
    send_to_char("Whom do you wish to kiss??\r\n", ch);

  else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);

  else if (ch == vict)
    send_to_char("You feel kind of silly.\r\n", ch);

  else if (!IS_NPC(vict) && !vict->desc)        /* linkless */
    send_to_char("Linkless people don't really respond well.\r\n", ch);

  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
        FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);

  else if (!AWAKE(vict))
  {
    if ( IS_AFFECTED(vict, AFF_SLEEP) && (GET_SEX(ch) == SEX_FEMALE))
    {
      REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_SLEEP);
      GET_POS(vict) = POS_RESTING;
      sprintf(buf2, "You wake %s up from a deep enchanted sleep with your kiss.\r\n", GET_NAME(vict));
      send_to_char(buf2, ch);
      sprintf(buf2, "%s kisses you, waking you up from a deep enchanted sleep.\r\n", GET_NAME(ch));
      send_to_char(buf2, vict);
      sprintf(buf2, "%s wakes %s up from a deep enchanted sleep with a kiss.\r\n", GET_NAME(ch), GET_NAME(vict));
      act(buf2, FALSE, ch, 0, vict, TO_NOTVICT);
    }
    else
      send_to_char("Shhhh. Try again later!\r\n", ch);
  }

  else
  {
    sprintf(buf2, "You kiss %s.\r\n", GET_NAME(vict));
    send_to_char(buf2, ch);
    sprintf(buf2, "%s kisses you.\r\n", GET_NAME(ch));
    send_to_char(buf2, vict);
    sprintf(buf2, "%s kisses %s.", GET_NAME(ch), GET_NAME(vict));
    act(buf2, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

ACMD(do_mobhunt)
{
  struct char_data *vict;
 
  if(!IS_NPC(ch)) { send_to_char("Mobiles only.\r\n", ch); return; }
  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Hunt who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }

    sprintf(buf, "You now hunt %s.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    HUNTING(ch) = vict;
  
}

ACMD(do_sprinkle)
{
  struct obj_data *obj, *cont;

  two_arguments(argument, buf, buf2);

  if (!*buf) {
    send_to_char("Which herb do you want to sprinkle into which jar?\r\n", ch);
    return;
  }

  if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    send_to_char("You aren't carrying that.\r\n", ch);
    return;
  }
  if (GET_OBJ_TYPE(obj) != ITEM_HERB) {
    send_to_char("Only herbs can be sprinkled into jars.\r\n", ch);
    return;
  }
  if (!*buf2) {
    sprintf(buf, "What you want to sprinkle %s into?\r\n", 
                  obj->short_description);
    send_to_char(buf, ch);
    return;
  }
  if (!(cont = get_obj_in_list_vis(ch, buf2, ch->carrying))) {
    send_to_char("You can't sprinkle herbs into something your aren't carrying!\r\n", ch);
    return;
  }
  if (GET_OBJ_TYPE(cont) != ITEM_JAR) {
    sprintf(buf, "You must use a jar for sprinkling and %s is not a jar.\r\n", 
                  cont->short_description);
    return;
  }
  if (GET_OBJ_VAL(cont, 3) > 0) {
    send_to_char("That jar has already been mixed.\r\nThe contents must be imbibed before anything new can be put into it.\r\n", ch);
    return;
  }
  perform_put(ch, obj, cont);
}

ACMD(do_mix)
{
  struct obj_data *obj;
  one_argument(argument, arg);

  if (GET_CLASS(ch) != CLASS_DRUID  &&
      GET_CLASS(ch) != CLASS_RANGER &&
      GET_CLASS(ch) != CLASS_BEASTMASTER && 
      GET_CLASS(ch) != CLASS_FORESTAL &&
      GET_CLASS(ch) != CLASS_SAGE &&
      GET_CLASS(ch) != CLASS_FIANNA &&
      GET_CLASS(ch) != CLASS_SHAMAN &&
      GET_LEVEL(ch) <= 150) {
    send_to_char("Only druids and druid remorts can use this skill.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_MIX)) {
    send_to_char("You have not yet learned the art of mixing.\r\n", ch);
    send_to_char("Please refer to HELP MIX for more information.\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("What is it that you want to mix?\r\n", ch);
    return;
  }
  if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
     send_to_char("You don't seem to have one of those.\r\n", ch);
     return;
  }
  if (GET_OBJ_TYPE(obj) != ITEM_JAR) {
    send_to_char("You can only mix the contents of jars.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(obj, 3) > 0) {
    send_to_char("That jar has already been mixed.\r\n", ch);
    return;
  }

  act("You mix the contents of $p.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n mixes the contents of $p.", FALSE, ch, obj, 0, TO_ROOM);
  GET_OBJ_VAL(obj, 3)++;   
}

const int number_of_mixable_spells = 29;

const int herb_combos[][3]={
  {1, 2, SPELL_ARMOR},
  {1, 3, SPELL_BLESS},
  {1, 5, SPELL_CURE_BLIND},
  {1, 6, SPELL_QI_RESTORE},
  {1, 7, SPELL_RESTORE_MANA},
  {1, 8, SPELL_RESTORE_MEGAMANA},
  {1, 9, SPELL_BREATHE},
  {2, 3, SPELL_DETECT_ALIGN},
  {2, 4, SPELL_INVISIBLE},
  {2, 5, SPELL_POISON},
  {2, 6, SPELL_REGENERATE},
  {2, 7, SPELL_HASTE},
  {2, 8, SPELL_REFRESH},
  {2, 9, SPELL_FLY}, 
  {3, 4, SPELL_SLEEP},
  {3, 5, SPELL_STRENGTH},
  {3, 6, SPELL_BLUR},
  {3, 8, SPELL_DUMBNESS},
  {4, 5, SPELL_INFRAVISION},
  {4, 6, SPELL_DARKWARD},
  {4, 7, SPELL_REMOVE_POISON},
  {5, 6, SPELL_CURE_CRITIC},
  {5, 7, SPELL_HEAL},
  {5, 8, SPELL_FIRESHIELD},
  {6, 7, SPELL_MANA_ECONOMY},
  {6, 8, SPELL_INSOMNIA},
  {7, 8, SPELL_GROWTH},
  {7, 9, SPELL_SANCTUARY},
  {8, 9, SPELL_MIST_FORM},
  {-1, -1, -1}
};

ACMD(do_imbibe)
{
  struct obj_data *cont, *obj, *next_obj;
  int i=0, j=0, FOUND=0;
  int obj_values[4];

  one_argument(argument, arg); 

  if(!*arg) {
    send_to_char("What do you want to imbibe?\r\n", ch);
    return;
  }
  if (!(cont = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    send_to_char("You don't have one of those. Better check you inventory.\r\n", ch);
    return;
  }
  if (GET_OBJ_TYPE(cont) != ITEM_JAR) {
    send_to_char("You can only imbibe the contents of jars.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(cont, 3) <= 0) {
    send_to_char("That jar has not yet been mixed. Try mixing it first.\r\n", ch);
    return;
  }
  for (obj = cont->contains; obj; obj = next_obj) {
    next_obj = obj->next_content;
    if (CAN_SEE_OBJ(ch, obj)) {
      obj_values[i] = GET_OBJ_VAL(obj, 0);
      i++;
    } 
  }

  for (j=0; j<number_of_mixable_spells;j++) {
    if ((herb_combos[j][0] == obj_values[0] && 
         herb_combos[j][1] == obj_values[1]) || 
        (herb_combos[j][0] == obj_values[1] &&
         herb_combos[j][1] == obj_values[0])) {
      call_magic(ch, ch, 0, herb_combos[j][2], ABT_SPELL, GET_LEVEL(ch), CAST_SPELL, FALSE);
      FOUND = 1;
      sprintf(buf, "\r\nThe magic of the herbal combination is too much for %s. \r\nIt shatters into pieces.\r\n", cont->short_description);
      send_to_char(buf, ch);
      extract_obj(cont);
      return;
    }
  }
  send_to_char("This jar does not contain a valid herb combination.\r\n", ch);
  send_to_char("Nothing happens.\r\n", ch);
  return;
}

ACMD(do_beg)
{
  struct char_data *vict;
  int chance, goal, gold;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_BEG)) { send_to_char("You don't know how!\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
    send_to_char("They don't allow your sort in here, get out!\r\n", ch);
    WAIT_STATE(ch, 50);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Beg from who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("Come on now, you're gonna get charity from yourself?!\r\n", ch);
    return;
  }
  if (!IS_NPC(vict)) {
    send_to_char("Players will not succumb to your trickery.\r\n", ch);
    send_to_char("Try a mob.\r\n", ch);
    return;
  }
  chance = GET_CHA(ch) + GET_SKILL(ch, SKILL_BEG) + number(10, 45);
  goal = number(70, 150);
  gold = MAX(1, number((GET_GOLD(vict) * .02), (GET_GOLD(vict) * .1)));

  if (GET_LEVEL(vict) > GET_LEVEL(ch))
    goal = goal + (GET_LEVEL(vict) - GET_LEVEL(ch));

  if (chance > goal) {
    GET_GOLD(vict) -= gold;
    GET_GOLD(ch) += gold;
    sprintf(buf2, "%s give you a look of pity, handing you %d coins.\r\n", GET_NAME(vict), gold);
    send_to_char(buf2, ch);
    sprintf(buf2, "%s gives %s a look of pity, handing over some coins.", GET_NAME(ch), GET_NAME(vict));
    act(buf2, FALSE, ch, 0, vict, TO_NOTVICT);
    WAIT_STATE(ch, 50);
    return;
  } 
  if (chance < goal && (goal - chance) <= 10) {
    sprintf(buf2, "%s give you a look of pity, but gives you nothing.\r\n", 
            GET_NAME(vict));
    send_to_char(buf2, ch);
    sprintf(buf2, "%s gives %s a look of pity, promising to 'catch him next time'.", GET_NAME(ch), GET_NAME(vict));
    act(buf2, FALSE, ch, 0, vict, TO_NOTVICT); 
    WAIT_STATE(ch, 50);
    return;
  } else
    sprintf(buf2, "%s give you a look of contempt, calling out your bluff!\r\n",
                   GET_NAME(vict));
    send_to_char(buf2, ch);
    sprintf(buf2, "%s gives %s a look of contempt and attacks!", GET_NAME(ch), GET_NAME(vict));
    act(buf2, FALSE, ch, 0, vict, TO_NOTVICT); 
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, 50);
    return;
}

ACMD(do_bite)
{
  struct char_data *vict;
  int percent, prob;
  int bite;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Who do you wish to bite?\r\n", ch);
    return;
  } else if (!(GET_CLASS(ch) == CLASS_VAMPIRE)) {
    send_to_char("Only vampires can bite.\r\n", ch);
    return;
  } else if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Your victim does not appear to be in the vicinity.\r\n", ch);
    return;
  } else if (vict == ch) {
    send_to_char("You can gain nothing by biting yourself.\r\n", ch);
    return;
  } else if (IS_NPC(vict)) {
    send_to_char("You can only bite other players to gain health.\r\n", ch);
    return;
  } else if (GET_LEVEL(vict) <= 99) {
    send_to_char("Players of such low stature are not worth your time.\r\n", ch);
    return;
  } else if (GET_LEVEL(vict) >= LVL_IMMORT) {
    send_to_char("Sometimes Gods bite back... that may be a bad idea.\r\n", ch);
    return;
  } else if (FIGHTING(vict)) {
    send_to_char("Your victim is moving too fast for you to bite.\r\n", ch);
    return;

  } else
  
  bite = dice (10, 15);

  if (GET_HIT(vict) <= 2) {
    send_to_char("Your victim is already drained to near death. Their "
                 "blood would lend no strength.\r\n", ch);
    return;
  } else
  if (GET_MAX_HIT(ch) <= GET_HIT(ch)) {
    send_to_char("Your blood supply is already at its peak.\r\n", ch);
    return;
  } else
  
  percent = number(1, 101);
  prob = ((GET_STR(vict) + GET_DEX(vict) + GET_INT(vict)) + number (1, 20));
  
  if (percent < prob && AWAKE(vict)) {

    act("$N twists from your bared fangs and pushes you away with a look "
        "of horror on $S face.", FALSE, ch, 0, vict, TO_CHAR);
    act("$n makes a sudden move in an attempt to bite you, but you twist "
        "away and out of $s reach.", FALSE, ch, 0, vict, TO_VICT);
    act("$n makes a sudden move in an attempt to bite $N, but $N twists "
        "away and out of $n's reach.", FALSE, ch, 0, vict, TO_NOTVICT);

  } else {

  GET_HIT(vict) = MAX(2, (GET_HIT(vict) - bite));
  GET_HIT(ch) += bite;
  GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
  act("Before $N can realize your true intentions, you pierce $S neck "
      "with your fangs, drawing upon $S blood.", TRUE, ch, 0, vict, TO_CHAR);
  act("$n leans slightly in toward you and in a sudden movement sinks "
      "$s fangs into your neck!", TRUE, ch, 0, vict, TO_VICT);
  act("$n leans slightly in toward $N and in a sudden movement sinks "
      "$s fangs into $N's neck!", TRUE, ch, 0, vict, TO_NOTVICT);
  return;
  }
}

ACMD(do_vanish)
{
  int percent, prob;

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("I swear... all you vampire wanna-be's out there...\r\n", ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_VANISH)) {
    send_to_char("You must learn the ancients arts before using them.\r\n", ch);
    return;
  }
 
  if (!FIGHTING(ch)) {
    send_to_char("There is no need to vanish. You are without foes.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_VANISH) + 20;

  if (percent <= prob) {
    if (FIGHTING(FIGHTING(ch)) == ch)
      stop_fighting(FIGHTING(ch));
    stop_fighting(ch); 
    act("Calling upon your dark magics, you rise into the air, out of the\r\n"
        "foray and away to a haven.\r\n", TRUE, ch, 0, 0, TO_CHAR);
    act("You watch in frustration as $n rises from the ground, flying high\r\n"
        "away from the battle. Alas, you must kill the beast another day.\r\n",
        TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(number(10, 20)));
    look_at_room(ch, 0);
    act("A mist coalesces in the room, solidifying into $n.",
        TRUE, ch, 0, 0, TO_ROOM);
    return;
  } else {
    send_to_char("The fighting is too intense, you cannot find escape.\r\n", ch);
    return;
  }
}


ACMD(do_mesmerize)
{
  struct affected_type af[1];
  struct char_data *vict;
  int percent;
  int prob;

  percent = (number(1, 101));   /* 101% is a complete failure */
  prob = (number(1, 70) + GET_CHA(ch)); 

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You try looking dark and mysterious, but... it just "
                 "ain't working. Sorry...\r\n", ch);
    return;
  }


  if (!GET_SKILL(ch, SKILL_MESMERIZE)) {
    send_to_char("Entrancing others can only be done if you are taught "
                 "the art. Seek a Master to train you.\r\n", ch);
    return;
  }


  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Mesmerize who?\r\n", ch);
      return;
    }
  }

  if (vict == ch) {
    send_to_char("You cannot entrance yourself.\r\n", ch);
    return;
  }

  if (GET_CLASS(vict) == CLASS_VAMPIRE) {
    send_to_char("Vampire cannot entrance one another - it is not possible.\r\n"
, ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to entrance that player.\r\n", ch);
    return;
  }

  if (prob > percent) {
    act("You stare deeply into $N's eyes but fail to entrance $M.",
         FALSE, ch, 0, vict, TO_CHAR);
    act("$n stares into your eyes, attempting to entrance you.\r\n",
        FALSE, ch, 0, vict, TO_VICT);
    act("$n stares deeply into $N's eyes, but without apparent affect.",
        FALSE, ch, 0, vict, TO_NOTVICT);
    return;
  }

  if (percent > prob) {

      af[0].bitvector = AFF_MESMERIZED;
      af[0].duration = number(50, 150) / 25;
      affect_join(vict, af, FALSE, FALSE, FALSE, FALSE);  

      act("You stare deeply into $N's eyes, entrancing $M in a fugue state.",
           FALSE, ch, 0, vict, TO_CHAR);
      act("$n stares deeply into your eyes, entrancing you into a fugue state.\r
\n"
          "You are helpless to do anything.\r\n",
          FALSE, ch, 0, vict, TO_VICT);
      act("$n stares deeply into $N's eyes, entrancing $M.",
          FALSE, ch, 0, vict, TO_NOTVICT);
      return;
  } else
  return;
}    
 

ACMD(do_hiss)
{

  struct affected_type af[2];
  struct char_data *vict;
  int percent;
  int prob;
  int flee;

  percent = (number(1, 101));   /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_HISS);
  flee = number(1, 10);

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You give a gentle hiss. Hm...\r\n", ch);
    return;
  }
    
  if (!GET_SKILL(ch, SKILL_HISS)) {
    send_to_char("You give a hiss, but with little affect.\r\n"
                 "You must train with the Masters to hiss with power.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Hiss at who?\r\n", ch);
      return;
    }
  }

  if (!FIGHTING(ch)) {
    send_to_char("You hiss quietly to yourself in anticipation of blood.\r\n", ch);
    act("$n hisses quietly to $mself in anticipation of blood.\r\n",
        FALSE, ch, 0, vict, TO_ROOM);
    return;
  }

  if (vict == ch) {
    send_to_char("Keep your dignity about you. Others may be watching\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {    
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (prob < percent) {
    act("You hiss fiercely at $N, but $E stands $S ground.\r\n",
         FALSE, ch, 0, vict, TO_CHAR);
    act("$n hisses venomously at you, but you stand your ground.\r\n",
        FALSE, ch, 0, vict, TO_VICT);
    act("$n hisses venomously at $N, but $E stands $S ground.\r\n",
        FALSE, ch, 0, vict, TO_NOTVICT);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return;
  } 

  if (percent < prob) {

    if (AFF_FLAGGED(vict, AFF_HISS)) {
      if (flee > 8 && GET_CLASS(vict) != CLASS_VAMPIRE) {
        act("You hiss fiercely at $N, causing $M to run in fear.\r\n",
             FALSE, ch, 0, vict, TO_CHAR);
        act("$n hisses venomously at you, causing you to run in fear.\r\n",
            FALSE, ch, 0, vict, TO_VICT);
        act("$n hisses venomously at $N, causing $M to run in fear.\r\n",
            FALSE, ch, 0, vict, TO_NOTVICT);
        do_flee(vict, "", 0, SCMD_FLEE);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
      } else {
      act("You hiss fiercely at $N, but $E stands $S ground.\r\n",
           FALSE, ch, 0, vict, TO_CHAR);
      act("$n hisses venomously at you, but you stand your ground.\r\n",
          FALSE, ch, 0, vict, TO_VICT);
      act("$n hisses venomously at $N, but $E stands $S ground.\r\n",
          FALSE, ch, 0, vict, TO_NOTVICT);
      WAIT_STATE(ch, PULSE_VIOLENCE * 2);
      return;
      }
    } 

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -10;
    af[0].duration = 2;
    af[0].bitvector = AFF_HISS;

    af[1].location = APPLY_DAMROLL;
    af[1].modifier = -10;
    af[1].duration = 2;
    af[1].bitvector = AFF_HISS;

    affect_join(vict, af, FALSE, FALSE, FALSE, FALSE);
    affect_join(vict, af+1, FALSE, FALSE, FALSE, FALSE);

    act("You hiss fiercely at $N, turning $M pale with fear.\r\n",
         FALSE, ch, 0, vict, TO_CHAR);
    act("$n hisses venomously at you, turning you pale with fear.\r\n",
        FALSE, ch, 0, vict, TO_VICT);
    act("$n hisses venomously at $N, turning $M pale with fear.\r\n",
        FALSE, ch, 0, vict, TO_NOTVICT); 
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    return;
  }
  else
  return;
} 


ACMD(do_hover)
{
  byte percent;

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You have no fangs, need for blood. Are you then a "
                 "special breed of vampire?! I think not...\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("You cannot call upon the darkness when so distracted.\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_HOVER)) {
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HOVER);
    send_to_char("You settle gently to the ground, letting go of the "
                 "darkness... for now.\r\n", ch);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_HOVER))
    return;

  send_to_char("Wielding dark powers, you rise into the air.\r\n", ch);
  SET_BIT_AR(AFF_FLAGS(ch), AFF_HOVER);
}

ACMD(do_find_flesh)
{
  int found = 0;
  register struct char_data *i;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_FIND_FLESH)) {
    send_to_char("You must learn the art of finding the flesh first.\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You have no fangs, need for blood. Are you then a "
                 "special breed of vampire?! I think not...\r\n", ch);
    return;
  }

  if (!*arg) {
    send_to_char("You must specify who it is you thirst for.\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("You cannot call upon the darkness when so distracted.\r\n", ch);
    return;
  }

  for (i = character_list; i; i = i->next) {
   if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
     found = 1;
     act("$n closes $s eyes, going into a momentary trance.",
          TRUE, ch, 0, 0, TO_ROOM);
     send_to_char("You reach out with your vampire mind, finding your prey.\r\n"
                  "You feel a momentary dizziness, a loss of blood.\r\n", ch);
     sprintf(buf, "/cr%-25s - %s/c0\r\n", GET_NAME(i), world[i->in_room].name);
     send_to_char(buf, ch);
     GET_HIT(ch) -= number(125, 400);
     return;
     } 
  } 
  send_to_char("That for whom you thirst is beyond your senses.\r\n", ch);
  act("$n closes $s eyes, going into a momentary trance.",
       TRUE, ch, 0, 0, TO_ROOM);
  return; 
}

ACMD(do_burrow)
{
  if (!GET_SKILL(ch, SKILL_SACRED_EARTH)) {
    send_to_char("You must learn to use the gifts of the earth first.\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You have no fangs, need for blood. Are you then a "
                 "special breed of vampire?! I think not...\r\n", ch);
    return;
  }
  if (FIGHTING(ch)) {
    send_to_char("You cannot do this in a fight.\r\n", ch);
    return;
  }
  if (world[ch->in_room].sector_type != SECT_FOREST) {
    send_to_char("You can only burrow into the earth.\r\n"
                 "Try finding a forest.\r\n", ch);
    return;
  }
  send_to_char("You burrow down into the dirt, protecting your body from "
               "the sun and any passersby.\r\n", ch);
  act("As $n lays upon the ground and closes $s eyes,\r\n"
      "the earth beneath $m begins to tremble. Slowly,\r\n"
      "$n sinks into the earth, vanishing from sight.\r\n",
      TRUE, ch, 0, 0, TO_ROOM);
  SET_BIT_AR(AFF_FLAGS(ch), AFF_BURROW);
   GET_POS(ch) = POS_RESTING;
}

ACMD(do_rise)
{
  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("Only vampires rise. You are not a vampire.\r\n", ch);
    return;
  } 

  if (!AFF_FLAGGED(ch, AFF_BURROW)) {
    send_to_char("You are not in the earth, you are already risen.\r\n", ch);
    return;
  }
  send_to_char("You rise from the earth, ready to face your foes.\r\n", ch);
  act("With a great trembling, the earth parts and $n rises from beneath you.",
       TRUE, ch, 0, 0, TO_ROOM);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_BURROW);
  GET_POS(ch) = POS_STANDING;
}


ACMD(do_deathcall)
{
  register struct char_data *vict;
  int success, blood_loss, found = 0;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_DEATH_CALL)) {
    send_to_char("You must learn to call to your victims first.\r\n", ch);
    send_to_char("Seek a master.\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("You have no fangs, need for blood. Are you then a "
                 "special breed of vampire?! I think not...\r\n", ch);
    return;
  }

  if (!*arg) {
    send_to_char("You must specify who it is that you wish to kill.\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("You cannot call upon the dark arts when so distracted.\r\n", ch);
    return;
  } 
  success = number(1, 100) + GET_TIER(ch);
  blood_loss = number(400, 750);

  for (vict = character_list; vict; vict = vict->next) {
   if (CAN_SEE(ch, vict) && vict->in_room != NOWHERE && 
       isname(arg, vict->player.name)) {

      if (GET_LEVEL(vict) <= 100 && GET_CLASS(vict) <= CLASS_VAMPIRE) {
        send_to_char("That mortal is not powerful enough for you to Call.\r\n", ch);
        return;
      } 
      if (IS_AFFECTED(vict, AFF_DEATHCALL)) {
        send_to_char("That flesh has already been called.\r\n", ch);
        return;
      }
      if (success >= 75) {
        act("You send your call out across the world, finding your mark.\r\n"
            "You have sent your DeathCall to $N.", 
            TRUE, ch, 0, vict, TO_CHAR); 
        act("$n tilts $s head back, uttering incoherant sounds into the sky.\r\n",
            TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("A vampire has sent a DeathCall out to you.\r\n"
                     "You have been marked for death.\r\n", vict);
        found = 1;
        SET_BIT_AR(AFF_FLAGS(vict), AFF_DEATHCALL);
        GET_HIT(ch) -= blood_loss;
        return;
      }
      if (success <= 74) {
        act("You send your call out across the world, searching for your mark.\r\n"
            "Your blood is not strong enough to reach $N.\r\n",
             TRUE, ch, 0, vict, TO_CHAR);
        act("$n tilts $s head back, uttering incoherant sounds into the sky.\r\n",
             TRUE, ch, 0, 0, TO_ROOM); 
        GET_HIT(ch) -= blood_loss;
        return;
      }
    }
  }
  send_to_char("You cannot find anyone by that name.\r\n", ch);
  return;
}
 
ACMD(do_scorch)
{
  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("Only Titans have the ability to scorch the earth.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_SCORCH)) {
    send_to_char("You must first learn how to call the magma from the bowels of the earth.\r\n", ch);
    return;
  } 
  if (SECT(ch->in_room) != SECT_FOREST && SECT(ch->in_room) != SECT_FIELD &&
      SECT(ch->in_room) != SECT_HILLS && SECT(ch->in_room) != SECT_MOUNTAIN &&
      SECT(ch->in_room) != SECT_SAND && SECT(ch->in_room) != SECT_ROAD &&
      SECT(ch->in_room) != SECT_DESERT && SECT(ch->in_room) != SECT_SWAMP) {
     send_to_char("There is no earth to scorch here.\r\n", ch);
     return;
  }
  if (GET_HIT(ch) <= 1499) {
    send_to_char("You haven't enough health to perform this task.\r\n", ch);
    return;
  }
  if (number(1, 10) >= 5) {
    send_to_char("You kneel down, placing your hands upon the earth.\r\n", ch);
    send_to_char("Your head is filled with an odd ringing sound, you begin to perspire.\r\n", ch);
    send_to_char("/cwYou haven't the ability to call this power right now./c0\r\n", ch);
    act("$n kneels down, placing $s hands upon the earth.\r\nNothing happens.",
        TRUE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) -= 1500;
    return;
  }
  send_to_char("You kneel down, placing your hands upon the earth.\r\n", ch);
  send_to_char("Your head is filled with an odd ringing sound, you begin to perspire.\r\n", ch);
  send_to_char("The earth beneath your hands begins to heat up, becoming red-hot.\r\n", ch);
  act("$n kneels down, placing $s hands upon the earth.\r\n"
      "$n begins to perspire, breathing heavily.\r\n"
      "/cwCollapsing in a heap, $n lays back upon the now/cr red-hot/cw earth./c0\r\n",
      TRUE, ch, 0, 0, TO_ROOM);
  SET_BIT_AR(ROOM_FLAGS(ch->in_room), ROOM_SCORCHED);
  GET_HIT(ch) -= 1500;
  return; 

}


ACMD(do_absorb)
{
  struct affected_type af;

  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("Only titans have the ability to absorb the earth's energies.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_ABSORB)) {
    send_to_char("You must first learn the art of absorbing the earth's energies.\r\n", ch);
    return;
  }
  if (SECT(ch->in_room) != SECT_FOREST && SECT(ch->in_room) != SECT_FIELD &&
      SECT(ch->in_room) != SECT_HILLS && SECT(ch->in_room) != SECT_MOUNTAIN &&
      SECT(ch->in_room) != SECT_SAND && SECT(ch->in_room) != SECT_ROAD &&
      SECT(ch->in_room) != SECT_DESERT && SECT(ch->in_room) != SECT_SWAMP) {
     send_to_char("This area is not suitable for absorbing the earth's energies.\r\n", ch);
     return;
  }
  send_to_char("You kneel down onto the ground, placing your hands upon the earth.\r\nYou go into a trance.\r\n", ch);
  act("$n kneels down onto the ground, $s eyes going vacant.\r\n",
       TRUE, ch, 0, 0, TO_ROOM);  

  af.type = SKILL_ABSORB;
  af.duration = 1;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_ABSORB;
  affect_to_char(ch, &af);
  return;
}

ACMD(do_cloud_cover)
{
  struct char_data *vict;
  struct affected_type af;

  one_argument(argument, arg);

  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("Only titans possess the power to create a cloud.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_CLOUD)) {
    send_to_char("You must first learn the art of creating clouds.\r\n", ch);
    return;
  }
  if (GET_HIT(ch) <= 1000) {
    send_to_char("You have not the strength to call a locator cloud.\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Who is it you wish to cover with a locator cloud?\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Look again, big guy. No one in this room by that name.\r\n", ch);
    return;
  }
  act("You call to the elements, creating a dark cloud that hovers just \r\n"
      "over $N's right shoulder.\r\n", TRUE, ch, 0, vict, TO_CHAR);
  act("$n throws $s head back, calling to the sky. A dark cloud begins to\r\n"
      "form just over your right shoulder.\r\n",
      TRUE, ch, 0, 0, TO_VICT);
  act("$n throws $s head back, calling to the sky. A dark cloud forms just\r\n"
      "over $N's right shoulder.\r\n", TRUE, ch, 0, vict, TO_NOTVICT);
  GET_HIT(ch) -= number(500, 1000);
  af.type = SKILL_CLOUD;
  af.duration = 10;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_CLOUD;
  affect_to_char(vict, &af);
  return; 
}


ACMD(do_elemental_summon)
{
  struct char_data *vict, *elemental;
  int mob_num, hit_loss;

  one_argument(argument, arg);
  hit_loss = number(1500, 2000);

  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("Only titans possess the power to summon an elemental.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_SUMMON_ELEM)) {
    send_to_char("You must first learn the art of summoning elementals.\r\n", ch);
    return;
  }
  if (GET_HIT(ch) <= hit_loss) {
    send_to_char("You have not the strength to call an elemental.\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Who is it you wish to summon an elemental upon?\r\n", ch);
    return;
  }
  for (vict = character_list; vict; vict = vict->next) {
    if (CAN_SEE(ch, vict) && vict->in_room != NOWHERE &&
        isname(arg, vict->player.name)) {
      if (!IS_NPC(ch) && !pkill_ok(ch, vict) && !IS_REMORT(vict)) {
        send_to_char("You may not yet summon attacks against that person.\r\n", ch);
        return;
      }
      if (IS_AFFECTED(vict, AFF_UNDER_ATTACK)) {
        send_to_char("They already are hunted by an elemental.\r\n", ch);
        return;
      }
      mob_num = number(1245, 1248);
      elemental = read_mobile(mob_num, VIRTUAL);
      char_to_room(elemental, IN_ROOM(ch));
      HUNTING(elemental) = vict;
      GET_HIT(ch) -= hit_loss;
      act("\r\nYou conjure $N into being.\r\n"
          "It bows before you once before moving off to "
          "find its prey.\r\n", TRUE, ch, 0, elemental, TO_CHAR);
      act("$n conjures an elemental to $s bidding.\r\n"
          "It bows before him before moving off to hunt its prey.\r\n",
          TRUE, ch, 0, 0, TO_ROOM);
      SET_BIT_AR(AFF_FLAGS(vict), AFF_UNDER_ATTACK);
      return;
    }
    send_to_char("There is no one in Dibrova by that name.\r\n", ch);
    return;
  }
}

ACMD(do_describe)
{
  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (IS_NPC(ch))
    send_to_char("Your description is fine... go away.\r\n", ch);
  else if (strstr(argument, "(") || strstr(argument, ")"))
    send_to_char("Descriptions can't contain the ( or ) characters.\r\n", ch);
  else if (strlen(argument) > MAX_INPUT_LENGTH) {
    sprintf(buf, "Sorry, titles can't be longer than %d characters.\r\n",
            MAX_INPUT_LENGTH);
    send_to_char(buf, ch);
  } else {
    set_descrip(ch, argument);
    sprintf(buf, "Okay, your descrip is now %s.\r\n", ch->player.long_descr);
    send_to_char(buf, ch);
  } 

}

ACMD(do_deceive)
{
  struct affected_type af;
  byte percent;

  if (IS_AFFECTED(ch, AFF_DECEIVE)) {
    send_to_char("You remove the veil of deception from around yourself.\r\n", ch);
    affect_from_char(ch, SKILL_DECEIVE, ABT_SKILL);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */

  if (percent > GET_SKILL(ch, SKILL_DECEIVE)) {
    send_to_char("You attempt to disguise your identity, but fail.\r\n", ch);
    return;
  }

  send_to_char("You veil yourself in deception, disguising your true identity.\r\n", ch);
  af.type = SKILL_SNEAK;
  af.duration = GET_TIER(ch) * 2;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DECEIVE;
  affect_to_char(ch, &af);
}

ACMD(do_furl)
{
  if (GET_CLASS(ch) != CLASS_DEMON) {
    send_to_char("Only demons have the ability to furl their wings.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_BESTOW_WINGS)) {
    send_to_char("You must first learn to unfurl your wings.\r\n", ch);
    return;
  }
  if (!IS_AFFECTED(ch, AFF_UNFURLED)) {
    send_to_char("Your wings are already at rest, you have no need to furl them.\r\n", ch);
    return;
  }
  act("$n settels $s demonic wings neatly onto $s back.",
  FALSE, ch, 0, 0, TO_ROOM);          
  send_to_char("You furl your wings, settling them into place on your back.\r\n", ch);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_UNFURLED);
  return;
}

ACMD(do_unfurl)
{
  struct affected_type af;

  if (GET_CLASS(ch) != CLASS_DEMON) {
    send_to_char("Only demons have the ability to furl their wings.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_BESTOW_WINGS)) {
    send_to_char("You must first learn to unfurl your wings.\r\n", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_UNFURLED)) {
    send_to_char("Your wings are already unfurled!\r\n", ch);
    return;
  }
  act("You unfurl your wings, stretching them out to their full span.",
       FALSE, ch, 0, 0, TO_CHAR);
  act("$n unfurls $s demonic wings to there full span, shrouding you in darkness.",
       FALSE, ch, 0, 0, TO_ROOM);

  af.type = SKILL_BESTOW_WINGS;
  af.duration = GET_TIER(ch) * 5;
  af.modifier = 15;
  af.location = APPLY_HITROLL;
  af.bitvector = AFF_UNFURLED;
  affect_to_char(ch, &af);
}

ACMD(do_playtime)
{

  sprintf(buf, "You've played %d hrs and %d minutes.\r\n",
                ch->player.time.played / 3600,
               (ch->player.time.played / 3600) % 60);
  send_to_char(buf, ch);
  return;

}

ACMD(do_sorc_dam)
{
  int lvl, hits, sorc, drd_mnk, pal_dk, clr;

  send_to_char("Lvl      Hits     Sorc     Drd/Mnk  Pal/DK   Clr\r\n", ch);

  for (lvl = 1; lvl<151;lvl++) {
    if(lvl<=10)
     hits = lvl*10+1;
    else if(lvl<=25)
     hits=lvl*22;
    else if(lvl<=50)
     hits=lvl*30;
    else if(lvl<=75)
     hits=lvl*40;
    else if(lvl<=100)
     hits=lvl*45;
    else if(lvl<=125)
     hits=lvl*50;
    else
     hits=lvl*55;
 
    sorc = hits*.08;
    drd_mnk = hits*.06;
    pal_dk = hits*.05;
    clr = hits*.04;

    sprintf(buf, "%d %9d %9d %9d %9d %9d\r\n",
             lvl, hits, sorc, drd_mnk, pal_dk, clr);
    send_to_char(buf, ch);
  }
} 

ACMD(do_rub)
{
  int chance;

  if (!GET_SKILL(ch, SKILL_RUB)) {
    send_to_char("You haven't learned this skill yet.\r\n", ch);
    return;
  }

  if (!IS_AFFECTED(ch, AFF_BLIND)) {
    act("You rub at your eyes, making sure everything's clear.\r\n",
         FALSE, ch, 0, 0, TO_CHAR);
    act("$n rubs at $s eyes, clearing $s vision.\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    return;
  } else

  chance = number (1, 100);
 
  if (chance >= 51) {
    act("You rub at your eyes, clearing your vision\r\n"
        "You can see again!\r\n", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rubs at $s eyes, it appears $e can see again!\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_BLIND);
    look_at_room(ch, 0);
    WAIT_STATE(ch, 100);
    return;
  } else
  act("You rub at your eyes, but your vision does not clear.\r\n",
       FALSE, ch, 0, 0, TO_CHAR);
  act("$n rubs at $s eyes but still appears to be blinded!\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  WAIT_STATE(ch, 100);
  return;
}
