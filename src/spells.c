/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "hometowns.h"
#include "dg_scripts.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct cha_app_type cha_app[];
extern struct int_app_type int_app[];
extern struct index_data *obj_index;
extern const int rev_dir[];
extern char *dirs[];
extern char *wear_bits[];
extern struct char_data *character_list;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;

extern struct zone_data *zone_table;
extern int top_of_zone_table;

extern int mini_mud;
extern int pk_allowed;

extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;
extern char *hometowns[];
extern const int weapon_dam_dice[][2];
extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];

extern struct attack_hit_type attack_hit_text[];

void clearMemory(struct char_data * ch);
void old_act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);
void act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, const void *vict_obj, int type);

void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int mag_savingthrow(struct char_data * ch, int type);
void clearMemory(struct char_data * ch);
int check_get_corpse(struct char_data *ch, struct obj_data *obj);


/*
 * Special spells appear below.
 */

ASPELL(spell_create_water)
{
  int water;

  void name_to_drinkcon(struct obj_data * obj, int type);
  void name_from_drinkcon(struct obj_data * obj);

  if (ch == NULL || obj == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if ((GET_OBJ_VAL(obj, 2) != LIQ_WATER) && (GET_OBJ_VAL(obj, 1) != 0)) {
      name_from_drinkcon(obj);
      GET_OBJ_VAL(obj, 2) = LIQ_SLIME;
      name_to_drinkcon(obj, LIQ_SLIME);
    } else {
      water = MAX(GET_OBJ_VAL(obj, 0) - GET_OBJ_VAL(obj, 1), 0);
      if (water > 0) {
	if (GET_OBJ_VAL(obj, 1) >= 0)
	  name_from_drinkcon(obj);
	GET_OBJ_VAL(obj, 2) = LIQ_WATER;
	GET_OBJ_VAL(obj, 1) += water;
	name_to_drinkcon(obj, LIQ_WATER);
	weight_change_object(obj, water);
	act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
      }
    }
  }
}


ASPELL(spell_recall)
{

  int load_room=0;
  
  if (victim == NULL || IS_NPC(victim))
    return;

  if (victim != ch) {
    send_to_char("You can only recall yourself, sorry.\r\n", ch);
    return;
  }
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) ||
      PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD)) {
    send_to_char("Sorry, finish your fight first.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_ARENA)) {
    send_to_char("Nothing happens.\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char("You can't recall while fishing. Reelin first.\r\n", ch);
    return;
  }
  if (FIGHTING(ch) || FIGHTING(victim)) {
    send_to_char("No recall in a fight, sorry.\r\n", ch);
    return;
  }


  if (IS_AFFECTED(victim, AFF_BERSERK)) {
    send_to_char( "Berserker rage obscures your understanding of magic!\r\n", ch );
    return;
  }

  if(GET_MOUNT(ch)) {
   GET_RIDER(GET_MOUNT(ch)) = NULL;
   GET_MOUNT(ch) = NULL;
  }

  if ((load_room = GET_LOADROOM(ch)) != NOWHERE)
    load_room = real_room(load_room);
  else
    load_room = real_room(get_default_room(ch));
  act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, load_room); 
  act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);

}

/* moved to mag_points, where it belongs 10.7.01 - Rapideye
ASPELL(spell_refresh)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 
  send_to_char("You feel the spring return to your step.\r\n", victim);
  GET_MOVE(victim) += 25;
  GET_MOVE(victim) = MIN(GET_MOVE(victim), GET_MAX_MOVE(victim));
}
*/
ASPELL(spell_vim_restore)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 
  send_to_char("You feel your vim return.\r\n", victim);
  GET_VIM(victim) += 25;
  GET_VIM(victim) = MIN(GET_VIM(victim), GET_MAX_VIM(victim));
}

ASPELL(spell_qi_restore)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 
  send_to_char("You feel your qi become more balanced.\r\n", victim);
  GET_QI(victim) += 25;
  GET_QI(victim) = MIN(GET_QI(victim), GET_MAX_QI(victim));
}

ASPELL(spell_aria_restore)
{
  send_to_char("Your aria strengthens.\r\n", victim);
  GET_ARIA(victim) = MIN(GET_ARIA(victim)+25, GET_MAX_ARIA(victim));
}

ASPELL(spell_teleport)
{
  int to_room;
  extern int top_of_world;

  if (victim != NULL)
    return;

  do {
    to_room = number(0, top_of_world);
  } while (ROOM_FLAGGED(to_room, ROOM_PRIVATE) ||
           ROOM_FLAGGED(to_room, ROOM_DEATH));
  act("$n slowly fades out of existence and is gone.",
      FALSE, victim, 0, 0, TO_ROOM);
  char_from_room(victim);
  char_to_room(victim, to_room);
  act("$n slowly fades into existence.", FALSE, victim, 0, 0, TO_ROOM);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_little_sister)
{
  int to_room;

  to_room = 24001;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_dearthwood)
{
  int to_room;

  to_room = 32065;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_grasslands)
{
  int to_room;

  to_room = 6126;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_tanglewood)
{
  int to_room;

  to_room = 3433;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_valley)
{
  int to_room;

  to_room = 10904;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_darkwood)
{
  int to_room;

  to_room = 13172;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_sinister)
{
  int to_room;

  to_room = 1411;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_hermit)
{
  int to_room;

  to_room = 11844;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_kailaani)
{
  int to_room;

  to_room = 16346;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}


ASPELL(spell_treewalk_thewster)
{
  int to_room;

  to_room = 18905;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_treewalk_crystalmir)
{
  int to_room;

  to_room = 7630;

  send_to_char("You harness the power of the trees, taking yourself "
               "to a new location.\r\n", ch);
  act("$n harnesses the power of the trees, taking $mself far away.",
      FALSE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(to_room));
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

#define SUMMON_FAIL "You failed.\r\n"

ASPELL(spell_summon)
{
  int i;

  if (ch == NULL || victim == NULL)
    return;

  if ((GET_LEVEL(ch) < LVL_IMMORT) && (GET_LEVEL(victim) >= LVL_IMMORT)) {
    send_to_char("You are not mighty enough to summon a god.\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) + 3 < GET_LEVEL(victim) && 
      (GET_CLASS(ch) <= (CLASS_VAMPIRE || CLASS_TITAN || CLASS_SAINT || 
       CLASS_DEMON))) {
    send_to_char("Their level is too far above yours. You fail.\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) ||
      PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD)) {
    send_to_char("Sorry, finish your fight first.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 

  if (FIGHTING(victim)) {
    send_to_char("They are in the middle of a fight, you'll have to wait.\r\n", ch);
    return;
  }
  if (PLR_FLAGGED(victim, PLR_FISHING)) {
    send_to_char("Sorry, they're fishing right now.\r\n", ch);
    return;
  }
  if (!PRF_FLAGGED(ch, PRF_SUMMONABLE)) {
   send_to_char("You can't summon when you have nosummon on yourself.\r\n", ch);
   send_to_char("Type NOSUMMON to use the spell.\r\n", ch);
   return;
  }

  for (i = 0; i <= top_of_zone_table; i++){
    if ((zone_table[i].number*100 <= world[IN_ROOM(ch)].number &&
         (zone_table[i].number+1)*100 > world[IN_ROOM(ch)].number &&
         zone_table[i].development)) {
      send_to_char("This entire area of the world is under a camoflaging ward.\r\n", ch);
      send_to_char("You cannot summon anyone here.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed) {
    if (MOB_FLAGGED(victim, MOB_AGGRESSIVE)) {
      act("As the words escape your lips and $N travels\r\n"
	  "through time and space towards you, you realize that $E is\r\n"
	  "aggressive and might harm you, so you wisely send $M back.",
	  FALSE, ch, 0, victim, TO_CHAR);
      return;
    }
    if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE) &&
	!PLR_FLAGGED(victim, PLR_KILLER)) {
      sprintf(buf, "%s just tried to summon you to: %s.\r\n"
	      "%s failed because you have summon protection on.\r\n"
	      "Type NOSUMMON to allow other players to summon you.\r\n",
	      GET_NAME(ch), world[ch->in_room].name,
	      (ch->player.sex == SEX_MALE) ? "He" : "She");
      send_to_char(buf, victim);

      sprintf(buf, "You failed because %s has summon protection on.\r\n",
	      GET_NAME(victim));
      send_to_char(buf, ch);

      sprintf(buf, "%s failed summoning %s to %s.",
	      GET_NAME(ch), GET_NAME(victim), world[ch->in_room].name);
      mudlog(buf, BRF, LVL_IMMORT, TRUE);
      return;
    }
  }

  if (MOB_FLAGGED(victim, MOB_NOSUMMON) || 
      ROOM_FLAGGED(ch->in_room, ROOM_ARENA) ||
      (IS_NPC(victim) && mag_savingthrow(victim, SAVING_SPELL))) {
    send_to_char(SUMMON_FAIL, ch);
    return;
  }

  act("$n disappears suddenly.", TRUE, victim, 0, 0, TO_ROOM);

  char_from_room(victim);
  char_to_room(victim, ch->in_room);

  act("$n arrives suddenly.", TRUE, victim, 0, 0, TO_ROOM);
  act("$n has summoned you!", FALSE, ch, 0, victim, TO_VICT);
  look_at_room(victim, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}



ASPELL(spell_locate_object)
{
  struct obj_data *i;
  char name[MAX_INPUT_LENGTH];
  int j;

  strcpy(name, fname(obj->name));
  j = level >> 1;

  for (i = object_list; i && (j > 0); i = i->next) {
    if (!isname(name, i->name))
      continue;
 
    if (IS_OBJ_STAT(i, ITEM_NOLOCATE)) {
      send_to_char("That item is protected by magic.\r\n", ch);
      continue;
    }

    if (i->carried_by)
      sprintf(buf, "%s is being carried by %s.\n\r",
	      i->short_description, PERS(i->carried_by, ch));
    else if (i->in_room != NOWHERE)
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      world[i->in_room].name);
    else if (i->in_obj)
      sprintf(buf, "%s is in %s.\n\r", i->short_description,
	      i->in_obj->short_description);
    else if (i->worn_by)
      sprintf(buf, "%s is being worn by %s.\n\r",
	      i->short_description, PERS(i->worn_by, ch));
    else
      sprintf(buf, "%s's location is uncertain.\n\r",
	      i->short_description);

    CAP(buf);
    send_to_char(buf, ch);
    j--;
  }

  if (j == level >> 1)
    send_to_char("You sense nothing.\n\r", ch);
}



ASPELL(spell_charm)
{
  struct affected_type af;

  if (victim == NULL || ch == NULL)
    return;

  if (victim == ch)
    send_to_char("You like yourself even better!\r\n", ch);
  else if (!IS_NPC(victim) && !PRF_FLAGGED(victim, PRF_SUMMONABLE))
    send_to_char("You fail because SUMMON protection is on!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_SANCTUARY))
    send_to_char("Your victim is protected by sanctuary!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_INDESTR_AURA))
    send_to_char("Your victim has an indestructable aura about them!\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_NOCHARM))
    send_to_char("Your victim resists!\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_CHARM))
    send_to_char("You can't have any followers of your own!\r\n", ch);
  else if (IS_AFFECTED(victim, AFF_CHARM) || level < GET_LEVEL(victim))
    send_to_char("You fail.\r\n", ch);
  /* player charming another player - no legal reason for this */
  else if (!pk_allowed && !IS_NPC(victim))
    send_to_char("You fail - shouldn't be doing it anyway.\r\n", ch);
  else if (circle_follow(victim, ch))
    send_to_char("Sorry, following in circles can not be allowed.\r\n", ch);
  else if (mag_savingthrow(victim, SAVING_PARA))
    send_to_char("Your victim resists!\r\n", ch);
  else {
    if (victim->master)
      stop_follower(victim);

    add_follower(victim, ch);

    af.type = SPELL_CHARM;

    if (GET_INT(victim))
      af.duration = 24 * 18 / GET_INT(victim);
    else
      af.duration = 24 * 18;

    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    if (IS_NPC(victim)) {
      GET_GOLD(victim) = 0;
      REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_AGGRESSIVE);
      REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_SPEC);
    }
  }
}



ASPELL(spell_identify)
{
  int i;
  int found;

  struct time_info_data age(struct char_data * ch);

  extern char *spells[];

  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  if (obj) {
    send_to_char("You feel informed:\r\n", ch);
    sprintf(buf, "Object '%s', Item type: ", obj->short_description);
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    send_to_char("WEAR POS: ", ch);
    sprintbitarray(GET_OBJ_WEAR(obj), wear_bits, TW_ARRAY_MAX, buf1);
    strcat(buf1, "\r\n");
    send_to_char(buf1, ch);

    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbitarray(obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
    }
    send_to_char("Item is: ", ch);
    sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf);
    sprintf(buf, "%s\r\nMinimum level: %d\r\n", buf, obj->obj_flags.minlevel);
    send_to_char(buf, ch);

    sprintf(buf, "Weight: %d, Value: %d, Rent: %d\r\n",
	    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
    send_to_char(buf, ch);

    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);

      if (GET_OBJ_VAL(obj, 1) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 1)]);
      if (GET_OBJ_VAL(obj, 2) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 2)]);
      if (GET_OBJ_VAL(obj, 3) >= 1)
	sprintf(buf, "%s %s", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%s\r\n", buf);
      send_to_char(buf, ch);
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      sprintf(buf, "This %s casts: ", item_types[(int) GET_OBJ_TYPE(obj)]);
      sprintf(buf, "%s %s\r\n", buf, spells[GET_OBJ_VAL(obj, 3)]);
      sprintf(buf, "%sIt has %d maximum charge%s and %d remaining.\r\n", buf,
	      GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 1) == 1 ? "" : "s",
	      GET_OBJ_VAL(obj, 2));
      send_to_char(buf, ch);
      break;
/*
    case ITEM_NEEDLE:
      sprintf(buf, "%s Causes an average per-round damage of %.1f.\r\n", buf,
                ((weapon_dam_dice[obj->obj_flags.minlevel][0] / 2.0) *
                 weapon_dam_dice[obj->obj_flags.minlevel][1]) / 2);
      send_to_char(buf, ch);
      break;
*/
    case ITEM_WEAPON:
      sprintf(buf, "Damage Dice is '%dD%d'", 
      weapon_dam_dice[obj->obj_flags.minlevel][0],
      weapon_dam_dice[obj->obj_flags.minlevel][1]);
      sprintf(buf, "%s for an average per-round damage of %.1f.\r\n", buf,
	        (weapon_dam_dice[obj->obj_flags.minlevel][0] / 2.0) * 
                 weapon_dam_dice[obj->obj_flags.minlevel][1]);
      if (GET_OBJ_SPELL(obj)) {
        sprintf(buf, "%sCasts %s %d%% of the time.\r\n", buf,
                     spells[GET_OBJ_SPELL(obj)], GET_OBJ_SPELL_EXTRA(obj));
      }
      send_to_char(buf, ch);
      sprintf(buf, "Damage type is %s.\r\n", 
                    attack_hit_text[GET_OBJ_VAL(obj, 3)].singular);
      send_to_char(buf, ch);
      break;
    case ITEM_ARMOR:
      sprintf(buf, "AC-apply is %d\r\n", GET_OBJ_VAL(obj, 0));
      send_to_char(buf, ch);
      break;
    case ITEM_LIGHT:
      if (GET_OBJ_VAL(obj, 2))
        sprintf(buf, "Will provide light for about %d more hours.\r\n",
                GET_OBJ_VAL(obj, 2));
      else
        sprintf(buf, "It's burned out.\r\n");
      send_to_char(buf, ch);
      break;
    }
    found = FALSE;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
      if ((obj->affected[i].location != APPLY_NONE) &&
	  (obj->affected[i].modifier != 0)) {
	if (!found) {
	  send_to_char("Can affect you as :\r\n", ch);
	  found = TRUE;
	}
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "   Affects: %s By %d\r\n", buf2, obj->affected[i].modifier);
	send_to_char(buf, ch);
      }
    }
  } else if (victim) {		/* victim */
    sprintf(buf, "Name: %s\r\n", GET_NAME(victim));
    send_to_char(buf, ch);
    if (!IS_NPC(victim)) {
      sprintf(buf, "%s is %d years, %d months, %d days and %d hours old.\r\n",
	      GET_NAME(victim), age(victim).year, age(victim).month,
	      age(victim).day, age(victim).hours);
      send_to_char(buf, ch);
    }
    sprintf(buf, "Height %d in, Weight %d pounds\r\n",
	    GET_HEIGHT(victim), GET_WEIGHT(victim));
    sprintf(buf, "%sLevel: %d, Hits: %d\r\n", buf,
	    GET_LEVEL(victim), GET_HIT(victim));
    sprintf(buf, "%sAC: %d, Hitroll: %d, Damroll: %d, Spellpower: %d\r\n", buf,
	    GET_AC(victim), GET_HITROLL(victim), GET_DAMROLL(victim),
            GET_SPELLPOWER(victim));
    sprintf(buf, "%sStr: %d/%d, Int: %d, Wis: %d, Dex: %d, Con: %d, Cha: %d, "
                 "Luck: %d\r\nFrom: %s\r\n",
	    buf, GET_STR(victim), GET_ADD(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim), GET_CHA(victim),
        GET_LUCK(victim), hometowns[GET_HOME(victim)]);
    send_to_char(buf, ch);

  }
}

ASPELL(spell_restore_mana)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 
  GET_MANA(ch) += number(14, 22);
  GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
  send_to_char("A small rush of magical energy flows through you.\r\n", ch); 
}

ASPELL(spell_restore_megamana)
{
  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 
  GET_MANA(ch) += number(30, 50);
  GET_MANA(ch) = MIN(GET_MANA(ch), GET_MAX_MANA(ch));
  send_to_char("A large rush of magical energy flows through you.\r\n", ch); 
}

ASPELL(spell_enchant_weapon)
{
  int i;

  if ((GET_OBJ_TYPE(obj) == ITEM_WEAPON) &&
      !IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC)) {

    for (i = 0; i < MAX_OBJ_AFFECT; i++)
      if (obj->affected[i].location != APPLY_NONE)
	return;

    SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_MAGIC);

    obj->affected[0].location = APPLY_HITROLL;
    obj->affected[0].modifier = 1 + number(1, (obj->obj_flags.minlevel / 30));

    obj->affected[1].location = APPLY_DAMROLL;
    obj->affected[1].modifier = 1 + number(1, (obj->obj_flags.minlevel / 30));

    if (IS_GOOD(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_EVIL);
      act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
    } else if (IS_EVIL(ch)) {
      SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_ANTI_GOOD);
      act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
      act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
    }
  }
}


ASPELL(spell_detect_poison)
{
  if (victim) {
    if (victim == ch) {
      if (IS_AFFECTED(victim, AFF_POISON))
        send_to_char("You can sense poison in your blood.\r\n", ch);
      else
        send_to_char("You feel healthy.\r\n", ch);
    } else {
      if (IS_AFFECTED(victim, AFF_POISON))
        act("You sense that $E is poisoned.", FALSE, ch, 0, victim, TO_CHAR);
      else
        act("You sense that $E is healthy.", FALSE, ch, 0, victim, TO_CHAR);
    }
  }

  if (obj) {
    switch (GET_OBJ_TYPE(obj)) {
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
    case ITEM_FOOD:
      if (GET_OBJ_VAL(obj, 3))
	act("You sense that $p has been contaminated.",FALSE,ch,obj,0,TO_CHAR);
      else
	act("You sense that $p is safe for consumption.", FALSE, ch, obj, 0,
	    TO_CHAR);
      break;
    default:
      send_to_char("You sense that it should not be consumed.\r\n", ch);
    }
  }
}
#define PORTAL 20

ASPELL(spell_dimension_door)
{
  /* create a dimensional door */
  struct obj_data *tmp_obj, *tmp_obj2;
  struct extra_descr_data *ed;
  struct room_data *rp, *nrp;
  struct char_data *tmp_ch = (struct char_data *) victim;
  char buf[512];
  int i;

  assert(ch);
  assert((level >= 0) && (level <= LVL_IMPL));

  if (tmp_ch == ch) {
    send_to_char("You can't do that.  the fabric of existence is "
                 "much too delicate.\r\n", ch);
    return;
  }

  if (PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char("You'll need to pack up your gear. Reelin first.\r\n", ch);
    return;
  }
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) ||
      PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD)) {
    send_to_char("Sorry, finish your fight first.\r\n", ch);
    return;
  }
  
  if (AFF_FLAGGED(tmp_ch, AFF_NOTRACK)) {
    send_to_char("You cannot locate them.\r\n", ch);
    return;
  }
 
  if (AFF_FLAGGED(tmp_ch, AFF_NOMAGIC)) {
    send_to_char("Powerful magics protect them. Your magic is not "
                 "strong enough to find them.\r\n", ch);
    return;
  }

  if (IS_NPC(tmp_ch))
  {
	  if (MOB_FLAGGED(tmp_ch, MOB_NOSUMMON))
	  {
		  send_to_char("You can't create a dimensional door to that location.\r\n", ch);
		  return;
	  }

  }
  if (!IS_NPC(tmp_ch))
  {
	  if(!PLR_FLAGGED(tmp_ch, PRF_SUMMONABLE))
	  {
		  send_to_char("You can't create a dimensional door to that location.\r\n", ch);
		  return;
	  }
  }

  if (FIGHTING(tmp_ch)) {
    send_to_char("They are fighting right now, you'll have to wait.\r\n", ch);
    return;
  }
  /*
    check target room for legality.
   */
  rp = &world[ch->in_room];
  tmp_obj = read_object(PORTAL, VIRTUAL);
  if (!rp || !tmp_obj) {
    send_to_char("The magic fails.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }
  if (IS_SET_AR(rp->room_flags, ROOM_TUNNEL) || 
      IS_SET_AR(rp->room_flags, ROOM_SINGLE_SPACE)) {
    send_to_char("There is no room in here to summon!\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }

  if (IS_SET_AR(rp->room_flags, ROOM_GODROOM)) {
    send_to_char("This room is protected by ancient and powerful magics.\r\n", ch);
    extract_obj(tmp_obj);
    return;
  }

    if (IS_SET_AR(rp->room_flags, ROOM_NOPORTAL)) {
    send_to_char("This room is protected by ancient and powerful magics.\r\n", ch);
    extract_obj(tmp_obj);
    return;
  } 

  for (i = 0; i <= top_of_zone_table; i++){
    if ((zone_table[i].number*100 <= world[IN_ROOM(tmp_ch)].number &&
         (zone_table[i].number+1)*100 > world[IN_ROOM(tmp_ch)].number &&
         zone_table[i].development)) {
      send_to_char("That entire area of the world is under a camoflaging ward.\r\n", ch);
      send_to_char("You cannot create a door to that place.\r\n", ch);
      extract_obj(tmp_obj);
      return;
    }
  }

  if (!(nrp = &world[tmp_ch->in_room])) {
    char str[180];
    sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
    log(str);
    send_to_char("The magic cannot locate the target\n", ch);
    extract_obj(tmp_obj);
    return;
  }

  if (ROOM_FLAGGED(tmp_ch->in_room, ROOM_NOMAGIC)) {
    send_to_char("Your target is protected against your magic.\n\r", ch);
    extract_obj(tmp_obj);
    return;
  }

  sprintf(buf, "Through the mists of the portal, you can faintly see %s",nrp->name);

  CREATE(ed , struct extra_descr_data, 1);
  ed->next = tmp_obj->ex_description;
  tmp_obj->ex_description = ed;
  CREATE(ed->keyword, char, strlen(tmp_obj->name) + 1);
  strcpy(ed->keyword, tmp_obj->name);
  ed->description = str_dup(buf);

  tmp_obj->obj_flags.value[0] = 1;
  tmp_obj->obj_flags.value[1] = tmp_ch->in_room;
  obj_to_room(tmp_obj,ch->in_room);

  act("A $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
  act("A $p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);

/* Portal at other side */
   rp = &world[ch->in_room];
   tmp_obj2 = read_object(PORTAL, VIRTUAL);
   if (!rp || !tmp_obj2) {
     send_to_char("The magic fails.\n\r", ch);
     extract_obj(tmp_obj2);
     return;
   }
   if (ROOM_FLAGGED(tmp_ch->in_room, ROOM_GODROOM) || ROOM_FLAGGED(tmp_ch->in_room, ROOM_NOPORTAL)) {
     send_to_char("A power greater than you could ever be protects that room. \r\n", ch);
     extract_obj(tmp_obj);
     extract_obj(tmp_obj2);
     return;
   }
  sprintf(buf,"Through the mists of the portal, you can faintly see %s", rp->name);

  CREATE(ed , struct extra_descr_data, 1);
  ed->next = tmp_obj2->ex_description;
  tmp_obj2->ex_description = ed;
  CREATE(ed->keyword, char, strlen(tmp_obj2->name) + 1);
  strcpy(ed->keyword, tmp_obj2->name);
  ed->description = str_dup(buf);
  tmp_obj2->obj_flags.value[0] = 1;
  tmp_obj2->obj_flags.value[1] = ch->in_room;
  obj_to_room(tmp_obj2,tmp_ch->in_room);
  act("A $p suddenly appears.", TRUE, tmp_ch, tmp_obj2, 0, TO_ROOM);
  act("A $p suddenly appears.", TRUE, tmp_ch, tmp_obj2, 0, TO_CHAR);

  act("$n jumps through the dimensional door and is gone.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  act("The $p disappears as $n goes through it.",TRUE,ch,tmp_obj,0,TO_ROOM);
  extract_obj(tmp_obj);

  char_to_room(ch, tmp_ch->in_room);
  act("The $p disappears as $n jumps out of it.",TRUE,ch,tmp_obj,0,TO_ROOM);
  extract_obj(tmp_obj2);
  send_to_char("  The world spins and you look around to find that you have reached your target.\r\n\r\n\r\n\r\n", ch);
  look_at_room(ch, 0);
  entry_memory_mtrigger(ch);
  greet_mtrigger(ch, -1);
  greet_memory_mtrigger(ch);
}

ASPELL(spell_portal)
{
  struct obj_data *portal, *tportal;
  struct extra_descr_data *new_descr, *new_tdescr;
  char buf[80];
  int i;

  if (ch == NULL || victim == NULL)
    return;

  for (i = 0; i <= top_of_zone_table; i++){
    if (ROOM_FLAGGED(IN_ROOM(victim), ROOM_NOPORTAL) ||
        ROOM_FLAGGED(IN_ROOM(victim), ROOM_ARENA) ||
        (zone_table[i].number*100 <= world[IN_ROOM(victim)].number &&
         (zone_table[i].number+1)*100 > world[IN_ROOM(victim)].number &&
         zone_table[i].development)) {
      send_to_char("You can't create a portal to that location.\r\n", ch);
      return;
    }
  }
  if (PLR_FLAGGED(ch, PLR_FISHING)) {
    send_to_char("You'll have to pack up your gear first. Reelin.\r\n", ch);
    return;
  }

  if (IS_NPC(victim))
  {
	  if (MOB_FLAGGED(victim, MOB_NOSUMMON))
	  {
		  send_to_char("You can't create a portal to that location.\r\n", ch);
		  return;
	  }

  }
  if (!IS_NPC(victim))
  {
	  if(!PLR_FLAGGED(victim, PRF_SUMMONABLE))
	  {
		  send_to_char("You can't create a portal to that location.\r\n", ch);
		  return;
	  }
  }

  /* create the portal */
  portal = read_object(PORTAL, VIRTUAL);
  GET_OBJ_VAL(portal, 0) = world[victim->in_room].number;
  GET_OBJ_TIMER(portal) = (int) (GET_LEVEL(ch) / 10);
  CREATE(new_descr, struct extra_descr_data, 1);
  new_descr->keyword = str_dup("portal gate gateway");
  sprintf(buf, "You can barely make out %s.", world[victim->in_room].name);
  new_descr->description = str_dup(buf);
  new_descr->next = portal->ex_description;
  portal->ex_description = new_descr;
  obj_to_room(portal, ch->in_room);
  act("$n conjures a portal out of thin air.",
       TRUE, ch, 0, 0, TO_ROOM);
  act("You conjure a portal out of thin air.",
       TRUE, ch, 0, 0, TO_CHAR);

  /* create the return portal only if the player is above level 45 */
  if (GET_LEVEL(ch) >= 45) {
    for (i = 0; i <= top_of_zone_table; i++){
      if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOPORTAL) ||
          (zone_table[i].number*100 <= world[IN_ROOM(ch)].number &&
           (zone_table[i].number+1)*100 > world[IN_ROOM(ch)].number &&
           zone_table[i].development)) {
        return;
      }
    }
    tportal = read_object(PORTAL, VIRTUAL);
    GET_OBJ_VAL(tportal, 0) = world[ch->in_room].number;
    GET_OBJ_VAL(tportal, 1) = world[ch->in_room].number;

    GET_OBJ_TIMER(tportal) = (int) (GET_LEVEL(ch) / 10);
    CREATE(new_tdescr, struct extra_descr_data, 1);
    new_tdescr->keyword = str_dup("portal gate gateway");
    sprintf(buf, "You can barely make out %s.", world[ch->in_room].name);
    new_tdescr->description = str_dup(buf);
    new_tdescr->next = tportal->ex_description;
    tportal->ex_description = new_tdescr;
    obj_to_room(tportal, victim->in_room);
    act("A glowing portal appears out of thin air.",
         TRUE, victim, 0, 0, TO_ROOM);
    act("A glowing portal opens here for you.",
         TRUE, victim, 0, 0, TO_CHAR);
  }
}

ASPELL(spell_createspring)
{
  struct obj_data *spring = '\0';
  int spring_num = 10051; 
  *buf = '\0';

  if (GET_SKILL(ch, SPELL_CREATESPRING) == 0) {
    send_to_char("That spell is unfamiliar to you.\r\n", ch);
    return ;
  }

  if((world[IN_ROOM(ch)].sector_type == SECT_INSIDE) ||
     (world[ch->in_room].sector_type == SECT_CITY)) {
    send_to_char("You cannot create a spring here!\r\n", ch);
    return ;
  }

  if((world[ch->in_room].sector_type == SECT_WATER_SWIM) || 
     (world[ch->in_room].sector_type == SECT_OCEAN) ||
     (world[ch->in_room].sector_type == SECT_WATER_NOSWIM)) {
    send_to_char("How can you create a spring in water?\r\n", ch);
    return;
  }

  if (world[ch->in_room].sector_type == SECT_UNDERWATER) {
    send_to_char("You cannot create a spring underwater!\r\n", ch);
    return;
  }
 
  if (ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
    send_to_char("An unforseen force prevents you from casting spells.\r\n",ch);
    return;
  }

  spring = read_object(spring_num, VIRTUAL);
  GET_OBJ_TIMER(spring) = 2 + (GET_LEVEL(ch) >> 1);
  obj_to_room(spring, obj->in_room);
  act("You create $p!", FALSE, ch, spring, 0, TO_CHAR);
  act("$n creates $p!", FALSE, ch, spring, 0, TO_ROOM);
  send_to_char(buf, ch);
  return;
}

/* Yes I ripped this off of call -- Vedic */
/* ;-) 
#define CALL_MOVE_COST 50
#define CALL_HORSE 30
#define CALL_UNICORN 31
#define CALL_PEGASUS 32
#define CALL_DRAGON 33

ASPELL(spell_undead_mount) {

  struct char_data *pet;
  struct follow_type *f;
  int chance;
  int penalty=0;


  if (GET_SKILL(ch, SPELL_UNDEAD_MOUNT) == 0)
    {
    send_to_char("That spell is unfamiliar to you.\r\n", ch);
    return ;
    }

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

  chance = number(0, GET_SKILL(ch, SPELL_UNDEAD_MOUNT) + GET_LEVEL(ch));

  if (chance < 50) {
    act("Nothing Happens.", FALSE, ch, 0, 0, TO_CHAR);
    return;

  } else if (chance  < 70)
    pet = read_mobile(CALL_HORSE, VIRTUAL);
  else if (chance  < 90)
    pet = read_mobile(CALL_UNICORN, VIRTUAL);
  else if (chance  < 100)
    pet = read_mobile(CALL_PEGASUS, VIRTUAL);
  else
    pet = read_mobile(CALL_DRAGON, VIRTUAL);

  act("You summon $N to be your mount.", FALSE, ch, 0, pet, TO_CHAR);
  act("$n summons $N to be $m mount.", FALSE, ch, 0, pet, TO_ROOM);

  IS_CARRYING_W(pet) = 1000;
  IS_CARRYING_N(pet) = 100;
  SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);  

  char_to_room(pet, IN_ROOM(ch));
  add_follower(pet, ch);
}

*/

ASPELL(spell_opaque)
{
  if (IS_OBJ_STAT(obj, ITEM_INVISIBLE)) {
    send_to_char("It's already invisible...\r\n", ch);
    return;

  } else
    act("$p shimmers and fades from sight.", FALSE, ch, obj, 0, TO_ROOM);
    act("$p shimmers and fades from sight.", FALSE, ch, obj, 0, TO_CHAR);
    SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
}

ASPELL(pray_reverse_align)
{
  if (IS_NPC(victim)) {
    send_to_char("You can only use this on players.\r\n", ch);
    return;
  }
  if (!pkill_ok) {
    send_to_char("You may not attack that player.\r\n", ch);
  }

  send_to_char("Your prayer is answered.\r\n", ch);
  send_to_char("You feel a change in yourself.\r\n", victim);
  GET_ALIGNMENT(victim) = GET_ALIGNMENT(victim) * -1;
}


ASPELL(pray_pacify)
{
  struct char_data *vict;


  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  } 

  if (number(1, 100) <= 75) {
    for(vict=world[ch->in_room].people; vict; vict=vict->next_in_room) {
      if (FIGHTING(vict)) {
        stop_fighting(vict);
        send_to_char("You feel quite peaceful.  What was all "
                     "the fighing about?\r\n", vict);
      }
      if (IS_NPC(vict)) {
        send_to_char("You lose your will to kill.\r\n", vict);
        clearMemory(vict);
        HUNTING(vict)=0;
      }
    }
  }
}

#define EXITN(room, door)     (world[room].dir_option[door])

ASPELL(spell_knock)
{
  struct room_direction_data *back = 0;
  int other_room = 0, dir;
  int open = 0;

  for(dir = 0; dir < 6; dir++)
  {
    if(!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
      continue;

    if(!IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED))
      continue;
   
    if(!IS_SET(EXIT(ch, dir)->exit_info, EX_LOCKED))
      continue;

    if(IS_SET(EXIT(ch, dir)->exit_info, EX_PICKPROOF))
    {
      if(EXIT(ch, dir)->keyword)
      {
        sprintf(buf, "The %s resists your magic.\r\n", fname(EXIT(ch,dir)->keyword));
        send_to_char(buf2,ch);
      }
      else
        send_to_char("It resists your magic.\r\n", ch);
      continue;
    }
  open ++;
  (TOGGLE_BIT(EXITN(ch->in_room, dir)->exit_info, EX_LOCKED));

    if ((EXIT(ch, dir)->to_room) != NOWHERE) {
      other_room = EXIT(ch, dir)->to_room;
      if((back = world[other_room].dir_option[rev_dir[dir]]))
        if(back->to_room != ch->in_room)
          back = 0;
    }
   
    if(back)
      (TOGGLE_BIT(EXITN(other_room, rev_dir[dir])->exit_info, EX_LOCKED));
     
    send_to_char("*Click*\r\n", ch);
 }//End for()
 
 if(!open)
 {
   send_to_char("Nothing happens.\r\n",ch);
 }

}

ASPELL(pray_retrieve_corpse)
{
  struct obj_data *i = 0;
  char name[MAX_INPUT_LENGTH];
  int num = 0;
  bool corpse_found=0;
  for (i = object_list; i; i = i->next) {
    if (!str_cmp(name, i->name))
      continue;
    if (i->in_room != NOWHERE) {
      if (GET_OBJ_TYPE(i) == ITEM_CONTAINER) {
        if (GET_OBJ_VAL(i, 3) == 1) {
			if (!check_get_corpse(ch, i) <= 0) {
				if (num == 0) {
					act("You call the corpse from where it lay.",
						FALSE, ch, 0, 0, TO_CHAR);
					act("$n closes $s eyes and summons a corpse from where it lay.",
						FALSE, ch, 0, 0, TO_ROOM);  
					obj_from_room(i);
					obj_to_room(i, ch->in_room);
					num = 1;
					corpse_found = TRUE;
				}
			}
			else if (check_get_corpse(ch, i) <= 0)
				send_to_char("You are not allowed to retrieve that corpse.\r\n", ch);
		}
      }
    }
  }
  if (!corpse_found)
    send_to_char("\r\n/cwYou cannot locate that corpse./c0\r\n", ch); 
}

/*
ASPELL(spell_animate_corpse) {

  struct char_data *undead = NULL;
  struct follow_type *f;
  int chance;
  int penalty=0;

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  }
  if (GET_OBJ_VAL(obj, 3) != 1) {
    send_to_char("You can only animate corpses.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(obj, 3) == 1) {
    if (obj->obj_flags.pid && GET_PFILEPOS(ch) != obj->obj_flags.pid) {
      send_to_char("You can't touch that corpse.\r\n", ch);
      return;
    }
  }
  for (f = ch->followers; f; f = f->next)
    if (IN_ROOM(ch) == IN_ROOM(f->follower)) {
      if (IS_MOB(f->follower))
        penalty+=5;
      else
        penalty+=2;
    }

  if (GET_CHA(ch) < (penalty + 5)) {
    send_to_char("You can not animate any more corpses than you already have.\r\n", ch);
    return;
  }

  chance = number(1, 100);

  if (chance < 30) {
    act("Your arts fail in their attempt to give $p life.", 
        FALSE, ch, obj, 0, TO_CHAR);
    act("$n bends over $p, but is unable to give it life.", 
        FALSE, ch, obj, 0, TO_NOTVICT);
    return;
  } else if (chance  < 100) {
    if (GET_LEVEL(ch) < 100)
      undead = read_mobile(3151, VIRTUAL);
    if (GET_LEVEL(ch) < 125 && GET_LEVEL(ch) > 99)
      undead = read_mobile(3152, VIRTUAL);
    if (GET_LEVEL(ch) > 124)
      undead = read_mobile(3153, VIRTUAL);
    else
      undead = read_mobile(3150, VIRTUAL);
  }

if (undead) {
  act("Your dark magics penetrate the veil of death, bringing life to "
      "$p's dead bones.", FALSE, ch, obj, 0, TO_CHAR);
  act("$n's dark magics bring life to $p, bringing it to life!", 
       FALSE, ch, obj, 0, TO_ROOM);

  IS_CARRYING_W(undead) = 1000;
  IS_CARRYING_N(undead) = 100;
  SET_BIT_AR(AFF_FLAGS(undead), AFF_CHARM);

  obj_from_room(obj);
  char_to_room(undead, IN_ROOM(ch));
  add_follower(undead, ch); 
}
else {log("animate dead: *undead is NULL");
}
}
*/
ASPELL(pray_holy_water)
{

  void name_to_drinkcon(struct obj_data * obj, int type);
  void name_from_drinkcon(struct obj_data * obj);

  if (GET_OBJ_TYPE(obj) != ITEM_DRINKCON) {
    send_to_char("You can only give your benediction to liquids in containers.\r\n", ch);
    return;
  }

  if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
    if (GET_OBJ_VAL(obj, 1) == 0) {
      send_to_char("That container is empty, nothing to bless.\r\n", ch);
      return;
    } else {
      GET_OBJ_VAL(obj, 2) = LIQ_GOODALIGN;
      name_to_drinkcon(obj, LIQ_GOODALIGN);
      act("$p shimmers briefly.", FALSE, ch, obj, 0, TO_CHAR);
      act("$p shimmers briefly as $n lays a hand upon it.", 
           FALSE, ch, obj, 0, TO_ROOM);
    }
  }
}

ASPELL(pray_choir)
{
  int percent, prob;

  if (GET_CLASS(ch) != CLASS_SAINT) {
    send_to_char("I swear... all you saint wanna-be's out there...\r\n", ch);
    return;
  }
  if (!GET_PRAYER(ch, PRAY_CHOIR)) {
    send_to_char("You cannot pray for that which you do not know how to pray for.\r\n", ch);
    return;
  }
  if (!FIGHTING(ch)) {
    send_to_char("There is no need to pray for help. You are without foes.\r\n", ch);
    return;
  }
  if (GET_ALIGNMENT(ch) <= 0) {
    send_to_char("Your alignment is too evil, you can not use this prayer.\r\n",ch);
    return;
  }
  percent = number(1, 101);
  prob = GET_PRAYER(ch, PRAY_CHOIR) + 20;

  if (percent <= prob) {
    if (FIGHTING(FIGHTING(ch)) == ch)
      stop_fighting(FIGHTING(ch));
    stop_fighting(ch);
    act("You utter a cry to heaven, begging for the mercy of the angels.\r\n"
        "You are granted solace.\r\n", TRUE, ch, 0, 0, TO_CHAR);
    act("You watch in frustration as $n is taken to into the sky, aided\r\n"
        "by the angels of heaven.\r\n",
        TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(number(21, 24)));
    look_at_room(ch, 0);
    act("The sound of singing angels fills the room as $n is gently dropped to the ground.",
        TRUE, ch, 0, 0, TO_ROOM);
    return;
  } else {
    send_to_char("The fighting is too intense, you cannot find escape.\r\n", ch);
    return;
  }
} 

ASPELL(spell_sense_spawn)
{
  int chance;

  chance = number(1, 100);

  if (chance <= 65) {
    send_to_char("You fail to sense when the next spawn will occur.\r\n", ch);
    return;
  }

  else
  sprintf(buf, "This zone should respawn in %d minutes.\r\n", 
         (zone_table[world[ch->in_room].zone].lifespan -
          zone_table[world[ch->in_room].zone].age));
  send_to_char(buf, ch);
}
