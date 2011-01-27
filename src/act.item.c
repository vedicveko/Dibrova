/**************************************************************************
*   File: act.item.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
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
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "dg_scripts.h"
#include "clan.h"

/* extern variables */
extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct index_data *obj_index;
extern char *drinks[];
extern int drink_aff[][3];
extern int port;
extern int don_rooms[];
extern int clan_don_rooms[];
void die(struct char_data * ch, struct char_data * killer);
extern struct zone_data *zone_table;
void claim_zone(struct char_data *ch, struct obj_data *obj);
extern int double_exp;

// Prototypes
int perform_remove(struct char_data *ch, int pos);
int can_get_corpse(struct char_data *ch, int num);
int check_get_corpse(struct char_data *ch, struct obj_data *obj);
void zedit_save_to_disk(int zone_num);
void colorless_str(char *str);
extern int real_zone(int number);
extern struct obj_data *object_list;
extern struct char_data *character_list;
bool perform_wear(struct char_data * ch, struct obj_data * obj, int where, int wearall);
void save_corpses(void);


int bid_num = 0;

void perform_put(struct char_data * ch, struct obj_data * obj,
		      struct obj_data * cont)
{
  if (!drop_otrigger(obj, ch))
    return;
  if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0) &&
      GET_OBJ_TYPE(cont) != ITEM_JAR) {
    act("$p won't fit in $P.", FALSE, ch, obj, cont, TO_CHAR);
    return;
  }
  if (IS_OBJ_STAT(obj, ITEM_NOTRANSFER) &&
     !IS_OBJ_STAT(cont, ITEM_NOTRANSFER) && GET_LEVEL(ch) < LVL_IMMORT) {
    act("You cannot put this particular item in $P.",
         FALSE, ch, obj, cont, TO_CHAR);
    return;
  }
  else {
    obj_from_char(obj);
    obj_to_obj(obj, cont);
    if (GET_OBJ_TYPE(cont) == ITEM_JAR) {
      act("You sprinkle $p into $P.", FALSE, ch, obj, cont, TO_CHAR);
      act("$n sprinkles $p into $P.", TRUE, ch, obj, cont, TO_ROOM);
    } else
    act("You put $p in $P.", FALSE, ch, obj, cont, TO_CHAR);
    act("$n puts $p in $P.", TRUE, ch, obj, cont, TO_ROOM);
  }
  save_corpses();

}



/* Defines for tradein */
#define LETTER_D		26193
#define LETTER_I		26194
#define LETTER_B		26195
#define LETTER_R		26196
#define LETTER_O		26197
#define LETTER_V		26198
#define LETTER_A		26199
#define LETTER_VALUE	50
// Clubs
#define CARD_CLUBS_ACE		26040
#define CARD_CLUBS_TWO		26041
#define CARD_CLUBS_THREE	26042
#define CARD_CLUBS_FOUR		26043
#define CARD_CLUBS_FIVE		26044
#define CARD_CLUBS_SIX		26045
#define CARD_CLUBS_SEVEN	26046
#define CARD_CLUBS_EIGHT	26047
#define CARD_CLUBS_NINE		26048
#define CARD_CLUBS_TEN		26049
#define CARD_CLUBS_JACK		26050
#define CARD_CLUBS_QUEEN	26051
#define CARD_CLUBS_KING		26052
// Spades
#define CARD_SPADES_ACE		26053
#define CARD_SPADES_TWO		26054
#define CARD_SPADES_THREE	26055
#define CARD_SPADES_FOUR	26056
#define CARD_SPADES_FIVE	26057
#define CARD_SPADES_SIX		26058
#define CARD_SPADES_SEVEN	26059
#define CARD_SPADES_EIGHT	26060
#define CARD_SPADES_NINE	26061
#define CARD_SPADES_TEN		26062
#define CARD_SPADES_JACK	26063
#define CARD_SPADES_QUEEN	26064
#define CARD_SPADES_KING	26065
// Hearts
#define CARD_HEARTS_ACE		26066
#define CARD_HEARTS_TWO		26067
#define CARD_HEARTS_THREE	26068
#define CARD_HEARTS_FOUR	26069
#define CARD_HEARTS_FIVE	26070
#define CARD_HEARTS_SIX		26071
#define CARD_HEARTS_SEVEN	26072
#define CARD_HEARTS_EIGHT	26073
#define CARD_HEARTS_NINE	26074
#define CARD_HEARTS_TEN		26075
#define CARD_HEARTS_JACK	26076
#define CARD_HEARTS_QUEEN	26077
#define CARD_HEARTS_KING	26078
// Diamonds
#define CARD_DIAMONDS_ACE		26079
#define CARD_DIAMONDS_TWO		26080
#define CARD_DIAMONDS_THREE		26081
#define CARD_DIAMONDS_FOUR		26082
#define CARD_DIAMONDS_FIVE		26083
#define CARD_DIAMONDS_SIX		26084
#define CARD_DIAMONDS_SEVEN		26085
#define CARD_DIAMONDS_EIGHT		26086
#define CARD_DIAMONDS_NINE		26087
#define CARD_DIAMONDS_TEN		26088
#define CARD_DIAMONDS_JACK		26089
#define CARD_DIAMONDS_QUEEN		26090
#define CARD_DIAMONDS_KING		26091
// Reward values for cards
#define CARD_VALUE_ONE_PAIR			15
#define CARD_VALUE_TWO_PAIR			25
#define CARD_VALUE_THREEOFAKIND		35
#define CARD_VALUE_STRAIGHT			50
#define CARD_VALUE_FLUSH			60
#define CARD_VALUE_FULLHOUSE		75
#define CARD_VALUE_FOUROFAKIND		125
#define CARD_VALUE_STRAIGHTFLUSH	250
// Total number of cards in a set.
#define CARD_TOTAL				52
// Quills
#define QUILL_VNUM				12
#define QUILL_VALUE				2
// put struct below in handler.c when this command is done as in accepting cards also.
struct obj_data *get_obj_in_list_vis_new(struct char_data *ch, int vnum, struct obj_data *list)
{
  struct obj_data *i;
      //if (i = read_data(vnum, VIRTUAL))
  for (i = list; i; i = i->next_content)
	  if (GET_OBJ_VNUM(i) == vnum)
	  return (i);

  return (NULL);
}

ACMD(do_exchange)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  struct obj_data *obj1=0, *obj2=0, *obj3=0, *obj4=0, *obj5=0, *obj6=0, *obj7=0, *i=0, *list;
  int number = 0;

  list = ch->carrying;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if (IS_NPC(ch))
	  send_to_char("Huh?!?\n\r", ch);
  else if (str_cmp(world[ch->in_room].name, "By The Temple Altar"))
	  send_to_char("Sorry, you cannot do that here.\r\n", ch);
  else if (arg1[0] == '\0') {
	  send_to_char("You can only exchange letters and quills.\r\n", ch);
	  send_to_char("Syntax: exchange <letters/quill>.\r\n", ch); }
  else if (!str_cmp(arg1, "quill")) {
	  for (i = list; i; i = i->next_content) {
		  if (!obj1)
			  if (GET_OBJ_VNUM(i) == QUILL_VNUM) {
				  obj1 = get_obj_in_list_vis_new(ch, QUILL_VNUM, ch->carrying);
				  number++;
		  }
	  }
	  if ((obj1 != 0) && (number == 1)) {
		  obj_from_char(obj1);
		  extract_obj(obj1);
		  send_to_char("The Questmaster looks at you.\r\n", ch);
		  sprintf(buf, "/cWThe Questmaster makes a slight gesture and /cw%s /cWflies away from you to him./c0\r\n", obj1->short_description);
		  send_to_char(buf, ch);
		  sprintf(buf, "The Questmaster gives you %d questpoints.\r\n", QUILL_VALUE);
		  send_to_char(buf, ch);
		  act("The Questmaster receives a Fiery Quill from $n", FALSE, ch, NULL, NULL, TO_NOTVICT);
		  act("The Questmaster gives a few questpoints to $n.\r\n", FALSE, ch, NULL, NULL, TO_NOTVICT);
		  GET_QPOINTS(ch) += QUILL_VALUE;
		  save_char(ch, NOWHERE);
		  sprintf(buf, "QUEST: %s has exchanged a Fiery Quill at the Questmaster.", GET_NAME(ch));
		  log(buf);
	  }
  } // end of quill
  else if (!str_cmp(arg1, "letters")) {
	  for (i = list; i; i = i->next_content) {
		  if (!obj1)
			  if (GET_OBJ_VNUM(i) == LETTER_D) {
				  obj1 = get_obj_in_list_vis_new(ch, LETTER_D, ch->carrying);
				  number++; }
		  if (!obj2)
			  if (GET_OBJ_VNUM(i) == LETTER_I) {
				  obj2 = get_obj_in_list_vis_new(ch, LETTER_I, ch->carrying);
				  number++; }
		  if (!obj3)
			  if (GET_OBJ_VNUM(i) == LETTER_B) {
				  obj3 = get_obj_in_list_vis_new(ch, LETTER_B, ch->carrying);
				  number++; }
		  if (!obj4)
			  if (GET_OBJ_VNUM(i) == LETTER_R) {
				  obj4 = get_obj_in_list_vis_new(ch, LETTER_R, ch->carrying);
				  number++; }
		  if (!obj5)
			  if (GET_OBJ_VNUM(i) == LETTER_O) {
				  obj5 = get_obj_in_list_vis_new(ch, LETTER_O, ch->carrying);
				  number++; }
		  if (!obj6)
			  if (GET_OBJ_VNUM(i) == LETTER_V) {
				  obj6 = get_obj_in_list_vis_new(ch, LETTER_V, ch->carrying);
				  number++; }
		  if (!obj7)
			  if (GET_OBJ_VNUM(i) == LETTER_A) {
				  obj7 = get_obj_in_list_vis_new(ch, LETTER_A, ch->carrying);
				  number++; }
	  } // end of loop
	  if (number == 7) {
			obj_from_char(obj1);
			obj_from_char(obj2);
			obj_from_char(obj3);
			obj_from_char(obj4);
			obj_from_char(obj5);
			obj_from_char(obj6);
			obj_from_char(obj7);
			extract_obj(obj1);
			extract_obj(obj2);
			extract_obj(obj3);
			extract_obj(obj4);
			extract_obj(obj5);
			extract_obj(obj6);
			extract_obj(obj7);
		  send_to_char("The Questmaster looks at you.\r\n", ch);
		  send_to_char("/cWThe Questmaster makes a slight gesture and /cwthe letters/cb D, I, B, R, O, V, A /cWflies away from you to him./c0\r\n", ch);
		  sprintf(buf, "The Questmaster gives you %d questpoints.\r\n", LETTER_VALUE);
		  send_to_char(buf, ch);
		  act("The Questmaster receives a full set of letters from $n", FALSE, ch, NULL, NULL, TO_NOTVICT);
		  act("The Questmaster gives a large amount of questpoints to $n.\r\n", FALSE, ch, NULL, NULL, TO_NOTVICT);
		  GET_QPOINTS(ch) += LETTER_VALUE;
		  save_char(ch, NOWHERE);
		  sprintf(buf, "QUEST: %s has exchanged a full set of letters at the Questmaster.", GET_NAME(ch));
		  log(buf);
	  } else {
		  send_to_char("You do not have a full set of letters!\r\n", ch); }
  }
  else if (!str_cmp(arg1, "qps"))
  {
	  if (GET_QPOINTS(ch) < 10)
		  send_to_char("You do not have enough gold.\r\n", ch);
	  else {
		  GET_GOLD(ch) += 100000;
		  GET_QPOINTS(ch) -= 10;
		  send_to_char("The Questmaster removes 10 questpoints from you in exchange for 100.000 gold coins.\r\n", ch);
		  return;
	  }
	  return;
 } else {
	  send_to_char("You can only exchange letters and quills.\r\n", ch);
	  send_to_char("Syntax: exchange <letters/quill>.\r\n", ch);
 }
} // end of do_tradein



/* The following put modes are supported by the code below:

	1) put <object> <container>
	2) put all.<object> <container>
	3) put all <container>

	<container> must be in inventory or on ground.
	all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj, *cont;
  struct char_data *tmp_char;
  int obj_dotmode, cont_dotmode, found = 0;

  two_arguments(argument, arg1, arg2);
  obj_dotmode = find_all_dots(arg1);
  cont_dotmode = find_all_dots(arg2);

  if (!*arg1)
    send_to_char("Put what in what?\r\n", ch);
  else if (cont_dotmode != FIND_INDIV)
    send_to_char("You can only put things into one container at a time.\r\n", ch);
  else if (!*arg2) {
    sprintf(buf, "What do you want to put %s in?\r\n",
	    ((obj_dotmode == FIND_INDIV) ? "it" : "them"));
    send_to_char(buf, ch);
  } else {
    generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
    if (!cont) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
    } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER)
      act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
    else if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
      send_to_char("You'd better open it first!\r\n", ch);
    else {
      if (obj_dotmode == FIND_INDIV) {	/* put <obj> <container> */
	if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	  sprintf(buf, "You aren't carrying %s %s.\r\n", AN(arg1), arg1);
	  send_to_char(buf, ch);
        } else if (IS_OBJ_STAT(obj, ITEM_NOTRANSFER) &&
                  !IS_OBJ_STAT(cont, ITEM_NOTRANSFER) &&
                   GET_LEVEL(ch) < LVL_IMMORT) {
          send_to_char("You cannot put that into anything.\r\n", ch);
	} else if (obj == cont)
	  send_to_char("You attempt to fold it into itself, but fail.\r\n", ch);
	else
	  perform_put(ch, obj, cont);
      } else {
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
	      (obj_dotmode == FIND_ALL || isname(arg1, obj->name))) {
	    found = 1;
	    perform_put(ch, obj, cont);
	  }
	}
	if (!found) {
	  if (obj_dotmode == FIND_ALL)
	    send_to_char("You don't seem to have anything to put in it.\r\n", ch);
	  else {
	    sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
	    send_to_char(buf, ch);
	  }
	}
      }
    }
  }
}

bool takable(struct obj_data *obj)
{

   if((GET_OBJ_TYPE(obj) == ITEM_DEED) || (GET_OBJ_TYPE(obj) == ITEM_MONEY) || (GET_OBJ_TYPE(obj) == ITEM_RUNE) || (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)) {
     return TRUE;
  } else {
    return FALSE;
  }
}


int can_take_obj(struct char_data * ch, struct obj_data * obj)
{
  int val;
  /* Added by Samhadi for do_assign_player stuff 5/3/00 */
  if ((val = check_get_corpse(ch, obj)) <= 0 && (GET_LEVEL(ch) < LVL_IMMORT) && (!takable(obj))) {
    if (val == 0)
      act("$p: you can't touch that corpse!", FALSE, ch, obj, 0, TO_CHAR);
    else if (val == -1)
      act("$p: you can't touch that corpse!", FALSE, ch, obj->in_obj, 0, TO_CHAR);
    return val;
  }

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
    act("$p: you can't carry that many items.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    act("$p: you can't carry that much weight.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
    act("$p: you can't take that!", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  } else if (obj->obj_flags.player != GET_IDNUM(ch)) {
    if (obj->obj_flags.player != -1) {
    send_to_char("This is owned eq and you may not touch it.\r\n", ch);
    return 0;
    }
  }
  return 1;
}

void get_check_money(struct char_data * ch, struct obj_data * obj)
{
  if ((GET_OBJ_TYPE(obj) == ITEM_MONEY) && (GET_OBJ_VAL(obj, 0) > 0)) {
    obj_from_char(obj);
    if (GET_OBJ_VAL(obj, 0) > 1) {
      sprintf(buf, "There were %d coins.\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
    }
    GET_GOLD(ch) += GET_OBJ_VAL(obj, 0);
    extract_obj(obj);
  }
}


void perform_get_from_container(struct char_data * ch, struct obj_data * obj,
                                struct obj_data * cont, int mode, int subcmd)
{

  if (mode == FIND_OBJ_INV || (can_take_obj(ch, obj) > 0)) {
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      act("$p: you can't hold any more items.", FALSE, ch, obj, 0, TO_CHAR);
    if (GET_OBJ_TYPE(cont) == ITEM_JAR && GET_OBJ_VAL(cont, 3) > 0)
      act("The contents of $p have already been mixed, they can't be removed.",
           FALSE, ch, cont, 0, TO_CHAR);
    else if (get_otrigger(obj, ch)) {
      obj_from_obj(obj);
      obj_to_char(obj, ch);
      if (subcmd != SCMD_SLEIGHT) {
        act("You get $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
        act("$n gets $p from $P.", TRUE, ch, obj, cont, TO_ROOM);

      } else
        act("You snatch $p from $P.", FALSE, ch, obj, cont, TO_CHAR);
      get_check_money(ch, obj);
    }
  }
  save_corpses();

}


void get_from_container(struct char_data * ch, struct obj_data * cont,
			     char *arg, int mode, int subcmd)
{
  struct obj_data *obj, *next_obj;
  int obj_dotmode, found = 0;

  obj_dotmode = find_all_dots(arg);

/*  if (GET_OBJ_VAL(cont, 3) == 1) {
    if (cont->obj_flags.pid && GET_PFILEPOS(ch) != cont->obj_flags.pid) {
      send_to_char("You can't touch that corpse.\r\n", ch);
      return;
    }
  }*/


  /* Following code restricts pc corpse looting to pc owner, imps, pkiller and unjarred mobs */
/*  if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || (ch->desc)) {
    sprintf(buf, "The corpse of %s is lying here.", GET_NAME(ch));
    if (IS_SET(GET_OBJ_VAL(cont, 2), -2)
        && GET_OBJ_RNUM(cont) == -1
        && strcmp(buf, cont->description)
        && strcmp(GET_NAME(ch), GET_NAME(GET_PKLOOTER(cont)))
        && GET_LEVEL(ch) < LVL_IMPL) {
      send_to_char("You low-down grave robber!\r\n", ch);
      send_to_char("Stealing from the dead is shameful!\r\n", ch);
      act("$n shamelessly tries to loot $p!", TRUE, ch, cont, 0, TO_ROOM);
      return;
    }
  } */

  if (IS_SET(GET_OBJ_VAL(cont, 1), CONT_CLOSED))
    act("$p is closed.", FALSE, ch, cont, 0, TO_CHAR);
  else if (obj_dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains))) {
      sprintf(buf, "There doesn't seem to be %s %s in $p.", AN(arg), arg);
      act(buf, FALSE, ch, cont, 0, TO_CHAR);
    } else if (IS_OBJ_STAT(obj, ITEM_NOTRANSFER) &&
              (!isname(GET_NAME(ch), cont->name)) &&
               GET_LEVEL(ch) < LVL_IMMORT)
      act("You cannot take this particular item.", FALSE, ch, 0, 0, TO_CHAR);
    else if (obj->obj_flags.player != -1 && obj->obj_flags.player != GET_IDNUM(ch)) {
        act("$p is owned eq - you cannot take it.", FALSE, ch, obj, 0, TO_CHAR);
    } else
      perform_get_from_container(ch, obj, cont, mode, subcmd);
  } else {
    if (obj_dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = cont->contains; obj; obj = next_obj) {
      next_obj = obj->next_content;
     if (CAN_SEE_OBJ(ch, obj) &&
	  (obj_dotmode == FIND_ALL || isname(arg, obj->name)) &&
          (obj->obj_flags.player == -1 ||
           obj->obj_flags.player == GET_IDNUM(ch))) {
	found = 1;
	perform_get_from_container(ch, obj, cont, mode, subcmd);
      }
    }
    if (!found) {
      if (obj_dotmode == FIND_ALL)
	act("$p seems to be empty.", FALSE, ch, cont, 0, TO_CHAR);
      else {
	sprintf(buf, "You can't seem to find any %ss in $p.", arg);
	act(buf, FALSE, ch, cont, 0, TO_CHAR);
      }
    }
  }
}


int perform_get_from_room(struct char_data * ch, struct obj_data * obj, int subcmd)
{

  if (GET_MOUNT(ch)) {
    act("You'll have to dismount in order to get $p.", FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  }
  if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_VAULT)) &&
     (GET_OBJ_TYPE(obj) == ITEM_DEED) &&
     (zone_table[world[IN_ROOM(ch)].zone].owner == (GET_CLAN(ch) -1))) {
     send_to_char("Sorry, that deed has been vaulted.\r\n", ch);
     return 0;
  }
  if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_VAULT)) &&
     (GET_OBJ_TYPE(obj) == ITEM_DEED) &&
     (zone_table[world[IN_ROOM(ch)].zone].owner != GET_CLAN(ch))) {
     sprintf(buf, "/cRCLAN: %s has looted %s!/c0\r\n",
             GET_NAME(ch), obj->short_description);
     SET_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);
     send_to_all(buf);
     zone_table[GET_OBJ_VAL(obj, 0)].owner = -1;
  }
  if (can_take_obj(ch, obj) && get_otrigger(obj, ch)) {
    obj_from_room(obj);
    obj_to_char(obj, ch);
    if (subcmd != SCMD_SLEIGHT) {
      act("You get $p.", FALSE, ch, obj, 0, TO_CHAR);
      act("$n gets $p.", TRUE, ch, obj, 0, TO_ROOM);
      save_corpses();
    } else
      act("You snatch $p.", FALSE, ch, obj, 0, TO_CHAR);
      get_check_money(ch, obj);
      save_corpses();
      return 1;
  }
  return 0;
}


void get_from_room(struct char_data * ch, char *arg, int subcmd)
{
  struct obj_data *obj, *next_obj;
  int dotmode, found = 0;

  dotmode = find_all_dots(arg);


  if (dotmode == FIND_INDIV) {
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
      return;
    }

 if (GET_OBJ_VAL(obj, 3) == 1) {
    if (obj->obj_flags.pid && GET_PFILEPOS(ch) != obj->obj_flags.pid) {
      send_to_char("You can't touch that corpse.\r\n", ch);
      return;
    }
 }

  if (GET_OBJ_VAL(obj, 3) == 2 && GET_CLASS(ch) < CLASS_VAMPIRE) {
    if (obj->obj_flags.pid && GET_PFILEPOS(ch) != obj->obj_flags.pid) {
      send_to_char("You can't touch that corpse.\r\n", ch);
      return;
    }
  }

  if (IS_OBJ_STAT(obj, ITEM_NOTRANSFER) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("You cannot take that item.", ch);
    return;
  }

      perform_get_from_room(ch, obj, subcmd);

      /*else {
      if (GET_OBJ_VAL(obj, 3) == 1) {
        if (obj->obj_flags.pid && GET_PFILEPOS(ch) != obj->obj_flags.pid) {
          send_to_char("You can't touch that corpse.\r\n", ch);
          return;
        }
      }
      perform_get_from_room(ch, obj, subcmd);
    }*/
  } else {
    if (dotmode == FIND_ALLDOT && !*arg) {
      send_to_char("Get all of what?\r\n", ch);
      return;
    }
    for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) &&
	  (dotmode == FIND_ALL || isname(arg, obj->name))) {
	found = 1;
	perform_get_from_room(ch, obj, subcmd);
      }
    }
    if (!found) {
      if (dotmode == FIND_ALL)
	send_to_char("There doesn't seem to be anything here.\r\n", ch);
      else {
	sprintf(buf, "You don't see any %ss here.\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  }
}



ACMD(do_get)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  int cont_dotmode, found = 0, mode;
  struct obj_data *cont;
  struct char_data *tmp_char;

  two_arguments(argument, arg1, arg2);

  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
    send_to_char("Your arms are already full!\r\n", ch);
  else if (!*arg1)
    send_to_char("Get what?\r\n", ch);
  else if (!*arg2)
    get_from_room(ch, arg1, subcmd);
  else {
    cont_dotmode = find_all_dots(arg2);
    if (cont_dotmode == FIND_INDIV) {
      mode = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &cont);
      if (!cont) {
	sprintf(buf, "You don't have %s %s.\r\n", AN(arg2), arg2);
	send_to_char(buf, ch);
      } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER &&
                 GET_OBJ_TYPE(cont) != ITEM_JAR)
	act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
      else if ((mode == FIND_OBJ_ROOM) && GET_MOUNT(ch))
        act("You'll have to dismount before you can get anything from $p.", FALSE, ch, cont, 0, TO_CHAR);
      else
	get_from_container(ch, cont, arg1, mode, subcmd);
    } else {
      if (cont_dotmode == FIND_ALLDOT && !*arg2) {
	send_to_char("Get from all of what?\r\n", ch);
	return;
      }
      for (cont = ch->carrying; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
	    found = 1;
	    get_from_container(ch, cont, arg1, FIND_OBJ_INV, subcmd);
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    found = 1;
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	  }
        }
      for (cont = world[ch->in_room].contents; cont; cont = cont->next_content)
	if (CAN_SEE_OBJ(ch, cont) &&
	    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))) {
	  if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER) {
            if (GET_MOUNT(ch))
              send_to_char("You can't reach it! You'll have to dismount.\r\n", ch);
            else {
              get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, subcmd);
	      found = 1;
            }
	  } else if (cont_dotmode == FIND_ALLDOT) {
	    act("$p is not a container.", FALSE, ch, cont, 0, TO_CHAR);
	    found = 1;
	  }
        }
      if (!found) {
	if (cont_dotmode == FIND_ALL)
	  send_to_char("You can't seem to find any containers.\r\n", ch);
	else {
	  sprintf(buf, "You can't seem to find any %ss here.\r\n", arg2);
	  send_to_char(buf, ch);
	}
      }
    }
  }
}


void perform_drop_gold(struct char_data * ch, int amount,
		            byte mode, sh_int RDR)
{
  struct obj_data *obj;

  if (amount <= 0)
    send_to_char("Heh heh heh.. we are jolly funny today, eh?\r\n", ch);
  else if (GET_GOLD(ch) < amount)
    send_to_char("You don't have that many coins!\r\n", ch);
  else {
    if (mode != SCMD_JUNK) {
      WAIT_STATE(ch, PULSE_VIOLENCE);	/* to prevent coin-bombing */
      obj = create_money(amount);
      if (mode == SCMD_DONATE) {
	send_to_char("You throw some gold into the air where it disappears in a puff of smoke!\r\n", ch);
	act("$n throws some gold into the air where it disappears in a puff of smoke!",
	    FALSE, ch, 0, 0, TO_ROOM);
	obj_to_room(obj, RDR);
	act("$p suddenly appears in a puff of orange smoke!", 0, 0, obj, 0, TO_ROOM);
      } else {
        if (!drop_wtrigger(obj, ch)) {
          extract_obj(obj);
          return;
        }
	send_to_char("You drop some gold.\r\n", ch);
	sprintf(buf, "$n drops %s.", money_desc(amount));
	act(buf, TRUE, ch, 0, 0, TO_ROOM);
        obj->obj_flags.player = -1;
	obj_to_room(obj, ch->in_room);
      }
    } else {
      sprintf(buf, "$n drops %s which disappears in a puff of smoke!",
	      money_desc(amount));
      act(buf, FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("You drop some gold which disappears in a puff of smoke!\r\n", ch);
    }
    GET_GOLD(ch) -= amount;
  }
}


#define VANISH(mode) ((mode == SCMD_DONATE || mode == SCMD_JUNK) ? \
		      "  It vanishes in a puff of smoke!" : "")

int perform_drop(struct char_data * ch, struct obj_data * obj,
		     byte mode, char *sname, sh_int RDR)
{
  int value;

  if (!drop_otrigger(obj, ch))
    return 0;
  if ((mode == SCMD_DROP) && !drop_wtrigger(obj, ch))
    return 0;

  /* (MJ) */
  if (GET_OBJ_TYPE(obj) == ITEM_KEY) {
    extract_obj(obj);
    sprintf(buf, "%s vanishes in a puff of smoke.\r\n", obj->short_description);
    send_to_char(CAP(buf), ch);
    act("$n drops a key and it vanishes in a puff of smoke!", TRUE, ch, 0, 0, TO_ROOM);
    return 0;
  }

  /* (END MJ) */
  else /* Can't junk containers now. -Sam */
  if((GET_OBJ_TYPE(obj) == ITEM_CONTAINER &&
     !IS_OBJ_STAT(obj, ITEM_NOTRANSFER)) && (mode == SCMD_JUNK)){
    sprintf(buf, "You don't really want to junk %s now, do you?\r\n", obj->
      short_description);
    send_to_char(buf, ch);
    return 0;
  }
  else /* Or donate containers. -Sam */
  if((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (mode == SCMD_DONATE)){
    sprintf(buf, "Sorry, you cannot donate containers.\r\nIf you really want to donate %s, then go to the donation room and drop it off.\r\n", obj->
     short_description);
    send_to_char(buf, ch);
    return 0;
  }
  else
  if ((IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOTRANSFER)) &&
       GET_LEVEL(ch) < LVL_IMMORT) {
    sprintf(buf, "You can't %s $p, it must be CURSED!", sname);
    act(buf, FALSE, ch, obj, 0, TO_CHAR);
    return 0;
  }
  sprintf(buf, "You %s $p.%s", sname, VANISH(mode));
  act(buf, FALSE, ch, obj, 0, TO_CHAR);
  sprintf(buf, "$n %ss $p.%s", sname, VANISH(mode));
  act(buf, TRUE, ch, obj, 0, TO_ROOM);
  obj_from_char(obj);

  if ((mode == SCMD_DONATE) && IS_OBJ_STAT(obj, ITEM_NODONATE))
    mode = SCMD_JUNK;

  switch (mode) {
  case SCMD_DROP:
	  if((GET_OBJ_TYPE(obj) == ITEM_DEED) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_VAULT)))
		  claim_zone(ch, obj);
	  obj_to_room(obj, ch->in_room);
	  return 0;
	  break;
  case SCMD_DONATE:
    obj_to_room(obj, RDR);
    act("$p suddenly appears in a puff a smoke!", FALSE, 0, obj, 0, TO_ROOM);
    return 0;
    break;
  case SCMD_JUNK:
    value = number(1, 5);
//    value = MAX(1, MIN(200, GET_OBJ_COST(obj) >> 4));
    extract_obj(obj);
    return value;
    break;
  default:
    log("SYSERR: Incorrect argument passed to perform_drop");
    break;
  }

  return 0;
}



ACMD(do_drop)
{
#if 0
  extern int donation_room_1;
  extern int donation_room_2;  /* uncomment if needed! */
  extern int donation_room_3;  /* uncomment if needed! */
#endif
  struct obj_data *obj, *next_obj;
  sh_int RDR = 0;
  byte mode = SCMD_DROP;
  int dotmode, amount = 0;
  char *sname;

  switch (subcmd) {
  case SCMD_JUNK:
    sname = "junk";
    mode = SCMD_JUNK;
    break;
  case SCMD_DONATE:
    sname = "donate";
    mode = SCMD_DONATE;
    switch (number(0, 2)) {
    case 0:
      mode = SCMD_JUNK;
      break;
    case 1:
    case 2:
      if (find_clan_by_id(GET_CLAN(ch)) >= 0)
      RDR = real_room(clan_don_rooms[GET_CLAN(ch)]);
      else
      RDR = real_room(don_rooms[GET_HOME(ch)]);
      break;
    }
    if (RDR == NOWHERE) {
      send_to_char("Sorry, you can't donate anything right now.\r\n", ch);
      return;
    }
    break;
  default:
    sname = "drop";
    break;
  }

  argument = one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "What do you want to %s?\r\n", sname);
    send_to_char(buf, ch);
    return;
  } else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg))
      perform_drop_gold(ch, amount, mode, RDR);
    else {
      /* code to drop multiple items.  anyone want to write it? -je */
      send_to_char("Sorry, you can't do that to more than one item at a time.\r\n", ch);
    }
    return;
  } else {
    dotmode = find_all_dots(arg);

    /* Can't junk or donate all */
    if ((dotmode == FIND_ALL) && (subcmd == SCMD_JUNK || subcmd == SCMD_DONATE)) {
      if (subcmd == SCMD_JUNK)
	send_to_char("Go to the dump if you want to junk EVERYTHING!\r\n", ch);
      else
	send_to_char("Go do the donation room if you want to donate EVERYTHING!\r\n", ch);
      return;
    }
    if (dotmode == FIND_ALL) {
      if (!ch->carrying)
	send_to_char("You don't seem to be carrying anything.\r\n", ch);
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  amount += perform_drop(ch, obj, mode, sname, RDR);
	}
    } else if (dotmode == FIND_ALLDOT) {
      if (!*arg) {
	sprintf(buf, "What do you want to %s all of?\r\n", sname);
	send_to_char(buf, ch);
	return;
      }
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have any %ss.\r\n", arg);
	send_to_char(buf, ch);
      }
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
	amount += perform_drop(ch, obj, mode, sname, RDR);
	obj = next_obj;
      }
    } else {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf, ch);
      } else
	amount += perform_drop(ch, obj, mode, sname, RDR);
    }
  }

  if (amount && (subcmd == SCMD_JUNK)) {
    send_to_char("You have been rewarded by the gods!\r\n", ch);
    act("$n has been rewarded by the gods!", TRUE, ch, 0, 0, TO_ROOM);
    GET_GOLD(ch) += amount;
  }
}


void perform_give(struct char_data * ch, struct char_data * vict,
		       struct obj_data * obj)
{
  if ((IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOTRANSFER)) &&
      GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch)) {
    act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0, TO_CHAR);
    return;
  }
  if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
    act("$N seems to have $S hands full.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }

  if (obj->obj_flags.player != GET_IDNUM(vict)) {
    if (obj->obj_flags.player != -1) {
      send_to_char("You cannot give owned eq to anyone.\r\n", ch);
      return;
    }
  }


  if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) > CAN_CARRY_W(vict)) {
    act("$E can't carry that much weight.", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  if (!give_otrigger(obj, ch, vict) || !receive_mtrigger(vict, ch, obj))
    return;

  obj_from_char(obj);
  obj_to_char(obj, vict);
  act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
  act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
  act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);

}

/* utility function for give */
struct char_data *give_find_vict(struct char_data * ch, char *arg)
{
  struct char_data *vict;

  if (!*arg) {
    send_to_char("To who?\r\n", ch);
    return NULL;
  } else if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return NULL;
  } else if (vict == ch) {
    send_to_char("What's the point of that?\r\n", ch);
    return NULL;
  } else
    return vict;
}


void perform_give_gold(struct char_data * ch, struct char_data * vict,
		            int amount)
{
  if (amount <= 0) {
    send_to_char("Heh heh heh ... we are jolly funny today, eh?\r\n", ch);
    return;
  }
  if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_ADMIN))) {
    send_to_char("You don't have that many coins!\r\n", ch);
    return;
  }
  send_to_char(OK, ch);
  sprintf(buf, "$n gives you %d gold coins.", amount);
  act(buf, FALSE, ch, 0, vict, TO_VICT);
  sprintf(buf, "$n gives %s to $N.", money_desc(amount));
  act(buf, TRUE, ch, 0, vict, TO_NOTVICT);
  if(IS_NPC(ch)) {
    sprintf(buf, "CHEAT: %s gives %d gold to %s/c0", GET_NAME(ch), amount, GET_NAME(vict));
    log(buf);
}

  if (IS_NPC(ch) || (GET_LEVEL(ch) < LVL_ADMIN))
    GET_GOLD(ch) -= amount;
  GET_GOLD(vict) += amount;

  bribe_mtrigger(vict, ch, amount);
}


ACMD(do_give)
{
  int amount, dotmode;
  struct char_data *vict;
  struct obj_data *obj, *next_obj;

  argument = one_argument(argument, arg);

  if (!*arg)
    send_to_char("Give what to who?\r\n", ch);
  else if (is_number(arg)) {
    amount = atoi(arg);
    argument = one_argument(argument, arg);
    if (!str_cmp("coins", arg) || !str_cmp("coin", arg)) {
      argument = one_argument(argument, arg);
      if ((vict = give_find_vict(ch, arg)))
	perform_give_gold(ch, vict, amount);
      return;
    } else {
      /* code to give multiple items.  anyone want to write it? -je */
      send_to_char("You can't give more than one item at a time.\r\n", ch);
      return;
    }
  } else {
    one_argument(argument, buf1);
    if (!(vict = give_find_vict(ch, buf1)))
      return;
    dotmode = find_all_dots(arg);
    if (dotmode == FIND_INDIV) {
      if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
	send_to_char(buf, ch);
      } else
	perform_give(ch, vict, obj);
    } else {
      if (dotmode == FIND_ALLDOT && !*arg) {
	send_to_char("All of what?\r\n", ch);
	return;
      }
      if (!ch->carrying)
	send_to_char("You don't seem to be holding anything.\r\n", ch);
      else
	for (obj = ch->carrying; obj; obj = next_obj) {
	  next_obj = obj->next_content;
	  if (CAN_SEE_OBJ(ch, obj) &&
	      ((dotmode == FIND_ALL || isname(arg, obj->name))))
	    perform_give(ch, vict, obj);
	}
    }
  }
}



void weight_change_object(struct obj_data * obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if (obj->in_room != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if ((tmp_ch = obj->carried_by)) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if ((tmp_obj = obj->in_obj)) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("SYSERR: Unknown attempt to subtract weight from an object.");
  }
}



void name_from_drinkcon(struct obj_data * obj)
{
  int i;
  char *new_name;
  extern struct obj_data *obj_proto;

  for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++);

  if (*((obj->name) + i) == ' ') {
    new_name = str_dup((obj->name) + i + 1);
    if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
      free(obj->name);
    obj->name = new_name;
  }
}



void name_to_drinkcon(struct obj_data * obj, int type)
{
  char *new_name;
  extern struct obj_data *obj_proto;
  extern char *drinknames[];

  CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);
  sprintf(new_name, "%s %s", drinknames[type], obj->name);
  if (GET_OBJ_RNUM(obj) < 0 || obj->name != obj_proto[GET_OBJ_RNUM(obj)].name)
    free(obj->name);
  obj->name = new_name;
}



ACMD(do_drink)
{
  struct obj_data *temp;
  struct affected_type af;
  int amount, weight;
  int on_ground = 0;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Drink from what?\r\n", ch);
    return;
  }
  if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    if (!(temp = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else
      on_ground = 1;
  }
  if ((GET_OBJ_TYPE(temp) != ITEM_DRINKCON) &&
      (GET_OBJ_TYPE(temp) != ITEM_FOUNTAIN)) {
    send_to_char("You can't drink from that!\r\n", ch);
    return;
  }
  if (on_ground && (GET_OBJ_TYPE(temp) == ITEM_DRINKCON)) {
    send_to_char("You have to be holding that to drink from it.\r\n", ch);
    return;
  }

  if (on_ground && GET_MOUNT(ch)) {
    act("You'll have to dismount before you can drink from $p.", FALSE, ch, temp, 0, TO_CHAR);
    return;
  }

  if ((GET_COND(ch, DRUNK) > 10) && (GET_COND(ch, THIRST) > 0)) {
    /* The pig is drunk */
    send_to_char("You can't seem to get close enough to your mouth.\r\n", ch);
    act("$n tries to drink but misses $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  if ((GET_COND(ch, FULL) > 20) && (GET_COND(ch, THIRST) > 0)) {
    send_to_char("Your stomach can't contain anymore!\r\n", ch);
    return;
  }
  if (!GET_OBJ_VAL(temp, 1)) {
    send_to_char("It's empty.\r\n", ch);
    return;
  }
  if (subcmd == SCMD_DRINK) {
    sprintf(buf, "$n drinks %s from $p.", drinks[GET_OBJ_VAL(temp, 2)]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM);

    sprintf(buf, "You drink the %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);

    if (drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] > 0)
      amount = (25 - GET_COND(ch, THIRST)) / drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK];
    else
      amount = number(3, 10);

  } else {
    act("$n sips from $p.", TRUE, ch, temp, 0, TO_ROOM);
    sprintf(buf, "It tastes like %s.\r\n", drinks[GET_OBJ_VAL(temp, 2)]);
    send_to_char(buf, ch);
    amount = 1;
  }

  amount = MIN(amount, GET_OBJ_VAL(temp, 1));

  /* You can't subtract more than the object weighs */
  weight = MIN(amount, GET_OBJ_WEIGHT(temp));

  weight_change_object(temp, -weight);	/* Subtract amount */

  gain_condition(ch, DRUNK,
	 (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][DRUNK] * amount) / 4);

  gain_condition(ch, FULL,
	  (int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][FULL] * amount) / 4);

  gain_condition(ch, THIRST,
	(int) ((int) drink_aff[GET_OBJ_VAL(temp, 2)][THIRST] * amount) / 4);

  if (GET_COND(ch, DRUNK) > 10)
    send_to_char("You feel drunk.\r\n", ch);

  if (GET_COND(ch, THIRST) > 20)
    send_to_char("You don't feel thirsty any more.\r\n", ch);

  if (GET_COND(ch, FULL) > 20)
    send_to_char("You are full.\r\n", ch);

  /* specialty drinks */
  if (GET_OBJ_VAL(temp, 2) == LIQ_HEALTH) {
    send_to_char("You feel a rush of renewed health!\r\n", ch);
    GET_HIT(ch) = MIN((GET_HIT(ch) + number(10, 20)), GET_MAX_HIT(ch));
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_MANA) {
    send_to_char("You feel a surge in your arcane powers!\r\n", ch);
    GET_MANA(ch) = MIN((GET_MANA(ch) + number(10, 20)), GET_MAX_MANA(ch));
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_QI) {
    send_to_char("You feel a refocus of your qi!\r\n", ch);
    GET_QI(ch) = MIN((GET_QI(ch) + number(10, 20)), GET_MAX_QI(ch));
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_MOVE) {
    send_to_char("You feel a rush of renewed energy!\r\n", ch);
    GET_MOVE(ch) = MIN((GET_MOVE(ch) + number(10, 20)), GET_MAX_MOVE(ch));
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_VIM) {
    send_to_char("You feel a rush of renewed vigor!\r\n", ch);
    GET_VIM(ch) = MIN((GET_VIM(ch) + number(10, 20)), GET_MAX_VIM(ch));
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_GOODALIGN) {
    send_to_char("You feel an intense purity as you take a drink.\r\n", ch);
    GET_ALIGNMENT(ch) = MAX((GET_ALIGNMENT(ch) + number(50, 75)), 1000);
    amount = 3;
  }
  if (GET_OBJ_VAL(temp, 2) == LIQ_IMMORTALITY) {
    if (GET_CLASS(ch) >= CLASS_ASSASSIN && GET_LEVEL(ch) == 150) {
      send_to_char("\r\nYou drink the contents of the chalice, draining it\r\n"
                   "completely. You feel an immediate change in your body.\r\n"
                  "\r\nYour body swells and contorts, growing to unbelievable\r\n"
                   "proportions. The Chalice of the Titans has renedered you\r\n"
                   "immortal.\r\n", ch);
      act("$n drains the contents of the chalice, $s body swelling and\r\n"
          "contorting as $e swells to unbelievable proportions.\r\n\r\n"
          "The Chalice has rendered $n immortal. A titan has entered the world.",
           TRUE, ch, 0, 0, TO_ROOM);
      GET_CLASS(ch) = CLASS_TITAN;
      GET_RACE(ch) = RACE_GIANT;
      GET_TIER(ch) = 1;
      GET_LEVEL(ch) = 1;
      GET_EXP(ch) = 1;
      SET_SKILL(ch, SKILL_SECOND_ATTACK, 95);
      SET_SKILL(ch, SKILL_THIRD_ATTACK, 95);
      SET_SKILL(ch, SKILL_FOURTH_ATTACK, 95);
      SET_SKILL(ch, SKILL_FIFTH_ATTACK, 95);
      SET_SKILL(ch, SKILL_SIXTH_ATTACK, 95);
      SET_SKILL(ch, SKILL_HARNESS_WIND, 95);
      SET_SPELL(ch, SPELL_GROWTH, 95);
      affect_total(ch);
      double_exp = 20;
	  sprintf(buf, "/cCThe Chalice has renedered %s immortal. A titan has entered the world./c0\r\n", GET_NAME(ch));
	  sprintf(buf, "%s/cRDouble Experience is declared for the next 20 minutes, \r\nin honour of %s./c0\r\n", buf, GET_NAME(ch));
	  send_to_all(buf);
    } else
  send_to_char("You drink the contents of the chalice.\r\n"
               "Nothing happens.\r\n", ch);
  amount = 3;
  }

  if (GET_OBJ_VAL(temp, 3)) {	/* The shit was poisoned ! */
    send_to_char("Oops, it tasted rather strange!\r\n", ch);
    act("$n chokes and utters some strange sounds.", TRUE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  /* empty the container, and no longer poison. */
  if (GET_OBJ_VAL(temp, 1) <= amount) {
    send_to_char("You drink the last of it.\r\n", ch);
    GET_OBJ_VAL(temp, 1) = 0;
  } else
  GET_OBJ_VAL(temp, 1) -= amount;
  if (GET_OBJ_VAL(temp, 1) <= 0) {	/* The last bit */
    GET_OBJ_VAL(temp, 2) = 0;
    GET_OBJ_VAL(temp, 3) = 0;
    name_from_drinkcon(temp);
  }
  if (FIGHTING(ch))
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  return;
}



ACMD(do_eat)
{
  struct obj_data *food;
  struct affected_type af;
  int amount;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Eat what?\r\n", ch);
    return;
  }
  if (!(food = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return;
  }
  if (subcmd == SCMD_TASTE && ((GET_OBJ_TYPE(food) == ITEM_DRINKCON) ||
			       (GET_OBJ_TYPE(food) == ITEM_FOUNTAIN))) {
    do_drink(ch, argument, 0, SCMD_SIP);
    return;
  }
  if ((GET_OBJ_TYPE(food) != ITEM_FOOD) && (GET_LEVEL(ch) < LVL_ADMIN)) {
    send_to_char("You can't eat THAT!\r\n", ch);
    return;
  }
  if (GET_COND(ch, FULL) > 20) {/* Stomach full */
    act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_EAT) {
    act("You eat the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n eats $p.", TRUE, ch, food, 0, TO_ROOM);
  } else {
    act("You nibble a little bit of the $o.", FALSE, ch, food, 0, TO_CHAR);
    act("$n tastes a little bit of $p.", TRUE, ch, food, 0, TO_ROOM);
  }

  amount = (subcmd == SCMD_EAT ? GET_OBJ_VAL(food, 0) : 1);

  gain_condition(ch, FULL, amount);

  if (GET_COND(ch, FULL) > 20)
    act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

  if (GET_OBJ_VAL(food, 3) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    /* The shit was poisoned ! */
    send_to_char("Oops, that tasted rather strange!\r\n", ch);
    act("$n coughs and utters some strange sounds.", FALSE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_POISON;
    af.duration = amount * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_join(ch, &af, FALSE, FALSE, FALSE, FALSE);
  }
  if (subcmd == SCMD_EAT)
    extract_obj(food);
  else {
    if (!(--GET_OBJ_VAL(food, 0))) {
      send_to_char("There's nothing left now.\r\n", ch);
      extract_obj(food);
    }
  }
  if (FIGHTING(ch))
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}


ACMD(do_pour)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *from_obj = NULL, *to_obj = NULL;
  int amount;

  two_arguments(argument, arg1, arg2);

  if (subcmd == SCMD_POUR) {
    if (!*arg1) {		/* No arguments */
      act("From what do you want to pour?", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
      act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (subcmd == SCMD_FILL) {
    if (!*arg1) {		/* no arguments */
      send_to_char("What do you want to fill?  And what are you filling it from?\r\n", ch);
      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      send_to_char("You can't find it!", ch);
      return;
    }
    if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
      act("You can't fill $p!", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!*arg2) {		/* no 2nd argument */
      act("What do you want to fill $p from?", FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }
    if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
      sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg2), arg2);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
      act("You can't fill something from $p.", FALSE, ch, from_obj, 0, TO_CHAR);
      return;
    }
    if (GET_OBJ_VAL(from_obj, 2) >= 16) {
      act("You cannot seem to get the liquid into your $p.\r\n",
           FALSE, ch, to_obj, 0, TO_CHAR);
      return;
    }

  }
  if (GET_OBJ_VAL(from_obj, 1) == 0) {
    act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {	/* pour */
    if (!*arg2) {
      act("Where do you want it?  Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (!str_cmp(arg2, "out")) {
      act("$n empties $p.", TRUE, ch, from_obj, 0, TO_ROOM);
      act("You empty $p.", FALSE, ch, from_obj, 0, TO_CHAR);

      weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); /* Empty */

      GET_OBJ_VAL(from_obj, 1) = 0;
      GET_OBJ_VAL(from_obj, 2) = 0;
      GET_OBJ_VAL(from_obj, 3) = 0;
      name_from_drinkcon(from_obj);

      return;
    }
    if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
      act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) &&
	(GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)) {
      act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }
  if (to_obj == from_obj) {
    act("A most unproductive effort.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if ((GET_OBJ_VAL(to_obj, 1) != 0) &&
      (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
    act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
    act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
    return;
  }
  if (subcmd == SCMD_POUR) {
    sprintf(buf, "You pour the %s into the %s.",
	    drinks[GET_OBJ_VAL(from_obj, 2)], arg2);
    send_to_char(buf, ch);
  }
  if (subcmd == SCMD_FILL) {
    act("You gently fill $p from $P.", FALSE, ch, to_obj, from_obj, TO_CHAR);
    act("$n gently fills $p from $P.", TRUE, ch, to_obj, from_obj, TO_ROOM);
  }
  /* New alias */
  if (GET_OBJ_VAL(to_obj, 1) == 0)
    name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

  /* First same type liq. */
  GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

  /* Then how much to pour */
  GET_OBJ_VAL(from_obj, 1) -= (amount =
			 (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));

  GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

  if (GET_OBJ_VAL(from_obj, 1) < 0) {	/* There was too little */
    GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
    amount += GET_OBJ_VAL(from_obj, 1);
    GET_OBJ_VAL(from_obj, 1) = 0;
    GET_OBJ_VAL(from_obj, 2) = 0;
    GET_OBJ_VAL(from_obj, 3) = 0;
    name_from_drinkcon(from_obj);
  }
  /* Then the poison boogie */
  GET_OBJ_VAL(to_obj, 3) =
    (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

  /* And the weight boogie */
  weight_change_object(from_obj, -amount);
  weight_change_object(to_obj, amount);	/* Add weight */

  return;
}



void wear_message(struct char_data * ch, struct obj_data * obj, int where)
{
  char *wear_messages[][2] = {
    {"$n lights $p and holds it.",
    "You light $p and hold it."},

    {"$n slides $p on to $s right ring finger.",
    "You slide $p on to your right ring finger."},

    {"$n slides $p on to $s left ring finger.",
    "You slide $p on to your left ring finger."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p around $s neck.",
    "You wear $p around your neck."},

    {"$n wears $p on $s body.",
    "You wear $p on your body.",},

    {"$n wears $p on $s head.",
    "You wear $p on your head."},

    {"$n puts $p on $s legs.",
    "You put $p on your legs."},

    {"$n wears $p on $s feet.",
    "You wear $p on your feet."},

    {"$n puts $p on $s hands.",
    "You put $p on your hands."},

    {"$n wears $p on $s arms.",
    "You wear $p on your arms."},

    {"$n straps $p around $s arm as a shield.",
    "You start to use $p as a shield."},

    {"$n wears $p about $s body.",
    "You wear $p around your body."},

    {"$n wears $p around $s waist.",
    "You wear $p around your waist."},

    {"$n puts $p on around $s right wrist.",
    "You put $p on around your right wrist."},

    {"$n puts $p on around $s left wrist.",
    "You put $p on around your left wrist."},

    {"$n wields $p.",
    "You wield $p."},

    {"$n grabs $p.",
    "You grab $p."},

    {"$n clips $p to $s right ear.",
    "You clip $p to your right ear."},

    {"$n clips $p to $s left ear.",
    "You clip $p to your left ear."},

    {"$n slides $p on $s right thumb.",
    "You slide $p on your right thumb."},

    {"$n slides $p on $s left thumb.",
    "You slide $p on your left thumb."},

    {"$n puts $p over $s eyes.",
    "You put $p over your eyes."},

    {"$n puts $p on $s face.",
    "You put $p on your face."},

    {"$n throws $p into the air, which begins to hover around $s head!",
     "You throw $p into the air, which begins to hover around your head!"},

    {"$n throws $p into the air, which begins to hover around $s head!",
     "You throw $p into the air, which begins to hover around your head!"},

    {"$n wears $p on $s ankles.",
     "You wear $p on your ankles."},

    {"$n wears $p on $s back.",
     "You wear $p on your back."},

    {"$n readies $p.",
     "You ready $p."}

  };

  act(wear_messages[where][0], TRUE, ch, obj, 0, TO_ROOM);
  act(wear_messages[where][1], FALSE, ch, obj, 0, TO_CHAR);
}



bool perform_wear(struct char_data * ch, struct obj_data * obj, int where, int wearall)
{
  /*
   * ITEM_WEAR_TAKE is used for objects that do not require special bits
   * to be put into that position (e.g. you can hold any object, not just
   * an object with a HOLD bit.)
   */
  int i;

  int wear_bitvectors[] = {
    ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_NECK,
    ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
    ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
    ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
    ITEM_WEAR_WIELD, ITEM_WEAR_TAKE, ITEM_WEAR_EAR, ITEM_WEAR_EAR,
    ITEM_WEAR_THUMB, ITEM_WEAR_THUMB, ITEM_WEAR_EYES, ITEM_WEAR_FACE,
    ITEM_WEAR_FLOAT_1, ITEM_WEAR_FLOAT_1, ITEM_WEAR_ANKLE, ITEM_WEAR_BACK};

  char *already_wearing[] = {
    "You're already using a light.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both of your ring fingers.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You can't wear anything else around your neck.\r\n",
    "You're already wearing something on your body.\r\n",
    "You're already wearing something on your head.\r\n",
    "You're already wearing something on your legs.\r\n",
    "You're already wearing something on your feet.\r\n",
    "You're already wearing something on your hands.\r\n",
    "You're already wearing something on your arms.\r\n",
    "You're already using a shield.\r\n",
    "You're already wearing something about your body.\r\n",
    "You already have something around your waist.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something around both of your wrists.\r\n",
    "You're already wielding a weapon.\r\n",
    "You're already holding something.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both your ears.\r\n",
    "YOU SHOULD NEVER SEE THIS MESSAGE.  PLEASE REPORT.\r\n",
    "You're already wearing something on both your thumbs.\r\n",
    "You're already wearing something over your eyes.\r\n",
    "You're already wearing something on your face.\r\n",
    "Holy butt nuggets Batman!\r\n",
    "You already have enough items floating around you.\r\n",
    "You already have something on your ankles.\r\n",
    "You already have something on your back.\r\n"
  };

  if (obj->obj_flags.clan != GET_CLAN(ch) -1) {
    if (obj->obj_flags.clan != -1) {
      sprintf(buf, "Only members of clan %s may use this eq.\r\n",
                    clan[obj->obj_flags.clan].name);
      send_to_char(buf, ch);
      return (FALSE);
    }
  }

  if (obj->obj_flags.player != GET_IDNUM(ch)) {
    if (obj->obj_flags.player != -1) {
      send_to_char("This is owned eq and you may not wear it.\r\n", ch);
      return (FALSE);
    }
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (obj->obj_flags.vamp_wear >= 1 &&
      obj->obj_flags.vamp_wear != GET_OBJ_VAMP(GET_EQ(ch, i)) &&
      GET_OBJ_VAMP(GET_EQ(ch, i)) != -1) {
      send_to_char("You are already wearing one or more pieces from a seperate set of the Unholy Five.\r\nYou may only wear pieces of the same set.\r\n.", ch);
      return (FALSE);
      }
    }
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (obj->obj_flags.titan_wear >= 1 &&
      obj->obj_flags.titan_wear != GET_OBJ_TITAN(GET_EQ(ch, i)) &&
      GET_OBJ_TITAN(GET_EQ(ch, i)) != -1) {
      send_to_char("You are already wearing one or more pieces from a seperate set of the Five Suits of Might.\r\nYou may only wear pieces of the same set.\r\n.", ch);
      return (FALSE);
      }
    }
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (obj->obj_flags.saint_wear >= 1 &&
      obj->obj_flags.saint_wear != GET_OBJ_SAINT(GET_EQ(ch, i)) &&
      GET_OBJ_SAINT(GET_EQ(ch, i)) != -1) {
      send_to_char("You are already wearing one or more pieces from a seperate set of the Five Holy Rainments.\r\nYou may only wear pieces of the same set.\r\n.", ch);
      return (FALSE);
      }
    }
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (obj->obj_flags.demon_wear >= 1 &&
      obj->obj_flags.demon_wear != GET_OBJ_DEMON(GET_EQ(ch, i)) &&
      GET_OBJ_DEMON(GET_EQ(ch, i)) != -1) {
      send_to_char("You are already wearing one or more pieces from a seperate set of the Five Suits of Malice.\r\nYou may only wear pieces of the same set.\r\n.", ch);
      return (FALSE);
      }
    }
  }

  if (!IS_NPC(ch)) {
    if (GET_CLASS(ch) < CLASS_VAMPIRE && obj->obj_flags.minlevel > GET_LEVEL(ch)){
      sprintf(buf, "You must be at least level %d to use %s.\r\n",
                    obj->obj_flags.minlevel,
                    obj->short_description);
      send_to_char(buf, ch);
      return (FALSE);
    }
  }

  if (IS_SIZE_SMALL(ch) && (IS_OBJ_STAT(obj, ITEM_MEDIUM_SIZE) ||
      IS_OBJ_STAT(obj, ITEM_LARGE_SIZE)) && !IS_OBJ_STAT(obj, ITEM_SMALL_SIZE)){
    send_to_char("That item is too big for you.\r\n", ch);
    return (FALSE);
  }
  if (IS_SIZE_SMALL(ch) && (IS_OBJ_STAT(obj, ITEM_TINY_SIZE)) &&
     !IS_OBJ_STAT(obj, ITEM_SMALL_SIZE)) {
    send_to_char("That item is too small for you.\r\n", ch);
    return (FALSE);
  }
  if (IS_SIZE_TINY(ch) && (IS_OBJ_STAT(obj, ITEM_MEDIUM_SIZE) ||
      IS_OBJ_STAT(obj, ITEM_LARGE_SIZE) || IS_OBJ_STAT(obj, ITEM_SMALL_SIZE)) &&
     !IS_OBJ_STAT(obj, ITEM_TINY_SIZE)) {
    send_to_char("That item is too big for you.\r\n", ch);
    return (FALSE);
  }
  if (IS_SIZE_MEDIUM(ch) && (IS_OBJ_STAT(obj, ITEM_SMALL_SIZE) ||
      IS_OBJ_STAT(obj, ITEM_TINY_SIZE)) && !IS_OBJ_STAT(obj, ITEM_MEDIUM_SIZE)){
    send_to_char("That item is too small for you.\r\n", ch);
    return (FALSE);
  }
  if (IS_SIZE_MEDIUM(ch) && (IS_OBJ_STAT(obj, ITEM_LARGE_SIZE)) &&
     !IS_OBJ_STAT(obj, ITEM_MEDIUM_SIZE)) {
    send_to_char("That item is too big for you.\r\n", ch);
    return (FALSE);
  }
  if (IS_SIZE_LARGE(ch) && (IS_OBJ_STAT(obj, ITEM_MEDIUM_SIZE) ||
      IS_OBJ_STAT(obj, ITEM_SMALL_SIZE) || IS_OBJ_STAT(obj, ITEM_TINY_SIZE)) &&
     !IS_OBJ_STAT(obj, ITEM_LARGE_SIZE)) {
    send_to_char("That item is too small for you.\r\n", ch);
    return (FALSE);
  }

  /* first, make sure that the wear position is valid. */
  if (!CAN_WEAR(obj, wear_bitvectors[where])) {
    act("You can't wear $p there.", FALSE, ch, obj, 0, TO_CHAR);
    return (FALSE);
  }
  /* demons with Netherclaws can't use hand eq */
  if (where == WEAR_HANDS && IS_AFFECTED(ch, AFF_NETHERCLAW)) {
    send_to_char("You cannot wear anything on your hands with the Netherclaw.\r\n", ch);
    return (FALSE);
  }

  /* for neck, finger/wrist/ear/thumb try pos 2 if pos 1 is already full */
  if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) ||
      (where == WEAR_WRIST_R) || (where == WEAR_EAR_R) ||
      (where == WEAR_THUMB_R) || (where == WEAR_FLOAT_1))
    if (GET_EQ(ch, where))
      where++;

  if (GET_EQ(ch, where)) {
    if(wearall == NOTWEARALL){
      if(perform_remove(ch, where))
        perform_wear(ch, obj, where, NOTWEARALL);
      else
        send_to_char("You have no space in your inventory for the item you are trying to replace.\r\n", ch);
    }
    else
      send_to_char(already_wearing[where], ch);
    return (FALSE);
  }

  if (obj->item_number == real_object(361) &&
      GET_LEVEL(ch) == 150 && IS_REMORT(ch)) {
    send_to_char("\r\n/cWYour entire body begins to glow, a holy light shining all about you.\r\n", ch);
    send_to_char("You feel the power of the Gods course through you, your entire being\r\n", ch);
    send_to_char("infused with the power of immortality.\r\n\r\n", ch);
    send_to_char("You have become spawn of the Gods, Saint Immortal./c0\r\n", ch);
    GET_CLASS(ch) = CLASS_SAINT;
    GET_LEVEL(ch) = 1;
    GET_TIER(ch) = 1;
    GET_EXP(ch) = 1;
    SET_SKILL(ch, SKILL_SECOND_ATTACK, 95);
    SET_SKILL(ch, SKILL_THIRD_ATTACK, 95);
    SET_SKILL(ch, SKILL_FOURTH_ATTACK, 95);
    SET_SKILL(ch, SKILL_FIFTH_ATTACK, 95);
    SET_SKILL(ch, SKILL_SIXTH_ATTACK, 95);
    SET_PRAYER(ch, PRAY_EXORCISM, 95);
	double_exp = 20;
	sprintf(buf, "/cW%s has become a spawn of the Gods, a Saint Immortal!/c0\r\n", GET_NAME(ch));
	sprintf(buf, "%s/cRDouble Experience is declared for the next 20 minutes, \r\nin honour of %s./c0\r\n", buf, GET_NAME(ch));
	send_to_all(buf);
  }

  if (obj->item_number == real_object(499) &&
      GET_LEVEL(ch) == 150 && IS_REMORT(ch)) {
    send_to_char("\r\n/cWYour entire body begins to glow, hellfire burning all about you.\r\n", ch);
    send_to_char("You feel the power of the Unholy Gods course through you, your entire being\r\n", ch);
    send_to_char("infused with the power of immortality.\r\n\r\n", ch);
    send_to_char("You have become one with the Horde of Evil, Demon Immortal!/c0\r\n", ch);
    GET_CLASS(ch) = CLASS_DEMON;
    GET_LEVEL(ch) = 1;
    GET_TIER(ch) = 1;
    GET_EXP(ch) = 1;
    SET_SKILL(ch, SKILL_SECOND_ATTACK, 95);
    SET_SKILL(ch, SKILL_THIRD_ATTACK, 95);
    SET_SKILL(ch, SKILL_FOURTH_ATTACK, 95);
    SET_SKILL(ch, SKILL_FIFTH_ATTACK, 95);
    SET_SKILL(ch, SKILL_SIXTH_ATTACK, 95);
	double_exp = 20;
	sprintf(buf, "/cL%s has become one with the Horde of Evil, a Demon Immortal!/c0\r\n", GET_NAME(ch));
	sprintf(buf, "%s/cRDouble Experience is declared for the next 20 minutes, \r\nin honour of %s./c0\r\n", buf, GET_NAME(ch));
	send_to_all(buf);
  }

  if (!wear_otrigger(obj, ch, where))
    return (FALSE);

  wear_message(ch, obj, where);
  if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BLIND))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BLIND))
			send_to_char("Your vision fades away.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_INVISIBLE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_INVISIBLE))
			send_to_char("Your body fades away.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_ALIGN))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_ALIGN))
			send_to_char("You now know the difference between good and evil.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_INVIS))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_INVIS))
			send_to_char("You start seeing invisible lifeforms.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_MAGIC))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_MAGIC))
			send_to_char("You can now detect hidden magic.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SENSE_LIFE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SENSE_LIFE))
			send_to_char("You develop superior vision.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_WATERWALK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_WATERWALK))
			send_to_char("You gain the knowledge of waterwalking.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SANCTUARY))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SANCTUARY))
			send_to_char("You should not see this. Sanctuary is a illegal affection on objects, please report.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_GROUP))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_GROUP))
			send_to_char("You should not see this. Group as an affection on objs is illegal.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CURSE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CURSE))
			send_to_char("You become cursed.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_INFRAVISION))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_POISON))
			send_to_char("Your eyes starts to glow red.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_POISON))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PROTECT_EVIL))
			send_to_char("You feel poison running trough your veins.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PROTECT_EVIL))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PROTECT_EVIL))
			send_to_char("You feel protected from the dark forces.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PROTFROMGOOD))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PROTFROMGOOD))
			send_to_char("You feel protected from the holy forces.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SLEEP))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SLEEP))
			send_to_char("You go to sleep.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_NOTRACK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_NOTRACK))
			send_to_char("Your trails vanish behind you.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SILENCE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SILENCE))
			send_to_char("You become silent.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_FIRESHIELD))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_FIRESHIELD))
			send_to_char("You are surrounded by a shield of fire.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SNEAK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SNEAK))
			send_to_char("You start to move silently.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HIDE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HIDE))
			send_to_char("You start hiding.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PARALYZE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PARALYZE))
			send_to_char("You are paralyzed.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CHARM))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM))
			send_to_char("You are charmed.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_FLY))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_FLY))
			send_to_char("You start flying.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BREATHE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BREATHE))
			send_to_char("You can now breathe underwater.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HASTE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HASTE))
			send_to_char("You start moving faster.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CHAOSARMOR))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CHAOSARMOR))
			send_to_char("You feel ready to fight at war!\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HOLYARMOR))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HOLYARMOR))
			send_to_char("You become affected by holy armor.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_AEGIS))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_AEGIS))
			send_to_char("You become affected by aegis.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_REGENERATE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_REGENERATE))
			send_to_char("Your body starts regenerating.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BLACKDART))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BLACKDART))
			send_to_char("You get affected by black dart.\r\n", ch);
  obj_from_char(obj);
  equip_char(ch, obj, where);
   return (TRUE);

}



int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg)
{
  int where = -1;

  static char *keywords[] = {
    "!RESERVED!",
    "finger",
    "!RESERVED!",
    "neck",
    "!RESERVED!",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "shield",
    "about",
    "waist",
    "wrist",
    "!RESERVED!",
    "!RESERVED!",
    "!RESERVED!",
    "ear",
    "!RESERVED!",
    "thumb",
    "!RESERVED!",
    "eyes",
    "face",
    "floating",
    "!RESERVED!",
    "ankles",
    "back",
    "\n"
  };

  if (!arg || !*arg) {
    if (CAN_WEAR(obj, ITEM_WEAR_FINGER))      where = WEAR_FINGER_R;
    if (CAN_WEAR(obj, ITEM_WEAR_NECK))        where = WEAR_NECK_1;
    if (CAN_WEAR(obj, ITEM_WEAR_BODY))        where = WEAR_BODY;
    if (CAN_WEAR(obj, ITEM_WEAR_HEAD))        where = WEAR_HEAD;
    if (CAN_WEAR(obj, ITEM_WEAR_LEGS))        where = WEAR_LEGS;
    if (CAN_WEAR(obj, ITEM_WEAR_FEET))        where = WEAR_FEET;
    if (CAN_WEAR(obj, ITEM_WEAR_HANDS))       where = WEAR_HANDS;
    if (CAN_WEAR(obj, ITEM_WEAR_ARMS))        where = WEAR_ARMS;
    if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))      where = WEAR_SHIELD;
    if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))       where = WEAR_ABOUT;
    if (CAN_WEAR(obj, ITEM_WEAR_WAIST))       where = WEAR_WAIST;
    if (CAN_WEAR(obj, ITEM_WEAR_WRIST))       where = WEAR_WRIST_R;
    if (CAN_WEAR(obj, ITEM_WEAR_EAR))         where = WEAR_EAR_R;
    if (CAN_WEAR(obj, ITEM_WEAR_THUMB))       where = WEAR_THUMB_R;
    if (CAN_WEAR(obj, ITEM_WEAR_EYES))        where = WEAR_EYES;
    if (CAN_WEAR(obj, ITEM_WEAR_FACE))        where = WEAR_FACE;
    if (CAN_WEAR(obj, ITEM_WEAR_FLOAT_1))     where = WEAR_FLOAT_1;
    if (CAN_WEAR(obj, ITEM_WEAR_ANKLE))       where = WEAR_ANKLE;
    if (CAN_WEAR(obj, ITEM_WEAR_BACK))        where = WEAR_BACK;
  } else {
    if (((where = search_block(arg, keywords, FALSE)) < 0) ||
        (*arg=='!')) {
      sprintf(buf, "'%s'?  What part of your body is THAT?\r\n", arg);
      send_to_char(buf, ch);
      return -1;
    }
  }

  return where;
}



ACMD(do_wear)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj, *next_obj;
  int where, dotmode, items_worn = 0;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("Wear what?\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) == CLASS_DEMON)
  {
	  if (IS_GOOD(ch))
	  {
		  send_to_char("The unholy powers that be have forsaken you since you have strayed from the path!\r\n", ch);
		  return;
	  }
  }
  if (GET_CLASS(ch) == CLASS_SAINT)
  {
	  if (IS_EVIL(ch))
	  {
		  send_to_char("The Gods of Holiness frown upon you and your errant ways!\r\n", ch);
		  return;
	  }
  }
  dotmode = find_all_dots(arg1);

  if (*arg2 && (dotmode != FIND_INDIV)) {
    send_to_char("You can't specify the same body location for more than one item!\r\n", ch);
    return;
  }
  if (dotmode == FIND_ALL) {
    for (obj = ch->carrying; obj; obj = next_obj) {
      next_obj = obj->next_content;
      if (CAN_SEE_OBJ(ch, obj) && (where = find_eq_pos(ch, obj, 0)) >= 0) {
	items_worn++;
	perform_wear(ch, obj, where, WEARALL);
      }
    }
    if (!items_worn)
      send_to_char("You don't seem to have anything wearable.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg1) {
      send_to_char("Wear all of what?\r\n", ch);
      return;
    }
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have any %ss.\r\n", arg1);
      send_to_char(buf, ch);
    } else
      while (obj) {
	next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
	if ((where = find_eq_pos(ch, obj, 0)) >= 0)
	  perform_wear(ch, obj, where, WEARALL);
	else
	  act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
	obj = next_obj;
      }
  } else {
    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
      send_to_char(buf, ch);
    } else {
      if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
	perform_wear(ch, obj, where, NOTWEARALL);
      else if (!*arg2)
	act("You can't wear $p.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}



ACMD(do_wield)
{
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Wield what?\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) == CLASS_DEMON)
  {
	  if (IS_GOOD(ch))
	  {
		  send_to_char("The unholy powers that be have forsaken you since you have strayed from the path!\r\n", ch);
		  return;
	  }
  }
  else if (GET_CLASS(ch) == CLASS_SAINT)
  {
	  if (IS_EVIL(ch))
	  {
		  send_to_char("The Gods of Holiness frown upon you and your errant ways!\r\n", ch);
		  return;
	  }
  }

  if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
    return; 
  } else {
    if (!CAN_WEAR(obj, ITEM_WEAR_WIELD)) {
      send_to_char("You can't wield that.\r\n", ch);
      return;
    }
    else if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
      send_to_char("It's too heavy for you to use.\r\n", ch);
      return;
    }
    else {
      perform_wear(ch, obj, WEAR_WIELD, NOTWEARALL);
    } 
  }
}



ACMD(do_grab)
{
  struct obj_data *obj;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hold what?\r\n", ch);
  else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  }/* else
    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
    sprintf(buf, "%s is a weapon. You must use dualwield "
                 "to hold a second weapon.\r\n", obj->short_description);
    send_to_char(buf, ch);
  }*/ else
    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
      perform_wear(ch, obj, WEAR_LIGHT, NOTWEARALL);
    else {
      if (!CAN_WEAR(obj, ITEM_WEAR_HOLD) && GET_OBJ_TYPE(obj) != ITEM_WAND &&
      GET_OBJ_TYPE(obj) != ITEM_STAFF && GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
	  GET_OBJ_TYPE(obj) != ITEM_POTION)
	send_to_char("You can't hold that.\r\n", ch);
      else
	perform_wear(ch, obj, WEAR_HOLD, NOTWEARALL);
    }

}



int perform_remove(struct char_data * ch, int pos)
{
  struct obj_data *obj;

  if (!(obj = GET_EQ(ch, pos))) {
    log("Error in perform_remove: bad pos passed.");
    return FALSE;
  }
  if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)){
    act("$p: you can't carry that many items!", FALSE, ch, obj, 0, TO_CHAR);
    return FALSE;
  }
  else {
    if (!remove_otrigger(obj, ch))
      return FALSE;
    obj_to_char(unequip_char(ch, pos), ch);
    act("You stop using $p.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n stops using $p.", TRUE, ch, obj, 0, TO_ROOM);
    if (IS_AFFECTED(ch, AFF_NETHERCLAW)) {
      act("You unmeld your hand from your weapon, the Netherclaw disappears.",
           FALSE, ch, 0, 0, TO_CHAR);
      act("$n unmelds $s hand from $s weapon, $s Netherclaw disappears.",
           FALSE, ch, 0, 0, TO_ROOM);
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_NETHERCLAW);
      affect_from_char(ch, SPELL_NETHERCLAW, ABT_SPELL);
    }

	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BLIND))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BLIND))
			send_to_char("Your can see again.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_INVISIBLE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_INVISIBLE))
			send_to_char("Your become visible again.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_ALIGN))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_ALIGN))
			send_to_char("You loose the knowledge of good and evil.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_INVIS))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_INVIS))
			send_to_char("You stop seeing invisible lifeforms.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_DETECT_MAGIC))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_DETECT_MAGIC))
			send_to_char("You can no longer detect hidden magic.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SENSE_LIFE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SENSE_LIFE))
			send_to_char("You no longer have superior vision.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_WATERWALK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_WATERWALK))
			send_to_char("You loose the knowledge of waterwalking.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SANCTUARY))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SANCTUARY))
			send_to_char("You should not see this. Sanctuary is a illegal affection on objects, please report.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_GROUP))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_GROUP))
			send_to_char("You should not see this. Group as an affection on objs is illegal.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CURSE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CURSE))
			send_to_char("Your curse dissapears.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_INFRAVISION))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_INFRAVISION))
			send_to_char("Your eyes looses their red glow.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_POISON))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_POISON))
			send_to_char("You feel the poison stop running trough your veins.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PROTECT_EVIL))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PROTECT_EVIL))
			send_to_char("You are no longer protected by the dark forces.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PROTFROMGOOD))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PROTFROMGOOD))
			send_to_char("You are no longer protected by the holy forces.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SLEEP))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SLEEP))
			send_to_char("You wake up.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_NOTRACK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_NOTRACK))
			send_to_char("Your start leaving tracks behind you again.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SILENCE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SILENCE))
			send_to_char("You stop being silent.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_FIRESHIELD))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_FIRESHIELD))
			send_to_char("Your shield of fire surrounding you dissapears in a thin mist.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_SNEAK))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_SNEAK))
			send_to_char("You forget how to move silently.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HIDE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HIDE))
			send_to_char("You stop hiding.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_PARALYZE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_PARALYZE))
			send_to_char("You are no longer paralyzed.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CHARM))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM))
			send_to_char("You are no longer charmed.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_FLY))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_FLY))
			send_to_char("You stop flying, and slowly sink down to the ground.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BREATHE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BREATHE))
			send_to_char("You loose your ability to breath underwater.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HASTE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HASTE))
			send_to_char("You slow down.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_CHAOSARMOR))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_CHAOSARMOR))
			send_to_char("You do not feel ready to fight at war!\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_HOLYARMOR))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_HOLYARMOR))
			send_to_char("You are no longer affected by holy armor.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_AEGIS))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_AEGIS))
			send_to_char("You are no longer affected by aegis.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_REGENERATE))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_REGENERATE))
			send_to_char("Your body stops regenerating.\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_BLACKDART))
		if (!IS_SET_AR(AFF_FLAGS(ch), AFF_BLACKDART))
			send_to_char("Your are no longer affected by black dart.\r\n", ch);
	/*
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_))
		send_to_char(".\r\n", ch);
	if (IS_SET_AR(obj->obj_flags.bitvector, AFF_))
		send_to_char(".\r\n", ch);*/
  }
  return TRUE;
}



ACMD(do_remove)
{
  struct obj_data *obj;
  int i, dotmode, found;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Remove what?\r\n", ch);
    return;
  }
  dotmode = find_all_dots(arg);

  if (dotmode == FIND_ALL) {
    found = 0;
    for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
	perform_remove(ch, i);
	found = 1;
      }
    if (!found)
      send_to_char("You're not using anything.\r\n", ch);
  } else if (dotmode == FIND_ALLDOT) {
    if (!*arg)
      send_to_char("Remove all of what?\r\n", ch);
    else {
      found = 0;
      for (i = 0; i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
	    isname(arg, GET_EQ(ch, i)->name)) {
	  perform_remove(ch, i);
	  found = 1;
	}
      if (!found) {
	sprintf(buf, "You don't seem to be using any %ss.\r\n", arg);
	send_to_char(buf, ch);
      }
    }
  } else {
    if (!(obj = get_object_in_equip_vis(ch, arg, ch->equipment, &i))) {
      sprintf(buf, "You don't seem to be using %s %s.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
    } else
      perform_remove(ch, i);
  }
}

#define MIN_SAC_EXP 2
#define MAX_SAC_EXP 240
ACMD(do_sac)
{
  struct obj_data *obj;
  struct char_data *tmp_char;
  int dotmode, exp=0;
  char msg[80];

  one_argument(argument, arg);

  dotmode = find_all_dots(arg);

  if (!*arg) {
    send_to_char("Sacrifice what?\r\n", ch);
    return;
  }
  if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &obj)){
    if (GET_OBJ_VAL(obj, 3) != 1) {
      send_to_char("You can't sacrifice that!\r\n", ch);
      return;
    } else {
      exp = GET_OBJ_EXP(obj);
      if (exp < 0) {
        send_to_char("You can't sacrifice player corpses.\r\n", ch);
        return;
      } else {
        sprintf(buf, "You sacrifice %s\r\n", obj->short_description);
        send_to_char(buf, ch);
      }

      if (exp < MIN_SAC_EXP)
        exp = MIN_SAC_EXP;
      if (exp > MAX_SAC_EXP)
        exp = MAX_SAC_EXP;
      sprintf(msg, "You are rewarded with %d experience points\r\n", exp);
      act("$n sacrifices a corpse.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char(msg, ch);
      extract_obj(obj);
      gain_exp(ch, exp);
    }
  } else{
    send_to_char("You can't find it.\r\n", ch);
    return;
  }
}


/* struct for syls */
struct syllable {
  char *org;
  char *new;
};

/* extern variables */
extern char *spells[];
extern char *prayers[];
// extern struct abil_info_type abil_info[];
extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
extern struct syllable syls[];

/* extern procedures */
int mag_rescost(struct char_data * ch, int spellnum, byte type);


char *get_spell_name(char *argument)
{
  char *s;

  s = strtok(argument, "'");
  s = strtok(NULL, "'");

  return s;
}


char *potion_names[] = {
	"milky white",
	"bubbling white",
	"glowing ivory",
	"glowing blue",
	"bubbling yellow",
	"light green",
	"gritty brown",
	"blood red",
	"swirling purple",
	"flickering green",
	"cloudy blue",
	"glowing red",
	"sparkling white",
	"incandescent blue",
   "puke green",
   "blurry red",
   "clear fizzing",
	"light blue",
	"clear",
	"deep green",
	"fiery red",
	"identify",
	"brilliant green",
	"bright orange",
	"crystal clear"
};

#define FINAL_POTION 31

void make_potion(struct char_data *ch, int potion, struct obj_data *container)
{
	struct obj_data *final_potion;
	struct extra_descr_data *new_descr;
	int can_make = TRUE, mana, dam, num = 0;

	/* Modify this list to suit which spells you
	   want to be able to mix. */
	switch (potion) {
	case SPELL_CURE_BLIND:
	  num = 0;
	  break;
/*Question, why are there cure spells under brew, the cleric skill?*/
	case SPELL_CURE_LIGHT:
	  num = 1;
	  break;
	case SPELL_CURE_CRITIC:
	  num = 2;
	  break;
	case SPELL_DETECT_MAGIC:
	  num = 3;
	  break;
	case SPELL_DETECT_INVIS:
	  num = 4;
	  break;
	case SPELL_DETECT_POISON:
	  num = 5;
	  break;
	case SPELL_REMOVE_POISON:
	  num = 6;
	  break;
	case SPELL_STRENGTH:
	  num = 7;
	  break;
	case SPELL_WORD_OF_RECALL:
	  num = 8;
	  break;
	case SPELL_FLY:
	  num = 9;
	  break;
	case SPELL_WATERWALK:
	  num = 10;
	  break;
	case SPELL_INFRAVISION:
	  num = 11;
	  break;
	case SPELL_HEAL:
	  num = 12;
	  break;
/*Question, why are there sanc spells under brew, the cleric skill?*/
	case SPELL_SANCTUARY:
	  num = 13;
	  break;
  case SPELL_GROWTH:
	 num = 14;
	 break;
  case SPELL_BLUR:
	 num = 15;
	 break;
  case SPELL_HASTE:
	 num = 16;
	 break;
  case SPELL_ARMOR:
	 num = 17;
	 break;
  case SPELL_INVISIBLE:
	 num = 18;
	 break;
  case SPELL_MINOR_STRENGTH:
	 num = 19;
	 break;
  case SPELL_FIRESHIELD:
	 num = 20;
	 break;
  case SPELL_IDENTIFY:
	 num = 21;
	 break;
  case SPELL_MAJOR_STRENGTH:
	 num = 22;
	 break;
  case SPELL_MANA_ECONOMY:
	 num = 23;
	 break;
  case SPELL_MIST_FORM:
	 num = 24;
	 break;
  default:
	 can_make = FALSE;
	 break;
	}


  if (GET_SPELL(ch, potion) == 0) {
    send_to_char("You can only brew potions for spells you know.\r\n", ch);
    return;
  }

	if (can_make == FALSE) {
		send_to_char("That spell cannot be mixed into a"
			     " potion.\r\n", ch);
		return;
	}
          else if ((number(1, 7) == 7) && (GET_LEVEL(ch) < LVL_IMMORT)) {
		send_to_char("As you begin mixing the potion, it violently"
			     " explodes!\r\n",ch);
		act("$n begins to mix a potion, but it suddenly explodes!",
		    FALSE, ch, 0,0, TO_ROOM);
		extract_obj(container);
		dam = number(GET_LEVEL(ch), mag_rescost(ch, potion, ABT_SPELL));
		GET_HIT(ch) -= dam;
		update_pos(ch);
                if (GET_POS(ch) <= POS_INCAP) {
		  die(ch,ch);
		}
		return;
	}


	/* requires x3 mana to mix a potion than the spell */
	mana = mag_rescost(ch, potion, ABT_SPELL) * 3;
	if (GET_MANA(ch) - mana > 0) {
	    if (GET_LEVEL(ch) < LVL_IMMORT)
                GET_MANA(ch) -= mana;
		sprintf(buf, "You create a %s potion.\r\n",
		    spells[potion]);
		send_to_char(buf, ch);
		act("$n creates a potion!", FALSE, ch, 0, 0, TO_ROOM);
		extract_obj(container);
	}
	else {
		send_to_char("You don't have enough mana to mix"
			     " that potion!\r\n", ch);
		return;
	}

	final_potion = read_object(FINAL_POTION, VIRTUAL);;
	final_potion->in_room = NOWHERE;
	sprintf(buf2, "%s %s potion", potion_names[num], spells[potion]);
	final_potion->name = str_dup(buf2);

	sprintf(buf2, "A %s potion lies here.", potion_names[num]);
	final_potion->description = str_dup(buf2);

	sprintf(buf2, "a %s potion", potion_names[num]);
	final_potion->short_description = str_dup(buf2);

	/* extra description coolness! */
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword = str_dup(final_potion->name);
	sprintf(buf2, "It appears to be a %s potion.", spells[potion]);
	new_descr->description = str_dup(buf2);
	new_descr->next = NULL;
	final_potion->ex_description = new_descr;

	GET_OBJ_VAL(final_potion, 0) = GET_LEVEL(ch);
	GET_OBJ_VAL(final_potion, 1) = potion;
	GET_OBJ_VAL(final_potion, 2) = -1;
	GET_OBJ_VAL(final_potion, 3) = -1;
	GET_OBJ_COST(final_potion) = GET_LEVEL(ch) * 10;
	GET_OBJ_WEIGHT(final_potion) = 1;
	GET_OBJ_RENT(final_potion) = 0;
        final_potion->obj_flags.player = -1;
        final_potion->obj_flags.clan = -1;
        final_potion->obj_flags.vamp_wear = -1;
        final_potion->obj_flags.titan_wear = -1;
        final_potion->obj_flags.saint_wear = -1;
        final_potion->obj_flags.demon_wear = -1;

	obj_to_char(final_potion, ch);
}

ACMD(do_brew)
{
	struct obj_data *container = NULL;
	struct obj_data *obj, *next_obj;
	char bottle_name[MAX_STRING_LENGTH];
	char spell_name[MAX_STRING_LENGTH];
	char *temp1, *temp2;
	int potion, found = FALSE;

	temp1 = one_argument(argument, bottle_name);

	/* sanity check */
	if (temp1) {
	    temp2 = get_spell_name(temp1);
	    if (temp2)
		strcpy(spell_name, temp2);
	} else {
	    bottle_name[0] = '\0';
	    spell_name[0] = '\0';
	}


       	if ((!IS_SORCERER(ch) && !IS_SHADOWMAGE(ch) && !IS_WARLOCK(ch) &&
            !IS_ARCANIC(ch) && !IS_MAGI(ch) && !IS_DARKMAGE(ch) &&
            !IS_FORESTAL(ch) && !IS_PSIONIST(ch)) &&
            (GET_CLASS(ch) < CLASS_VAMPIRE || GET_LEVEL(ch) <= LVL_IMMORT)) {
		send_to_char("You have no idea how to mix potions!\r\n", ch);
		return;
	}
	if (!*bottle_name) {
		send_to_char("What do you wish to mix in where?\r\n", ch);
		return;
	}

	for (obj = ch->carrying; obj; obj = next_obj) {
		next_obj = obj->next_content;
		if (obj == NULL)
			return;
		else if (!(container = get_obj_in_list_vis(ch, bottle_name,
			ch->carrying)))
			continue;
		else
			found = TRUE;
	}
	if (found != FALSE && (GET_OBJ_TYPE(container) != ITEM_DRINKCON)) {
		send_to_char("That item is not a drink container!\r\n", ch);
		return;
	}
	if (found == FALSE) {
		sprintf(buf, "You don't have %s in your inventory!\r\n",
			bottle_name);
		send_to_char(buf, ch);
		return;
	}

	if (!spell_name || !*spell_name) {
	    send_to_char("Spell names must be enclosed in single quotes!\r\n",
			 ch);
	    return;
	}

	potion = find_abil_num(spell_name, spells);

	if ((potion < 1) || (potion > MAX_ABILITIES)) {
		send_to_char("Mix what spell?!?\r\n", ch);
		return;
	}
/*	if (GET_SKILL(ch, potion) == 0) {
		 send_to_char("You are unfamiliar with potion brewing.\r\n",
		 ch);
		return;
	} */
	make_potion(ch, potion, container);
}


char *garble_spell(int spellnum)
{
  char lbuf[256];
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }
  return buf;
}
#define FINAL_SCROLL 30

void make_scroll(struct char_data *ch, int scroll, struct obj_data *paper)
{
  struct obj_data *final_scroll;
  struct extra_descr_data *new_descr;
  int can_make = TRUE, mana, dam = 0, prayernum = 0;

   prayernum = scroll;

   switch (scroll) {
     case PRAY_CURE_LIGHT:
       scroll = SPELL_CURE_LIGHT;
       break;
     case PRAY_CURE_CRITICAL:
       scroll = SPELL_CURE_CRITIC;
       break;
     case PRAY_BLESSING:
       scroll = SPELL_BLESS;
       break;
     case PRAY_HEAVY_SKIN:
       scroll = SPELL_ARMOR;
       break;
     case PRAY_SANCTUARY:
       scroll = SPELL_SANCTUARY;
       break;
     case PRAY_CURE_BLIND:
       scroll = SPELL_CURE_BLIND;
       break;
     case PRAY_REFRESH:
       scroll = SPELL_REFRESH;
       break;
     case PRAY_HEAL:
       scroll = SPELL_HEAL;
       break;
     case PRAY_SECOND_SIGHT:
       scroll = SPELL_DETECT_INVIS;
       break;
     case PRAY_DETECT_POISON:
       scroll = SPELL_DETECT_POISON;
       break;
     case PRAY_REMOVE_POISON:
       scroll = SPELL_REMOVE_POISON;
       break;
     case PRAY_LIFT_CURSE:
       scroll = SPELL_REMOVE_CURSE;
       break;
     case PRAY_INFRAVISION:
       scroll = SPELL_INFRAVISION;
       break;
     case PRAY_GUIDING_LIGHT:
       scroll = SPELL_WORD_OF_RECALL;
       break;
/*
These dont seem to want to work... SM
     case PRAY_CURE_SERIOUS:
       scroll = PRAY_CURE_SERIOUS;
       break;
     case PRAY_HOLY_ARMOR:
       scroll = PRAY_HOLY_ARMOR;
       break;
     case PRAY_SACRED_SHIELD:
       scroll = PRAY_SACRED_SHIELD;
       break;
     case PRAY_DEFLECT:
       scroll = PRAY_DEFLECT;
       break;
     case PRAY_VITALITY:
       scroll = PRAY_VITALITY;
       break;
     case PRAY_REVERSE_ALIGN:
       scroll = PRAY_REVERSE_ALIGN;
       break;
*/
     case PRAY_HASTE:
       scroll = SPELL_HASTE;
       break;
     default:
       can_make = FALSE;
       break;
   }



   if (can_make == FALSE) {
     send_to_char("You cannot scribe that prayer onto a scroll.\r\n", ch);
     return;
    }
   if (!GET_PRAYER(ch, prayernum)) {
     send_to_char("You cannot scribe a prayer which you do not already know!\r\n", ch);
     return;
   }
   else if ((number(1, 7) == 7) && (GET_LEVEL(ch) < LVL_IMMORT)) {
          send_to_char("As you begin inscribing the final rune, the"
                       " scroll violently explodes!\r\n",ch);
          act("$n tries to scribe a spell, but it explodes!",FALSE, ch, 0,0, TO_ROOM);
		extract_obj(paper);
		dam = number(GET_LEVEL(ch), mag_rescost(ch, prayernum, ABT_PRAYER));
		GET_HIT(ch) -= dam;
		update_pos(ch);
                if (GET_POS(ch) <= POS_INCAP) {
		  die(ch,ch);
		}
		return;
	}
   /* requires x3 mana to scribe a scroll than the spell */
   mana = mag_rescost(ch, prayernum, ABT_PRAYER);

   if (GET_MANA(ch) - mana > 0) {
     if (GET_LEVEL(ch) < LVL_IMMORT) GET_MANA(ch) -= mana;
       sprintf(buf, "You create a scroll of %s.\r\n", prayers[prayernum]);
       send_to_char(buf, ch);
       act("$n creates a scroll!", FALSE, ch, 0, 0, TO_ROOM);
       extract_obj(paper);
     }
     else {
       send_to_char("You don't have enough mana to scribe such a powerful spell!\r\n", ch);
       return;
     }


        final_scroll = read_object(FINAL_SCROLL, VIRTUAL);
	final_scroll->in_room = NOWHERE;
	sprintf(buf2, "%s %s scroll",
		spells[scroll], garble_spell(scroll));
	final_scroll->name = str_dup(buf2);

	sprintf(buf2, "Some parchment inscribed with the runes '%s' lies here.",
		garble_spell(scroll));
	final_scroll->description = str_dup(buf2);

	sprintf(buf2, "a %s scroll", garble_spell(scroll));
	final_scroll->short_description = str_dup(buf2);

	/* extra description coolness! */
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword = str_dup(final_scroll->name);
	sprintf(buf2, "It appears to be a %s scroll.", spells[scroll]);
	new_descr->description = str_dup(buf2);
	new_descr->next = NULL;
	final_scroll->ex_description = new_descr;

	GET_OBJ_TYPE(final_scroll) = ITEM_SCROLL;
	SET_BIT_AR(GET_OBJ_WEAR(final_scroll), ITEM_WEAR_TAKE);
//	SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_MAGIC);
	GET_OBJ_VAL(final_scroll, 0) = GET_LEVEL(ch);
        final_scroll->obj_flags.player = -1;
        final_scroll->obj_flags.clan = -1;
        final_scroll->obj_flags.vamp_wear = -1;
        final_scroll->obj_flags.titan_wear = -1;
        final_scroll->obj_flags.saint_wear = -1;
        final_scroll->obj_flags.demon_wear = -1;
	GET_OBJ_VAL(final_scroll, 1) = scroll;
	GET_OBJ_VAL(final_scroll, 2) = -1;
	GET_OBJ_VAL(final_scroll, 3) = -1;
	GET_OBJ_COST(final_scroll) = GET_LEVEL(ch) * 2;
	GET_OBJ_WEIGHT(final_scroll) = 1;
	GET_OBJ_RENT(final_scroll) = GET_LEVEL(ch) * 3;

	obj_to_char(final_scroll, ch);
}

void scribe_rune(struct char_data *ch, struct obj_data *obj)
{

      int save = 0;
      save = GET_WIS(ch) + GET_INT(ch);

      if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
        send_to_char("You cannot scribe runes in the Grid.", ch);
        return;
      }
      if (ROOM_FLAGGED(ch->in_room, ROOM_NOPORTAL)) {
        send_to_char("You cannot scribe runes in this room.\r\n", ch);
        return;
      }

      if(save > number(1, 70) || GET_LEVEL(ch) >= LVL_IMMORT) {
        GET_OBJ_VAL(obj, 1) = world[ch->in_room].number;
        send_to_char("You scribe the rune.\r\n", ch);
        act("$n scribes a rune of recall", FALSE, ch, 0, 0, TO_ROOM);
      }
      else {
        send_to_char("The rune explodes!\r\n", ch);
        act("As $n begins scribing a rune it explodes!", FALSE, ch, 0, 0, TO_ROOM);
        damage(ch, ch, GET_LEVEL(ch) * 2, SKILL_FLAIL, ABT_SKILL);
        obj_from_char(obj);
        extract_obj(obj);
      }
}

ACMD(do_scribe)
{
	struct obj_data *paper = NULL;
	struct obj_data *obj, *next_obj;
	char paper_name[MAX_STRING_LENGTH];
	char spell_name[MAX_STRING_LENGTH];
	char *temp1, *temp2;
	int scroll = 0, found = FALSE;

	temp1 = one_argument(argument, paper_name);

	/* sanity check */
	if (temp1) {
	    temp2 = get_spell_name(temp1);
	    if (temp2)
		strcpy(spell_name, temp2);
	} else {
	    paper_name[0] = '\0';
	    spell_name[0] = '\0';
	}


	if (GET_CLASS(ch) < CLASS_VAMPIRE &&
           !(IS_CLERIC(ch) || IS_TEMPLAR(ch) || IS_CRUSADER(ch) ||
              IS_ARCANIC(ch) || IS_INQUISITOR(ch) || IS_STORR(ch) || IS_SAGE(ch)
	    || GET_LEVEL(ch) >= LVL_IMMORT)) {
		send_to_char("You have no idea how to scribe scrolls!\r\n", ch);
		return;
	}
	if (!*paper_name) {
		send_to_char("What do you wish to scribe where?\r\n", ch);
		return;
	}


	for (obj = ch->carrying; obj; obj = next_obj) {
		next_obj = obj->next_content;
		if (obj == NULL)
			return;
		else if (!(paper = get_obj_in_list_vis(ch, paper_name,
			ch->carrying)))
			continue;
		else
			found = TRUE;
	}
	if ((found) && ((GET_OBJ_TYPE(paper) != ITEM_NOTE) && (GET_OBJ_TYPE(paper) != ITEM_RUNE))) {
		send_to_char("You can't write on that!\r\n", ch);
		return;
	}


	if (found == FALSE) {
		sprintf(buf, "You don't have %s in your inventory!\r\n",
			paper_name);
		send_to_char(buf, ch);
		return;
	}

        if(found && GET_OBJ_TYPE(paper) == ITEM_RUNE && GET_OBJ_VAL(paper, 1)) {
		send_to_char("That rune has already been inscribed.\r\n", ch);
		return;
	}
        if(found && (GET_OBJ_TYPE(paper) == ITEM_RUNE)) {
		scribe_rune(ch, paper);
		return;
        }
	if (!spell_name || !*spell_name) {
	    send_to_char("Spell names must be enclosed in single quotes!\r\n",
			 ch);
	    return;
	}

	scroll = find_abil_num(spell_name, prayers);

	if ((scroll < 1) || (scroll > MAX_ABILITIES)) {
		send_to_char("Scribe what spell?!?\r\n", ch);
		return;
	}

	/*if (GET_SKILL(ch, scroll) == 0) {
		 send_to_char("You don't know any spell like that!\r\n",
		 ch);
		return;
	}*/
	make_scroll(ch, scroll, paper);
}

ACMD(do_hone)
{

  struct obj_data *obj, *stone;

  if (abil_info[ABT_SKILL][SKILL_HONE].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_HONE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("Not now!!\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!arg) {
    send_to_char("Hone what?", ch);
    return;
  }

  obj = GET_EQ(ch, WEAR_WIELD);

  if (!obj || !isname(arg, obj->name)) {
    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      sprintf(buf, "You don't seem to have a %s.\r\n", arg);
      send_to_char(buf, ch);
      return;
    }
  }

  if (GET_OBJ_TYPE(obj) != ITEM_WEAPON ||
      ((GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT) &&
       (GET_OBJ_VAL(obj, 3) != TYPE_SLASH - TYPE_HIT) &&
       (GET_OBJ_VAL(obj, 3) != TYPE_STAB - TYPE_HIT))) {
    send_to_char("You can only hone edged weapons.\r\n", ch);
    return;
  }

  if (!(stone = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(stone) != ITEM_STONE)) {
    send_to_char("You are not holding a sharpening stone.\r\n", ch);
    return;
  }
  if (!GET_OBJ_VAL(stone, 0)) {
    act("$p is worn out.", FALSE, ch, stone, 0, TO_CHAR);
    return;
  }

  obj->obj_flags.special = number(1, GET_SKILL(ch, SKILL_HONE));
  act("You hone $p to a razors edge.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n hones $p to a razors edge.", FALSE, ch, obj, 0, TO_ROOM);
  GET_OBJ_VAL(stone, 0)--;

}

ACMD(do_sleight)
{

  if (abil_info[ABT_SKILL][SKILL_SLEIGHT].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_SLEIGHT)) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_SLEIGHT) > number(1, 101))
    do_get(ch, argument, 0, SCMD_SLEIGHT);
  else
    do_get(ch, argument, 0, SCMD_GET);
}

ACMD(do_dualwield)
{
  struct obj_data *obj;

  if (abil_info[ABT_SKILL][SKILL_DUAL_WIELD].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  one_argument(argument, arg);

  if(!*arg) {
    send_to_char("Dualwield what?\r\n", ch);
    return;
  }

  if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg);
    send_to_char(buf,ch);
    return;
  }

  if (GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("But you are already holding something!\r\n", ch);
    return;
  }

  if (GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
    send_to_char("That is not a weapon!\r\n", ch);
    return;
  }

  if(IS_THIEF(ch) && (GET_OBJ_VAL(obj, 3) != TYPE_PIERCE - TYPE_HIT))
  {
    send_to_char("Thieves can only Dualwield piercing weapons!\r\n", ch);
    return;
  }

  if(GET_SKILL(ch, SKILL_DUAL_WIELD) > 0){
    sprintf(buf, "You start to use %s as your secondary weapon.\r\n", obj->short_description);
    send_to_char(buf, ch);
    perform_wear(ch, obj, WEAR_HOLD, NOTWEARALL);
  }
  else
    send_to_char("You can't figure out how to wield two weapons at once!\r\n", ch);

}


/* added for do_assign_player stuff. -Sam 5/3/00 */
int can_get_corpse(struct char_data *ch, int num)
{
  struct char_data *vict;
  int i = 0;

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if (GET_IDNUM(ch) != num) {
      if (!(vict = get_player_idnum(num)))
        return FALSE;
      while(i < MAX_ASSIGNED) {
        if (CORPSE_RETRIEVAL(vict)[i++] == GET_IDNUM(ch))
          return TRUE;
      }
      return FALSE;
    }
  }
  return TRUE;
}

/* added for do_assign_player stuff. -Samhadi 5/3/00 */
int check_get_corpse(struct char_data *ch, struct obj_data *obj)
{
  int cant_take_inner_corpse = FALSE, cant_take_outer_corpse = FALSE;

  if(IS_NPC(ch)) { return TRUE; }

  if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
      (GET_OBJ_VAL(obj, 3) == 1)) {
    if ((obj->obj_flags.pid != 0) && \
        (!(can_get_corpse(ch, obj->obj_flags.pid))))
      cant_take_inner_corpse = TRUE;
  }

  if (cant_take_inner_corpse)
    return 0;

  if ((obj->in_obj != NULL) && \
      (GET_OBJ_TYPE(obj->in_obj) == ITEM_CONTAINER) && \
      (GET_OBJ_VAL(obj->in_obj, 3) == 1)) {
    if ((obj->in_obj->obj_flags.pid != 0) && \
        (!(can_get_corpse(ch, obj->in_obj->obj_flags.pid))))
      cant_take_outer_corpse = TRUE;
  }

  if (cant_take_outer_corpse)
    return -1;

  return 1;
}


/* BEGIN AUCTION */

int curbid = 0;                 /* current bid on item being auctioned */
struct obj_data *obj_selling = NULL;   /* current object for sale */
struct char_data *ch_selling = NULL;   /* current character selling obj */
struct char_data *ch_buying  = NULL;   /* current character buying the object */

void start_auction(struct char_data * ch, struct obj_data * obj, int bid)
{
  /* Take object from character and set variables */
  obj_from_char(obj);
  obj_to_room(obj, real_room(1240));
  obj_selling = obj;
  ch_selling = ch;
  ch_buying = NULL;
  curbid = bid;
  GET_BIDDER(obj) = NULL;

  /* Tell the character where his item went */
  sprintf(buf, "%s is taken from your hands to be auctioned!\r\n",
          obj_selling->short_description);
  CAP(buf);
  send_to_char(buf, ch_selling);
  if (bid_num == 999) {
    GET_OBJ_BID_NUM(obj) = 1;
  } else {
    GET_OBJ_BID_NUM(obj) = bid_num + 1;
  }
  bid_num = GET_OBJ_BID_NUM(obj);
  GET_OBJ_BID_AMNT(obj) = curbid;
  GET_OBJ_AUC_TIMER(obj) = 30;
  GET_OBJ_AUC_SELLER(obj) = GET_IDNUM(ch_selling);
  SET_BIT_AR(PLR_FLAGS(ch), PLR_SELLER);

  /* Anounce the item is being sold */
  sprintf(buf, "\r\n/cRAUCTION:: /cw%s has placed %s/cc [Bidnum: %d, Level %d]/cw up for bid at %d coins./c0\r\n",
          GET_NAME(ch_selling), obj_selling->short_description,
                   GET_OBJ_BID_NUM(obj_selling),
                   obj_selling->obj_flags.minlevel, curbid);
  CAP(buf);
  send_to_all(buf);

}

ACMD(do_auction)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  int bid = 0;

  two_arguments(argument, arg1, arg2);

  if (PLR_FLAGGED(ch, PLR_SELLER)) {
    send_to_char("You cannot auction more than one item at a time.\r\n", ch);
    return;
  }

  if (!*arg1) {
    send_to_char("Auction what?\r\n", ch);
    return;
  }
  else if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
                  send_to_char(buf, ch);
    return;
  }

  else if (!*arg2 && (bid = obj->obj_flags.cost) <= 0) {
    sprintf(buf, "What should be the minimum bid?\r\n");
    send_to_char(buf, ch);
    return;
  }
  else if (*arg2 && (bid = atoi(arg2)) <= 0) {
    send_to_char("Come on? One coin at least?\r\n", ch);
    return;
  }
  else if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
    send_to_char("You can't auction containers!\r\n", ch);
    return;
  }
  else if (IS_OBJ_STAT(obj, ITEM_NOSELL) || IS_OBJ_STAT(obj, ITEM_NOTRANSFER)) {
    send_to_char("Sorry but you can't sell that!\r\n", ch);
    return;
  }
  else {
    send_to_char(OK, ch);
    start_auction(ch, obj, bid);
    return;
  }
}

ACMD(do_bid)
{
  struct obj_data *nr;
  struct char_data *seller, *old_buyer;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int found=0, amount, item_num;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    /* display the auction menu */
    send_to_char(" /cw------------------------/ccDibrovAuctions, Inc./cw------------------------ \r\n"
                 "| /cc Auc/cw                                                               |\r\n"
                 "|/cc Number  Item                           Level     Price     Timer /cw  |\r\n"
                 "|--------------------------------------------------------------------|\r\n"
                 "|                                                                    |\r\n", ch);

    for (nr = object_list; nr; nr = nr->next) {

      if (GET_OBJ_BID_NUM(nr) >= 1) {

        strcpy(buf2, nr->short_description);
        colorless_str(buf2);

        sprintf(buf, "/cw| /c0  %-5d %-30s %-9d %-10d %-6d/cw |\r\n",
              GET_OBJ_BID_NUM(nr), buf2, nr->obj_flags.minlevel,
              GET_OBJ_BID_AMNT(nr), GET_OBJ_AUC_TIMER(nr));

        send_to_char(buf, ch);
        found++;
      }
    }
    if (found <= 0)
      send_to_char("/cw| /ccNothing is currently for sale.                                  /cw   |\r\n", ch);

    send_to_char("/cw|                                                                    |\r\n", ch);
    send_to_char(" -------------------------------------------------------------------- \r\n", ch);
    return;
  }
  if (!is_number(arg1)) {
    send_to_char("You need to specify which item you are bidding on using its au
ction number.\r\n", ch);
    send_to_char("Type \'bid\' to see what is available.\r\n", ch);
    return;
  }

  item_num = atoi(arg1);

  /* Auction identify */
  if (!*arg2) {
    for (nr = object_list; nr; nr = nr->next) {
      if (GET_OBJ_BID_NUM(nr) == item_num) {
        spell_identify(10, ch, 0, nr);
        return;
      }
    }
    send_to_char("That is not a valid bid number.\r\n", ch);
    return;
  }
  if (!is_number(arg2)) {
    send_to_char("You need to specify the amount you wish to bid.\r\n", ch);
    send_to_char("Type 'bid' to see the minimum bids on all items up for auction
.\r\n", ch);
    return;
  }
  if (item_num <= 0) {
    send_to_char("That is not a valid bid number.\r\n", ch);
    return;
  }

  /* They've specified two numbers, now lets do some bidding! */

  amount = atoi(arg2);

  for (nr = object_list; nr; nr = nr->next) {
    if (GET_OBJ_BID_NUM(nr) == item_num) {
      if (GET_OBJ_BID_AMNT(nr) + (GET_OBJ_BID_AMNT(nr)*.1) <= amount) {
        if (GET_GOLD(ch) < amount) {
          send_to_char("You haven't enough money for that!\r\n", ch);
          return;
        }
        sprintf(buf, "/cRAUCTION:: /cwYou bid %d coins on %s./c0\r\n",
                amount, nr->short_description);
        send_to_char(buf, ch);
        GET_BIDDER(nr) = ch;

        for(old_buyer=character_list;old_buyer;old_buyer=old_buyer->next) {
          if (GET_OBJ_AUC_PLAYER(nr) == GET_IDNUM(old_buyer)) {
            sprintf(buf, "/cRAUCTION:: /cwA bid of %d coins has been "
                         "placed on %s./c0\r\n",
                         GET_OBJ_BID_AMNT(nr), nr->short_description);
            send_to_char(buf, old_buyer);
          }
        }

        GET_OBJ_BID_AMNT(nr) = amount;
        GET_OBJ_AUC_PLAYER(nr) = GET_IDNUM(ch);
        GET_OBJ_AUC_TIMER(nr) = 30;
        for (seller = character_list; seller; seller = seller->next) {
          if (GET_OBJ_AUC_SELLER(nr) == GET_IDNUM(seller)) {
            sprintf(buf, "/cRAUCTION:: /cwA bid of %d coins has been placed on %s./c0\r\n",
                    GET_OBJ_BID_AMNT(nr), nr->short_description);
            send_to_char(buf, seller);
            return;
          }
        }
      }
    }
  }
  send_to_char("You've either specified a non-existing auc number, or the \r\n"
        "price you bid was less than 10% more than the current bid.\r\n", ch);
  return;
}

/* END AUCTION */


ACMD(do_taint)
{
  struct obj_data *target, *source, *next_obj;
  struct char_data *vict;
  int percent, prob;

  two_arguments(argument, arg, buf);

  if (!GET_SKILL(ch, SKILL_TAINT_FLUID)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Who's fluid do you want to taint?\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Your victim does not seem to be in the room.\r\n", ch);
    return;
  }
  if (!*buf) {
    sprintf(buf2, "Which item of %s's do you wish to taint?\r\n", GET_NAME(vict));
    send_to_char(buf2, ch);
    return;
  }
  if (!(target = get_obj_in_list_vis(vict, buf, vict->carrying))) {
    sprintf(buf2, "%s doesn't have one of those.\r\n", GET_NAME(vict));
    send_to_char(buf2, ch);
    return;
  }
  for (target = vict->carrying; target; target = next_obj) {
    next_obj = target->next_content;
    if (CAN_SEE_OBJ(ch, target) && GET_OBJ_TYPE(target) == ITEM_DRINKCON) {
      if (GET_OBJ_VAL(target, 1) <= 0) {
        sprintf(buf2, "Drat! %s's %s is currently empty. You'll wait until it gets filled up.\r\n", GET_NAME(vict), target->short_description);
        send_to_char(buf2, ch);
        return;
      }
      if (GET_OBJ_VAL(target, 3)) {
        sprintf(buf2, "The contents of %s are already poisoned.\r\nLooks like someone else already beat you to it!\r\n", target->short_description);
        send_to_char(buf2, ch);
        return;
      }
      percent = number(1, 110);
      prob = GET_SKILL(ch, SKILL_TAINT_FLUID);

      for (source = ch->carrying; source; source = next_obj) {
        next_obj = source->next_content;
        if (CAN_SEE_OBJ(ch, source) && GET_OBJ_TYPE(source) == ITEM_DRINKCON) {
          if (GET_OBJ_VAL(source, 3) <= 0) {
            send_to_char("How can you poison something with liquid that isn't poisoned?!\r\n", ch);
            return;
          }
          if (GET_OBJ_VAL(source, 3) >= 1 && percent < prob) {
            sprintf(buf2, "\r\nYou unobtrusively slip the contents of %s into %s's %s.\r\n",                source->short_description, GET_NAME(vict), target->short_description);
            send_to_char(buf2, ch);
            GET_OBJ_VAL(target, 3)++;
            GET_OBJ_VAL(source, 1) = 0;
            sprintf(buf2, "\r\n%s gives you a little wink and a chuckle and turns away.\r\n", GET_NAME(ch));
            send_to_char(buf2, vict);
            act("$n gives a little smirk as $e brushes up against $N.",
                 FALSE, ch, 0, vict, TO_NOTVICT);
            return;
          }
          send_to_char("Uh oh. Looks like you've been caught!\r\n", ch);
          act("$n just tried to poison your $p!", FALSE, ch, target, 0, TO_VICT);
          act("$n just tried to poison $N's $p!", FALSE, ch, target, vict, TO_NOTVICT);
          WAIT_STATE(ch, 10);
          return;
        }
      }
      send_to_char("You have nothing in your inventory which could be used to poison.\r\n", ch);
      return;
    }
  }
}

#define MIN_BLOOD 250
#define MAX_BLOOD 500
ACMD(do_drain)
{
  struct obj_data *obj, *vial, *item;
  struct char_data *tmp_char;
  int dotmode, blood=0, r_num;
  char arg2[MAX_INPUT_LENGTH];

  two_arguments(argument, arg, arg2);

  dotmode = find_all_dots(arg);

  vial = get_obj_in_list_vis(ch, "empty_vial", ch->carrying);

  if (!(GET_CLASS(ch) == CLASS_VAMPIRE)) {
    send_to_char("Only vampires can drain corpses.\r\n", ch);
    return;
  }

  if (!*arg) {
    send_to_char("You wish to drain which corpse?\r\n", ch);
    return;
  }

  if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &obj)){
    if (!*arg2) {
      if (GET_OBJ_VAL(obj, 3) != 1) {
        send_to_char("You can only drain corpses!\r\n", ch);
        return;
      } else
      blood = GET_OBJ_EXP(obj);
      if (blood < 0) {
        send_to_char("You cannot drain player corpses.\r\n", ch);
        return;
      } else

      blood = GET_OBJ_EXP(obj);
      sprintf(buf, "You feel a rush of vitality as you drain away "
                   "last vestiges of life that was left\r\n"
                   "in %s.\r\n\r\n"
                   "/crAaah. The Blood is the Life./c0\r\n",
                    obj->short_description);
      act("$n kneels down, drinking away the last of the life "
          "that was left within the corpse.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char(buf, ch);
      extract_obj(obj);
      GET_HIT(ch) += blood;
      GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
      return;
    } else
    if (generic_find(arg2, FIND_OBJ_INV, ch, &tmp_char, &vial)) {
      if (vial->item_number == real_object(80)) {
        extract_obj(vial);
        r_num = real_object(81);
        item = read_object(r_num, REAL);
        obj_to_char(item, ch);
        send_to_char("You drain the corpse's blood into the vial.\r\n", ch);
        act("$n drains the last of the blood from the corpse into a vial.",
             TRUE, ch, 0, 0, TO_ROOM);
        extract_obj(obj);
        return;
      }
    }
    else {
        send_to_char("You need the vials sold in the Poor Alley in Midgaard.\r\n", ch);
        send_to_char("You can fill them with the corpse's blood.\r\n", ch);
        return;
    }
  }
  else
  send_to_char("You cannot find that for which you search.\r\n", ch);
}


#define MIN_MANA_GAIN 50
#define MAX_MANA_GAIN 300
ACMD(do_soulsuck)
{
  struct obj_data *obj;
  struct char_data *tmp_char;
  int blood=0, chance = 0, prob = 0;

  one_argument(argument, arg);

  if (GET_SKILL(ch, SKILL_SOULSUCK) < 1)
  {
	  send_to_char("Huh?!\r\n", ch);
	  return;
  }

  chance = number(1, 110);
  prob   = GET_SKILL(ch, SKILL_SOULSUCK);

  if (!(GET_CLASS(ch) == CLASS_DEMON)) {
    send_to_char("Only demons can suck the souls from corpses.\r\n", ch);
    return;
  }

  if (!*arg) {
    send_to_char("You wish to suck the soul from which corpse?\r\n", ch);
    return;
  }
  blood = GET_OBJ_EXP(obj);

  if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &tmp_char, &obj)){
    if (GET_OBJ_VAL(obj, 3) != 1) {
      send_to_char("You can only suck the soul from corpses!\r\n", ch);
      return;
    } else
    if (blood < 0) {
      send_to_char("You cannot suck the soul from a player corpse.\r\n", ch);
      return;
    }
	else if (prob > chance)
	{
		blood = GET_OBJ_EXP(obj);
		sprintf(buf, "You feel a rush of arcane energy as you suck out "
			"the soul of %s.\r\n\r\n", obj->short_description);
		act("$n kneels down, sucking out the soul from the corpse.",
			TRUE, ch, 0, 0, TO_ROOM);
		send_to_char(buf, ch);
		extract_obj(obj);
		GET_MANA(ch) += blood;
		GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
		return;
	} else {
		send_to_char("You failed.\r\n", ch);
		return;
	}
  }
}


ACMD(do_castout)
{
  struct obj_data *pole;
  int fail;

  if (PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char("You are already fishing!\r\n", ch);
    return;
  }
  if (!(pole = GET_EQ(ch, WEAR_HOLD)) ||
      (GET_OBJ_TYPE(pole) != ITEM_POLE)) {
    send_to_char("You need to be holding a fishing pole first.\r\n", ch);
    return;
  }
  if (!ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH) &&
      !ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH)) {
    send_to_char("This is not a good place to fish, you'll want to find a"
                 " better spot.\r\n", ch);
    return;
  }
  fail = number(1, 10);
  if (fail <= 3) {
    send_to_char("You pull your arm back and try to cast out your line, but "
                 "it gets all tangled up.\r\nTry again.\r\n", ch);
    act("$n pulls $s arm back, trying to cast $s fishing line out into the "
        "water,\r\nbut ends up just a bit tangled.\r\n",
         FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  /* Ok, now they've gone through the checks, now set them fishing */
  SET_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
  send_to_char("You cast your line out into the water, hoping for a bite.\r\n", ch);
  act("$n casts $s line out into the water, hoping to catch some food.\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  return;
}

ACMD(do_reelin)
{
  int success, f_num, fish_num;
  struct obj_data *fish;

  if (!PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char("You aren't even fishing!\r\n", ch);
    return;
  }
  if (!PLR_FLAGGED(ch, PLR_FISH_ON)) {
    send_to_char("You reel in your line, but alas... nothing on the end.\r\n"
                 "Better luck next time.\r\n", ch);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    act("$n reels $s line in, but with nothing on the end.\r\n",
        FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  /* Ok, they are fishing and have a fish on */
  success = number(1, 10);

  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
  REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);

  if (success <= 6) {
    send_to_char("You reel in your line, putting up a good fight, but you "
                 "lose him!\r\nTry again?\r\n", ch);
    act("$n reels $s line in, fighting with whatever is on the end, but loses "
        "the catch.\r\n", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH)) {
    fish_num = number(10030, 10039);
    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    sprintf(buf, "You reel in %s! Nice catch!\r\n", fish->short_description);
    act("Wow! $n reels in a helluva catch! Looks like $p!\r\n",
        FALSE, ch, fish, 0, TO_ROOM);
    send_to_char(buf, ch);
    obj_to_char(fish, ch);
    return;
  } else
  if (ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH)) {
    fish_num = number(10040, 10050);
    f_num = real_object(fish_num);
    fish = read_object(f_num, REAL);
    sprintf(buf, "You reel in %s! Nice catch!\r\n", fish->short_description);
    send_to_char(buf, ch);
    obj_to_char(fish, ch);
    return;
  } else
  send_to_char("You should never see this message, please report it.\r\n", ch);
  return;
}

ACMD(do_connect)
{
  struct obj_data *piece, *connect_obj, *final_obj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int piece_num;

  two_arguments(argument, arg1, arg2);

  if (!*arg1 || !*arg2) {
    send_to_char("Syntax: connect <piece> <main item>\r\n", ch);
    return;
  }

  if ((piece=get_obj_in_list_vis(ch, arg1, ch->carrying)) == NULL) {
    sprintf(buf, "You don't seem to have a %s in your inventory.\r\n", arg1);
    send_to_char(buf, ch);
    return;
  }
  if ((connect_obj=get_obj_in_list_vis(ch, arg2, ch->carrying)) == NULL) {
    sprintf(buf, "You don't seem to have a %s in your inventory.\r\n", arg2);
    send_to_char(buf, ch);
    return;
  }
  if (GET_OBJ_TYPE(piece) != ITEM_PIECE) {
    act("$p is not a valid piece to use in connections.\r\n",
        FALSE, ch, piece, 0, TO_CHAR);
    return;
  }
  if (GET_OBJ_VAL(piece, 0) != GET_OBJ_VNUM(connect_obj)) {
    act("$p does not fit together with $P, but it does fit with "
        "a different item.\r\n", FALSE, ch, piece, connect_obj, TO_CHAR);
    return;
  }
  act("You hold $p and $P, bringing them together to form one item.\r\n",
      FALSE, ch, piece, connect_obj, TO_CHAR);
  act("$n holds $p and $P, bring them together to form one item!\r\n",
      FALSE, ch, piece, connect_obj, TO_ROOM);
  extract_obj(piece);
  extract_obj(connect_obj);
  piece_num = real_object(GET_OBJ_VAL(piece, 1));
  final_obj = read_object(piece_num, REAL);
  obj_to_char(final_obj, ch);
  return;
}
