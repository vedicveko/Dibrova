/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
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

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int pk_allowed;
extern char *dirs[];
extern int newbie_level;
extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
void clearMemory(struct char_data * ch);


/* extern functions */
void raw_kill(struct char_data * ch, struct char_data * killer);
void check_killer(struct char_data * ch, struct char_data * vict);
extern void send_to_arena(char *);
room_rnum find_target_room(struct char_data *ch, char *rawroomstr);
int search_block(char *arg, char **list, int exact);
int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim);
// void write_event(char *event);

/* Acmd's */
ACMD(do_fog);


ACMD(do_fog)
{
	struct char_data *vict = NULL;
	int chance = 0, roll = 0;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	struct affected_type af;
	two_arguments(argument, arg1, arg2);
	if (!is_class(ch, CLASS_DRUID))
	{
		send_to_char("Huh?!\r\n", ch);
		return;
	}
	if (GET_SKILL(ch, SKILL_FOG) < 1)
	{
		send_to_char("Huh!?\r\n", ch);
		return;
	}
	if (!*arg1)
	{
		send_to_char("You can use 'Fog' for the following purposes:\r\n", ch);
		if (GET_SKILL(ch, SKILL_FOG_MIND) > 0)
			send_to_char("				- Mind      : Makes mobiles forget.\r\n", ch);
		if (GET_SKILL(ch, SKILL_FOG_MIST_AURA) > 0)
			send_to_char("				- Mist Aura : Become surrounded by fog.\r\n", ch);
	} else {
		if (is_abbrev(arg1, "mind"))
		{
			if (GET_SKILL(ch, SKILL_FOG_MIND) > 0)
			{
				if (FIGHTING(ch))
					vict = FIGHTING(ch);
				else {
					if (!*arg2)
					{
						send_to_char("A hazy ball of fog veers aimlessly about the room before dissipating!\r\n", ch);
						return;
					}
					else if (!(vict = get_char_room_vis(ch, arg2)))
					{
						send_to_char("A hazy ball of fog veers aimlessly about the room before dissipating!\r\n", ch);
						return;
					}
				}
				if (!IS_NPC(vict))
				{
					send_to_char("Not on players!\r\n", ch);
					return;
				}
				if (GET_VIM(ch) < 750)
				{
					send_to_char("You do not have enough energy!\r\n", ch);
					return;
				}
				chance = SKILL_FOG_MIND;
				roll   = number(1, 101);
				if (chance > roll)
				{
					MEMORY(ch) = NULL;
					clearMemory(ch);
					forget(ch, vict);
					forget(vict, ch);
					MEMORY(vict) = NULL;
					clearMemory(vict);
					HUNTING(ch) = 0;
					act("A hazy ball of fog hurls towards $N's head, enveloping it's mind!", TRUE, ch, 0, vict, TO_CHAR);
					act("A hazy ball of fog hurls towards $N's head, enveloping it's mind!", TRUE, ch, 0, vict, TO_ROOM);
					GET_VIM(ch) -= 750;
				} else {
					send_to_char("A hazy ball forms and then suddenly dissipates out of control!\r\n", ch);
					return;
				}
			} else {
				send_to_char("You do not know how to do that.\r\n", ch);
				return;
			}
		}
		else if (is_abbrev(arg1, "mist aura"))
		{
			chance = SKILL_FOG_MIST_AURA;
			roll   = number(1, 101);
			if (GET_VIM(ch) < 100)
			{
				send_to_char("You do not have enough energy to do that!\r\n", ch);
				return;
			}
			if (chance > roll)
			{
				af.type = SKILL_FOG_MIST_AURA;
				af.duration = GET_LEVEL(ch);
				af.modifier = 0;
				af.location = APPLY_NONE;
				af.bitvector = AFF_FOG_MIST_AURA;
				affect_to_char(ch, &af);
				send_to_char("A rolling mist sweeps into the room and blankets you from view.\r\n", ch);
				act("A rolling mist sweeps into the room and blankets $n from view", TRUE, ch, 0, 0, TO_ROOM);
				GET_VIM(ch) -= 100;
				return;
			} else {
				send_to_char("You failed.\r\n", ch);
				return;
			}
		} else {
			send_to_char("No such option to fog.\r\n", ch);
			return;
		}
	}
}

ACMD(do_assist)
{
  struct char_data *helpee, *opponent;

  if (FIGHTING(ch)) {
    send_to_char("You're already fighting!  How can you assist someone else?\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Whom do you wish to assist?\r\n", ch);
  else if (!(helpee = get_char_room_vis(ch, arg)))
    send_to_char(NOPERSON, ch);
  else if (helpee == ch)
    send_to_char("You can't help yourself any more than this!\r\n", ch);
  else {
    for (opponent = world[ch->in_room].people;
	 opponent && (FIGHTING(opponent) != helpee);
	 opponent = opponent->next_in_room)
		;

    if (!opponent)
      act("But nobody is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
      act("You can't see who is fighting $M!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!pk_allowed && !IS_NPC(opponent) && !pkill_ok(ch, opponent)) /* prevent accidental pkilling */
      act("Use 'murder' if you really want to attack $N.", FALSE,
	  ch, 0, opponent, TO_CHAR);
    else {
      send_to_char("You join the fight!\r\n", ch);
      act("$N assists you!", 0, helpee, 0, ch, TO_CHAR);
      act("$n assists $N.", FALSE, ch, 0, helpee, TO_NOTVICT);
      hit(ch, opponent, TYPE_UNDEFINED);
    }
  }
}

/* Warn newbies about attacking mobs that will kill them.
   return TRUE if the hit should happen, or FALSE if we
   want to prevent the violence.                          */
int warn_newbie(struct char_data *ch, struct char_data *vict) {

  if (GET_LEVEL(ch) > newbie_level || GET_CLASS(ch) >= CLASS_VAMPIRE)
    return TRUE; /* He's a big boy now. */

  if (GET_LEVEL(ch) > GET_LEVEL(vict)-5)
    return TRUE; /* At least it won't be a slaughter. */

  if (ch->char_specials.warned == vict) {
    send_to_char("Brave men die too...\r\n", ch);
    return TRUE; /* He's already been warned.  Maybe he'll listen next time. */
  }

  /* The guy needs the warning. */
  act("Attacking $N is a VERY bad idea.  $E is much stronger than you.  "
    "Perhaps a warthog would be more your speed.", FALSE, ch, 0, vict, TO_CHAR);
  ch->char_specials.warned = vict;
  return FALSE;

}

ACMD(do_hit)
{
  struct char_data *vict;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Hit who?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, arg)))
    send_to_char("They don't seem to be here.\r\n", ch);
  else if (vict == ch) {
    send_to_char("You hit yourself...OUCH!.\r\n", ch);
    act("$n hits $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
  } else if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(vict))
    act("$N is just such a good friend, you simply can't hit $M.", FALSE, ch, 0, vict, TO_CHAR);
  else {
    if (!pk_allowed && !pkill_ok(ch, vict)) {
      if (!IS_NPC(vict) && !IS_NPC(ch)) {
      	if (subcmd != SCMD_MURDER) {
      	  send_to_char("Use 'murder' to hit another player.\r\n", ch);
	        return;
	      } else {
	          if (!pk_allowed && !pkill_ok(ch, vict)) {
              send_to_char("You are forbidden to attack that player.\r\n", ch);
              return;
            }
            /* check_killer(ch, vict); */
	      }
      }
      if (IS_AFFECTED(ch, AFF_CHARM) && !IS_NPC(ch->master) && !IS_NPC(vict))
      	return;	/* you can't order a charmed pet to attack a player */
    }
    if ((GET_POS(ch) == POS_STANDING) && (vict != FIGHTING(ch))) {
      if (warn_newbie(ch, vict)) {
        hit(ch, vict, TYPE_UNDEFINED);
        WAIT_STATE(ch, PULSE_VIOLENCE + 2);
        sprintf(buf, "%s has attacked %s!\r\n", GET_NAME(ch), GET_NAME(vict));
    //    write_event(buf);
      }
    } else
      send_to_char("You do the best you can!\r\n", ch);
  }
}



ACMD(do_kill)
{
  struct char_data *vict;

  if ((GET_LEVEL(ch) < LVL_IMPL) || IS_NPC(ch)) {
    do_hit(ch, argument, cmd, subcmd);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Kill who?\r\n", ch);
  } else {
    if (!(vict = get_char_room_vis(ch, arg)))
      send_to_char("They aren't here.\r\n", ch);
    else if (ch == vict)
      send_to_char("Your mother would be so sad.. :(\r\n", ch);
    else {
      act("You chop $M to pieces!  Ah!  The blood!", FALSE, ch, 0, vict, TO_CHAR);
      act("$N chops you to pieces!", FALSE, vict, 0, ch, TO_CHAR);
      act("$n brutally slays $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      raw_kill(vict, ch);
    }
  }
}



ACMD(do_backstab)
{
  struct char_data *vict;
  int percent, prob, apr;
//  bool failed = FALSE;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_BACKSTAB)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Backstab who?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you sneak up on yourself?\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != (TYPE_PIERCE - TYPE_HIT)) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != (TYPE_STAB - TYPE_HIT)))
  {
    send_to_char("Only piercing and stabbing weapons can be used for backstabbing.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't backstab a fighting person -- they're too alert!\r\n", ch);
    return;
  }
  
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to backstab another day. */

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
     act("You notice $N lunging at you!", FALSE, vict, 0, ch, TO_CHAR);
     act("$e notices you lunging at $m!", FALSE, vict, 0, ch, TO_VICT);
     act("$n notices $N lunging at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
     hit(vict, ch, TYPE_UNDEFINED);
     return;
   }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BACKSTAB);
  if(IS_AFFECTED(ch, AFF_HIDE)) { percent = 50; }
  if(IS_AFFECTED(vict, AFF_AWARE)) {percent = 101; }
  /* NEW! */
  apr = 0;

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_BACKSTAB, ABT_SKILL);
    improve_abil(ch, SKILL_BACKSTAB, ABT_SKILL);
  }
  else {
    if (GET_SKILL(ch, SKILL_SECOND_BSTAB) >= number(1, 150)) {
      if (abil_info[ABT_SKILL][SKILL_SECOND_BSTAB].min_level[(int)GET_CLASS(ch)] > GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
        apr = apr;
      } else
      apr++;  
      if (GET_SKILL(ch, SKILL_THIRD_BSTAB) >= number(1, 150)){
        if (abil_info[ABT_SKILL][SKILL_THIRD_BSTAB].min_level[(int)GET_CLASS(ch)] > GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
        apr = apr;
      } else
        apr++;
        if (GET_SKILL(ch, SKILL_FOURTH_BSTAB) >= number(1, 150)){
          if (abil_info[ABT_SKILL][SKILL_FOURTH_BSTAB].min_level[(int)GET_CLASS(ch)] > GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
        apr = apr;
      } else
  	  apr++;
        }
      }
    }

    apr = MAX(-1, MIN(apr, 6));
    if ((apr >= 0)) {
      for (; apr >= 0 && (percent < prob); apr--)
        hit(ch, vict, SKILL_BACKSTAB);
        if (number(0, 28) < GET_DEX(ch)) {
          if (ch->in_room != vict->in_room)
            return;
          else {
            act("You twist your dagger in $n's back, that had to hurt!", FALSE, vict, 0, ch, TO_VICT);
            act("$N twists $S dagger in your back, /cRThat REALLY hurt!/c0", FALSE, vict, 0, ch, TO_CHAR);
            act("$N twists $S dagger in $n's back, that had to hurt!", FALSE, vict, 0, ch, TO_NOTVICT);
            damage(ch, vict, GET_LEVEL(ch) * 3, SKILL_FLAIL, ABT_SKILL);
         }
       }
     } 
   }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_order)
{
  char name[100], message[256];
  char buf[256];
  bool found = FALSE;
  int org_room;
  struct char_data *vict;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message)
    send_to_char("Order who to do what?\r\n", ch);
  else if (!(vict = get_char_room_vis(ch, name)) && !is_abbrev(name, "followers"))
    send_to_char("That person isn't here.\r\n", ch);
  else if (ch == vict)
    send_to_char("You obviously suffer from skitzofrenia.\r\n", ch);

  else {
    if (IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not approve of you giving orders.\r\n", ch);
      return;
    }


    if (vict) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, vict, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, vict, TO_ROOM);

      if ((vict->master != ch) || !IS_AFFECTED(vict, AFF_CHARM))
      	act("$n has an indifferent look.", FALSE, vict, 0, 0, TO_ROOM);
      else {
      	send_to_char(OK, ch);
      	command_interpreter(vict, message);
      }
    } else {			/* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, vict, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
	if (org_room == k->follower->in_room)
	  if (IS_AFFECTED(k->follower, AFF_CHARM)) {
	    found = TRUE;
	    command_interpreter(k->follower, message);
	  }
      }
      if (found)
      	send_to_char(OK, ch);
      else
      	send_to_char("Nobody here is a loyal subject of yours!\r\n", ch);
    }
  }
}



ACMD(do_flee)
{
  int i, attempt, loss;
  char retmsg[512];
  char armsg[512];
  struct char_data *v;

  armsg[0] = '\0';

  if (GET_CLASS(ch) == CLASS_VAMPIRE) {
    send_to_char("Flee from a fight? A vampire? Never.\r\n", ch);
    return;
  }

  if (GET_POS(ch) < POS_FIGHTING) {
    send_to_char("You are not in a position to flee! Get up!\r\n", ch);
    return;
  }

  if (GET_POS(ch) == POS_RESTING) {
    send_to_char("You'll have to stand up first...\r\n", ch);
    return;
  }

  if (GET_POS(ch) == POS_SITTING) {
    send_to_char("You'll have to stand up first...\r\n", ch);
    return;
  }

  if (!FIGHTING(ch)){
    send_to_char("But you're not fighting anyone!\r\n", ch);
    return;
  }

  if (GET_MOVE(ch) <= 0) {
    send_to_char("You have no moves left! You can't flee!\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BERSERK))
  {
	send_to_char( "Your berserker rage prevents you from spotting an avenue of escape!\r\n", ch );
	return;
  }

  if (IS_AFFECTED(ch, AFF_SEALED)) {
    send_to_char("You cannot flee! You are magically sealed to this room!\r\n", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_MESMERIZED)) {
    send_to_char("You cannot flee! You are magically sealed to this room!\r\n", ch);
    return;
  }


  v = FIGHTING(ch);

  if (IS_NPC(ch) && GET_MOB_WAIT(ch)) {
    send_to_char("You cannot flee!\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) == CLASS_TITAN) {
    if (number(1, 10) >= 6) {
      send_to_char("You attempt to flee, but your large girth prevents it!\r\n"
                   "You may be trapped!\r\n", ch);
      return;
    }
  }

  for (i = 0; i < 6; i++) {
    attempt = number(0, NUM_OF_DIRS - 1);	/* Select a random direction */
    if (CAN_GO(ch, attempt) &&
	!IS_SET_AR(ROOM_FLAGS(EXIT(ch, attempt)->to_room), ROOM_DEATH)) {
      if (subcmd == SCMD_FLEE) {
        act("$n panics, and attempts to flee!", TRUE, ch, 0, 0, TO_ROOM);
        if (FIGHTING(ch)) {
          if (!IS_NPC(ch) && !ROOM_FLAGGED(ch->in_room, ROOM_ARENA)) {
            // VEDIC Changed to prevent a bug. See the history
            loss = GET_LEVEL(ch) * GET_LEVEL(FIGHTING(ch));
            if(loss < GET_EXP(ch)) {
              gain_exp(ch, -loss);
            }
          }
        }
        if (FIGHTING(FIGHTING(ch)) == ch)
          stop_fighting(FIGHTING(ch));
        stop_fighting(ch);
      } else
        act("$n tucks tail and slithers out of battle, realizing that $e "
            "may be in over $s head", TRUE, ch, 0, 0, TO_ROOM);
      if (do_simple_move(ch, attempt, TRUE)) {
        if (subcmd == SCMD_FLEE) {
          send_to_char("You flee head over heels.\r\n", ch);
          sprintf(armsg, "%s flees from %s!\r\n", GET_NAME(ch), GET_NAME(v));
        } else {
          sprintf(retmsg, "You decide you'd better not take any chances and "
                "slink your way out of this one.  Better to live and be "
                "ashamed than die and be dead.\r\nYou retreat %s.\r\n",
                dirs[attempt]);
          send_to_char(retmsg, ch);
          sprintf(armsg,"%s retreats from %s!\r\n", GET_NAME(ch), GET_NAME(v));
        }
      } else {
        if (subcmd == SCMD_FLEE)
          act("$n tries to flee, but can't!", TRUE, ch, 0, 0, TO_ROOM);
        else
          act("$n tries to find a way out of the fight, but $e is blocked "
              "from every direction!", TRUE, ch, 0, 0, TO_ROOM);
      }
      if (*armsg && ROOM_FLAGGED(ch->in_room, ROOM_ARENA) &&
          ROOM_FLAGGED(v->in_room, ROOM_ARENA)) {
        send_to_arena(armsg);
      }
      return;
    }
  }
  if (subcmd == SCMD_FLEE)
    send_to_char("PANIC!  You couldn't escape!\r\n", ch);
  else
    send_to_char("You can't find any way out of the fight!  You are "
                 "trapped!\r\n", ch);
}


ACMD(do_bash)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);


  if (!GET_SKILL(ch, SKILL_BASH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("Bash from a mount... hm... interesting concept.", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to bash another day. */

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_BASH);

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (IS_AFFECTED(vict, AFF_BALANCE))
    percent = 101;

  if (GET_RACE(ch) != RACE_GIANT) {
    if (IS_NPC(vict) && GET_RACE(vict) == RACE_NPC_GIANT) 
      percent = 101;
  }

  if (GET_POS(vict) <= POS_SITTING) {
    send_to_char("You realize the futility of trying to bash someone who is already down.\r\n", ch);
    send_to_char("You go flying past your victim and land in a heap.\r\n", ch);
    percent = 101;
  }


  if (percent > prob && can_bash(ch, vict) != FALSE) {
    GET_POS(ch) = POS_SITTING;
    damage(ch, vict, 0, SKILL_BASH, ABT_SKILL);
    improve_abil(ch, SKILL_BASH, ABT_SKILL);
  } else {
      GET_POS(vict) = POS_SITTING;
      WAIT_STATE(vict, PULSE_VIOLENCE);
      damage(ch, vict, 1, SKILL_BASH, ABT_SKILL);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_rescue)
{
  struct char_data *vict, *tmp_ch;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_RESCUE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Whom do you want to rescue?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("What about fleeing instead?\r\n", ch);
    return;
  }
  if (FIGHTING(ch) == vict) {
    send_to_char("How can you rescue someone you are trying to kill?\r\n", ch);
    return;
  }
  for (tmp_ch = world[ch->in_room].people; tmp_ch &&
       (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

  if (!tmp_ch) {
    act("But nobody is fighting $M!", FALSE, ch, 0, vict, TO_CHAR);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_RESCUE);

  if (percent > prob) {
    send_to_char("You fail the rescue!\r\n", ch);
    improve_abil(ch, SKILL_RESCUE, ABT_SKILL);
    return;
  }
  send_to_char("Banzai!  To the rescue...\r\n", ch);
  act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
  act("$n heroically rescues $N!", FALSE, ch, 0, vict, TO_NOTVICT);

  if (FIGHTING(vict) == tmp_ch)
    stop_fighting(vict);
  if (FIGHTING(tmp_ch))
    stop_fighting(tmp_ch);
  if (FIGHTING(ch))
    stop_fighting(ch);

  set_fighting(ch, tmp_ch);
  set_fighting(tmp_ch, ch);

  WAIT_STATE(vict, 2 * PULSE_VIOLENCE);

}



ACMD(do_kick)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_KICK)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to kick another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_KICK);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KICK, ABT_SKILL);
    improve_abil(ch, SKILL_KICK, ABT_SKILL);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_KICK, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

}
 

ACMD(do_deathblow)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_DEATHBLOW)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Deathblow who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  percent = ((10 - (GET_DEX(vict) / 3)) << 1) + number(1, 101);	/* 101% is a complete
								 * failure */
  prob = GET_SKILL(ch, SKILL_DEATHBLOW);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_DEATHBLOW, ABT_SKILL);
    improve_abil(ch, SKILL_DEATHBLOW, ABT_SKILL);
  } else
    damage(ch, vict, 2*(GET_LEVEL(ch)) >> 1, SKILL_DEATHBLOW, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_circle)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_CIRCLE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Circle who?\r\n", ch);
      return;
    }
  }

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != (TYPE_PIERCE - TYPE_HIT)) && (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != (TYPE_STAB - TYPE_HIT)))
  {
    send_to_char("Only piercing and stabbing weapons can be used for circle.\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_CIRCLE);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_CIRCLE, ABT_SKILL);
    improve_abil(ch, SKILL_CIRCLE, ABT_SKILL);
  } else {
    damage(ch, vict, MIN(2*GET_LEVEL(ch), 70), SKILL_CIRCLE, ABT_SKILL);

    if (number(0,30) < GET_DEX(ch)) {
      if (ch->in_room != vict->in_room)
        return;
      else {
        act("You twist your dagger in $n's back, that had to hurt!", FALSE, vict, 0, ch, TO_VICT);
        act("$N twists $S dagger in your back, /cRThat REALLY hurt!/c0", FALSE, vict, 0, ch, TO_CHAR);
        act("$N twists $S dagger in $n's back, that had to hurt!", FALSE, vict, 0, ch, TO_NOTVICT);
        damage(ch, vict, GET_LEVEL(ch), SKILL_FLAIL, ABT_SKILL);
     }
   }
}

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}
 

ACMD(do_stun) {
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_STUN)) {
    send_to_char("You do not know this skill.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Stun who?\r\n", ch);
      return;
    }
  }

  if (!IS_NPC(vict) && !pkill_ok(ch, vict)) {
    send_to_char("You can not attack another player here.\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("You stun yourself...OUCH!.\r\n", ch);
    act("$n tries to stun $mself, and says OUCH!", FALSE, ch, 0, vict, TO_ROOM);
    return;
  }

  if (!(GET_MOVE(ch) > 10)) {
    send_to_char("You are too tired to stun.\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_POS(vict) <= POS_SITTING) {
    if (GET_SKILL(ch, SKILL_STUN) <= number(50, 120)) {
      act("$N sees your stun coming and rolls away, jumping to $S feet!",
           TRUE, ch, 0, vict, TO_CHAR);
      act("You see $n's stun coming and roll away, jumping to your feet!",
           TRUE, ch, 0, 0, TO_VICT);
      act("$N sees $n's stun coming and rolls away, jumping to $S feet!",
           TRUE, ch, 0, vict, TO_ROOM);
      GET_POS(vict) = POS_STANDING;
      WAIT_STATE(ch, PULSE_VIOLENCE * 3);
      return;
    }
  }

  percent = number(68, 83) + GET_DEX(vict);
  prob = GET_SKILL(ch, SKILL_STUN);

  if (IS_AFFECTED(vict, AFF_AWARE))
    percent = 101; 

  if (percent < prob && can_bash(ch, vict) != FALSE) {
    WAIT_STATE(vict, PULSE_VIOLENCE*2);
    damage(ch, vict, GET_LEVEL(ch) + number(1, 10), SKILL_STUN, ABT_SKILL);
    GET_POS(vict) = POS_SITTING;
  } else {
    send_to_char("You make an attempt at stunning, but fail.\r\n", ch);
    damage(ch, vict, 0, SKILL_STUN, ABT_SKILL);
    improve_abil(ch, SKILL_STUN, ABT_SKILL);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE*3);
  GET_MOVE(ch) -= 10;
}


ACMD(do_eyegouge)
{

  struct affected_type af[2];
  struct char_data *vict;
  int percent;
  int prob;
  int skilltype;

  percent = (number(1, 101));	/* 101% is a complete failure */

  if (GET_CLASS(ch)==CLASS_THIEF || GET_CLASS(ch) == CLASS_SHADOWMAGE || GET_CLASS(ch) == CLASS_MERCENARY)
    skilltype = SKILL_EYEGOUGE;
  else if (GET_CLASS(ch)==CLASS_DRUID || GET_CLASS(ch)==CLASS_MONK \
        || GET_CLASS(ch)==CLASS_SAMURAI || GET_CLASS(ch)==CLASS_SHAMAN \
        || GET_CLASS(ch)==CLASS_STORR || GET_CLASS(ch)==CLASS_PSIONIST \
        || GET_CLASS(ch)==CLASS_COMBAT_MASTER || GET_CLASS(ch)==CLASS_ACROBAT \
        || GET_CLASS(ch)==CLASS_BEASTMASTER \
        || GET_CLASS(ch)>=CLASS_VAMPIRE || GET_LEVEL(ch) >= LVL_IMMORT)
    skilltype = SKILL_DIRTTHROW;
  else {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  prob = GET_SKILL(ch, skilltype);

  if (!prob) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Attack who?\r\n", ch);
      return;
    }
  }

  if (vict == ch) {
    send_to_char("Silly you...\r\n", ch);
    return;
  }
  
  if((skilltype == SKILL_DIRTTHROW) && ((SECT(ch->in_room) == SECT_WATER_NOSWIM)
                                    || (SECT(ch->in_room) == SECT_WATER_SWIM)
                                    || (SECT(ch->in_room) == SECT_UNDERWATER)
                                    || (SECT(ch->in_room) == SECT_OCEAN)
                                    || (SECT(ch->in_room) == SECT_FLYING))){
   send_to_char("You cannot dirt throw here!\r\n", ch);
   return;
   }
                                          


  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_EQ(ch, WEAR_HOLD) && GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need at least one free hand to do that.\r\n", ch);
    return;
  }

  if (GET_EQ(vict, WEAR_EYES))
    percent -= 20; /* adjust later to use ac apply for this item. */

  if (percent > prob || AFF_FLAGGED(vict, AFF_NOBLIND)) {
    damage(ch, vict, 0, skilltype, ABT_SKILL); /* You missed! */
    improve_abil(ch, skilltype, ABT_SKILL);
  } else {
    damage(ch, vict, GET_LEVEL(ch), skilltype, ABT_SKILL);
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
  }
  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
}

ACMD(do_trip)
{

  struct char_data *vict;
  int percent, prob;

  prob = GET_SKILL(ch, SKILL_TRIP);
  if (!prob) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Trip who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("You trip yourself.\r\n", ch);
    GET_POS(ch) = POS_SITTING;
    return;
  }
 
  if (GET_MOUNT(ch)) {
    send_to_char("You must have some long-assed legs to trip from a mount.\r\n", ch);
    return;
  }

  if (GET_POS(vict) <= POS_SITTING) {
    send_to_char("How will you trip someone who is already on the ground?\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to trip another day. */

  percent = number(1, 101);	/* 101% is a complete failure */

  if (MOB_FLAGGED(vict, MOB_NOBASH))
    percent = 101;

  if (IS_AFFECTED(vict, AFF_BALANCE))
    percent = 101;
  
  if (IS_AFFECTED(vict, AFF_FLY))
    percent = 101;

  if (GET_RACE(ch) != RACE_GIANT) {
    if (IS_NPC(vict) && GET_RACE(vict) == RACE_NPC_GIANT) 
      percent = 101;
  }

  if (percent > prob && can_bash(ch, vict) != FALSE) {
    damage(ch, vict, 0, SKILL_TRIP, ABT_SKILL);
    GET_POS(ch) = POS_SITTING;
    improve_abil(ch, SKILL_TRIP, ABT_SKILL);
  } else {
    WAIT_STATE(vict, PULSE_VIOLENCE);
    GET_POS(vict) = POS_SITTING;
    damage(ch, vict, 1, SKILL_TRIP, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_sweep)
{

  struct char_data *vict;
  int percent, prob;
  int hit_count=0;
  char msg[512];

  prob = GET_SKILL(ch, SKILL_SWEEP);
  if (!prob) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  if(ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
    send_to_char("This room has such a peaceful, easy feeling.\r\n", ch);
    return;
  }

  if (GET_MOUNT(ch)) {
    send_to_char("It ain't gonna be easy performing a maneuver like sweep "
                 "from a mount... try again.\r\n", ch);
    return;
  }

  if (ch == world[IN_ROOM(ch)].people && !ch->next_in_room){
    send_to_char("You look for someone to sweep and realize - you are "
                 "all alone.\r\n", ch);
    return;
  }

  for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room)
/*
   if (GET_RIDER(vict) && (!(PLR_FLAGGED(GET_RIDER(vict), PLR_PKILL_OK))) &&
      (!(ROOM_FLAGGED(vict->in_room, ROOM_ARENA))))
       continue;
*/

    if (IS_NPC(vict)) {
      percent = number(1, 110 - GET_DEX(vict));

      if (GET_POS(vict) <= POS_SITTING) {
        send_to_char("You find it quite impossible to sweep someone who is already on the ground.\r\n", ch);
        percent = 110;
      } 

      if (IS_AFFECTED(vict, AFF_BALANCE))
        percent = 110;

      if (IS_AFFECTED(vict, AFF_FLY))
        percent = 110;

      if (prob > percent && can_bash(ch, vict) != FALSE) {
        sprintf(msg, "Your sweep knocks %s to the ground.\r\n", GET_NAME(vict));
        send_to_char(msg, ch);
        act("$n's sweep sends $N sailing to the ground.", FALSE, ch,
            0, vict, TO_ROOM);
        damage(ch, vict, GET_LEVEL(ch), SKILL_SWEEP, ABT_SKILL);
        WAIT_STATE(vict, PULSE_VIOLENCE * 2);
        GET_POS(vict) = POS_SITTING;
        hit_count++;
      } else {
        hit(vict, ch, TYPE_UNDEFINED); 
      }
    }

  if (!hit_count) {
    send_to_char("You gracefully sweep your leg across the floor in an "
                 "attempt to knock your victim's feet out from under "
                 "them, but miss - and end up on your arse, looking "
                 "quite the fool.", ch);
    act("$n's failed sweep lands $m straight on $s ass.", FALSE, ch,
         0, vict, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    improve_abil(ch, SKILL_SWEEP, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

}

/* This is Kaan's BADASS Groinkick skill */
ACMD(do_groinkick)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_GROINKICK)) {
    send_to_char("It takes a bit of practice to kick groins - you "
                 "haven't practiced yet!.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kick who in the groin?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Ummmm...ok...\r\nYou kick yourself in the groin.\r\n", ch);
    GET_HIT(ch) = MAX(5, GET_HIT(ch)-20);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = ((10 - (GET_DEX(vict))) << 1) + number(1, 101);	/* 101% is a complete
								 * failure */
  prob = GET_SKILL(ch, SKILL_GROINKICK)+GET_LEVEL(ch)-33;

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_GROINKICK, ABT_SKILL);
    improve_abil(ch, SKILL_GROINKICK, ABT_SKILL);
  } else
    damage(ch, vict, (GET_LEVEL(ch) + (GET_STR(ch) * 2)) << 1, SKILL_GROINKICK, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

/*Kaan's Brain Skill*/
ACMD(do_brain)
{
  struct char_data *vict;
  int percent, prob;

  prob = GET_SKILL(ch, SKILL_BRAIN);
  if (!prob) {
    send_to_char("Use your head, man - you can't brain yet!\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Brain who?\r\n", ch);
      return;
    }
  }

  if (GET_EQ(ch, WEAR_WIELD) && GET_EQ(ch, WEAR_HOLD)) {
    send_to_char("You have to let go of one of your items!\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("Maybe a good book might help instead...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 100);
  if (GET_EQ(vict, WEAR_FACE))
    percent += GET_OBJ_VAL(GET_EQ(vict, WEAR_FACE), 0);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_BRAIN, ABT_SKILL);
    improve_abil(ch, SKILL_BRAIN, ABT_SKILL);
  } else
    damage(ch, vict, 4*number(10, GET_LEVEL(ch)), SKILL_BRAIN, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}
 

ACMD(do_motherearth) {

  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_MOTHEREARTH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_VIM(ch) < 20) {
    send_to_char("Your Vim is depleted.\r\n", ch);
    return;
  }
  GET_VIM(ch) -= 20;

  if (GET_SKILL(ch, SKILL_MOTHEREARTH) + GET_CHA(ch) < number(1, 130)) {
    damage(ch, vict, 0, SKILL_MOTHEREARTH, ABT_SKILL);
    improve_abil(ch, SKILL_MOTHEREARTH, ABT_SKILL);
  } else
    damage(ch, vict, dice(4, 7) + GET_LEVEL(ch), SKILL_MOTHEREARTH, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_gut)
{
  struct char_data *vict;
  int percent, prob, healthpercent;
  struct obj_data *piece;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_GUT)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Gut who?\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT) {
    send_to_char("Only slashing weapons can be used for gutting.\r\n", ch);
    return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_GUT);

  if (GET_MAX_HIT(vict) > 0) 
    healthpercent = (100 * GET_HIT(vict)) / GET_MAX_HIT(vict);
  else 
    healthpercent = -1;	
 
  if (healthpercent >= 10) {
    send_to_char("They are not hurt enough for you to attempt that.\r\n", ch);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
    return;
  }

  if (percent > prob) {
    sprintf(buf, "Even in %s's bad state, they manage to avoid your wild slash.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    send_to_char("You avoid a wild slash at your midsection.\r\n", ch);
    improve_abil(ch, SKILL_GUT, ABT_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE * 4);
  } else {

    /* EWWWW */
    GET_HIT(vict) = -10;
 
    act("You gut $N!", FALSE, ch, 0, vict, TO_CHAR);
    act("$N guts you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$n brutally guts $N!", FALSE, ch, 0, vict, TO_NOTVICT);

    act("$N looks down in horror as their intestines spill out!", FALSE, ch, 0, vict, TO_ROOM);
    act("$N looks down in horror as their intestines spill out!", FALSE, ch, 0, vict, TO_CHAR);
    act("$N looks down in horror as their intestines spill out!", FALSE, vict, 0, ch, TO_CHAR);
   hit(vict, ch, TYPE_UNDEFINED);

  if(!(piece = read_object(11, VIRTUAL))) {
    log("SYSERR: do_gut. Error loading object 11.");
    return;
  }
  obj_to_room(piece, ch->in_room);
  
  /* Seemed to be giving us some kind of wierd memory error*/
  /* piece = create_obj();

  piece->name = "intestine";
  piece->short_description = "An icky pile of intestines";
  piece->description = "An icky pile of intestines is here - colon and all.";

  piece->item_number = NOTHING;
  piece->in_room = NOWHERE;
  SET_BIT_AR(GET_OBJ_WEAR(piece), ITEM_WEAR_TAKE);
  GET_OBJ_TYPE(piece) = ITEM_FOOD;
  GET_OBJ_VAL(piece, 0) = 1;   
  GET_OBJ_VAL(piece, 3) = 1;   
  SET_BIT_AR(GET_OBJ_EXTRA(piece), ITEM_NODONATE);
  GET_OBJ_WEIGHT(piece) = 1;
  GET_OBJ_RENT(piece) = 1;
  obj_to_room(piece, ch->in_room);
  */

    WAIT_STATE(vict, PULSE_VIOLENCE * 4);
    update_pos(vict);
  }

}


ACMD(do_knockout)
{
  struct affected_type af[2];
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (!GET_SKILL(ch, SKILL_KNOCKOUT)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Knockout who?\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
 
  if (vict == ch) {
    send_to_char("That would be funny to see.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_BLUDGEON - TYPE_HIT) {
    send_to_char("Only bludgeoning weapons can be used for knocking out.\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can't knock out a fighting person -- they're too alert!\r\n", ch);
    return;
  }

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_KNOCKOUT);

  if (AWAKE(vict) && (percent < prob)) {
    act("You are knocked out when $N hits you upside your head.", FALSE, vict, 0, ch, TO_CHAR);
    act("$n sees stars, and slumps over, knocked out.", FALSE, vict, 0, ch, TO_VICT);
    act("$n sees stars, and slumps over, knocked out, after $N hits $m upside the head.", FALSE, vict, 0, ch, TO_NOTVICT);
   
    af[0].location = APPLY_AC;
    af[0].modifier = 1;
    af[0].duration = 2;
    af[0].bitvector = AFF_SLEEP;
    affect_join(vict, af, FALSE, FALSE, FALSE, FALSE);
    GET_POS(vict) = POS_SLEEPING;

    WAIT_STATE(ch, PULSE_VIOLENCE * 3);

  }  else {
    act("You notice $N raising $s weapon at you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you raising your weapon!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N raising $s weapon at $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(ch, vict, SKILL_KNOCKOUT);
    improve_abil(ch, SKILL_KNOCKOUT, ABT_SKILL);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  }
}


int chance(int num)
{
    if (number(1,100) <= num) return 1;
    else return 0;
}

ACMD(do_throw)
{ 

  struct char_data *vict;
  struct obj_data *obj;
  int percent, prob;
  int damage_val;
  two_arguments(argument, buf, buf2);


  if (!GET_SKILL(ch, SKILL_THROW)) {
    send_to_char("You don't know how!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, buf2))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Throw what at who?\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

 if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying))) {
    send_to_char("Throw what at who?\r\n", ch);
    return;
  }
  
  if (vict == ch) {
    send_to_char("That would be funny to see.\r\n", ch);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_THROW);

  damage_val = GET_STR(ch) + (GET_OBJ_WEIGHT(obj) * 2);

  if (damage_val > 100) {
	damage_val = 100;
  }

  if (percent > prob) {
    /* miss like a mother fucker. */
      damage(ch, vict, 0, SKILL_THROW, ABT_SKILL);
        /* victim */
      act("$N throws $p at you and misses by a long shot.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
      act("You throw $p at $n but, miss by a long shot.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
      act("$N throws $p at $n but, misses by a long shot.", FALSE, vict, obj, ch, TO_NOTVICT);
      improve_abil(ch, SKILL_THROW, ABT_SKILL);
      extract_obj(obj);
      return;
  }

  else {
      if (GET_OBJ_TYPE(obj) == ITEM_SCROLL || (GET_OBJ_TYPE(obj) == ITEM_NOTE)) {
        /* victim */
        act("$N hits you upside the head with $p and exclaims, Bad Doggie!", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You hit $n in the head with $p and exclaim, Bad Doggie!", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N hits $n in the head with $p and exclaims, Bad Doggie!", FALSE, vict, obj, ch, TO_NOTVICT);
        extract_obj(obj);

      }

      else if (GET_OBJ_TYPE(obj) == ITEM_WEAPON) {
       /* victim */
        act("$N throws $p at you and cuts your chest.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and cut $s chest.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and cuts $s chest.", FALSE, vict, obj, ch, TO_NOTVICT);

        extract_obj(obj);
      }

      else if (GET_OBJ_TYPE(obj) == ITEM_POTION) {

         /* victim */
        act("$N throws $p at you and it goes right down your throat!", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and it goes right down $s throat!", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and it goes right down $s throat!", FALSE, vict, obj, ch, TO_NOTVICT);
        
        if (chance(50)) {

           mag_objectmagic(vict, obj, buf, ABT_SPELL);
	}	
	else {

        /* victim */
        act("You gag and spit out $p.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("$n gags and spits out $p.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$n gags and spits out $p.", FALSE, vict, obj, ch, TO_NOTVICT);
	extract_obj(obj);  
	}
      }

      else {
        act("$N throws $p and hits you square in the chest.", FALSE, vict, obj, ch, TO_CHAR);
        /* ch */
        act("You throw $p at $n and hit $m in the chest.", FALSE, vict, obj, ch, TO_VICT);
        /* everyone else */
        act("$N throws $p at $n and hits $m in the chest.", FALSE, vict, obj, ch, TO_NOTVICT);
       extract_obj(obj);
      }

  }

  damage(ch, vict, damage_val, SKILL_THROW, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
  /* all done */

}


ACMD(do_tangleweed) {

  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_TANGLEWEED)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }


  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Who?\r\n", ch);
      return;
    }
  }
  if (GET_POS(vict) <= POS_SITTING) {
    send_to_char("That person is already all tangled up!\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_TANGLEWEED) + GET_DEX(ch) < number(1, 130)) {
    damage(ch, vict, 0, SKILL_TANGLEWEED, ABT_SKILL);
    improve_abil(ch, SKILL_TANGLEWEED, ABT_SKILL);
  } else {
    damage(ch, vict, dice(7, 7) + GET_LEVEL(ch), SKILL_TANGLEWEED, ABT_SKILL);
    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    GET_POS(vict) = POS_SITTING;
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_roundhouse)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_ROUNDHOUSE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Who do you want to roundhouse kick?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_ROUNDHOUSE);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_ROUNDHOUSE, ABT_SKILL);
    improve_abil(ch, SKILL_ROUNDHOUSE, ABT_SKILL);
  } else
    damage(ch, vict, dice(4, 7) + GET_LEVEL(ch), SKILL_ROUNDHOUSE, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}
 
ACMD(do_kickflip)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_KICKFLIP)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Kickflip who?\r\n", ch);
      return;
    }
  }
  if (GET_POS(vict) <= POS_SITTING) {
    send_to_char("How will you knock over someone who is already on the ground?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_KICKFLIP) + GET_DEX(ch) - GET_DEX(vict);

  if (percent > prob) {
    GET_POS(ch) = POS_SITTING;
    damage(ch, vict, 0, SKILL_KICKFLIP, ABT_SKILL);
    improve_abil(ch, SKILL_KICKFLIP, ABT_SKILL);
    GET_HIT(ch) -= 5;
  } else {
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
    damage(ch, vict, GET_LEVEL(ch), SKILL_KICKFLIP, ABT_SKILL);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

ACMD(do_disarm)
{
  int abil;
  struct obj_data *weap;

  if (!(abil=GET_SKILL(ch, SKILL_DISARM))) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!FIGHTING(ch)) {
    send_to_char("But you are not fighting.\r\n", ch);
    return;
  }

  if (!(weap=GET_EQ(FIGHTING(ch), WEAR_WIELD))) {
    send_to_char("They aren't using a weapon.\r\n", ch);
    return;
  }

  if (abil > number(1, 101)+(GET_DEX(FIGHTING(ch))>>1)) {
    act("You knock $p from $N's hand!", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
    act("$n knocks $p from your hand!", FALSE, ch, weap, FIGHTING(ch), TO_VICT);
    act("$n knocks $p from $N's hand!", FALSE, ch, weap, FIGHTING(ch), TO_ROOM);
    obj_to_char(unequip_char(FIGHTING(ch), WEAR_WIELD), FIGHTING(ch));
  } else {
    act("You fail to disarm $N", FALSE, ch, weap, FIGHTING(ch), TO_CHAR);
    act("$n fails to disarm you", FALSE, ch, weap, FIGHTING(ch), TO_VICT);
    act("$n fails to disarm $N", FALSE, ch, weap, FIGHTING(ch), TO_ROOM);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);

}

ACMD(do_warcry)
{

struct affected_type af;

  if (!GET_SKILL(ch, SKILL_WARCRY)) {
    send_to_char("You can't.\r\n", ch);
    return;
  }

  if (GET_MANA(ch) < 20) {
    send_to_char("You don't have the mana to perform it properly.\r\n", ch);
    return;
  }

  act("You belt out a blood-freezing warcry!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n belts out a blood-freezing warcry!", FALSE, ch, 0, 0, TO_ROOM);
  if (GET_SKILL(ch, SKILL_WARCRY) > number(1, 126-GET_CON(ch)) && 
      !AFF_FLAGGED(ch, AFF_WARCRY)) {
    af.bitvector= AFF_WARCRY;
    af.location = APPLY_HITROLL;
    af.modifier = 2;
    af.duration = 4;
    affect_to_char(ch, &af);
    GET_MANA(ch) -= 20;
  } else
    send_to_char(NOEFFECT, ch);

  WAIT_STATE(ch, PULSE_VIOLENCE);

}

ACMD(do_chop)
{
  struct char_data *vict;
  int percent, prob;

  if (abil_info[ABT_SKILL][SKILL_CHOP].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_CHOP)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Chop who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_CHOP);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_CHOP, ABT_SKILL);
    improve_abil(ch, SKILL_CHOP, ABT_SKILL);
  } else
    damage(ch, vict, dice(9, 10) + GET_LEVEL(ch), SKILL_CHOP, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_shieldpunch)
{

  struct char_data *vict;
  struct obj_data *shield;

  if (!GET_SKILL(ch, SKILL_SHIELDPUNCH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  if (!(shield = GET_EQ(ch, WEAR_SHIELD))) {
    send_to_char("This skill requires you to wear a shield.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Shieldpunch who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_SHIELDPUNCH) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_SHIELDPUNCH, ABT_SKILL);
    improve_abil(ch, SKILL_SHIELDPUNCH, ABT_SKILL);
  } else
    damage(ch, vict, 3*GET_OBJ_WEIGHT(shield), SKILL_SHIELDPUNCH, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_awe)
{

  struct char_data *vict;
  struct affected_type aff[1];
  int mod=2;

  if (!GET_SKILL(ch, SKILL_AWE)) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }
  one_argument(argument, arg);
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Who would you like to awe?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Sure.  You're impressed.\r\n", ch);
    return;
  }
  if (GET_QI(ch) < 20) {
    send_to_char("Your Qi is too weak.\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  GET_QI(ch) -= 20;
  if (GET_LEVEL(ch) > 100) mod++;

  if (GET_SKILL(ch, SKILL_AWE) < number(1, 101)) {
    act("You try to awe $N with a display of might but fail.",
        FALSE, ch, 0, vict, TO_CHAR);
    act("$n tries to awe you with a display of $s might but fail.",
        FALSE, ch, 0, vict, TO_VICT);
    act("$n tries to awe $N with a display of $s might but fails.",
        FALSE, ch, 0, vict, TO_NOTVICT);
    improve_abil(ch, SKILL_AWE, ABT_SKILL);
  } else {
    act("You awe $N with a display of might.",
        FALSE, ch, 0, vict, TO_CHAR);
    act("$n awes you with a display of $s might.",
        FALSE, ch, 0, vict, TO_VICT);
    act("$n awes $N with a display of $s might.",
        FALSE, ch, 0, vict, TO_NOTVICT);
    aff[0].bitvector= AFF_AWE;
    aff[0].location = APPLY_HITROLL;
    aff[0].modifier = -mod;
    aff[0].duration = 2;
    affect_join(vict, aff, FALSE, FALSE, FALSE, FALSE);
  }
  hit(ch, vict, TYPE_UNDEFINED);
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_backslash)
{
  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_BACKSLASH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Backslash who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT) {
    send_to_char("You must wield a slashing weapon to perform "
                 "a backslash.\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_BACKSLASH) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_BACKSLASH, ABT_SKILL);
    improve_abil(ch, SKILL_BACKSLASH, ABT_SKILL);
  } else
    damage(ch, vict, GET_SKILL(ch, SKILL_BACKSLASH),SKILL_BACKSLASH, ABT_SKILL);
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_uppercut)
{
  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_UPPERCUT)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Uppercut who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (GET_EQ(ch, WEAR_HOLD) && GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need at least one free hand to do that.\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_UPPERCUT) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_UPPERCUT, ABT_SKILL);
    improve_abil(ch, SKILL_UPPERCUT, ABT_SKILL);
  } else {
    if GET_EQ(ch, WEAR_HANDS)
      damage(ch, vict, (2*GET_OBJ_VAL(GET_EQ(ch, WEAR_HANDS), 0)) + GET_STR(ch),
             SKILL_UPPERCUT, ABT_SKILL);
    else
      damage(ch, vict, GET_STR(ch) - 2, SKILL_UPPERCUT, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);

}

ACMD(do_charge)
{
  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_CHARGE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
      send_to_char("Charge who?\r\n", ch);
      return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You are already fighting them. You can't charge someone you're already in combat with!\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  if (!GET_MOUNT(ch)) {
   send_to_char("Its pretty hard to charge into battle with no steed.\r\n", ch);
   return;
  }
/*
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT) &&
      (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_STAB - TYPE_HIT)) {
    send_to_char("You can only charge into battle with a slahsing "
                 "or stabbing weapon.\r\n", ch);
    return;
  }
*/
  if (GET_SKILL(ch, SKILL_CHARGE) < number(1, 101) && can_bash(ch, vict) != FALSE) {
    GET_POS(ch) = POS_SITTING;
    damage(ch, vict, 0, SKILL_CHARGE, ABT_SKILL);
    improve_abil(ch, SKILL_CHARGE, ABT_SKILL);
  } else {
    damage(ch,vict,(dice(5, 25)+GET_STR(ch))+(GET_LEVEL(ch)*2),SKILL_CHARGE,ABT_SKILL);
    GET_POS(vict) = POS_SITTING;
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}


ACMD(do_drown)
{

  struct char_data *vict;
  int flail, prob;

  if (!GET_SKILL(ch, SKILL_DROWN)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Drown who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if ((SECT(ch->in_room) != SECT_WATER_NOSWIM) ||
      (SECT(ch->in_room) != SECT_WATER_SWIM)) {
    send_to_char("Drowning someone on dry land is pretty damn hard.\r\n", ch);
    return;
  }

  if (IS_NPC(vict)) {
    flail = number(1, 130);
  }
  else {
    flail = GET_SKILL(vict, SKILL_FLAIL) + GET_STR(vict) + GET_DEX(vict);
  }
  prob = GET_SKILL(ch, SKILL_DROWN) + GET_STR(ch) + GET_DEX(ch);

  if (flail > prob) {
    damage(ch, vict, 0, SKILL_DROWN, ABT_SKILL);
    improve_abil(ch, SKILL_DROWN, ABT_SKILL);
  } else
    damage(ch, vict, GET_LEVEL(ch) * 4, SKILL_DROWN, ABT_SKILL);


  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_fury)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_BLADED_FURY].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_BLADED_FURY)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Name your target.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT) &&
      (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_STAB - TYPE_HIT)  &&
      (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT)) {
    send_to_char("You can only perform this skill with edged weapons.\r\n",ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_BLADED_FURY) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_BLADED_FURY, ABT_SKILL);
    improve_abil(ch, SKILL_BLADED_FURY, ABT_SKILL);
  } else {
    damage(ch, vict, dice(20, 16)+GET_DEX(ch), SKILL_BLADED_FURY, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}


ACMD(do_powerslash)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_POWERSLASH].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_POWERSLASH)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Powerslash whom?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if ((GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_SLASH - TYPE_HIT)) {
    send_to_char("You can only powerslash with slashing weapons.\r\n",ch);
    return;
  }
  if (GET_SKILL(ch, SKILL_POWERSLASH) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_POWERSLASH, ABT_SKILL);
    improve_abil(ch, SKILL_POWERSLASH, ABT_SKILL);
  } else {
    damage(ch, vict, GET_LEVEL(ch) + number(165, 450)+GET_STR(ch), 
           SKILL_POWERSLASH, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_swarm) {

  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_SWARM)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_VIM(ch) < (20-GET_CHA(ch))) {
    send_to_char("Your Vim is depleted.\r\n", ch);
    return;
  }
  GET_VIM(ch) -= (20-GET_CHA(ch));

  if (GET_SKILL(ch, SKILL_SWARM) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_SWARM, ABT_SKILL);
    improve_abil(ch, SKILL_SWARM, ABT_SKILL);
  } else
    damage(ch, vict, dice(6, 11)+GET_LEVEL(ch), SKILL_SWARM, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_elemental) {

  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_ELEMENTAL)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_VIM(ch) < 20) {
    send_to_char("Your Vim is depleted.\r\n", ch);
    return;
  }
  GET_VIM(ch) -= 20;

  if (GET_SKILL(ch, SKILL_ELEMENTAL) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_ELEMENTAL, ABT_SKILL);
    improve_abil(ch, SKILL_ELEMENTAL, ABT_SKILL);
  } else
    damage(ch, vict, dice(9, 10)+GET_LEVEL(ch), SKILL_ELEMENTAL, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_monsoon)
{

  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_MONSOON)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_VIM(ch) < 25) {
    send_to_char("You don't have enough Vim to conjure a monsoon.\r\n", ch);
    return;
  }
  GET_VIM(ch) -= 25;

  if (GET_SKILL(ch, SKILL_MONSOON) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_MONSOON, ABT_SKILL);
    improve_abil(ch, SKILL_MONSOON, ABT_SKILL);
  } else
    damage(ch, vict, dice(9, 14)+GET_LEVEL(ch), SKILL_MONSOON, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}


ACMD(do_tackle)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_TACKLE].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_TACKLE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_TACKLE) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_TACKLE, ABT_SKILL);
    GET_POS(ch) = POS_SITTING;
    update_pos(ch);
    improve_abil(ch, SKILL_TACKLE, ABT_SKILL);
  } else {
    damage(ch, vict, dice(10, 10) + GET_LEVEL(ch), SKILL_TACKLE, ABT_SKILL);
    GET_POS(vict) = POS_SITTING;
    update_pos(vict);
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_pressure)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_PRESSURE].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_PRESSURE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (GET_EQ(ch, WEAR_HOLD) && GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need at least one free hand to do that.\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_PRESSURE) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_PRESSURE, ABT_SKILL);
    improve_abil(ch, SKILL_PRESSURE, ABT_SKILL);
  } else
    damage(ch, vict, dice(10, 10) + (GET_LEVEL(ch) * 1.5), SKILL_PRESSURE, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_ki)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_KI].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_KI)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_KI) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_KI, ABT_SKILL);
    improve_abil(ch, SKILL_KI, ABT_SKILL);
  } else {
    damage(ch, vict, dice(16, 15) + GET_LEVEL(ch), SKILL_KI, ABT_SKILL);
    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_headcut)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_HEADCUT].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_HEADCUT)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You cannot do that while wielding a weapon.\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_HEADCUT) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_HEADCUT, ABT_SKILL);
    improve_abil(ch, SKILL_HEADCUT, ABT_SKILL);
  } else
    damage(ch, vict, dice(20, 18) + GET_LEVEL(ch), SKILL_HEADCUT, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_kamikaze)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_KAMIKAZE].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_KAMIKAZE)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (GET_EQ(ch, WEAR_HOLD) && GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need at least one free hand to do that.\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_KAMIKAZE) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_KAMIKAZE, ABT_SKILL);
    improve_abil(ch, SKILL_KAMIKAZE, ABT_SKILL);
  } else
    damage(ch, vict, dice(15, 18) + GET_LEVEL(ch), SKILL_KAMIKAZE, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_headbut)
{
  struct char_data *vict;

  if (!GET_SKILL(ch, SKILL_HEADBUT)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if ((GET_SKILL(ch, SKILL_HEADBUT) < number(1, 101)) && can_bash(ch, vict) != FALSE) {
    damage(ch, vict, 0, SKILL_HEADBUT, ABT_SKILL);
    improve_abil(ch, SKILL_HEADBUT, ABT_SKILL);
  } else {
    damage(ch, vict, dice(4, 7) + GET_LEVEL(ch), SKILL_HEADBUT, ABT_SKILL);
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_bearhug)
{
  struct char_data *vict;


  if (abil_info[ABT_SKILL][SKILL_BEARHUG].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_BEARHUG)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_BEARHUG) < number(1, 101)) {
    damage(ch, vict, 0, SKILL_BEARHUG, ABT_SKILL);
    improve_abil(ch, SKILL_BEARHUG, ABT_SKILL);
  } else {
    damage(ch, vict, dice(9, 14) + GET_LEVEL(ch), SKILL_BEARHUG, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_bodyslam)
{
  struct char_data *vict;

  if (abil_info[ABT_SKILL][SKILL_BODYSLAM].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  } 

  if (!GET_SKILL(ch, SKILL_BODYSLAM)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Identify your target.\r\n", ch);
      return;
    }
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if ((GET_SKILL(ch, SKILL_BODYSLAM) < number(1, 101)) && can_bash(ch, vict) != FALSE) {
    damage(ch, vict, 0, SKILL_BODYSLAM, ABT_SKILL);
    improve_abil(ch, SKILL_BODYSLAM, ABT_SKILL);
  } else {
    damage(ch, vict, dice(10, 15) + GET_LEVEL(ch), SKILL_BODYSLAM, ABT_SKILL);
    GET_POS(vict) = POS_SITTING;
    update_pos(vict);
    WAIT_STATE(vict, PULSE_VIOLENCE);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_stomp)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_STOMP)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Stomp who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to stomp another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_STOMP);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_STOMP, ABT_SKILL);
    improve_abil(ch, SKILL_STOMP, ABT_SKILL);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_STOMP, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

}

ACMD(do_swat)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_SWAT)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Swat who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to swat another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_SWAT);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_SWAT, ABT_SKILL);
    improve_abil(ch, SKILL_SWAT, ABT_SKILL);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 1, SKILL_SWAT, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

}


ACMD(do_trickpunch)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_TRICKPUNCH)) {
    send_to_char("You do not know how to trick punch.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Punch who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to trick punch another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_TRICKPUNCH);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_TRICKPUNCH, ABT_SKILL);
    improve_abil(ch, SKILL_TRICKPUNCH, ABT_SKILL);
  } else
    damage(ch, vict, GET_LEVEL(ch) >> 2, SKILL_TRICKPUNCH, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 1);

}


ACMD(do_lowblow)
{
  struct char_data *vict;
  int percent, prob;

  if (abil_info[ABT_SKILL][SKILL_LOWBLOW].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }   

  if (!GET_SKILL(ch, SKILL_LOWBLOW)) {
    send_to_char("You may be a dirty, rotten thief but you haven't
                  learned the art of the low blow yet.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("It doesn't appear that your victim is available.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);  
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to blow another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_LOWBLOW);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_LOWBLOW, ABT_SKILL);
    improve_abil(ch, SKILL_LOWBLOW, ABT_SKILL);
  } else
    damage(ch, vict, dice(10, 8) + GET_LEVEL(ch), SKILL_LOWBLOW, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3); 

}

ACMD(do_knee)
{
  struct char_data *vict;
  int percent, prob;

  if (abil_info[ABT_SKILL][SKILL_KNEE].min_level[(int)GET_CLASS(ch)] > 
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  if (!GET_SKILL(ch, SKILL_KNEE)) {
    send_to_char("You don't know how to knee yet.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("It doesn't appear that your victim is available.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Aren't we funny today...\r\n", ch);  
    send_to_char("Aren't we funny today...\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, vict))
    return; /* Live to knee another day. */

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_KNEE);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_KNEE, ABT_SKILL);
    improve_abil(ch, SKILL_KNEE, ABT_SKILL);
  } else
    damage(ch, vict, dice(10, 10) + (GET_LEVEL(ch) * 1.5), SKILL_KNEE, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 3); 

}


/*MJ*/
int perform_push(struct char_data *ch, int dir, int need_specials_check,
            struct char_data *attacker )
{
  extern char *dirs[];
  int was_in;
  int House_can_enter(struct char_data * ch, sh_int house);
  void death_cry(struct char_data * ch);
  int special(struct char_data *ch, int cmd, char *arg);

  if (need_specials_check && special(ch, dir + 1, ""))
    return 0;

  /* charmed? */
  if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->in_room == ch->master->in_room) {
    send_to_char("The thought of leaving your master makes you weep.\r\n.\r\n", ch);
    act("$n burst into tears.", FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }

  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_ATRIUM)) {
    if (!House_can_enter(ch, world[EXIT(ch, dir)->to_room].number)) {
      send_to_char("You are pushed, but you can't tresspass!\r\n", ch);
      return 0;
    }
  }
  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      world[EXIT(ch, dir)->to_room].people != NULL) {
    send_to_char("You are pushed, but there isn't enough room.\r\n", ch);
    return 0;
  }
  sprintf(buf2, "$n is pushed to the %s by $N.", dirs[dir] );
  act(buf2, TRUE, ch, 0, attacker, TO_NOTVICT);
  was_in = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);

  if (!IS_AFFECTED(ch, AFF_SNEAK))
    act("$n fall rolling on the ground", TRUE, ch, 0, 0, TO_ROOM);

  if (ch->desc != NULL)
    look_at_room(ch, 0);

  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_DEATH) && GET_LEVEL(ch) < LVL_IMMORT)
  {
    death_cry(ch);
    extract_char(ch);
    return 0;
  }
  return 1;
}


ACMD(do_push)
{
  char name[100], todir[256];
  int to;
  struct char_data *victim=NULL;
  extern char *dirs[];

  if (abil_info[ABT_SKILL][SKILL_PUSH].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  half_chop(argument, name, todir);
  if (!*name || !*todir)
    send_to_char("Push whom where?\r\n", ch);
  else if (!(victim = get_char_room_vis(ch, name)) )
    send_to_char("Nowhere by that name here.\r\n", ch);
  else if (MOB_FLAGGED(victim, MOB_SPEC))
    send_to_char("You cannot push this one, you haven't the strength.\r\n", ch);
  else if (ch == victim)
    send_to_char("But... can't you just walk?\r\n", ch);
  else if (!ok_damage_shopkeeper(ch, victim))
    send_to_char("You can't push shopkeepers, sorry.\r\n", ch);
  else if (IS_AFFECTED(ch, AFF_CHARM))
    send_to_char("No, no... that's your master.\r\n", ch);
  else if ((to = search_block(todir, dirs, FALSE)) < 0) {
    send_to_char( "That is not a direction.\r\n", ch );
    return;
  } else {
    strcpy( todir, dirs[to] );
    if (GET_POS(victim) <= POS_SITTING ) {
      send_to_char( "You can't push anybody who is lying on the ground.\r\n", ch);
      return;
    }
    if (GET_POS(victim) == POS_FIGHTING) {
      sprintf( buf, "No! you can't push %s while fighting!\r\n", HSSH(ch));
      send_to_char( buf, ch );
      return;
    }
    if (IS_AFFECTED(victim, AFF_BALANCE)) {
      act("$n tries to push $N, but $N easily keeps $S footing.",
           FALSE, ch, 0, victim, TO_NOTVICT);
      act("You try to push $N out of the room, but he is too well balanced.",
           FALSE, ch, 0 , victim, TO_CHAR);
      act("$n tries to push you, but you are too well balanced.",
           FALSE, ch, 0, victim, TO_VICT);
      return;
    }
    sprintf(buf, "$n is trying to push you to the %s!", todir);
    act( buf, FALSE, ch, 0, victim, TO_VICT );
    act( "$n is trying to push $N", FALSE, ch, 0, victim, TO_NOTVICT);
    if (!CAN_GO( victim, to)) {
      act("You can't push $M there - there's a closed door.", 
           FALSE, ch, 0, victim, TO_CHAR);
    } else if ( GET_LEVEL(victim) >= LVL_IMMORT && GET_LEVEL(ch) != LVL_IMPL) {
      send_to_char( "Oh, no, no, no.\r\n", ch );
      send_to_char( "Is trying to push you... what a mistake!\r\n", victim);
    } else if ( (GET_LEVEL(victim) - GET_LEVEL(ch) > 4) &&
                 GET_CLASS(ch) < CLASS_VAMPIRE ) {
      sprintf( buf, "You can't push %s.\r\n", HMHR(victim) );
      send_to_char( buf, ch );
      sprintf( buf, "%s can't push you.\r\n", GET_NAME(ch) );
      send_to_char( buf, victim );
    } else if ( MOB_FLAGGED(victim, MOB_NOBASH)) {
      send_to_char( "Ouch! Is too big for you!\r\n", ch );
    } else if ((dice(1,20)+3)-(GET_STR(ch)-GET_STR(victim)) < GET_STR(ch)) {
     /* You can balance the check above, this works fine for me */
      if (perform_push(victim, to, TRUE, ch)) {
        sprintf(buf, "\r\nYou give %s a good shove.\r\n", GET_NAME(victim));
        send_to_char(buf, ch);
        sprintf( buf, "\r\n/cw%s has pushed you!/c0\r\n", GET_NAME(ch));
        send_to_char( buf, victim );
      }
    } else {
      send_to_char( "Oops... you fail.", ch );
      sprintf( buf, "%s fail.\r\n", HSSH(ch) );
      *buf = UPPER(*buf);
      send_to_char( buf, victim );
    }
  }
}

/* END NEW PUSH */



ACMD(do_psychostab)
{
  struct char_data *vict;
  int percent, prob;

  if (abil_info[ABT_SKILL][SKILL_PSYCHOTIC_STABBING].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }   

  prob = GET_SKILL(ch, SKILL_PSYCHOTIC_STABBING);
  if (!prob) {
    send_to_char("You are not crazy enough to use that skill.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Stab who?\r\n", ch);
      return;
    }
  }

  if (vict == ch) {
    send_to_char("Maybe you shouldn't do that...\r\n", ch);
    return;
  }  

  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }
  if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_PIERCE - TYPE_HIT) {
    send_to_char("Only piercing weapons can be used for stabbing.\r\n", ch);
    return;
  } 

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 100);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_PSYCHOTIC_STABBING, ABT_SKILL);
    improve_abil(ch, SKILL_PSYCHOTIC_STABBING, ABT_SKILL);
  } else
    damage(ch, vict, 4 * number(60, GET_LEVEL(ch)), SKILL_PSYCHOTIC_STABBING, ABT_SKILL);

  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}  

ACMD(do_sword_thrust)
{
  struct char_data *vict;
  int dam, w_type, prob, percent;
  one_argument(argument, buf);

  /* Info needed to determine if they are successful on the skill attempt */
  percent = number(1, 101);     /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_SWORD_THRUST);

  /* Make sure victim is available */
  if (!(vict = get_char_room_vis(ch, buf))) {
    if (FIGHTING(ch))
      vict = FIGHTING(ch);
    else {
      send_to_char("Sword Thrust who?\r\n", ch);
      return;
    }
  }

  /* Make sure they are fighting */
  if (GET_POS(ch) != POS_FIGHTING) {
    send_to_char("You must be in battle to use this maneuver.\r\n", ch);
    return;
  }

  /* Make sure the samurai has the power */
  if(GET_MOVE(ch) < 50) {
    send_to_char("You are too tired to perform this maneuver.\r\n", ch);
    return;
  }

  /* Make sure the target isn't a god */
  if(GET_LEVEL(vict) >= LVL_IMMORT) {
    send_to_char("You can not strike someone of such power!\r\n", ch);
    return;
  }

  /* Don't be a dumbass you moron */
  if(vict == ch) {
    send_to_char("You really want to kill yourself?!\r\n", ch);
    return;
  }

  /* Gotta have a weapon to do this and only some weapons can do this */
  w_type = GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3);
  if (!GET_EQ(ch, WEAR_WIELD)) {
    send_to_char("You need to wield a weapon to make it a success.\r\n", ch);
    return;
  }

  if ((w_type == TYPE_CLAW) || (w_type == TYPE_BITE) || (w_type == TYPE_BLAST)) {
    send_to_char("You need a different weapon to perform this maneuver.\r\n", ch);
    return;
  }

  /* Last check to see if they pass their skill checks */
  if (percent > prob) {
    send_to_char("You thrust forward and miss!\r\n", ch);
    GET_MOVE(ch) = GET_MOVE(ch) - 25; /* 1/2 what you loose otherwise */
    return;
  }

  /* Next, we need to calculate the damage and subtract the moves */
  dam = GET_LEVEL(ch) * number(2, 3) + GET_DAMROLL(ch);
  GET_MOVE(ch) = GET_MOVE(ch) - 50;

  /* Now, damage modifiers */
  if((w_type != TYPE_SLASH) && (w_type != TYPE_PIERCE) && (w_type != TYPE_STAB))
    dam = dam/2;

  if((w_type == TYPE_STING) || (w_type == TYPE_CRUSH) || (w_type == TYPE_WHIP))
    dam = dam/2; /* this is actually /4 if you include the above /2 */

  /* Now, apply the damage and send out the messages */
  sprintf(buf,"You thrust your weapon into your victim's belly! (%d)\r\n",dam);
  send_to_char(buf,ch);
  send_to_char("$n thrusts $s weapon into your belly!\r\n", vict);
  damage(ch,vict,dam,w_type, ABT_SKILL);
}

void monk_technique(struct char_data *ch)
{
  int technique_roll=0, technique_damage=0, tornado_count=0, tornado_damage=0, i=0;
  struct affected_type af1, af2;
            
  if (!IS_NPC(ch) && (GET_CLASS(ch) == CLASS_MONK || 
       GET_CLASS(ch) == CLASS_ACROBAT || GET_CLASS(ch) == CLASS_PSIONIST || 
       GET_CLASS(ch) == CLASS_COMBAT_MASTER || GET_CLASS(ch) == CLASS_STORR ||
       GET_CLASS(ch) == CLASS_SAMURAI || GET_CLASS(ch) == CLASS_SHAMAN) && 
       !GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_SHIELD) && 
       !GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_HANDS) &&
      (GET_LEVEL(FIGHTING(ch)) < LVL_IMMORT))
              
    {
      if(dice(1,100) <= 20)  /* Perform technique 20% of time */
      {
        technique_roll = GET_LEVEL(ch)+(3*dice(1,GET_LEVEL(ch)));
            
        if(number(1,3) == 1) /* Perform kick technique */
        {
          if(technique_roll <= 150) /* Lotus Kick */
          {
            technique_damage = ( dice(1,100) + 5 ) * MAX( GET_LEVEL(ch) / 45 , 1 );
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("/cW$N strikes you with a brilliant Lotus Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYou strike $n with a brilliant Lotus Kick!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N strikes $n with a brilliant Lotus Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 300)  /* Tornado Kick */
          {
            technique_damage = 0;
			tornado_damage = ( dice(1,100) + 5 ) * MAX( GET_LEVEL(ch) / 70 , 1 );
            act("/cW$n leaps of the ground beginning a brutal spinning kick!/c0", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("/cWYou leap off the ground into a brutal spinning kick!/c0\r\n",ch);
              
            tornado_count = (dice(1,25)*GET_LEVEL(ch)/450)+1; /* Calculate number of kicks */
            for(i=0;i < tornado_count;i++) { 
                tornado_damage -= 20;

              act("/cW...$N rotates and strikes you with a brutal Tornado Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              sprintf(buf,"/cW...Your Tornado Kick connects with great force!/c0\r\n");
              send_to_char(buf,ch);
              act("/cW...$N rotates and strikes $n with a brutal Tornado Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
              technique_damage += tornado_damage;
            }
          }
          else if(technique_roll <= 550) /* Crescent Kick */
          {
            technique_damage = 150 + dice(1,GET_LEVEL(ch)) + GET_HITROLL(ch);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("/cW$N strikes you with a lightning quick Crescent Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYou strike $n with a lightning quick Crescent Kick!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N strikes $n with a lightning quick Crescent Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);

            if(!MOB_FLAGGED(FIGHTING(ch), MOB_NOBLIND) && 
               !IS_AFFECTED(FIGHTING(ch), AFF_BLIND) &&
               !AFF_FLAGGED(FIGHTING(ch), AFF_NOBLIND))
            {
              af1.location = APPLY_HITROLL;
              af1.modifier = -4;
              af1.duration = 2;
              af1.bitvector = AFF_BLIND;
              affect_join(FIGHTING(ch), &af1, FALSE, FALSE, FALSE, FALSE);
              
              af2.location = APPLY_AC;
              af2.modifier = 40;
              af2.duration = 2;
              af2.bitvector = AFF_BLIND;
              affect_join(FIGHTING(ch), &af2, FALSE, FALSE, FALSE, FALSE);
        
              act("/cWYou have been blinded!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              act("/cW$n seems to be blinded!/c0", TRUE, FIGHTING(ch), 0, ch, TO_ROOM);
            }
          }
          else /* Butterfly Kick */
          {
            tornado_damage = 200 + 2*GET_HITROLL(ch) + dice(1,100);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
            
            act("/cW$N kicks the wind out of you!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYour left foot knocks the wind out of $n!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);  
            act("/cW$N kicks the wind out of $n!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            technique_damage = tornado_damage;
            
            tornado_damage = 200 + 2*GET_HITROLL(ch) + dice(1,100);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
             
            act("/cW$N catches you off-guard with $S right foot!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYour right foot catches $n off-guard!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N connects with $S second foot catching $n off-guard!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
              
            technique_damage += tornado_damage;

            if(dice(1,2) == 1) /* Affect victim with bash */
            {
              act("/cWYou topple to the ground from the force of $N's Butterfly Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              sprintf(buf,"/cW$n topples over from the force of your Butterfly Kick!/c0");
              act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              act("/cW$n topples over from the force of $N Butterfly Kick!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
           
              GET_POS(FIGHTING(ch)) = POS_SITTING;
              WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE);
            }
            
          }
        }
        else /* Perform palm technique */
        {
          if(technique_roll <= 100) /* Iron Fist */
          {  
            technique_damage = (dice(1,100)+5)*(MAX(1,GET_LEVEL(ch)/30));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            act("/cW$N's Iron Fist enlightens you with pain!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYour Iron Fist enlightens $n with pain!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$n grimaces in pain from $N's Iron Fist!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 200) /* Dragon Claw */
          {
            technique_damage = ((GET_HITROLL(ch) * dice(1,GET_LEVEL(ch)))/50) + GET_HITROLL(ch);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
             
            act("/cW$N rips into you with a wicked Dragon Claw!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYour Dragon Claw brutally rakes $n!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N rips into $n with a wicked Dragon Claw!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 350) /* Red Buddha Fist */
          { 
            tornado_damage = 100+dice(1,GET_LEVEL(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;
            
            act("/cW$N touches you with a glowing hand!/cY", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYour glowing palm warms against $n!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N touches $n with a glowing hand!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            technique_damage = tornado_damage;
             
            tornado_damage = 150+dice(GET_LEVEL(ch),3);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && tornado_damage >= 2)
              tornado_damage /= 2;

            act("/cW$N's palms suddenly bursts into flames!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWFlames suddenly errupt from your palm as your Red Buddha Fist connects!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cWFlames errupt from $N's palm!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
             
            technique_damage += tornado_damage;
          }
          else if(technique_roll <= 400) /* Devil's Claw */ 
          {
            technique_damage = 100+dice(3,GET_LEVEL(ch)) + (3*GET_DEX(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
             
            act("/cWYou feel your life draining as $N's Devil's Claw rakes you!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            sprintf(buf,"/cWYou usurp life from $n with your Devil's Claw!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$n looks horrified as $N claws their life away!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
         
            if(GET_HIT(ch) < GET_MAX_HIT(ch))
            {
              act("/cW...Your wounds mend as you drain $n's precious life force!/c0", FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + technique_damage/10);
            }
          } 
          else if(technique_roll <= 450) /* Eye of the Phoenix */
          {
            technique_damage = dice(10,GET_WIS(ch)) + dice(4,GET_HITROLL(ch)) + (GET_LEVEL(ch)*2);
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
          
            send_to_char("/cWYour hands glow as you channel your spiritual energy to them!/c0\r\n",ch);
            sprintf(buf,"/cYYou release the Eye of the Phoenix to punish $n!/c0");
            act("/cW$N punishes you with a glowing palm-strike!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N punishes $n with a glowing palm-strike!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
          else if(technique_roll <= 500) /* Seven Star Earthquake Fist */
          {
            technique_damage = 50+GET_HITROLL(ch) + dice(4,GET_LEVEL(ch)) + dice(7,GET_STR(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
            
            sprintf(buf,"/cWYou unleash a flurry of Seven Strikes upon $n!/c0");
            act("/cW$N batters you with a flurry of attacks!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N batters $n with a flurry of attacks!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
            
            if(dice(1,10) == 1)
            {
              act("/cW$n falls over, overwhelmed by your ferocious assault!/c0", FALSE, FIGHTING(ch), 0, ch, TO_VICT);
              act("/cWYou fall over, overwhelmed by $N's ferocious assault!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
              act("/cW$n falls over, overwhelmed by $N's ferocious assault!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
             
              GET_POS(FIGHTING(ch)) = POS_SITTING; 
              WAIT_STATE(FIGHTING(ch), PULSE_VIOLENCE);
              
            }
          }
          else if(technique_roll <= 590) /* Heavenly Thunder Palm */
          {
            technique_damage = 50+dice(4,GET_HITROLL(ch)) + (dice(1,GET_DEX(ch))*dice(1,GET_WIS(ch))*dice(1,GET_STR(ch)));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)
              technique_damage /= 2;
           
            send_to_char("/cWYour eyes close as you prepare your Heavenly Thunder Palm!/c0\r\n",ch);
            sprintf(buf,"/cWYou electrify $n with a stream of spiritual energy!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            act("/cW$N electrifies you with a crackling palm-strike!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act("/cW$N electrifies $n with a crackling palm-strike!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }  
          else /* The Fabled Quivering Palm */
          {
            technique_damage = dice(2, MAX(1,GET_WIS(ch)-10));
            technique_damage *= dice(2, MAX(1,GET_DEX(ch)-10));
            technique_damage *= dice(1, MAX(1,GET_STR(ch)-14));
            technique_damage *= dice(1, MAX(1,GET_CON(ch)-14));
            technique_damage += dice(10, GET_LEVEL(ch));
            technique_damage += dice(10, GET_HITROLL(ch));
            if (IS_AFFECTED(FIGHTING(ch), AFF_SANCTUARY) && technique_damage >= 2)  
              technique_damage /= 2;
            
            send_to_char("/cWYour hand begins to quiver violently as you prepare to strike!/c0\r\n",ch);
            sprintf(buf,"/cWYou unlease the fabled Quivering Palm upon $n!/c0");
            act(buf, FALSE, FIGHTING(ch), 0, ch, TO_VICT);
            // Yeah, get Bruce Lee on the mother fucker
            act("/cWYour jaw drops as $N unleashes the fabled Quivering Palm upon you!/c0", FALSE, FIGHTING(ch), 0, ch, TO_CHAR);
            act("/cWYour jaw drops as $N unleashes the fabled Quivering Palm upon $n!/c0", FALSE, FIGHTING(ch), 0, ch, TO_NOTVICT);
          }
            
        }
        damage(ch, FIGHTING(ch), technique_damage, SKILL_FLAIL, ABT_SKILL);    
      }
    }
}

ACMD(do_berserk)
{

  struct affected_type aff[1];

  if (!GET_SKILL(ch, SKILL_BERSERK)) {
    send_to_char("You don't know how.\r\n", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_BERSERK)) {
    send_to_char("You are already berserking!\r\n", ch);
    return;
  }

  if (GET_SKILL(ch, SKILL_BERSERK) < number(1, 101)) {
	send_to_char("You fail\r\n", ch);
    improve_abil(ch, SKILL_BERSERK, ABT_SKILL);
  } else {
    send_to_char("You enter a berserker rage and fly into battle!\r\n", ch);

    act("$n flies into battle with a berserker rage, spraying spittle everywhere!",
        FALSE, ch, 0, 0, TO_NOTVICT);
    aff[0].bitvector= AFF_BERSERK;
    aff[0].location = APPLY_DAMROLL;
    aff[0].modifier = 10;
    aff[0].duration = 10;
    affect_join(ch, aff, FALSE, FALSE, FALSE, FALSE);
    GET_HIT(ch) = GET_HIT(ch) / 2;
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_inject)
{
 send_to_char("Not ready yet, sorry.", ch);
}
/*
  struct char_data *vict;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int poison;

  two_arguments(argument, arg1, arg2);

  if (!GET_SKILL(ch, SKILL_INJECT)) {
    send_to_char("You haven't learned how to properly inject yet.\r\n", ch);
    return;
  }

  if (abil_info[ABT_SKILL][SKILL_INJECT].min_level[(int)GET_CLASS(ch)] >
      GET_LEVEL(ch) && GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You are not at a high enough level to use this skill yet.\r\n"
                  , ch);
    return;
  }

  if (!GET_EQ(ch, WEAR_WIELD) || 
       GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) != ITEM_NEEDLE) {
    send_to_char("You have to be wielding a needle to inject anyone.\r\n", ch); 
    return;
  }

  if (!IS_AFFECTED(ch, AFF_SNEAK)) {
    send_to_char("You have to use sneak to use this skill.\r\n", ch);
    return;
  }

  if (!*arg1) {
    send_to_char("Who do you want to inject?\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg1))) {
      send_to_char("It doesn't appear that your victim is available.\r\n", ch);
      return;
  } 
  if (vict == ch) {
    send_to_char("Just remember... users are losers.\r\n", ch);
  }

  poison = 0;
 
  if (!(poison = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
    sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg), arg2);
    send_to_char(buf, ch);
    return;
  }
}
*/ 

ACMD(do_trample) {

  struct char_data *vict;
  int percent, prob;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_TRAMPLE)) {
    send_to_char("You do not know this skill.\r\n", ch);
    return;
  }
  if (!GET_MOUNT(ch)) {
    send_to_char("You suddenly grow hooves, or what? Get a horse!\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Trample who?\r\n", ch);
      return;
    }
  }
  if (!IS_NPC(vict) && !pkill_ok(ch, vict)) {
    send_to_char("You can not attack another player here.\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("Trample yourself with your horse's hooves while you are on your horse... right - okay.\r\n", ch);
    act("$n looks a bit confused from up on his mount.", 
         FALSE, ch, 0, vict, TO_ROOM);
    return;
  }
  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  if (GET_MOVE(ch) < 15)
  {
	  send_to_char("You are to tired!!\r\n", ch);
	  return;
  }

  percent = number(65, 83) + GET_DEX(vict);
  prob = GET_SKILL(ch, SKILL_TRAMPLE);

  if (IS_AFFECTED(vict, AFF_AWARE))
    percent = 101;

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID))
    percent += 10;

  GET_MOVE(ch) -= 15;

  if (percent < prob && can_bash(ch, vict) != FALSE) {
    WAIT_STATE(vict, PULSE_VIOLENCE*2);
    damage(ch, vict, (GET_LEVEL(ch) * 2) + dice(4, 10),SKILL_TRAMPLE,ABT_SKILL);
    GET_POS(vict) = POS_SITTING;
  } else {
    damage(ch, vict, 0, SKILL_TRAMPLE, ABT_SKILL);
    improve_abil(ch, SKILL_TRAMPLE, ABT_SKILL);
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*3);
}  

ACMD(do_swoop)
{
  struct char_data *vict;
  int percent, prob;

  one_argument(argument, buf);

  if (GET_CLASS(ch) != CLASS_VAMPIRE) {
    send_to_char("Only vampires have the art of swoop.\r\n", ch);
    return;
  }
 
  if (!GET_SKILL(ch, SKILL_SWOOP)) {
    send_to_char("You have yet to learn to swoop. Seek a master's "
                 "teachings.\r\n", ch);
    return;
  }

  if (!AFF_FLAGGED(ch, AFF_HOVER)) {
    send_to_char("You must be in the air before you can swoop down upon "
                 "a victim.\r\n", ch);
    return;
  } 

  if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Swoop down upon whom?\r\n", ch);
    return;
  }
  if (vict == ch) {
    send_to_char("How can you swoop down upon yourself?\r\n", ch);
    return;
  }
  if (FIGHTING(vict)) {
    send_to_char("You can only swoop down upon the unsuspecting.\r\n"
            "You are making way too much commotion right now.\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  } 

  if (MOB_FLAGGED(vict, MOB_AWARE)) {
    act("You notice $N swooping down upon you!", FALSE, vict, 0, ch, TO_CHAR);
    act("$e notices you swooping down upon $m!", FALSE, vict, 0, ch, TO_VICT);
    act("$n notices $N swooping down upon $m!", FALSE, vict, 0, ch, TO_NOTVICT);
    hit(vict, ch, TYPE_UNDEFINED);
    return;
  }

  percent = number(1, 101);     /* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_SWOOP);

  if (AWAKE(vict) && (percent > prob)) {
    damage(ch, vict, 0, SKILL_SWOOP, ABT_SKILL);
    improve_abil(ch, SKILL_SWOOP, ABT_SKILL);
  }  
  else {
    hit(ch, vict, SKILL_SWOOP);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}  


ACMD(do_rend)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_REND)) {
    send_to_char("You must first learn to rend from a Master.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("You cannot rend that which is not there.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Rend yourself? Are you a suicidal vampire then?\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_REND);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_REND, ABT_SKILL);
  } else
    damage(ch, vict, dice(12, 13) + (MAX(1, GET_TIER(ch)) * 25), 
           SKILL_REND, ABT_SKILL);


  WAIT_STATE(ch, PULSE_VIOLENCE * 3);

} 


ACMD(do_tear)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_TEAR)) {
    send_to_char("You must first learn to tear from a Master.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Your victim must be present.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("You wish to lay your own dead flesh open?\r\n"
                 "For what purpose?\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_TEAR);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_TEAR, ABT_SKILL);
  } else
    damage(ch, vict, dice(15, 10) + (number(100, 400)), SKILL_TEAR, ABT_SKILL);


  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

} 

ACMD(do_spit_blood)
{
  struct char_data *vict;
  int percent, prob;

  if (!GET_SKILL(ch, SKILL_SPIT_BLOOD)) {
    send_to_char("You must first learn to spit blood from a Master.\r\n", ch);
    return;
  }

  one_argument(argument, arg);

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Your victim must be present.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("You wish to spit your own blood upon your body?\r\n"
                 "For what purpose?\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  }

  percent = number(1, 101);
  prob = GET_SKILL(ch, SKILL_SPIT_BLOOD);

  if (percent > prob) {
    damage(ch, vict, 0, SKILL_SPIT_BLOOD, ABT_SKILL);
  } else
    damage(ch, vict, dice(25, 25) + number(250,300), SKILL_SPIT_BLOOD, ABT_SKILL
);


  WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_harness)
{
  struct char_data *vict;
  int element, percent, prob;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  
  two_arguments(argument, arg1, arg2);

  element = 0;

  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("This supernatural skill can only be used by titans.\r\n", ch);
    return;
  }
  if (!*arg1) {
    send_to_char("Which of the elements do you wish to harness?\r\n", ch);
    return;
  }
  if (is_abbrev(arg1, "wind")) {
    if (!GET_SKILL(ch, SKILL_HARNESS_WIND)) {
      send_to_char("You have not yet learned to harness the winds.\r\n", ch);
      return;
    }
    element = 1;
  }
  if (is_abbrev(arg1, "water")) {
    if (!GET_SKILL(ch, SKILL_HARNESS_WATER)) {
      send_to_char("You have not yet learned to harness the water.\r\n", ch);
      return;
    }
    element = 2;
  }
  if (is_abbrev(arg1, "fire")) {
    if (!GET_SKILL(ch, SKILL_HARNESS_FIRE)) {
      send_to_char("You have not yet learned to harness fire.\r\n", ch);
      return;
    }
    element = 3;
  }
  if (is_abbrev(arg1, "earth")) {
    if (!GET_SKILL(ch, SKILL_HARNESS_EARTH)) {
      send_to_char("You have not yet learned to harness the earth.\r\n", ch);
      return;
    }
    element = 4;
  }
  if (!*arg2) {
    sprintf(buf, "Who is it you wish to harness the power of %s against?\r\n",arg1);
    send_to_char(buf, ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg2))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Your victim must be present.\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Why would you harness such a power against yourself?\r\n",ch);
    return;
  }

  percent = number(25, 101) + GET_TIER(ch);
  prob = number(50, 100);

  if (percent > prob) {
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    if (element == 1) {
      if (GET_MOVE(ch) <= 49) {
        send_to_char("You don't have enough moves for that!\r\n", ch);
      } else
        damage(ch, vict, number(350, 600) + (GET_STR(ch) * 2), 
             SKILL_HARNESS_WIND, ABT_SKILL);
        GET_MOVE(ch) -= 50;
    }
    if (element == 2) {
      if (GET_MOVE(ch) <= 76) {
        send_to_char("You don't have enough moves for that!\r\n", ch);
      } else
        damage(ch, vict, number(500, 750) + (GET_STR(ch) * 2),
             SKILL_HARNESS_WIND, ABT_SKILL);
        GET_MOVE(ch) -= 75;
    }
    if (element == 3) {
      if (GET_MOVE(ch) <= 99) {
        send_to_char("You don't have enough moves for that!\r\n", ch);
      } else
        damage(ch, vict, number(700, 850) + (GET_STR(ch) * 2),
             SKILL_HARNESS_WIND, ABT_SKILL);
        GET_MOVE(ch) -= 100;
    }
    if (element == 4) {
      if (GET_MOVE(ch) <= 124) {
        send_to_char("You don't have enough moves for that!\r\n", ch);
      } else
        damage(ch, vict, number(800, 1000) + (GET_STR(ch) * 2),
             SKILL_HARNESS_WIND, ABT_SKILL);
        GET_MOVE(ch) -= 125;
    }
  } else
    WAIT_STATE(ch, PULSE_VIOLENCE *2);
    damage(ch, vict, 0, SKILL_HARNESS_WIND, ABT_SKILL);
}

ACMD(do_bellow)
{
  struct affected_type af[2];
  struct char_data *vict;
  int success;

  one_argument(argument, arg);

  if (GET_CLASS(ch) != CLASS_TITAN) {
    send_to_char("Only titans have the lung capacity to bellow.\r\n", ch);
    return;
  }
  if (!GET_SKILL(ch, SKILL_BELLOW)) {
    send_to_char("You must first learn to bellow.\r\n", ch);
    return;
  }
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      vict = FIGHTING(ch);
    } else {
      send_to_char("Bellow at who?\r\n", ch);
      return;
    }
  }
  if (vict == ch) {
    send_to_char("Titans do not have a sense of humor.\r\n", ch);
    return;
  }

  if (!pk_allowed && !pkill_ok(ch, vict)) {
    send_to_char("You are forbidden to attack that player.\r\n", ch);
    return;
  } 

  success = number(10, 20) + (GET_HIT(ch)*100/GET_MAX_HIT(ch));

  if (success >= 100) {
    act("You bellow out a lunge full of air, deafening $N.\r\n",
         TRUE, ch, 0, vict, TO_CHAR);
    act("$n bellows out a lunge full of air, deafening you!.\r\n",
         TRUE, ch, 0, vict, TO_VICT);
    act("$n bellows out a lunge full of air, deafening $N!.\r\n",
         TRUE, ch, 0, vict, TO_NOTVICT); 
    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    af[0].location = APPLY_AC;
    af[0].modifier = 50;
    af[0].duration = 1;
    af[0].bitvector = AFF_DEAF;

    af[1].location = APPLY_HITROLL;
    af[1].modifier = -20;
    af[1].duration = 1;
    af[1].bitvector = AFF_DEAF;

    affect_join(vict, af, FALSE, FALSE, FALSE, FALSE);
    affect_join(vict, af+1, FALSE, FALSE, FALSE, FALSE);

  } else if (success >= 60 && success <=99) {
    act("You bellow out a lunge full of air.\r\n",
         TRUE, ch, 0, 0, TO_CHAR);
    act("$n bellows out a lunge full of air, directly at you.\r\n", 
         TRUE, ch, 0, vict, TO_VICT);
    act("$n bellows out a lunge full of air, directly at $N.\r\n",
         TRUE, ch, 0, vict, TO_NOTVICT);
    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  } else
    act("You bellow out with your best, but it has no affect.",
         TRUE, ch, 0, 0, TO_CHAR);
    act("$n bellows out with $s best, but you are able to shrug it off.",
         TRUE, ch, 0, vict, TO_VICT);
    act("$n bellows out with $s best, but it has no affect on $N.",
         TRUE, ch, 0, vict, TO_NOTVICT);
}

ACMD(do_turn) {

  struct char_data *vict;
  int percent;

  one_argument(argument, arg);

  if (!GET_SKILL(ch, SKILL_TURN)) {
    send_to_char("You do not know this skill.\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Turn and attack who?\r\n", ch);
    return;
  }
  if (GET_MOUNT(ch)) {
    send_to_char("You can't use this skill while mounted.\r\n", ch);
    return;
  }

  if (!(FIGHTING(ch))) {
    send_to_char("You aren't engaged in combat!\r\n", ch);
    return;
  }
 
  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      if (ch->in_room == FIGHTING(ch)->in_room) {
        vict = FIGHTING(ch);
      } else {
        send_to_char("They are not close enough!\r\n", ch);
        return;
      }
    } else {
      send_to_char("Switch your attacks to who?\r\n", ch);
      return;
    }
  }

  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    act("$n tries to attack $N but trips and falls down.",
        FALSE, ch, 0, vict, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    return;
  }

  percent = number(0, 101);
  if (percent > GET_SKILL(ch, SKILL_TURN))
    send_to_char("You fail.\r\n", ch);
  else {
    stop_fighting(ch);
    set_fighting(ch, vict);
    act("You spin and attack $N!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n spins and attacks $N!", FALSE, ch, NULL, vict, TO_ROOM);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_wheel) {
 
  struct char_data *vict;
  int percent = number(0, 101);

  one_argument(argument, arg);
 
  if (!GET_SKILL(ch, SKILL_WHEEL)) {
    send_to_char("You do not know this skill.\r\n", ch);
    return;
  }
  if (!*arg) {
    send_to_char("Who do you wish to attack?\r\n", ch);
    return;
  }
  if (!GET_MOUNT(ch)) {
    send_to_char("Wheel what? You're not even mounted.\r\n", ch);
    return;
  }

  if (!(FIGHTING(ch))) {
    send_to_char("You aren't engaged in combat!\r\n", ch);
    return;
  }

  if (!(vict = get_char_room_vis(ch, arg))) {
    if (FIGHTING(ch)) {
      if (ch->in_room == FIGHTING(ch)->in_room) {
        vict = FIGHTING(ch);
      } else {
        send_to_char("They are not close enough!\r\n", ch);
        return;
      }
    } else {
      send_to_char("Wheel and strike whom?\r\n", ch);
      return;
    }
  }
 
  if (AFF_FLAGGED(ch, AFF_CHARM)) {
    act("$n tries to attack $N but trips and falls down.",
        FALSE, ch, 0, vict, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    return;
  }

  if (percent > GET_SKILL(ch, SKILL_WHEEL))
    send_to_char("You fail.\r\n", ch);
  else {
    stop_fighting(ch);
    set_fighting(ch, vict);
    act("You wheel your mount and strike $N!", FALSE, ch, 0, vict, TO_CHAR);
    act("$n wheels $s mount and strikes $N!", FALSE, ch, NULL, vict, TO_ROOM);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_gauge) {
 
  struct char_data *vict;
  struct affected_type af;
  int percent = 0, prob = 0;

  one_argument(argument, arg); 

  if (!(vict = get_char_room_vis(ch, arg))) {
    send_to_char("Gauge whose weaknesses?\r\n", ch);
    return;
  }

  if (!IS_NPC(vict)) {
    if (!pk_allowed && !pkill_ok(ch, vict)) 
    send_to_char("You can't gauge other players unless you and your victim are pkillers.\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(vict, AFF_GAUGE)) {
    send_to_char("Nothing happens...\r\n", ch);
    return;
  }


  percent = number(1, 101);     /* 101% is a complete failure */ 
  prob = GET_SKILL(ch, SKILL_GAUGE);  

  if (percent > prob) {
    send_to_char("You fail to sense any weaknesses.\r\n", ch);
    act("$n tries to find $N's weaknesses but fails.\r\n",
        FALSE, ch, 0, vict, TO_ROOM);
    return;
  }

  if (number (0, 25) > GET_DEX(ch)) {
    send_to_char("They sense your evil intent and attack!\r\n", ch);
    act("$n tries to find $N's weaknesses, but gets attacked instead!\r\n",
        FALSE, ch, 0, vict, TO_ROOM); 
    hit(vict, ch, TYPE_UNDEFINED);
  } else {
 
    af.type = SKILL_GAUGE;
    af.duration = 3;
    af.modifier = (number(2, 6));
    af.location = APPLY_DAMROLL;
    af.bitvector = AFF_GAUGE;
    affect_to_char(vict, &af);
    send_to_char("You probe for its weaknesses.\r\n", ch);
    act("$n probes for $N's weaknesses.\r\n",
        FALSE, ch, 0, vict, TO_ROOM); 
  }

}
ACMD(do_quickdraw) {


  if (!GET_SKILL(ch, SKILL_QUICKDRAW))
    send_to_char("You aren't adept enough to use this skill.\r\n", ch);
  else
    perform_quickdraw(ch, NULL, SCMD_PRACTICE_QUICKDRAW);

}
