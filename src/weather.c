/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
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
#include "handler.h"
#include "interpreter.h"
#include "db.h"

extern struct time_info_data time_info;

extern struct descriptor_data *descriptor_list;
extern struct room_data *world;

extern struct obj_data *object_list;
extern struct char_data *character_list;

void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void sector_update();
void die(struct char_data *ch, struct char_data *killer);
ACMD(do_weather);
void auc_timer();

void auc_timer()
{
  struct char_data *tch;
  struct obj_data *obj;
  struct descriptor_data *d;
  int playing =0, deadweight=0;

  for (obj = object_list; obj; obj = obj->next) {
    if (GET_OBJ_AUC_TIMER(obj) >= 2)
      GET_OBJ_AUC_TIMER(obj)--;
    if (GET_OBJ_AUC_TIMER(obj) == 1) {
      GET_OBJ_AUC_TIMER(obj) = 0;
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

        if (GET_BIDDER(obj)) {
          obj_to_char(obj, GET_BIDDER(obj));
          GET_GOLD(GET_BIDDER(obj)) -= GET_OBJ_BID_AMNT(obj);
          sprintf(buf, "/cRAUCTION::/cw You pay %d coins for %s./c0\r\n",
                  GET_OBJ_BID_AMNT(obj), obj->short_description);
          send_to_char(buf, GET_BIDDER(obj));
          GET_BIDDER(obj) = NULL;
        }

        if (GET_OBJ_AUC_PLAYER(obj) <= 0 &&
            GET_OBJ_AUC_SELLER(obj) == GET_IDNUM(tch)) {
          sprintf(buf, "/cRAUCTION:: /cwUnfortunately, no one wants %s.\r\n"
                       " /cw         It is returned to you./c0\r\n",
                       obj->short_description);
          send_to_char(buf, tch);
          obj_from_room(obj);
          obj_to_char(obj, tch);
          GET_OBJ_BID_NUM(obj) = 0;
          REMOVE_BIT_AR(PLR_FLAGS(tch), PLR_SELLER);
          return;
        }
        if (GET_OBJ_AUC_SELLER(obj) == GET_IDNUM(tch)) {
          GET_GOLD(tch) += GET_OBJ_BID_AMNT(obj);
          sprintf(buf, "/cRAUCTION:: /cwYou receive %d coins for %s./c0\r\n",
                  GET_OBJ_BID_AMNT(obj), obj->short_description);
          send_to_char(buf, tch);
          REMOVE_BIT_AR(PLR_FLAGS(tch), PLR_SELLER);
          GET_OBJ_BID_NUM(obj) = 0;
          GET_OBJ_AUC_PLAYER(obj) = -1;
        }
       }
      }
    }
  }
} 

void weather_and_time(int mode)
{
  another_hour(mode);
  if (mode)
    weather_change();
}


void noon()
{
  struct descriptor_data *d;
  int INS_COST;

  if (time_info.hours != 12)
    return;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;

    if (PLR_FLAGGED(d->character, PLR_INSURED)) {
      INS_COST = (GET_LEVEL(d->character) * 50) +
                 (GET_TIER(d->character) * 7500);
      if (GET_BANK_GOLD(d->character) < INS_COST) {
        send_to_char("You could not afford your equipment insurance policy "
                   "any longer. Your policy has been cancelled.", d->character);
        REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_INSURED);
      }
      sprintf(buf, "Your equipment insurance policy is due.\r\n"
                   "%d coins have been withdrawn from your account.\r\n",
                   INS_COST);
      send_to_char(buf, d->character);
      GET_BANK_GOLD(d->character) -= INS_COST;
    }
  } 
}


void another_hour(int mode)
{
  time_info.hours++;

  if (mode) {
    switch (time_info.hours) {
  
    case 5:
      weather_info.sunlight = SUN_RISE;
      send_to_outdoor("The sun rises in the east.\r\n");
      break;
    case 6:
      weather_info.sunlight = SUN_LIGHT;
      send_to_outdoor("A new day in Dibrova has begun.\r\n");
      break;
    case 21:
      weather_info.sunlight = SUN_SET;
      send_to_outdoor("The sun sinks slowly into Dibrova's horizon.\r\n");
      break;
    case 22:
      weather_info.sunlight = SUN_DARK;
      send_to_outdoor("Night descends upon Dibrova.\r\n");
      break;
    default:
      break;
    }
  }
  if (time_info.hours > 23) {	/* Changed by HHS due to bug ??? */
    time_info.hours -= 24;
    time_info.day++;
    if(weather_info.sky == SKY_SNOWING) { weather_info.ground_snow++; }
    
    else if(weather_info.ground_snow && weather_info.sky != SKY_SNOWING) { weather_info.ground_snow--; }

    if (time_info.day > 34) {
      time_info.day = 0;
      time_info.month++;

      if (time_info.month > 16) {
	time_info.month = 0;
	time_info.year++;
      }
    }
  }
}

void weather_change(void)
{
  int diff, change;
  if ((time_info.month >= 9) && (time_info.month <= 16))
    diff = (weather_info.pressure > 985 ? -2 : 2);
  else
    diff = (weather_info.pressure > 1500 ? -2 : 2);

  weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1400);
  weather_info.pressure = MAX(weather_info.pressure, 700);

  change = 0;

  switch (weather_info.sky) {
  case SKY_CLOUDLESS:
    if (weather_info.pressure < 990)
      change = 1;
    else if (weather_info.pressure < 1010)
      if (dice(1, 4) == 1)
	change = 1;
    break;
  case SKY_CLOUDY:
    if (weather_info.pressure < 970)
      change = 2;
    else if (weather_info.pressure < 990)
      if (dice(1, 4) == 1)
	change = 2;
      else
	change = 0;
    else if (weather_info.pressure > 1030)
      if (dice(1, 4) == 1)
	change = 3;

    break;
  case SKY_RAINING:
    if (weather_info.pressure < 970)
      if (dice(1, 4) == 1)
	change = 4;
      else if (number(1, 4) == 2)
        change = 7;
      else
	change = 0;
    else if (weather_info.pressure > 1030)
      change = 5;
    else if (weather_info.pressure > 1010)
      if (dice(1, 4) == 1)
	change = 5;

    break;
  case SKY_LIGHTNING:
    if (weather_info.pressure < 900)
      change = 8;
    else if (weather_info.pressure > 990)
      if (dice(1, 4) == 1)
	change = 6;

    break;

  case SKY_SNOWING:
    if (weather_info.pressure > 1050)
      change = 7;
    break;
    
  default:
    change = 0;
    weather_info.sky = SKY_CLOUDLESS;
    break;
  }

  switch (change) {
  case 0:
    break;
  case 1:
    send_to_outdoor("The sky begins to darken as thunderclouds roll in./c0\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 2:
    send_to_outdoor("It starts to rain.\r\n");
    weather_info.sky = SKY_RAINING;
    break;
  case 3:
    send_to_outdoor("The clouds disappear.\r\n");
    weather_info.sky = SKY_CLOUDLESS;
    break;
  case 4:
    send_to_outdoor("Lightning starts to show in the sky.\r\n");
    weather_info.sky = SKY_LIGHTNING;
    break;
  case 5:
    send_to_outdoor("The rain stops.\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 6:
    send_to_outdoor("The lightning stops.\r\n");
    weather_info.sky = SKY_RAINING;
    break;
  case 7:
    send_to_outdoor("The snow stops falling.\r\n");
    weather_info.sky = SKY_CLOUDY;
    break;
  case 8:
    send_to_outdoor("Large clumps of snow begin falling from the sky.\r\n");
    weather_info.sky = SKY_SNOWING;
    weather_info.ground_snow++;
    break;
  default:
    break;
  }
}


void turkey_load() {

  int room_num = 0;
  struct char_data *turkey = NULL;

  room_num = real_room(number(100, 32500));

  if (room_num != NOWHERE && number(1, 100) <= 5) {
 
    turkey = read_mobile(25005, VIRTUAL);
    char_to_room(turkey, room_num);
  }
  
}

void vamp_eq_load() {

  int room_num, obj_num;
  struct obj_data *imm_obj;

  room_num = real_room(number(100, 32500));

  if (room_num != NOWHERE && number(1, 500) <= 5) {
    /* Immortal eq random loads */
    obj_num = number(26200, 26599);
    imm_obj = read_object(obj_num, VIRTUAL);
    obj_to_room(imm_obj, room_num);
  }
  /* End immortal eq random loads */ 
}

void quest_obj_loads() {

  int room_num, obj_num, which_one;
  struct obj_data *imm_obj;

  room_num = real_room(number(100, 50000));

  if (room_num != NOWHERE && number(1, 500) <= 18) {
    /* quest obj loads */
    which_one = number(1, 7);
    if (which_one <= 3) {
      obj_num = number(26192, 26199);
      imm_obj = read_object(obj_num, VIRTUAL);
      obj_to_room(imm_obj, room_num);
    } else if (which_one <= 6 && which_one >= 4) {
      obj_num = 12;
      imm_obj = read_object(obj_num, VIRTUAL);
      obj_to_room(imm_obj, room_num);
    } else {
      obj_num = number(26033, 26091);
      imm_obj = read_object(obj_num, VIRTUAL);
      obj_to_room(imm_obj, room_num);
    }
  }    
}


void titan_absorbtion() {

  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;

    if (GET_CLASS(d->character) == CLASS_TITAN &&
        IS_AFFECTED(d->character, AFF_ABSORB)) {

      send_to_char("The earth's energies nourish you.\r\n",
                    d->character);
      GET_HIT(d->character) = MIN(GET_MAX_HIT(d->character),
             (GET_HIT(d->character) + GET_LEVEL(d->character) +
              GET_CON(d->character) * 2));
      GET_MANA(d->character) = MIN(GET_MAX_MANA(d->character),
              GET_MANA(d->character) + GET_LEVEL(d->character) +
              GET_INT(d->character));
      GET_MOVE(d->character) = MIN(GET_MAX_MOVE(d->character),
              GET_MOVE(d->character) + GET_LEVEL(d->character) +
              GET_DEX(d->character));
      GET_QI(d->character) = MIN(GET_MAX_QI(d->character),
              GET_QI(d->character) + GET_LEVEL(d->character) +
              GET_INT(d->character));
      GET_VIM(d->character) = MIN(GET_MAX_VIM(d->character),
              GET_VIM(d->character) + GET_LEVEL(d->character) +
              GET_INT(d->character)); 
    }
  }
}

void check_fishing() {

  struct descriptor_data *d;
  int bite;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;

    if (PLR_FLAGGED(d->character, PLR_FISHING) &&
      (!ROOM_FLAGGED(d->character->in_room, ROOM_SALTWATER_FISH) &&
       !ROOM_FLAGGED(d->character->in_room, ROOM_FRESHWATER_FISH)))
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_FISHING);

    if (PLR_FLAGGED(d->character, PLR_FISHING) && 
       !PLR_FLAGGED(d->character, PLR_FISH_ON)) {

      bite = number(1, 10);
    
      if (bite >= 7 && bite <= 8) {
        send_to_char("Time goes by... not even a nibble.\r\n", d->character);
      } else if (bite >= 6) {
       send_to_char("You feel a slight jiggle on your line.\r\n", d->character);
      } else if (bite >= 4) {
       send_to_char("You feel a very solid pull on your line!\r\n", 
                     d->character); 
       SET_BIT_AR(PLR_FLAGS(d->character), PLR_FISH_ON);
      } else if (bite >= 2) {
       send_to_char("Your line suddenly jumps to life, FISH ON!!!\r\n",
                     d->character);
       SET_BIT_AR(PLR_FLAGS(d->character), PLR_FISH_ON);
      }
    }
  }  
}

void vamp_sun_dam() {
  
  struct descriptor_data *d;
  int dam;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;

    dam = number (200, 300);
	if (SECT(d->character->in_room) == SECT_CAVE)
		return;

    if ((GET_CLASS(d->character) == CLASS_VAMPIRE) &&
      weather_info.sunlight == SUN_RISE &&
      !IS_AFFECTED((d->character), AFF_SHROUD) &&
      !IS_AFFECTED((d->character), AFF_BURROW)) {
      send_to_char("/crThe heat of the sun scorches your flesh!/c0\r\n",
                      d->character);
      if (GET_HIT(d->character) < dam) {
        GET_HIT(d->character) = (GET_HIT(d->character) - (GET_HIT(d->character) + 5));
        send_to_char("You must find blood and don a shroud, else remain "
                     "at the mercy of your foes!\r\n", d->character);
      } else
      GET_HIT(d->character) = GET_HIT(d->character) - dam;
    }
    if ((GET_CLASS(d->character) == CLASS_VAMPIRE) &&
        weather_info.sunlight == SUN_LIGHT &&
        !IS_AFFECTED((d->character), AFF_SHROUD) &&
        !IS_AFFECTED((d->character), AFF_BURROW)) {
      send_to_char("/crThe heat of the sun scorches your flesh!/c0\r\n",
                    d->character);
      if (GET_HIT(d->character) < dam) {
        GET_HIT(d->character) = (GET_HIT(d->character) - (GET_HIT(d->character) + 5));
        send_to_char("You must find blood and don a shroud, else remain "
                   "at the mercy of your foes!\r\n", d->character);
      } else
      GET_HIT(d->character) = GET_HIT(d->character) - dam;
    }
  }
}

/* Some sectors will cause things to happen based on */
/* the type of room you are in.  I'm putting this func     */
/* here because I can't think of any appropriate place.    */
void sector_affects() {

  struct descriptor_data *d;
  int i, dir, was_in, dam;

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;
    if(!IS_NPC(d->character) && GET_EXP(d->character) < 1) { 
       GET_EXP(d->character) = 10;
    }
    dam = number (200, 300);

    if(GET_POS(d->character) == POS_SLEEPING) {
      switch(number(0, 5)) {
      case 1:
       // Little bit of a prank, sorry Kaan. :p 
       send_to_char("You dream you had a Kaan blowup doll of your own.\r\n", d->character);
       break;
     case 2:
       send_to_char("You dream of never complaining about Dibrova again.\r\n", d->character);

       break;
     case 3:
       send_to_char("You dream of a horde of rabid wee turtles, oh the humanity!\r\n", d->character);
       break;
     case 4:
       if(GET_SEX(d->character) == 1) {
	send_to_char("You dream of Shannon Elizabeth, yummy.....\r\n", d->character);
       }
       else {
        send_to_char("You dream of N'Sync, yummy.....\r\n", d->character);
       }
       break;
     default:
       break;
    }
  }
  
  if(ROOM_FLAGGED(d->character->in_room, ROOM_HEALING) &&
          GET_CLASS(d->character) != CLASS_VAMPIRE) {
    send_to_char("A healing wind blows through the room.\r\n",
                  d->character);
    GET_HIT(d->character) = MIN(GET_MAX_HIT(d->character), 
            GET_HIT(d->character) + GET_LEVEL(d->character) + 
            GET_CON(d->character));
    GET_MANA(d->character) = MIN(GET_MAX_MANA(d->character), 
            GET_MANA(d->character) + GET_LEVEL(d->character) +
            GET_INT(d->character));
    GET_MOVE(d->character) = MIN(GET_MAX_MOVE(d->character), 
            GET_MOVE(d->character) + GET_LEVEL(d->character) +
            GET_DEX(d->character));
    GET_QI(d->character) = MIN(GET_MAX_QI(d->character), 
            GET_QI(d->character) + GET_LEVEL(d->character) +
            GET_INT(d->character));
    GET_VIM(d->character) = MIN(GET_MAX_VIM(d->character), 
            GET_VIM(d->character) + GET_LEVEL(d->character) +
            GET_INT(d->character));
  }
  if (ROOM_FLAGGED(d->character->in_room, ROOM_HAVEN) &&
      GET_CLASS(d->character) != CLASS_VAMPIRE &&
      GET_LEVEL(d->character) < LVL_IMMORT) {
      send_to_char("The mist seethes into your body, draining your life!\r\n",
                    d->character);
        GET_HIT(d->character) = MIN(GET_MAX_HIT(d->character),
                GET_HIT(d->character) - GET_LEVEL(d->character));
        GET_MANA(d->character) = MIN(GET_MAX_MANA(d->character),
                GET_MANA(d->character) - GET_LEVEL(d->character));
        GET_MOVE(d->character) = MIN(GET_MAX_MOVE(d->character),
                GET_MOVE(d->character) - GET_LEVEL(d->character));
        GET_QI(d->character) = MIN(GET_MAX_QI(d->character),
                GET_QI(d->character) - GET_LEVEL(d->character));
        GET_VIM(d->character) = MIN(GET_MAX_VIM(d->character),
                GET_VIM(d->character) - GET_LEVEL(d->character));
  }

  switch(SECT(d->character->in_room)) {
    case SECT_DESERT:
      send_to_char("/crThe intense heat of the desert drains you./c0\r\n",
                   d->character);
      if (GET_HIT(d->character) > 3) GET_HIT(d->character)-=2;
      break;
    case SECT_SNOW:
      send_to_char("/cCFrigid winds tear at your skin./c0\r\n",
                   d->character);
      if (GET_HIT(d->character) > 3) GET_HIT(d->character)-=2;
      break;
    case SECT_UNDERWATER:
      if (!(IS_AFFECTED(d->character, AFF_BREATHE))) {
        send_to_char("/cWYou thrash in agony as water floods into your lungs.  "
                     "You're /cRDROWNING!/c0\r\n", d->character);
        GET_HIT(d->character) -= number(20, 40);
        update_pos(d->character);
      }
      break;
    case SECT_OCEAN:
      if (PRF_FLAGGED(d->character, PRF_NOHASSLE) ||
          AFF_FLAGGED(d->character, AFF_FLY) ||
          AFF_FLAGGED(d->character, AFF_HOVER)) break;
      for (i=0; i < 4; i++) { 
        dir = number(0, NUM_OF_DIRS-1);
        if (CAN_GO(d->character, dir)) {
          act("$n is tossed by the waves.", FALSE, d->character, 0, 0, TO_ROOM);
          send_to_char("/cBYou are tossed violently by the large waves./c0\r\n",
                       d->character);
          was_in = d->character->in_room;
          char_from_room(d->character); 
          char_to_room(d->character, world[was_in].dir_option[dir]->to_room);
          look_at_room(d->character, 0);
          return;
        }
      }
      break;
    } /* switch */
  } /* for... */

}

/* ported from Eltanin by AxL, 2feb97 */
/*  added by Miikka M. Kangas  8-14-91 (note this is called with own pulses!)*/

/*   I was getting bored so I wanted to add some edge to weather.    */
void beware_lightning(void)
{
  int dam = 0;
  struct char_data *victim = NULL, *temp = NULL;
  char buf[256];
  extern struct char_data *character_list;


  if (!(weather_info.sky == SKY_LIGHTNING))
    return;   /*   go away if its not even stormy!   */

  if (number(0, 17))
    return;  /*  Bolt only 10% of time  */

  if (number(0,500)) {   /* nobody targeted 99% */
    send_to_outdoor("You hear the clap of distant thunder.\n\r");
    return;
  }

  for (victim = character_list; victim; victim = temp) {
    temp = victim->next;
    if ( (OUTSIDE(victim) == TRUE) && (!IS_NPC(victim)) ) {  /* PCs only */
      if  ( (number(0, 9) == 0) ) {  /* hit 10%  */


/* damage routine here */

dam = number(1, (GET_MAX_HIT(victim) * 2));

if (IS_AFFECTED(victim, AFF_SANCTUARY))
          dam = MIN(dam, 18);  /* Max 18 damage when sanctuary */

dam = MIN(dam, GET_LEVEL(victim) * 3);  /* no more than level * 3 hp per round */

dam = MAX(dam, 0);    /* no less than 0 hp per round */

if ((GET_LEVEL(victim) >= LVL_IMMORT) && !IS_NPC(victim))
        /* You can't damage an immortal! */
          dam = 0;

if(dam > GET_HIT(victim)) {
  GET_HIT(victim) = -1;
}
else {
  GET_HIT(victim) -= dam;
}
act("KAZAK! a lightning bolt hits $n.  You hear a sick sizzle.",
         TRUE, victim, 0, 0, TO_ROOM);

act("KAZAK! a lightning bolt hits you.  You hear a sick sizzle.",
         FALSE, victim, 0, 0, TO_CHAR);

        if (dam > (GET_MAX_HIT(victim) >> 2))
          act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

        send_to_outdoor("*BOOM*  You hear the clap of thunder.\n\r");
        update_pos(victim);
        switch (GET_POS(victim)) {
          case POS_MORTALLYW:
            act("$n is mortally wounded, and will die soon, if not aided.",
             TRUE, victim, 0, 0, TO_ROOM);

    act("You are mortally wounded, and will die soon, if not aided.",
             FALSE, victim, 0, 0, TO_CHAR);
            break;

  case POS_INCAP:

    act("$n is incapacitated and will slowly die, if not aided.",
             TRUE, victim, 0, 0, TO_ROOM);

    act("You are incapacitated an will slowly die, if not aided.",
              FALSE, victim, 0, 0, TO_CHAR);
            break;

  case POS_STUNNED:
            act("$n is stunned, but will probably regain conscience again.",
              TRUE, victim, 0, 0, TO_ROOM);

    act("You're stunned, but will probably regain conscience again.",
              FALSE, victim, 0, 0, TO_CHAR);
            break;
          case POS_DEAD:
            act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
            act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
            break;
          default:  /* >= POS_SLEEPING */
            if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
              act("You wish that your wounds would stop BLEEDING that much!",
              FALSE,victim,0,0,TO_CHAR);

    }  /* if BLEEDING */
            break;
        } /* switch */

        if (GET_POS(victim) == POS_DEAD) {
          sprintf(buf, "Thunderstorm killed %s", GET_NAME(victim));
          log(buf);
          die(victim, victim);
        } /* of death */
        return;
      } else { /* number(0,10) */
          act("KAZAK! a lightning bolt hits nearby.",
            FALSE, victim, 0, 0, TO_ROOM);
          act("KAZAK! a lightning bolt hits nearby.",
            FALSE, victim, 0, 0, TO_CHAR);
          send_to_outdoor("*BOOM*  You hear the clap of thunder.\n\r");
          return;    /*  only 1 bolt at a time   */
      }
    }  /*  if outside=true */
  } /* of for victim hunt */
} /* of procedure */



#define NUM_DAYS 35
/* Match this to the number of days per month; this is the moon cycle */
#define NUM_MONTHS 17
/* Match this to the number of months defined in month_name[].  */
#define MAP_WIDTH 72
#define SHOW_WIDTH MAP_WIDTH/2
#define MAP_HEIGHT 9
/* Should be the string length and number of the constants below.*/
const char * star_map[] =
{
"   W.N     ' .     :. M,N     :  y:.,N    `  ,       B,N      .      .  ",
" W. :.N .      G,N  :M.: .N  :` y.N    .      :     B:   .N       :     ",
"    W:N    G.N:       M:.,N:.:   y`N      ,    c.N           .:    `    ",
"   W.`:N       '. G.N  `  : ::.      y.N      c'N      B.N R., ,N       ",
" W:'  `:N .  G. N    `  :    .y.N:.          ,     B.N      :  R:   . .N",
":' '.   .    G:.N      .'   '   :::.  ,  c.N   :c.N    `        R`.N    ",
"      :       `        `        :. ::. :     '  :        ,   , R.`:N    ",
"  ,       G:.N              `y.N :. ::.c`N      c`.N   '        `      .",
"     ..        G.:N :           .:   c.N:.    .              .          "
};

/***************************CONSTELLATIONS*******************************
  Lupus     Gigas      Pyx      Enigma   Centaurus    Terken    Raptus
   The       The       The       The       The         The       The  
White Wolf  Giant     Pixie     Sphinx    Centaur      Drow     Raptor
*************************************************************************/	
const char *sun_map[] =
{
")'|'(",
"- O -",
").|.("
}; 
const char *moon_map[] =
{
" @@@ ",
"@@@@@",
" @@@ "
};

void look_sky(struct char_data *ch)
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[4];
    int starpos, sunpos, moonpos, moonphase, i, linenum;

   if(weather_info.sky > 0) {
	send_to_char("The cloudy sky obscures your vision.\r\n", ch);
	do_weather(ch, 0, 0, 0);
	return;
   }

   send_to_char("You gaze up towards the heavens and see:\n\r",ch);

   sunpos  = (MAP_WIDTH * (24 - time_info.hours) / 24);
   moonpos = (sunpos + time_info.day * MAP_WIDTH / NUM_DAYS) % MAP_WIDTH;
   if ((moonphase = ((((MAP_WIDTH + moonpos - sunpos ) % MAP_WIDTH ) +
                      (MAP_WIDTH/16)) * 8 ) / MAP_WIDTH)
		         > 4) moonphase -= 8;
   starpos = (sunpos + MAP_WIDTH * time_info.month / NUM_MONTHS) % MAP_WIDTH;
   /* The left end of the star_map will be straight overhead at midnight during 
      month 0 */

   for ( linenum = 0; linenum < MAP_HEIGHT; linenum++ )
   {
     if ((time_info.hours >= 6 && time_info.hours <= 18) &&
         (linenum < 3 || linenum >= 6))
       continue;
     sprintf(buf,"/cw");
     for ( i = MAP_WIDTH/4; i <= 3*MAP_WIDTH/4; i++)
     {
       /* plot moon on top of anything else...unless new moon & no eclipse */
       if ((time_info.hours >= 6 && time_info.hours <= 18)  /* daytime? */
        && (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && ((sunpos == moonpos && time_info.hours == 12) || moonphase != 0  ) /*no eclipse*/
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"/cW@");
         else
           strcat(buf," ");
       }
       else
       if ((linenum >= 3) && (linenum < 6) && /* nighttime */
           (moonpos >= MAP_WIDTH/4 - 2) && (moonpos <= 3*MAP_WIDTH/4 + 2) /* in sky? */
        && ( i >= moonpos - 2 ) && (i <= moonpos + 2) /* is this pixel near moon? */
        && (moon_map[linenum-3][i+2-moonpos] == '@'))
       {
         if ((moonphase < 0 && i - 2 - moonpos >= moonphase) ||
             (moonphase > 0 && i + 2 - moonpos <= moonphase))
           strcat(buf,"/cW@");
         else
           strcat(buf," ");
       }
       else /* plot sun or stars */
       {
         if (time_info.hours>=6 && time_info.hours<=18) /* daytime */
         {
           if ( i >= sunpos - 2 && i <= sunpos + 2 )
           {
             sprintf(buf2,"/cy%c",sun_map[linenum-3][i+2-sunpos]);
             strcat(buf,buf2);
           }
           else
             strcat(buf," ");
         }
         else
         {
           switch (star_map[linenum][(MAP_WIDTH + i - starpos)%MAP_WIDTH])
           {
             default     : strcat(buf," ");    break;
             case '.'    : strcat(buf,".");    break;
             case ','    : strcat(buf,",");    break;
             case ':'    : strcat(buf,":");    break;
             case '`'    : strcat(buf,"`");    break;
             case 'R'    : strcat(buf,"/cR ");  break;
             case 'G'    : strcat(buf,"/cG ");  break;
             case 'B'    : strcat(buf,"/cB ");  break;
             case 'W'    : strcat(buf,"/cW ");  break;
             case 'M'    : strcat(buf,"/cw ");  break;
             case 'N'    : strcat(buf,"/cb ");  break;
             case 'y'    : strcat(buf,"/cy ");  break;
             case 'c'    : strcat(buf,"/cr ");  break;
           }
         }
       }
     }
     strcat(buf,"/cW\n\r");
     send_to_char(buf,ch);
   }
}

