/**************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "screen.h"
#include "clan.h"
#include "dg_scripts.h"

/* extern variables */
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;

/* All credits for 'new say' from Mythran */
ACMD(do_say)
{
  char buf2[MAX_STRING_LENGTH];
  skip_spaces(&argument);

  if (!*argument)
  {
    send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_COURT) && !IS_AFFECTED(ch, AFF_SPEAK) &&
      GET_LEVEL(ch) <= 150) {
    send_to_char("Sorry, you'll have to wait your turn to speak.\r\n", ch);
    return;
  }  
  if (IS_AFFECTED(ch, AFF_BURROW)) {
    send_to_char("You are burrowed into the earth - who would you speak to?\r\n", ch);
    return;
  }
  
  if (argument[strlen(argument) - 1] == '?')
  {
    sprintf(buf, "You ask, '/cw%s/c0'", argument);
    sprintf(buf2, "$n asks, '/cw%s/c0'", argument);
  }
  else if(argument[strlen(argument) - 1] == '!')
  {
    sprintf(buf, "You exclaim, '/cw%s/c0'", argument);
    sprintf(buf2, "$n exclaims, '/cw%s/c0'", argument);
  }
  else if (argument[strlen(argument) - 1] == '.' &&
           argument[strlen(argument) - 2] == '.' &&
           argument[strlen(argument) - 3] == '.')
  {
    sprintf(buf, "You mutter, '/cw%s/c0'", argument);     
    sprintf(buf2, "$n mutters, '/cw%s/c0'", argument);
  }
  else
  {
    sprintf(buf, "You say, '/cw%s/c0'", argument);
    sprintf(buf2, "$n says, '/cw%s/c0'", argument);
  } 

  if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
  send_to_char(OK, ch);
  else
  {
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    act(buf2, FALSE, ch, 0, argument, TO_ROOM|DG_NO_TRIG);
  }

  /* trigger check */
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}       


ACMD(do_gsay)
{
  struct char_data *k;
  struct follow_type *f;

  skip_spaces(&argument);

  if (!IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char("But you are not the member of a group!\r\n", ch);
    return;
  }
  if (!*argument)
    send_to_char("Yes, but WHAT do you want to group-say?\r\n", ch);
  else {
    if (ch->master)
      k = ch->master;
    else
      k = ch;

    sprintf(buf, "$n tells the group, '/cw%s/c0'", argument);

    if (IS_AFFECTED(k, AFF_GROUP) && (k != ch))
      act(buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP);
    for (f = k->followers; f; f = f->next)
      if (IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch))
	act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You tell the group, '/cw%s/c0'", argument);
      act(buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }
  }

}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
  send_to_char(CCRED(vict, C_NRM), vict);
  sprintf(buf, "$n tells you, '%s'", arg);
  act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(vict, C_NRM), vict);

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    send_to_char(CCRED(ch, C_CMP), ch);
    sprintf(buf, "You tell $N, '%s'", arg);
    act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    send_to_char(CCNRM(ch, C_CMP), ch);
  }

  GET_LAST_TELL(vict) = GET_IDNUM(ch);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  struct char_data *vict;

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("There are no players here by that name.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)	/* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PRF_FLAGGED(vict, PRF_NOTELL) || ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else
    perform_tell(ch, vict, buf2);
}


ACMD(do_reply)
{
  struct char_data *tch = character_list;

  skip_spaces(&argument);

  if (GET_LAST_TELL(ch) == NOBODY)
    send_to_char("You have no-one to reply to!\r\n", ch);
  else if (!*argument)
    send_to_char("What is your reply?\r\n", ch);
  else if (PRF_FLAGGED(tch, PRF_NOTELL))
    send_to_char("They have gone to notell, sorry.\r\n", ch);
  else {
    /*
     * Make sure the person you're replying to is still playing by searching
     * for them.  Note, now last tell is stored as player IDnum instead of
     * a pointer, which is much better because it's safer, plus will still
     * work if someone logs out and back in again.
     */
				     
    while (tch != NULL && GET_IDNUM(tch) != GET_LAST_TELL(ch))
      tch = tch->next;

    if (tch == NULL)
      send_to_char("They are no longer playing.\r\n", ch);
    else
      perform_tell(ch, tch, argument);
  }
}


ACMD(do_spec_comm)
{
  struct char_data *vict;
  char *action_sing, *action_plur, *action_others;

  if (subcmd == SCMD_WHISPER) {
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      send_to_char("This is not an immortal option for communication./r\n", ch);
      return;
    }
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
  } else {
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
    sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
    send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (vict == ch)
    send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else {
    sprintf(buf, "$n %s you, '%s'", action_plur, buf2);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
    act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000	/* arbitrary */

ACMD(do_write)
{
  struct obj_data *paper = 0, *pen = 0;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {		/* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {		/* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", penname);
      send_to_char(buf, ch);
      return;
    }
  } else {		/* there was one arg.. let's see what we can find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
      pen = paper;
      paper = 0;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!GET_EQ(ch, WEAR_HOLD)) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
	      papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = GET_EQ(ch, WEAR_HOLD);
    else
      pen = GET_EQ(ch, WEAR_HOLD);
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN)
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
    /* we can write - hooray! */
     /* this is the PERFECT code example of how to set up:
      * a) the text editor with a message already loaed
      * b) the abort buffer if the player aborts the message
      */
     ch->desc->backstr = NULL;
     send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
     /* ok, here we check for a message ALREADY on the paper */
     if (paper->action_description) {
	/* we str_dup the original text to the descriptors->backstr */
	ch->desc->backstr = str_dup(paper->action_description);
	/* send to the player what was on the paper (cause this is already */
	/* loaded into the editor) */
	send_to_char(paper->action_description, ch);
     }
    act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
     /* assign the descriptor's->str the value of the pointer to the text */
     /* pointer so that we can reallocate as needed (hopefully that made */
     /* sense :>) */
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}



ACMD(do_page)
{
  struct descriptor_data *d;
  struct char_data *vict;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_IMMORT) {
	for (d = descriptor_list; d; d = d->next)
	  if (!d->connected && d->character)
	    act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
	send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if ((vict = get_char_vis(ch, arg)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(OK, ch);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
      return;
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  extern int level_can_shout;
//  extern int holler_move_cost;
  struct descriptor_data *i;
  char color_on[24];
  byte gmote = FALSE;

  /* Array of flags which must _not_ be set in order for comm to be heard */
  static int channels[] = {
    0,
    PRF_DEAF,
    PRF_NOGOSS,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    0,
    0
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   *           [1] name of the action
   *           [2] message if you're not on the channel
   *           [3] a color string.
   */

  static char *com_msgs[][4] = {
    {"You cannot commune!!\r\n",
      "commune",
      "",
    KYEL},

    {"You cannot shout!!\r\n",
      "shout",
      "Turn off your noshout flag first!\r\n",
    KYEL},

    {"You cannot gossip!!\r\n",
      "gossip",
      "You aren't even on the channel!\r\n",
    KYEL},

    {"You cannot auction!!\r\n",
      "auction",
      "You aren't even on the channel!\r\n",
    KMAG},

    {"You cannot congratulate!\r\n",
      "congrat",
      "You aren't even on the channel!\r\n",
    KGRN},

    {"You cannot talk on the Private Channel!\r\n",
      "private-say",
      "You aren't even on the channel!\r\n",
    KYEL},

    {"You cannot sing!\r\n",
      "sing",
      "You aren't even on the channel!\r\n",
    KYEL}

  };
 
  ACMD(do_gmote);

  if (subcmd == SCMD_PRIVATE && GET_PRIVATE(ch) == 0) {
    send_to_char("You are not on a private channel!", ch);
    return;
  }

  /* to keep pets, etc from being ordered to shout */
  if (!ch->desc)
    return;

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_COURT) && GET_LEVEL(ch) <= 150) {
    send_to_char("Keep your voice down, or be fined for comtempt of court!\r\n", ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < level_can_shout) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    level_can_shout, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd]) && subcmd != SCMD_PRIVATE) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);


  if(subcmd == SCMD_GMOTE || (subcmd == SCMD_GOSSIP && *argument == '@')) {
    subcmd = SCMD_GOSSIP;
    gmote = TRUE;
  }


  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n",
	    com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

  if (gmote) {
    if (*argument == '@')
      do_gmote(ch, argument + 1, 0, 1);
    else
      do_gmote(ch, argument, 0, 1);
    return;
  }

/*  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < holler_move_cost) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= holler_move_cost;
  }
*/
  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP)
      if (subcmd==SCMD_GRATZ)
        sprintf(buf1, "/cGYou %s, '%s'/c0", com_msgs[subcmd][1],
                argument);
      else
        sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1],
                argument, KNRM);
    else
      sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
  }

  sprintf(buf, "$n %ss, '%s'", com_msgs[subcmd][1], argument);

  /* now send all the strings out */ for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i != ch->desc && i->character &&
	!PRF_FLAGGED(i->character, channels[subcmd]) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) &&
	!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF)) {

     if (subcmd == SCMD_PRIVATE &&
        (GET_PRIVATE(ch) != GET_PRIVATE(i->character)))
       continue;

      if (subcmd == SCMD_SHOUT &&
	  ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
	   GET_POS(i->character) < POS_RESTING))
	continue;

      if (subcmd == SCMD_COMMUNE && GET_LEVEL(i->character) < LVL_IMMORT)
        continue;
 
      /* For deafness affect */
      if (!IS_AFFECTED(i->character, AFF_DEAF)) {
        if (COLOR_LEV(i->character) >= C_NRM)
          send_to_char(color_on, i->character);
        act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
        if (COLOR_LEV(i->character) >= C_NRM)
          send_to_char(KNRM, i->character);
      } else {
        if (ch->in_room == i->character->in_room)
          act("You see $n's lips move.", FALSE, ch, 0, i->character, TO_VICT);
        act("Your ears ring.", FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      }
    }
  }
}


ACMD(do_qcomm)
{
  struct descriptor_data *i;

  if (!PRF_FLAGGED(ch, PRF_QUEST)) {
    send_to_char("You aren't even part of the quest!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    sprintf(buf, "%s?  Yes, fine, %s we must, but WHAT??\r\n", CMD_NAME,
	    CMD_NAME);
    CAP(buf);
    send_to_char(buf, ch);
  } else {
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else {
      if (subcmd == SCMD_QSAY)
	sprintf(buf, "/cGYou quest-say, '%s'/c0", argument);
      else
	strcpy(buf, argument);
      act(buf, FALSE, ch, 0, argument, TO_CHAR);
    }

    if (subcmd == SCMD_QSAY)
      sprintf(buf, "/cG$n quest-says, '%s'/c0", argument);
    else
      strcpy(buf, argument);

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i != ch->desc &&
	  PRF_FLAGGED(i->character, PRF_QUEST))
	act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
  }
}


/*
 * I didn't write this command, i just modified it, all credits should
 * go to original coder
 */
ACMD (do_ctell)
{
  struct descriptor_data *i;
  int minlev=1, c=0;
  char level_string[6]="\0\0\0\0\0\0";

  skip_spaces (&argument);


/* The syntax of ctell for imms is different then for morts
   mort: ctell <bla bla bla>    imms: ctell <clan_num> <bla bla bla>
   Imms cannot actually see ctells but they can send them */

  if (GET_LEVEL(ch) >= LVL_IMMORT){
    c = atoi (argument);
    if ((c <= 0) || (c > num_of_clans)){
      send_to_char ("There is no clan with that number.\r\n", ch);
      return;
    }
    while ((*argument != ' ') && (*argument != '\0'))
      argument++;
    while (*argument == ' ') argument++;
  }
  else
    if((c=GET_CLAN(ch))==0 || GET_CLAN_RANK(ch)==0) {
      send_to_char ("You're not part of a clan.\r\n", ch);
      return;
    }

  skip_spaces (&argument);

  if (!*argument){
    send_to_char ("What do you want to tell your clan?\r\n", ch);
    return;
  }

  if (*argument == '#'){
    argument++;
    minlev = atoi (argument);
    if (minlev > clan[c].ranks){
      send_to_char ("No one has a clan rank high enough to hear you!\r\n", ch);
      return;
    }
    while (*argument != ' ') argument++;
    while (*argument == ' ') argument++;
    sprintf (level_string, "(%d)", minlev);
  }

  if (PRF_FLAGGED(ch,PRF_NOREPEAT))
    sprintf (buf1, OK);
  else
      sprintf (buf1, "/cWYou /cCtell your clan%s, '/cC%s/cC'\r\n/c0",level_string, argument);
  send_to_char (buf1, ch);

  for (i = descriptor_list; i; i=i->next){
      if (i->character->player_specials->saved.clan == c && GET_CLAN_RANK(i->character) > 0)
	  {
		  if (i->character->player_specials->saved.clan_rank >= minlev +1)
		  {
			  if (strcmp (i->character->player.name, ch->player.name))
			  {
				  sprintf (buf, "/cW%s /cCtells your clan%s, '/cC%s/cC'\r\n/c0",
					  (((IS_AFFECTED(ch, AFF_INVISIBLE)) &&
                      (!IS_AFFECTED(i->character, AFF_DETECT_INVIS))) ?
                      "Someone" : ch->player.name), level_string, argument);
				  send_to_char (buf, i->character);
			  }
		  }
      }
  }
  return;
}



ACMD(do_private_channel)
{
  struct char_data *vict;
  struct descriptor_data *i;

  half_chop(argument, buf, buf2);

  if (subcmd == PRIVATE_HELP) {
  send_to_char("Private Channel (PC) commands\r\n", ch);
  send_to_char("-------------------------------------------------\r\n", ch);
  send_to_char("popen   - opens your own private channel.\r\n", ch);
  send_to_char("padd    - adds a player to your PC.\r\n", ch);
  send_to_char("premove - remove a player from your PC.\r\n", ch);
  send_to_char("pclose  - closes your private channel.\r\n", ch);
  send_to_char("pwho    - lists all members on the current PC.\r\n", ch);
  send_to_char("poff    - exits you from your current PC.\r\n\r\n", ch);
  send_to_char("  NOTE: If you don't want to be added to another\r\n", ch);
  send_to_char("        player's PC open your own with no players.\r\n", ch);
  send_to_char("\r\nTo talk on the channel use -- private, psay or .\r\n",ch);
  } else if (subcmd == PRIVATE_OPEN) {
    GET_PRIVATE(ch) = GET_IDNUM(ch);
    send_to_char("You have just opened your own Private Channel.\r\n", ch);
  } else if (subcmd == PRIVATE_OFF) {
    GET_PRIVATE(ch) = 0;
    send_to_char("You have just quit any Private Channels.\r\n", ch);
  } else if (subcmd == PRIVATE_CLOSE) {
    GET_PRIVATE(ch) = 0;
    /* now remove all people on the private channel */
    for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
      if ((GET_PRIVATE(i->character) == GET_IDNUM(ch)) &&
          (ch != i->character)) {
        GET_PRIVATE(i->character) = 0;
        sprintf(buf, "%s has just closed their Private Channel.\r\n",
               GET_NAME(ch));
        send_to_char(buf, i->character);
      }
     send_to_char("You have just CLOSED your Private Channel.\r\n", ch);
  } else if (subcmd == PRIVATE_WHO) {
    if (GET_PRIVATE(ch) == 0)
      send_to_char("You are not on a private channel\r\n",ch);
    else {
      /* show all people on the private channel */
      send_to_char("Private Channel Members\r\n", ch);
      send_to_char("-----------------------\r\n", ch);
      for (i = descriptor_list; i; i = i->next)
      if (!i->connected)
        if (GET_PRIVATE(i->character) == GET_PRIVATE(ch)) {
          sprintf(buf, "%s\r\n", GET_NAME(i->character));
          send_to_char(buf, ch);
        }
    }
  } else if (subcmd == PRIVATE_CHECK) {
      /* show all people on the ALL private channels */
      send_to_char("Private Channels\r\n", ch);
      send_to_char("---------------------------------------------\r\n", ch);
      for (i = descriptor_list; i; i = i->next)
        if (!i->connected) {
        sprintf(buf, "[%-5d]  %s\r\n",
                  GET_PRIVATE(i->character), GET_NAME(i->character));
          send_to_char(buf, ch);
      }
  } else if (subcmd == PRIVATE_REMOVE) {
      if (!*buf)
        send_to_char("Who do you wish to add to you private channel?\r\n", ch);
      else if (!(vict = get_char_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
      else if (IS_NPC(vict))
        send_to_char("NPC's cannot be on private channels\r\n", ch);
      else if (GET_PRIVATE(vict) != GET_IDNUM(ch)) {
        sprintf(buf,"%s is NOT on your Private Channel!\r\n",
                GET_NAME(vict));
        send_to_char(buf, ch);
      } else {
        GET_PRIVATE(vict) = 0;
        sprintf(buf,"You have been REMOVED from %s's Private Channel!\r\n",
                GET_NAME(ch));
        send_to_char(buf, vict);
        sprintf(buf,"%s has been REMOVED from your Private Channel!\r\n",
                GET_NAME(vict));
        send_to_char(buf, ch);
      }
  } else if (subcmd == PRIVATE_ADD) {
    if (GET_PRIVATE(ch) != GET_IDNUM(ch))
      send_to_char("You must open your own private channel first\r\n",ch);
    else if (!*buf)
        send_to_char("Who do you wish to add to you private channel?\r\n", ch);
    else if (!(vict = get_char_vis(ch, buf)))
        send_to_char(NOPERSON, ch);
    else if (ch == vict)
        GET_PRIVATE(ch) = GET_IDNUM(ch);
    else if (IS_NPC(vict))
        send_to_char("NPC's cannot be added to private channels\r\n", ch);
    else if (GET_PRIVATE(vict) != 0) {
        sprintf(buf,"%s is already on another private channel!\r\n",
                GET_NAME(vict));
        send_to_char(buf, ch);
    } else {
        GET_PRIVATE(vict) = GET_IDNUM(ch);
        sprintf(buf,"You have been ADDED to %s's Private Channel!\r\n",
                GET_NAME(ch));
        send_to_char(buf, vict);
        sprintf(buf,"%s has been ADDED to your Private Channel!\r\n",
                GET_NAME(vict));
        send_to_char(buf, ch);
    }
  }
}

ACMD (do_vctell)
{
  struct descriptor_data *i;
  int c=0;

  skip_spaces (&argument);

  if (GET_LEVEL(ch) >= LVL_IMMORT || IS_NPC(ch)){
    c = atoi (argument);
    if ((c <= 0) || (c > num_of_clans)){
      send_to_char ("There is no clan with that number.\r\n", ch);
      return;
    }
    while ((*argument != ' ') && (*argument != '\0'))
      argument++;
    while (*argument == ' ') argument++;
  }
  else
    if((c=GET_CLAN(ch))==0 || GET_CLAN_RANK(ch)==0) {
      send_to_char ("You're not part of a clan.\r\n", ch);
      return;
    }

  skip_spaces (&argument);

  if (!*argument){
    send_to_char ("What do you want to tell your clan?\r\n", ch);
    return;
  }


  if (PRF_FLAGGED(ch,PRF_NOREPEAT))
    sprintf (buf1, OK);
  else
      sprintf (buf1, "/cWYou /cCtell your clan, '%s'\r\n/c0", argument);
  send_to_char (buf1, ch);

  for (i = descriptor_list; i; i=i->next){
   if (!i->connected && i != ch->desc) {
    if(i->character->player_specials->saved.clan) { 
		if (i->character->player_specials->saved.clan == c && GET_CLAN_RANK(i->character) > 0) {
          if (strcmp (i->character->player.name, ch->player.name)){
            sprintf (buf, "/cC%s tells your clan, '%s'\r\n/c0",
                     (((IS_AFFECTED(ch, AFF_INVISIBLE)) &&
                      (!IS_AFFECTED(i->character, AFF_DETECT_INVIS))) ?
                      "Someone" : ch->player.name), argument);
            send_to_char (buf, i->character);
          }
        
      }
     }
    }
  }
  return;
}

