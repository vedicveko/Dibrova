/*****************************************************************************
**                                                                          **
** Skills for mobiles.                                                      **
**                                                                          **
**                                                       Vedic -- 10/20/99  **
**                                                                          **
*****************************************************************************/

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "utils.h"

extern struct room_data *world;
ACMD(do_flee);
void send_to_zone_outdoor(int zone_rnum, char *messg);
void mobsay(struct char_data *ch, const char *msg);


void do_mob_bash(struct char_data *ch, struct char_data *vict)
{
  struct char_data *mob = NULL;
  struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
  int hit_roll = 0, to_hit = 0;

  if (!wielded) 
    send_to_char("You need a weapon to bash.\r\n", ch);
  else if (GET_POS(vict) < POS_FIGHTING) 
    send_to_char("How can you bash someone who's already down?\r\n", ch);
  else {
    hit_roll = number (1,80) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));

    if (GET_LEVEL(vict) >= LVL_IMMORT)  
      hit_roll = 0;

    if (AFF_FLAGGED(vict, AFF_BALANCE))
      hit_roll = 0;

    if (hit_roll < to_hit) {
      GET_POS(ch) = POS_SITTING;
      damage(ch, vict, 0, SKILL_BASH, ABT_SKILL);
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
      GET_POS(vict) = POS_SITTING;
      damage(ch, vict, GET_LEVEL(ch) / 2, SKILL_BASH, ABT_SKILL);
      WAIT_STATE(vict, PULSE_VIOLENCE);
    } 
  }
    GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;
}

void do_mob_disarm(struct char_data *ch, struct char_data *vict)
{

  int hit_roll = 0, to_hit = 0;
  struct obj_data *weap; 
 
  hit_roll = number (1,80) + GET_DEX(ch);
  to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
  if (GET_LEVEL(vict) >= LVL_IMMORT)  
    hit_roll = 0;

  if (!(weap = GET_EQ(FIGHTING(ch), WEAR_WIELD)))
    send_to_char("Nope, sorry\r\n", ch);
  else {
    if (hit_roll < to_hit) { 	/* Disarm */
      act("You knock $p from $N's hand!", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
      act("$n knocks $p from your hand!", FALSE, ch, weap, FIGHTING(ch), TO_VICT); 
      act("$n knocks $p from $N's hand!", FALSE, ch, weap, FIGHTING(ch), TO_NOTVICT);
      obj_to_char(unequip_char(FIGHTING(ch), WEAR_WIELD), FIGHTING(ch));
    } else {
      act("You fail to disarm $N", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
      act("$n fails to disarm you", FALSE, ch, weap, FIGHTING(ch), TO_VICT);
      act("$n fails to disarm $N", FALSE, ch, weap, FIGHTING(ch), TO_NOTVICT);
    }
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
  GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;

}

void do_mob_kickflip(struct char_data *ch, struct char_data *vict)
{
  struct char_data *mob = NULL;
  int hit_roll = 0, to_hit = 0;
  
  hit_roll = number (1,80) + GET_STR(ch);
  to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));
  if (GET_LEVEL(vict) >= LVL_IMMORT)  
    hit_roll = 0;

  if (hit_roll < to_hit) {
    GET_POS(ch) = POS_SITTING;
    damage(ch, vict, 0, SKILL_KICKFLIP, ABT_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  } else {
    GET_POS(vict) = POS_SITTING;
    damage(ch, vict, GET_LEVEL(ch) / 2, SKILL_KICKFLIP, ABT_SKILL);
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;
}


/* This is for straight damage skills like kick and such. */

void do_generic_skill(struct char_data *ch, struct char_data *vict, int type, int dam)
{

  int hit_roll = 0, to_hit = 0;
  
  if (dam == 0) { dam = GET_LEVEL(ch); }
  hit_roll = number (1,80) + GET_STR(ch);
  to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));

  if (hit_roll < to_hit) {       
    damage(ch, vict, 0, type, ABT_SKILL);
  } else {
    damage(ch, vict, dam, type, ABT_SKILL);
  }  
  GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;
}

/* this is for bash, trip, kickflip, etc */

void do_generic_knock_down(struct char_data *ch, struct char_data *vict, int type)
{
	struct char_data *mob = NULL;
  int hit_roll = 0, to_hit = 0;
  
  if (GET_POS(vict) < POS_FIGHTING)  
    send_to_char("They are already down!\r\n", ch);
  else {
    hit_roll = number (1,80) + GET_STR(ch);
    to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));

	if (AFF_FLAGGED(vict, AFF_BALANCE))
      hit_roll = 0;

    if (hit_roll < to_hit) {
      GET_POS(ch) = POS_SITTING;
      damage(ch, vict, 0, type, ABT_SKILL);
      WAIT_STATE(ch, PULSE_VIOLENCE * 2)
    } else {
      GET_POS(vict) = POS_SITTING;
      damage(ch, vict, GET_LEVEL(ch) / 2, type, ABT_SKILL);
      WAIT_STATE(vict, PULSE_VIOLENCE);
    }
  }
  GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;
}

void mob_steal_eq(struct char_data *ch, struct char_data *vict)
{

  struct obj_data *obj;
  int percent, to_steal = 0, save;

  percent = number(1, GET_LEVEL(ch)) + GET_DEX(ch);
  save = number(1, GET_LEVEL(vict)) + GET_DEX(vict);

  if (GET_POS(vict) <= POS_SLEEPING)
    save = 0;		/* ALWAYS SUCCESS */

  if (GET_POS(ch) != POS_STANDING) { return; }

  switch(number(0, NUM_WEARS)) {
      
  case 1:
    if (GET_EQ(vict, WEAR_FINGER_R)) {
      to_steal = WEAR_FINGER_R;
    }
    break;
  case 2:
    if (GET_EQ(vict, WEAR_FINGER_L)) {
      to_steal = WEAR_FINGER_L;
    }
    break;
  case 3:
    if (GET_EQ(vict, WEAR_NECK_1)) {
      to_steal = WEAR_NECK_1;
    }
    break;
  case 4:
    if (GET_EQ(vict, WEAR_NECK_2)) {
      to_steal = WEAR_NECK_2;
    }
    break;
  case 14:
    if (GET_EQ(vict, WEAR_WRIST_R)) {
      to_steal = WEAR_WRIST_R;
    }
    break;
  case 15:
    if (GET_EQ(vict, WEAR_WRIST_L)) {
      to_steal = WEAR_WRIST_L;
    }
    break;
  case 18:
    if (GET_EQ(vict, WEAR_EAR_R)) {
      to_steal = WEAR_EAR_R;
    }
    break;
  case 19:
    if (GET_EQ(vict, WEAR_EAR_L)) {
      to_steal = WEAR_EAR_L;
    }
    break;
  case 20:
    if (GET_EQ(vict, WEAR_THUMB_R)) {
      to_steal = WEAR_THUMB_R;
    }
    break;
  case 21:
    if (GET_EQ(vict, WEAR_THUMB_L)) {
      to_steal = WEAR_THUMB_L;
    }
    break;
  default:
    break;
  }
  
  if (!to_steal) { return; }
  
  obj = GET_EQ(vict, to_steal);
  
  if (percent > save) {
	  if(IS_OBJ_STAT(obj, ITEM_NOTRANSFER) || IS_OBJ_STAT(obj, ITEM_NOSTEAL))
	  {
		  act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_ROOM);
		  act("$n tries to steal something from you.", TRUE, ch, 0, vict, TO_VICT);
		  send_to_char("You fail.\r\n", ch);
		  return;
	  } else {
		  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
		  obj_to_char(unequip_char(vict, to_steal), ch);
		  sprintf(buf, "THIEF: %s steals %s from %s\r\n", GET_NAME(ch), obj->short_description, GET_NAME(vict));
		  log(buf);
	  }
  } else {
    act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
    act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
  }
  
}

void mob_steal_from_inv(struct char_data *ch, struct char_data *vict)
{

  struct obj_data *obj, *list;
  int percent, save, found = 0;

  percent = number(1, GET_LEVEL(ch)) + GET_DEX(ch);
  save = number(1, GET_LEVEL(vict)) + GET_DEX(vict);

  if (GET_POS(vict) <= POS_SLEEPING)
    save = 0;		/* ALWAYS SUCCESS */

  if (GET_POS(ch) != POS_STANDING) { return; }

  
  list = vict->carrying;
  for (obj = list; obj; obj = obj->next_content) {
    if (CAN_SEE_OBJ(vict, obj)) {
      found = 1;
    }
  }
  if (found == 1) {
    for (obj = list; obj; obj = obj->next_content)
      {
	if (percent > save) {
if(IS_OBJ_STAT(obj, ITEM_NOTRANSFER) || IS_OBJ_STAT(obj, ITEM_NOSTEAL)) {
  act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_ROOM);
  act("$n tries to steal something from you.", TRUE, ch, 0, vict, TO_VICT);
  send_to_char("You fail.\r\n", ch);
  return;
}
	  obj_from_char(obj);
	  obj_to_char(obj, ch);
	  act("$n steals $p from $N.", FALSE, ch, obj, vict, TO_NOTVICT);
          sprintf(buf, "THIEF: %s steals %s from %s\r\n", GET_NAME(ch), obj->short_description, GET_NAME(vict));
	  log(buf);
	  return;
	} else {
	  act("$n tried to steal something from you!", FALSE, ch, 0, vict, TO_VICT);
	  act("$n tries to steal something from $N.", TRUE, ch, 0, vict, TO_NOTVICT);
	  return;
	}
      }
  }
}


#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

static int warrior = 1;

void warrior_combat(struct char_data *ch, struct char_data *vict)
{   
    int wstyle = 1;

    // rescue helpers
    // hurt combat
    // standard combat 

    switch(wstyle) {
     case 1:
     switch(warrior) {
      case 1:
        do_mob_bash(ch, vict);
       break;
     case 2:
       do_generic_skill(ch, vict, SKILL_KICK, 0);
       break;
     case 3:
       do_generic_skill(ch, vict, SKILL_STUN, 0);
       WAIT_STATE(vict, PULSE_VIOLENCE * 2);
       break;
     case 4:
        do_generic_knock_down(ch, vict, SKILL_BODYSLAM);
        break;
     } // warrior
     break;
    case 2:
     switch(warrior) {
       case 1:
        do_generic_skill(ch, vict, SKILL_STUN, 0);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 2:
        do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, 0);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 3:
        do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, 0);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 4:
        do_mob_disarm(ch, vict);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
     } // warrior
     break;
    case 3:
     switch(warrior) {
      case 1:
        do_generic_skill(ch, vict, SKILL_BEARHUG, 0);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 2:
        do_generic_knock_down(ch, vict, SKILL_BODYSLAM);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 3:
        do_generic_skill(ch, vict, SKILL_DEATHBLOW, GET_LEVEL(ch) * 3);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 4:
        do_mob_bash(ch, vict);
        WAIT_STATE(vict, PULSE_VIOLENCE * 3);
        break;
      } // warrior
      break;
    case 4:
     switch(warrior) {
      case 1:
        do_generic_skill(ch, vict, SKILL_STUN, 0);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 2:
        do_generic_skill(ch, vict, SKILL_DEATHBLOW, GET_LEVEL(ch) * 3);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 3:
        do_generic_skill(ch, vict, SKILL_DEATHBLOW, GET_LEVEL(ch) * 3);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        break;
      case 4:
        do_mob_bash(ch, vict);
        WAIT_STATE(vict, PULSE_VIOLENCE * 3);
        break;
     } // warrior
     break;

   default:
    break;
  } // wstyle

  if(warrior == 4) {
    warrior = 0;

    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) || MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN))
    {
      wstyle = number(3, 4);
    }
    else { 
      wstyle = number(1, 4);
    }
  }
  warrior++;

}

void giant_combat(struct char_data *ch, struct char_data *vict)
{
    

  switch (number(1, 4)) {

      case 1:
	do_mob_bash(ch, vict);
	break;
      case 2:
	do_generic_skill(ch, vict, SKILL_STOMP, 0);
        break;
      case 3:
	do_generic_skill(ch, vict, SKILL_SWAT, 0);
        break;
      case 4:
	do_generic_skill(ch, vict, SKILL_SWAT, 0);
        break;
       default:
	break;
      }    
}

void monk_combat(struct char_data *ch, struct char_data *vict)
{
        switch (number(1, 7)) {
         
         case 1:
	  do_generic_knock_down(ch, vict, SKILL_KICKFLIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
          break;
        case 2:
	  do_generic_knock_down(ch, vict, SKILL_TACKLE);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
          break;
        case 3:
          do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, 0);
          break;
        case 4:
          do_generic_skill(ch, vict, SKILL_CHOP, 0);
          break;
        case 5:
          do_generic_knock_down(ch, vict, SKILL_TRIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
          break;
        case 6:
          do_generic_skill(ch, vict, SKILL_KAMIKAZE, 0);
	  break;
        case 7:
          do_generic_skill(ch, vict, SKILL_PRESSURE, 0);
          break;
        default:
          break;
       }
} 

static int thief = 1;

void thief_combat(struct char_data *ch, struct char_data *vict)
{
        int hit_roll, to_hit, tstyle = 1;
        struct affected_type af[2];

	hit_roll = number (1,100) + GET_STR(ch);
	to_hit = (100 - (int) (100*GET_LEVEL(ch)/250));


        switch(tstyle) {

          case 1:
           switch(thief) {
             case 1:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 2);
             break;
             case 2:
	       do_generic_skill(ch, vict, SKILL_BRAIN, 0);
	     break;
	     case 3:
	       do_generic_skill(ch, vict, SKILL_KNEE, 0);
	     break;
	     case 4:
	       do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, GET_LEVEL(ch) * 1.5);
	     break;
           }
          break;


          case 2:
           switch(thief) {
             case 1:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 2);
             break;
             case 2:
	       do_generic_skill(ch, vict, SKILL_TRICKPUNCH, 0);
	     break;
	     case 3:
	       do_generic_skill(ch, vict, SKILL_GROINKICK, 0);
	     break;
	     case 4:
	       do_generic_skill(ch, vict, SKILL_LOWBLOW, 0);
	     break;
           }
          break;

          case 3:
           switch(thief) {
             case 1:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 2);
             break;
             case 2:
	       do_generic_skill(ch, vict, SKILL_TRICKPUNCH, 0);
	     break;
	     case 3:
	       do_generic_skill(ch, vict, SKILL_KNEE, 0);
	     break;
	     case 4:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 2);
	     break;
           }
          break;


         // Blind my victim       
          case 4:
           switch(thief) {
             case 1:
              if(IS_AFFECTED(vict, AFF_BLIND)) {
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 2);
                break;
              }
              else {
                if(GET_EQ(vict, WEAR_EYES)) { to_hit -= 20; }
	        if ((GET_LEVEL(ch) > 10) && (hit_roll < to_hit)) {  
	          damage(ch, vict, GET_LEVEL(ch), SKILL_EYEGOUGE, ABT_SKILL);
		  af[0].location = APPLY_HITROLL;
		  af[0].modifier = -4;
		  af[0].duration = 2;
		  af[0].bitvector = AFF_BLIND;
		
		  af[1].location = APPLY_AC;
		  af[1].modifier = 40;
		  af[1].duration = 2;
		  af[1].bitvector = AFF_BLIND;
		
		  affect_join(vict, af, FALSE, FALSE, FALSE, FALSE);
		  affect_join(vict, af+1, FALSE, FALSE, FALSE, FALSE);
	        } else {
	          damage(ch, vict, 0, SKILL_EYEGOUGE, ABT_SKILL);
	        }

                GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 2;
                break;
              }
             break;
             case 2:
	      do_generic_skill(ch, vict, SKILL_TRIP, GET_LEVEL(ch) * 1.5);
             break;
             case 3:
	      do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, GET_LEVEL(ch) * 1.5);
             break;
             case 4:
	      do_generic_skill(ch, vict, SKILL_TRIP, GET_LEVEL(ch) * 1.5);
             break;

           }
          break;
          // I'm hurt, lets keep my victim down as much as possible.
          case 5:
           switch(thief) {
             case 1:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
                WAIT_STATE(vict, PULSE_VIOLENCE * 4);
             break;
             case 2:
	      do_generic_skill(ch, vict, SKILL_GROINKICK, GET_LEVEL(ch) * 1.5);
	     break;
	     case 3:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 4);
	     break;
	     case 4:
	        do_generic_knock_down(ch, vict, SKILL_TRIP);
        WAIT_STATE(vict, PULSE_VIOLENCE * 4);
	     break;
           }
          break;
      default:
        break;
     } // tstyle

  if(thief == 4) {
    thief = 0;

    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE) || MOB_FLAGGED(ch, MOB_AGGR_TO_ALIGN))
    {
      tstyle = number(3, 5);
    }
    else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 3) {
      tstyle = 5;
    }
    else { 
      tstyle = number(1, 5);
    }
  }
  thief++;


}

void pal_combat(struct char_data *ch, struct char_data *vict)
{

        switch(number(1, 3)) {

	  case 1:
	   do_mob_bash(ch, vict);
        WAIT_STATE(vict, PULSE_VIOLENCE * 3);
           break;
          case 2:
           do_generic_skill(ch, vict, SKILL_UPPERCUT, 0);
           break;
          case 3:
           do_generic_skill(ch, vict, SKILL_KICK, 0);
           break;
          default:
           break;
         }
}

void citizen_combat(struct char_data *ch, struct char_data *vict)
{

  int zone;
  zone = IN_ROOM(ch);

  if ((zone != 0) && (zone != NOWHERE))
    zone = world[zone].zone;
      switch(number(0, 10)) {
       case 0:
  sprintf(buf, "%s shouts, 'Guards! Guards! %s is attacking me!'\r\n", GET_NAME(ch), GET_NAME(vict));
        send_to_zone_outdoor(zone, buf);
        break;
      case 5:
        mobsay(ch, "Take this you vile bandit!");
        do_generic_skill(ch, vict, SKILL_GROINKICK, 0);
        do_flee(ch, "", 0, SCMD_FLEE); 
        break;
   }   

}

void cleric_spells(struct char_data *ch, struct char_data *vict)
{

   switch(number((GET_LEVEL(ch) >> 1), GET_LEVEL(ch))) {
    case 1: case 2: case 3: case 4: case 5:
    cast_spell(ch, ch, NULL, PRAY_CURE_LIGHT, ABT_PRAYER, FALSE);
    break;
  case 6: case 7:
    cast_spell(ch, vict, NULL, SPELL_DUMBNESS, ABT_SPELL, FALSE);
    break;
  case 8: case 9:
    cast_spell(ch, ch, NULL, PRAY_CURE_CRITICAL, ABT_PRAYER, FALSE);
    break;
  case 10: case 11:
    cast_spell(ch, vict, NULL, SPELL_DUMBNESS, ABT_SPELL, FALSE);
    break;
  case 12:
    cast_spell(ch, vict, NULL, PRAY_CURE_LIGHT, ABT_PRAYER, FALSE);
    break;
  case 27:
    cast_spell(ch, vict, NULL, PRAY_CALL_LIGHTNING, ABT_PRAYER, FALSE);
    break;
  case 33:
    if (IS_EVIL(ch))
      cast_spell(ch, vict, NULL, PRAY_DISPEL_GOOD, ABT_PRAYER, FALSE);
    else if (IS_GOOD(ch))
      cast_spell(ch, vict, NULL, PRAY_DISPEL_EVIL, ABT_PRAYER, FALSE);
    else
      cast_spell(ch, vict, NULL, PRAY_CALL_LIGHTNING, ABT_PRAYER, FALSE);
    break;
  case 35:
    cast_spell(ch, vict, NULL, PRAY_HARM, ABT_PRAYER, FALSE);
    break;
  case 47:
    cast_spell(ch, vict, NULL, PRAY_ELEMENTAL_BURST, ABT_PRAYER, FALSE);
    break;
  case 60:
    cast_spell(ch, vict, NULL, PRAY_WINDS_PAIN, ABT_PRAYER, FALSE);
    break;
  case 77:
    cast_spell(ch, vict, NULL, PRAY_HAND_BALANCE, ABT_PRAYER, FALSE);
    break;
  case 90:
    cast_spell(ch, vict, NULL, PRAY_SPIRIT_STRIKE, ABT_PRAYER, FALSE);
    break;
  case 100:
    cast_spell(ch, vict, NULL, PRAY_ABOMINATION, ABT_PRAYER, FALSE);
    break;
  case 110:
    cast_spell(ch, vict, NULL, PRAY_WINDS_RECKONING, ABT_PRAYER, FALSE);
    break;
  case 120:
    cast_spell(ch, ch, NULL, PRAY_HEAL, ABT_PRAYER, FALSE);
    break;
  case 130:
    cast_spell(ch, vict, NULL, PRAY_ANGEL_BREATH, ABT_PRAYER, FALSE);
    break;
  case 135:
 //   cast_spell(ch, vict, NULL, PRAY_WEAKENED_FLESH, ABT_PRAYER, FALSE);
    cast_spell(ch, vict, NULL, PRAY_ANGEL_BREATH, ABT_PRAYER, FALSE);
    break;
  case 140:
    cast_spell(ch, vict, NULL, PRAY_PLAGUE, ABT_PRAYER, FALSE);
    break;
  case 150:
    cast_spell(ch, vict, NULL, PRAY_SOUL_SCOURGE, ABT_PRAYER, FALSE);
    break;
  default:
    if(!number(0, 3)) {
      cast_spell(ch, ch, NULL, PRAY_CURE_LIGHT, ABT_PRAYER, FALSE);
    }
    break;
  }
}
void cleric_combat(struct char_data *ch, struct char_data *vict)
{
   int cstyle = 1;

   if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4) {
     cstyle = number(2, 3);
   }
   else {
     cstyle = number(1, 2);
   }

   switch(cstyle) {
     case 1:
      cleric_spells(ch, vict);
      break;
     case 2:
      switch(number(1, 2)) {
       case 1:
        if(GET_LEVEL(ch) > 28) {
         cast_spell(ch, vict, NULL, PRAY_BLINDNESS, ABT_PRAYER, FALSE);
         
        }
        
        else { cleric_spells(ch, vict); }

       break;
       case 2:
        if(GET_LEVEL(ch) > 128) {
         cast_spell(ch, ch, NULL, PRAY_WEAKENED_FLESH, ABT_PRAYER, FALSE);
         
        }
        
        else { cleric_spells(ch, vict); }

       break;
      }    
     break;
     case 3:
       if(GET_LEVEL(ch) < 11) {
         cast_spell(ch, ch, NULL, PRAY_CURE_LIGHT, ABT_PRAYER, FALSE);
         break;
       }
       else if(GET_LEVEL(ch) > 11) {
         cast_spell(ch, ch, NULL, PRAY_CURE_CRITICAL, ABT_PRAYER, FALSE);
         break;
       }  
       else {
         cast_spell(ch, ch, NULL, PRAY_HEAL, ABT_PRAYER, FALSE);
         break;
       }
      break;
     default:
      break;
   } // cstyle 

}

void dragon_combat(struct char_data *ch, struct char_data *vict)
{

   switch(number(1, 5)) {

    case 1:
     cast_spell(ch, vict, NULL, SPELL_FIRE_BREATH, ABT_SPELL, FALSE);
     break;
    case 2:
     cast_spell(ch, vict, NULL, SPELL_GAS_BREATH, ABT_SPELL, FALSE);
     break;
    case 3:
     cast_spell(ch, vict, NULL, SPELL_FROST_BREATH, ABT_SPELL, FALSE);
     break;
    case 4:
     cast_spell(ch, vict, NULL, SPELL_ACID_BREATH, ABT_SPELL, FALSE);
     break;
    case 5:
     cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BREATH, ABT_SPELL, FALSE);
     break;
    default:
     break;
   }

}

void advanced_mobile_combat(struct char_data *ch, struct char_data *vict)
{


   switch(GET_RACE(ch)) {
    case RACE_NPC_GIANT:
     giant_combat(ch, vict);
     break;
    case RACE_NPC_DRAGON:
     dragon_combat(ch, vict);
     break;
    default:
     break;
}
   switch(GET_CLASS(ch)) {

   case CLASS_NPC_WARRIOR:
    warrior_combat(ch, vict);
   break;
   break;
   case CLASS_NPC_MONK:
     monk_combat(ch, vict);
   break;
   case CLASS_NPC_THIEF:
     thief_combat(ch, vict);
   break;
   case CLASS_NPC_PALADIN:
     pal_combat(ch, vict);
   break;
   case CLASS_NPC_CITIZEN:
     citizen_combat(ch, vict);
   break;
   case CLASS_NPC_CLERIC:
     cleric_combat(ch, vict);
     GET_SKILL_WAIT(ch) = PULSE_VIOLENCE * 3;
   break;
   default:
   break;
   }
}
