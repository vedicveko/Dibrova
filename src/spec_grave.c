/*
***********************************************************************
*  FILE: spec_grave.c                                                 * 
*  USAGE: Specials module for areas.                                  *
*                                                                     *
*  All rights reserved, see license.doc for more information.         *
*                                                                     *
*  This file is based on Diku-MUD Copyright (C) 1990, 1991.           *
***********************************************************************
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"

/*   external vars  */

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;

/********************************************************************/

SPECIAL(grave_undertaker)
{
  if ((event != EVENT_PULSE) || !AWAKE(ch))
    return(FALSE);
  
  if (GET_STR(ch) < 25)
    GET_STR(ch) = 25;
  do_get(ch,"corpse",0);
  do_sacrifice(ch,"all",0);
  
  return FALSE;
}


SPECIAL(grave_demilich)
{
  struct char_data *victim;

  if ((event != EVENT_PULSE) || !AWAKE(ch) || !FIGHTING(ch))
    return FALSE;

  for(victim = world[ch->in_room].people; victim; victim = victim->next_in_room)
    if ((!IS_NPC(victim))&&(FIGHTING(victim) == ch))
		  /* person is a player who is fighting demlich */
      if (GET_HIGHEST_LEVEL(victim) >= number(0,75)) 
		  /* demilich wants high level victims */
        if (GET_HIGHEST_LEVEL(victim) >= number(0,75))
	          /* high level more likely to resist */
	{
	  act("$n tried to capture your soul.",FALSE,ch,0,victim,TO_VICT);
	  act("$n gives $N a icy cold stare.",FALSE,ch,0,victim,TO_NOTVICT);
	  return FALSE;
	}
	else
	{
	  act("$n sucks you into one of his gems.",FALSE,ch,0,victim,TO_VICT);
	  act("$N disappears into one of $n's eyes.",FALSE,ch,0,victim,TO_NOTVICT);
	  act("You trap $N in one of your gems.",FALSE,ch,0,victim,TO_CHAR);
          send_to_char("Your soul is trapped within the demilich.\r\n",victim);
	  send_to_char("Slowly you feel your life-force drain away...\r\n",victim);
	  GET_HIT(victim) = 1;
	  GET_MOVE(victim) = 0;
	  GET_MANA(victim) = 0;
	  char_from_room(victim);
	  switch(victim->player.hometown)
	  {
	    case 0:
	      char_to_room(victim, real_room(4));
	      break;
	    case 12:
	      char_to_room(victim, real_room(1200));
	      break;
	    case 30:
	    default:
	      char_to_room(victim, real_room(3001));
	      break;
	  }
	  do_look(victim,"",15);
	  return TRUE;
	}

  return FALSE;
}


SPECIAL(grave_ghoul)
{
  if ((event != EVENT_PULSE) || !FIGHTING(ch)) return (FALSE);

  if (!number(0,5)) {
    act("$n bites $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n bites you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_paralyze(GET_HIGHEST_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
      FIGHTING(ch), 0);
    return (TRUE);
  }
  return (FALSE);
}


SPECIAL(grave_priest)
{
  cd_t *vict;
  char buf[MAX_STRING_LENGTH];
  long num_in_room = 0, vict_num;

  if ((event != EVENT_PULSE) || number(0,4) || !AWAKE(smob)) return (FALSE);

  if (FIGHTING(smob)) {
    do_say(smob, "You are commiting blasphemy!", 0);
    return (FALSE);
  }

  for (vict = world[smob->in_room].people; vict; vict = vict->next_in_room)
    if (!IS_NPC(vict) && CAN_SEE(smob, vict)) num_in_room++;

  if (!num_in_room) return (FALSE);

  vict_num = number(1,num_in_room);

  for (vict = world[smob->in_room].people; vict; vict = vict->next_in_room) {
    if (!IS_NPC(vict) && CAN_SEE(smob, vict)) vict_num--;
    if (!vict_num && !IS_NPC(vict)) break;
  }
  if (!vict) return (FALSE);

  if (IS_GOOD(smob) && IS_GOOD(vict)) {
    sprintf(buf, "You, %s, are blessed.", GET_NAME(vict));
    do_say(smob, buf, 0);
    cast_bless(10, smob, GET_NAME(vict), SPELL_TYPE_SPELL, vict, 0);
  } else if (IS_EVIL(smob) && IS_EVIL(vict)) {
    act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, smob,
      0, vict, TO_NOTVICT);
    act("$n grins and says, 'You, $N, are truly wretched.'", FALSE, smob,
      0, vict, TO_VICT);
  } else if (IS_NEUTRAL(smob) && IS_NEUTRAL(vict)) {
    sprintf(buf, "You, %s, follow the True Path.", GET_NAME(vict));
    do_say(smob, buf, 0);
  } else if (IS_NEUTRAL(vict) || IS_NEUTRAL(smob)) {
    sprintf(buf, "%s, it is not too late for you to mend your ways.",
      GET_NAME(vict));
    do_say(smob, buf, 0);
  } else {
    sprintf(buf, "Blasphemy!  %s, your presence will stain this temple no "
      "more!", GET_NAME(vict));
    do_yell(smob, buf, 0);
  }
  return (FALSE);
}
/********************************************************************/


