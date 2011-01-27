/**************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/
/*
 *  Modified to work with Circle 3.0 beta 11 Winter of 97 by DSW
 *  Some changes by: StormeRider <silk@ICI.NET>
 *  Modified to work (if you can call it that) with Dibrova by Vedic <vedic@thevedic.net>
 */


#include "conf.h"
#include "sysdep.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "interpreter.h"
#include "comm.h"
#include "db.h"
#include "clan.h"

extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *character_list;
extern struct object_data *object_list;
extern struct object_data *obj_proto;
extern struct char_data *mob_proto;
extern struct zone_data *zone_table;
extern int top_of_mobt;
extern int top_of_world;
extern int double_exp;
extern struct room_data *world;
ACMD(do_say);
SPECIAL(shop_keeper);
int chance(int num);

int real_mobile(int virtual);

#define IS_QUESTOR(ch)     (PLR_FLAGGED(ch, PLR_QUESTOR))

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

// NEED TO CHANGE BEFORE FINAL!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define QUEST_OBJQUEST1 50 
#define QUEST_OBJQUEST2 51 
#define QUEST_OBJQUEST3 52 
#define QUEST_OBJQUEST4 53 
#define QUEST_OBJQUEST5 54 

/* Local functions */

void generate_quest(struct char_data *ch, struct char_data *questman);
void quest_update(void);

struct obj_data *create_object(int vnum, int dummy)
{
int r_num;
struct obj_data *tobj;

if((r_num = real_object(vnum)) < 0)
tobj = NULL;
else
 tobj = read_object(r_num, REAL);
 return(tobj);
}


/* The main quest function */

ACMD(do_autoquest)
{   char arg2[MAX_STRING_LENGTH], arg1[MAX_STRING_LENGTH];
    struct char_data *questman= NULL;
    struct obj_data *obj=NULL;
    struct obj_data *questinfoobj = NULL;
    struct char_data *questinfo = NULL;
    two_arguments(argument, arg1, arg2);

 
    if (is_abbrev(arg1, "info")) {
      if (IS_QUESTOR(ch)) {
        if (GET_QUESTOBJ(ch) > 0) {
          questinfoobj = get_obj_num(real_object(GET_QUESTOBJ(ch)));
	  if (questinfoobj != NULL) {
	    sprintf(buf, "/cwYou are on a quest to recover the fabled /cW%s/cw!/c0\r\n", questinfoobj->name);
	    send_to_char(buf, ch);
          } else send_to_char("/cwYou aren't currently on a quest./c0\r\n",ch);
            return;
	  } else if (GET_QUESTMOB(ch) > 0) {
            questinfo = get_char_num(real_mobile(GET_QUESTMOB(ch)));
	    if (questinfo != NULL) {
	      sprintf(buf, "/cwYou are on a quest to slay the dreaded/cW %s/cw!/c0\r\n", GET_NAME(questinfo));
	      send_to_char(buf, ch);
	    } else 
              send_to_char("/cwYou aren't currently on a quest./c0\r\n",ch);
	      return;
	    }
  	  } else
	    send_to_char("/cwYou aren't currently on a quest./c0\r\n",ch);
         return;
    }
    if (is_abbrev(arg1, "points"))
    {
	sprintf(buf, "/cwYou have /cW%d /cwquest points./c0\r\n",GET_QPOINTS(ch));
	send_to_char(buf, ch);
        return;
    }
    else if (is_abbrev(arg1, "time")) {
      if (!IS_QUESTOR(ch)) {
        send_to_char("/cwYou aren't currently on a quest./c0\r\n",ch);
        if (GET_NEXTQUEST(ch) > 1) {
	  sprintf(buf, "/cwThere are/cW %d /cwminutes remaining until you can go on another quest./c0\r\n",
          GET_NEXTQUEST(ch));
	  send_to_char(buf, ch);
        } else if (GET_NEXTQUEST(ch) == 1) {
	  sprintf(buf, "/cwThere is /cWless than a minute remaining/cw until you can go on another quest./c0\r\n");
	  send_to_char(buf, ch);
        }
      }
	
      else if (GET_COUNTDOWN(ch) > 0) {
	sprintf(buf, "/cwTime left for current quest: /cW%d/c0\r\n",
                      GET_COUNTDOWN(ch));
	send_to_char(buf, ch);
      } else {
        sprintf(buf, "/cwThere are /cW%d/cw minute(s) remaining until you can go on another quest./c0\r\n", GET_NEXTQUEST(ch));
        send_to_char(buf, ch);
      }
      return;
    }

/* checks for a mob flagged MOB_QUESTMASTER */
    for ( questman = world[ch->in_room].people; questman ;
                        questman = questman->next_in_room )
    {
	if (!IS_NPC(questman)) continue;
        if (MOB_FLAGGED(questman, MOB_QUESTMASTER))
           break;
    }

    if (questman == NULL)
    {
        send_to_char("You can't do that here.\r\n",ch);
       return;
    }

    if ( FIGHTING(questman) != NULL)
    {
	send_to_char("Wait until the fighting stops.\r\n",ch);
        return;
    }

    GET_QUESTGIVER(ch) = GET_MOB_VNUM(questman);


//*****Aquest Fail******


  if (is_abbrev(arg1, "fail"))
  {
     act("$n informs $N $e has decided to fail $S quest.", FALSE, ch, NULL,
          questman, TO_ROOM);
     act("/cwYou inform $N you have failed $S quest./c0", FALSE, ch, NULL,
          questman, TO_CHAR);
     
     if (GET_QUESTGIVER(ch) != GET_MOB_VNUM(questman))
     {
        sprintf(buf, "/cwI never sent you on a quest! Perhaps you're thinking
		      of someone else./c0\r\n");
        send_to_char(buf, ch);
      return;
     }

     if (IS_QUESTOR(ch))
     {
	if (GET_COUNTDOWN(ch) > 0)
	{
	   sprintf(buf, "/cwYou're not worthy yet, come back later!/c0\r\n");

	   REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_QUESTOR);
	   GET_QUESTGIVER(ch) = 0;     
	   GET_COUNTDOWN(ch) = 0;
	   GET_QUESTMOB(ch) = 0;
	   GET_QUESTOBJ(ch) = 0;
	   GET_NEXTQUEST(ch) = 20;

	   sprintf(buf, "QUEST: %s has failed his aquest.", GET_NAME(ch));
	 log(buf);
	 return;
	}
      }
	if (GET_NEXTQUEST(ch) > 0)
	{
	   sprintf(buf, "/cwYou have to wait to fail another quest./c0\r\n");
	}
	else
	{
	   sprintf(buf, "/cwYou can't fail a quest you haven't
                        started./c0\r\n");
	}
	 send_to_char(buf, ch);
	 return;
    } 


    if (is_abbrev(arg1, "request"))
    {
     
        act( "/cw$n asks $N for a quest./c0",FALSE, ch, NULL, questman, TO_ROOM); 
	act ("/cwYou ask $N for a quest./c0",FALSE, ch, NULL, questman, TO_CHAR);
	if (IS_QUESTOR(ch))
	{
	    sprintf(buf, "/cwBut you're already on a quest!/c0\r\n");
               send_to_char(buf, ch);
	    return;
	}
	if (GET_NEXTQUEST(ch) > 0)
	{
	    sprintf(buf, "/cwYour quest timer has not yet expired,/cW %s/cw./c0\r\n", GET_NAME(ch));
                send_to_char(buf, ch);
	    sprintf(buf, "/cwCome back later./c0\r\n");
                send_to_char(buf, ch);
	    return;
	}

	sprintf(buf, "/cwThank you, brave /cW%s/cw!/c0\r\n",GET_NAME(ch));
                send_to_char(buf, ch);
	generate_quest(ch, questman);
        if (GET_QUESTMOB(ch) > 0 || GET_QUESTOBJ(ch) > 0)
	{
            // SHOULD BE number(15, 30);
            GET_COUNTDOWN(ch) = number(15, 30);
	    SET_BIT_AR(PLR_FLAGS(ch), PLR_QUESTOR);
	    sprintf(buf, "/cwYou have/cW %d /cwminutes to complete this quest./c0\r\n",
               GET_COUNTDOWN(ch));
                send_to_char(buf, ch);
	    sprintf(buf, "/cwMay the gods go with you!/c0\r\n");
                send_to_char(buf, ch);
	}
	return;
    }
    else if (is_abbrev(arg1, "complete"))
    {
      act( "$n informs $N $e has completed $s quest.",FALSE, ch, NULL, questman, TO_ROOM); 
	  act ("/cwYou inform $N you have completed $s quest./c0",FALSE, ch, NULL, questman, TO_CHAR);
	if (GET_QUESTGIVER(ch) != GET_MOB_VNUM(questman))
	{
	    sprintf(buf, "/cwI never sent you on a quest! Perhaps you're thinking of someone else./c0\r\n");
                send_to_char(buf, ch);
	    return;
	}

	if (IS_QUESTOR(ch))
	{

	    if (GET_QUESTMOB(ch) == 1 && GET_COUNTDOWN(ch) > 0)
                {
		int reward = 0, pointreward = 0, expreward = 0;

		reward = GET_LEVEL(ch) * 20;
		pointreward = number(1,3); 

		sprintf(buf, "/cwCongratulations on completing your quest!/c0\r\n");
                send_to_char(buf, ch);
		sprintf(buf,"/cwAs a reward, I am giving you /cW%d quest points/cw, and /cY%d gold/cw./c0\r\n",pointreward, reward);
                send_to_char(buf, ch);
                if (chance(60))
		{
                expreward = ((GET_LEVEL(ch) * 1000) / number(2, 5)); 
		sprintf(buf, "/cwYou gain/cW %d experience/cw!/c0\r\n",expreward);
		send_to_char(buf, ch);
		GET_EXP(ch) += expreward;
		}
	        REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_QUESTOR);
                GET_QUESTGIVER(ch) = 0;
                GET_COUNTDOWN(ch) = 0;
                GET_QUESTMOB(ch) = 0;
                GET_QUESTOBJ(ch) = 0;
                // SHOULD BE 20
                GET_NEXTQUEST(ch) = 20;
		GET_GOLD(ch) += reward;
		GET_QPOINTS(ch) += pointreward;
                GET_EXP(ch) += expreward;
sprintf(buf, "QUEST: %s has completed his aquest.", GET_NAME(ch));
log(buf);

	        return;
	    }
	    else if (GET_QUESTOBJ(ch) > 0 && GET_COUNTDOWN(ch) > 0)
	    {
		bool obj_found = FALSE;

    for (obj = ch->carrying; obj; obj = obj->next_content) {
      if (GET_OBJ_VNUM(obj) == GET_QUESTOBJ(ch)) {
        
			obj_found = TRUE;
            	        break;
     }
    }
		if (obj_found == TRUE)
		{
		    int reward, pointreward;

		    reward = GET_LEVEL(ch) * 20;
		    pointreward = number(1, 3);

		    act("/cwYou hand $p to $N./c0",FALSE,
                             ch, obj, questman, TO_CHAR);
		    act("/cw$n hands $p to $N./c0",FALSE,
                         ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "/cwCongratulations on completing your quest!/c0\r\n");
                send_to_char(buf, ch);
		    sprintf(buf,"/cwAs a reward, I am giving you /cW%d quest points/cw, and /cY%d gold/cw./c0\r\n",pointreward,reward);
                send_to_char(buf, ch);

	            REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_QUESTOR);
	            GET_QUESTGIVER(ch) = 0;
	            GET_COUNTDOWN(ch) = 0;
	            GET_QUESTMOB(ch) = 0;
		    GET_QUESTOBJ(ch) = 0;
                    // SHOULD BE 20
	            GET_NEXTQUEST(ch) = 20;
		    GET_GOLD(ch) += reward;
		    GET_QPOINTS(ch) += pointreward;
		    extract_obj(obj);
                    if(obj) { log("obj in aquest still exists"); }
sprintf(buf, "QUEST: %s has completed his aquest.", GET_NAME(ch));
log(buf);		    
 return;
		}
		else
		{
		    sprintf(buf, "/cwYou haven't completed the quest yet, but there is still time!/c0\r\n");
                send_to_char(buf, ch);
		    return;
		}
		return;
	    }
	    else if ((GET_QUESTMOB(ch) > 0 || GET_QUESTOBJ(ch) > 0) && GET_COUNTDOWN(ch) > 0)
	    {
		sprintf(buf, "/cwYou haven't completed the quest yet, but there is still time!/c0\r\n");
                send_to_char(buf, ch);
		return;
	    }
	}
	if (GET_NEXTQUEST(ch) > 0)
	    sprintf(buf,"/cwBut you didn't complete your quest in time!/c0\r\n");
	else sprintf(buf, "/cwYou have to/cW REQUEST /cwa quest first, /cW%s/cw./c0\r\n",GET_NAME(ch));
                send_to_char(buf, ch);
	return;
    }

    send_to_char("/cwAQUEST commands: /cWPOINTS INFO TIME REQUEST COMPLETE FAIL./c0\r\n",ch);
    send_to_char("/cwFor more information, type '/cWHELP AQUEST/cw'./c0\r\n",ch);
    return;
}

void generate_quest(struct char_data *ch, struct char_data *questman)
{
    struct char_data *victim = NULL;
    struct char_data *tch_last = NULL, *tch = NULL;
    struct obj_data *questitem;
    int level_diff, i, found;

    /*  Randomly selects a mob from the world mob list. If you don't
	want a mob to be selected, make sure it is immune to summon.
	Or, you could add a new mob flag called ACT_NOQUEST. The mob
	is selected for both mob and obj quests, even tho in the obj
	quest the mob is not used. This is done to assure the level
	of difficulty for the area isn't too great for the player. */

/* added another mob flag - MOB_QUESTABLE - for mobs it is ok to quest
   against. Although this isn't used */

/* Now we take a break for *TALES FROM THE LIFE OF VEDIC*
I rolled out of bed late this morning. My room was littered with empty boxes of girl scout cookies, sashes, skirts, and the torn remnants of the various badges and honors earned. 
Troop 152 took their pledge to serve to the extreme last night, and I'd like to send a shout out to all of them. They came to Casa Vedic as GIRL scouts, but they left as WOMEN.
-- 12/2/00 --
*/
 found = 0;
 for (i = number(100, top_of_world); i < top_of_world; i++) {
   for (tch = world[i].people;tch;tch = tch->next_in_room) {
     if (tch_last == tch) { continue; }
        tch_last = tch;
        level_diff = GET_LEVEL(tch) - GET_LEVEL(ch);

        if ((level_diff > -5 && level_diff < 20) &&  
           (!ROOM_FLAGGED(tch->in_room, ROOM_GODROOM) && 
            GET_MOB_VNUM(tch) == GET_MOB_VNUM(tch) && 
            !ROOM_FLAGGED(tch->in_room, ROOM_GRID) && 
            !ROOM_FLAGGED(tch->in_room, ROOM_ARENA) && 
            !ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL) && 
(zone_table[world[real_room(30100)].zone].number != zone_table[world[IN_ROOM(tch)].zone].number) && 
(zone_table[world[real_room(30200)].zone].number != zone_table[world[IN_ROOM(tch)].zone].number) && 
(zone_table[world[real_room(30300)].zone].number != zone_table[world[IN_ROOM(tch)].zone].number) && 
(zone_table[world[real_room(30400)].zone].number != zone_table[world[IN_ROOM(tch)].zone].number) && 
(zone_table[world[real_room(30500)].zone].number != zone_table[world[IN_ROOM(tch)].zone].number)) &&
     (mob_index[GET_MOB_RNUM(tch)].func != shop_keeper)) {
        victim = tch;
        found = 1; 
        break;
       }
      }
      if(found)
       {

        break;
       }
     }
//}
    if (victim == NULL || chance(30))
    {
   sprintf(buf, "/cwI'm sorry, but I don't have any quests for you at this time./c0\r\n");
  send_to_char(buf, ch);
  sprintf(buf, "/cwTry again later./c0\r\n");
  send_to_char(buf, ch);
  GET_NEXTQUEST(ch) = 5;
  return;
    }
  SET_BIT_AR(AFF_FLAGS(victim), AFF_NOMAGIC);

// SHOULD BE 50
//if (victim) {
    if (chance(50))  /* prefer item quests myself */
    {
	int objvnum = 0;
	switch(number(0,4))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	    case 4:
	    objvnum = QUEST_OBJQUEST5;
	    break;
	}
        questitem = create_object(objvnum, GET_LEVEL(ch) );
        if(questitem == NULL)
           {
           log("questitem does not EXIST!!");
           send_to_char("Error: questitem does not exist! please notify the imms\r\n",ch);
           return;
           }
	obj_to_room(questitem, victim->in_room);

        GET_QUESTOBJ(ch) = GET_OBJ_VNUM(questitem);

	switch(number(0,1))
	{
	    case 0:
              sprintf(buf, "/cwA rare and valuable %s has been stolen from the museum!/c0\r\n", questitem->short_description);
                send_to_char(buf, ch);
           break;

	   case 1:
              sprintf(buf, "/cwAn Imp stole %s from my family! Please get it back!/c0\r\n", questitem->short_description);
                send_to_char(buf, ch);
           break;
	}

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */
	sprintf(buf, "/cwLook in the general area of /cW%s!/cw/c0\r\n",
          zone_table[world[victim->in_room].zone].name);
                send_to_char(buf, ch);
sprintf(buf, "QUEST: NAME: %s OBJ: %d ZONE: %d", GET_NAME(ch), GET_QUESTOBJ(ch), world[victim->in_room].number);
log(buf);
	return;
    }

    /* Quest to kill a mob */

    else 
    {
    switch(number(0,3))
    {
	case 0:
        sprintf(buf, "/cwAn enemy of mine, %s, is making vile threats against the Lord of Jareth./c0\r\n",
                 GET_NAME(victim));
                send_to_char(buf, ch);
        sprintf(buf, "/cwThis threat must be eliminated!/c0\r\n");
                send_to_char(buf, ch);
	break;

	case 1:
	sprintf(buf, "/cwJareth's most heinous criminal, %s, has escaped from the Labyrinth!/c0\r\n",

               GET_NAME(victim));
                send_to_char(buf, ch);
	sprintf(buf, "/cwSince the escape, %s has murdered %d civilians!/c0\r\n",
           GET_NAME(victim), number(2,20));
                send_to_char(buf, ch);            
	break;
      
       case 2:
        sprintf(buf, "/cwI have discovered one of Encar's spies, %s./c0\r\n",
                 GET_NAME(victim));
                send_to_char(buf, ch);
        sprintf(buf, "/cwThis threat must be eliminated!/c0\r\n");
                send_to_char(buf, ch);

       case 3:
        sprintf(buf, "/cwAn enemy of mine, %s, needs to be eliminated once and for all./c0\r\n",
                 GET_NAME(victim));
                send_to_char(buf, ch);
        sprintf(buf, "/cwKill him for me and you will be rewarded./c0\r\n");
                send_to_char(buf, ch);

    }
    if (world[victim->in_room].name != NULL)
    {
        sprintf(buf, "/cwSeek/cW %s/cw out somewhere in the vicinity of /cW%s/cw!/c0\r\n",
            GET_NAME(victim), zone_table[world[victim->in_room].zone].name);
                send_to_char(buf, ch);

    }
    GET_QUESTMOB(ch) = GET_MOB_VNUM(victim);
sprintf(buf, "QUEST: NAME: %s MOB: %d ROOM: %d", GET_NAME(ch), GET_QUESTMOB(ch), world[victim->in_room].number);
log(buf);
    }

    return;
}
/* Called from update_handler() by pulse_area */

void quest_update(void)
{
  struct char_data *ch, *ch_next;
  int i = 0;

  for(i=0; i < num_of_clans; i++) {
    if(clan[i].war_timer) {
       clan[i].war_timer--;
    }
  }

  if (double_exp == 1) {
    send_to_all("/cRDouble experience has ended./c0\r\n");
    double_exp = 0;
  }
  if (double_exp == 2) {
    double_exp = double_exp - 1;
    sprintf(buf, "/cRThere is %d minute left of double experience./c0\r\n", 
            double_exp);
    send_to_all(buf);
  }
  if (double_exp >= 3) {
    double_exp = double_exp - 1;
    sprintf(buf, "/cRThere are %d minutes left of double experience./c0\r\n",
            double_exp);
    send_to_all(buf);
  }

  for ( ch = character_list; ch; ch = ch_next ) {
    ch_next = ch->next;

    if (AFF_FLAGGED(ch, AFF_BURROW)) {
      send_to_char("The earth nourishes you, healing your wounds.\r\n", ch);
      GET_HIT(ch) = MIN(GET_MAX_HIT(ch),
              GET_HIT(ch) + GET_LEVEL(ch) +
              GET_CON(ch));
      GET_MANA(ch) = MIN(GET_MAX_MANA(ch),
              GET_MANA(ch) + GET_LEVEL(ch) +
              GET_INT(ch));
      GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch),
              GET_MOVE(ch) + GET_LEVEL(ch) +
              GET_DEX(ch));
      GET_QI(ch) = MIN(GET_MAX_QI(ch),
              GET_QI(ch) + GET_LEVEL(ch) +
              GET_INT(ch));
      GET_VIM(ch) = MIN(GET_MAX_VIM(ch),
              GET_VIM(ch) + GET_LEVEL(ch) +
              GET_INT(ch));
    }
    if (IS_NPC(ch)) continue;
       

    if (GET_PKSAFETIMER(ch) > 0) {
      if (--(GET_PKSAFETIMER(ch)) <= 0) {
        GET_PKSAFETIMER(ch) = 0;
        send_to_char("You may now pkill again. Good luck.\r\n", ch);
        sprintf(buf, "PKINFO:: Pkill safetime for %s has ended.\r\n", 
                GET_NAME(ch));
        send_to_pkillers(buf);
      }
    }

    if (GET_PIT_TIMER(ch) > 0) {
      GET_PIT_TIMER(ch)--;
      if (GET_PIT_TIMER(ch) == 0) {
        send_to_char("You may now enter the Pit again. Good luck.\r\n", ch);
      }
    }

    if (GET_NEXTQUEST(ch) > 0) {
      GET_NEXTQUEST(ch)--;

      if (GET_NEXTQUEST(ch) == 0) {
        send_to_char("/cGQUEST INFO: You may now quest again./c0\r\n",ch);
        return;
      }
    }
    if (IS_QUESTOR(ch)) {
      if (--GET_COUNTDOWN(ch) <= 0) {
        GET_NEXTQUEST(ch) = 20; 
        sprintf(buf, "/cGQUEST INFO: You have run out of time for your "
                     "quest!\r\nYou may quest again in %d minutes./c0\r\n",
                      GET_NEXTQUEST(ch));
        send_to_char(buf, ch);
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_QUESTOR);
        GET_QUESTGIVER(ch) = 0;
        GET_COUNTDOWN(ch) = 0;
        GET_QUESTMOB(ch) = 0;
        GET_QUESTOBJ(ch) = 0;
      }
      if (GET_COUNTDOWN(ch) > 0 && GET_COUNTDOWN(ch) < 6) {
        send_to_char("/cGQUEST INFO: Better hurry, you're almost out of "
                     "time for your quest!/c0\r\n",ch);
        return;
      }
    }
  }
  return;
}

