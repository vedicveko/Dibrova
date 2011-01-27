/**************************************************************************
 * Usage: This is code for guilds, trainers, and the like.                *
 * Created: 7/13/99                                                       *
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
extern struct zone_data *zone_table;  
extern int drink_aff[][3];
extern int newbie_level;
extern char *prac_types[];

int spell_sort_info[MAX_ABILITIES+1];
int skill_sort_info[MAX_ABILITIES+1];
int chant_sort_info[MAX_ABILITIES+1];
int prayer_sort_info[MAX_ABILITIES+1];
int song_sort_info[MAX_ABILITIES+1];
int double_exp = 0;

extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
extern char *skills[];
extern char *spells[];
extern char *chants[];
extern char *prayers[];
extern char *songs[];
extern struct int_app_type int_app[];
extern int prac_params[4][NUM_CLASSES];
extern int level_exp(int class, int level);

int perform_remove(struct char_data *ch, int pos);

void sort_spells(void)
{
  int a, b=0, tmp;

  /* initialize array */
  for (a = 1; a < MAX_ABILITIES; a++)
    spell_sort_info[a] = a;

  /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */

  for (a = 1; a < MAX_ABILITIES - 1 && spell_sort_info[b]!='\n'; a++)
    for (b = a + 1; b < MAX_ABILITIES; b++)
      if (strcmp(spells[spell_sort_info[a]], spells[spell_sort_info[b]]) > 0) 
      {
	     tmp = spell_sort_info[a];
	     spell_sort_info[a] = spell_sort_info[b];
	     spell_sort_info[b] = tmp;
      }
}

char *how_good(int percent)
{
  static char buf[256];

  if (percent == 0)
    strcpy(buf, " /cw(/cc---------------/cw)/c0");
  else if (percent <= 10)
    strcpy(buf, " /cw(/cr=/cc--------------/cw)/c0");
  else if (percent <= 20)
    strcpy(buf, " /cw(/cr===/cc------------/cw)/c0");
  else if (percent <= 30)
    strcpy(buf, " /cw(/cr=====/cc----------/cw)/c0");
  else if (percent <= 40)
    strcpy(buf, " /cw(/cr=====/cy==/cc--------/cw)/c0");
  else if (percent <= 50)
    strcpy(buf, " /cw(/cr=====/cy====/cc------/cw)/c0");
  else if (percent <= 60)
    strcpy(buf, " /cw(/cr=====/cy=====/cg=/cc----/cw)/c0");
  else if (percent <= 70)
    strcpy(buf, " /cw(/cr=====/cy=====/cg==/cc---/cw)/c0");
  else if (percent <= 80)
    strcpy(buf, " /cw(/cr=====/cy=====/cg===/cc--/cw)/c0");
  else if (percent <= 90)
    strcpy(buf, " /cw(/cr=====/cy=====/cg====/cc-/cw)/c0");
  else
    strcpy(buf, " /cw(/cr=====/cy=====/cg=====/cw)/c0");

  return (buf);
}


#define LEARNED_LEVEL	0	/* % known which is considered "learned" */
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
#define PRAC_TYPE	3	/* should it say 'spell' or 'skill'?	 */

#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
#define MINGAIN(ch) (prac_params[MIN_PER_PRAC][(int)GET_CLASS(ch)])
#define MAXGAIN(ch) (prac_params[MAX_PER_PRAC][(int)GET_CLASS(ch)])
#define ABIL(ch)    (prac_types[prac_params[PRAC_TYPE][(int)GET_CLASS(ch)]])

void list_abilities(struct char_data * ch)
{
  int i, pt, ct, sortpos;
  char buf[100];
  char **abil_list;

  *buf = '\0';
  *buf2 = '\0';

  if (!GET_PRACTICES(ch))
    strcpy(buf, "You have no practice sessions remaining.\r\n");
  else
    sprintf(buf, "You have %d practice session%s remaining.\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));
  send_to_char(buf, ch);

  for (pt = 0; pt < 5; pt++) {
    switch (pt) {
    case ABT_SKILL: abil_list = skills;  break;
    case ABT_SPELL: abil_list = spells;  break;
    case ABT_CHANT: abil_list = chants;  break;
    case ABT_PRAYER:abil_list = prayers; break;
    case ABT_SONG:  abil_list = songs;   break;
    default:
      send_to_char("Something fucked up.\r\n", ch);
      return;
      break;
    }
    if (!CAN_CAST(ch) && pt == ABT_SPELL)  continue;
    if (!CAN_CHANT(ch)&& pt == ABT_CHANT)  continue;
    if (!CAN_PRAY(ch) && pt == ABT_PRAYER) continue;
    if (!CAN_SING(ch) && pt == ABT_SONG)   continue; 
    sprintf(buf2, "%s\r\nYou know the following class based %ss:\r\n", buf2,
            prac_types[pt]);

    for (ct=0, sortpos = 1; sortpos < MAX_ABILITIES; sortpos++) {
      i = sortpos;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
      strcat(buf2, "**OVERFLOW**\r\n");
      break;
    }
    if ((GET_LEVEL(ch) >= abil_info[pt][i].min_level[(int)GET_CLASS(ch)]) && 
         GET_CLASS(ch) < CLASS_VAMPIRE) { 
      sprintf(buf2, "%s%-20s %s\r\n", buf2, abil_list[i], 
              how_good(get_ability(ch, pt, i)));
      ct++;
    }
    if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
      if (get_ability(ch, pt, i) != 0) {
        sprintf(buf2, "%s%-20s %s\r\n", buf2, abil_list[i], 
                how_good(get_ability(ch, pt, i)));
        ct++;
      }
    }
  /* MAYBE */
  }
    if (!ct) strcat(buf2, " -none-\r\n");
  }
  page_string(ch->desc, buf2, 1);
}

void list_ability(struct char_data * ch, int num, int type) {

  char **abil_list;
  
  switch (type) {
    case ABT_SKILL: abil_list = skills;  break;
    case ABT_SPELL: abil_list = spells;  break;
    case ABT_CHANT: abil_list = chants;  break;
    case ABT_PRAYER:abil_list = prayers; break;
    case ABT_SONG:  abil_list = songs;   break;
    default:
      send_to_char("Something fucked up.\r\n", ch);
      return;
      break;
  }
  sprintf(buf2, "%s%-20s %s\r\n", buf2, abil_list[num], how_good(get_ability(ch, type, num)));
  page_string(ch->desc, buf, 1);
}

int generic_skillmaster(struct char_data *ch, int cmd, char * argument,
                        int skill_num, int minlevs[NUM_CLASSES], int cost,
                        int class) {
  int percent, skill_num2;
  
  if (IS_NPC(ch) || (!CMD_IS("practice")))
    return 0;

  if(GET_EXP(ch) < 1) {
    send_to_char("No sorry.\r\n", ch);
    return 1;
  }

  skip_spaces(&argument);
  
  if ((CMD_IS("practice")) && (!*argument)) 
  {
    if (!GET_PRACTICES(ch))
      strcpy(buf, "You have no practice sessions remaining.\r\n");
    else
      sprintf(buf, "You have %d practice session%s remaining.\r\n\r\n",
	    GET_PRACTICES(ch), (GET_PRACTICES(ch) == 1 ? "" : "s"));
    send_to_char("The skillmaster says, 'I can teach you the following "
                 "abilities:'\r\n", ch);
    list_ability(ch, skill_num, ABT_SKILL);
    return 1; 
  }
  
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return 1; 
  }
  
  skill_num2 = find_abil_num(argument, skills);
  if (skill_num != skill_num2) {
    sprintf(buf, "The skillmaster says, 'I can only teach you to %s.'\r\n",
            skills[skill_num] );
    send_to_char(buf, ch);
    return 1; 
  }

  if(GET_LEVEL(ch) < minlevs[class]) {
    sprintf(buf, "The skillmaster says, 'You aren't experienced "
                 "enough to learn this skill.'\r\n");
    send_to_char(buf, ch);
    return 1;
  }

  if(GET_GOLD(ch) <= cost) {
      send_to_char("You do not have enough money!\r\n", ch);
      return 1; }
    
  if (GET_SKILL(ch, skill_num) >= LEARNED(ch)) {
    send_to_char("You are already learned in that area.\r\n", ch);
    return 1; }

  GET_GOLD(ch) -=  cost; 

  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);

  return 1;
}

int generic_guild(struct char_data *ch, void *me, int cmd, char *argument, 
                  int class_type)
{
  int skill_num, percent, type, r_num;
//  int exp_needed;
  struct descriptor_data *pt;
  struct obj_data *item;
  
  if (IS_NPC(ch) || (!CMD_IS("practice") && !CMD_IS("gain")))
    return 0;

  if(GET_EXP(ch) < 1) {
    send_to_char("No sorry.\r\n", ch);
    return 1;
  }
  
  skip_spaces(&argument);

  if (CMD_IS("gain") && ((GET_CLASS(ch) == class_type))) {
 
       if (GET_LEVEL(ch) >= LVL_IMMORT) {
          send_to_char("The guildmaster says '",ch);
	  send_to_char("I am good but not that good.'\r\n",ch);
	  return 1;
       }

       if (GET_LEVEL(ch) == LVL_IMMORT -1) {
	 send_to_char("Your guildmaster says '",ch);
	 send_to_char(" I have taught you everything I know.\r\n",ch);
	 return 1;
       }
       
       if (GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)){
	 GET_LEVEL(ch) += 1;
	 advance_level(ch);
	 if(!PRF_FLAGGED(ch, PRF_AUTOTITLE))
           set_title(ch, NULL);
	 sprintf(buf, "Congratulations!  You've advanced to level %d!\r\n",
		 GET_LEVEL(ch));
	 send_to_char(buf, ch);
	 if (GET_LEVEL(ch) == newbie_level+1)
	   send_to_char("/cWWARNING: You will no longer be able to "
			"recall for free!/c0", ch);
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

/*
	 sprintf(buf, "/cGGRATS:: %s has gained a level!/c0\r\n", GET_NAME(ch));
	 GET_HIT(ch) = GET_MAX_HIT(ch);
	 GET_MANA(ch) = GET_MAX_MANA(ch);
	 GET_MOVE(ch) = GET_MAX_MOVE(ch);
	 GET_QI(ch) = GET_MAX_QI(ch);
	 GET_VIM(ch) = GET_MAX_VIM(ch);
	 GET_ARIA(ch) = GET_MAX_ARIA(ch);
	 for (pt = descriptor_list; pt; pt = pt->next)
	   if (!pt->connected && pt->character && pt->character != ch)
	     send_to_char(buf, pt->character);
*/

/* ****** LEVEL QUEST ******* */
send_to_char("Boo.\r\n", ch);

  
      if (GET_LEVEL(ch) <= 5) {
        char_from_room(ch);
        char_to_room(ch, real_room(26750));
      }
      if (GET_LEVEL(ch) <= 10 && GET_LEVEL(ch) > 5) {
        char_from_room(ch);
        char_to_room(ch, real_room(26751));
      }
      if (GET_LEVEL(ch) <= 15 && GET_LEVEL(ch) > 10) {
        char_from_room(ch);
        char_to_room(ch, real_room(26752));
      }
      if (GET_LEVEL(ch) <= 20 && GET_LEVEL(ch) > 15) {
        char_from_room(ch);
        char_to_room(ch, real_room(26753));
      }
      if (GET_LEVEL(ch) <= 25 && GET_LEVEL(ch) > 20) {
        char_from_room(ch);
        char_to_room(ch, real_room(26754));
      }
      if (GET_LEVEL(ch) <= 30 && GET_LEVEL(ch) > 25) {
        char_from_room(ch);
        char_to_room(ch, real_room(26755));
      }
      if (GET_LEVEL(ch) <= 35 && GET_LEVEL(ch) > 30) {
        char_from_room(ch);
        char_to_room(ch, real_room(26756));
      }
      if (GET_LEVEL(ch) <= 40 && GET_LEVEL(ch) > 35) {
        char_from_room(ch);
        char_to_room(ch, real_room(26757));
      }
      if (GET_LEVEL(ch) <= 45 && GET_LEVEL(ch) > 40) {
        char_from_room(ch);
        char_to_room(ch, real_room(26758));
      }
      if (GET_LEVEL(ch) <= 50 && GET_LEVEL(ch) > 45) {
        char_from_room(ch);
        char_to_room(ch, real_room(26759));
      }
      if (GET_LEVEL(ch) <= 55 && GET_LEVEL(ch) > 50) {
        char_from_room(ch);
        char_to_room(ch, real_room(26760));
      }
      if (GET_LEVEL(ch) <= 60 && GET_LEVEL(ch) > 55) {
        char_from_room(ch);
        char_to_room(ch, real_room(26761));
      }
      if (GET_LEVEL(ch) <= 65 && GET_LEVEL(ch) > 60) {
        char_from_room(ch);
        char_to_room(ch, real_room(26762));
      }
      if (GET_LEVEL(ch) <= 70 && GET_LEVEL(ch) > 65) {
        char_from_room(ch);
        char_to_room(ch, real_room(26763));
      }
      if (GET_LEVEL(ch) <= 75 && GET_LEVEL(ch) > 70) {
        char_from_room(ch);
        char_to_room(ch, real_room(26764));
      }
      if (GET_LEVEL(ch) <= 80 && GET_LEVEL(ch) > 75) {
        char_from_room(ch);
        char_to_room(ch, real_room(26765));
      }
      if (GET_LEVEL(ch) <= 85 && GET_LEVEL(ch) > 80) {
        char_from_room(ch);
        char_to_room(ch, real_room(26766)); 
      }
      if (GET_LEVEL(ch) <= 90 && GET_LEVEL(ch) > 85) {
        char_from_room(ch);
        char_to_room(ch, real_room(26767));
      }
      if (GET_LEVEL(ch) <= 95 && GET_LEVEL(ch) > 90) {
        char_from_room(ch);
        char_to_room(ch, real_room(26768));
      }
      if (GET_LEVEL(ch) <= 100 && GET_LEVEL(ch) > 95) {
        char_from_room(ch);
        char_to_room(ch, real_room(26769));
      }
      if (GET_LEVEL(ch) <= 105 && GET_LEVEL(ch) > 100) {
        char_from_room(ch);
        char_to_room(ch, real_room(26770));
      }
      if (GET_LEVEL(ch) <= 110 && GET_LEVEL(ch) > 105) {
        char_from_room(ch);
        char_to_room(ch, real_room(26771));
      }
      if (GET_LEVEL(ch) <= 115 && GET_LEVEL(ch) > 110) {
        char_from_room(ch);
        char_to_room(ch, real_room(26772));
      }
      if (GET_LEVEL(ch) <= 120 && GET_LEVEL(ch) > 115) {
        char_from_room(ch);
        char_to_room(ch, real_room(26773));
      }
      if (GET_LEVEL(ch) <= 125 && GET_LEVEL(ch) > 120) {
        char_from_room(ch);
        char_to_room(ch, real_room(26774));
      }
      if (GET_LEVEL(ch) <= 130 && GET_LEVEL(ch) > 125) {
        char_from_room(ch);
        char_to_room(ch, real_room(26775));
      }
      if (GET_LEVEL(ch) <= 135 && GET_LEVEL(ch) > 130) {
        char_from_room(ch);
        char_to_room(ch, real_room(26776));
      }
      if (GET_LEVEL(ch) <= 140 && GET_LEVEL(ch) > 135) {
        char_from_room(ch);
        char_to_room(ch, real_room(26777));
      }
      if (GET_LEVEL(ch) <= 145 && GET_LEVEL(ch) > 140) {
        char_from_room(ch);
        char_to_room(ch, real_room(26778));
      }
      if (GET_LEVEL(ch) <= 150 && GET_LEVEL(ch) > 145) {
        char_from_room(ch);
        char_to_room(ch, real_room(26779));
      }
      look_at_room(ch, 0);


     
         sprintf(buf, "/cGGRATS:: %s has gained a level!/c0\r\n", GET_NAME(ch));
         GET_HIT(ch) = GET_MAX_HIT(ch);
         GET_MANA(ch) = GET_MAX_MANA(ch);
         GET_MOVE(ch) = GET_MAX_MOVE(ch);
         GET_QI(ch) = GET_MAX_QI(ch);
         GET_VIM(ch) = GET_MAX_VIM(ch);
         GET_ARIA(ch) = GET_MAX_ARIA(ch);
         for (pt = descriptor_list; pt; pt = pt->next)
           if (!pt->connected && pt->character && pt->character != ch)
             send_to_char(buf, pt->character); 

/* **** END LEVEL QUEST */
	 
       } else {
	 send_to_char("You must earn more experience before "
		      "you may gain.\r\n", ch);
       }
       return 1;
  }
  
  if (CMD_IS("gain") && (GET_CLASS(ch) != class_type)) {
       send_to_char("You do not see your guildmaster here.\r\n",ch);
       return 1;
     }
  if(GET_EXP(ch) < 1) {
    send_to_char("No sorry.\r\n", ch);
    return 1;
  }
  if ((CMD_IS("practice")) && (!*argument) && ((GET_CLASS(ch)==class_type))) {
    list_abilities(ch);
    return 1;
  }
  if ((CMD_IS("practice")) && (GET_CLASS(ch)!=class_type)) {
       send_to_char("You do not see your guild master here.\r\n",ch);
      return 1;
    }
  if (GET_PRACTICES(ch) <= 0) {
    send_to_char("You do not seem to be able to practice now.\r\n", ch);
    return 1;
  }

  skill_num = find_abil_num(argument, skills);
  type = ABT_SKILL;

  if (skill_num < 0 || GET_LEVEL(ch) <
      abil_info[type][skill_num].min_level[(int) GET_CLASS(ch)]) {
    /* No go - check the alternate lists if appropriate */
    if (CAN_CAST(ch)) {
      skill_num = find_abil_num(argument, spells);
      type = ABT_SPELL;
    }  if (CAN_CHANT(ch)) {
      skill_num = find_abil_num(argument, chants);
      type = ABT_CHANT;
    } else if (CAN_PRAY(ch)) {
      skill_num = find_abil_num(argument, prayers);
      type = ABT_PRAYER;
    } else if (CAN_SING(ch)) {
      skill_num = find_abil_num(argument, songs);
      type = ABT_SONG;
    }
  }

  /* check to see if they can practice the spell/skill */
  if (skill_num < 1) {
    send_to_char("The guildmaster says, 'I didn't understand that.'\r\n", ch);
    return 1;
  }

  send_to_char("You practice for a while...\r\n", ch);
  GET_PRACTICES(ch)--;

  percent = GET_SKILL(ch, skill_num);
  percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

  SET_SKILL(ch, skill_num, MIN(LEARNED(ch), percent));

  if (GET_SKILL(ch, skill_num) >= LEARNED(ch))
    send_to_char("You are now learned in that area.\r\n", ch);

  return 1;
}

SPECIAL(guild)
{
  int skill_num, percent, type, d;
  char buf[256];
//  struct descriptor_data *pt;
//  struct obj_data *item;
  int real_zone(int number);
  struct descriptor_data *k;
  char tmpstr[128];
  char *s;

  void reset_zone(int zone);
  void set_title(struct char_data * ch, char *title);
  extern struct int_app_type int_app[];

  if (IS_NPC(ch))
    return 0;

  if (CMD_IS("practice")){


    if (GET_PRACTICES(ch) <= 0) {
      send_to_char("You do not seem to be able to practice now.\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
      send_to_char("I cannot teach you any longer.\r\n"
                   "You have surpassed my teachings.\r\n", ch);
      return 1;
    }

    two_arguments(argument, buf, buf2);
    type = ABT_SKILL;

    if (!*buf) {
      list_abilities(ch);
      return 1;
    }
    if (is_abbrev(buf, "skill"))
      type = ABT_SKILL;
    else if (is_abbrev(buf, "spell"))
      type = ABT_SPELL;
    else if (is_abbrev(buf, "chant"))
      type = ABT_CHANT;
    else if (is_abbrev(buf, "prayer"))
      type = ABT_PRAYER;
    else {
      send_to_char("That is a not a valid ability type.\r\nValid types are: skill, spell, chant, prayer\r\n", ch);
      return 1;
    }

    if (!*buf2) {
      send_to_char("You have to specify which ability!", ch);
      return 1;
    }


    s = strtok(argument, "'");

    s = strtok(NULL, "'");

    if (s == NULL) {
      send_to_char("You must enclose the ability name in singles quotes ( ' ).\r\n", ch);
      return 1;
    }
    skill_num = find_abil_num(s, skills);

    if (is_abbrev(buf, "skill"))
      skill_num = find_abil_num(s, skills);
    else if (is_abbrev(buf, "spell"))
      skill_num = find_abil_num(s, spells);
    else if (is_abbrev(buf, "chant"))
      skill_num = find_abil_num(s, chants);
    else if (is_abbrev(buf, "prayer"))
      skill_num = find_abil_num(s, prayers);


    if (skill_num < 1 || GET_LEVEL(ch) <
        abil_info[type][skill_num].min_level[(int) GET_CLASS(ch)]) {
      sprintf(buf, "You do not know of that %s.\r\n", prac_types[type]);
      send_to_char(buf, ch);
      return 1;
    }
    if (get_ability(ch, type, skill_num) >= LEARNED(ch)) {
      send_to_char("You are already learned in that area.\r\n", ch);
      return 1;
    }
    send_to_char("You practice for a while...\r\n", ch);
    GET_PRACTICES(ch)--;

    percent = get_ability(ch, type, skill_num);
    percent += MIN(MAXGAIN(ch), MAX(MINGAIN(ch), int_app[GET_INT(ch)].learn));

    set_ability(ch, type, skill_num, MIN(LEARNED(ch), percent));

    if (get_ability(ch, type, skill_num) >= LEARNED(ch))
      send_to_char("You are now learned in that area.\r\n", ch);

    return 1;
  }

/* removed - autogain implemented 12/10/01 
  }else if (CMD_IS("gain")){

  if(GET_EXP(ch) < 1 || !GET_EXP(ch)) {
    send_to_char("No sorry.\r\n", ch);
    return 1;
  }
    if (GET_LEVEL(ch) >= LVL_IMMORT-1) {
      send_to_char("You can't gain any more.\r\n", ch);
      return TRUE;
    }

    for (i = character_list; i; i = i->next)
      if (!PRF_FLAGGED(ch, PRF_NOGRID)) {
      if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT) {
          if (zone_table[world[real_room(26000)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number) {
            if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
                (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) || 
                ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
                (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
            sprintf(buf, "%s is currently in the Grid. Please wait "
                         "until they are out.\r\n", GET_NAME(i));
            send_to_char(buf, ch);
            return 1;
            }
        } else
        if( zone_table[world[real_room(26100)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26200)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26300)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26400)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26500)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26600)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        } else
        if( zone_table[world[real_room(26700)].zone].number ==
            zone_table[world[IN_ROOM(i)].zone].number) {
          if (((GET_LEVEL(i) - GET_LEVEL(ch)) <= 25 &&
              (GET_LEVEL(i) - GET_LEVEL(ch)) >= 1) ||
              ((GET_LEVEL(ch) - GET_LEVEL(i)) <= 25 &&
              (GET_LEVEL(ch) - GET_LEVEL(i)) >= 1)) {
          sprintf(buf, "%s is currently in the Grid. Please wait "
                       "until they are out.\r\n", GET_NAME(i));
          send_to_char(buf, ch);
          return 1;
          }
        }
      }
      }

    if (GET_EXP(ch) >= level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1)){

      if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
        if (GET_LEVEL(ch) == 30) {
          GET_MAX_HIT(ch) += number(3, 6);
          GET_LEVEL(ch) = 1;
          GET_TIER(ch) += 1;
          GET_EXP(ch) = 1;
          send_to_char("\r\n/cRYou gain a tier!/c0\r\n\r\n", ch);
          sprintf(buf, "/cR%s has attained tier %d!/c0\r\n", 
                  GET_NAME(ch), GET_TIER (ch));
          send_to_all(buf);
          GET_HIT(ch) = GET_MAX_HIT(ch);
          GET_MANA(ch) = GET_MAX_MANA(ch);
          GET_QI(ch) = GET_MAX_QI(ch);
          GET_VIM(ch) = GET_MAX_VIM(ch);
          GET_MOVE(ch) = GET_MAX_MOVE(ch);
          if (GET_CLASS(ch) == CLASS_TITAN && GET_TIER(ch) == 3)
            SET_SKILL(ch, SKILL_HARNESS_WATER, 95);
          if (GET_CLASS(ch) == CLASS_TITAN && GET_TIER(ch) == 5)
            SET_SKILL(ch, SKILL_HARNESS_FIRE, 95);
          if (GET_CLASS(ch) == CLASS_TITAN && GET_TIER(ch) == 7)
            SET_SKILL(ch, SKILL_HARNESS_EARTH, 95);
        } else {
          GET_LEVEL(ch) += 1;
          advance_level(ch);
          send_to_char("\r\n/cRYou have gained in strength and stamina.\r\n"
                       "You gain a level!/c0\r\n\r\n", ch);
          GET_HIT(ch) = GET_MAX_HIT(ch);
          GET_MANA(ch) = GET_MAX_MANA(ch);
          GET_QI(ch) = GET_MAX_QI(ch);
          GET_VIM(ch) = GET_MAX_VIM(ch);
          GET_MOVE(ch) = GET_MAX_MOVE(ch);
        }
      } if (GET_CLASS(ch) < CLASS_VAMPIRE) {   

      GET_LEVEL(ch) += 1;
      advance_level(ch);
      if(!PRF_FLAGGED(ch, PRF_AUTOTITLE))
        set_title(ch, NULL);
      sprintf(buf, "Congratulations!  You've advanced to level %d!\r\n",
                   GET_LEVEL(ch));
      send_to_char(buf, ch);
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
      sprintf(buf, "/cGGRATS:: %s has gained a level!/c0\r\n", GET_NAME(ch));
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      GET_QI(ch) = GET_MAX_QI(ch);
      GET_VIM(ch) = GET_MAX_VIM(ch);
      for (pt = descriptor_list; pt; pt = pt->next)
        if (!pt->connected && pt->character && pt->character != ch)
          send_to_char(buf, pt->character);
      }

      if (GET_CLASS(ch) <=8) {
        
      if (GET_LEVEL(ch) <= 5 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26000)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26750));
      }
      if (GET_LEVEL(ch) <= 10 && GET_LEVEL(ch) > 5 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26000)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26751));
      }
      if (GET_LEVEL(ch) <= 15 && GET_LEVEL(ch) > 10 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26000)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26752));
      }
      if (GET_LEVEL(ch) <= 20 && GET_LEVEL(ch) > 15 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26000)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26753));
      }
      if (GET_LEVEL(ch) <= 25 && GET_LEVEL(ch) > 20 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26100)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26754));
      } 
      if (GET_LEVEL(ch) <= 30 && GET_LEVEL(ch) > 25 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26100)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26755));
      }
      if (GET_LEVEL(ch) <= 35 && GET_LEVEL(ch) > 30 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26100)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26756));
      }
      if (GET_LEVEL(ch) <= 40 && GET_LEVEL(ch) > 35 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26100)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26757));
      }
      if (GET_LEVEL(ch) <= 45 && GET_LEVEL(ch) > 40 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26200)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26758));
      }
      if (GET_LEVEL(ch) <= 50 && GET_LEVEL(ch) > 45 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26200)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26759));
      } 
      if (GET_LEVEL(ch) <= 55 && GET_LEVEL(ch) > 50 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26200)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26760));
      }
      if (GET_LEVEL(ch) <= 60 && GET_LEVEL(ch) > 55 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26200)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26761));
      }
      if (GET_LEVEL(ch) <= 65 && GET_LEVEL(ch) > 60 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26300)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26762));
      }
      if (GET_LEVEL(ch) <= 70 && GET_LEVEL(ch) > 65 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26300)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26763));
      }
      if (GET_LEVEL(ch) <= 75 && GET_LEVEL(ch) > 70 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26300)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26764));
      }   
      if (GET_LEVEL(ch) <= 80 && GET_LEVEL(ch) > 75 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26300)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26765));
      }
      if (GET_LEVEL(ch) <= 85 && GET_LEVEL(ch) > 80 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26400)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26766));
      }
      if (GET_LEVEL(ch) <= 90 && GET_LEVEL(ch) > 85 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26400)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26767));
      }
      if (GET_LEVEL(ch) <= 95 && GET_LEVEL(ch) > 90 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26400)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26768));
      }
      if (GET_LEVEL(ch) <= 100 && GET_LEVEL(ch) > 95 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26400)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26769));
      } 
      if (GET_LEVEL(ch) <= 105 && GET_LEVEL(ch) > 100 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26500)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26770));
      }
      if (GET_LEVEL(ch) <= 110 && GET_LEVEL(ch) > 105 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26500)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26771));
      }
      if (GET_LEVEL(ch) <= 115 && GET_LEVEL(ch) > 110 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26500)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26772));
      }
      if (GET_LEVEL(ch) <= 120 && GET_LEVEL(ch) > 115 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26500)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26773));
      }
      if (GET_LEVEL(ch) <= 125 && GET_LEVEL(ch) > 120 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26600)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26774));
      }
      if (GET_LEVEL(ch) <= 130 && GET_LEVEL(ch) > 125 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26600)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26775));
      }
      if (GET_LEVEL(ch) <= 135 && GET_LEVEL(ch) > 130 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26600)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26776));
      }
      if (GET_LEVEL(ch) <= 140 && GET_LEVEL(ch) > 135 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26600)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26777));
      }
      if (GET_LEVEL(ch) <= 145 && GET_LEVEL(ch) > 140 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26700)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26778));
      }
      if (GET_LEVEL(ch) <= 150 && GET_LEVEL(ch) > 145 && !PRF_FLAGGED(ch, PRF_NOGRID)) {
        reset_zone(world[real_room(26700)].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(26779));
      }
      if (!PRF_FLAGGED(ch, PRF_NOGRID)) {
        look_at_room(ch, 0);
        if (ch->affected) {
          while (ch->affected)
            affect_remove(ch, ch->affected);
          send_to_char("All spells removed.\r\n", ch);
        }
      }
    }

    } else {
      send_to_char("You must earn more experience before "
                   "you may gain.\r\n", ch);
    }
    return 1;
  }
*/
/* REMORT */

  if (CMD_IS("remort")) {
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      send_to_char("Uh... an immortal remorting?!\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) >= 9) {
      send_to_char("That would be a trick... a remort remorting.\r\n", ch);
      return 1;
    }
    if (GET_LEVEL(ch) <= 149) {
      send_to_char("You may not remort until you have reached the "
                   "maximum mortal level (150).\r\n"
                   "You will also need 10 mil in gold and "
                   "1.9 billion total experience.\r\n", ch);
      return 1;
    }
    if (GET_GOLD(ch) <= 9999999) {
      send_to_char("You do not have enough gold. You need "
                   "10 mil in order to remort.\r\n"
                   "You will also need 1.9 billion total experience.\r\n", ch);
      return 1;
    }
    if (GET_EXP(ch) <= 1900000000) {
      send_to_char("You do not have enough experience. You need "
                   "2 billion experience in order to remort.\r\n"
                   "You will also need 10 million gold.\r\n", ch);
      return 1;
    }   
    one_argument(argument, arg);


    if (GET_CLASS(ch) == CLASS_THIEF) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "Assassin (combine with dark knight)\r\n"
                      "Shadowmage (combine with sorceror)\r\n"
                      "Acrobat (combine with monk)\r\n"
                      "Mercenary (combine with Gladiator)\r\n"
                      "Rogue (combine with Cleric)\r\n"
                      "Ranger (combine with druid)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "assassin")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ASSASSIN;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d); 
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "mercenary")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_MERCENARY;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME
(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "rogue")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ROGUE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }
        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 

      } else if (is_abbrev(arg, "shadowmage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SHADOWMAGE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "acrobat")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ACROBAT;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "ranger")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_RANGER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }   
        return 1;
        } else
        send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                     " Assassin\r\n"
                     " Shadowmage\r\n"
                     " Rogue\r\n"
                     " Acrobat\r\n"
                     " Mercenary\r\n"
                     " Ranger\r\n", ch);
        return 1;
      }



    if (GET_CLASS(ch) == CLASS_GLADIATOR) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccReaper /c0(combine with dark knight)\r\n"
                      "/ccMercenary /c0(combine with thief)\r\n"
                      "/ccWarlock /c0(combine with sorceror)\r\n"
                      "/ccCombat Master /c0(combine with monk)\r\n"
                      "/ccBeastmaster /c0(combine with druid)\r\n"
                      "/ccChampion /c0(combine with paladin)\r\n"
                      "/ccCrusader /c0(combine with cleric)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "reaper")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_REAPER; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "mercenary")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_MERCENARY;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME
(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "warlock")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_WARLOCK;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "combat master")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_COMBAT_MASTER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "beastmaster")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_BEASTMASTER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "champion")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_CHAMPION;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "crusader")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_CRUSADER;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Warlock\r\n"
                   " Champion\r\n"
                   " Reaper\r\n"
                   " Crusader\r\n"
                   " Beastmaster\r\n"
                   " Combat Master\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_SORCERER) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccDarkmage /c0(combine with dark knight)\r\n"
                      "/ccWarlock /c0(combine with gladiator)\r\n"
                      "/ccShadowmage /c0(combine with thief)\r\n"
                      "/ccPsionist /c0(combine with monk)\r\n"
                      "/ccForestal /c0(combine with druid)\r\n"
                      "/ccMagi /c0(combine with paladin)\r\n"
                      "/ccArcanic /c0(combine with cleric)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "darkmage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_DARKMAGE; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "warlock")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_WARLOCK;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "shadowmage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SHADOWMAGE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "psionist")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_PSIONIST;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "forestal")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_FORESTAL;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "magi")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_MAGI;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "arcanic")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ARCANIC;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Shadowmage\r\n"
                   " Warlock\r\n"
                   " Arcanic\r\n"
                   " Magi\r\n"
                   " Darkmage\r\n"
                   " Forestal\r\n"
                   " Psionist\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_CLERIC) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccTemplar /c0(combine with paladin)\r\n"
                      "/ccCrusader /c0(combine with gladiator)\r\n"
                      "/ccArcanic /c0(combine with sorceror)\r\n"
                      "/ccStorr /c0(combine with monk)\r\n"
                      "/ccRogue /c0(combine with thief)\r\n"
                      "/ccInquisitor /c0(combine with Dark Knight)\r\n"
                      "/ccSage /c0(combine with cleric)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "sage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SAGE; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "inquisitor")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_INQUISITOR;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "rogue")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ROGUE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "storr")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_STORR;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "crusader")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_CRUSADER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "templar")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_TEMPLAR;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "arcanic")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ARCANIC;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Templar\r\n"
                   " Crusader\r\n"
                   " Arcanic\r\n"
                   " Storr\r\n"
                   " Rogue\r\n"
                   " Inquisitor\r\n"
                   " Sage\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_MONK) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccSamurai /c0(combine with paladin)\r\n"
                      "/ccCombat Master /c0(combine with gladiator)\r\n"
                      "/ccPsionist /c0(combine with sorceror)\r\n"
                      "/ccAcrobat /c0(combine with thief)\r\n"
                      "/ccShaman /c0(combine with druid)\r\n"
                      "/ccDestroyer /c0(combine with dark knight)\r\n"
                      "/ccStorr /c0(combine with cleric)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "samurai")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SAMURAI; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "combat master")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_COMBAT_MASTER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "destroyer")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_DESTROYER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "psionist")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_PSIONIST;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "acrobat")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ACROBAT;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "shaman")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SHAMAN;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "storr")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_STORR;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Acrobat\r\n"
                   " Psionist\r\n"
                   " Combat Master\r\n"
                   " Storr\r\n"
                   " Samurai\r\n"
                   " Destroyer\r\n"
                   " Shaman\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_DRUID) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccFianna /c0(combine with paladin)\r\n"
                      "/ccBeastmaster /c0(combine with gladiator)\r\n"
                      "/ccForestal /c0(combine with sorceror)\r\n"
                      "/ccRanger /c0(combine with thief)\r\n"
                      "/ccShaman /c0(combine with monk)\r\n"
                      "/ccStyxian /c0(combine with dark knight)\r\n"
                      "/ccSage /c0(combine with cleric)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "ranger")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_RANGER; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "styxian")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_STYXIAN;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "forestal")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_FORESTAL;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "beastmaster")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_BEASTMASTER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "sage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SAGE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "shaman")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SHAMAN;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "fianna")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_FIANNA;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Ranger\r\n"
                   " Forestal\r\n"
                   " Beastmaster\r\n"
                   " Sage\r\n"
                   " Styxian\r\n"
                   " Shaman\r\n"
                   " Fianna\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_PALADIN) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccFianna /c0(combine with druid)\r\n"
                      "/ccSamurai /c0(combine with monk)\r\n"
                      "/ccTemplar /c0(combine with cleric)\r\n"
                      "/ccMagi /c0(combine with sorceror)\r\n"
                      "/ccChampion /c0(combine with gladiator)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "fianna")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_FIANNA; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "samurai")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_SAMURAI;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "templar")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_TEMPLAR;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "magi")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_MAGI;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "champion")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_CHAMPION;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Fianna\r\n"
                   " Samurai\r\n"
                   " Templar\r\n"
                   " Magi\r\n"
                   " Champion\r\n", ch);
      return 1;
    }
    if (GET_CLASS(ch) == CLASS_DARK_KNIGHT) {
      if (!*arg) {
         send_to_char("Usage: /ccremort <classname>/c0\r\n"
                      "Classes available are as follows:\r\n\r\n"
                      "/ccAssassin /c0(combine with thief)\r\n"
                      "/ccDarkmage /c0(combine with sorceror)\r\n"
                      "/ccInquisitor /c0(combine with dark knight)\r\n"
                      "/ccStyxian /c0(combine with druid)\r\n"
                      "/ccDestroyer /c0(combine with monk)\r\n"
                      "/ccReaper /c0(combine with gladiator)\r\n", ch);
        return 1;
      }
      if (is_abbrev(arg, "assassin")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_ASSASSIN; 
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "styxian")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_STYXIAN;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "destroyer")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_DESTROYER;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else if (is_abbrev(arg, "inquisitor")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_INQUISITOR;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        }

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "darkmage")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_DARKMAGE;
        GET_LEVEL(ch) = 50;
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1; 
      } else if (is_abbrev(arg, "reaper")) {
        send_to_char("The guildmaster bows before you, remortalizing you "
                     "into a new entity!\r\n", ch);
        GET_CLASS(ch) = CLASS_REAPER;
        GET_LEVEL(ch) = 50;      
        GET_EXP(ch) = 1;
        GET_GOLD(ch) = GET_GOLD(ch) - 10000000;
        for (d = 0; d < NUM_WEARS; d++) {
          if (GET_EQ(ch, d)) {
            perform_remove(ch, d);
          }
        } 

        sprintf(tmpstr, "/cC\r\nThe world stops for one brief moment to "
                        "acknowledge a new entity within.\r\n"
                        "%s has remorted. All hail./c0\r\n\r\n"
                        "/cRFor the next ten minutes experience is "
                        "doubled in honor of %s./c0\r\n", GET_NAME(ch), GET_NAME(ch));
          double_exp = 10;
          for (k = descriptor_list; k; k = k->next) {
            if (!k->connected &&
                !PRF_FLAGGED(k->character, PRF_NOINFO))
            send_to_char(tmpstr, k->character);
          }
        return 1;
      } else
      send_to_char("That is not a valid class. Classes are:\r\n\r\n"
                   " Assassin\r\n"
                   " Darkmage\r\n"
                   " Inquisitor\r\n"
                   " Reaper\r\n", ch);
      return 1;
    }
  return 0;
  }



/* END REMORT */
  return 0;
}

/* standard guilds */

SPECIAL(monk_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_MONK));}
SPECIAL(paladin_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_PALADIN));}
SPECIAL(druid_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_DRUID));}
SPECIAL(sorcerer_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_SORCERER));}
SPECIAL(cleric_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_CLERIC));}
SPECIAL(gladiator_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_GLADIATOR));}
SPECIAL(thief_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_THIEF));}
SPECIAL(dkn_guild)   {return(generic_guild(ch,me,cmd,argument,CLASS_DARK_KNIGHT));}

/* special trainers */

SPECIAL(old_sailor)
{                /* Ma  Cl  Th  Wa  Ra  DK  Mo  Pa*/
  int minlevs[] = {  1,  1,  1,  1,  1,  1,  1,  1};

  if (GET_SKILL(ch, SKILL_THROW) == 0) {
    return 0;
  }
  else {   
    return generic_skillmaster(ch, cmd, argument, SKILL_THROW, minlevs,1, GET_CLASS(ch)); 
  }
}

SPECIAL(riding_trainer)
{                /*  Ma   Cl   Th   Wa   Ra   DK   Mo   Pa*/
  int minlevs[] = {  20,  20,  20,  20,  20,  20,  20,  20};

  if (!FIGHTING(ch)) {
    return generic_skillmaster(ch, cmd, argument, SKILL_RIDE, minlevs,1, GET_CLASS(ch)); 
  }
  else { return 0; }
}

