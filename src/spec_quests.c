/**************************************************************************
 * File: spec_quests.c                                                    *
 * Usage: This is code for automated quests.                               *
 * Created: 7/9/99                                                        *
 **************************************************************************/


#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "olc.h"


/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];
extern int drink_aff[][3];
extern char *rank[][3];
extern int newbie_level;
void mobsay(struct char_data *ch, const char *msg);
void mobaction(struct char_data *ch, char *string, char *action);

extern struct zone_data *zone_table;
extern int top_of_mobt;
extern int top_of_world;

int real_mobile(int virtual);
int glad_bet_amnt = 0;
int pit_mob_vnum = 0;

/* extern functions */
void add_follower(struct char_data * ch, struct char_data * leader);
extern int level_exp(int class, int level);
struct char_data *find_npc_by_name(struct char_data * chAtChar, char *pszName,
                                             int iLen);
int chance(int num);
ACMD(do_say);
ACMD(do_flee);
ACMD(do_move);
ACMD(do_stand);
ACMD(do_assist);
ACMD(do_tell);

SPECIAL(lord_keep_quest) /* Vedic 7/9/99 */
{

  struct obj_data *item;
//  struct char_data *vict = (struct char_data *) me;

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(12012)) {
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "The Lord takes the head from you.\n");
	strcat(buf,  "The Lord says, \'You have done a great service in defeating the minotaur.\'\n");
        strcat(buf,  "The Lord says, \'Here is your reward.\'\n");
	strcat(buf,  "The Lord gives you 10,000 gold.\n");
	send_to_char(buf, ch);
        if (GET_RANK(ch) < RANK_BARON && !PLR_FLAGGED(ch, PLR_KEEPRANK)) {
         send_to_char("I shall also bestow the rank of Baron upon you!\r\n\r\n", ch);
         GET_RANK(ch) = RANK_BARON;
     sprintf(buf, "\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n", GET_NAME(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);
     send_to_all(buf);
        }
	GET_GOLD(ch) += 10000;

     sprintf(buf, "QUEST: %s has completed the minotaur quest.\r\n", GET_NAME(ch));
        log(buf);
	return TRUE;
      }
    }
    
  }
return FALSE;
}

SPECIAL(elven_tapestry_quest) /* Vedic 11/9/99 */
{
  struct obj_data *item;
  struct char_data *vict = (struct char_data *) me;
  int r_num;

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(5605)) { /* 5605 */
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "Reklan takes an Elven tapestry of Lore from you.\r\n");
	send_to_char(buf, ch);

	for (item = vict->carrying; item; item = item->next_content) {
	  if (item->item_number == real_object(19070)) {   /* 19070 */
	     obj_from_char(item);
	     extract_obj(item);
	     r_num = real_object(19070);
	     item = read_object(r_num, REAL);
             obj_to_char(item, ch);
	     sprintf(buf,  "Reklan says, \'The entire Elven Nation is indebted to you. Take this helm and may the Gods speed you on your journies.\'\r\n");
	     send_to_char(buf, ch);

     sprintf(buf, "QUEST: %s has completed the elf quest.\r\n", GET_NAME(ch));
        log(buf);
	     return TRUE;
	  }  
	  return FALSE;
	}
	return TRUE;
      }
    }
    
  }
return FALSE;
}


/* Throw Quest -- Vedic 7/14/99 */

SPECIAL(pon_neroth)
{

  struct obj_data *item;
  int r_num;

  if (FIGHTING((struct char_data *)me))
    return FALSE;

  if (!cmd) {
    if (number(0, 5) == 0) {
      act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Hey, want to do a job for me?", 0, 0);
      return TRUE;
    }
  }

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(12009)) {
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "Pon Neroth takes the mane from you.\n");
	strcat(buf,  "Pon Neroth says, \'Wow, thanks.\'\n");
        strcat(buf,  "Pon Neroth says, \'Here is your payment.\'\n");
	strcat(buf,  "Pon Neroth gives you 500 gold coins.\r\n");
	send_to_char(buf, ch);
	GET_GOLD(ch) += 500;

	sprintf(buf, "Pon Neroth says, \'I have a friend named Omel in Jareth who could use your help.\'\n");
	strcat(buf, "Pon Neroth says, \'Give this coin to him and he'll know I sent you.\'\r\n");
	strcat(buf, "Pon Neroth gives you a Smelly Cow clan coin.\r\n");
	send_to_char(buf, ch);

        r_num = real_object(25000);
        item = read_object(r_num, REAL);
        obj_to_char(item, ch);

	return TRUE;
      }
    }
    return FALSE;
  }
return FALSE;
}

SPECIAL(omel) 
{

  struct obj_data *item;
  struct char_data *vict = (struct char_data *) me;
  int r_num;

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25000)) {
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "Omel takes the coin from you.\n");
	strcat(buf, "Omel says, \'Ahh I see Pon sent you. How is the old chap?\'\r\n");
        strcat(buf, "Omel says, \'As I\'m sure Pon told you, I am on a quest for Duke Stalatharata of Malathar to find the dread....\'\n");
        strcat(buf, "Omel get an intense look and his face and his voice turns to sharp whisper.\n");
	send_to_char(buf, ch);
        
        sprintf(buf, "Omel says, \'... /cRBooger Beast/c0 ...\'\n");
        strcat(buf, "Omel says, \'The only problem is every time I am about to catch the little beast, he disappears without a trace.\'\n");
        strcat(buf, "Omel says, \'If you could bring the Booger Beast's nose
back and present it to me I would be most grateful.\'\n");
        strcat(buf, "Omel says, \' You can probably find the vile little beast in the DearthWood\'\r\n");
        send_to_char(buf, ch);
	return TRUE;
      }
    }
    
  }

  if (CMD_IS("present")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25001)) {
         sprintf(buf, "Omel says, \'You Found It!\'\n");
         strcat(buf, "Omel says, \'Now we shall head to Malathar and give it to the Duke!\'\r\n");
         send_to_char(buf, ch);
         if(vict->master != ch) {
           add_follower(vict, ch);
         }

	 obj_from_char(item);
	 extract_obj(item);
         r_num = real_object(25002);
         item = read_object(r_num, REAL);
         obj_to_char(item, ch);
	 return TRUE;
      }
    }


    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25002)) {
        sprintf(buf, "Omel says, \'That nose is stale, find me a new one!\'\n");
         send_to_char(buf, ch);

         obj_from_char(item);
         extract_obj(item);
         return TRUE;
      }
    }
  }
return FALSE;
}

SPECIAL(booger_beast) 
{
  int num = 0;

  if(cmd)
    return 0;

  if (FIGHTING(ch)) {

    switch (number(0, 15)){
    case 0:
      if (number(0, 5) > 4) {
	cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
      }
      break;

    case 5:
      act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
      act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
      call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, ABT_SPELL, 50, CAST_SPELL, FALSE);  
      break;

    }
    return 1;
  }
  num = real_room(number(32001, 32099)); /* 32001 32099 */
  if(num) {
  do_say(ch, "/cGeep, eep/c0", 0, 0);
  act("The booger beast wrinkles its furry nose and vanishes from your sight.",FALSE,ch,0, 0 , TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, num);  
  act("The booger beast pops out of nowhere.", FALSE, ch, 0, 0 , TO_ROOM);
  do_say(ch, "/cGeep, eep/c0", 0, 0);
  }
  return 1;    
} 

#define DUKES_GUARD 25003

SPECIAL(duke_stalatharata)
{
  struct obj_data *item;
  struct char_data *vict = (struct char_data *) me, *leader, *guard;
  int r_num;

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25002) || item->item_number == real_object(25002)) {

          if (!(leader = find_npc_by_name(ch, "Omel", 4))) {
	    do_say(vict, "Who are you! Where is Omel? Guards! Guards!", 0, 0);
	    guard = read_mobile(DUKES_GUARD, VIRTUAL);
            char_to_room(guard, IN_ROOM(ch));
            act("A guard walks in from the north, he looks pissed!", FALSE, vict, 0, 0 , TO_ROOM);
	    hit(guard, ch, TYPE_UNDEFINED);
	    return TRUE;
	  }

	obj_from_char(item);
	extract_obj(item);
        send_to_char("The Duke takes the nose of the booger beast from you.\r\n", ch);
	do_say(vict, "Finally someone has been brave enough to slay the dread booger beast!", 0, 0);
	do_say(vict, "It may have taken two of you to do it, but atleast it is
done.", 0, 0);
        do_say(vict, "Take this seal and give it to Dworn the Dwarf for your reward.", 0, 0);
	send_to_char("The Duke gives you the Seal of Malathar.\r\n", ch);
        r_num = real_object(25003);
        item = read_object(r_num, REAL);
        obj_to_char(item, ch);
        if(GET_RANK(ch) < RANK_CITIZEN && !PLR_FLAGGED(ch, PLR_KEEPRANK)) {
        do_say(vict, "I shall also bestow the rank of Citizen upon you!", 0, 0);
        send_to_char("\r\n\r\n", ch);
         GET_RANK(ch) = RANK_CITIZEN;
        sprintf(buf, "\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n", GET_NAME(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);
        send_to_all(buf);
        }
	return TRUE;
      }
    }
    return FALSE;
  }
return FALSE;
}


SPECIAL(dworn)
{
  struct obj_data *item;
  struct char_data *vict = (struct char_data *) me;

  if(!cmd) {

    switch (number(0, 10)) {
    case 0:
      act("Dworn throws a rock into the sea, it skips one time.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 1:
      act("Dworn throws a rock into the sea, it skips two times.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 2:
      act("Dworn throws a rock into the sea, it skips three times.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 3:
      act("Dworn throws a rock into the sea, it skips four times.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 4:
      act("Dworn throws a rock into the sea, it skips five times.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 5:
      act("Dworn throws a rock into the sea, it skips six times.", FALSE, vict, 0, 0,TO_ROOM);
      return (1);
    case 6:
      act("Dworn throws a rock into the sea, it skips seven times.", FALSE, vict, 0, 0,TO_ROOM);
      do_say(vict, "Woohoo!", 0, 0);
      return (1);
    default:
      return (0);
    }
  }

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25003)) {
	obj_from_char(item);
	extract_obj(item);
	send_to_char("Dworn take the Seal of Malathar from you.\r\n", ch); 
	do_say(vict, "I see you were sent by the Duke.", 0, 0);
	do_say(vict, "I'm afraid I can't teach what you need to know. See the old sailor who lives up on the cliff.", 0, 0);
 
        if (GET_SKILL(ch, SKILL_THROW) == 0) {
	SET_SKILL(ch, SKILL_THROW, 1); 
	}

     sprintf(buf, "QUEST: %s has completed the throw quest.\r\n", GET_NAME(ch));
        log(buf);
	return TRUE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* old_sailor is in spec_guilds.c */
/* End of Throw Quest specs */

SPECIAL(pinky_quest) /* Vedic 7/25/99 */
{

  struct obj_data *item;

  if (CMD_IS("give")) {

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(1080)) {
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "The constable takes the finger from you.\n");
	strcat(buf,  "The constable says, \'You have done a great service in killing Jacque.\'\n");
        strcat(buf,  "The constable says, \'Here is your reward.\'\n");
	strcat(buf,  "The constable gives you 5,000 gold.\n");
	send_to_char(buf, ch);
	GET_GOLD(ch) += 5000;
  
     sprintf(buf, "QUEST: %s has completed the pinky quest.\r\n", GET_NAME(ch));
        log(buf);
	return TRUE;
      }
    }

  }
return FALSE;
}

/* Crack Quest 9/4/99 */
#define CRACK 10
SPECIAL(cracker) 
{

  struct obj_data *item;
  int r_num;

  if (CMD_IS("yes")) { /* Don't say nope, just use dope. */
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(CRACK)) { /* cool refreshing crack */
       sprintf(buf,  "The pale man says, 'You've already got the stuff.'\n");
	send_to_char(buf, ch);
	return TRUE;
      }
    }
    sprintf(buf, "The pale man looks around nervously...\r\n");
    strcat(buf, "The pale man says, 'Here, take this stuff to Tommy in McGintey
                 Cove, He'll pay you real good for it.\n");
    strcat(buf, "The pale man gives you some mysterious white powder.\n");
    send_to_char(buf, ch);

    r_num = real_object(CRACK); /* It does a body good.... */
    item = read_object(r_num, REAL);
    obj_to_char(item, ch);

    return TRUE;
   
  }
  if (CMD_IS("no")) {
    send_to_char("The pale man says, 'You don't believe all of that Just Say No\nshit do you?\n", ch);
    return TRUE;
  }
    
return FALSE;
}

SPECIAL(tommy) 
{

  struct obj_data *item;

  if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(CRACK)) { /* cool refreshing crack */
	obj_from_char(item);
	extract_obj(item);
	sprintf(buf, "Tommy takes the mysterious white powder from you.\n");
	strcat(buf,  "Tommy says, \'Thanks man!\'\n");
	strcat(buf,  "Tommy gives you 3,000 gold.\n");
	send_to_char(buf, ch);
	GET_GOLD(ch) += 3000;

     sprintf(buf, "QUEST: %s has completed the crack quest.\r\n", GET_NAME(ch));
        log(buf);
	return TRUE;
      }
    }
    
  }
return FALSE;
}

/* End of Crack Quest */

/* Giant's Stomach -- Vedic 11/22/99 */

#define GIANT 163

SPECIAL(giant_eats)
{
  struct char_data *giant;

  if (!CMD_IS("west"))
    return FALSE;

if (GET_SKILL(ch, SKILL_DIG)) {
  act("The stomping noise becomes deafening as you notice a huge shadow envelopyou.\n", FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
  act("The stomping noise becomes deafening as you notice a huge shadow envelop you.\n", FALSE, ch, 0, 0, TO_CHAR); 
  giant = read_mobile(GIANT, VIRTUAL);
  char_to_room(giant, IN_ROOM(ch));
  hit(giant, ch, TYPE_UNDEFINED);
return TRUE;
}
else {

  act("The stomping noise becomes deafening as you notice a huge shadow envelop you.\n", FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
  act("The stomping noise becomes deafening as you notice a huge shadow envelop you.\n", FALSE, ch, 0, 0, TO_CHAR); 

act("Before you can think a giant gnarled hand grabs you and you can barely catch a glimpse of the giant's maw before the darkness comes.\r\n",
      FALSE, ch, 0, 0, TO_CHAR);

  act("Before you can think a giant hand grabs $n and swoops $m away.\r\n", FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, real_room(164));
  GET_POS(ch) = POS_STANDING;
  act("$n falls down from above, landing with a splash.\r\n", FALSE, ch, 0, 0, TO_ROOM);
  act("You shake your head to clear it and stand up.\r\n",
      FALSE, ch, 0, 0, TO_CHAR);
  return TRUE;
}
}

#define OGDLEN2 165 

SPECIAL(giant_farts)
{

  struct char_data *leader, *ogdlen;

  if (!CMD_IS("east"))
    return FALSE;

  act("You hear a slight rumbling from behind you. You turn around just in time to see a thick /cggreen cloud/c0 rushing at you.\r\nUh Oh...\r\n/cgFFFFppPPPpppPTTTT!!!/c0\r\n", FALSE, ch, 0, 0, TO_CHAR);

  act("$n is carried away in a massive fart and disappears.\r\n", FALSE, ch, 0, (struct char_data *)me, TO_ROOM);

  if (!(leader = find_npc_by_name(ch, "Ogdlen", 6))) {
    send_to_char("You have the nagging feeling you forgot something.\r\n", ch);
  }
  else {
  //  char_from_room(leader);
    extract_char(leader);
    ogdlen = read_mobile(OGDLEN2, VIRTUAL);
    char_to_room(ogdlen, real_room(174));
  }
  char_from_room(ch);
  char_to_room(ch, real_room(174));
  GET_POS(ch) = POS_STANDING;
  act("$n falls down from above, landing with a thud.", FALSE, ch, 0, 0, TO_ROOM);
  act("You shake your head to clear it and stand up.\r\n",
      FALSE, ch, 0, 0, TO_CHAR);
  return TRUE;
}

#define OGDLEN1  164 

SPECIAL(ogdlen) 
{

  struct char_data *vict = (struct char_data *) me;
  struct char_data *i;

  if (!cmd) {
    if (GET_POS(vict) == POS_RESTING) {
      if (number(0, 1) == 0) {
	act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
	do_say(vict, "Hi, I seem to be lost. Will you help me out of here?", 0, 0);
	send_to_char("Type OKAY to agree.\r\n", ch);
	return TRUE;
      }
    }
  }

  if (CMD_IS("okay")) {
 	if(vict->master == ch) {
	 sprintf(buf, "Ogdlen says, 'You're already helping me.'\n");
	 send_to_char(buf, ch);
         return TRUE;
        } 
/* MATT */
        for (i = character_list; i; i = i->next)
          if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT &&
              i->player.name != ch->player.name) {
            if (zone_table[world[real_room(100)].zone].number ==
                zone_table[world[IN_ROOM(i)].zone].number) {
       send_to_char("Sorry, someone else is doing the quest right now.", ch);
       send_to_char("You'll have to wait until they are out of this zone.\r\n", ch);
       return TRUE;
          }
         }
         sprintf(buf, "Ogdlen says, 'Good Deal! Let's Go!'\n");
         send_to_char(buf, ch);
	 do_stand(vict, 0, 0, 0);
         add_follower(vict, ch);
	 return TRUE;
        
  }
return FALSE;
}

SPECIAL(ogdlen2) 
{

  struct char_data *vict = (struct char_data *) me;

  if (!cmd) {
    if (number(0, 1) == 0) {
      do_say(vict, "Wow! You did it.", 0, 0);
      send_to_char("Type OKAY for a reward.\r\n", ch);
      return TRUE;
    }
  }

  if (CMD_IS("okay")) {
  
         sprintf(buf, "Ogdlen says, 'I shall now instruct you in an ancient Dwarven skill'\n");
	 strcat(buf, "Ogdlen teaches you how to digup and bury items!\r\n");
         send_to_char(buf, ch);
	 if (!(GET_SKILL(ch, SKILL_DIG))) {
	   SET_SKILL(ch, SKILL_DIG, 100); 
	   SET_SKILL(ch, SKILL_BURY, 100);
           GET_CHA(ch) += 1; 
	 }
	 send_to_char("Ogdlen waves goodbye and leaves east.\r\n", ch);
	 extract_char(vict);

     sprintf(buf, "QUEST: %s has completed the Giant quest.\r\n", GET_NAME(ch));
        log(buf);
        if(GET_RANK(ch) < RANK_LORD && !PLR_FLAGGED(ch, PLR_KEEPRANK)) {

         GET_RANK(ch) = RANK_LORD;
        sprintf(buf, "\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n", 
         GET_NAME(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);
        send_to_all(buf);
        }
	 return TRUE;
        
  }
return FALSE;
}

SPECIAL(mean_goblin)
{
  struct char_data *vict = (struct char_data *) me, *leader;
  leader = find_npc_by_name(ch, "Ogdlen", 6);

  if (cmd) {
    return FALSE;
  }
	
  if (!(leader)) { return FALSE; }

  if (FIGHTING((struct char_data *)me)) {
    switch(number(0, 10)) {
    case 0:
      do_generic_skill(vict, leader, SKILL_ROUNDHOUSE, 0);
      break;
    case 1:
      break;
    case 2:
      break;
    case 3: case 4: case 5: case 6: case 7:
      do_generic_skill(vict, leader, SKILL_KNEE, 0);
      break;
    default:
      break;

    }
    return TRUE;
  }
  if (!(leader = find_npc_by_name(ch, "Ogdlen", 6))) {
    return FALSE;
  }
  else {
     do_mob_bash(vict, leader);
     return TRUE;
  }
  return FALSE;
}
/* End of Giant's Stomach */

// Fairy Follower Quest 21022000 

SPECIAL(fairy_follower)
{
 
  if (!ch->master)
    return FALSE;

  if (ch->master->in_room != ch->in_room)
    return FALSE;

  if (cmd) 
    return FALSE;

   if (FIGHTING(ch->master)) {
     if(GET_HIT(ch->master) < (GET_MAX_HIT(ch->master) / 4)) {
       mobsay(ch, "Flee!");
       if(!FIGHTING(ch)) {
         do_assist(ch, GET_NAME(ch->master), 0, 0);
       }
       if(!number(0, 2) && FIGHTING(ch)) {
        cast_spell(ch, FIGHTING(ch), NULL, SPELL_TELEPORT, ABT_SPELL, FALSE);
        mobaction(ch, 0, "laugh");
       }
       return TRUE;
     }
   }   
   switch (number(0, 70)) {
   case 0:
     cast_spell(ch, ch->master, NULL, SPELL_BLESS, ABT_SPELL, FALSE);
     mobaction(ch, GET_NAME(ch->master), "hug");
     return TRUE;
   case 5:
     cast_spell(ch, ch->master, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
     mobaction(ch, "fairy", "giggle");
     return TRUE;
   case 10:
     cast_spell(ch, ch->master, NULL, SPELL_ARMOR, ABT_SPELL, FALSE);
     return TRUE;
   case 15:
     cast_spell(ch, ch->master, NULL, SPELL_REFRESH, ABT_SPELL, FALSE);
     mobaction(ch, GET_NAME(ch->master), "kiss");
     return TRUE;
   case 20:
     cast_spell(ch, ch->master, NULL, SPELL_FLY, ABT_SPELL, FALSE);
     mobaction(ch, GET_NAME(ch->master), "flirt");
     return TRUE;
   case 25:
       cast_spell(ch, ch->master, NULL, SPELL_REGENERATE, ABT_SPELL, FALSE);
       return TRUE;
  case 30:
       cast_spell(ch, ch->master, NULL, SPELL_INVISIBLE, ABT_SPELL, FALSE);
       mobaction(ch, GET_NAME(ch->master), "kiss");
       return TRUE;
  default:
    return FALSE;

  } // switch
}

#define FAIRY 25009 

ACMD(do_fairyfollower)
{
 
  struct char_data *fairy;
  struct follow_type *f;

  if (!GET_SKILL(ch, SKILL_FAIRYFOLLOWER)) {
    send_to_char("You don't know how to do that.\r\n", ch);
    return;
  }

  for (f = ch->followers; f; f = f->next) {
    if (IN_ROOM(ch) == IN_ROOM(f->follower)) {
      if (IS_MOB(f->follower)) {
	send_to_char("You can't attract any fairies right now.\r\n", ch);
	return;
    
      }
    }
  }

  if(GET_MOVE(ch) <= 100 || !IS_GOOD(ch)) {
    send_to_char("Fairies only follow the good of heart.\r\n", ch); 
    return;
  }
  else {
    improve_abil(ch, SKILL_FAIRYFOLLOWER, ABT_SKILL);
    fairy = read_mobile(FAIRY, VIRTUAL);
    char_to_room(fairy, IN_ROOM(ch));
    act("You summon $n.\n", FALSE, fairy, 0, ch, TO_CHAR);
    act("You summon $n.\n", FALSE, fairy, 0, ch, TO_VICT);
    act("$N summons $n.\n", FALSE, fairy, 0, ch, TO_NOTVICT);
    add_follower(fairy, ch);
    mobaction(fairy, GET_NAME(ch), "greet");
    GET_MOVE(ch) -= number(80, 99); 
    WAIT_STATE(ch, PULSE_VIOLENCE * 3); 

  }
}

#define BOTTLE 25009 

SPECIAL(fairy_catch) 
{
  int num = 0, chance, r_num;
  struct obj_data *obj, *item;
  struct char_data *vict = (struct char_data *) me;

  obj = get_obj_in_list_vis(ch, "bottle", ch->carrying);
  chance = number(0, 65) + GET_LUCK(ch) + GET_DEX(ch);

if (!cmd) { 
  num = real_room(number(32001, 32099)); /* 601 699 */
  if(num) {
 act("A fairy giggles and vanishes from your sight.", FALSE, vict, 0, 0 , TO_ROOM);

    char_from_room(vict);
    char_to_room(vict, num);
    act("A fairy pops out of nowhere.", FALSE, vict, 0, 0, TO_ROOM);
	return TRUE;
  }
  

return FALSE; 
}

  if (CMD_IS("catch")) {
    if(!(obj)) {
          act("You try to catch $n with your bare hands, but fall flat on your face in the attempt.", FALSE, vict, 0, ch, TO_CHAR);
          act("You try to catch $n with your bare hands, but fall flat on your face in the attempt.", FALSE, vict, 0, ch, TO_VICT);
	  act("$N tries to catch $n with $m bare hands, but falls flat on $m face in the attempt.", FALSE, vict, 0, ch, TO_NOTVICT);
	  GET_POS(ch) = POS_SITTING;
	  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
	  return TRUE;
    }
    if(chance > number(70, 100)) { 
           act("With a superb swipe, you catch $n in a bottle.", FALSE, vict, 0, ch, TO_CHAR);
          act("With a superb swipe, you catch $n in a bottle.", FALSE, vict, 0, ch, TO_VICT);
	  act("With a superb swipe, $N catches $n in a bottle.", FALSE, vict, 0, ch, TO_NOTVICT);

          r_num = real_object(BOTTLE); /* ... */
          item = read_object(r_num, REAL);
          obj_to_char(item, ch);
          obj_from_char(obj);
          extract_obj(obj);
          extract_char(vict);

          return TRUE;

    }
    else {
          act("You try to catch $n in a bottle, but fall flat on your face in the attempt.", FALSE, vict, 0, ch, TO_CHAR);
          act("You try to catch $n in a bottle, but fall flat on your face in the attempt.", FALSE, vict, 0, ch, TO_VICT);
	  act("$N tries to catch $n in a bottle, but falls flat on $m face in the attempt.", FALSE, vict, 0, ch, TO_NOTVICT);
	  GET_POS(ch) = POS_SITTING;
	  WAIT_STATE(ch, PULSE_VIOLENCE * 3);
	  return TRUE;

    }
return FALSE;

  }
  return FALSE;
} 

SPECIAL(fairy_hermit)
{
  struct obj_data *item;
  struct char_data *vict = (struct char_data *) me;

  if(!cmd) {
	if(!number(0, 15)) { 
          mobsay(vict, "I am in need of assistance brave sir!");
          mobsay(vict, "Type OKAY to hear my sad tale.");
          return TRUE;
       }
       return FALSE;
  }

if(CMD_IS("okay")) {
mobsay(vict, "Long ago I wasn't the horrid and wretched man I am now.");
mobsay(vict, "I had a family and land, a good life. Then one day a terrible");
mobsay(vict, "plague swept across the land, killing my crops, and my family.");
mobsay(vict, "I was one of the lucky ones, it just left me like this.");
mobsay(vict, "I am to weak to help myself now. I need you to capture a fairy");
mobsay(vict, "in a bottle and bring it back to me, it is my only hope.");
mobsay(vict, "I will pay you everything I have. Please do this.");
send_to_char("CATCH a fairy in bottle and bring it back to the Hermit for a reward.\r\n", ch);
	return TRUE;
}

if (CMD_IS("give")) {
  
    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(BOTTLE)) {
	obj_from_char(item);
	extract_obj(item);
send_to_char("The Hermit takes the fairy in a bottle from you.\r\n", ch);        
send_to_char("The Hermit takes the bottle and adds some burbon and spice.\r\n", ch);
send_to_char("After shaking the bottle harshly for a few minutes, the Hermits drinks it down\r\n in one gulp, fairy and all.\r\n", ch); 
send_to_char("The Hermit's skin glows brightly for moment, blinding you.\r\n", ch);
send_to_char("When your sight returns you see a handsome young man standing in the place
of the hermit.\r\n", ch);
mobsay(vict, "The only way I know how to repay you is to impart some of the");
mobsay(vict, "magic of the fairy upon you. For now on you will be able to");
mobsay(vict, "summon fairies at will with the FAIRYFOLLOWER skill!");
send_to_char("The Hermit waves and runs off naked into the forest.\r\n", ch); 
extract_char(vict); 
        if (GET_SKILL(ch, SKILL_FAIRYFOLLOWER) == 0) {
	SET_SKILL(ch, SKILL_FAIRYFOLLOWER, 50); 
	}
	return TRUE;

     sprintf(buf, "QUEST: %s has completed the fairy quest.\r\n", GET_NAME(ch));
        log(buf);
      }
    }
    return FALSE;
}
return FALSE;
}

// End of Fairy Follower

// Library Quest 03042000
SPECIAL(library_quest)
{

  struct char_data *vict = (struct char_data *) me;
  struct obj_data *item;

  if(CMD_IS("okay")) {
mobsay(vict, "The library is losing alot of books.");
mobsay(vict, "Find them and give them to me, you will get a reward.");
    return TRUE;
  }

  if (CMD_IS("give")) {

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25011)) { 
        obj_from_char(item);
        extract_obj(item);
        send_to_char("The Librarian's assistant takes the book from you.\r\n", ch);
mobsay(vict, "You found it! Thank You! Here is your reward!");
       mobaction(vict, GET_NAME(ch), "hug");
       send_to_char("The Librarian's assistant heals you fully.\r\n", ch); 
       GET_HIT(ch)  = GET_MAX_HIT(ch);
       GET_MANA(ch) = GET_MAX_MANA(ch);
       GET_MOVE(ch) = GET_MAX_MOVE(ch);
       GET_QI(ch)   = GET_MAX_QI(ch);
       GET_VIM(ch)  = GET_MAX_VIM(ch);
       GET_ARIA(ch) = GET_MAX_ARIA(ch);


     sprintf(buf, "QUEST: %s has completed the library quest.\r\n", GET_NAME(ch));
        log(buf);
        if(GET_RANK(ch) < RANK_CITIZEN && !PLR_FLAGGED(ch, PLR_KEEPRANK)) {

         GET_RANK(ch) = RANK_CITIZEN;
        sprintf(buf, "\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n", 
                     GET_NAME(ch), rank[(int)GET_RANK(ch)][(int)GET_SEX(ch)]);
        send_to_all(buf);
        }
        return TRUE;
      }
    }

  }
return FALSE;
}

// Arena Quest Vedic 04042000
#define MOBTEN		25010
#define MOBTWENTY	25011
#define MOBTHIRTY	25012
#define MOBFORTY	25013
#define MOBFIFTY	25014
#define MOBSIXTY	25015
#define MOBSEVENTY	25016
#define MOBEIGHTY	25017
#define MOBNINETY	25018
#define MOBHUNDRED	25019
#define MOBONETEN	25020
#define MOBONETWENTY	25021
#define MOBONETHIRTY	25022
#define MOBONEFORTY	25023
#define MOBONEFIFTY	25024
#define ARENA_TOKEN     25031
#define PROOF_TOKEN     25012

SPECIAL(arena_quest_master)
{

  struct char_data *vict = (struct char_data *) me, *arena;
  struct obj_data *item;
  int r_num, exp = 0, found = 0;

  if (CMD_IS("trade")) {
    for (item = ch->carrying; item; item = item->next_content) {
      if (GET_OBJ_VNUM(item) == ARENA_TOKEN) {
        exp = number((GET_LEVEL(ch) * 10), (GET_LEVEL(ch) * 12));
        found = 1;
        obj_from_char(item);
        extract_obj(item);
        sprintf(buf, "You receive %d exp points in trade for your token.\r\n)",                 exp);
        send_to_char(buf, ch);
        act("The Arena Master takes $n's token in trade.\r\n",
             FALSE, ch, 0, 0, TO_ROOM);
        GET_EXP(ch) += exp;
        return 1;
      }
    }
    if (!found) {
        send_to_char("You don't have an Arena Token to trade.\r\n", ch);
        return 1;
    }
  } 

if(CMD_IS("okay")) {
mobsay(vict, "Welcome to the Midgaard Arena!");
mobsay(vict, "If your level is a mulitiple of ten you can challenge one of");
mobsay(vict, "our gladiators to a duel to the death!");
mobsay(vict, "The winner will recieve an increase to a random stat.");
mobsay(vict, "/cRIf you die in the Arena Quest, you will be unable to retrieve your corpse!/c0");
mobsay(vict, "Give me an arena token to enter!");
mobsay(vict, "Or you can head north and practice your killing skills!");
return TRUE;
}

  if(CMD_IS("give")) {
    if (GET_LEVEL(ch) != 10 && GET_LEVEL(ch) != 20 && GET_LEVEL(ch) != 30 && \
        GET_LEVEL(ch) != 40 && GET_LEVEL(ch) != 50 && GET_LEVEL(ch) != 60 && \
        GET_LEVEL(ch) != 70 && GET_LEVEL(ch) != 80 && GET_LEVEL(ch) != 90 && \
       GET_LEVEL(ch) != 100 && GET_LEVEL(ch) != 110 && GET_LEVEL(ch) != 120 && \
        GET_LEVEL(ch) != 130 && GET_LEVEL(ch) != 140 && GET_LEVEL(ch) != 150) {
      send_to_char("Your level must be a multiple of ten.\r\n", ch);
      return FALSE;
    }

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(ARENA_TOKEN)) {
        obj_from_char(item);
        extract_obj(item);

    switch(GET_LEVEL(ch)) {

    case 10:
      arena = read_mobile(MOBTEN, VIRTUAL);
    break;
    case 20:
      arena = read_mobile(MOBTWENTY, VIRTUAL);
    break;
    case 30:
      arena = read_mobile(MOBTHIRTY, VIRTUAL);
    break;
    case 40:
      arena = read_mobile(MOBFORTY, VIRTUAL);
    break;
    case 50:
      arena = read_mobile(MOBFIFTY, VIRTUAL);
    break;
    case 60:
      arena = read_mobile(MOBSIXTY, VIRTUAL);
    break;
    case 70:
      arena = read_mobile(MOBSEVENTY, VIRTUAL);
    break;
    case 80:
      arena = read_mobile(MOBEIGHTY, VIRTUAL);
    break;
    case 90:
      arena = read_mobile(MOBNINETY, VIRTUAL);
    break;
    case 100:
      arena = read_mobile(MOBHUNDRED, VIRTUAL);
    break;
    case 110:
      arena = read_mobile(MOBONETEN, VIRTUAL);
    break;
    case 120:
      arena = read_mobile(MOBONETWENTY, VIRTUAL);
    break;
    case 130:
      arena = read_mobile(MOBONETHIRTY, VIRTUAL);
    break;
    case 140:
      arena = read_mobile(MOBONEFORTY, VIRTUAL);
    break;
    case 150:
      arena = read_mobile(MOBONEFIFTY, VIRTUAL);
    break;
    default:
      mobsay(vict, "Your level must be a multiple of ten to particapate.");
      return TRUE;
    }
     send_to_char("You are whisked away to the arena!", ch);
     act("$N is whisked away to the arena!", FALSE, vict, 0, ch, TO_NOTVICT);
  char_from_room(ch);
  char_to_room(ch, real_room(25051));    

  if (zone_table[world[real_room(25051)].zone].number !=
      zone_table[world[IN_ROOM(arena)].zone].number) {  
    char_to_room(arena, real_room(25052));
    r_num = real_object(PROOF_TOKEN); /* It does a body good.... */
    item = read_object(r_num, REAL);
    obj_to_char(item, arena);
  }

  return TRUE;
  }
  }
  return FALSE;
  }
return FALSE;
}

SPECIAL(arena_quest_exiter)
{

  struct char_data *vict = (struct char_data *) me;
  struct obj_data *item;
  int didadd = FALSE;
  int max;
  
  if(GET_LEVEL(ch) < 50)
    max = 18;
  else if (GET_LEVEL(ch) < 75)
    max = 21;
  else if (GET_LEVEL(ch) < 100)
    max = 23;
  else if (GET_LEVEL(ch) < 125)
    max = 24;
  else /* 125+ */
    max = 25;

  if (CMD_IS("give")) {

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(PROOF_TOKEN)) { 
        obj_from_char(item);
        extract_obj(item);
        send_to_char("The Arena Master takes the token of proof from you.\r\n", ch);
mobsay(vict, "Congratulations on defeating your opponent!");
       send_to_char("The Arena Master heals you fully.\r\n\r\n", ch); 
       GET_HIT(ch)  = GET_MAX_HIT(ch);
       GET_MANA(ch) = GET_MAX_MANA(ch);
       GET_MOVE(ch) = GET_MAX_MOVE(ch);
       GET_QI(ch)   = GET_MAX_QI(ch);
       GET_VIM(ch)  = GET_MAX_VIM(ch);
       GET_ARIA(ch) = GET_MAX_ARIA(ch);
     while(!didadd){  
       switch(number(1, 11)) {
       case 1:
        send_to_char("Your hitpoints increases by 20.\r\n", ch);
        GET_MAX_HIT(ch) += 20;
        didadd = TRUE;
        break;
       case 2:
        send_to_char("Your movepoints increases by 20.\r\n", ch);
        GET_MAX_MOVE(ch) += 20;
        didadd = TRUE;
        break;
       case 3:
        if (!(ch->real_stats.intel >= max)){
          send_to_char("Your INT increases by 1.\r\n", ch);
          ch->real_stats.intel++;
          didadd = TRUE;
        }
        break;
       case 4:
        if (!(ch->real_stats.wis >= max)){
          send_to_char("Your WIS increases by 1.\r\n", ch);
          ch->real_stats.wis++;
          didadd = TRUE;
        }
        break;       
       case 5:
        if (!(ch->real_stats.str >= max)){
          send_to_char("Your STR increases by 1.\r\n", ch);
          ch->real_stats.str++;
          didadd = TRUE;
        }
        break;
       case 6:
        if (!(ch->real_stats.dex >= max)){
          send_to_char("Your DEX increases by 1.\r\n", ch);
          ch->real_stats.dex++;
          didadd = TRUE;
        }
        break;
       case 7:
        if (!(ch->real_stats.con >= max)){
          send_to_char("Your CON increases by 1.\r\n", ch);
          ch->real_stats.con++;
          didadd = TRUE;
        } 
        break;
       case 8:
        if (!(ch->real_stats.luck >= max)){
          send_to_char("Your LUCK increases by 2.\r\n", ch);
          ch->real_stats.luck++;
          didadd = TRUE;
        }
        break;
       case 9:
        if (!(ch->real_stats.cha >= max)){
          send_to_char("Your CHA increases by 2.\r\n", ch);
           ch->real_stats.cha++;
          didadd = TRUE;
        }
        break;
       case 10:
        send_to_char("You get 100,000 gold coins.\r\n", ch);
        GET_GOLD(ch) += 100000;
        didadd = TRUE; 
        break;
       case 11:
        // MANA, VIM, QI
        switch(GET_CLASS(ch)) {
          case CLASS_DRUID:
            send_to_char("Your Vim increases by 50.\r\n", ch);
            GET_MAX_VIM(ch) += 50;
            didadd = TRUE;
            break;
          case CLASS_MONK:
            send_to_char("Your Qi increases by 50.\r\n", ch);
            GET_MAX_QI(ch) += 50;
            didadd = TRUE;
            break;
          case CLASS_PALADIN:
          case CLASS_DARK_KNIGHT:
          case CLASS_CLERIC:
          case CLASS_SORCERER:
            send_to_char("Your Mana increases by 50.\r\n", ch);
            GET_MAX_MANA(ch) += 50;
            didadd = TRUE;
            break;
          default:
            send_to_char("Your Moves increase by 50.\r\n", ch);
            GET_MAX_MOVE(ch) += 50;
            didadd = TRUE;
            break;
         }
       break;
       }
     }
        sprintf(buf, "/cGGRATS:: %s has completed the arena quest!/c0\r\n", GET_NAME(ch));
        send_to_all(buf);
        sprintf(buf, "QUEST: %s has completed the arena quest.\r\n", GET_NAME(ch));
        affect_total(ch);
        log(buf);
      }
    }
  char_from_room(ch);
  char_to_room(ch, real_room(25050)); 
  return TRUE;
  }
return FALSE;
}

// End of Arena Quest


SPECIAL(token_exchange)
{

  struct obj_data *item;

  if(IS_NPC(ch)) { send_to_char("No way dork\r\n", ch); }

  if (CMD_IS("give")) {

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(26000)) {
        obj_from_char(item);
        extract_obj(item);
        send_to_char("The Questman nods and exchanges your token for "
                     "one quest point.\r\n", ch);
        GET_QPOINTS(ch) += 1;
        return TRUE;
      } 
    }
  }
  return FALSE;
}

SPECIAL(quest_shop)
{

  struct obj_data *obj = NULL;

  if (CMD_IS("list")) {
    sprintf(buf, "/cGQuest Equipment Available: /cg(all items 325 qps)\r\n/c0"
           	 "/cWALL QUEST EQ IS LEVEL 50 MINIMUM!!!\r\n"
                 "/cw----------------------------------------------\r\n\r\n"
                 "/cg1.  the Crown of Power\r\n"
                 "2.  Able's Cowl of Magic\r\n"
                 "3.  Kaan's Righteous Visage\r\n"
                 "4.  the Masque of Power\r\n"
                 "5.  the Eye of Dibrova\r\n"
                 "6.  the Eye of Magic\r\n"
    		 "7.  Magneto's Earring of Power\r\n"
		 "8.  Dibrovan Earring of all Magic\r\n"
		 "9.  Inindo's Neckguard of Power\r\n"
		 "10. the Emblem of Magic\r\n"
 		 "11. the Armguards of Wrath\r\n"
		 "12. Dibrovan Circlet of Power\r\n"
		 "13. Tsol's Bracer of Fury\r\n"
		 "14. Dibrovan Bangle of Power\r\n"
		 "15. Vedic's Spiked Gauntlets\r\n"
		 "16. Dibrovan Hand-Wraps\r\n"
		 "17. Dibrovan Thumb Band\r\n"
		 "18. Dibrovan Thumb Loop\r\n"
 		 "19. Dibrovan Cirlce of Power\r\n"
		 "20. Dibrovan Twine of Magic\r\n"
		 "21. Dibrovan Trappings of Rage\r\n"
		 "22. Dibrovan Endowment of Power\r\n"
		 "23. Dibrovan Mantle of Courage\r\n"
		 "24. Dibrovan Cloak of Magic\r\n"
		 "25. Dibrovan Belt of Might\r\n"
		 "26. Dibrovan Sash of Magic\r\n"
		 "27. Dibrovan Leggings of Strength\r\n"
		 "28. Dibrovan Leggings of Magic\r\n"
		 "29. Rodger's Boots of Shitkicking\r\n"
		 "30. Dibrovan Sandals of Magic\r\n"
		 "31. the Essence of Dibrova\r\n"
		 "32. the Magic of Dibrova\r\n"
		 "33. Dibrovan Power Gem\r\n"
		 "34. Dibrovan Rod of Magic\r\n"
		 "35. Dibrovan Shield of Might\r\n"
		 "36. Dibrovan Shield of Magic\r\n"
		 "37. the Light of Dibrova\r\n"
		 "38. the Caster's Light of Dibrova\r\n"
		 "39. The Dibrovan Anklet of Power\r\n"
		 "40. The Dibrovan Anklet of Magik\r\n"
		 "41. The Dibrovan Cloak of Power\r\n"
		 "42. The Dibrovan Cloak of Magik\r\n"
		 "43. The Dibrovan Bag of Holding\r\n"
                 "/cW***ALL WEAPONS ARE FOR LEVEL 150 ONLY!!!***\r\n"
		 "/cg44. Dibrovan Dagger of Death\r\n"
		 "45. Dibrovan Sword of Death\r\n"
		 "46. Dibrovan Mace of Sorrows\r\n"
		 "47. Dibrovan Staff of Power\r\n\r\n"
		 "/cGQuest Potions:/cg\r\n"
                 "/cw--------------/cg\r\n\r\n"
                 "48. Blazeward                 5 qps\r\n"
                 "49. Indesctructable Aura     10 qps\r\n\r\n");
    page_string(ch->desc, buf, 1);
    send_to_char(buf, ch);

    return TRUE;
  }

  if (CMD_IS("identify")) {

    int number;

    one_argument(argument, arg);
   
    if (!*arg) {
      send_to_char("You must specify which item number\r\n"
                   "(i.e., identify 2)\r\n", ch); 
      return TRUE;
    }
    
    number = atoi(arg);
 
    if (number <= 0 || number > 49) {
      send_to_char("Only the eq can be identified on the list.\r\n"
                   "Please choose a number between 1 and 48.\r\n", ch);
      return TRUE;
    } 
    if (number >= 1 && number <= 49) {
 
		if (number <= 38)
			obj = read_object((26099 + number), VIRTUAL);
		else
		{
			if (number == 39)
				obj = read_object((26144), VIRTUAL);
			else if (number == 40)
				obj = read_object((26145), VIRTUAL);
			else if (number == 41)
				obj = read_object((26146), VIRTUAL);
			else if (number == 42)
				obj = read_object((26147), VIRTUAL);
			else if (number == 43)
				obj = read_object((26148), VIRTUAL);
			else if (number == 44)
				obj = read_object((26138), VIRTUAL);
			else if (number == 45)
				obj = read_object((26139), VIRTUAL);
			else if (number == 46)
				obj = read_object((26140), VIRTUAL);
			else if (number == 47)
				obj = read_object((26141), VIRTUAL);
			else if (number == 48)
				obj = read_object((26142), VIRTUAL);
			else if (number == 49)
				obj = read_object((26143), VIRTUAL);

		}

      spell_identify(150, ch, 0, obj); 
      return TRUE; 

    } else {
      send_to_char("That is not a valid option for this shop.\r\n"
                   "Please try a number list above (1-42).\r\n", ch);
      return FALSE;
    }
  return FALSE;	
}


  if (CMD_IS("buy")) {

    int q_eq;

    if (IS_NPC(ch)) {
      send_to_char("No chance. Sorry.\r\n", ch);
      return FALSE;
    }

    one_argument(argument, arg);

    if (!*arg) {
      send_to_char("Please choose an item 1-49 on the list.\r\n"
                   "Type 'list' to see what is available.\r\n", ch);
      return TRUE;
    }

    q_eq = atoi(arg);

    if (q_eq <= 0 || q_eq >= 50) {
      send_to_char("Please choose an item 1-49 on the list.\r\n"
                   "Type 'list' to see what is available.\r\n", ch);
      return TRUE;
    }

	if (q_eq <= 38)
			obj = read_object((26099 + q_eq), VIRTUAL);
		else
		{
			if (q_eq == 39)
				obj = read_object(26144, VIRTUAL);
			else if (q_eq == 40)
				obj = read_object(26145, VIRTUAL);
			else if (q_eq == 41)
				obj = read_object(26146, VIRTUAL);
			else if (q_eq == 42)
				obj = read_object(26147, VIRTUAL);
			else if (q_eq == 43)
				obj = read_object(26148, VIRTUAL);
			else if (q_eq == 44)
				obj = read_object(26138, VIRTUAL);
			else if (q_eq == 45)
				obj = read_object(26139, VIRTUAL);
			else if (q_eq == 46)
				obj = read_object(26140, VIRTUAL);
			else if (q_eq == 47)
				obj = read_object(26141, VIRTUAL);
			else if (q_eq == 48)
				obj = read_object(26142, VIRTUAL);
			else if (q_eq == 49)
				obj = read_object(26143, VIRTUAL);

		}

    if (q_eq >=1 && q_eq <= 47) {
      if (GET_QPOINTS(ch) <= 324) {
        sprintf(buf, "You are %d quest points short on that purchase, %s.\r\n",
                (325 - GET_QPOINTS(ch)), GET_NAME(ch));
        send_to_char(buf, ch);
        return TRUE;
      } else {
        obj_to_char(obj, ch);        
        GET_QPOINTS(ch) = (GET_QPOINTS(ch) - 325); 
        sprintf(buf, "The Questor reverantly hands you %s, made specially for you.\r\n",
                      obj->short_description);
        obj->obj_flags.player = GET_IDNUM(ch);
        send_to_char(buf, ch);
        sprintf(buf2, "(QUEST) %s has purchased %s.", GET_NAME(ch), 
                                                      obj->short_description);
        mudlog(buf2, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE); 
        return TRUE;
      }
    }
    if (q_eq == 48) {
      if (GET_QPOINTS(ch) <= 4) {
        sprintf(buf, "You are %d quest points short on that purchase, %s.\r\n",
                (5 - GET_QPOINTS(ch)), GET_NAME(ch));
        send_to_char(buf, ch);
        return TRUE;
        } else {
        obj_to_char(obj, ch);
        GET_QPOINTS(ch) = (GET_QPOINTS(ch) - 5);
        sprintf(buf, "The Questor reverantly hands you %s.\r\n",
                obj->short_description);
        send_to_char(buf, ch);
        return TRUE;
      }
    }
    if (q_eq == 49) {
      if (GET_QPOINTS(ch) <= 9) {
        sprintf(buf, "You are %d quest points short on that purchase, %s.\r\n",
                (9 - GET_QPOINTS(ch)), GET_NAME(ch));
        send_to_char(buf, ch);
        return TRUE;
        } else {
        obj_to_char(obj, ch);
        GET_QPOINTS(ch) = (GET_QPOINTS(ch) - 10);
        sprintf(buf, "The Questor reverantly hands you %s.\r\n",
                      obj->short_description);
        send_to_char(buf, ch);
        return TRUE;
      }
    }  
    return FALSE;
  }
  return FALSE;
}

/* Rub Quest */
SPECIAL(seer_rub)
{

  struct obj_data *item;
  int r_num;

  if (FIGHTING((struct char_data *)me))
    return FALSE;

  if (!cmd) {
    if (number(0, 5) == 0) {
      act("$n lifts $s face skyward.\r\n", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "The Gods tell me that I must have the Orb of Septra.\r\n"
                 "                I would pay dearly for such an item.", 0, 0);
      return TRUE;
    }
  }

  if (CMD_IS("give")) {

    if (GET_SKILL(ch, SKILL_RUB) > 0) {
      send_to_char("You have already completed this quest.\r\n", ch);
      return TRUE;
    }

    for (item = ch->carrying; item; item = item->next_content) {
      if (item->item_number == real_object(25025)) {
        obj_from_char(item);
        extract_obj(item);
        sprintf(buf, "You give the Orb of Septra to the Seer.\n");
        strcat(buf,  "The Seer says, \'You have done well.\'\n");
        strcat(buf,  "The Seer says, \'Please take this as payment.\'\n"); 
        strcat(buf,  "The Seer gives you a floating orb.\n");
        strcat(buf,  "The Seer says, \'As well, please receive the skill to rub blindness from your eyes.\n");
        strcat(buf,  "You now are learned in the rub skill.\r\n");
        send_to_char(buf, ch);

        r_num = real_object(25026);
        item = read_object(r_num, REAL);
        obj_to_char(item, ch);

        SET_SKILL(ch, SKILL_RUB, 100); 

        return TRUE;
      }
    }
    return FALSE;
  }
return FALSE;
}

#define GLAD_PIT_START_ROOM     20150
ACMD(do_pitbet)
{
  int num;
  struct char_data *pit_mob;
  char arg1[MAX_STRING_LENGTH];

  two_arguments(argument, arg, arg1);

  if (ch->in_room != real_room(6836)) {
    send_to_char("You cannot use that command here.\r\n", ch);
     return;
  }
  if (glad_bet_amnt >= 1) {
    send_to_char("Sorry, someone is already in the Pit.\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < 50) {
    send_to_char("Sorry, no one under level 50 in the Pit.\r\n", ch);
    return;
  }
  if (GET_CLASS(ch) >= 9) {
    send_to_char("Sorry, no remorts in the Pit.\r\n", ch);
    return;
  }
  if (GET_PIT_TIMER(ch) >= 1) {
    send_to_char("Sorry, come back when your pit timer is at zero.\r\n", ch);
    sprintf(buf, "You still have %d minutes left.\r\n", GET_PIT_TIMER(ch));
    send_to_char(buf, ch);
    return;
  }
  if (!*arg) {
    send_to_char("Would you like to bet quest points or gold?\r\n"
                 "Syntax: pitbet <qp or gold> <amnt>\r\n", ch);
    return;
  }
  if (!is_abbrev(arg, "qp") && !is_abbrev(arg, "gold")) {
    send_to_char("You can only bet quest points or gold.\r\n"
                 "Syntax: pitbet qp <amnt>\r\n"
                 "               or\r\n"
                 "       pitbet gold <amnt>\r\n", ch);
    return;
  }
  if (!*arg1) {
    sprintf(buf, "How much do you want to bet?\r\n");
    send_to_char(buf, ch);
    return;
  }

  /* now that we know no one else is in the pit and know *
   * how many of which bet the player wants to wager,    *
   * let's put them into the pit                         */

  glad_bet_amnt = atoi(arg1);
  num = real_room(GLAD_PIT_START_ROOM);

  if (is_abbrev(arg, "qp"))
    SET_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_QP);
  else if (is_abbrev(arg, "gold"))
    SET_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_GOLD);
  else { /* we should never get here */
    send_to_char("Something went wrong. Fight canceled.\r\n", ch);
    return;
  }

  /* Make sure they haven't bet more than they have */
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) && glad_bet_amnt > GET_QPOINTS(ch)) {
   send_to_char("Sorry, you don't have that many quest points to bet.\r\n", ch);
   REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_QP);
   glad_bet_amnt = 0;
   return;
  }
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD) && glad_bet_amnt > GET_GOLD(ch)) {
   send_to_char("Sorry, you don't have that much gold to bet.\r\n", ch);
   REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_GOLD);
   glad_bet_amnt = 0;
   return;
  }

  /* Set some maxes for betting amounts */
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) && glad_bet_amnt > 10) {
    send_to_char("Sorry, 10 qps is the max you can bet on yourself.\r\n", ch);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_QP);
    glad_bet_amnt = 0;
    return;
  }
  if (PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD) && glad_bet_amnt > 100000) {
    send_to_char("Sorry, 100,000 gold is the max you can bet on yourself.\r\n", ch);
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_GOLD);
    glad_bet_amnt = 0;
    return;
  }

  /* Now take them into the Pit, remove all their affects, *
   * and give them a mob to fight                          */

  if (ch->affected) {
    while (ch->affected)
    affect_remove(ch, ch->affected);
    send_to_char("All spells removed.\r\n", ch);
  }

  char_from_room(ch);
  char_to_room(ch, num);
  look_at_room(ch, 0);

  switch(GET_LEVEL(ch)) {
    case 50: case 51: case 52: case 53: case 54: case 55:
      pit_mob = read_mobile(number(11712, 11717), VIRTUAL);
      break;
    case 56: case 57: case 58: case 59: case 60: case 61:
      pit_mob = read_mobile(number(13204, 13206), VIRTUAL);
      break;
    case 62: case 63: case 64: case 65: case 66: case 67:
      pit_mob = read_mobile(number(11900, 11904), VIRTUAL);
      break;
    case 68: case 69: case 70: case 71: case 72: case 73:
      pit_mob = read_mobile(number(9704, 9709), VIRTUAL);
      break;
    case 74: case 75: case 76: case 77: case 78: case 79:
      pit_mob = read_mobile(number(7517, 7519), VIRTUAL);
      break;
    case 80: case 81: case 82: case 83: case 84: case 85:
      pit_mob = read_mobile(number(4728, 4732), VIRTUAL);
      break;
    case 86: case 87: case 88: case 89: case 90: case 91:
      pit_mob = read_mobile(number(3221, 3229), VIRTUAL);
      break;
    case 92: case 93: case 94: case 95: case 96: case 97:
      pit_mob = read_mobile(number(1402, 1404), VIRTUAL);
      break;
    case 98: case 99: case 100: case 101: case 102: case 103:
      pit_mob = read_mobile(number(9336, 9337), VIRTUAL);
      break;
    case 104: case 105: case 106: case 107: case 108: case 109:
      pit_mob = read_mobile(number(4701, 4704), VIRTUAL);
      break;
    case 110: case 111: case 112: case 113: case 114: case 115:
      pit_mob = read_mobile(number(804, 815), VIRTUAL);
      break;
    case 116: case 117: case 118: case 119: case 120: case 130:
      pit_mob = read_mobile(number(7845, 7848), VIRTUAL);
      break;
    case 131: case 132: case 133: case 134: case 135: case 136:
      pit_mob = read_mobile(number(7845, 7848), VIRTUAL);
      break;
    case 137: case 138: case 139: case 140: case 141: case 142:
      pit_mob = read_mobile(number(7831, 7833), VIRTUAL);
      break;
    case 143: case 144: case 145: case 146: case 147: case 148:
      pit_mob = read_mobile(number(7841, 7847), VIRTUAL);
      break;
    case 149: case 150:
      pit_mob = read_mobile(number(13410, 13417), VIRTUAL);
      break;
    default:
      pit_mob = read_mobile(number(13410, 13417), VIRTUAL);
      break;
  }

  char_to_room(pit_mob, real_room(number(20100, 20199)));
  pit_mob_vnum = GET_MOB_VNUM(pit_mob);

  send_to_char("\r\nYou are taken away to the Pit.\r\n"
               "May whatever gods you worship protect you.\r\n", ch);
  act("$n is ushered into the pit. Wish $m luck.\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  sprintf(buf, "\r\n/cR%s has entered the gladiator pit.\r\n"
               "%s will be facing %s in a dual to the death!/c0\r\n",
               GET_NAME(ch), GET_NAME(ch), GET_NAME(pit_mob));
  send_to_all(buf);
  return;
}
