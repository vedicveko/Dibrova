/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "dg_scripts.h"

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern int newbie_level;
extern int max_exp_gain;
extern int max_exp_loss;
extern int double_exp;
struct message_list weapon_messages[MAX_MESSAGES];/* standard messages    */
struct message_list spell_messages[MAX_MESSAGES]; /* spell messages    */

int level_exp(int class, int level);
char *title_male(int class, int level);
char *title_female(int class, int level);
void save_corpses(void);


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

  if (age < 15)
    return (p0);		/* < 15   */
  else if (age <= 29)
    return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
  else if (age <= 44)
    return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
  else if (age <= 59)
    return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
  else if (age <= 79)
    return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
  else
    return (p6);		/* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* manapoint gain pr. game hour */
int mana_gain(struct char_data * ch)
{
  int gain;

  if (IS_NPC(ch)) {
    /* Neat and fast */
    gain = GET_LEVEL(ch);
  } else {
    gain = graf(age(ch).year, 4, 8, 12, 16, 12, 10, 8);

    /* Class calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain <<= 1;
      break;
    case POS_RESTING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_SITTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    }

    if ((IS_SORCERER(ch)) || (IS_CLERIC(ch)) ||
        (GET_CLASS(ch) >= CLASS_BARD && GET_CLASS(ch) < CLASS_VAMPIRE)) 
      gain <<= 1;
  }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_REGENERATE))
    gain *= 1.5;
  
   if(IS_AFFECTED(ch, AFF_BERSERK))
      gain = 0;


  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID) ||
     (zone_table[world[real_room(20100)].zone].number ==
      zone_table[world[IN_ROOM(ch)].zone].number)) {
    return 0;
  } else {
  return (gain);
  }
}


int hit_gain(struct char_data * ch)
/* Hitpoint gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
    /* Neat and fast */
  } else {

    gain = graf(age(ch).year, 8, 12, 20, 32, 16, 10, 4);

    /* Class/Level calculations */

    /* Skill/Spell calculations */

    /* Position calculations    */

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);	/* Divide by 8 */
      break;
    }

    if ((IS_SORCERER(ch)) || (IS_CLERIC(ch)))
      gain >>= 1;
  }

  if (GET_CLASS(ch) == CLASS_VAMPIRE)
    gain = 0;

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_REGENERATE))
    gain *= 1.5;

  if (GET_SKILL(ch, SKILL_FAST_HEAL) >= 1)
    gain += (GET_SKILL(ch, SKILL_FAST_HEAL) * 2);

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID) ||
     (zone_table[world[real_room(20100)].zone].number ==
      zone_table[world[IN_ROOM(ch)].zone].number)) {
    return 0;
  } else {
  return (gain);
  } 
}



int move_gain(struct char_data * ch)
/* move gain pr. game hour */
{
  int gain;

  if (IS_NPC(ch)) {
    return (GET_LEVEL(ch));
    /* Neat and fast */
  } else {
    gain = graf(age(ch).year, 16, 20, 24, 20, 16, 12, 10);

    /* Class/Level calculations */

    /* Skill/Spell calculations */
    if (IS_AFFECTED(ch, AFF_REGENERATE))
      gain *= 1.5;

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POS_SLEEPING:
      gain += (gain >> 1);	/* Divide by 2 */
      break;
    case POS_RESTING:
      gain += (gain >> 2);	/* Divide by 4 */
      break;
    case POS_SITTING:
      gain += (gain >> 3);	/* Divide by 8 */
      break;
    }

  if (IS_AFFECTED(ch, AFF_POISON))
    gain >>= 2;

  if (GET_SKILL(ch, SKILL_FAST_HEAL) >= 1)
    gain += (GET_SKILL(ch, SKILL_FAST_HEAL) + 5);

  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;

  if (IS_AFFECTED(ch, AFF_BERSERK))
    gain = 0;

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID) ||
     (zone_table[world[real_room(20100)].zone].number ==
      zone_table[world[IN_ROOM(ch)].zone].number)) { 
    return 0;
  } else {
  return (gain);
  } 
  }
}



void set_title(struct char_data * ch, char *title)
{
  if (title == NULL) {
    if (GET_SEX(ch) == SEX_FEMALE)
      title = title_female(GET_CLASS(ch), GET_LEVEL(ch));
    else
      title = title_male(GET_CLASS(ch), GET_LEVEL(ch));
  }

  if (strlen(title) > MAX_TITLE_LENGTH)
    title[MAX_TITLE_LENGTH] = '\0';

  if (GET_TITLE(ch) != NULL)
    free(GET_TITLE(ch));

  GET_TITLE(ch) = str_dup(title);
}

void set_descrip(struct char_data * ch, char *descrip)
{
  if (strlen(descrip) > MAX_INPUT_LENGTH)
    descrip[MAX_INPUT_LENGTH] = '\0';

  if (ch->player.long_descr != NULL)
    free(ch->player.long_descr);

  ch->player.long_descr = str_dup(descrip);
}


void check_autowiz(struct char_data * ch)
{
#ifndef CIRCLE_UNIX
  return;
#else
  char buf[100];
  extern int use_autowiz;
  extern int min_wizlist_lev;

  if (use_autowiz && GET_LEVEL(ch) >= LVL_IMMORT) {
    sprintf(buf, "nice ../bin/autowiz %d %s %d %s %d &", min_wizlist_lev,
	    WIZLIST_FILE, LVL_IMMORT, IMMLIST_FILE, (int) getpid());
    mudlog("Initiating autowiz.", CMP, LVL_IMMORT, FALSE);
    system(buf);
  }
#endif /* CIRCLE_UNIX */
}


#define GRID_TOKEN   25020
void gain_exp(struct char_data * ch, int gain)
{

  bool is_altered = FALSE;
  int num_levels = 0, r_num;
  struct obj_data *obj;
  struct obj_data *item;
 
  if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 1 || GET_LEVEL(ch) >= LVL_IMMORT)))
    return;

  if (IS_NPC(ch)) {
    GET_EXP(ch) += gain;
    return;
  }
  if (gain > 0) {
    gain = MIN(max_exp_gain, gain);	/* put a cap on the max gain per kill */
    GET_EXP(ch) += gain;
    if (double_exp >= 1)
      GET_EXP(ch) += gain;
  } else if (gain < 0) {
    gain = MAX(-max_exp_loss, gain);	/* Cap max exp lost per death */
    GET_EXP(ch) += gain;
    if (double_exp >= 1)
      GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0) {
      GET_EXP(ch) = 0;
      return;
    }
  }


    if(GET_EXP(ch) < 0) {
      GET_EXP(ch) = 10;
      return;
    }

/*
    while (GET_LEVEL(ch) < LVL_IMMORT && GET_CLASS(ch) < CLASS_VAMPIRE &&
	GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }
*/

    while (GET_LEVEL(ch) < (LVL_IMMORT-1) && GET_CLASS(ch) < CLASS_VAMPIRE &&
           GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
      if (GET_CLASS(ch) <= 8) {
        obj = read_object(GRID_TOKEN, VIRTUAL);
        obj_to_char(obj, ch);
        send_to_char("You receive a Grid Token!\r\n", ch);
      }
    }

    if (is_altered) {
      sprintf(buf, "%s advanced %d level%s to level %d.",
		GET_NAME(ch), num_levels, num_levels == 1 ? "" : "s",
		GET_LEVEL(ch));
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "\007\007You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }

      switch(GET_LEVEL(ch)) {
      case 10: case 20: case 30: case 40: case 50: case 60: case 70: case 80:
      case 90: case 100: case 110: case 120: case 130: case 140: case 150:
        r_num = real_object(25031);
        item = read_object(r_num, REAL);
        item->obj_flags.player = GET_IDNUM(ch);
        obj_to_char(item, ch);
        send_to_char("Your level is a multiple of ten, you get an arena token!\r\nIt is yours and yours alone!\r\nNow go to the Midgaard Arena Master for a quest!\r\n", ch);
        break;
      default:
        break;
      }
      if (GET_LEVEL(ch) == newbie_level+1)
        send_to_char("/cWWARNING: You will no longer be able to "
                     "recall for free!/c0", ch);

     // check_autowiz(ch);
    } 


    if (GET_CLASS(ch) == CLASS_VAMPIRE) {
    if (GET_LEVEL(ch) == 30) {
      if (GET_EXP(ch) >= level_exp(GET_CLASS(ch),
          GET_LEVEL(ch) + 1)) {
        GET_MAX_HIT(ch) += number(3, 10);
        GET_LEVEL(ch) = 1;
        GET_TIER(ch) += 1;
        GET_EXP(ch) = 1;
        send_to_char("\r\n/cRYou have gained in strength and stamina, as well "
                     "as another tier.\r\nSoon you "
                     "will rule over all vampire!/c0\r\n\r\n", ch);
       sprintf(buf, "/cR%s has attained tier %d!/c0\r\n", GET_NAME(ch), GET_TIER(ch));
        send_to_all(buf);
      }
    }
    if (GET_LEVEL(ch) != LVL_IMMORT - 1 &&
        GET_LEVEL(ch) < LVL_IMMORT) {
      if (GET_EXP(ch) >= level_exp(GET_CLASS(ch),
          GET_LEVEL(ch) + 1)) {
        GET_LEVEL(ch) += 1;
        advance_level(ch);
        send_to_char("\r\n/cRYou have gained in strength and stamina. Soon you "
                     "will rule over all vampire!/c0\r\n\r\n", ch);
      }
    }
  }

  if(GET_EXP(ch) < 1) {
     GET_EXP(ch) =10;
  }

  if (GET_CLASS(ch) == CLASS_TITAN) {
    if (GET_LEVEL(ch) != LVL_IMMORT - 1 &&
        GET_LEVEL(ch) < LVL_IMMORT) {
      if (GET_EXP(ch) >= level_exp(GET_CLASS(ch),
          GET_LEVEL(ch) + 1)) {
        GET_LEVEL(ch) += 1;
        advance_level(ch);
        send_to_char("\r\n/cRYou have gained in strength and power. Soon you "
                     "will be Lord of all Titans!/c0\r\n\r\n", ch);
      }
    }
  }
  if (GET_CLASS(ch) == CLASS_SAINT) {
    if (GET_LEVEL(ch) != LVL_IMMORT - 1 &&
        GET_LEVEL(ch) < LVL_IMMORT) {
      if (GET_EXP(ch) >= level_exp(GET_CLASS(ch),
          GET_LEVEL(ch) + 1)) {
        GET_LEVEL(ch) += 1;
        advance_level(ch);
        send_to_char("\r\n/cRYou're devotion to purity has gained you strength "
               "and honor. Soon you will be among the Holiest!/c0\r\n\r\n", ch);
      }
    }
  }
  if (GET_CLASS(ch) == CLASS_DEMON) {
    if (GET_LEVEL(ch) != LVL_IMMORT - 1 &&
        GET_LEVEL(ch) < LVL_IMMORT) {
      if (GET_EXP(ch) >= level_exp(GET_CLASS(ch),
          GET_LEVEL(ch) + 1)) {
        GET_LEVEL(ch) += 1;
        advance_level(ch);
        send_to_char("\r\n/cRYou have gained in strength and arcane power.  "
                     "Soon you will rule over all demonkind!/c0\r\n\r\n", ch);
      }
    }
  }
}



void gain_exp_regardless(struct char_data * ch, int gain)
{
  int is_altered = FALSE;
  int num_levels = 0;

  GET_EXP(ch) += gain;
  if (GET_EXP(ch) < 0) {
    GET_EXP(ch) = 10;
    return;
  }

  if (!IS_NPC(ch)) {
    while (GET_LEVEL(ch) < LVL_IMPL &&
	GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)) {
      GET_LEVEL(ch) += 1;
      num_levels++;
      advance_level(ch);
      is_altered = TRUE;
    }

    if (is_altered) {
      if (num_levels == 1)
        send_to_char("You rise a level!\r\n", ch);
      else {
	sprintf(buf, "You rise %d levels!\r\n", num_levels);
	send_to_char(buf, ch);
      }
      if(!PRF_FLAGGED(ch, PRF_AUTOTITLE))
        set_title(ch, NULL);
      check_autowiz(ch);
    }
  }
}




void gain_condition(struct char_data * ch, int condition, int value)
{
  bool intoxicated;
  int move_loss = 0;

  if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
    GET_COND(ch, condition) = -1;
  }

  if (GET_COND(ch, condition) == -1)	/* No change */
    return;

  intoxicated = (GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition) += value;

  GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
  GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

  if (GET_COND(ch, condition) || PLR_FLAGGED(ch, PLR_WRITING))
    return;

  if(weather_info.sky == SKY_SNOWING && OUTSIDE(ch)) {
    send_to_char("The cold of the falling snow saps your strength. You feel sleepy.\r\n", ch);
    if(GET_LEVEL(ch) >= newbie_level) { 
      move_loss = number(10, 30);
      if(GET_MOVE(ch) > move_loss) {
         GET_MOVE(ch) = GET_MOVE(ch) - move_loss;
      }
    } 

  }

  switch (condition) {
  case FULL:
    send_to_char("You are hungry.\r\n", ch);
      if (GET_LEVEL(ch) >= newbie_level && GET_COND(ch, FULL) == 0) {
        send_to_char("You must eat! You are starving yourself!\r\n", ch);
          if (GET_HIT(ch) >= 6) {
            GET_HIT(ch) = MAX(5, GET_HIT(ch) - number(5, 15));
          }
      }
    return;
  case THIRST:
    send_to_char("You are thirsty.\r\n", ch);
      if (GET_LEVEL(ch) >= newbie_level && GET_COND(ch, THIRST) == 0) {
        send_to_char("You must find liquid! You are getting weak!\r\n", ch);
          if (GET_HIT(ch) >= 6) {
            GET_HIT(ch) = MAX(5, GET_HIT(ch) - number(5, 15));
          }
      }
    return;
  case DRUNK:
    if (intoxicated)
      send_to_char("You are now sober.\r\n", ch);
    return;
  default:
    break;
  }

}


void check_idling(struct char_data * ch)
{
  extern int free_rent;
  void Crash_rentsave(struct char_data *ch, int cost);

  if (++(ch->char_specials.timer) > 8) {
    if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
      GET_WAS_IN(ch) = ch->in_room;
      if (FIGHTING(ch)) {
	stop_fighting(FIGHTING(ch));
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\r\n", ch);
      save_char(ch, NOWHERE);
      Crash_crashsave(ch);
      char_from_room(ch);
      char_to_room(ch, 1);
    
/* CHANGED was 48 */
  } else if (ch->char_specials.timer > 48) {
      if (ch->in_room != NOWHERE) 
	char_from_room(ch);
      char_to_room(ch, 3);
      if (ch->desc)
	close_socket(ch->desc);
      ch->desc = NULL;
      if (free_rent)
	Crash_rentsave(ch, 0);
      else
	Crash_idlesave(ch);
      sprintf(buf, "%s force-rented and extracted (idle).", GET_NAME(ch));
      mudlog(buf, CMP, LVL_ADMIN, TRUE);
      extract_char(ch);
    }
  }

}


void newbie_point_update(void)
{
  struct char_data *i, *next_char;

  for (i = character_list; i; i = next_char) {
    next_char = i->next;

    if (GET_POS(i) >= POS_STUNNED && GET_LEVEL(i) < newbie_level) {
      GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
      if(GET_CLASS(i)==CLASS_MONK || IS_REMORT(i))
        GET_QI(i) = MIN(GET_QI(i) + mana_gain(i), GET_MAX_QI(i));
      if(GET_CLASS(i)==CLASS_DRUID || IS_REMORT(i))
        GET_VIM(i) = MIN(GET_VIM(i) + mana_gain(i), GET_MAX_VIM(i));
      if(GET_CLASS(i)==CLASS_BARD)
        GET_ARIA(i) = MIN(GET_ARIA(i) + mana_gain(i), GET_MAX_ARIA(i));
      if(GET_CLASS(i)==CLASS_CLERIC || GET_CLASS(i)==CLASS_PALADIN ||
        GET_CLASS(i)==CLASS_SORCERER || GET_CLASS(i)==CLASS_DARK_KNIGHT ||
        IS_REMORT(i))
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
    }
  }
}

/* Update PCs, NPCs, and objects */
void point_update(void)
{
  void update_char_objects(struct char_data * ch);	/* handler.c */
  void extract_obj(struct obj_data * obj);	/* handler.c */
  struct char_data *i, *next_char;
  struct obj_data *j, *next_thing, *jj, *next_thing2;

  /* characters */
  for (i = character_list; i; i = next_char) {
    next_char = i->next;
	
    gain_condition(i, FULL, -1);
    gain_condition(i, DRUNK, -1);
    gain_condition(i, THIRST, -1);
	
    if (GET_POS(i) >= POS_STUNNED) {
      GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), GET_MAX_HIT(i));
      if(GET_CLASS(i)==CLASS_MONK || IS_REMORT(i))
        GET_QI(i) = MIN(GET_QI(i) + mana_gain(i), GET_MAX_QI(i));
      if(GET_CLASS(i)==CLASS_DRUID || IS_REMORT(i))
        GET_VIM(i) = MIN(GET_VIM(i) + mana_gain(i), GET_MAX_VIM(i));
      if(GET_CLASS(i)==CLASS_BARD)
        GET_ARIA(i) = MIN(GET_ARIA(i) + mana_gain(i), GET_MAX_ARIA(i));
      if(GET_CLASS(i)==CLASS_CLERIC || GET_CLASS(i)==CLASS_PALADIN ||
        GET_CLASS(i)==CLASS_SORCERER || GET_CLASS(i)==CLASS_DARK_KNIGHT ||
        IS_REMORT(i))
        GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), GET_MAX_MANA(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), GET_MAX_MOVE(i));
      if (IS_AFFECTED(i, AFF_POISON))
	damage(i, i, GET_MAX_HIT(i) / 4, SPELL_POISON, ABT_SPELL);
      if (GET_POS(i) <= POS_STUNNED)
	update_pos(i);
     } else if (GET_POS(i) == POS_INCAP) 
        damage(i, i, 1, TYPE_SUFFERING, ABT_WEAPON);
   
    

/*
    ******************* VEDIC ********************
     else if (GET_POS(i) == POS_MORTALLYW)
      damage(i, i, 2, TYPE_SUFFERING, ABT_WEAPON); */
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) < LVL_ADMIN)
	check_idling(i);
    }
  }

  if(i) {
   if(!IS_NPC(i)) {
     GET_WEIGHT(i) = 1;
     IS_CARRYING_W(i) = 1;
   }

  }

  /* objects */
  for (j = object_list; j; j = next_thing) {
    next_thing = j->next;       /* Next in object list */
 
 
    /* If this is a corpse */
    if ((GET_OBJ_TYPE(j) == ITEM_CONTAINER) && GET_OBJ_VAL(j, 3))     {
      /* timer count down */
      if (GET_OBJ_TIMER(j) > 0)
        GET_OBJ_TIMER(j)--;

      if (!GET_OBJ_TIMER(j)) {
 
        if (j->carried_by)
          act("$p decays in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
        else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
          act("A quivering horde of maggots consumes $p.",
              TRUE, world[j->in_room].people, j, 0, TO_ROOM);
          act("A quivering horde of maggots consumes $p.",
              TRUE, world[j->in_room].people, j, 0, TO_CHAR);
        }

        for (jj = j->contains; jj; jj = next_thing2) {
          next_thing2 = jj->next_content;       /* Next in inventory */
          obj_from_obj(jj);
 
          if (j->in_obj) {
            obj_to_obj(jj, j->in_obj);
          }
          else if (j->carried_by) {
            obj_to_room(jj, j->carried_by->in_room);
          }
          else if (j->in_room != NOWHERE) {
            obj_to_room(jj, j->in_room);
          }
          else {
            sprintf(buf, "assert bug: %s %s", jj->short_description, world[j->in_room].name);
	    log(buf);
            mudlog(buf, BRF, LVL_ADMIN, TRUE);
          
            assert(FALSE);
          }
        }
        extract_obj(j);
      }
    }

    /* On any item that's not a corpse
     * ignore the timer if it's set to 0,
     * set items with a timer value of 1
     * to -1 so they get extracted and
     * decrement the timer on any object > 1.
     */
    if (GET_OBJ_TIMER(j) != 0) {
      save_corpses();
      /* timer count down */
      if (GET_OBJ_TIMER(j) == 1)
        GET_OBJ_TIMER(j) = -1;
      if ((GET_OBJ_TIMER(j) > 0) && !j->carried_by && !j->worn_by)
        GET_OBJ_TIMER(j)--;

    if (GET_OBJ_TIMER(j) == -1) {
 
     if (j->carried_by) {
       switch (GET_OBJ_TYPE(j)) {
        case ITEM_FOUNTAIN:
          act("$p dries up in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
          break;
        case ITEM_PORTAL:
          act("$p fades away from existance in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
          break;
        default:
          act("$p crumbles to dust in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
          break;
      }
    } else if (j->worn_by) {
        switch (GET_OBJ_TYPE(j)) {
         case ITEM_FOUNTAIN:
           act("$p dries up on your body.", FALSE, j->worn_by, j, 0, TO_CHAR);
           break;
         case ITEM_PORTAL:
           act("$p fades away from existance on your body.", FALSE, j->worn_by, j, 0, TO_CHAR);
           break;
         default:
           act("$p crumbles to dust and drops from your body.", FALSE, j->worn_by, j, 0, TO_CHAR);
           break;
        }
    } else if ((j->in_room != NOWHERE) && (world[j->in_room].people)) {
        switch (GET_OBJ_TYPE(j)) {
         case ITEM_FOUNTAIN:
           act("$p dries up.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
           act("$p dries up.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
           break;
         case ITEM_PORTAL:
           act("$p fades away from existance.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
           act("$p fades away from existance.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
           break;
         default:
           act("$p crumbles into a small pile of dust.", TRUE, world[j->in_room].people, j, 0, TO_ROOM);
           act("$p crumbles into a small pile of dust.", TRUE, world[j->in_room].people, j, 0, TO_CHAR);
           break;
        }
    }
    for (jj = j->contains; jj; jj = next_thing2) {
      next_thing2 = jj->next_content;       /* Next in inventory */
      obj_from_obj(jj);
 
      if (j->in_obj)
        obj_to_obj(jj, j->in_obj);
      else if (j->carried_by)
        obj_to_char(jj, j->carried_by);
      else if (j->worn_by)
        obj_to_char(jj, j->worn_by);
      else if (j->in_room != NOWHERE)
        obj_to_room(jj, j->in_room);
      else
        assert(FALSE);
     }
 
        save_corpses();
        extract_obj(j);
      }
    }
 
  }

}
    /* If the timer is set, count it down and at 0, try the trigger */
    /* note to .rej hand-patchers: make this last in your point-update() 
    else if (GET_OBJ_TIMER(j)>0) {
      GET_OBJ_TIMER(j)--;
      save_corpses();

      if (!GET_OBJ_TIMER(j))
        timer_otrigger(j);
     */
