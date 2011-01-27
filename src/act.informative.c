/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "screen.h"
#include "olc.h"
#include "clan.h"
#include "quest.h"
#include "arena.h"
#include "dg_scripts.h"

/* extern variables */
extern int top_of_objt;
extern struct obj_data *obj_proto ;
extern struct index_data *obj_index;
extern struct room_data *world;
extern struct index_data *mob_index;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct command_info cmd_info[];
extern struct str_app_type str_app[]; /* For the CAN_CARRY... macros. */
extern struct quest_definition quests[]; /* From quest.c */
extern int active_quest; /* From quest.c */
extern int active_quest_state; /* From quest.c */
extern int double_exp;
extern char *qstates[]; /* From quest.c */
extern int in_arena; /* arena.c */
extern int lo_lim; /* arena.c */
extern int hi_lim; /* arena.c */
extern const char *mana_rept_msgs[];
extern const char *hit_rept_msgs[];
extern const char *move_rept_msgs[];

extern char *credits;
extern char *news;
extern char *info;
extern char *motd;
extern char *imotd;
extern char *wizlist;
extern char *immlist;
extern char *policies;
extern char *handbook;
extern char *dirs[];
extern char *AEdirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *connected_types[];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern char *room_bits[];
extern char *sector_types[];
extern char *pc_class_types[]; /* New score */
extern char *pc_race_types[]; /* New score */
extern char *npc_class_types[];
extern char *npc_race_types[];
extern char *genders[];
extern char *god_lvls[];
extern char *god_types[];
extern char *hometowns[];
extern int most_on;
extern int port;
extern char *skills[];
extern char *spells[];
extern char *chants[];
extern char *prayers[];
extern char *songs[];
extern char *rank[][3];
extern char *apply_types[];
extern char *affected_bits[];
extern struct zone_data *zone_table;

/* Local Funcs... */
char *get_align_desc(int alignment);
char *get_desc_from_roll_value(int roll_value);
char *get_role_text(int level);
const char *show_equip(struct char_data *ch, int wearpos);
ACMD(do_reveal_nature);
bool S_MOB_FLAGGED(struct char_data *ch, int flag);
bool S_AFF_FLAGGED(struct char_data *ch, int flag);


/* extern functions */
long find_class_bitvector(char arg);
int level_exp(int class, int level);
void proc_color(char *inbuf, int color_lvl);
char *title_male(int class, int level);
char *title_female(int class, int level);
void look_sky(struct char_data *ch);
void list_ability(struct char_data * ch, int num, int type);
void colorless_str(char *str);
void warn_about_owned_zone(struct char_data *ch);

const char *snow[] = {
	"BUG PLEASE REPORT",
	"a sprinkling",
	"a light amount",
	"an ankle deep covering",
	"a knee deep covering",
      "a thigh deep covering",
	"a waist deep covering",
	"a chest deep covering",
	"\n"
};

void show_obj_to_char(struct obj_data * object, struct char_data * ch,
			int mode)
{
  bool found;
  char buf5[256];

  *buf = '\0';
  if ((mode == 0) && object->description) {
    strcpy(buf, object->description);

  }
  else if (object->short_description && ((mode == 1) ||
				 (mode == 2) || (mode == 3) || (mode == 4)))
  {
  if(GET_OBJ_TYPE(object) == ITEM_RUNE && real_room(GET_OBJ_VAL(object, 1))) {
    sprintf(buf5, "A rune to %s", world[real_room(GET_OBJ_VAL(object, 1))].name);
    strcpy(buf, buf5);
  }
  else {
     strcpy(buf, object->short_description);
  }

  }
  else if (mode == 5) {
    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
      if (object->action_description) {
      	strcpy(buf, "There is something written upon it:\r\n\r\n");
      	strcat(buf, object->action_description);
      	page_string(ch->desc, buf, 1);
      } else
      	act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    } else if (GET_OBJ_TYPE(object) != ITEM_DRINKCON) {
      strcpy(buf, "You see nothing special..");
    } else			/* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
      strcpy(buf, "It looks like a drink container.");
  }
  if (mode != 3) {
    found = FALSE;
    if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
      strcat(buf, " /cc(invisible)/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_QUEST)) {
      strcat(buf, " /cG(quest)/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_BLESS)) {
      strcat(buf, "/cC ..It shines with a holy light!/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_MAGIC)) {
      strcat(buf, " /cm..It pulses with arcane energies!/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_GLOW)) {
      strcat(buf, " /cc..It has a soft glowing aura!/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_HUM)) {
      strcat(buf, " /cW..It emits a faint humming sound!/c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_VAMP_CAN_SEE)) {
      strcat(buf, " /cl..a shroud of blackness clings to it./c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_TITAN_CAN_SEE)) {
      strcat(buf, " /cy..a great aura of power radiates from it./c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_SAINT_CAN_SEE)) {
      strcat(buf, " /cW..an aura of holy purity emits from it./c0");
      found = TRUE;
    }
    if (IS_OBJ_STAT(object, ITEM_DEMON_CAN_SEE)) {
      strcat(buf, " /cr..an aura of malice hangs about it./c0");
      found = TRUE;
    }
  }
  strcat(buf, "\r\n");
  page_string(ch->desc, buf, 1);
}

/*
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
		           int show)
{
  struct obj_data *i;
  bool found;

  found = FALSE;
  for (i = list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}
*/
// obj vnums for final scrolls and potions
#define SCRIBE_PROTO 30
#define BREW_PROTO   31
/* New version for obj stacking... */
void list_obj_to_char(struct obj_data * list, struct char_data * ch, int mode,
		           bool show) {
  struct obj_data *i, *j;
  char buf[10];
  bool found = FALSE;
  int num;


  for (i = list; i; i = i->next_content) {
  num = 0;
  for (j = list; j != i; j = j->next_content)
    if (j->item_number == NOTHING) {
      if (strcmp(j->short_description, i->short_description) == 0 && GET_OBJ_TYPE(j) != ITEM_RUNE)
      break;
    } else if ((GET_OBJ_VNUM(j) == SCRIBE_PROTO) || (GET_OBJ_VNUM(j) == BREW_PROTO)) {
      if (strcmp(j->short_description, i->short_description) == 0 && GET_OBJ_TYPE(j) != ITEM_RUNE)
      break;
    } else if (j->item_number == i->item_number && GET_OBJ_TYPE(j) != ITEM_RUNE)
    break;
    if (j!=i)
      continue;
    for (j = i; j; j = j->next_content)
      if (j->item_number == NOTHING) {
        if (strcmp(j->short_description, i->short_description) == 0 && GET_OBJ_TYPE(j) != ITEM_RUNE)
	   num++;
      } else if ((GET_OBJ_VNUM(j) == SCRIBE_PROTO) || (GET_OBJ_VNUM(j) == BREW_PROTO)) {
        if (strcmp(j->short_description, i->short_description) == 0 && GET_OBJ_TYPE(j) != ITEM_RUNE)
           num++;
      } else if (j->item_number == i->item_number && GET_OBJ_TYPE(j) != ITEM_RUNE)
	        num++;
            if(GET_OBJ_TYPE(i) == ITEM_RUNE) num = 1;

    if (CAN_SEE_OBJ(ch, i) && can_see_quest_obj(ch, i)) {
      if (num!=1) {
        sprintf(buf,"(%d) ",num);
        send_to_char(buf,ch);
      }
      if (IS_OBJ_STAT(i, ITEM_NOT_SEEN)) {
      } else 
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if (!found && show)
    send_to_char(" Nothing.\r\n", ch);
}


void diag_char_to_char(struct char_data * i, struct char_data * ch)
{
  int percent;

  if (GET_MAX_HIT(i) > 0)
    percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
  else
    percent = -1;		/* How could MAX_HIT be < 1?? */

  strcpy(buf, PERS(i, ch));
  CAP(buf);

  if (percent >= 100)
    strcat(buf, " is in excellent condition.\r\n");
  else if (percent >= 90)
    strcat(buf, " has a few scrapes.\r\n");
  else if (percent >= 75)
    strcat(buf, " has some small wounds and bruises.\r\n");
  else if (percent >= 50)
    strcat(buf, " has quite a few wounds.\r\n");
  else if (percent >= 30)
    strcat(buf, " sways dizzily.\r\n");
  else if (percent >= 15)
    strcat(buf, "'s eyes are glazed over.\r\n");
  else if (percent >= 0)
    strcat(buf, " is in terrible shape.\r\n");
  else
    strcat(buf, " is going down!\r\n");

  send_to_char(buf, ch);
}


void look_at_char(struct char_data * i, struct char_data * ch)
{
  int j, found;
  //struct obj_data *tmp_obj;

  if (PLR_FLAGGED(i, PLR_TOAD)) {
    act("You see a toad that looks amazingly like $m.", FALSE, i, 0, ch, TO_VICT);
    return;
  }
  if (AFF_FLAGGED(i, AFF_BURROW)) {
    send_to_char("You do not see that here.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_IMPEN) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("You can't see anything. The darkness is impenetrable.\r\n", ch);
    return;
  }

  if (!ch->desc)
    return;

   if (i->player.description)
    send_to_char(i->player.description, ch);
  else
    act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);

  diag_char_to_char(i, ch);

  found = FALSE;
  for (j = 0; !found && j < NUM_WEARS; j++)
    if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j)))
      found = TRUE;

  if (found) {
    act("\r\n$n is using:", FALSE, i, 0, ch, TO_VICT);
    for (j = 0; j < NUM_WEARS; j++)
      if (GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) {
      	send_to_char(where[j], ch);
      	show_obj_to_char(GET_EQ(i, j), ch, 1);
      }
  }
  if (ch != i && (GET_CLASS(ch) == CLASS_THIEF || 
      GET_CLASS(ch) == CLASS_ASSASSIN ||
      GET_CLASS(ch) == CLASS_SHADOWMAGE ||
      GET_CLASS(ch) == CLASS_ACROBAT ||
      GET_CLASS(ch) == CLASS_RANGER ||
	  GET_CLASS(ch) == CLASS_MERCENARY ||
      GET_CLASS(ch) >= CLASS_VAMPIRE ||
      GET_LEVEL(ch) >= LVL_IMMORT)) {
    found = FALSE;
    act("\r\nYou attempt to peek at $s inventory:", FALSE, i, 0, ch, TO_VICT);
	if (i->carrying) {
		list_obj_to_char(i->carrying, ch, 1, TRUE);
		found = TRUE;
	}
	/*
    for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 20) < GET_LEVEL(ch))) {
      	show_obj_to_char(tmp_obj, ch, 1);
      	found = TRUE;
      }
    }
*/
    if (!found)
      send_to_char("You can't see anything.\r\n", ch);
  }
  if (AFF_FLAGGED(i, AFF_CLOUD))
    act("/cwA /cLdark cloud/cw hangs over $S right shoulder./c0\r\n",
        TRUE, ch, 0, i, TO_CHAR);
  if (AFF_FLAGGED(i, AFF_UNFURLED))
    act("/crWings sprout grotesquely from $S back./c0\r\n",
        TRUE, ch, 0, i, TO_CHAR);
}


void list_one_char(struct char_data * i, struct char_data * ch)
{
  int percent, mob_name;
  struct char_data *mob;

  char *positions[] = {
    "/cw is lying here, dead.",
    "/cw is lying here, mortally wounded.",
    "/cw is lying here, incapacitated.",
    "/cw is lying here, stunned.",
    "/cw is sleeping here.",
    "/cw is resting here.",
    "/cw is sitting here.",
    "!FIGHTING!",
    "/cw is standing here."
  };

  if (!IS_NPC(i) && !i->desc && GET_LEVEL(ch) < LVL_IMMORT)
    return;

  if (IS_AFFECTED(i, AFF_BURROW) && GET_LEVEL(ch) < LVL_IMMORT)
    return;

  /* DECEIVE */
  if (AFF_FLAGGED(i, AFF_DECEIVE)) {
    mob_name = number(4501, 4508);
    mob = read_mobile(mob_name, VIRTUAL);
    sprintf(buf, "%s is standing here.\r\n/c0", mob->player.short_descr);
    CAP(buf);
    send_to_char(buf, ch);
    return;
  }

  if (GET_RIDER(i) && GET_RIDER(i) != ch)
    return;

  if (IS_NPC(i) && i->player.long_descr && GET_POS(i) == GET_DEFAULT_POS(i)) {
    if (AFF_FLAGGED(i, AFF_INVISIBLE))
      strcpy(buf, "*");
    else
      *buf = '\0';

    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
      if (GET_ALIGNMENT(i) <= -601)
        strcat(buf, "/cw(/cREvil/cw Aura) ");
      if (GET_ALIGNMENT(i) <= -201 && GET_ALIGNMENT(i) >= -600)
        strcat(buf, "/cw(/cLBad/cw Aura) ");
      if (GET_ALIGNMENT(i) <= 200 && GET_ALIGNMENT(i) >= -200)
        strcat(buf, "/cw(/cyNeutral/cw Aura) ");
      if (GET_ALIGNMENT(i) <= 600 && GET_ALIGNMENT(i) >= 201)
        strcat(buf, "/cw(/cwGood/cw Aura) ");
      else if (IS_GOOD(i))
        strcat(buf, "/cw(/cCHoly/cw Aura) ");
    }
    /* MOBFLAGS */
    if (PRF_FLAGGED(ch, PRF_MOBFLAGS)) {
      sprintf(buf2, "/cc(%d) /c0", GET_MOB_VNUM(i));
      strcat(buf, buf2);
    }

	if (GET_CLASS(ch) < CLASS_VAMPIRE)
	{
		if ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 19)
			strcat(buf, "/cc");
		else
			strcat(buf, "/cw");
	} else {
		if ((150 - GET_LEVEL(i)) <= 19)
			strcat(buf, "/cc");
		else
			strcat(buf, "/cw");
	}
    strcat(buf, i->player.long_descr);

    if (IS_NPC(i)) {

      if (GET_MAX_HIT(i) > 0)
        percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
      else
        percent = -1;               /* How could MAX_HIT be < 1?? */

      buf[strlen(buf) - 2] = '\0'; /* get rid of the newline */ 


      if (IS_NPC(i) && GET_QUESTMOB(ch) == GET_MOB_VNUM(i))
        strcat(buf, " /cR[Target]/c0");

      if (percent > 100)
        strcat(buf, " /cc(exceptional)/c0");
      else if (percent == 100)
        strcat(buf, "");
      else if (percent >= 95)
        strcat(buf, " /cc(excellent)/c0");
      else if (percent >= 90)
        strcat(buf, " /cr(scratched)/c0");
      else if (percent >= 75)
        strcat(buf, " /cr(bruised)/c0");
      else if (percent >= 50)
        strcat(buf, " /cr(wounded)/c0");
      else if (percent >= 30)
        strcat(buf, " /cr(nasty)/c0");
      else if (percent >= 15)
        strcat(buf, " /cr(hurt)/c0");
      else if (percent >= 0)
        strcat(buf, " /cr(awful)/c0");
      else
        strcat(buf, " /cr(dying)/c0");

      strcat(buf, "\r\n"); /* replace the newline */  
    }

    send_to_char(buf, ch);

    if (AFF_FLAGGED(i, AFF_SANCTUARY))
      act("/cW...$e glows with a bright light!/c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DEFLECT))
      act("/cm...$e is protected by a magical deflective shield./c0",
           FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_BLAZEWARD))
      act("/cY...$e glows with a brilliant golden blaze!/c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_INDESTR_AURA))
      act("/cW...$e glows with an indestructable aura!/c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_BLIND))
      act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_FLY) || AFF_FLAGGED(i, AFF_UNFURLED))
      act("/cC...$e is hovering in the air./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_HOVER))
      act("/cr...$e is hovering in midair./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_FIRESHIELD))
      act("/cR...$e is surrounded by a shroud of flames./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DARKWARD))
      act("/cL...a dark ward surrounds $m./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DIVINE_NIMBUS))
      act("/cc...a divine nimbus surrounds $m./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_CHAOSARMOR))
      act("/cL...$e is surrounded by chaotic shield of energy./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_SACRED_SHIELD))
      act("/cL...$e is surrounded by a sacred shroud of protection./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_COLD))
      act("/cW...$e is encased in a solid cone of ice./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_VAMP_AURA))
      act("/cl...$e is enshrouded in a hazy darkness./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_NETHERBLAZE))
      act("/cR...$e is surrounded by a raging blaze./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_EARTH_SHROUD))
      act("/cy...the power of the earth enshrouds $m./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_MESMERIZED))
      act("/cl...$e is in a motionless trance./c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_ABSORB))
      act("/cy...$e is kneeling on the ground, both palms upon the earth./c0",
          FALSE, i, 0, ch, TO_VICT);

    return;
  }
  if (IS_NPC(i)) {
    strcpy(buf, i->player.short_descr);
    CAP(buf);

 } else if (PLR_FLAGGED(i, PLR_TOAD)) {
   strcpy(buf, CCGRN(ch, C_NRM));
   strcat(buf, "An ugly toad here that looks amazingly like ");
   strcat(buf, GET_NAME(i));

 } else if (AFF_FLAGGED(i, AFF_INCOGNITO))
      sprintf(buf, "/cwSomeone/c0");
   else
      sprintf(buf, "/cw%s %s %s/c0", rank[(int)GET_RANK(i)][(int)GET_SEX(i)], 
              i->player.name, GET_TITLE(i));

  if (AFF_FLAGGED(i, AFF_INVISIBLE))
    strcat(buf, " (invisible)");
  if (AFF_FLAGGED(i, AFF_HIDE))
    strcat(buf, " (hidden)");
  if (!IS_NPC(i) && !i->desc)
    strcat(buf, " (linkless)");
  if (PLR_FLAGGED(i, PLR_WRITING))
    strcat(buf, " (writing)");
  if (PRF_FLAGGED(i, PRF_AFK))
    strcat(buf, " /cR(AFK)/c0");
  
  if (GET_POS(i) != POS_FIGHTING)
    if (GET_MOUNT(i))
      sprintf(buf, "%s/cw is here, riding on the back of %s./c0", buf,
              GET_NAME(GET_MOUNT(i)));
    else if (GET_RIDER(i) == ch)
      sprintf(buf, "%s/cw is here, ridden by you./c0", buf);
    else
      strcat(buf, positions[(int) GET_POS(i)]);
  else {
    if (FIGHTING(i)) {
      strcat(buf, " is here, fighting ");
      if (FIGHTING(i) == ch)
        strcat(buf, "YOU!");
      else {
      	if (i->in_room == FIGHTING(i)->in_room)
      	  strcat(buf, PERS(FIGHTING(i), ch));
      	else
      	  strcat(buf, "someone who has already left");
      	strcat(buf, "!");
      }
    } else			/* NIL fighting pointer */
      strcat(buf, " is here struggling with thin air.");
  }

  if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
    if (IS_EVIL(i))
      strcat(buf, " /cw(/cRRed/cw Aura)");
    else if (IS_GOOD(i))
      strcat(buf, " /cw(/cBBlue/cw Aura)");
  }
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if (AFF_FLAGGED(i, AFF_SANCTUARY))
    act("/cW...$e glows with a bright light!/c0", FALSE, i, 0, ch, TO_VICT);
    if (AFF_FLAGGED(i, AFF_DEFLECT))
      act("/cm...$e is protected by a magical deflective shield./c0",
           FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_BLAZEWARD))
    act("/cW...$e glows with a brilliant golden blaze!/c0",
         FALSE, i, 0, ch, TO_VICT); 
  if (AFF_FLAGGED(i, AFF_INDESTR_AURA))
    act("/cW...$e glows with an indestructable aura!/c0", 
         FALSE, i, 0, ch, TO_VICT); 
  if (AFF_FLAGGED(i, AFF_BLIND))
    act("...$e is groping around blindly!", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_FLY) || AFF_FLAGGED(i, AFF_UNFURLED))
    act("/cC...$e is floating in the air!/c0", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_HOVER))
    act("/cr...$e is hovering in midair!/c0", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_FIRESHIELD))
    act("/cR...$e is surrounded by a shroud of flames!/c0", FALSE, i, 0, ch, TO_VICT);
  if (AFF_FLAGGED(i, AFF_COLD))
    act("/cW...$e is encased in a solid cone of ice./c0", FALSE, i, 0, ch, TO_VICT);
  if (IS_NPC(i))
	  if (MOB_FLAGGED(i, MOB_INSTAGGRO))
		  hit(i, ch, TYPE_UNDEFINED);
}



void list_char_to_char(struct char_data * list, struct char_data * ch)
{
  struct char_data *i;

  if (IS_AFFECTED(ch, AFF_BURROW)) {
    send_to_char("You must rise from the earth to see who is near.\r\n", ch);
    return;
  }

  for (i = list; i; i = i->next_in_room)
    if (ch && ch != i) {
      if (CAN_SEE(ch, i))
      	list_one_char(i, ch);
	  if (IS_NPC(i)) {
			if (GET_LEVEL(ch) < LVL_IMMORT)
				if (!IS_AFFECTED(ch, AFF_SNEAK)) {
					if (!IS_AFFECTED(ch, AFF_FOG_MIST_AURA)) {
						if (!IS_AFFECTED(ch, AFF_MIST_FORM)) {
							if (MOB_FLAGGED(i, MOB_INSTAGGRO)) {
								act("$n instantly attacks you.\r\n", FALSE, i, 0, ch, TO_VICT);
								act("$n instantly attacks $N.\r\n", FALSE, i, 0, ch, TO_NOTVICT); 
								hit(i, ch, TYPE_UNDEFINED);
								update_pos(ch);
							}
						}
					}
				}
	  }
      else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch) &&
	       AFF_FLAGGED(i, AFF_INFRAVISION))
	      send_to_char("You see a pair of glowing red eyes looking your way.\r\n", ch);
	}
}


void do_auto_exits(struct char_data * ch)
{
  int door;

  *buf = '\0';

  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
        !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN))
      sprintf(buf, "%s%s ", buf, AEdirs[door]);
//      sprintf(buf, "%s%c ", buf, LOWER(*dirs[door]));

  sprintf(buf2, "%s[ Exits: %s]%s/c0\r\n", CCCYN(ch, C_NRM),
	  *buf ? buf : "None! ", CCNRM(ch, C_NRM));

  send_to_char(buf2, ch);
}


ACMD(do_exits)
{
  int door;

  *buf = '\0';

  if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
    return;
  }
  for (door = 0; door < NUM_OF_DIRS; door++)
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
      if (GET_LEVEL(ch) >= LVL_IMMORT) {
	sprintf(buf2, "%-5s - [%5d] %s\r\n", dirs[door],
		world[EXIT(ch, door)->to_room].number,
		world[EXIT(ch, door)->to_room].name);
        strcat(buf, CAP(buf2));
      } else {
        if (!IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN)) {
          sprintf(buf2, "%-5s - ", dirs[door]);
          /*MJ*/
          if (IS_IMPEN(EXIT(ch, door)->to_room))
            strcat(buf2, "No way to tell in this darkness.\r\n");
	  if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
	    strcat(buf2, "Too dark to tell\r\n");
	  else {
	    strcat(buf2, world[EXIT(ch, door)->to_room].name);
	    strcat(buf2, "\r\n");
          }
          strcat(buf, CAP(buf2));
	}
      }
    }
  send_to_char("Obvious exits:\r\n", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char(" None.\r\n", ch);
}

ACMD(do_map);

char *blood_messages[] = {
  "Should never see this.",
  "/crSpots of blood stain the ground./c0",
  "/crSpots of blood stain the ground./c0",
  "/crThere is a large red blood stain on the ground./c0",
  "/crThere is a large red blood stain on the ground./c0",
  "/crAll around you is coated in spilled blood./c0",
  "/crAll around you is coated in spilled blood./c0",
  "/crBlood is splashed everywhere, there must have been an awesome battle here!/c0",
  "/crBlood is splashed everywhere, there must have been an awesome battle here!/c0",
  "/crHacked body parts lie soaking in blood on the ground./c0",
  "/crHacked body parts lie soaking in blood on the ground./c0",
  "\n"
};

void look_at_room(struct char_data * ch, int ignore_brief)
{
  if (!ch->desc)
    return;

  if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    return;
  /*MJ*/
  }
  if (IS_AFFECTED(ch, AFF_BURROW)) {
    send_to_char("You are surrounding by the nourishing earth.\r\n"
                 "You can see nothing.\r\n", ch);
    return;
  }
  if (IS_IMPEN(ch->in_room) && GET_LEVEL(ch) < LVL_IMMORT) {
    send_to_char("You are surrounded by an impenetrable darkness...\r\n", ch);
    return;
  } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You see nothing but infinite darkness...\r\n", ch);
    return;
  }
  send_to_char(CCCYN(ch, C_NRM), ch);
  if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
    sprintbitarray(ROOM_FLAGS(ch->in_room), room_bits, RF_ARRAY_MAX, buf);
    sprinttype(SECT(ch->in_room), sector_types, buf1);
    sprintf(buf2, "[%5d] %s [ %s] [ %s ]", world[ch->in_room].number,
	    world[ch->in_room].name, buf, buf1);
    send_to_char(buf2, ch);
  } else
    send_to_char(world[ch->in_room].name, ch);

  send_to_char(CCNRM(ch, C_NRM), ch);
  send_to_char("\r\n", ch);

  if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief ||
      ROOM_FLAGGED(ch->in_room, ROOM_DEATH))
    send_to_char(world[ch->in_room].description, ch);

  /* vampire haven rooms */
  if (ROOM_FLAGGED(ch->in_room, ROOM_HAVEN))
    send_to_char("\r\n/crThe entire room is shrouded in a red mist./c0\r\n", ch);
  
  /* scorched earth rooms */
  if (ROOM_FLAGGED(ch->in_room, ROOM_SCORCHED))
    send_to_char("\r\n/cyThe earth /crsimmers and swells/cy in this place./c0\r\n", ch);

  if(weather_info.ground_snow && weather_info.ground_snow < 7 && OUTSIDE(ch)) {
    sprintf(buf, "The ground is covered with %s of snow.\r\n", snow[weather_info.ground_snow]);
    send_to_char(buf, ch);
  }
  else if(weather_info.ground_snow > 7 && OUTSIDE(ch)) {
    send_to_char("The ground is covered with snow as far as your eyes can see.\r\n", ch);
  }

  if (RM_BLOOD(ch->in_room) > 0)
    act(blood_messages[RM_BLOOD(ch->in_room)], FALSE, ch, 0, 0, TO_CHAR);


  //warn_about_owned_zone(ch);

  if (PRF_FLAGGED(ch, PRF_AUTOMAP))
    do_map(ch, 0, 0, 0);

  /* autoexits */
  if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
    do_auto_exits(ch);

  /* now list characters & objects */
  send_to_char(CCGRN(ch, C_NRM), ch);
  list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);
  send_to_char(CCYEL(ch, C_NRM), ch);
  list_char_to_char(world[ch->in_room].people, ch);
  send_to_char(CCNRM(ch, C_NRM), ch);
}



void look_in_direction(struct char_data * ch, int dir)
{
  if (EXIT(ch, dir)) {
    if (EXIT(ch, dir)->general_description)
      send_to_char(EXIT(ch, dir)->general_description, ch);
    else
      send_to_char("You see nothing special.\r\n", ch);

    if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "The %s is closed.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    } else if (IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) && EXIT(ch, dir)->keyword) {
      sprintf(buf, "The %s is open.\r\n", fname(EXIT(ch, dir)->keyword));
      send_to_char(buf, ch);
    }
  } else
    send_to_char("Nothing special there...\r\n", ch);
}



void look_in_obj(struct char_data * ch, char *arg)
{
  struct obj_data *obj = NULL;
  struct char_data *dummy = NULL;
  int amt, bits;

  if (!*arg)
    send_to_char("Look in what?\r\n", ch);
  else if (!(bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
				 FIND_OBJ_EQUIP, ch, &dummy, &obj))) {
    sprintf(buf, "There doesn't seem to be %s %s here.\r\n", AN(arg), arg);
    send_to_char(buf, ch);
  } else if ((GET_OBJ_TYPE(obj) != ITEM_DRINKCON) &&
	     (GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) &&
	     (GET_OBJ_TYPE(obj) != ITEM_JAR) &&
	     (GET_OBJ_TYPE(obj) != ITEM_CONTAINER))
    send_to_char("There's nothing inside that!\r\n", ch);
  else {
    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER ||
        GET_OBJ_TYPE(obj) == ITEM_JAR) {
      if (IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED))
	send_to_char("It is closed.\r\n", ch);
      else {
	send_to_char(fname(obj->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	  send_to_char(" (carried): \r\n", ch);
	  break;
	case FIND_OBJ_ROOM:
	  send_to_char(" (here): \r\n", ch);
	  break;
	case FIND_OBJ_EQUIP:
	  send_to_char(" (used): \r\n", ch);
	  break;
	}

        if (GET_OBJ_TYPE(obj) == ITEM_JAR) {
          send_to_char("/cc\r\nFloating in a pool of glowing liquid:/c0\r\n", ch);
          list_obj_to_char(obj->contains, ch, 2, TRUE);
        } else
	list_obj_to_char(obj->contains, ch, 2, TRUE);
      }
    } else {		/* item must be a fountain or drink container */
      if (GET_OBJ_VAL(obj, 1) <= 0)
	send_to_char("It is empty.\r\n", ch);
      else {
	if (GET_OBJ_VAL(obj,0) <= 0 || GET_OBJ_VAL(obj,1)>GET_OBJ_VAL(obj,0)) {
	  sprintf(buf, "Its contents seem somewhat murky.\r\n"); /* BUG */
	} else {
	  amt = (GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0);
	  sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
	  sprintf(buf, "It's %sfull of a %s liquid.\r\n", fullness[amt], buf2);
	}
	send_to_char(buf, ch);
      }
    }
  }
}



char *find_exdesc(char *word, struct extra_descr_data * list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word, i->keyword))
      return (i->description);

  return NULL;
}


/*
 * Given the argument "look at <target>", figure out what object or char
 * matches the target.  First, see if there is another char in the room
 * with the name.  Then check local objs for exdescs.
 */
void look_at_target(struct char_data * ch, char *arg)
{
  int bits, found = 0, j;
  struct char_data *found_char = NULL;
  struct obj_data *obj = NULL, *found_obj = NULL;
  char *desc;

  if (!ch->desc)
    return;

  if (!*arg) {
    send_to_char("Look at what?\r\n", ch);
    return;
  }
  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP |
		      FIND_CHAR_ROOM, ch, &found_char, &found_obj);

  /* Is the target a character? */
  if (found_char != NULL) {
    look_at_char(found_char, ch);
    if (ch != found_char) {
      if (CAN_SEE(found_char, ch))
	act("$n looks at you.", TRUE, ch, 0, found_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, found_char, TO_NOTVICT);
    }
    return;
  }
  /* Does the argument match an extra desc in the room? */
  if ((desc = find_exdesc(arg, world[ch->in_room].ex_description)) != NULL) {
    page_string(ch->desc, desc, 0);
    return;
  }
  /* Does the argument match an extra desc in the char's equipment? */
  for (j = 0; j < NUM_WEARS && !found; j++)
    if (GET_EQ(ch, j) && CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
      if ((desc = find_exdesc(arg, GET_EQ(ch, j)->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  /* Does the argument match an extra desc in the char's inventory? */
  for (obj = ch->carrying; obj && !found; obj = obj->next_content) {
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  }

  /* Does the argument match an extra desc of an object in the room? */
  for (obj = world[ch->in_room].contents; obj && !found; obj = obj->next_content)
    if (CAN_SEE_OBJ(ch, obj))
	if ((desc = find_exdesc(arg, obj->ex_description)) != NULL) {
	send_to_char(desc, ch);
	found = 1;
      }
  if (bits) {			/* If an object was found back in
				 * generic_find */
    if (!found)
      show_obj_to_char(found_obj, ch, 5);	/* Show no-description */
    else
      show_obj_to_char(found_obj, ch, 6);	/* Find hum, glow etc */
  } else if (!found)
    send_to_char("You do not see that here.\r\n", ch);
}


ACMD(do_look)
{
  static char arg2[MAX_INPUT_LENGTH];
  int look_type;

  if (!ch->desc)
    return;

  if (GET_POS(ch) < POS_SLEEPING)
    send_to_char("You can't see anything but stars!\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_BLIND))
    send_to_char("You can't see a damned thing, you're blind!\r\n", ch);
  else if (IS_DARK(ch->in_room) && !CAN_SEE_IN_DARK(ch)) {
    send_to_char("It is pitch black...\r\n", ch);
    list_char_to_char(world[ch->in_room].people, ch);	/* glowing red eyes */
  } else {
    half_chop(argument, arg, arg2);

    if (subcmd == SCMD_READ) {
      if (!*arg)
	send_to_char("Read what?\r\n", ch);
      else
	look_at_target(ch, arg);
      return;
    }
    if (!*arg)			/* "look" alone, without an argument at all */
      look_at_room(ch, 1);
    else if(is_abbrev(arg, "sky"))
      {
  	if (!OUTSIDE(ch))
  		{
    			send_to_char("You can't see the sky indoors.\n\r", ch);
   			 return;
  		}
  		else
  		{
    			look_sky(ch);
    			return;
  		}
	}

    else if (is_abbrev(arg, "in"))
      look_in_obj(ch, arg2);
    /* did the char type 'look <direction>?' */
    else if ((look_type = search_block(arg, dirs, FALSE)) >= 0)
      look_in_direction(ch, look_type);
    else if (is_abbrev(arg, "at"))
      look_at_target(ch, arg2);
    else
      look_at_target(ch, arg);
  }
}



ACMD(do_examine)
{
  int bits;
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Examine what?\r\n", ch);
    return;
  }
  look_at_target(ch, arg);

  bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_OBJ_TYPE(tmp_object) == ITEM_DRINKCON) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_FOUNTAIN) ||
	(GET_OBJ_TYPE(tmp_object) == ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\r\n", ch);
      look_in_obj(ch, arg);
    }
  }
}



ACMD(do_gold)
{
  if (GET_GOLD(ch) == 0)
    send_to_char("You're broke!\r\n", ch);
  else if (GET_GOLD(ch) == 1)
    send_to_char("You have one miserable little gold coin.\r\n", ch);
  else {
    sprintf(buf, "You have %d gold coins in hand.\r\n", GET_GOLD(ch));
    send_to_char(buf, ch);
  }
}

/* Totaly re-wrote the do_score procdure so it just looks a 
   damsite better than a regula stock one - ShadowMaster, AKA NS */
ACMD(do_score)
{
  int clan_num;
  struct obj_data;

  const char *col_imm_levs[] = {
  	"/crApprentice",		/* LVL_IMMORT */
  	"/crNOBITS    ",
  	"/crNOBITS    ",
  	"/crNOBITS    ",
  	"/crNOBITS    ",
  	"/crNOBITS    ",
  	"/crSenior    ",
  	"/crNOBITS    ",
  	"/crElder     ",
  	"/crAncient   "           /* LVL_IMPL */
  };

  if (IS_NPC(ch))
    return;
 
   sprintf(buf, "\r\n/cy.---------------------------------------------------------------------------./c0\n\r");
   sprintf(buf + strlen(buf),"/cy|   /ccName:/cr %-15s/c0 ",GET_NAME(ch));

    sprintf(buf, "%s/ccSex: /cr%-8s/c0  ", buf, genders[(int)GET_SEX(ch)]);
    sprintf(buf, "%s/ccRace: /cr%-9s/c0", buf, pc_race_types[(int)GET_RACE(ch)]);
    sprintf(buf, "%s/ccClass: /cr%-13s/cy|/c0\r\n", buf, pc_class_types[(int)GET_CLASS(ch)]);

    strcpy(buf2, GET_TITLE(ch));
    colorless_str(buf2);
    sprintf(buf + strlen(buf),"/cy|  /ccTitle:/cr %-65s /cy|/c0\n\r", buf2);

    switch (GET_POS(ch)) {
      case POS_DEAD: 
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are DEAD!                                                     /cy|/c0\r\n");
      break;
    case POS_MORTALLYW:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are mortally wounded!  You should seek help!                  /cy|/c0\r\n");
      break;
    case POS_INCAP:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are incapacitated, slowly fading away...                      /cy|/c0\r\n");
      break;
    case POS_STUNNED:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are stunned!  You can't move!                                 /cy|/c0\r\n");
      break;
    case POS_SLEEPING:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are sleeping.                                                 /cy|/c0\r\n");
      break;
    case POS_RESTING:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are resting.                                                  /cy|/c0\r\n");
      break;
    case POS_SITTING:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are sitting.                                                  /cy|/c0\r\n");
      break;
    case POS_FIGHTING:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are fighting, stop looking at your stats and get on with it!  /cy|/c0\r\n");
      break;
    case POS_STANDING:
      if (!AFF_FLAGGED(ch, AFF_FLY) && !AFF_FLAGGED(ch, AFF_UNFURLED) &&
          !AFF_FLAGGED(ch, AFF_HOVER))
        sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are standing.                                                 /cy|/c0\r\n");
      break;
    default:
      sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are floating.                                                 /cy|/c0\r\n");
      break;
    }
   if (AFF_FLAGGED(ch, AFF_FLY) || AFF_FLAGGED(ch, AFF_UNFURLED))
     sprintf(buf + strlen(buf), "/cy|/cc Status: /crYou are floating.                                                 /cy|/c0\r\n");

   sprintf(buf + strlen(buf),"/cy|-----------.---------------------------------------------------------------|/c0\n\r");
   send_to_char(buf, ch);
   
   #define BAR_LEN 40 /* length of entire bar */

   sprintf(buf, "/cy|/cc STR: [/cr%2d/cc/cc]/cy |/cc [/cr", GET_STR(ch));
   make_bar(buf2, GET_HIT(ch), GET_MAX_HIT(ch), BAR_LEN);
   strcat(buf, buf2);
   if (GET_HIT(ch) > (GET_MAX_HIT(ch)*66/100))
      sprintf(buf + strlen(buf), "/cc] Health [/cg%4d/cc///cg%-4d/cc] /cy|/c0\n\r", GET_HIT(ch), GET_MAX_HIT(ch));
   else if (GET_HIT(ch) > (GET_MAX_HIT(ch)*33/100))
      sprintf(buf + strlen(buf), "/cc] Health [/cy%4d/cc///cy%-4d/cc] /cy|/c0\n\r", GET_HIT(ch), GET_MAX_HIT(ch));
   else
      sprintf(buf + strlen(buf), "/cc] Health [/cr%4d/cc///cr%-4d/cc] /cy|/c0\n\r", GET_HIT(ch), GET_MAX_HIT(ch));

   sprintf(buf + strlen(buf),"/cy|/cc INT: [/cr%2d/cc] /cy|/cc [/cr", GET_INT(ch));
   make_bar(buf2, GET_MANA(ch), GET_MAX_MANA(ch), BAR_LEN);
   strcat(buf, buf2);
   if (GET_MANA(ch) > (GET_MAX_MANA(ch)*66/100))
      sprintf(buf + strlen(buf), "/cc]  Magic [/cg%4d/cc///cg%-4d/cc] /cy|/c0\n\r", GET_MANA(ch), GET_MAX_MANA(ch));
   else if (GET_MANA(ch) > (GET_MAX_MANA(ch)*33/100))
      sprintf(buf + strlen(buf), "/cc]  Magic [/cy%4d/cc///cy%-4d/cc] /cy|/c0\n\r", GET_MANA(ch), GET_MAX_MANA(ch));
   else
      sprintf(buf + strlen(buf), "/cc]  Magic [/cr%4d/cc///cr%-4d/cc] /cy|/c0\n\r", GET_MANA(ch), GET_MAX_MANA(ch));

   sprintf(buf + strlen(buf),"/cy|/cc WIS: [/cr%2d/cc] /cy|/cc [/cr", GET_WIS(ch));
   make_bar(buf2, GET_QI(ch), GET_MAX_QI(ch), BAR_LEN);
   strcat(buf, buf2);
   if (GET_QI(ch) > (GET_MAX_QI(ch)*66/100))
      sprintf(buf + strlen(buf), "/cc]     QI [/cg%4d/cc///cg%-4d/cc] /cy|/c0\n\r", GET_QI(ch), GET_MAX_QI(ch));
   else if (GET_QI(ch) > (GET_MAX_QI(ch)*33/100))
      sprintf(buf + strlen(buf), "/cc]     QI [/cy%4d/cc///cy%-4d/cc] /cy|/c0\n\r", GET_QI(ch), GET_MAX_QI(ch));
   else
      sprintf(buf + strlen(buf), "/cc]     QI [/cr%4d/cc///cr%-4d/cc] /cy|/c0\n\r", GET_QI(ch), GET_MAX_QI(ch));

   sprintf(buf + strlen(buf),"/cy|/cc DEX: [/cr%2d/cc] /cy|/cc [/cr", GET_DEX(ch));
   make_bar(buf2, GET_VIM(ch), GET_MAX_VIM(ch), BAR_LEN);
   strcat(buf, buf2);

   if (GET_VIM(ch) > (GET_MAX_VIM(ch)*66/100))
      sprintf(buf + strlen(buf), "/cc]    Vim [/cg%4d/cc///cg%-4d/cc] /cy|/c0\n\r", GET_VIM(ch), GET_MAX_VIM(ch));
   else if (GET_VIM(ch) > (GET_MAX_VIM(ch)*33/100))
      sprintf(buf + strlen(buf), "/cc]    Vim [/cy%4d/cc///cy%-4d/cc] /cy|/c0\n\r", GET_VIM(ch), GET_MAX_VIM(ch));
   else
      sprintf(buf + strlen(buf), "/cc]    Vim [/cr%4d/cc///cr%-4d/cc] /cy|/c0\n\r", GET_VIM(ch), GET_MAX_VIM(ch));

   sprintf(buf + strlen(buf),"/cy|/cc CHA: [/cr%2d/cc] /cy|/cc [/cr", GET_CHA(ch));
   make_bar(buf2, GET_MOVE(ch), GET_MAX_MOVE(ch), BAR_LEN);
   strcat(buf, buf2);
   if (GET_MOVE(ch) > (GET_MAX_MOVE(ch)*66/100))
      sprintf(buf + strlen(buf), "/cc] Travel [/cg%4d/cc///cg%-4d/cc] /cy|/c0\n\r", GET_MOVE(ch), GET_MAX_MOVE(ch));
   else if (GET_MOVE(ch) > (GET_MAX_MOVE(ch)*33/100))
      sprintf(buf + strlen(buf), "/cc] Travel [/cy%4d/cc///cy%-4d/cc] /cy|/c0\n\r", GET_MOVE(ch), GET_MAX_MOVE(ch));
   else
      sprintf(buf + strlen(buf), "/cc] Travel [/cr%4d/cc///cr%-4d/cc] /cy|/c0\n\r", GET_MOVE(ch), GET_MAX_MOVE(ch));

   sprintf(buf + strlen(buf),"/cy|/cc CON: [/cr%2d/cc] /cy|----------------------------------------.----------------------|/c0\n\r", GET_CON(ch));   
   send_to_char(buf, ch);   
   
   if (GET_CLAN(ch)) {
     clan_num = find_clan_by_id(GET_CLAN(ch));
     strcpy(buf2, clan[clan_num].name);
     colorless_str(buf2);
     sprintf(buf, "/cy|/cc LCK: [/cr%2d/cc] /cy|/cc Clan: /cr%-32s /cy|/cc Deaths:/cr %-12d /cy|/c0\r\n", GET_LUCK(ch), buf2, (int)GET_NUM_DEATHS(ch));
     strcpy(buf2, clan[clan_num].rank_name[GET_CLAN_RANK(ch) -1]);
     colorless_str(buf2);
     sprintf(buf + strlen(buf), "/cy|/cc AGE: /cr%4d/cc /cy|/cc Rank: /cr%-32s /cy|/cc Killed:/cr %-12d /cy|/c0\r\n", GET_AGE(ch), buf2, (int)GET_NUM_KILLS(ch));
   } else {
     sprintf(buf, "/cy|/cc LCK: [/cr%2d/cc] /cy|/cc Clan: /crNone                             /cy|/cc Deaths:/cr %-12d /cy|/c0\r\n", GET_LUCK(ch), (int)GET_NUM_DEATHS(ch));
     sprintf(buf + strlen(buf), "/cy|/cc AGE: /cr%4d/cc /cy|/cc Rank: /crNone                             /cy|/cc Killed:/cr %-12d /cy|/c0\r\n", GET_AGE(ch), (int)GET_NUM_KILLS(ch));
   }

   send_to_char(buf, ch);

   sprintf(buf, "/cy|-----------^-.----------------------.---------------^---.------------------|/c0\n\r");
   sprintf(buf, "%s/cy| /ccArmor:/cr %-4d/cy |/cc   ", buf, GET_AC(ch));
   
   if (GET_LEVEL(ch) >= LVL_IMMORT)
	   sprintf(buf, "%sLevel: %-12s", buf, col_imm_levs[GET_LEVEL(ch)-LVL_IMMORT]);
   else
   	   sprintf(buf, "%sLevel: /cr%-10d", buf, GET_LEVEL(ch));
   
   sprintf(buf, "%s/cy  |  /ccItems: /cr%4d/cc///cr%-4d /cy|", buf, IS_CARRYING_N(ch), CAN_CARRY_N(ch));

   if (GET_COND(ch, FULL) == -1)
      sprintf(buf + strlen(buf), " /ccHunger:/cr Never    /cy|/c0\n\r");
   else if (GET_COND(ch, FULL) == 0)
      sprintf(buf + strlen(buf), " /ccHunger:/cr Starved  /cy|/c0\n\r");
   else if (GET_COND(ch, FULL) <= 10)
      sprintf(buf + strlen(buf), " /ccHunger:/cr Very     /cy|/c0\n\r");
   else if (GET_COND(ch, FULL) <= 19)
      sprintf(buf + strlen(buf), " /ccHunger:/cr Slighty  /cy|/c0\n\r");
   else if (GET_COND(ch, FULL) <= 22)
      sprintf(buf + strlen(buf), " /ccHunger:/cr Peckish  /cy|/c0\n\r");
   else 
      sprintf(buf + strlen(buf), " /ccHunger:/cr Stuffed  /cy|/c0\n\r");

   send_to_char(buf, ch);
   
   sprintf(buf, "/cy| /ccAlign:/cr %-5d/cy| /ccTot Exp:/cr %-12d/cy|/cc Weight:/cr %4d/cc///cr%-4d/cy |", GET_ALIGNMENT(ch), GET_EXP(ch), IS_CARRYING_W(ch), CAN_CARRY_W(ch));
   
   if (GET_COND(ch, THIRST) == -1)
      sprintf(buf + strlen(buf), " /ccThirst:/cr Never    /cy|/c0\n\r");
   else if (GET_COND(ch, THIRST) <=1)
      sprintf(buf + strlen(buf), " /ccThirst:/cr Parched  /cy|/c0\n\r");
   else if (GET_COND(ch, THIRST) <=10)
      sprintf(buf + strlen(buf), " /ccThirst:/cr Very     /cy|/c0\n\r");
   else if (GET_COND(ch, THIRST) <=20)
      sprintf(buf + strlen(buf), " /ccThirst:/cr Little   /cy|/c0\n\r");
   else
      sprintf(buf + strlen(buf), " /ccThirst:/cr Not Yet  /cy|/c0\n\r");

  send_to_char(buf, ch);
   
  sprintf(buf, "/cy| /cc  DAM:/cr %-3d  /cy|/c0  ", GET_DAMROLL(ch));

  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) < LVL_IMMORT)
      sprintf(buf + strlen(buf), "/ccNeeded:/cr %-12d/cy|", level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1) - GET_EXP(ch));
    else if (GET_LEVEL(ch) == LVL_IMPL)
      sprintf(buf + strlen(buf), "/ccNeeded:/cr Nothing Now/cy |");
    else
      sprintf(buf + strlen(buf), "/ccNeeded:/cr Promotion./cy  |");
  }

  sprintf(buf + strlen(buf), "/cc QPoint:/cr %-10d/cy|", GET_QPOINTS(ch));

    if (GET_COND(ch, DRUNK) == -1)
      sprintf(buf + strlen(buf), "  /ccDrunk:/cr Never    /cy|/c0\n\r");
    else if (GET_COND(ch, DRUNK) >= 20)
      sprintf(buf + strlen(buf), "  /ccDrunk:/cr PiSSeD   /cy|/c0\n\r");
    else if (GET_COND(ch, DRUNK) <= 19 && GET_COND(ch, DRUNK) >= 15)
      sprintf(buf + strlen(buf), "  /ccDrunk:/cr Merry    /cy|/c0\n\r");
    else if (GET_COND(ch, DRUNK) <= 14 && GET_COND(ch, DRUNK) >= 6)
      sprintf(buf + strlen(buf), "  /ccDrunk:/cr Tipsy    /cy|/c0\n\r");
    else if (GET_COND(ch, DRUNK) <= 5)
      sprintf(buf + strlen(buf), "  /ccDrunk:/cr Sober    /cy|/c0\n\r");

   send_to_char(buf, ch);
   
   sprintf(buf, "/cy|/cc   HIT:/cr %-3d  /cy|/cc    Gold:/cY %-12d/cy|-----------.-------^------------------|/c0\n\r", GET_HITROLL(ch), GET_GOLD(ch));
   send_to_char(buf, ch);

   sprintf(buf, "/cy|/cc SPELL:/cr %-3d  /cy|/cc    Bank:/cY %-12d/cy| /cc", GET_SPELLPOWER(ch), GET_BANK_GOLD(ch));

   if (PLR_FLAGGED(ch, PLR_QUESTOR))
      sprintf(buf, "%sQTime: /cr%2d /cy|/cc Rank: /cr%-19s/cy|/c0\n\r", buf, GET_COUNTDOWN(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);
   else   
      sprintf(buf, "%sQTime: /cr%2d /cy|/cc Rank: /cr%-19s/cy|/c0\n\r", buf, GET_NEXTQUEST(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);

   send_to_char(buf, ch);

   sprintf(buf, "/cy|-------------^----------------------^-----------^--------------------------|/c0\n\r");

  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
   sprintf(buf, "%s/cy|/cr You are summonable by other players.                                      /cy|/c0\r\n", buf);
  else
   sprintf(buf, "%s/cy|/cr You have summon protection activated.                                     /cy|/c0\r\n", buf);

  if (GET_PKSAFETIMER(ch) > 0)
   sprintf(buf, "%s/cy|/cr You can resume pkilling in %d minutes.                                    /cy|/c0\r\n", buf, GET_PKSAFETIMER(ch));

  if (affected_by_spell(ch, SPELL_BLUR, ABT_SPELL))
   sprintf(buf, "%s/cy|/cr Your exact location is hard to pinpoint.                                  /cy|/c0\r\n", buf);
  if (AFF_FLAGGED(ch, AFF_INCOGNITO))
   sprintf(buf, "%s/cy|/cr You are travelling incognito.                                             /cy|/c0\r\n", buf);
   if (AFF_FLAGGED(ch, AFF_DROWSE))
   sprintf(buf, "%s/cy|/cr You feel an extreme weariness.                                            /cy|/c0\r\n", buf);
  if (PLR_FLAGGED(ch, PLR_INSURED))
   sprintf(buf, "%s/cy|/cr You have up-to-date equipment insurance.                                  /cy|/c0\r\n", buf);
  if (PLR_FLAGGED(ch, PLR_PKILL_OK)) {
  if (GET_PKSAFETIMER(ch) > 0)
   sprintf(buf, "%s/cy|/cr You can resume pkilling in %d minutes.                                    /cy|/c0\r\n", buf, GET_PKSAFETIMER(ch));
  else
   sprintf(buf, "%s/cy|/cr You may kill other players who are player killers.                        /cy|/c0\r\n", buf);
  }
  send_to_char(buf, ch);

   sprintf(buf, "/cy`---------------------------------------------------------------------------'/c0\n\r");
   send_to_char(buf, ch);
}

ACMD(do_inventory)
{
  send_to_char("You are carrying:\r\n", ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}

const char *show_equip(struct char_data *ch, int wearpos)
{
	struct obj_data *obj = NULL;
	obj = NULL;
	if (wearpos >= 0 && wearpos <= NUM_WEARS)
	{
		if (!GET_EQ(ch, wearpos))
		{
			return "/cw(empty)/c0";
		}
		else if ((obj = GET_EQ(ch, wearpos)) != NULL && CAN_SEE_OBJ(ch, obj))
			return (obj->short_description);
		else
			return "Something";
	} else
		return "None.";
}

ACMD(do_equipment)
{
  char output[MAX_STRING_LENGTH];
  send_to_char("You are using:\r\n", ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FLOAT_1], show_equip(ch, WEAR_FLOAT_1));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FLOAT_2], show_equip(ch, WEAR_FLOAT_2));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_LIGHT], show_equip(ch, WEAR_LIGHT));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_HEAD], show_equip(ch, WEAR_HEAD));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FACE], show_equip(ch, WEAR_FACE));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_EAR_R], show_equip(ch, WEAR_EAR_R));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_EYES], show_equip(ch, WEAR_EYES));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_EAR_L], show_equip(ch, WEAR_EAR_L));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_NECK_1], show_equip(ch, WEAR_NECK_1));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_NECK_2], show_equip(ch, WEAR_NECK_2));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_ABOUT], show_equip(ch, WEAR_ABOUT));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_BACK], show_equip(ch, WEAR_BACK));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_BODY], show_equip(ch, WEAR_BODY));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_ARMS], show_equip(ch, WEAR_ARMS));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_SHIELD], show_equip(ch, WEAR_SHIELD));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_WRIST_R], show_equip(ch, WEAR_WRIST_R));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_WRIST_L], show_equip(ch, WEAR_WRIST_L));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_HANDS], show_equip(ch, WEAR_HANDS));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_THUMB_R], show_equip(ch, WEAR_THUMB_R));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_THUMB_L], show_equip(ch, WEAR_THUMB_L));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FINGER_R], show_equip(ch, WEAR_FINGER_R));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FINGER_L], show_equip(ch, WEAR_FINGER_L));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_WIELD], show_equip(ch, WEAR_WIELD));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_HOLD], show_equip(ch, WEAR_HOLD));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_WAIST], show_equip(ch, WEAR_WAIST));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_LEGS], show_equip(ch, WEAR_LEGS));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_ANKLE], show_equip(ch, WEAR_ANKLE));
  send_to_char(output, ch);
  sprintf(output, "/cw%s/c0%s/c0 \r\n", where[WEAR_FEET], show_equip(ch, WEAR_FEET));
  send_to_char(output, ch);
  return;
}
/*
ACMD(do_equipment)
{
  int i, found = 0;

  send_to_char("You are using:\r\n", ch);
  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      if (CAN_SEE_OBJ(ch, GET_EQ(ch, i))) {
        sprintf(buf, "/cw%s/c0", where[i]);
      	send_to_char(buf, ch);
      	show_obj_to_char(GET_EQ(ch, i), ch, 1);
      	found = TRUE;
      } else {
        sprintf(buf, "/cw%s/c0Something\r\n", where[i]);
        send_to_char(buf, ch);
        found = TRUE;
      }
    } else if (!GET_EQ(ch, i)) {
      sprintf(buf, "/cw%-20s (empty)\r\n/c0", where[i]);
      send_to_char(buf, ch);
    }
  }
}
*/
ACMD(do_time)
{
  char *suf;
  int weekday, day;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am"));

  /* 35 days in a month */
  weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

  strcat(buf, weekdays[weekday]);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  day = time_info.day + 1;	/* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	  day, suf, month_name[(int) time_info.month], time_info.year);

  send_to_char(buf, ch);
}


ACMD(do_weather)
{
  static char *sky_look[] = {
    "cloudless",
    "cloudy",
    "rainy",
  "lit by flashes of lightning",
  "obscured by the falling snow"};

  if (OUTSIDE(ch)) {
    sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	    (weather_info.change >= 0 ? "you feel a warm wind from south" :
	     "you feel like bad weather is due"));
    send_to_char(buf, ch);
  } else
    send_to_char("You have no feeling about the weather at all.\r\n", ch);
}


ACMD(do_help)
{
  extern int top_of_helpt;
  extern struct help_index_element *help_table;
  extern char *help;

  int chk, bot, top, mid, minlen;

  if (!ch->desc)
    return;

  skip_spaces(&argument);

  if (!*argument) {
    page_string(ch->desc, help, 0);
    return;
  }
  if (!help_table) {
    send_to_char("No help available.\r\n", ch);
    return;
  }

  bot = 0;
  top = top_of_helpt;
  minlen = strlen(argument);

  for (;;) {
    mid = (bot + top) / 2;

    if (bot > top) {
      send_to_char("There is no help on that word.\r\n", ch);
      sprintf(buf, "MISSING HELP FILE:: %s", argument);
      mudlog(buf, BRF, LVL_ADMIN, TRUE);
      return;
    } else if (!(chk = strn_cmp(argument, help_table[mid].keyword, minlen)) &&
               GET_LEVEL(ch) >= help_table[mid].level) {
      /* trace backwards to find first matching entry. Thanks Jeff Fink! */
      while ((mid > 0) &&
	 (!(chk = strn_cmp(argument, help_table[mid - 1].keyword, minlen))))
	mid--;
      page_string(ch->desc, help_table[mid].entry, 0);
      return;
    } else {
      if (chk > 0)
        bot = mid + 1;
      else
        top = mid - 1;
    }
  }
}

struct who_list {
  char *name;
  int level;
  int tier;
  struct who_list *next;
};

struct who_list *add_to_who(struct who_list *head, char *str,
        struct char_data *ch)
{
  struct who_list *tmp, *prev = NULL, *to_add = NULL;
  int a = 0, b = 0;

  if (str && ch) {
    CREATE(to_add, struct who_list, 1);
    to_add->name = str_dup(str);
	to_add->level = GET_LEVEL(ch);
    to_add->tier = GET_TIER(ch);
    to_add->next = NULL;
  } else {
    log("SYSERR: NULL str or ch in add_to_who");
    return NULL;
  }

  if (!head)
    return to_add;

  if (IS_IMMORT(ch))
	  a = 150;
  else
	  a = to_add->level;
  for (tmp = head; tmp; tmp = tmp->next)
  {
	  if (tmp->tier >= 1)
		  b = 150;
	  else
		  b = tmp->level;
	  if (a > b)
	  { 
		  if (prev)
			  prev->next = to_add;
		  if (head == tmp)
			  head = to_add;
		  to_add->next = tmp;
		  return head;
	  }
	  prev = tmp;
  }
  prev->next = to_add;
  return head;
}

void output_who(struct who_list *head, struct char_data *ch)
{
  struct who_list *tmp, *next;

  if (!head) {
    log("SYSERR: output_who: hey, no head?");
    return;
  }

  for (tmp = head; tmp; tmp = next) {
    next = tmp->next;
     send_to_char(tmp->name, ch);
     if (next) {
      if (tmp->level >= 151 && next->level <= 150){
        send_to_char("\r\n", ch);}
     }

    if (!tmp || !tmp->name)
      log("SYSERR: output_who: trying to free invalid tmp->name");
    else {
      free(tmp->name);
    }
    if (!tmp)
      log("SYSERR: output_who: trying to free invalid tmp struct");
    else {
      free(tmp);
    }
  }
}




#define WHO_FORMAT \
"format: who [minlev[-maxlev]] [-n name] [-c classlist] [-s] [-o] [-q] [-r] [-z]\r\n"


ACMD(do_who)
{
  struct descriptor_data *d, *dt, *dt1;
  struct char_data *tch;
  struct who_list *who_head = NULL;
  int multis_left = 0;
  char temp[1024];
  int idletime=0;   /* Storm 8/2/98 */
  char name_search[MAX_INPUT_LENGTH];
  char mode;
  size_t i;
  int low = 0, high = LVL_IMPL, localwho = 0, questwho = 0;
  int showclass = 0, short_list = 0, outlaws = 0, num_can_see = 0;
  int who_room = 0;
  int clan_num=0;
  int lp;

  skip_spaces(&argument);
  strcpy(buf, argument);
  name_search[0] = '\0';

  while (*buf) {
    half_chop(buf, arg, buf1);
    if (isdigit(*arg)) {
      sscanf(arg, "%d-%d", &low, &high);
      strcpy(buf, buf1);
    } else if (*arg == '-') {
      mode = *(arg + 1);	/* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	      outlaws = 1;
	      strcpy(buf, buf1);
	      break;
      case 'z':
        localwho = 1;
        strcpy(buf, buf1);
        break;
      case 's':
        short_list = 1;
        strcpy(buf, buf1);
        break;
      case 'q':
	      questwho = 1;
	      strcpy(buf, buf1);
	      break;
      case 'l':
	      half_chop(buf1, arg, buf);
	      sscanf(arg, "%d-%d", &low, &high);
	      break;
      case 'n':
	      half_chop(buf1, name_search, buf);
	      break;
      case 'r':
	      who_room = 1;
	      strcpy(buf, buf1);
	      break;
      case 'c':
	      half_chop(buf1, arg, buf);
	      for (i = 0; i < strlen(arg); i++)
      	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
        send_to_char(WHO_FORMAT, ch);
        return;
        break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(WHO_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */

  for (lp = 1; lp<=2; lp++) {
    switch(lp){
    case 1:
//    send_to_char("\r\n/cB Players\r\n--------------------------/c0\r\n", ch);
      break;
    case 2:
      if (num_can_see){
      send_to_char("\r\n", ch);  /* Skip a line between imps and players. */
      }
      break;
    }

    for (d = descriptor_list; d; d = d->next) {
      if (d->connected!=CON_PLAYING&&d->connected!=CON_OEDIT
          &&d->connected!=CON_REDIT&&d->connected!=CON_ZEDIT
          &&d->connected!=CON_MEDIT&&d->connected!=CON_SEDIT)
        continue;

      if (d->original)
        tch = d->original;
      else if (!(tch = d->character))
        continue;

      if ((GET_LEVEL(tch) >= LVL_IMMORT && lp==1) ||
          (GET_LEVEL(tch) < LVL_IMMORT && lp==2)){
        if (*name_search && str_cmp(GET_NAME(tch), name_search) &&
            !strstr(GET_TITLE(tch), name_search))
          continue;
        if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
          continue;
        if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
            !PLR_FLAGGED(tch, PLR_THIEF) && !PLR_FLAGGED(tch, PLR_EXILE))
          continue;
        if (questwho && !PRF_FLAGGED(tch, PRF_QUEST))
          continue;
        if (localwho && world[ch->in_room].zone != world[tch->in_room].zone)
          continue;
        if (who_room && (tch->in_room != ch->in_room))
          continue;
        if (showclass && !(showclass & (1 << GET_CLASS(tch))))
          continue;
        if (short_list) {
          if (GET_LEVEL(tch) >= LVL_IMMORT) {
            sprintbit(GET_GOD_TYPE(tch), god_types, buf1);
            sprintf(buf, "/cwA brilliant shimmer of light/c0\r\n"); 
/* KAAN */
          } else if (GET_CLASS(tch) >= CLASS_VAMPIRE) {
            sprintf(buf, "[ %d %3d %s %s] %-12.12s/c0",
                    GET_TIER(tch), GET_LEVEL(tch), RACE_ABBR(tch),
                    CLASS_ABBR(tch), GET_NAME(tch));
          } else if (PRF_FLAGGED(tch, PRF_NORANK) && 
                     GET_CLASS(tch) < CLASS_VAMPIRE) {
            sprintf(buf, "[ - %3d %s %s] %-12.12s/c0",
                    GET_LEVEL(tch), RACE_ABBR(tch), CLASS_ABBR(tch),
                    GET_NAME(tch));
/* END KAAN */
          } else
            sprintf(buf, "[ - %3d %s %s] %s %-12.12s/c0",
	            GET_LEVEL(tch), RACE_ABBR(tch), CLASS_ABBR(tch), rank[(int)GET_RANK(tch)][(int)GET_SEX(tch)], GET_NAME(tch));
          num_can_see++;
          send_to_char(buf, ch);
        } else {
          if (GET_LEVEL(tch) >= LVL_IMMORT){
   if(PRF_FLAGGED(tch, PRF_IHIDE)) {
     if(IHIDE(ch)) {
      sprintf(buf, "%s", IHIDE(tch));
     }
            } else {
            sprintbit(GET_GOD_TYPE(tch), god_types, buf1);
            sprintf(buf, "%s%s/cw- %s %s/c0", god_lvls[GET_LEVEL(tch) - LVL_IMMORT], buf1, GET_NAME(tch), 
                         GET_TITLE(tch));
	            num_can_see++;
            }
/* KAAN */
          } else if (GET_CLASS(tch) >= CLASS_VAMPIRE) {
            sprintf(buf, "/cy[ /cR%d/cr %3d/cb %s/cg %s/cy]/cw %s %s/c0",
                    GET_TIER(tch), GET_LEVEL(tch), RACE_ABBR(tch),
                    CLASS_ABBR(tch), GET_NAME(tch), GET_TITLE(tch));
			num_can_see++;
          } else if (PRF_FLAGGED(tch, PRF_NORANK)) {
            sprintf(buf, "/cy[/cg - /cr%3d/cb %s/cg %s/cy]/cw %s %s/c0",
            GET_LEVEL(tch), RACE_ABBR(tch), CLASS_ABBR(tch),
            GET_NAME(tch), GET_TITLE(tch)); 
            num_can_see++; //Fix for num chars -Samhadi
/* END KAAN */
          }else{
            sprintf(buf, "/cy[/cg - /cr%3d/cb %s/cg %s/cy]/cw %s %s %s/c0",
            GET_LEVEL(tch), RACE_ABBR(tch), CLASS_ABBR(tch), rank[(int)GET_RANK(tch)][(int)GET_SEX(tch)], GET_NAME(tch),
                    GET_TITLE(tch));
            num_can_see++;
          }
          if (find_clan_by_id(GET_CLAN(tch))) {
            clan_num = find_clan_by_id(GET_CLAN(tch));
            if(GET_CLAN_RANK(tch) > 0)
              sprintf(buf, "%s (%s /c0of %s/c0)", buf,
                clan[clan_num].rank_name[GET_CLAN_RANK(tch) -1],
                clan[clan_num].name);
            else
              sprintf(buf, "%s (Applying to %s)", buf, clan[clan_num].name);
          }

          if (GET_INVIS_LEV(tch))
      	    sprintf(buf, "%s (i%d)", buf, GET_INVIS_LEV(tch));
          else if (AFF_FLAGGED(tch, AFF_INVISIBLE))
	          strcat(buf, " (invis)");

          if (PLR_FLAGGED(tch, PLR_MAILING))
      	    strcat(buf, " (mailing)");

          if (tch->desc &&
                  (PLR_FLAGGED(tch, PLR_WRITING)||
                   tch->desc->connected==CON_OEDIT||
                   tch->desc->connected==CON_REDIT||
                   tch->desc->connected==CON_ZEDIT||
                   tch->desc->connected==CON_MEDIT||
                   tch->desc->connected==CON_SEDIT))
      	    strcat(buf, " (writing)");
          if (GET_PKSAFETIMER(tch) > 0)
                  strcat(buf, " (Pktimeout)");
          if (PRF_FLAGGED(tch, PRF_DEAF))
	          strcat(buf, " (deaf)");
          if (PRF_FLAGGED(tch, PRF_NOTELL))
	          strcat(buf, " /cr(notell)/c0");
          if (PRF_FLAGGED(tch, PRF_QUEST))
      	    strcat(buf, " /cG(quest)/c0");
          if (PLR_FLAGGED(tch, PLR_THIEF))
      	    strcat(buf, " (THIEF)");
          if (PLR_FLAGGED(tch, PLR_KILLER))
      	    strcat(buf, " (KILLER)");
          if (PLR_FLAGGED(tch, PLR_EXILE))
            strcat(buf, " /cY(/cyExile/cY)/c0 ");
          if (PLR_FLAGGED(tch, PLR_BOUNTY))
            strcat(buf, " /cR(/crBounty/cR)/c0 ");
          if (PRF_FLAGGED(tch, PRF_AFK))
             strcat(buf, " /cR(AFK)/c0");
          if (ROOM_FLAGGED(IN_ROOM(tch), ROOM_ARENA))
             strcat(buf, " /cR(Arena)/c0");
          
          /* Rapax 11/99 */
          if (PRF_FLAGGED(tch, PRF_MULTIPLAY) && GET_LEVEL(ch) > LVL_CODER) {
            for(dt = descriptor_list; dt; dt = dt->next) {
              if(!dt->connected) {
                if(dt->host && *dt->host)
                  if (!strcmp(dt->host, tch->desc->host)) {
                    multis_left = 0;
                    for(dt1 = dt->next; dt1; dt1 = dt1->next){
                      if(!dt1->connected) {
                        if(dt1->host && *dt1->host)
                          if(!strcmp(dt1->host, tch->desc->host)){
                            multis_left++;
                            break;
                          }
                       }
                     }
                    if(!multis_left) {
                      if(dt->original)
                        sprintf(temp, " /cG(%s)/c0", dt->original->player.name);
                      else
                        sprintf(temp, " /cG(%s)/c0", dt->character->player.name);
                      strcat(buf, temp);
                    }
                    else
                      continue;
                }
              }
            }
          }
       /* End Multi tag  -Rapax*/ 
       /*  Storm 8/2/98    */
          idletime = tch->char_specials.timer * SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN;
          if (idletime > 10)
             strcat(buf, " /cR(idle)/c0");

          strcat(buf, "\r\n");
//          send_to_char(buf, ch);
          who_head = add_to_who(who_head, buf, tch); 
        }				/* endif shortlist */
      }
    }				/* end of for */
  } /* end of for 1 to 2... */
  if (short_list && (num_can_see % 4))
    send_to_char("\r\n", ch);
  if (who_head)
    output_who(who_head, ch);
  if (num_can_see == 0)
    sprintf(buf, "\r\nNo-one at all!\r\n");
  else if (num_can_see == 1)
    sprintf(buf, "\r\nOne lonely character displayed.\r\n");
  else
    sprintf(buf, "\r\n%d characters displayed.\r\n", num_can_see);
  sprintf(buf, "%sMost on today: %d\r\n", buf, most_on);
  send_to_char(buf, ch);

  if (active_quest != NO_QUEST) {
    sprintf(buf, "/cGThe %s quest is %s./c0\r\n",
            quests[active_quest].quest_name, qstates[active_quest_state]);
  send_to_char(buf, ch);
  }
  if (double_exp == 1) {
    sprintf(buf, "/cR%d minute remaining of double experience./c0\r\n", 
            double_exp);
    send_to_char(buf, ch);
  }
  else if (double_exp > 1) {
    sprintf(buf, "/cR%d minutes remaining of double experience./c0\r\n", 
            double_exp);
    send_to_char(buf, ch);
  }

  switch (in_arena) {
  case ARENA_START:
    sprintf(buf, "/cRThe arena is accepting players level %d to %d./c0\r\n",
            lo_lim, hi_lim);
    send_to_char(buf, ch);
    break;
  case ARENA_RUNNING:
    send_to_char("/cRThe arena is running./c0\r\n", ch);
    break;
  }

}


#define USERS_FORMAT \
"format: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"

ACMD(do_users)
{
  extern char *connected_types[];
  char line[200], line2[220], idletime[10], classname[20];
  char state[30], *timeptr, *format, mode;
  char name_search[MAX_INPUT_LENGTH], host_search[MAX_INPUT_LENGTH];
  struct char_data *tch;
  struct descriptor_data *d;
  size_t i;
  int low = 0, high = LVL_IMPL, num_can_see = 0;
  int showclass = 0, outlaws = 0, playing = 0, deadweight = 0;

  host_search[0] = name_search[0] = '\0';

  strcpy(buf, argument);
  while (*buf) {
    half_chop(buf, arg, buf1);
    if (*arg == '-') {
      mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
      switch (mode) {
      case 'o':
      case 'k':
	outlaws = 1;
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'p':
	playing = 1;
	strcpy(buf, buf1);
	break;
      case 'd':
	deadweight = 1;
	strcpy(buf, buf1);
	break;
      case 'l':
	playing = 1;
	half_chop(buf1, arg, buf);
	sscanf(arg, "%d-%d", &low, &high);
	break;
      case 'n':
	playing = 1;
	half_chop(buf1, name_search, buf);
	break;
      case 'h':
	playing = 1;
	half_chop(buf1, host_search, buf);
	break;
      case 'c':
	playing = 1;
	half_chop(buf1, arg, buf);
	for (i = 0; i < strlen(arg); i++)
	  showclass |= find_class_bitvector(arg[i]);
	break;
      default:
	send_to_char(USERS_FORMAT, ch);
	return;
	break;
      }				/* end of switch */

    } else {			/* endif */
      send_to_char(USERS_FORMAT, ch);
      return;
    }
  }				/* end while (parser) */
  strcpy(line,
	 "Num Class     Name         State          Idl Login@   Site\r\n");
  strcat(line,
	 "--- --------- ------------ -------------- --- -------- ------------------------\r\n");
  send_to_char(line, ch);

  one_argument(argument, arg);

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected && playing)
      continue;
    if (!d->connected && deadweight)
      continue;
    if (!d->connected) {
      if (d->original)
	tch = d->original;
      else if (!(tch = d->character))
	continue;

      if (*host_search && !strstr(d->host, host_search))
	continue;
      if (*name_search && str_cmp(GET_NAME(tch), name_search))
	continue;
      if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
	continue;
      if (outlaws && !PLR_FLAGGED(tch, PLR_KILLER) &&
	  !PLR_FLAGGED(tch, PLR_THIEF))
	continue;
      if (showclass && !(showclass & (1 << GET_CLASS(tch))))
	continue;
      if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
	continue;

      if (d->original)
	sprintf(classname, "[%3d %s]", GET_LEVEL(d->original),
		CLASS_ABBR(d->original));
      else
	sprintf(classname, "[%3d %s]", GET_LEVEL(d->character),
		CLASS_ABBR(d->character));
    } else
      strcpy(classname, "   -   ");

    timeptr = asctime(localtime(&d->login_time));
    timeptr += 11;
    *(timeptr + 8) = '\0';

    if (!d->connected && d->original)
      strcpy(state, "Switched");
    else
      strcpy(state, connected_types[d->connected]);

    if (d->character && !d->connected && GET_LEVEL(d->character) < LVL_ADMIN)
      sprintf(idletime, "%3d", d->character->char_specials.timer *
	      SECS_PER_MUD_HOUR / SECS_PER_REAL_MIN);
    else
      strcpy(idletime, "");

    format = "%3d %-7s %-12s %-14s %-3s %-8s ";

    if (d->character && d->character->player.name) {
      if (d->original)
	sprintf(line, format, d->desc_num, classname,
		d->original->player.name, state, idletime, timeptr);
      else
	sprintf(line, format, d->desc_num, classname,
		d->character->player.name, state, idletime, timeptr);
    } else
      sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED",
	      state, idletime, timeptr);

    if (d->host && *d->host)
      sprintf(line + strlen(line), "[%s]\r\n", d->host);
    else
      strcat(line, "[Hostname unknown]\r\n");

    if (d->connected) {
      sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
      strcpy(line, line2);
    }
    if (d->connected || (!d->connected && CAN_SEE(ch, d->character))) {
      send_to_char(line, ch);
      num_can_see++;
    }
  }

  sprintf(line, "\r\n%d visible sockets connected.\r\n", num_can_see);
  send_to_char(line, ch);
}


/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{
  extern char circlemud_version[];

  switch (subcmd) {
  case SCMD_CREDITS:
    page_string(ch->desc, credits, 0);
    break;
  case SCMD_NEWS:
    page_string(ch->desc, news, 0);
    break;
  case SCMD_INFO:
    page_string(ch->desc, info, 0);
    break;
  case SCMD_WIZLIST:
    page_string(ch->desc, wizlist, 0);
    break;
  case SCMD_IMMLIST:
    page_string(ch->desc, immlist, 0);
    break;
  case SCMD_HANDBOOK:
    page_string(ch->desc, handbook, 0);
    break;
  case SCMD_POLICIES:
    page_string(ch->desc, policies, 0);
    break;
  case SCMD_MOTD:
    page_string(ch->desc, motd, 0);
    break;
  case SCMD_IMOTD:
    page_string(ch->desc, imotd, 0);
    break;
  case SCMD_CLEAR:
    send_to_char("\033[H\033[J", ch);
    break;
  case SCMD_VERSION:
    send_to_char(circlemud_version, ch);
    send_to_char(strcat(strcpy(buf, DG_SCRIPT_VERSION), "\r\n"), ch);
    break;
  case SCMD_WHOAMI:
    send_to_char(strcat(strcpy(buf, GET_NAME(ch)), "\r\n"), ch);
    break;
  default:
    return;
    break;
  }
}


void perform_mortal_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct descriptor_data *d;

  if (!*arg) {
    sprintf(buf, "Players in %s\r\n", zone_table[world[ch->in_room].zone].name);
    send_to_char(buf, ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
        if (GET_CLASS(ch) == CLASS_TITAN) {
          if (IS_AFFECTED(i, AFF_CLOUD) && (i->in_room != NOWHERE) &&
              GET_LEVEL(i) <= 150) {
            sprintf(buf, "%-25s - %s /cy[Cloud]/c0\r\n",
                    GET_NAME(i), world[i->in_room].name);
            send_to_char(buf, ch);
          }
        } 
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE) &&
            GET_LEVEL(i) <= 150 &&
	    (world[ch->in_room].zone == world[i->in_room].zone)) {
	  sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {			/* print only FIRST char, not all. */
    for (i = character_list; i; i = i->next)
      if (world[i->in_room].zone == world[ch->in_room].zone && CAN_SEE(ch, i) &&
	  (i->in_room != NOWHERE) && isname(arg, i->player.name) &&
           GET_LEVEL(i) <= 150) {
	sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
	send_to_char(buf, ch);
	return;
      }
    send_to_char("No-one around by that name.\r\n", ch);
  }
}


void print_object_location(int num, struct obj_data * obj, struct char_data * ch,
			        int recur)
{
  if (num > 0)
    sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
  else
    sprintf(buf, "%33s", " - ");

  if (obj->in_room > NOWHERE) {
    sprintf(buf + strlen(buf), "[%5d] %s\n\r",
	    world[obj->in_room].number, world[obj->in_room].name);
    send_to_char(buf, ch);
  } else if (obj->carried_by) {
    sprintf(buf + strlen(buf), "carried by %s\n\r",
	    PERS(obj->carried_by, ch));
    send_to_char(buf, ch);
  } else if (obj->worn_by) {
    sprintf(buf + strlen(buf), "worn by %s\n\r",
	    PERS(obj->worn_by, ch));
    send_to_char(buf, ch);
  } else if (obj->in_obj) {
    sprintf(buf + strlen(buf), "inside %s%s\n\r",
	    obj->in_obj->short_description, (recur ? ", which is" : " "));
    send_to_char(buf, ch);
    if (recur)
      print_object_location(0, obj->in_obj, ch, recur);
  } else {
    sprintf(buf + strlen(buf), "in an unknown location\n\r");
    send_to_char(buf, ch);
  }
}



void perform_immort_where(struct char_data * ch, char *arg)
{
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int num = 0, found = 0;

  if (!*arg) {
    send_to_char("Players\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
	  if (d->original)
	    sprintf(buf, "%-20s - [%5d] %s (in %s)\r\n",
		    GET_NAME(i), world[d->character->in_room].number,
		 world[d->character->in_room].name, GET_NAME(d->character));
	  else
	    sprintf(buf, "%-20s - [%5d] %s\r\n", GET_NAME(i),
		    world[i->in_room].number, world[i->in_room].name);
	  send_to_char(buf, ch);
	}
      }
  } else {
    for (i = character_list; i; i = i->next)
      if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
	found = 1;
	sprintf(buf, "M%3d. %-25s - [%5d] %s\r\n", ++num, GET_NAME(i),
		world[i->in_room].number, world[i->in_room].name);
	send_to_char(buf, ch);
      }
    for (num = 0, k = object_list; k; k = k->next)
      if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
	found = 1;
	print_object_location(++num, k, ch, TRUE);
      }
    if (!found)
      send_to_char("Couldn't find any such thing.\r\n", ch);
  }
}



ACMD(do_where)
{
  one_argument(argument, arg);

  if (GET_LEVEL(ch) >= LVL_IMMORT)
    perform_immort_where(ch, arg);
  else
    perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
  int i;

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
    return;
  }
  *buf = '\0';
  
  for (i = 1; i < LVL_IMMORT; i++) {
    if (i < LVL_IMMORT-1 && GET_CLASS(ch) <= 37)
      sprintf(buf + strlen(buf), "[%2d] %8d-%-8d : ", i,
              level_exp(GET_CLASS(ch), i), level_exp(GET_CLASS(ch), i+1) - 1);
    else if (i < 31 && GET_CLASS(ch) >= CLASS_VAMPIRE)
      sprintf(buf + strlen(buf), "[%2d] %8d-%-8d : ", i,
              level_exp(GET_CLASS(ch), i), level_exp(GET_CLASS(ch), i+1) - 1); 
    else if (i >= 31 && GET_CLASS(ch) >= CLASS_VAMPIRE)
      sprintf(buf + strlen(buf), "Non-Applicable : ");
    else
      sprintf(buf + strlen(buf), "[%2d] %8d            : ", i,
              level_exp(GET_CLASS(ch), i));
    switch (GET_SEX(ch)) {
    case SEX_MALE:
    case SEX_NEUTRAL:
      strcat(buf, title_male(GET_CLASS(ch), i));
      break;
    case SEX_FEMALE:
      strcat(buf, title_female(GET_CLASS(ch), i));
      break;
    default:
      send_to_char("Oh dear.  You seem to be sexless.\r\n", ch);
      break;
    }
    strcat(buf, "\r\n");
  }
  page_string(ch->desc, buf, 1);
}



ACMD(do_consider)
{
  struct char_data *victim;
  int diff;

  one_argument(argument, buf);

  if (!(victim = get_char_room_vis(ch, buf))) {
    send_to_char("Consider killing who?\r\n", ch);
    return;
  }
  if (victim == ch) {
    send_to_char("Easy!  Very easy indeed!\r\n", ch);
    return;
  }
  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\r\n", ch);
    return;
  }
  diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

  if (diff <= -10)
    if (GET_SEX(ch)==SEX_MALE)
      send_to_char("Pick on someone your own size, dick.\r\n", ch);
    else
      send_to_char("Pick on someone your own size, bitch.\r\n", ch);
  else if (diff <= -5)
    send_to_char("No sweat.\r\n", ch);
  else if (diff <= -2)
    send_to_char("Fairly Easy.\r\n", ch);
  else if (diff <= -1)
    send_to_char("Pretty close - you should be able to conquer.\r\n", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\r\n", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\r\n", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\r\n", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\r\n", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\r\n", ch);
  else if (diff <= 10)
    send_to_char("Are you mad!?\r\n", ch);
  else if (diff <= 100)
    send_to_char("You'll most definitely get your ass kicked.\r\n", ch);
  else
    send_to_char("Dude, you're fucked.\r\n", ch);

}

bool S_MOB_FLAGGED(struct char_data *ch, int flag)
{
	int i = 0;
	if (ch == NULL || flag == -1)
		return (FALSE);
	else if (!IS_NPC(ch))
		return (FALSE);
	else {
		for(i = 0; i < PM_ARRAY_MAX;i++)
		{
			if (ch->char_specials.saved.act[i] == flag)
				return (TRUE);
			else
				continue;
		}
		return (FALSE);
	}
}

bool S_AFF_FLAGGED(struct char_data *ch, int flag)
{
	int i = 0;
	if (ch == NULL || flag == -1)
		return (FALSE);
	else {
		for(i = 0; i < AF_ARRAY_MAX;i++)
		{
			if (ch->char_specials.saved.affected_by[i] == flag)
				return (TRUE);
			else
				continue;
		}
		return (FALSE);
	}
}
/*
ACMD(do_reveal_nature)
{
  struct char_data *vict;
  float chance = 0, plev = 0, mlev = 0;
  one_argument(argument, buf);

  if (!is_class(ch, CLASS_DRUID))
  {
	  send_to_char("Huh?!\r\n", ch);
	  return;
  }
  if (!GET_SKILL(ch, SKILL_REVEAL_NATURE))
  {
	  send_to_char("Huh?!\r\n", ch);
	  return;
  }
  if (!(vict = get_char_room_vis(ch, buf)))
  {
    send_to_char("No such creature here.\r\n", ch);
    return;
  }
  else if (vict == ch)
  {
    send_to_char("You can not consider yourself!\r\n", ch);
    return;
  }
  else if (!IS_NPC(vict))
  {
    send_to_char("Not one players.\r\n", ch);
    return;
  } else {
	  plev = (GET_LEVEL(ch));
	  mlev = (GET_LEVEL(vict));
	  chance = ((plev/mlev) * 100);
	  if (chance >= 100)
		  chance = 100;
	  sprintf(buf, "%sYou reveal the following information about /cw%s/c0.\r\n", buf, GET_NAME(vict));
	  sprintf(buf, "%sLevel: (/cw%d/c0)  \r\n", buf, GET_LEVEL(vict));
	  sprintf(buf, "%sClass: (/cw%s/c0) Race: (/cw%s/c0) \r\n", buf, npc_class_types[(int)GET_CLASS(vict)], npc_race_types[(int)GET_RACE(vict)]);
	  sprintf(buf, "%sYou got a /cw%d%% chance of killing /cw%s/c0.\r\n", buf, ((int)chance), GET_NAME(vict));
	  sprintf(buf, "%s%s...\r\n", buf, GET_NAME(vict));
	  if (S_AFF_FLAGGED(ch, AFF_BALANCE))
	  sprintf(buf, "%s ...appears to have very good balance. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_NOBASH))
	  sprintf(buf, "%s ...looks very strong and sturdy.\r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_AWARE))
	  sprintf(buf, "%s ...is very aware of you. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_MEMORY))
	  sprintf(buf, "%s ...looks back at you with remembering eyes. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_INSTAGGRO))
	  sprintf(buf, "%s ...is instantly aggresive. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_AGGR_EVIL))
	  sprintf(buf, "%s ...hates the evil and corrupted. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_AGGR_GOOD))
	  sprintf(buf, "%s ...hunts the saintly ones. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_AGGR_NEUTRAL))
	  sprintf(buf, "%s ...hates the weak standing between two sides. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_HELPER))
	  sprintf(buf, "%s ...follows his friends into battle. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_HUNTER))
	  sprintf(buf, "%s ...looks for someone to hunt.\r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_VENOMOUS))
	  sprintf(buf, "%s ...is very venomous. \r\n", buf);
	  if (S_MOB_FLAGGED(ch, MOB_WIMPY))
	  sprintf(buf, "%s ...is a wimp. \r\n", buf);
	  send_to_char(buf, ch);
	  return;
  }
}
*/
ACMD(do_diagnose)
{
  struct char_data *vict;

  one_argument(argument, buf);

  if (*buf) {
    if (!(vict = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    } else
      diag_char_to_char(vict, ch);
  } else {
    if (FIGHTING(ch))
      diag_char_to_char(FIGHTING(ch), ch);
    else
      send_to_char("Diagnose who?\r\n", ch);
  }
}


static char *ctypes[] = {
"off", "sparse", "normal", "complete", "\n"};

ACMD(do_color)
{
  int tp;

  if (IS_NPC(ch))
    return;

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "Your current color level is %s.\r\n", ctypes[COLOR_LEV(ch)]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
    send_to_char("Usage: color { Off | Sparse | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
  if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_1);
  if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_COLOR_2);
  sprintf(buf, "Your %scolor%s is now %s.\r\n", CCRED(ch, C_SPR),
	  CCNRM(ch, C_OFF), ctypes[tp]);
  send_to_char(buf, ch);
}


ACMD(do_toggle)
{
  if (IS_NPC(ch))
    return;
  if (GET_WIMP_LEV(ch) == 0)
    strcpy(buf2, "OFF");
  else
    sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

  sprintf(buf,
	  "Hit Pnt Display: %-3s    "
	  "     Brief Mode: %-3s    "
	  " Summon Protect: %-3s\r\n"

	  "   Move Display: %-3s    "
	  "   Compact Mode: %-3s    "
	  "       On Quest: %-3s\r\n"

	  "   Mana Display: %-3s    "
	  "         NoTell: %-3s    "
	  "   Repeat Comm.: %-3s\r\n"

	  " Auto Show Exit: %-3s    "
	  "           Deaf: %-3s    "
	  "     Wimp Level: %-3s\r\n"

	  " Gossip Channel: %-3s    "
	  "Auction Channel: %-3s    "
	  "  Grats Channel: %-3s\r\n"

	  "   Tips Channel: %-3s    "
	  "     Autoassist: %-3s    "
	  "      Autosplit: %-3s\r\n"

	  "       Autoloot: %-3s    "
	  "  Arena Channel: %-3s    "
	  "   Info Channel: %-3s\r\n"

	  "    Color Level: %-9s"
          "Auto Diagnose: %-3s    "
	  "        AutoSac: %-3s\r\n"

          "        Automap: %-3s    "    
          "      Autotitle: %-3s    "
          "  Rank on 'Who': %-3s\r\n"

          "    Grid at lvl: %-3s    "
          "       Autogold: %-3s    "
          "      No Follow: %-3s\r\n",

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
	  ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_SUMMONABLE)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
	  ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
	  YESNO(PRF_FLAGGED(ch, PRF_QUEST)),

	  ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
	  ONOFF(PRF_FLAGGED(ch, PRF_NOTELL)),
	  YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
	  YESNO(PRF_FLAGGED(ch, PRF_DEAF)),
	  buf2,

	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOAUCT)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),

	  ONOFF(PRF_FLAGGED(ch, PRF_SHOWTIPS)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOASSIST)),
	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOSPLIT)),

	  ONOFF(PRF_FLAGGED(ch, PRF_AUTOLOOT)),
	  ONOFF(PRF_FLAGGED(ch, PRF_ARENA)),
	  ONOFF(!PRF_FLAGGED(ch, PRF_NOINFO)),

	  ctypes[COLOR_LEV(ch)],
          ONOFF(PRF_FLAGGED(ch, PRF_AUTODIAG)),
          ONOFF(PRF_FLAGGED(ch, PRF_AUTOSAC)),
          ONOFF(PRF_FLAGGED(ch, PRF_AUTOMAP)),
          ONOFF(PRF_FLAGGED(ch, PRF_AUTOTITLE)),
          ONOFF(!PRF_FLAGGED(ch, PRF_NORANK)),
          ONOFF(!PRF_FLAGGED(ch, PRF_NOGRID)),
          ONOFF(!PRF_FLAGGED(ch, PRF_AUTOGOLD)),
          ONOFF(PRF_FLAGGED(ch, PRF_NOFOLLOW)));
  send_to_char(buf, ch);
}


struct sort_struct {
  int sort_pos;
  byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void sort_commands(void)
{
  int a, b, tmp;

  ACMD(do_action);

  num_of_cmds = 0;

  /*
   * first, count commands (num_of_commands is actually one greater than the
   * number of commands; it inclues the '\n'.
   */
  while (*cmd_info[num_of_cmds].command != '\n')
    num_of_cmds++;

  /* create data array */
  CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

  /* initialize it */
  for (a = 1; a < num_of_cmds; a++) {
    cmd_sort_info[a].sort_pos = a;
    cmd_sort_info[a].is_social = (cmd_info[a].command_pointer == do_action);
  }

  /* the infernal special case */
  cmd_sort_info[find_command("insult")].is_social = TRUE;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
  for (a = 1; a < num_of_cmds - 1; a++)
    for (b = a + 1; b < num_of_cmds; b++)
      if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
		 cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
	tmp = cmd_sort_info[a].sort_pos;
	cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
	cmd_sort_info[b].sort_pos = tmp;
      }
}



ACMD(do_commands)
{
  int no, i, cmd_num;
  int wizhelp = 0, socials = 0;
  struct char_data *vict;

  one_argument(argument, arg);

  if (*arg) {
    if (!(vict = get_char_vis(ch, arg)) || IS_NPC(vict)) {
      send_to_char("Who is that?\r\n", ch);
      return;
    }
    if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
      send_to_char("You can't see the commands of people above your level.\r\n", ch);
      return;
    }
  } else
    vict = ch;

  if (subcmd == SCMD_SOCIALS)
    socials = 1;
  else if (subcmd == SCMD_WIZHELP)
    wizhelp = 1;

  sprintf(buf, "The following %s%s are available to %s:\r\n",
	  wizhelp ? "privileged " : "",
	  socials ? "socials" : "commands",
	  vict == ch ? "you" : GET_NAME(vict));

  /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
  for (no = 1, cmd_num = 1; cmd_num < num_of_cmds; cmd_num++) {
    i = cmd_sort_info[cmd_num].sort_pos;
    if (cmd_info[i].minimum_level >= 0 &&
	GET_LEVEL(vict) >= cmd_info[i].minimum_level &&
	(cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
	(wizhelp || socials == cmd_sort_info[i].is_social)) {
      sprintf(buf + strlen(buf), "%-11s", cmd_info[i].command);
      if (!(no % 7))
	strcat(buf, "\r\n");
      no++;
    }
  }

  strcat(buf, "\r\n");
  send_to_char(buf, ch);
}


/* -20 to 20... */
/*  Return a description for hit/dam roll values */
char *get_desc_from_roll_value(int roll_value)
{
  static char buf[256];
  
  if (roll_value <= 4)
    strcpy(buf, "Pathetic!");
  else if (roll_value <= 10)
    strcpy(buf, "Really bad");
  else if (roll_value <= 20)
    strcpy(buf, "Bad");
  else if (roll_value <= 30)
    strcpy(buf, "Pretty bad");
  else if (roll_value <= 40)
    strcpy(buf, "Not too shabby");
  else if (roll_value <= 50)
    strcpy(buf, "Fair");
  else if (roll_value <= 60)
    strcpy(buf, "Pretty good");
  else if (roll_value <= 70)
    strcpy(buf, "Good");
  else if (roll_value <= 80)
    strcpy(buf, "Very good");
  else if (roll_value <= 90)
    strcpy(buf, "Great");
  else if (roll_value <= 100)
    strcpy(buf, "Outstanding");
  else if (roll_value <= 110)
    strcpy(buf, "Superb");
  else if (roll_value <= 120)
    strcpy(buf, "Awesome");
  else if (roll_value <= 130)
    strcpy(buf, "Sensational");
  else if (roll_value <= 140)
    strcpy(buf, "Phenomenal");
  else if (roll_value >  140 )
    strcpy(buf, "You rule!");
  
  return (buf);
}

/*  Return a description for alignment values */
char *get_align_desc(int alignment)
{
  static char buf[256];
  
  if (alignment <= -750)
    strcpy(buf, "The Devil's Spawn");
  else if (alignment <= -350)
    strcpy(buf, "Bad");
  else if (alignment <= 350)
    strcpy(buf, "Neutral");
  else if (alignment <= 750)
    strcpy(buf, "Nice");
  else strcpy(buf, "Saintly");
  
  return (buf);
}


/* Return the string for the god's role */
char *get_role_text(int level)
{
  static char buf[256];

  switch(level){
  case LVL_IMMORT:   strcpy(buf, "/cwIMMORTAL/c0"); break;
  case LVL_ADMIN:    strcpy(buf, "/cWADMIN/c0"); break;
  case LVL_QUESTOR:  strcpy(buf, "/cYQUESTOR/c0"); break;
  case LVL_BUILDER:  strcpy(buf, "/cGBUILDER/c0"); break;
  case LVL_BUILDER_H:strcpy(buf, "/cGHEAD BUILDER/c0"); break;
  case LVL_CODER:    strcpy(buf, "/cBCODER/c0"); break;
  case LVL_CODER_H:  strcpy(buf, "/cBHEAD CODER/c0"); break;
  case LVL_UNUSED_1: strcpy(buf, "/cWOOPS!/c0"); break;
  case LVL_UNUSED_2: strcpy(buf, "/cWOOPS!/c0"); break;
  case LVL_IMPL:     strcpy(buf, "/cRIMPLEMENTOR/c0"); break;
  default:         sprintf(buf, "/c0%d", level); break;
  }

  return(buf);
}

ACMD(do_scan)
{
  int door;
  int first=0;
  struct char_data *i;
  *buf='\0';

  if (AFF_FLAGGED(ch, AFF_BLIND)) {
    send_to_char("You can't see anything, you're blind!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_IMPEN)) {
    send_to_char("You can't even see THIS room, let alone another!\r\n", ch);
    return;
  }

  send_to_char("You scan the area for signs of life.\r\n", ch);
  act("$n scans the area for signs of life.", TRUE, ch, NULL, NULL, TO_ROOM);


  for (door = 0; door < NUM_OF_DIRS; door++){
    first=0;
    if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
	IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
      sprintf(buf, "%s%5s: Something blocks your view.\r\n", buf, dirs[door]);
    else if ((EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
        !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
        IS_DARK(EXIT(ch, door)->to_room) && !IS_AFFECTED(ch, AFF_INFRAVISION)))
      sprintf(buf, "%s%5s: It's too dark to see.\r\n", buf, dirs[door]);
    else if (EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
        !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
        !IS_SET(EXIT(ch, door)->exit_info, EX_HIDDEN) &&
        world[EXIT(ch, door)->to_room].people) {
      sprintf(buf, "%s/cw%5s: ", buf, dirs[door]);
      for (i = world[EXIT(ch, door)->to_room].people; i; i = i->next_in_room) {
        if (CAN_SEE(ch, i) && GET_LEVEL(i) <= 150) {
          if (!first)
            sprintf(buf, "%s/cc%s/cw\r\n", buf,
                    AFF_FLAGGED(i, AFF_INCOGNITO) ? "Someone" : GET_NAME(i));
          else
            sprintf(buf, "%s       /cc%s/cw\r\n", buf,
                    AFF_FLAGGED(i, AFF_INCOGNITO) ? "Someone" : GET_NAME(i));
          first++;
        }
      }
    }
  }
  if (*buf) send_to_char(buf, ch);
}

ACMD(do_whois)
{
  struct char_data *victim;
  one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Who would you like info on?\r\n", ch);
    return;
  } 
  if (!(victim = get_char_vis(ch, buf)) || IS_NPC(victim)) {
    send_to_char("There are no players currently online with that name.\r\n", ch);
    return;
  }
  if (GET_LEVEL(victim) >= LVL_IMMORT)
  {
	  send_to_char("There are no players currently online with that name.\r\n", ch);
	  return;
  }
  send_to_char("\r\n-----------------------------------------------------\r\n\r\n", ch);
  sprintf(buf2, "/cc  [ /cC%d /cg%d /cb%s /cr%s /cc] /cw%s %s/c0\r\n\r\n"
                "/cw  Age: %d\r\n"
                "  Monsters killed: %d      Times Killed: %d\r\n\r\n",
          GET_TIER(victim), GET_LEVEL(victim), RACE_ABBR(victim),
          CLASS_ABBR(victim), GET_NAME(victim), GET_TITLE(victim),
          GET_AGE(victim),
          (int) GET_NUM_KILLS(victim), (int) GET_NUM_DEATHS(victim));
  send_to_char(buf2, ch);
  if (PLR_FLAGGED(victim, PLR_PKILL_OK))
    send_to_char("/cR  This player has chosen to participate in pkill./c0\r\n", ch);
  else
    send_to_char("/cR  This player has chosen /cwnot/cR to participate in pkill./c0\r\n\r\n", ch);
  send_to_char("/c0-----------------------------------------------------", ch);
  return;
}


ACMD(do_has)
{
int percent = 0;
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


      act(buf, FALSE, ch, 0, 0, TO_ROOM);

      send_to_char(buf, ch);
}


ACMD(do_affects)
{
  struct affected_type* aff;
  char **abil_list;

  sprintbitarray(AFF_FLAGS(ch), affected_bits, AF_ARRAY_MAX, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
  send_to_char(buf, ch);


  if (ch->affected) {
    for (aff = ch->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      switch (aff->src) {
      case ABT_SPELL: abil_list = spells;  break;
      case ABT_CHANT: abil_list = chants;  break;
      case ABT_PRAYER:abil_list = prayers; break;
      case ABT_SONG:  abil_list = songs;   break;
      default: return; /* Damn.  Shouldn't happen! */
      }
   sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
           CCCYN(ch, C_NRM), (aff->type >= 0 && aff->type <= MAX_ABILITIES) ?
           abil_list[aff->type] : "TYPE UNDEFINED", CCNRM(ch, C_NRM));
   if (aff->modifier) {
   sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
     strcat(buf, buf2);
      }
      if (aff->bitvector) {
        if (*buf2)
          strcat(buf, ", sets ");
        else
          strcat(buf, "sets ");
        strcpy(buf2, affected_bits[aff->bitvector]);
        strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  } else
    send_to_char("You are not affected by magic.\r\n", ch);
}

ACMD(do_all)
{
  char **abil_list;
  char buf7[MAX_STRING_LENGTH];
  int i, j = 1;

  *buf7 ='\0';
  
  one_argument(argument, arg);

  if (!is_abbrev(arg, "spells") && 
      !is_abbrev(arg, "skills") && 
      !is_abbrev(arg, "prayers") && 
      !is_abbrev(arg, "chants")) {
    send_to_char("All what?\r\n\r\nCorrect syntax is:\r\nall skills\r\nall spells\r\nall prayers\r\nall chants\r\n", ch);
    return; 
  }

  if (is_abbrev(arg, "spells")) {
    abil_list = spells;

    for (i = 1; i < MAX_SPELLS; i++) {
      sprintf(buf7, "%s%3d %s\r\n", buf7, j, abil_list[i]);
      j++;
    }
    page_string(ch->desc, buf7, 1);
  }
  if (is_abbrev(arg, "skills")) {
    abil_list = skills;

    for (i = 1; i < MAX_SKILLS; i++) {
      sprintf(buf7, "%s%3d %s\r\n", buf7, j, abil_list[i]);
      j++;
    }
    page_string(ch->desc, buf7, 1);
  }
  if (is_abbrev(arg, "chants")) {
    abil_list = chants;

    for (i = 1; i < MAX_CHANTS; i++) {
      sprintf(buf7, "%s%3d %s\r\n", buf7, j, abil_list[i]);
      j++;
    }
    page_string(ch->desc, buf7, 1);
  }
  if (is_abbrev(arg, "prayers")) {
    abil_list = prayers;

    for (i = 1; i < MAX_PRAYERS; i++) {
      sprintf(buf7, "%s%3d %s\r\n", buf7, j, abil_list[i]);
      j++;
    }
    page_string(ch->desc, buf7, 1);
  }
}


ACMD(do_spells){
  extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
  char **abil_list;
  int i = 0, n = 0;
  char spsk[7];

  switch (subcmd) {
  case SCMD_SKILLS:
    strcpy(spsk, "Skill");
    abil_list = skills;
    break;
  case SCMD_SPELLS:
    strcpy(spsk, "Spell");
    abil_list = spells;
    break;
  case SCMD_CHANTS:
    strcpy(spsk, "Chant");
    abil_list = chants;
    break;
  case SCMD_PRAYERS:
    strcpy(spsk, "Prayer");
    abil_list = prayers;
    break;
  case SCMD_SONGS:
    strcpy(spsk, "Song");
    abil_list = songs;
    break;
  default:
    return;
    break;
  }

  if (!CAN_CAST(ch) && subcmd == SCMD_SPELLS) {
    send_to_char("Your class does not know of any spells.\r\n", ch);
    return;
  }
  if (!CAN_CHANT(ch) && subcmd==SCMD_CHANTS) {
    send_to_char("Your class does not know of any chants.\r\n", ch);
    return;
  }
  if (!CAN_PRAY(ch) && subcmd==SCMD_PRAYERS) {
    send_to_char("Your class does not know of any prayers.\r\n", ch);
    return;
  }
  if (!CAN_SING(ch) && subcmd==SCMD_SONGS) {
    send_to_char("Your class does not know of any songs.\r\n", ch);
    return;
  }

  sprintf(buf, "Your class offers the following %ss:\r\n"
               "Lvl %s\r\n------------------------------------\r\n", spsk, spsk);

for(n = 1; n < LVL_IMMORT; n++) {
  for (i = 1; i < MAX_ABILITIES; i++) {
    if (strlen(buf) >= MAX_STRING_LENGTH - 32) {
      strcat(buf, "**OVERFLOW**\r\n");
      break;
    }
    if (abil_info[subcmd][i].min_level[(int) GET_CLASS(ch)] == n)
      sprintf(buf, "%s%3d %s\r\n", buf,
           abil_info[subcmd][i].min_level[(int)GET_CLASS(ch)], abil_list[i]);
  }

}
  page_string(ch->desc, buf, 1);

}

ACMD(do_compare)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    struct obj_data *obj1;
    struct obj_data *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying)))
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content)
        {
            if ( !((obj2->obj_flags.type_flag == ITEM_WEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_FIREWEAPON) ||
                   (obj2->obj_flags.type_flag == ITEM_ARMOR) ||
                   (obj2->obj_flags.type_flag == ITEM_WORN))
            &&   CAN_SEE_OBJ(ch, obj2)
            &&   obj1->obj_flags.type_flag == obj2->obj_flags.type_flag
            && CAN_GET_OBJ(ch, obj2) )
                break;
        }

        if (!obj2)
        {
            send_to_char( "You aren't wearing anything comparable.\n\r", ch );
            return;
        }
    }
    else
    {
        if (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)))
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }
    }

    msg         = NULL;
    value1      = 0;
    value2      = 0;

    if (obj1 == obj2)
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->obj_flags.type_flag != obj2->obj_flags.type_flag )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch (obj1->obj_flags.type_flag)
        {
        default:
            msg = "You can't compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->obj_flags.value[0];
            value2 = obj2->obj_flags.value[0];
            break;

        case ITEM_WEAPON:
            value1 = obj1->obj_flags.value[1] + obj1->obj_flags.value[2];
            value2 = obj2->obj_flags.value[1] + obj2->obj_flags.value[2];
            break;
        }
    }

    if (!msg)
    {
             if (value1 == value2) msg = "$p and $P look about the same.";
        else if (value1  > value2) msg = "$p looks better than $P.";
        else                         msg = "$p looks worse than $P.";
    }

    act(msg, FALSE, ch, obj1, obj2, TO_CHAR);
    return;
}



/*
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *obj1, *obj2;
  float avg_dam1, avg_dam2;

  two_arguments(argument, arg1, arg2);

  if (!*arg1) {
    send_to_char("Compare what?\r\n", ch);
    return;
  } else if (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
    send_to_char(buf, ch);
    return;
  } else if (!*arg2) {
    send_to_char("What do you want to compare it to?\r\n", ch);
    return;
  } else if (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg2), arg2);
    send_to_char(buf, ch);
    return;
  } else {
    if (obj1->obj_flags.type_flag != obj2->obj_flags.type_flag) {
      send_to_char("I don't know how to compare different types "
                   "of objects.\r\n", ch);
    } else {
      sprintf(buf, "Comparing: %s and %s.\r\n\r\n",
              obj1->short_description, obj2->short_description);
      switch(obj1->obj_flags.type_flag) {
      case ITEM_WEAPON:
        avg_dam1 = ((GET_OBJ_VAL(obj1, 2) + 1) / 2.0) * GET_OBJ_VAL(obj1, 1);
        avg_dam2 = ((GET_OBJ_VAL(obj2, 2) + 1) / 2.0) * GET_OBJ_VAL(obj2, 1);
        if (avg_dam1 > avg_dam2)
          sprintf(buf, "%s%s will cause more damage on average.\r\n",
                        buf, obj1->short_description);
        else if (avg_dam2 > avg_dam1)
          sprintf(buf, "%s%s will cause more damage on average.\r\n",
                        buf, obj2->short_description);
        else
          sprintf(buf, "%sThese weapons will cause the same average "
                       "damage per round.\r\n", buf);
        break;
      case ITEM_ARMOR:
         ITEM_WEAR_TAKE doesn't count. 
        if ((GET_OBJ_WEAR(obj1)[0] & GET_OBJ_WEAR(obj2)[0]) > 1) {
          send_to_char("They protect different body parts.\r\n", ch);
          return; 
        }
        if (GET_OBJ_VAL(obj1, 0) > GET_OBJ_VAL(obj2, 0))
          sprintf(buf, "%sIt looks like %s will protect you better.\r\n",
                        buf, obj1->short_description);
        else if (GET_OBJ_VAL(obj1, 0) < GET_OBJ_VAL(obj2, 0))
          sprintf(buf, "%sIt looks like %s will protect you better.\r\n",
                        buf, obj2->short_description);
        else
          sprintf(buf, "%sThese items seem like they will protect you "
                       "equally well.\r\n", buf);
        break;
      default:
        send_to_char("I don't know how to compare them.\r\n", ch);
        return;
        break;
      };
      if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
        sprintf(buf, "%s%s looks more valuable.\r\n",
                     buf, obj1->short_description);
      else if (GET_OBJ_COST(obj1) > GET_OBJ_COST(obj2))
        sprintf(buf, "%s%s looks more valuable.\r\n",
                     buf, obj2->short_description);
      else
        sprintf(buf, "%sThese items look to be of similar value\r\n", buf);
      send_to_char(buf, ch);
    }
    return;
  }

}
*/


/* OBJ LIST HTML */
void make_facelist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("facelist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_FACE)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv facelist.tmp ~/../httpd/html/dibrova_objs/face_eq%d.txt &", port);
  system(mvcmd);
}

void make_eyelist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("eyelist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_EYES)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv eyelist.tmp ~/../httpd/html/dibrova_objs/eye_eq%d.txt &", port);
  system(mvcmd);
}

void make_headlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("headlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_HEAD)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv headlist.tmp ~/../httpd/html/dibrova_objs/head_eq%d.txt &", port);
  system(mvcmd);
}

void make_earlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("earlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_EAR)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv earlist.tmp ~/../httpd/html/dibrova_objs/ear_eq%d.txt &", port);
  system(mvcmd);
}

void make_necklist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("necklist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_NECK)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv necklist.tmp ~/../httpd/html/dibrova_objs/neck_eq%d.txt &", port);
  system(mvcmd);
}

void make_holdlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("holdlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_HOLD)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv holdlist.tmp ~/../httpd/html/dibrova_objs/hold_eq%d.txt &", port);
  system(mvcmd);
}

void make_armlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("armlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_ARMS)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv armlist.tmp ~/../httpd/html/dibrova_objs/arm_eq%d.txt &", port);
  system(mvcmd);
}  

void make_bodylist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("bodylist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_BODY)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv bodylist.tmp ~/../httpd/html/dibrova_objs/body_eq%d.txt &", port);
  system(mvcmd);
}

void make_aboutlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("aboutlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_ABOUT)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv aboutlist.tmp ~/../httpd/html/dibrova_objs/about_eq%d.txt &", port);
  system(mvcmd);
}  

void make_floatlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("floatlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_FLOAT_1)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv floatlist.tmp ~/../httpd/html/dibrova_objs/float_eq%d.txt &", port);
  system(mvcmd);
}

void make_handlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("handlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_HANDS)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv handlist.tmp ~/../httpd/html/dibrova_objs/hand_eq%d.txt &", port);
  system(mvcmd);
}

void make_thumblist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("thumblist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_THUMB)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv thumblist.tmp ~/../httpd/html/dibrova_objs/thumb_eq%d.txt &", port);
  system(mvcmd);
}

void make_waistlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("waistlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_WAIST)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv waistlist.tmp ~/../httpd/html/dibrova_objs/waist_eq%d.txt &", port);
  system(mvcmd);
}

void make_leglist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("leglist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_LEGS)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv leglist.tmp ~/../httpd/html/dibrova_objs/leg_eq%d.txt &", port);
  system(mvcmd);
}

void make_wieldlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("wieldlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_WIELD)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv wieldlist.tmp ~/../httpd/html/dibrova_objs/wield_eq%d.txt &", port);
  system(mvcmd);
}

void make_shieldlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("shieldlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_SHIELD)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv shieldlist.tmp ~/../httpd/html/dibrova_objs/shield_eq%d.txt &", port);
  system(mvcmd);
}

void make_feetlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("feetlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_FEET)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv feetlist.tmp ~/../httpd/html/dibrova_objs/feet_eq%d.txt &", port);
  system(mvcmd);
}

void make_wristlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("wristlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_WRIST)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv wristlist.tmp ~/../httpd/html/dibrova_objs/wrist_eq%d.txt &", port);
  system(mvcmd);
}

void make_fingerlist_html(void) {

  FILE *fp;
  char objects[MAX_STRING_LENGTH * 7];
  char mvcmd[256];
  int nr;

  if ((fp = fopen("fingerlist.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  objects[0] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++) {
    if (CAN_WEAR(&obj_proto[nr], ITEM_WEAR_FINGER)) {
      sprintf(objects, "%d&&%d&&%s\n",
      obj_index[nr].virtual, obj_proto[nr].obj_flags.minlevel,
      obj_proto[nr].short_description);
      proc_color(objects, 0);
      fprintf(fp, objects);
    }
  }

  fclose(fp);
  sprintf(mvcmd, "mv fingerlist.tmp ~/../httpd/html/dibrova_objs/finger_eq%d.txt &", port);
  system(mvcmd);
} 

/* END OBJ LIST HTML */


void make_who2html(void) {

  extern struct descriptor_data *descriptor_list;
  extern char *class_abbrevs[];
  FILE *fp;
  struct descriptor_data *d;
  struct char_data *ch;
  char players[MAX_STRING_LENGTH];
  char clanbuf[MAX_STRING_LENGTH];
  char mvcmd[256];
  int ct=0, i=0;

  if ((fp = fopen("who.tmp", "w")) == 0) {
    perror("Failed to open who.tmp for writing.");
    return;
  }

  players[0] = '\0';
  clanbuf[0] = '\0';

  fprintf(fp, "<html>\n"
    "<!-- Dibrova pages are designed for 800x600 displays -->\n"
    "<style>\n"
    "<!--\n"
    "  A:link {color:black;text-decoration:none;}\n"
    "  A:visited {color:black;text-decoration:none;}\n"
    "  A:hover {color:red;text-decoration:underline;}\n"
    "-->\n"
    "</style>\n\n"
    "<head>\n"
    "<title>Who's on?</title>\n"
    "<basefont face=\"times, helvetica\" size=\"1\">\n"
    "</head>\n");

  fprintf(fp, "<html>\n"
    "<body bgcolor=\"#ffffff\">\n"
    "<table>\n"
    "<!-- Left side (navigation) -->\n"
    "<tr><td width=\"160\" valign=\"top\">\n"
    "<font size=\"4\">\n"
    "<a href=index.shtml><img src=Home.gif border=0><br>\n"
    "<a href=hometowns.html><img src=Hometowns.gif border=0>\n"
    "<a href=classes.html><img src=Classes.gif border=0>\n"
    "<a href=races.html><img src=Races.gif border=0>\n"
    "<a href=who4000.html><img src=Whoison.gif border=0>\n"
    "<a href=kaan/zones/kaan.html><img src=Areas.gif border=0>\n"
    "<a href=kaan/kaan.html><img src=Building.gif border=0>\n"
    "<a href=code.html><img src=Code.gif border=0>\n"
    "<a href=links.html><img src=Links.gif border=0>\n"
    "<a href=contact.html><img src=Contacts.gif border=0>\n"
    "<a href=about.html><img src=About.gif border=0>\n"
    "<a href=map.htm><img src=Maps.gif border=0>\n"
    "</font>\n"
    "</td>\n\n");

  fprintf(fp, "<!-- Right side -->\n"
    "<td valign=\"top\" width=\"640\">\n"
    "<font size=\"12\">W</font><font size=\"6\">ho's \n"
    "<font size=\"12\">O</font>n</font><br>\n"
    "<img src=\"img/thinline.jpg\">\n");

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      if (d->original)
        ch = d->original;
      else if (!(ch = d->character) || GET_INVIS_LEV(ch))
        continue;
      ct++;

      if (GET_LEVEL(ch) < LVL_IMMORT)
        sprintf(players, "%s<strong><font size=\"4\" color=#000000>[</font>"
           "<font color=#ff0000>%d</font>"
           " <a href=races.html#_%s><font color=#0000ff>%s</font></a>"
           " <a href=classes.html#_%s><font color=#00ff00>%s</font></a>"
           "<font color=#000000>] %s %s %s (%s of %s)</font></strong><br>\n", players,
           GET_LEVEL(ch), RACE_ABBR(ch), RACE_ABBR(ch),
           CLASS_ABBR(ch), CLASS_ABBR(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)], GET_NAME(ch), GET_TITLE(ch), clan[find_clan_by_id(GET_CLAN(ch))].rank_name[GET_CLAN_RANK(ch) -1], clan[find_clan_by_id(GET_CLAN(ch))].name);
      else {
        /* Hide wizinvis yokels... */
	if (GET_INVIS_LEV(ch) >= LVL_IMMORT) {
        continue; }
        // Kaan is a wuss
        if(isname("Kaan", GET_NAME(ch))) { continue; }
        sprintbit(GET_GOD_TYPE(ch), god_types, buf1);
        sprintf(buf, "<p><strong><font color=black>%s %s - %s %s</font><strong><p>\n",
                god_lvls[GET_LEVEL(ch) - LVL_IMMORT], buf1,
                GET_NAME(ch), GET_TITLE(ch)); 
      proc_color(buf, 0);
      fprintf(fp, buf);
    }
  } 
  if (*players) {
    proc_color(players, 0);
    fprintf(fp, players);
  }
  if (!ct)
    fprintf(fp,"<p>Nobody!</p>\n");

  fprintf(fp, "<P><hr><P><TABLE BORDER=0 CELLPADDING=0 CELLSPACING=10><tr><td><strong>Active Clans:</td></tr></strong>\n");
  for(i=1; i < num_of_clans; i++)
    sprintf(clanbuf, "%s<tr><td><strong>Clan:</strong> [%-20s]</td><td> <strong>Members:</strong> [%3d]</td><td>  <strong>Power:</strong> [%5d]</td></tr>  <BR>\r\n", clanbuf, clan[i].name, clan[i].members, clan[i].power);

  proc_color(clanbuf, 0);
  fprintf(fp, clanbuf);
  fprintf(fp, "</TABLE>\n");

  fprintf(fp, "</td></table></font>\n\n</body>\n</html>\n");
  fclose(fp);
  sprintf(mvcmd, "mv who.tmp ~/public_html/who.html &");
  system(mvcmd);
}

ACMD(do_tnl)
{

  char tmp[256];

  if (GET_LEVEL(ch) > LVL_IMMORT-1) {
    send_to_char("You are all leveled out!\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) == 150) {
    sprintf(tmp, "$n needs %d experience to remort.", (1900000000 - GET_EXP(ch)));
    act(tmp, FALSE, ch, 0, 0, TO_ROOM);
    sprintf(tmp, "You need %d experience to remort.", (1900000000 - GET_EXP(ch)));
    act(tmp, FALSE, ch, 0, 0, TO_CHAR);
    return;
  }

  if ((level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1) - GET_EXP(ch)) < 1 ) {
    act("$n is ready to gain another level!", FALSE, ch, 0, 0, TO_ROOM);
    act("You are ready to gain another level!", FALSE, ch, 0, 0, TO_CHAR);
  } else {
    sprintf(tmp, "$n needs %d to gain another level.", level_exp(GET_CLASS(ch),
               GET_LEVEL(ch) + 1) - GET_EXP(ch));
    act(tmp, FALSE, ch, 0, 0, TO_ROOM);
    sprintf(tmp, "You need %d to gain another level.", level_exp(GET_CLASS(ch),
               GET_LEVEL(ch) + 1) - GET_EXP(ch));
    act(tmp, FALSE, ch, 0, 0, TO_CHAR);
  }
}

#define IS_DOOR(x, y)   IS_SET(world[x].dir_option[y]->exit_info, EX_CLOSED)
#define IS_HIDDEN(x, y) IS_SET(world[x].dir_option[y]->exit_info, EX_HIDDEN)

void map_draw_room(char map[9][10], int x, int y, room_rnum rnum,
struct char_data *ch)
{
  int door;

  map[y][x] = 'O';

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (!world[rnum].dir_option[door] || world[rnum].dir_option[door]->to_room == NOWHERE)
      continue; 
   
   /* Hides hidden exits */
   if(IS_HIDDEN(rnum, door))
      continue;    
       
    switch (door) {
        case NORTH: 
          IS_DOOR(rnum, door) ? (map[y-1][x] = '+') : (map[y-1][x] = '|'); 
          break;
        case EAST:  
          IS_DOOR(rnum, door) ? (map[y][x+1] = '[') : (map[y][x+1] = '-');
          break; 
        case SOUTH: 
          IS_DOOR(rnum, door) ? (map[y+1][x] = '+') : (map[y+1][x] = '|');
          break; 
        case WEST: 
          IS_DOOR(rnum, door) ? (map[y][x-1] = ']') : (map[y][x-1] = '-');
          break; 
        case UP:    
          IS_DOOR(rnum, door) ? (map[y-1][x+1] = 'x') : (map[y-1][x+1] = '/');
          break;
        case DOWN:
          IS_DOOR(rnum, door) ? (map[y+1][x-1] = 'x') : (map[y+1][x-1] = '/');
          break;
    }
    
  }
}

ACMD(do_map)
{
  int door, i;
  char map[9][10];

  /* blank the map */
  for(i=0;i<9;i++) {
    strcpy(map[i], "         ");
  }

  /* print out exits */
  map_draw_room(map, 4, 4, ch->in_room, ch);
  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (!EXIT(ch, door) || EXIT(ch, door)->to_room == NOWHERE) 
      continue;
    /* Make sure we don't branch off on hidden or exits with doors*/
    if (IS_DOOR(ch->in_room, door) || IS_HIDDEN(ch->in_room, door))
      continue;
    
    switch (door) {
        case NORTH: map_draw_room(map, 4, 1,EXIT(ch,door)->to_room,
ch); break;
        case EAST:  map_draw_room(map, 7, 4,EXIT(ch,door)->to_room,
ch); break;
        case SOUTH: map_draw_room(map, 4, 7,EXIT(ch,door)->to_room,
ch); break;
        case WEST:  map_draw_room(map, 1, 4,EXIT(ch,door)->to_room,
ch); break;
      }
    
  }

  /* make it obvious what room they are in */
  map[4][4] = '#';

  /* print out the map */
  for(i=0;i<9;i++) {
    sprintf(buf2, "%s\r\n", map[i]);
    send_to_char(buf2, ch);
  }
}

void make_newwho2html(void) {

  extern struct descriptor_data *descriptor_list;
  extern char *class_abbrevs[];
  FILE *fp;
  struct descriptor_data *d;
  struct char_data *ch;
  char players[MAX_STRING_LENGTH];
  char clanbuf[MAX_STRING_LENGTH];
  char mvcmd[256];
  int ct=0, i=0;

  if ((fp = fopen("newwho.tmp", "w")) == 0) {
    perror("Failed to open newwho.tmp for writing.");
    return;
  }

  players[0] = '\0';
  clanbuf[0] = '\0';

  for (d = descriptor_list; d; d = d->next)
    if (!d->connected) {
      if (d->original)
        ch = d->original;
      else if (!(ch = d->character) || GET_INVIS_LEV(ch))
        continue;
      ct++;

      if (GET_LEVEL(ch) < LVL_IMMORT)
       if(GET_CLAN(ch)) {
        sprintf(players, "%s<strong>[</font>"
           "<font color=#ff0000>%d</font>"
           " <a href=races.shtml#_%s><font color=#0000ff>%s</font></a>"
           " <a href=classes.shtml#_%s><font color=#00ff00>%s</font></a>"
           "<font color=#000000>] %s %s %s (%s of %s)</font></strong><br>\n", players,
           GET_LEVEL(ch), RACE_ABBR(ch), RACE_ABBR(ch),
           CLASS_ABBR(ch), CLASS_ABBR(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)], GET_NAME(ch), GET_TITLE(ch), clan[find_clan_by_id(GET_CLAN(ch))].rank_name[GET_CLAN_RANK(ch) -1], clan[find_clan_by_id(GET_CLAN(ch))].name); 
       }
       else {
        sprintf(players, "%s<strong>[</font>"
           "<font color=#ff0000>%d</font>"
           " <a href=races.shtml#_%s><font color=#0000ff>%s</font></a>"
           " <a href=classes.shtml#_%s><font color=#00ff00>%s</font></a>"
           "<font color=#000000>] %s %s %s</font></strong><br>\n", players,
           GET_LEVEL(ch), RACE_ABBR(ch), RACE_ABBR(ch),
           CLASS_ABBR(ch), CLASS_ABBR(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)], GET_NAME(ch), GET_TITLE(ch));
       }
      else {
        /* Hide wizinvis yokels... */
	if (GET_INVIS_LEV(ch) >= LVL_IMMORT) {
        continue; }
        // Kaan is a wuss
        if(isname("Kaan", GET_NAME(ch))) { continue; }
        sprintbit(GET_GOD_TYPE(ch), god_types, buf1);
        sprintf(buf, "<p><strong><font color=black>%s %s - %s %s</font><strong><p>\n",
                god_lvls[GET_LEVEL(ch) - LVL_IMMORT], buf1,
                GET_NAME(ch), GET_TITLE(ch)); 
      proc_color(buf, 0);
      fprintf(fp, buf);
    }
  } 
  if (*players) {
    proc_color(players, 0);
    fprintf(fp, players);
  }
  if (!ct)
    fprintf(fp,"<p>Nobody!</p>\n");

  fprintf(fp, "<P><hr><P><strong>Active Clans:</strong><br>\n");
  for(i=1; i < num_of_clans; i++)
    sprintf(clanbuf, "%s<strong>Clan: </strong> [%-20s] <strong>Members: </strong> [%3d] <strong>Power: </strong> [%5d]<BR>\r\n", clanbuf, clan[i].name, clan[i].members, clan[i].power);

  proc_color(clanbuf, 0);
  fprintf(fp, clanbuf);
  fclose(fp);
  sprintf(mvcmd, "mv newwho.tmp ~/public_html/newwho%d.html &", port);
  system(mvcmd);
}


ACMD(do_finger)
{
  struct char_file_u vbuf;

  skip_spaces(&argument);
  if (load_char(argument, &vbuf) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if (vbuf.level >= LVL_IMMORT) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  } else {
    sprintf(buf, "/cw-------------- Player: %s --------------/c0\r\n"
                 "\r\nLevel %d %s %s\r\n\r\n"
                 "%s was last on: %s\r\n\r\n",
                 vbuf.name, vbuf.level,
                 race_abbrevs[(int) vbuf.race],
                 class_abbrevs[(int) vbuf.class], 
                 vbuf.name, ctime(&vbuf.last_logon));
    send_to_char(buf, ch);
  }
}
