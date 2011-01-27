/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "house.h"
#include "hometowns.h"
#include "dg_scripts.h"
#include "mounts.h"

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern const int rev_dir[];
extern char *dirs[];
extern char *AEdirs[];
extern int movement_loss[];
extern struct index_data *mob_index;
extern int newbie_level;
extern struct obj_data *get_obj_in_list_type(int type, struct obj_data *list);
extern int nr_mount_types;
struct mount_types mountt[MAX_MOUNT_TYPES];

/* external functs */
int special(struct char_data *ch, int cmd, char *arg);
void death_cry(struct char_data *ch);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
int isname(char *str, char *namelist);
room_rnum find_target_room(struct char_data * ch, char *rawroomstr);

const char *leave_msg[]={
"leaves",
"walks",
"slithers",
"flits",
"swims",
"buzzes",
"shambles",
"walks",
"trundles",
"surges",
"strides",
"drifts",
"walks",
"walks",
"walks",
"walks",
"walks",
"walks",
"walks",
"stomps",
"walks",
"walks",
"walks",
"walks",
"flits",
"shambles",
"moves on dainty roots",
"surges",
"shambles",
"walks",
"moves",
"limps",
"walks",
"moves woodenly",
"flits",
"moves",
"shambles",
"moves",
"walks",
"scrambles",
"strides",
"slinks",
"slinks",
"scuttles",
"moves",
"moves",
"shambles",
"floats",
"stomps",
"moves",
"\n",
};

void crunchy_snow(struct char_data * ch)
{
  int door, was_in;

  act("You hear snow crunch as $n walks away.", FALSE, ch, 0, 0, TO_ROOM
);
  was_in = ch->in_room;

  for (door = 0; door < NUM_OF_DIRS; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      act("You hear foot steps crunching in the snow.", FALSE, ch, 0, 0
, TO_ROOM);
      ch->in_room = was_in;
    }
  }
}


/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
  struct obj_data *obj;
  int i;

/*
  if (ROOM_IDENTITY(ch->in_room) == DEAD_SEA)
    return 1;
*/

  // Immortals can walk on water
  if (GET_LEVEL(ch) >= LVL_IMMORT)
    return 1;

  /* Water-walk counts */
  if (AFF_FLAGGED(ch, AFF_WATERWALK))
    return 1;

  /* Fly counts too */
  if (AFF_FLAGGED(ch, AFF_FLY))
    return 1;

  /* Unfurled demon wings do the trick too */
  if (AFF_FLAGGED(ch, AFF_UNFURLED))
    return 1;

  /* Hover will hook you up */
  if (AFF_FLAGGED(ch, AFF_HOVER))
    return 1;

  /* non-wearable boats in inventory will do it */
  for (obj = ch->carrying; obj; obj = obj->next_content)
    if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
      return 1;

  /* and any boat you're wearing will do it too */
  for (i = 0; i < NUM_WEARS; i++)
    if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
      return 1;

  return 0;
}

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns :
 *   1 : If success.
 *   0 : If fail
 */
int do_simple_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in, need_movement;
  int special(struct char_data *ch, int cmd, char *arg);
  struct char_data *mover;
  struct descriptor_data *d;

  /*
   * Check for special routines (North is 1 in command list, but 0 here) Note
   * -- only check if following; this avoids 'double spec-proc' bug
   */
  if (need_specials_check && special(ch, dir + 1, ""))
    return 0;

  /* charmed? */
  if ((AFF_FLAGGED(ch, AFF_CHARM) &&
       ch->master && ch->in_room == ch->master->in_room) ||
      GET_RIDER(ch)) {
    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }
  if (AFF_FLAGGED(ch, AFF_SEALED)) {
    send_to_char("You have been sealed to this room. You cannot leave until the magic wears off.\r\n", ch);
    act("$n attempts to leave, but is sealed to this room.",
         FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }
  if (AFF_FLAGGED(ch, AFF_MESMERIZED)) {
    send_to_char("You have been mesmerizes and can not move.\r\n", ch);
    act("$n attempts to leave, but fails.",
         FALSE, ch, 0, 0, TO_ROOM);
    return 0;
  }

  /* We'll need to know who is doing the work... */
  mover = (GET_MOUNT(ch) && GET_MOUNT(ch)->in_room==ch->in_room) ? 
           GET_MOUNT(ch) : ch;
  /* if this room or the one we're going to needs a boat, check for one */
  /* Has_boat, counts other things now like FLY or WATERWALK      */
  if ((SECT(ch->in_room) == SECT_WATER_NOSWIM) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM) ||
      (SECT(ch->in_room) == SECT_OCEAN) ||
      (SECT(EXIT(ch, dir)->to_room) == SECT_OCEAN)) {
    if (!has_boat(mover)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return 0;
    }
  }

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if ((SECT(EXIT(ch, dir)->to_room) == SECT_NOTRAVEL)) {
      send_to_char("You can only go there aboard a ship.\r\n", ch);
      return 0;
    }
  }
  

  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if ((SECT(EXIT(ch, dir)->to_room) == SECT_UNDERWATER) &&
        !AFF_FLAGGED(ch, AFF_BREATHE)) {
      send_to_char("You can't go there.  You'd drown!\r\n", ch);
      return 0;
    }
  }
  if (GET_LEVEL(ch) < LVL_IMMORT) {
    if ((SECT(EXIT(ch, dir)->to_room) == SECT_FLYING) &&
        !AFF_FLAGGED(mover, AFF_FLY) && !AFF_FLAGGED(mover, AFF_HOVER) &&
        !AFF_FLAGGED(mover, AFF_UNFURLED)) {
      send_to_char("You need to be flying to go there.\r\n", ch);
      return 0;
    }
  }

  /* move points needed is avg. move loss for src and destination sect type */
  need_movement = (movement_loss[SECT(ch->in_room)] +
		   movement_loss[SECT(EXIT(ch, dir)->to_room)]) >> 1;

  if (AFF_FLAGGED(ch, AFF_FLY)) need_movement >>= 1;
  if (AFF_FLAGGED(ch, AFF_SLOW)) need_movement <<= 1;
  if (AFF_FLAGGED(ch, AFF_DROWSE)) need_movement <<=1;
  if (AFF_FLAGGED(ch, AFF_INCOGNITO)) need_movement <<= 1;
  if (AFF_FLAGGED(ch, AFF_RUN)) need_movement >>= 1;
  if (AFF_FLAGGED(ch, AFF_HOVER)) need_movement >>= 1;

  /*if (GET_MOVE(ch) < need_movement && !IS_NPC(ch)) { */
  if (GET_MOVE(ch) < need_movement && !GET_MOUNT(ch)) {
    if (need_specials_check && ch->master)
      send_to_char("You are too exhausted to follow.\r\n", ch);
    else
      send_to_char("You are too exhausted.\r\n", ch);
    return 0;
  }
  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_ATRIUM) && !IS_NPC(ch)) {
    if (world[EXIT(ch, dir)->to_room].number != real_room(30003) &&
       (!House_can_enter(ch, world[EXIT(ch, dir)->to_room].number))) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return 0;
    }
  }
  if ((IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) ||
       IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_SINGLE_SPACE)) &&
      IS_AFFECTED(ch, AFF_UNFURLED)) {
    send_to_char("Your wings are too wide to fit through that "
                 "direction!\r\n", ch);
    return 0;
  }

  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_TUNNEL) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 1) {
    send_to_char("There isn't enough room there for more than two "
                 "people!\r\n", ch);
    return 0;
  }
  if (IS_SET_AR(ROOM_FLAGS(EXIT(ch, dir)->to_room), ROOM_SINGLE_SPACE) &&
      num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) >= 1) {
    send_to_char("There isn't enough room there for more than one "
                 "person!\r\n", ch);
    return 0;
  } 

  /* Do the riding stuff. */
  if (!IS_NPC(ch) && GET_MOUNT(ch) &&
      ((GET_LEVEL(ch) < LVL_IMMORT) || !PRF_FLAGGED(ch, PRF_NOHASSLE)) && 
      (GET_SKILL(ch, SKILL_RIDE)+GET_DEX(ch)) <
      number(1, 101)+(int)(GET_ALIGNMENT(GET_MOUNT(ch))*.015)) {
    /* Poor bastard fell off! */
    act("$N bucks you off!", FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
    act("$N bucks $n who goes flying to the ground.", FALSE, ch, 0,
        GET_MOUNT(ch), TO_ROOM);
    GET_HIT(ch)-=4;
    GET_POS(ch) = POS_SITTING;
    GET_RIDER(GET_MOUNT(ch)) = NULL;
    GET_MOUNT(ch) = NULL;
    update_pos(ch);
    return 0;
  }

  /* Mounts don't like it indoors. */
  if (!IS_NPC(ch) && GET_MOUNT(ch) &&
      ((GET_LEVEL(ch) < LVL_IMMORT) || !PRF_FLAGGED(ch, PRF_NOHASSLE)) &&
      (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS)))
  {
			  if (GET_SKILL(ch, SKILL_RIDE) < number(70, 111))
			  {
				  /* Horse don't wanna move */
				  act("$N balks at moving through such a confined space.", 
					  FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
				  act("$n's mount balks at moving forward into such a confined space.", 
					  FALSE, ch, 0, GET_MOUNT(ch), TO_ROOM);
				  return 0;
			  }
  }

  if (IS_SET(EXIT(ch, dir)->exit_info, EX_STEEP) &&
      !GET_SKILL(ch, SKILL_CLIMB)) {
    send_to_char("\r\nThe way is too steep to negotiate.\r\n"
                 "Perhaps you could find some thief to climb it.\r\n", ch);
    return 0;

  } else
 
  if (GET_LEVEL(ch) < LVL_IMMORT && !IS_NPC(ch) && !GET_MOUNT(ch))
    GET_MOVE(ch) -= need_movement;
 
  if (AFF_FLAGGED(ch, AFF_HOVER)) {
    send_to_char("You feel a slight loss of blood.\r\n", ch);
    GET_HIT(ch) -= 5; 
  }

  if (IS_SET(EXIT(ch, dir)->exit_info, EX_HIDDEN))
    send_to_char("\r\n/cYYou found a secret exit!/c0\r\n\r\n", ch);

  if (IS_SET(EXIT(ch, dir)->exit_info, EX_STEEP)) {
    sprintf(buf, "\r\n/cyYou climb your way %s./c0\r\n\r\n", dirs[dir]);
    act(buf, TRUE, ch, 0, 0, TO_CHAR);
  }

  was_in = ch->in_room;
    if (!GET_MOUNT(ch)) {
    if (!ROOM_FLAGGED(ch->in_room, ROOM_IMPEN)) {
    if (!AFF_FLAGGED(ch, AFF_SNEAK) && !AFF_FLAGGED(ch, AFF_SHADOW)) {
      if (GET_CLASS(ch) == CLASS_VAMPIRE) {
      sprintf(buf2, "The shadows withdraw as %s %s %s.",
          AFF_FLAGGED(ch, AFF_INCOGNITO) ? "someone" : GET_NAME(ch),
          (AFF_FLAGGED(ch, AFF_FLY) || AFF_FLAGGED(ch, AFF_UNFURLED) || 
           AFF_FLAGGED(ch, AFF_HOVER)) ? "floats" :
          "leaves", dirs[dir]);
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
      }
	  else if (AFF_FLAGGED(ch, AFF_FOG_MIST_AURA))
	  {
		  sprintf(buf2, "The air suddenly feels warmer as if the air lost all humidity.");
		  act(buf2, TRUE, ch, 0, 0, TO_ROOM);
	  } else if (GET_CLASS(ch) == CLASS_TITAN) {
      sprintf(buf2, "The ground shakes as %s %s %s.",
          AFF_FLAGGED(ch, AFF_INCOGNITO) ? "someone" : GET_NAME(ch),
          (AFF_FLAGGED(ch, AFF_FLY) || AFF_FLAGGED(ch, AFF_UNFURLED) ||
           AFF_FLAGGED(ch, AFF_HOVER)) ? "floats" :
          "leaves", dirs[dir]);
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
	  } else if (GET_CLASS(ch) == CLASS_DEMON) {
      sprintf(buf2, "%s silently phases out of view.",
          AFF_FLAGGED(ch, AFF_INCOGNITO) ? "someone" : GET_NAME(ch));
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
      } else if(!IS_NPC(ch)) {
      sprintf(buf2, "%s %s %s.", 
          AFF_FLAGGED(ch, AFF_INCOGNITO) ? "Someone" : GET_NAME(ch),
          (AFF_FLAGGED(ch, AFF_FLY) || AFF_FLAGGED(ch, AFF_UNFURLED) ||
           AFF_FLAGGED(ch, AFF_HOVER)) ? "floats" : 
          "leaves", dirs[dir]);
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);

      }
      else {
      sprintf(buf2, "%s %s %s.", 
          AFF_FLAGGED(ch, AFF_INCOGNITO) ? "Someone" : GET_NAME(ch),
          (AFF_FLAGGED(ch, AFF_FLY) || AFF_FLAGGED(ch, AFF_UNFURLED) ||
           AFF_FLAGGED(ch, AFF_HOVER)) ? "floats" : 
          leave_msg[(int)GET_RACE(ch)], dirs[dir]);
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
      }
    }
   }
  } else {
    if (GET_POS(GET_MOUNT(ch)) >= POS_STANDING) {
      sprintf(buf2, "%s rides %s on the back of %s.",
              AFF_FLAGGED(ch, AFF_INCOGNITO) ? "Someone" : GET_NAME(ch),
              dirs[dir], GET_NAME(GET_MOUNT(ch)));
      act(buf2, TRUE, ch, 0, 0, TO_ROOM);
    } else {
      send_to_char("Your mount isn't ready to move now.\r\n", ch);
      return 0;
    }
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_COURT) && GET_LEVEL(ch) <= 150 &&
      IS_AFFECTED(ch, AFF_SPEAK))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_SPEAK);

  if ((ROOM_FLAGGED(ch->in_room, ROOM_SALTWATER_FISH) ||
       ROOM_FLAGGED(ch->in_room, ROOM_FRESHWATER_FISH)) &&
      (PLR_FLAGGED(ch, PLR_FISHING) || PLR_FLAGGED(ch, PLR_FISH_ON))) {
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISHING);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_FISH_ON);
    send_to_char("\r\nYou pack up your fishing gear and move on.\r\n\r\n", ch);
  }

  /* see if an entry trigger disallows the move */
  if (!entry_mtrigger(ch))
    return 0;
  if (!enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
    return 0;


  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[dir]->to_room);
  if (GET_MOUNT(ch)) { 
    look_at_room(ch, 0);
    char_from_room(GET_MOUNT(ch));
    char_to_room(GET_MOUNT(ch), world[was_in].dir_option[dir]->to_room);
    act("$n rides in on the back of $N.", TRUE, ch, 0, GET_MOUNT(ch), TO_ROOM);
  } 
  else if (AFF_FLAGGED(ch, AFF_FOG_MIST_AURA))
  {
	  look_at_room(ch, 0);
	  act("The air takes on a slight chill as a rainy mist decends upon you.", TRUE, ch, 0, 0, TO_ROOM);
  } else {
    if (!AFF_FLAGGED(ch, AFF_SNEAK) && !AFF_FLAGGED(ch, AFF_SHADOW))
     if (!ROOM_FLAGGED(ch->in_room, ROOM_IMPEN)) {
      if (AFF_FLAGGED(ch, AFF_INCOGNITO)) {
        act("Someone has arrived.", TRUE, ch, 0, 0, TO_ROOM);
      } else if (GET_CLASS(ch) == CLASS_TITAN) {
        act("The ground shakes as $n arrives.", TRUE, ch, 0, 0, TO_ROOM);
      } else if (GET_CLASS(ch) == CLASS_VAMPIRE) {
        act("Shadows become reality as $n arrives.", TRUE, ch, 0, 0, TO_ROOM);
	  } else if (GET_CLASS(ch) == CLASS_DEMON) {
        act("$n steps forth from a column of hellfire!.", TRUE, ch, 0, 0, TO_ROOM);
      } else {
        act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
      }
    }
    if (ch->desc != NULL) {
      look_at_room(ch, 0);
    }

  }

  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_SCORCHED) &&
     (GET_LEVEL(ch) < LVL_IMMORT || GET_CLASS(ch) != CLASS_TITAN)) {
    send_to_char("\r\n/crThe earth is scalding hot here! You are being burnt!/c0\r\n",ch);
    GET_HIT(ch) -= (GET_LEVEL(ch) * 5);
  } 
  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_COURT) &&
      GET_LEVEL(ch) <= 150) {
    send_to_char("\r\nYou feel a hushed reverance as you enter this room.\r\n"
                 "You feel compelled to keep silent.\r\n", ch);
  }

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected) continue;
    if(IS_NPC(d->character) && MOB_FLAGGED(d->character, MOB_INSTAGGRO) &&
      (d->character->in_room == ch->in_room)) {
      set_fighting(d->character, ch); 
    }
  }

  if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_DEATH) &&
      GET_LEVEL(ch) < LVL_IMMORT) {
    log_death_trap(ch);
    death_cry(ch);
    extract_char(ch);
    return 0;
  }

  entry_memory_mtrigger(ch);
  if (!greet_mtrigger(ch, dir)) {
    char_from_room(ch);
    char_to_room(ch, was_in);
    look_at_room(ch, 0);
  } else greet_memory_mtrigger(ch);
 
  return 1;
}


int perform_move(struct char_data *ch, int dir, int need_specials_check)
{
  int was_in;
  struct follow_type *k, *next;
  struct obj_data *obj=NULL;
  struct room_direction_data *back=0;
  int other_room=0;

  if (IS_MOB(ch))
    if (!mob_can_move(ch, dir))
      return 0;
  
  if(!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AFK))
    TOGGLE_BIT_AR(PRF_FLAGS(ch), PRF_AFK);

  if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS)
    return 0;
  else if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
    send_to_char("Alas, you cannot go that way...\r\n", ch);
  else if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
	  if ((obj) || (dir >= 0)) {
		  if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_USE_DOORS)) {
			  if (DOOR_IS_OPENABLE(ch, obj, dir))
				  if (DOOR_IS_CLOSED(ch, obj, dir))
					  if (!DOOR_IS_PICKPROOF(ch, obj, dir)) {
						  if (!obj && ((other_room = EXIT(ch, dir)->to_room) != NOWHERE))
							  if ((back = world[other_room].dir_option[rev_dir[dir]]))
								  if (back->to_room != ch->in_room)
									  back = 0;
								  OPEN_DOOR(ch->in_room, obj, dir);
								  sprintf(buf, "%s opens the %s.", GET_NAME(ch), (obj) ? "$p" : (EXIT(ch, dir)->keyword ? "$F" : "door"));
								  if (!(obj) || (obj->in_room != NOWHERE))
									  act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, dir)->keyword, TO_ROOM);
								  sprintf(buf, "The %s is opened from the other side.\r\n", (back->keyword ? fname(back->keyword) : "door"));
								  if (world[EXIT(ch, dir)->to_room].people) {
									  act(buf, FALSE, world[EXIT(ch, dir)->to_room].people, 0, 0, TO_ROOM);
									  act(buf, FALSE, world[EXIT(ch, dir)->to_room].people, 0, 0, TO_CHAR);
								  }
					  }
		  }
	  }
    if (IS_AFFECTED(ch, AFF_MIST_FORM) || GET_LEVEL(ch) >= LVL_IMMORT) {
      act("$n's mist form allows $m to pass right through the door.\r\n",
           FALSE, ch, 0, 0, TO_ROOM);
      act("Your mist form allows you to pass right through the door.\r\n",
           FALSE, ch, 0, 0, TO_CHAR);
      return (do_simple_move(ch, dir, need_specials_check));
    }
    if (IS_AFFECTED(ch, AFF_ETHEREAL)) {
      act("$n's ethereal form allows $m to pass right through the door.\r\n",
           FALSE, ch, 0, 0, TO_ROOM);
      act("Your ethereal form allows you to pass right through the door.\r\n",
           FALSE, ch, 0, 0, TO_CHAR);
      return (do_simple_move(ch, dir, need_specials_check));
    }
    if (EXIT(ch, dir)->keyword) {
      sprintf(buf2, "The %s seems to be closed.\r\n",
              fname(EXIT(ch, dir)->keyword));
      send_to_char(buf2, ch);
    } else
      send_to_char("It seems to be closed.\r\n", ch);
  } else {
    if (!ch->followers) {
      return (do_simple_move(ch, dir, need_specials_check));
    }

    was_in = ch->in_room;
    if (!do_simple_move(ch, dir, need_specials_check))
      return 0;

    for (k = ch->followers; k; k = next) {
      next = k->next;
      if ((k->follower->in_room == was_in) &&
	  (GET_POS(k->follower) >= POS_STANDING)) {
        if(!FIGHTING(k->follower)) {
   	  act("You follow $N.\r\n", FALSE, k->follower, 0, ch, TO_CHAR);
  	  perform_move(k->follower, dir, 1);
        }
      }
    }
    if(weather_info.ground_snow) {	
    	crunchy_snow(ch);
    }
    return 1;
  }
  return 0;
}

int mob_can_move(struct char_data *ch, int dir){

  char target[256];

  if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
    return FALSE;

  /* printf("*** %s\n", GET_NAME(ch)); */

  sprintf(target, "%d", world[EXIT(ch, dir)->to_room].number);
  switch(GET_MOB_VNUM(ch)){
  case 6754:
    return (isname(target, "6730 6731 6732 6733 6734 6735 6736 6737 "
                  "6738 6739 6740 6741 6742 6743 6744 6745 6746 6747 "
                  "6748 6749 6750 6751 6752 6753")==TRUE);
    break;
  case 6903:
    return (isname(target, "6903 6933 6934")==TRUE);
    break;
  case 6904:
    return (isname(target, "6903 6934")==TRUE);
    break;
  case 6905:
    return (isname(target, "6913 6935")==TRUE);
    break;
  case 6912:
    return (isname(target, "6915 6916")==TRUE);
    break;
  case 6913:
    return (isname(target, "6915 6916")==TRUE);
    break;
  case 6914:
    return (isname(target, "6915 6916")==TRUE);
    break;
  case 1817: case 1818: case 1819:
    return (isname(target, "1811 1812 1813 1814 1815 1816 1817 1818 "
                           "1819 1820 1821 1822 1823")==TRUE);
    break;
  default:
    return TRUE;
  }
  return TRUE;
}

ACMD(do_move)
{
  /*
   * This is basically a mapping of cmd numbers to perform_move indices.
   * It cannot be done in perform_move because perform_move is called
   * by other functions which do not require the remapping.
   */
  perform_move(ch, cmd - 1, 0);
}


int find_door(struct char_data *ch, char *type, char *dir, char *cmdname)
{
  int door;

  if (*dir) {			/* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
      send_to_char("That's not a direction.\r\n", ch);
      return -1;
    }
    if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
	if (isname(type, EXIT(ch, door)->keyword))
	  return door;
	else {
	  sprintf(buf2, "I see no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return -1;
      } else
	return door;
    else {
      send_to_char("I really don't see how you can close anything there.\r\n", ch);
      return -1;
    }
  } else {			/* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to %s?\r\n", cmdname);
      send_to_char(buf2, ch);
      return -1;
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return door;

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return -1;
  }
}

int find_door_dbash(struct char_data *ch, char *type, char *dir)
{
  int door;

  if (*dir) {			/* a direction was specified */
    if ((door = search_block(dir, dirs, FALSE)) == -1) {	/* Partial Match */
      send_to_char("That's not a direction.\r\n", ch);
      return -1;
    }
    if (EXIT(ch, door))
      if (EXIT(ch, door)->keyword)
	if (isname(type, EXIT(ch, door)->keyword))
	  return door;
	else {
	  sprintf(buf2, "There is no %s there.\r\n", type);
	  send_to_char(buf2, ch);
	  return -1;
      } else
	return door;
    else {
      sprintf(buf, "There is no %s in that direction.\r\n", type);
	  send_to_char(buf, ch);
      return -1;
    }
  } else {			/* try to locate the keyword */
    if (!*type) {
      sprintf(buf2, "What is it you want to bash?\r\n");
      send_to_char(buf2, ch);
      return -1;
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (isname(type, EXIT(ch, door)->keyword))
	    return door;

    sprintf(buf2, "There doesn't seem to be %s %s here.\r\n", AN(type), type);
    send_to_char(buf2, ch);
    return -1;
  }
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;

  for (o = ch->carrying; o; o = o->next_content)
    if (GET_OBJ_VNUM(o) == key)
      return 1;

  if (GET_EQ(ch, WEAR_HOLD))
    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key)
      return 1;

  return 0;
}



#define NEED_OPEN	1
#define NEED_CLOSED	2
#define NEED_UNLOCKED	4
#define NEED_LOCKED	8

char *cmd_door[] =
{
  "open",
  "close",
  "unlock",
  "lock",
  "pick",
  "bash"
};

const int flags_door[] =
{
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_OPEN,
  NEED_CLOSED | NEED_LOCKED,
  NEED_CLOSED | NEED_UNLOCKED,
  NEED_CLOSED | NEED_LOCKED
};


#define EXITN(room, door)		(world[room].dir_option[door])
#define OPEN_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_CLOSED)))
#define LOCK_DOOR(room, obj, door)	((obj) ?\
		(TOGGLE_BIT(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
		(TOGGLE_BIT(EXITN(room, door)->exit_info, EX_LOCKED)))

#define DOOR_IS_UNLOCKED(ch, obj, door) ((obj) ? \
                        (!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
                        (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))

void do_doorcmd(struct char_data *ch, struct obj_data *obj, int door, int scmd)
{
  int other_room = 0;
  struct room_direction_data *back = 0;

  if(scmd == SCMD_DOORBASH)
    sprintf(buf, "$n %ses ", cmd_door[scmd]);
  else
    sprintf(buf, "$n %ss ", cmd_door[scmd]);
    if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
      if ((back = world[other_room].dir_option[rev_dir[door]]))
        if (back->to_room != ch->in_room)
    	  back = 0;

  switch (scmd) {
  case SCMD_OPEN:
  case SCMD_CLOSE:
    OPEN_DOOR(ch->in_room, obj, door);
    if (back)
      OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char(OK, ch);
    break;
  case SCMD_UNLOCK:
  case SCMD_LOCK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("*Click*\r\n", ch);
    break;
  case SCMD_DOORBASH:
    OPEN_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("Crash! Boom!\r\n", ch);
    strcpy(buf, "$n bashes down ");
    break;
  case SCMD_PICK:
    LOCK_DOOR(ch->in_room, obj, door);
    if (back)
      LOCK_DOOR(other_room, obj, rev_dir[door]);
    send_to_char("The lock quickly yields to your skills.\r\n", ch);
    strcpy(buf, "$n skillfully picks the lock on ");
    break;
  }

  /* Notify the room */
  sprintf(buf + strlen(buf), "%s%s.", ((obj) ? "" : "the "), (obj) ? "$p" :
	  (EXIT(ch, door)->keyword ? "$F" : "door"));
  if (!(obj) || (obj->in_room != NOWHERE))
    act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);

  /* Notify the other room */
  if (((scmd == SCMD_OPEN) || (scmd == SCMD_CLOSE) ||
       (scmd == SCMD_DOORBASH)) && back) {
    sprintf(buf, "The %s is %s%s from the other side.\r\n",
	 (back->keyword ? fname(back->keyword) : "door"), cmd_door[scmd],
	    (scmd == SCMD_CLOSE) ? "d" : "ed");
    if (world[EXIT(ch, door)->to_room].people) {
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
      act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
    }
  }
}

int ok_pick(struct char_data *ch, int keynum, int pickproof, int scmd)
{
  int percent;

  percent = number(1, 101);

  if (scmd == SCMD_PICK) {
    if (keynum < 0)
      send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
    else if (pickproof)
      send_to_char("It resists your attempts to pick it.\r\n", ch);
    else if (percent > GET_SKILL(ch, SKILL_PICK_LOCK))
      send_to_char("You failed to pick the lock.\r\n", ch);
    else
      return (1);
    return (0);
  }
  return (1);
}

int ok_doorbash(struct char_data *ch, int keynum, int pickproof, int scmd)
{
  int percent;

  percent = number(1, 101);

  if (scmd == SCMD_DOORBASH) {
    if (pickproof)      {
      send_to_char("You cannot get it open. It is too strong. Ouch.\r\n", ch);
      GET_HIT(ch) = MAX(1, GET_HIT(ch) - 20);
    } else if (percent > GET_SKILL(ch, SKILL_DOORBASH)) {
     send_to_char("You bruise your shoulder attempting to get it open.\r\n",ch);
      GET_HIT(ch) = MAX(1, GET_HIT(ch) - number(1,20));
    } else {
      send_to_char("You bash the door open.\r\n", ch );
      return (1);
    }
    return (0);
  }
  return (1);
}

#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
/*
#define DOOR_IS_UNLOCKED(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_LOCKED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)))
*/
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))

#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))

ACMD(do_gen_door)
{
  int door = -1, keynum;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct obj_data *obj = NULL;
  struct char_data *victim = NULL;

  skip_spaces(&argument);
  if (!*argument) {
    sprintf(buf, "%s what?\r\n", cmd_door[subcmd]);
    send_to_char(CAP(buf), ch);
    return;
  }
  two_arguments(argument, type, dir);
  if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
    door = find_door(ch, type, dir, cmd_door[subcmd]);

  if ((obj) || (door >= 0)) {
    keynum = DOOR_KEY(ch, obj, door);
    if (!(DOOR_IS_OPENABLE(ch, obj, door)))
      act("You can't $F that!", FALSE, ch, 0, cmd_door[subcmd], TO_CHAR);
    else if (!DOOR_IS_OPEN(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_OPEN))
      send_to_char("But it's already closed!\r\n", ch);
    else if (!DOOR_IS_CLOSED(ch, obj, door) &&
	     IS_SET(flags_door[subcmd], NEED_CLOSED))
      send_to_char("But it's currently open!\r\n", ch);
    else if (!(DOOR_IS_LOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_LOCKED))
      send_to_char("Oh.. it wasn't locked, after all..\r\n", ch);
    else if (!(DOOR_IS_UNLOCKED(ch, obj, door)) &&
	     IS_SET(flags_door[subcmd], NEED_UNLOCKED))
      send_to_char("It seems to be locked.\r\n", ch);
    else if (!has_key(ch, keynum) && (GET_LEVEL(ch) < LVL_ADMIN) &&
	     ((subcmd == SCMD_LOCK) || (subcmd == SCMD_UNLOCK)))
      send_to_char("You don't seem to have the proper key.\r\n", ch);
    else if ((ok_pick(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door),
subcmd)) && (ok_doorbash(ch, keynum, DOOR_IS_PICKPROOF(ch, obj, door),
subcmd)))
      do_doorcmd(ch, obj, door, subcmd);
  }
  return;
}

ACMD(do_doorbash)
{
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *back=0;
  struct obj_data *obj=NULL;
  int percent=0,prob=0,other_room=0,door=-1;
  if (!GET_SKILL(ch, SKILL_DOORBASH)) {
    send_to_char("Huh?!?\r\n", ch);
    return;
  }
  skip_spaces(&argument);
  if (!*argument) {
	  sprintf(buf, "You might as well bash your head against a tree for all good it will do?!??\r\n");
	  send_to_char(CAP(buf), ch);
	  return;
  }
  percent = number(1, 101);	/* 101% is a complete failure */
  prob = GET_SKILL(ch, SKILL_DOORBASH);
  if (percent > prob) {
	  send_to_char("A sudden, blinding pain sends you reeling uncounscious!\r\n", ch);
//	  send_to_char("Perhaps bashing the door would be easier than bashing the wall around it?!?\r\n", ch);
	  GET_HIT(ch) -= (GET_MAX_HIT(ch)/10);
	  update_pos(ch);
	  if (GET_HIT(ch) <= 0)
		  send_to_char("Crack!  There goes your neck and you fade out permanently!\r\n", ch);
	  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
	  return;
  } else {
	  two_arguments(argument, type, dir);
	  door = find_door_dbash(ch, type, dir);
	  if ((obj) || (door >= 0)) {
		  if (!(DOOR_IS_OPENABLE(ch, obj, door)))
			  act("There is nothing to bash!", FALSE, ch, 0, NULL, TO_CHAR);
		  else if (!DOOR_IS_CLOSED(ch, obj, door) && IS_SET(flags_door[SCMD_OPEN], NEED_CLOSED))
			  send_to_char("But it's currently open!\r\n", ch);
		  else if (!DOOR_IS_PICKPROOF(ch, obj, door))
		  {
			if (!obj && ((other_room = EXIT(ch, door)->to_room) != NOWHERE))
				if ((back = world[other_room].dir_option[rev_dir[door]]))
					if (back->to_room != ch->in_room)
						back = 0;
					OPEN_DOOR(ch->in_room, obj, door);
					sprintf(buf, "The %s shatters before your mighty strength!", (obj) ? "$p" : (EXIT(ch, door)->keyword ? "$F" : "door"));
					act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_CHAR);
					sprintf(buf, "The %s shatters before %s's mighty strength!", (obj) ? "$p" : (EXIT(ch, door)->keyword ? "$F" : "door"), GET_NAME(ch));
					if (!(obj) || (obj->in_room != NOWHERE))
						act(buf, FALSE, ch, obj, obj ? 0 : EXIT(ch, door)->keyword, TO_ROOM);
					sprintf(buf, "The %s is bashed open from the other side.\r\n", (back->keyword ? fname(back->keyword) : "door"));
					if (world[EXIT(ch, door)->to_room].people) {
						act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_ROOM);
						act(buf, FALSE, world[EXIT(ch, door)->to_room].people, 0, 0, TO_CHAR);
					} 
		  }
		  else if (DOOR_IS_PICKPROOF(ch, obj, door)) {
			  send_to_char("A sudden, blinding pain sends you reeling uncounscious!\r\n", ch);
			  GET_HIT(ch) -= (GET_MAX_HIT(ch)/10);
			  update_pos(ch);
			  if (GET_HIT(ch) <= 0)
				  send_to_char("Crack!  There goes your neck and you fade out permanently!\r\n", ch);
			  WAIT_STATE(ch, PULSE_VIOLENCE + 2);
			  return;
		  }
	  }
  }
  return;
}

ACMD(do_enter)
{
  struct obj_data *obj = NULL;
  int door;

  if(FIGHTING(ch)) { 
     send_to_char("You're kinda busy right now....\r\n", ch); 
     return;
  }

  one_argument(argument, arg);

  if (subcmd == SCMD_JUMP) {
    if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg), arg);
      send_to_char(buf, ch);
      return;
    }   
    if (GET_OBJ_TYPE(obj) == ITEM_PORTAL) {
      if (real_room(GET_OBJ_VAL(obj, 0)) != NOWHERE) {
        act("\r\n/ccYou leap into $p./c0\r\n\r\n", FALSE, ch, obj, 0, TO_CHAR);
        act("\r\n/cc$n leaps into $p./c0", FALSE, ch, obj, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, real_room(GET_OBJ_VAL(obj, 0)));
        act("/cc$n leaps from a magical portal./c0", FALSE, ch, 0, 0, TO_ROOM);
      } else if (real_room(GET_OBJ_VAL(obj, 1)) != NOWHERE) {
        send_to_char("You leap into a magical portal.\r\n\r\n", ch);
        act("$n leaps into a magical portal.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, real_room(GET_OBJ_VAL(obj, 1)));
        act("$n leaps from a magical portal.", FALSE, ch, 0, 0, TO_ROOM);
      }
      look_at_room(ch, 1);
      return;
    }
    send_to_char("You cannot jump into that!\r\n", ch);
    return;
  }
  if (*buf) {			/* an argument was supplied, search for door
				 * keyword */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->keyword)
	  if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    sprintf(buf2, "There is no %s here.\r\n", buf);
    send_to_char(buf2, ch);
  } else if (IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are already indoors.\r\n", ch);
  else {
    /* try to locate an entrance */
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	      IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("You can't seem to find anything to enter.\r\n", ch);
  }
}


ACMD(do_leave)
{
  int door;

  if (!IS_SET_AR(ROOM_FLAGS(ch->in_room), ROOM_INDOORS))
    send_to_char("You are outside.. where do you want to go?\r\n", ch);
  else {
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (EXIT(ch, door))
	if (EXIT(ch, door)->to_room != NOWHERE)
	  if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED) &&
	    !IS_SET_AR(ROOM_FLAGS(EXIT(ch, door)->to_room), ROOM_INDOORS)) {
	    perform_move(ch, door, 1);
	    return;
	  }
    send_to_char("I see no obvious exits to the outside.\r\n", ch);
  }
}


ACMD(do_stand)
{
	if (AFF_FLAGGED(ch, AFF_BURROW))
	{
		send_to_char("Use rise to stand up.\r\n", ch);
		return;
	}
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SITTING:
    act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n clambers to $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_RESTING:
    act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting, and clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Do you not consider fighting as standing?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and put your feet on the ground.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and puts $s feet on the ground.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
    break;
  }
}


ACMD(do_sit)
{
  if (GET_MOUNT(ch)) {
    act("$n dismounts $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_ROOM);
    act("You dismount $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
     GET_RIDER(GET_MOUNT(ch)) = NULL;
     GET_MOUNT(ch) = NULL;
  }
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You're sitting already.\r\n", ch);
    break;
  case POS_RESTING:
    act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and sit down.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and sits down.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_rest)
{
  if (GET_MOUNT(ch)) {
    act("$n dismounts $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_ROOM);
    act("You dismount $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
     GET_RIDER(GET_MOUNT(ch)) = NULL;
     GET_MOUNT(ch) = NULL;
  }
  switch (GET_POS(ch)) {
  case POS_STANDING:
    act("You sit down and rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_SITTING:
    act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    break;
  case POS_RESTING:
    act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_SLEEPING:
    act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
    break;
  case POS_FIGHTING:
    act("Rest while fighting?  Are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
    break;
  default:
    act("You stop floating around, and stop to rest your tired bones.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
    break;
  }
}


ACMD(do_sleep)
{
  if (AFF_FLAGGED(ch, AFF_INSOMNIA)) {
     send_to_char("You cannot get to sleep... damn insomnia!!!\r\n", ch);
     return;
  }
  if (GET_MOUNT(ch)) {
    act("$n dismounts $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_ROOM);
    act("You dismount $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
     GET_RIDER(GET_MOUNT(ch)) = NULL;
     GET_MOUNT(ch) = NULL;
  }
  switch (GET_POS(ch)) {
  case POS_STANDING:
  case POS_SITTING:
  case POS_RESTING:
    send_to_char("You go to sleep.\r\n", ch);
    act("$n lies down and falls asleep.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  case POS_SLEEPING:
    send_to_char("You are already sound asleep.\r\n", ch);
    break;
  case POS_FIGHTING:
    send_to_char("Sleep while fighting?  Are you MAD?\r\n", ch);
    break;
  default:
    act("You stop floating around, and lie down to sleep.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops floating around, and lie down to sleep.",
	TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SLEEPING;
    break;
  }
}


ACMD(do_wake)
{
  struct char_data *vict;
  int self = 0;

  one_argument(argument, arg);
  if (*arg) {
    if (GET_POS(ch) == POS_SLEEPING)
      send_to_char("Maybe you should wake yourself up first.\r\n", ch);
    else if ((vict = get_char_room_vis(ch, arg)) == NULL)
      send_to_char(NOPERSON, ch);
    else if (vict == ch)
      self = 1;
    else if (GET_POS(vict) > POS_SLEEPING)
      act("$E is already awake.", FALSE, ch, 0, vict, TO_CHAR);
    else if (AFF_FLAGGED(vict, AFF_SLEEP))
      act("You can't wake $M up!", FALSE, ch, 0, vict, TO_CHAR);
    else if (GET_POS(vict) < POS_SLEEPING)
      act("$E's in pretty bad shape!", FALSE, ch, 0, vict, TO_CHAR);
    else {
      act("You wake $M up.", FALSE, ch, 0, vict, TO_CHAR);
      act("You are awakened by $n.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
      GET_POS(vict) = POS_SITTING;
    }
    if (!self)
      return;
  }
  if (AFF_FLAGGED(ch, AFF_SLEEP))
    send_to_char("You can't wake up!\r\n", ch);
  else if (GET_POS(ch) > POS_SLEEPING)
    send_to_char("You are already awake...\r\n", ch);
  else {
    send_to_char("You awaken, and sit up.\r\n", ch);
    act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_SITTING;
  }
}


ACMD(do_follow)
{
  struct char_data *leader;

  void stop_follower(struct char_data *ch);
  void add_follower(struct char_data *ch, struct char_data *leader);

  one_argument(argument, buf);

  if (*buf) {
    if (!(leader = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Whom do you wish to follow?\r\n", ch);
    return;
  }

  if (ch->master == leader) {
    act("You are already following $M.", FALSE, ch, 0, leader, TO_CHAR);
    return;
  }
  if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master)) {
    act("But you only feel like following $N!", FALSE, ch, 0, ch->master, TO_CHAR);
  } else {			/* Not Charmed follow person */
    if (leader == ch) {
      if (!ch->master) {
	send_to_char("You are already following yourself.\r\n", ch);
	return;
      }
      stop_follower(ch);
    } else {
      if(PRF_FLAGGED(leader, PRF_NOFOLLOW)) {
        send_to_char("They do not wish to be followed.\r\n", ch);
        return;
      }
      if (circle_follow(ch, leader)) {
	act("Sorry, but following in loops is not allowed.", FALSE, ch, 0, 0, TO_CHAR);
	return;
      }
      if (ch->master)
	stop_follower(ch);
      REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
      add_follower(ch, leader);
    }
  }
}

ACMD(do_recall)
{

  /* someone would try... */
  if (PLR_FLAGGED(ch, PLR_FROZEN)) {
    send_to_char("Nice try.\r\n", ch);
    return;
  }

  if (GET_CLASS(ch) == CLASS_VAMPIRE) {
    send_to_char("Recall? What is this mortal coil you speak of?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) > 10) {
    send_to_char("Your prayers fall on deaf ears.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
    send_to_char("You can't recall from the arena.\r\n", ch);
    return;
  }

  if (FIGHTING(ch)) {
    send_to_char("No recall in a fight, sorry.\r\n", ch);
    return;
  }
  
  act("$n says a quick prayer and disappears.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(get_default_room(ch)));
  act("$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);

}

ACMD(do_end)
{
  if (!ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use that command here.\r\n", ch);
    return;
  }

  act("$n decides $e's had enough.", TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(get_default_room(ch)));
  act("$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
  look_at_room(ch, 0);  
}

ACMD(do_incognito)
{

  if (AFF_FLAGGED(ch, AFF_INCOGNITO)) {
    send_to_char("You stop travelling incognito.\r\n", ch);
    act("$n identifies himself.", FALSE, ch, 0, 0, TO_ROOM);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INCOGNITO);
    return;
  }

  if (!GET_SKILL(ch, SKILL_INCOGNITO)) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }

  act("You attempt to disguise your identity.", FALSE, ch, 0, 0, TO_CHAR);

  if (GET_SKILL(ch, SKILL_INCOGNITO) > number(0, 101))
    SET_BIT_AR(AFF_FLAGS(ch), AFF_INCOGNITO);

}

ACMD(do_mount)
{
	int mtp = 0;
  int chance;
  struct char_data *mount;

  if (!(chance = GET_SKILL(ch, SKILL_RIDE))) {
    send_to_char("You have no idea how.\r\n", ch);
    return;
  }
  one_argument(argument, buf);
  if (*buf) {
    if (!(mount = get_char_room_vis(ch, buf))) {
      send_to_char(NOPERSON, ch);
      return;
    }
  } else {
    send_to_char("Mount what?\r\n", ch);
    return;
  }
  if (!IS_NPC(mount)) {
    send_to_char("Players don't appreciate that sort of behavior.\r\n", ch);
    return;
  }
  if (GET_MOUNT(ch)) {
    send_to_char("You are already riding.\r\n", ch);
    return;
  }
  if (GET_RIDER(mount)) {
    send_to_char("But it's already being ridden.\r\n", ch);
    return;
  }
  if (GET_RIDER(ch)) {
    send_to_char("This shouldn't happen.\r\n", ch);
    return;
  }
  if (GET_MOUNT(mount)) {
    send_to_char("That can't happen.\r\n", ch);
    return;
  }
  if (mount->mob_specials.mount_type > 0)
  {
	  mtp = mount->mob_specials.mount_type;
	  if (IS_GOOD(ch) && mountt_disallow_align(mtp, MOUNTT_GOOD) == TRUE)
	  {
		  send_to_char("You are to saintly.\r\n", ch);
		  return;
	  }
	  if (IS_NEUTRAL(ch) && mountt_disallow_align(mtp, MOUNTT_NEUTRAL) == TRUE)
	  {
		  send_to_char("This mount is only for thoose standing between good and evil.\r\n", ch);
		  return;
	  }
	  if (IS_EVIL(ch) && mountt_disallow_align(mtp, MOUNTT_EVIL) == TRUE)
	  {
		  send_to_char("You are to evil.\r\n", ch);
		  return;
	  }
	  if (mountt_disallow_race(mtp, GET_RACE(ch)) == TRUE)
	  {
		  send_to_char("This creature can not carry you.\r\n", ch);
		  return;
	  }
	  if (mountt_disallow_class(mtp, GET_CLASS(ch)) == TRUE)
	  {
		  send_to_char("This creature can not carry you.\r\n", ch);
		  return;
	  }
	  if (GET_CLASS(ch) >= CLASS_VAMPIRE && mountt_disallow_pty(mtp, MOUNTT_IMMORT) == TRUE)
	  {
		  send_to_char("This creature can not carry immortals.\r\n", ch);
		  return;
	  }
	  if (GET_CLASS(ch) > CLASS_BARD && GET_CLASS(ch) < CLASS_VAMPIRE && mountt_disallow_pty(mtp, MOUNTT_REMORT) == TRUE)
	  {
		  send_to_char("This creature can not carry remorts.\r\n", ch);
		  return;
	  }
	  if (GET_CLASS(ch) > CLASS_BARD && mountt_disallow_pty(mtp, MOUNTT_MORT) == TRUE)
	  {
		  send_to_char("This creature can not carry mortals.\r\n", ch);
		  return;
	  }

  }
  if (MOB_FLAGGED(mount, MOB_MOUNT)) {
    if ((chance > number(1, 101)) || PRF_FLAGGED(ch, PRF_NOHASSLE)) {
      GET_MOUNT(ch) = mount;
      GET_RIDER(mount) = ch;
      act("$n swings $s leg over $N's back.", FALSE, ch, 0, mount, TO_ROOM);
      act("You swing your leg over $N's back.", FALSE, ch, 0, mount, TO_CHAR);
      return;
    } else  {
      act("$n swings $s leg over $N's back and falls off the other side.",
          FALSE, ch, 0, mount, TO_ROOM);
      act("You swing your leg over $N's back, lose your balance and fall off.",
          FALSE, ch, 0, mount, TO_CHAR);
      GET_HIT(ch)--;
      GET_POS(ch) = POS_SITTING;
      return;
    }
  } else {
    send_to_char("That mob can't be ridden.\r\n", ch);
    return;
  }
}

ACMD(do_dismount)
{
  if (!GET_MOUNT(ch)) {
    send_to_char("But you're not riding?\r\n", ch);
    return;
  }
  act("$n dismounts $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_ROOM);
  act("You dismount $N.", FALSE, ch, 0, GET_MOUNT(ch), TO_CHAR);
  GET_RIDER(GET_MOUNT(ch)) = NULL;
  GET_MOUNT(ch) = NULL;
}

#define DRAG_MOVE_COST 30 /* move cost to drag a corpse one room */

ACMD(do_drag) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  struct obj_data *corpse;
  struct char_data *tmp_char;
  int corpse_dotmode, door, need_movement;

  two_arguments(argument, arg1, arg2);
  corpse_dotmode = find_all_dots(arg1);

  if (!*arg1)
    send_to_char("Drag what where?\r\n", ch);
  else if (corpse_dotmode != FIND_INDIV)
    send_to_char("You can only drag one corpse at a time.\r\n", ch);
  else if (!*arg2)
    send_to_char("You must specify a direction.\r\n", ch);
  else {
    generic_find(arg1, FIND_OBJ_ROOM, ch, &tmp_char, &corpse);

    if (!corpse) {
      sprintf(buf, "You don't see %s %s here.\r\n", AN(arg1), arg1);
      send_to_char(buf, ch);

    /* if you remove this check, players will be able to drag    */
    /* fountains, etc. :)  if you want to allow that sort of     */
    /* thing, you might want to check for !take flags or weight. */
    } else if (!IS_CORPSE(corpse))
      send_to_char("You can only drag corpses.\r\n", ch);
    else if ((door = search_block(arg2, dirs, FALSE)) < 0)
      send_to_char("That's not a direction.\r\n", ch);
    else {
      need_movement = (movement_loss[SECT(ch->in_room)] +
                       movement_loss[SECT(EXIT(ch, door)->to_room)]) / 2;
      need_movement += DRAG_MOVE_COST;

      if (!IS_NPC(ch) && (GET_LEVEL(ch) < LVL_IMMORT) &&
         (GET_MOVE(ch) < need_movement)) {
        act("You are too exhausted to drag $p with you!",
            FALSE, ch, corpse, NULL, TO_CHAR);
      } else if (perform_move(ch, door, 0)) {
        if (!IS_NPC(ch) && (GET_LEVEL(ch) < LVL_IMMORT))
          GET_MOVE(ch) -= DRAG_MOVE_COST;
        obj_from_room(corpse);
        obj_to_room(corpse, ch->in_room);
        act("You drag $p with you.", FALSE, ch, corpse, NULL, TO_CHAR);
        act("$n drags $p with $m.", FALSE, ch, corpse, NULL, TO_ROOM);
      }

    }

  }
}

ACMD(do_speedwalk) {
  int dir, r;

  for (r = 1; *argument && r; argument++) {
    while (*argument == ' ')
      ++argument;
    
    switch (*argument) {
      case 'N':
      case 'n':
        dir = NORTH;
        break;
      case 'E':
      case 'e':
        dir = EAST;
        break;
      case 'S':
      case 's':
        dir = SOUTH;
        break;
      case 'W':
      case 'w':
        dir = WEST;
        break;
      case 'U':
      case 'u':
        dir = UP;
        break;
      case 'D':
      case 'd':
        dir = DOWN;
        break;
      default:
        send_to_char("Alas, you can't go that way.\r\n", ch);
        return;
        break;
    }    
    if (!PRF_FLAGGED(ch, PRF_BRIEF))
      SET_BIT_AR(PRF_FLAGS(ch), PRF_BRIEF);
	if (!(GET_POS(ch) == POS_FIGHTING)) {
		r = perform_move(ch, dir, 1);
		if (r && *(argument + 1))
			send_to_char("\r\n", ch);
		WAIT_STATE(ch, 5);
	}
	else if (GET_POS(ch) == POS_FIGHTING) {
		send_to_char("No way!  You're fighting for your life!\r\n", ch);
		REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_BRIEF);
		return;
	}
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_BRIEF);
  }
}

/* this is where most of the ship commands will land */
 
struct obj_data *find_ship_by_vnum(int vnum) {
        extern struct obj_data * object_list;
        struct obj_data * i;
        for (i = object_list; i; i = i->next)
                if (GET_OBJ_TYPE(i) == ITEM_SHIP)
                        if (GET_OBJ_VNUM(i) == vnum)
                                return i;
        return 0;
}

ACMD(do_steer)
{
 int dir, ship_to_room;
 struct obj_data *tiller, *ship;
 room_rnum was_in;
 
  if (!(tiller = get_obj_in_list_type(ITEM_TILLER, world[ch->in_room].contents))) {
     send_to_char("There's nothing to steer with here.\r\n", ch);
     return;
     }
  if (!(ship = find_ship_by_vnum(GET_OBJ_VAL(tiller, 0)))) {
     send_to_char("Ship object missing.  Contact an immortal!\r\n", ch);
     return;
  }
 
  one_argument(argument, arg);
 
  if (!*arg) {
    send_to_char("Which direction do you wish to steer?\r\n", ch);
    return;
  }
// extern char *AEdirs[];
  if (((dir = search_block(arg, dirs, FALSE)) == -1) &&      /* Partial Match */
       ((dir = search_block(arg, AEdirs, FALSE)) == -1)) {
    send_to_char("That's not a direction.\r\n", ch);
    return;
  }


  if (!EXIT(ship, dir) || EXIT(ship, dir)->to_room == NOWHERE) {
   send_to_char("You can't sail in that direction.\r\n", ch);
   return;
  }
  if (EXIT_FLAGGED(EXIT(ship, dir), EX_CLOSED)) {
  if (EXIT(ch, dir)->keyword) {
    if (IS_SET(EXIT(ch, dir)->exit_info, EX_HIDDEN))
      send_to_char("\r\n/cYYou found a secret exit!/c0\r\n\r\n", ch);
    sprintf(buf2, "The %s seems to be closed.\r\n", fname(EXIT(ch, dir)->keyword));
    send_to_char(buf2, ch);
  } else {
    if (IS_SET(EXIT(ch, dir)->exit_info, EX_HIDDEN))
      send_to_char("\r\n/cYYou found a secret exit!/c0\r\n\r\n", ch);
    send_to_char("It seems to be closed.\r\n", ch);
    return;
    }
  }
 
  if ((SECT(EXIT(ship, dir)->to_room) != SECT_WATER_SWIM) &&
      (SECT(EXIT(ship, dir)->to_room) != SECT_WATER_NOSWIM) &&
      (SECT(EXIT(ship, dir)->to_room) != SECT_OCEAN)) {
    send_to_char("You'd run aground if you sailed in that direction!\r\n", ch);
    return;
  }

  sprintf(buf, "%s sails %s.\n\r", ship->short_description, dirs[dir]);
  send_to_room(buf, ship->in_room);
  ship_to_room = world[ship->in_room].dir_option[dir]->to_room;
  sprintf(buf, "%s sails %s to %s\r\n", ship->short_description, dirs[dir], world[ship_to_room].name);
  send_to_room(buf, tiller->in_room);
 
  was_in = ship->in_room;
  obj_from_room(ship);
  obj_to_room(ship, world[was_in].dir_option[dir]->to_room);
 
  sprintf(buf, "%s sails in.\r\n", ship->short_description);
  send_to_room(buf, ship->in_room);
 
}

ACMD(do_board)
{
 struct obj_data *ship;
 
  one_argument(argument, arg);
 
  if (!*arg) {
    send_to_char("What do you wish to board?\r\n", ch);
    return;
  }
 
  if (!(ship = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
    send_to_char("That ship isn't here.\r\n", ch);
    return;
  }
 
  if (GET_OBJ_TYPE(ship) != ITEM_SHIP) {
    send_to_char("You can't board that!\r\n", ch);
    return;
  }

   act("You board $p.", TRUE, ch, ship, 0, TO_CHAR);
   act("$n boards $p", TRUE, ch, ship, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, real_room(GET_OBJ_VAL(ship, 1)));
   act("$n comes aboard.", TRUE, ch, 0, 0, TO_ROOM);
 
   if (ch->desc != NULL)
     look_at_room(ch, 0);
 
   entry_memory_mtrigger(ch);
   greet_mtrigger(ch, -1);
   greet_memory_mtrigger(ch);
}

ACMD(do_disembark)
{
 extern struct obj_data * object_list;
 struct obj_data * i, *ship, *tiller = NULL;
 int j;
 bool found = FALSE;

    for (i = object_list; i; i = i->next) {
       if (GET_OBJ_TYPE(i) == ITEM_TILLER) {
         for (j = GET_OBJ_VAL(i, 1); j <= GET_OBJ_VAL(i, 2); j++) {
            if (IN_ROOM(ch) == real_room(j)) {
              found = TRUE;
              tiller = i;
            }
         }
       }
    }
 
    if (!found) {
      send_to_char("You aren't even aboard a ship!\r\n", ch);
      return;
    }
    if (!(ship = find_ship_by_vnum(GET_OBJ_VAL(tiller, 0)))) {
       send_to_char("Ship object missing.  Contact an immortal!\r\n", ch);
       return;
    }
 
    if (IN_ROOM(ch) != real_room(GET_OBJ_VAL(ship, 1))) {
      send_to_char("You can't disembark from here.\r\n", ch);
      return;
    }

    if ((SECT(ship->in_room) == SECT_WATER_NOSWIM) && !has_boat(ch)) {
      send_to_char("You need a boat to go there.\r\n", ch);
      return;
    }
 
   act("You disembark $p.", TRUE, ch, ship, 0, TO_CHAR);
   act("$n disembark $p", TRUE, ch, ship, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, ship->in_room);
   act("$n disembarks $p.", TRUE, ch, ship, 0, TO_ROOM);

  if (ch->desc != NULL)
    look_at_room(ch, 0);

   entry_memory_mtrigger(ch);
   greet_mtrigger(ch, -1);
   greet_memory_mtrigger(ch);
 
 
}

ACMD(do_lookout)
{
 extern struct obj_data * object_list;
 struct obj_data * i, *tiller = NULL, *ship;
 int j;
 room_rnum orig_location;
 bool found = FALSE;
 
    for (i = object_list; i; i = i->next) {
       if (GET_OBJ_TYPE(i) == ITEM_TILLER) {
         for (j = GET_OBJ_VAL(i, 1); j <= GET_OBJ_VAL(i, 2); j++) {
            if (IN_ROOM(ch) == real_room(j)) {
              found = TRUE;
              tiller = i;
            }
         }
       }
    }
 
    if (!found) {
      send_to_char("You aren't even aboard a ship!\r\n", ch);
      return;
    }
    if (!(ship = find_ship_by_vnum(GET_OBJ_VAL(tiller, 0)))) {
       send_to_char("Ship object missing.  Contact an immortal!\r\n", ch);
       return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_INDOORS) &&
       !(tiller = get_obj_in_list_type(ITEM_TILLER, world[ch->in_room].contents))) {
       send_to_char("You can't look out from here.\r\n", ch);
       return;
    }

   orig_location = ch->in_room;
   act("You look out around $p.", TRUE, ch, ship, 0, TO_CHAR);
   act("$n looks out around $p", TRUE, ch, ship, 0, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, ship->in_room);
   if (ch->desc != NULL)
     look_at_room(ch, 0);
   char_from_room(ch);
   char_to_room(ch, orig_location);
 
}

void perform_quickdraw(struct char_data *ch, struct char_data *victim, int scmd)
{
 // struct obj_data *wielded;
 
  if (scmd == SCMD_PRACTICE_QUICKDRAW) {

    switch (GET_POS(ch)) {
      case POS_DEAD:
        send_to_char("Maybe you'll be quicker in your next life...\r\n", ch);
        break;
      case POS_MORTALLYW:
      case POS_INCAP:
      case POS_STUNNED:
        send_to_char("You are unable to do much except bleed profusely.\r\n", ch);
        break;
      case POS_SLEEPING:
        send_to_char("You awaken suddenly.\r\n", ch);
        act("$n awakens with a start.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        send_to_char("You leap to your feet and stand ready to fight!\r\n", ch);
        act("$n leaps to $s feet, ready to fight!", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        break;
      case POS_RESTING:
      case POS_SITTING:
        send_to_char("You leap to your feet and stand ready to fight!\r\n", ch);
        act("$n leaps to $s feet, ready to fight!", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        break;
      case POS_FIGHTING:
        send_to_char("The battle has already begun!\r\n", ch);
        break;
      case POS_STANDING:
        send_to_char("You stand ready to fight!\r\n", ch);
        act("$n stands poised and growling!", TRUE, ch, 0, 0, TO_ROOM);
        break;
      default:
        mudlog("ERROR:: No valid POS for do_quickdraw", BRF, LVL_IMMORT, TRUE);
        break;
    }

  } else if (scmd == SCMD_REAL_QUICKDRAW) {
 
    switch (GET_POS(ch)) {
      case POS_DEAD:
        send_to_char("Maybe you'll be quicker in your next life...\r\n", ch);
        break;
      case POS_MORTALLYW:
      case POS_INCAP:
      case POS_STUNNED:
        send_to_char("You are unable to do much except bleed profusely.\r\n", ch);
        break;
      case POS_SLEEPING:
        send_to_char("Your sixth sense awakens you suddenly!\r\n", ch);
        act("$n awakens with a start and looks around!", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        send_to_char("You leap to your feet and stand ready to fight!\r\n", ch);
        act("$n leaps to $s feet, ready to fight!", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        sprintf(buf, "You sense %s's evil intent and attack!\r\n", GET_NAME(victim));
        send_to_char(buf, ch);
        act("$n senses $N's evil intent, spins and attacks $M!", TRUE, ch, 0, victim, TO_ROOM);
        hit(ch, victim, TYPE_UNDEFINED);
        break;
      case POS_RESTING:
      case POS_SITTING:
        send_to_char("You leap to your feet and stand ready to fight!\r\n", ch);
        act("$n leaps to $s feet, ready to fight!", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_STANDING;
        sprintf(buf, "You sense %s's evil intent and attack!\r\n", GET_NAME(victim));
        send_to_char(buf, ch);
        act("$n senses $N's evil intent, spins and attacks $M!", TRUE, ch, 0, victim, TO_ROOM);
        hit(ch, victim, TYPE_UNDEFINED);
        break;
      case POS_FIGHTING:
        send_to_char("You've already drawn your weapons!\r\n", ch);
        break;
      case POS_STANDING:
        sprintf(buf, "You sense %s's evil intent and attack!\r\n", GET_NAME(victim));
        send_to_char(buf, ch);
        act("$n senses $N's evil intent, spins and attacks $M!", TRUE, ch, 0, victim, TO_ROOM);
        hit(ch, victim, TYPE_UNDEFINED); 
        break;
      default:
        mudlog("ERROR:: No valid POS for do_quickdraw", BRF, LVL_IMMORT, TRUE);
        break;
    }
  }

}
