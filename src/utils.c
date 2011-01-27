/*************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "clan.h"
#include "olc.h"

extern struct time_data time_info;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern int top_of_zone_table;


int can_bash(struct char_data *ch, struct char_data *victim)
{
	if (ch == NULL || victim == NULL)
		return (-1);
	else {
		if (AFF_FLAGGED(victim, AFF_BALANCE))
			return FALSE;
		else if (!IS_NPC(victim))
		{
			if (GET_CLASS(victim) == CLASS_TITAN)
			{
				if (GET_CLASS(ch) == CLASS_TITAN)
					return TRUE;
				else
					return FALSE;
			}
		}
		else if (IS_NPC(victim))
		{
			if (MOB_FLAGGED(victim, MOB_NOBASH))
				return FALSE;
			else if (GET_RACE(victim) == RACE_NPC_GIANT)
			{
				if ((GET_CLASS(ch) == CLASS_TITAN) ||
					(GET_RACE(ch) == RACE_GIANT) ||
					(GET_RACE(ch) == RACE_DRIDER) ||
					(GET_RACE(ch) == RACE_CENTAUR) ||
					(GET_RACE(ch) == RACE_GARGOYLE))
					return TRUE;
				else
					return FALSE;
			}
			else
				return TRUE;
		}
		else
			return TRUE;
	}
	return TRUE;
}

int is_class(struct char_data *ch, int chclass)
{
	if (ch == NULL)
		return FALSE;
	else if (chclass < 0)
		return FALSE;
	else {
		switch (chclass)
		{
		case CLASS_SORCERER:
			switch (GET_CLASS(ch))
			{
			case CLASS_SORCERER:
			case CLASS_SHADOWMAGE:
			case CLASS_WARLOCK:
			case CLASS_ARCANIC:
			case CLASS_MAGI:
			case CLASS_DARKMAGE:
			case CLASS_FORESTAL:
			case CLASS_PSIONIST:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_CLERIC:
			switch (GET_CLASS(ch))
			{
			case CLASS_CLERIC:
			case CLASS_CRUSADER:
			case CLASS_ARCANIC:
			case CLASS_TEMPLAR:
			case CLASS_ROGUE:
			case CLASS_STORR:
			case CLASS_INQUISITOR:
			case CLASS_SAGE:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_THIEF:
			switch (GET_CLASS(ch))
			{
			case CLASS_THIEF:
			case CLASS_ASSASSIN:
			case CLASS_SHADOWMAGE:
			case CLASS_ACROBAT:
			case CLASS_MERCENARY:
			case CLASS_RANGER:
			case CLASS_ROGUE:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_GLADIATOR:
			switch (GET_CLASS(ch))
			{
			case CLASS_GLADIATOR:
			case CLASS_MERCENARY:
			case CLASS_WARLOCK:
			case CLASS_CHAMPION:
			case CLASS_REAPER:
			case CLASS_CRUSADER:
			case CLASS_BEASTMASTER:
			case CLASS_COMBAT_MASTER:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_DRUID:
			switch (GET_CLASS(ch))
			{
			case CLASS_DRUID:
			case CLASS_RANGER:
			case CLASS_BEASTMASTER:
			case CLASS_FORESTAL:
			case CLASS_SAGE:
			case CLASS_FIANNA:
			case CLASS_STYXIAN:
			case CLASS_SHAMAN:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_DARK_KNIGHT:
			switch (GET_CLASS(ch))
			{
			case CLASS_DARK_KNIGHT:
			case CLASS_ASSASSIN:
			case CLASS_REAPER:
			case CLASS_DARKMAGE:
			case CLASS_INQUISITOR:
			case CLASS_STYXIAN:
			case CLASS_DESTROYER:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_MONK:
			switch (GET_CLASS(ch))
			{
			case CLASS_MONK:
			case CLASS_ACROBAT:
			case CLASS_STORR:
			case CLASS_PSIONIST:
			case CLASS_SHAMAN:
			case CLASS_DESTROYER:
			case CLASS_COMBAT_MASTER:
			case CLASS_SAMURAI:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_PALADIN:
			switch (GET_CLASS(ch))
			{
			case CLASS_PALADIN:
			case CLASS_CHAMPION:
			case CLASS_MAGI:
			case CLASS_TEMPLAR:
			case CLASS_FIANNA:
			case CLASS_SAMURAI:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		case CLASS_BARD:
			switch (GET_CLASS(ch))
			{
			case CLASS_BARD:
				return TRUE;
				break;
			default:
				return FALSE;
			}
			break;
		default:
			return FALSE;
		}
		return FALSE;
	}
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
  /* error checking in case people call number() incorrectly */
  if (from > to) {
    int tmp = from;
    from = to;
    to = tmp;
  }

  return ((circle_random() % (to - from + 1)) + from);
}


/* simulates dice roll */
int dice(int number, int size)
{
  int sum = 0;

  if (size <= 0 || number <= 0)
    return 0;

  while (number-- > 0)
    sum += ((circle_random() % size) + 1);

  return sum;
}


int MIN(int a, int b)
{
  return a < b ? a : b;
}


int MAX(int a, int b)
{
  return a > b ? a : b;
}



/* Create a duplicate of a string */
char *str_dup(const char *source)
{
  char *new;

  CREATE(new, char, strlen(source) + 1);
  return (strcpy(new, source));
}



/* str_cmp: a case-insensitive version of strcmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
  int chk, i;

  for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }
  return (0);
}


/* strn_cmp: a case-insensitive version of strncmp */
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
  int chk, i;

  for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--)
    if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
      if (chk < 0)
	return (-1);
      else
	return (1);
    }
  return (0);
}


/* log a death trap hit */
void log_death_trap(struct char_data * ch)
{
  char buf[150];
  extern struct room_data *world;

  sprintf(buf, "%s hit death trap #%d (%s)", GET_NAME(ch),
	  world[ch->in_room].number, world[ch->in_room].name);
  mudlog(buf, BRF, LVL_IMMORT, TRUE);
}

/*
 * New variable argument log() function.  Works the same as the old for
 * previously written code but is very nice for new code.
 */
void basic_mud_log(const char *format, ...)
{
  va_list args;
  time_t ct = time(0);
  char *time_s = asctime(localtime(&ct));

//  if (logfile == NULL)
//    puts("SYSERR: Using log() before stream was initialized!");
  if (format == NULL)
    format = "SYSERR: log() received a NULL format.";

  time_s[strlen(time_s) - 1] = '\0';

  fprintf(stderr, "%-15.15s :: ", time_s + 4);

  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
  fflush(stderr);
}


/* the "touch" command, essentially. */
int touch(char *path)
{
  FILE *fl;

  if (!(fl = fopen(path, "a"))) {
    perror(path);
    return -1;
  } else {
    fclose(fl);
    return 0;
  }
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(char *str, char type, int level, byte file)
{
  char buf[MAX_STRING_LENGTH];
  struct descriptor_data *i;
  char *tmp, tp;
  time_t ct;

  ct = time(0);
  tmp = asctime(localtime(&ct));

  if (file)
    fprintf(stderr, "%-19.19s :: %s\n", tmp, str);
  if (level < 0)
    return;

  sprintf(buf, "[ %s ]\r\n", str);

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected && !PLR_FLAGGED(i->character, PLR_WRITING)) {
      tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
	    (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));

      if ((GET_LEVEL(i->character) >= level) && (tp >= type)) {
	send_to_char(CCGRN(i->character, C_NRM), i->character);
	send_to_char(buf, i->character);
	send_to_char(CCNRM(i->character, C_NRM), i->character);
      }
    }
}


void sprintbitarray(int bitvector[], char *names[], int maxar, char *result)
{
  int nr, teller, found = FALSE;

  *result = '\0';

  for(teller = 0; teller < maxar && !found; teller++)
    for (nr = 0; nr < 32 && !found; nr++) {
      if (IS_SET_AR(bitvector, (teller*32)+nr)) {
        if (*names[(teller*32)+nr] != '\n') {
          if (*names[(teller*32)+nr] != '\0') {
	    strcat(result, names[(teller*32)+nr]);
	    strcat(result, " ");
          }
        } else {
	  strcat(result, "UNDEFINED ");
        }
        }
      if (*names[(teller*32)+nr] == '\n')
        found = TRUE;
    }

  if (!*result)
    strcpy(result, "NOBITS ");
}


void sprintbit(long bitvector, char *names[], char *result)
{
  long nr;

  *result = '\0';

  if (bitvector < 0) {
    strcpy(result, "<INVALID BITVECTOR>");
    return;
  }

  for (nr = 0; bitvector; bitvector >>= 1) {
    if (IS_SET(bitvector, 1)) {
      if (*names[nr] != '\n') {
	strcat(result, names[nr]);
	strcat(result, " ");
      } else
	strcat(result, "UNDEFINED ");
    }
    if (*names[nr] != '\n')
      nr++;
  }

  if (!*result)
    strcpy(result, "NOBITS ");
}



void sprinttype(int type, char *names[], char *result)
{
  int nr = 0;

  while (type && *names[nr] != '\n') {
    type--;
    nr++;
  }

  if (*names[nr] != '\n')
    strcpy(result, names[nr]);
  else
    strcpy(result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_REAL_HOUR * now.hours;

  now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
  secs -= SECS_PER_REAL_DAY * now.day;

  now.month = -1;
  now.year = -1;

  return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
  long secs;
  struct time_info_data now;

  secs = (long) (t2 - t1);

  now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
  secs -= SECS_PER_MUD_HOUR * now.hours;

  now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
  secs -= SECS_PER_MUD_DAY * now.day;

  now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
  secs -= SECS_PER_MUD_MONTH * now.month;

  now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

  return now;
}



struct time_info_data age(struct char_data * ch)
{
  struct time_info_data player_age;

  player_age = mud_time_passed(time(0), ch->player.time.birth);

  player_age.year += 17;	/* All players start at 17 */

  return player_age;
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
  struct char_data *k;

  for (k = victim; k; k = k->master) {
    if (k == ch)
      return TRUE;
  }

  return FALSE;
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  assert(ch->master);

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM, ABT_SPELL))
      affect_from_char(ch, SPELL_CHARM, ABT_SPELL);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", TRUE, ch, 0, ch->master, TO_VICT);
  }

  if (ch->master->followers->follower == ch) {	/* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
  } else {			/* locate follower who is not head of list */
    for (k = ch->master->followers; k->next->follower != ch; k = k->next);

    j = k->next;
    k->next = j->next;
    free(j);
  }

  ch->master = NULL;
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_CHARM);
  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data * ch)
{
  struct follow_type *j, *k;

  if (ch->master)
    stop_follower(ch);

  for (k = ch->followers; k; k = j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data * ch, struct char_data * leader)
{
  struct follow_type *k;

  assert(!ch->master);

  ch->master = leader;

  CREATE(k, struct follow_type, 1);

  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;

  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  if (CAN_SEE(leader, ch))
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n starts to follow $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
  char temp[256];
  int lines = 0;

  do {
    lines++;
    fgets(temp, 256, fl);
    if (*temp)
      temp[strlen(temp) - 1] = '\0';
  } while (!feof(fl) && (*temp == '*' || !*temp));

  if (feof(fl))
    return 0;
  else {
    strcpy(buf, temp);
    return lines;
  }
}


int get_filename(char *orig_name, char *filename, int mode)
{
  char *prefix, *middle, *suffix, *ptr, name[64];

  switch (mode) {
  case CRASH_FILE:
    prefix = "plrobjs";
    suffix = "objs";
    break;
  case ETEXT_FILE:
    prefix = "plrtext";
    suffix = "text";
    break;
  case SCRIPT_VARS_FILE:
    prefix = LIB_PLRVARS;
    suffix = SUF_MEM;
    break;
  default:
    return 0;
    break;
  }

  if (!*orig_name)
    return 0;

  strcpy(name, orig_name);
  for (ptr = name; *ptr; ptr++)
    *ptr = LOWER(*ptr);

  switch (LOWER(*name)) {
  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':
    middle = "A-E";
    break;
  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':
    middle = "F-J";
    break;
  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':
    middle = "K-O";
    break;
  case 'p':  case 'q':  case 'r':  case 's':  case 't':
    middle = "P-T";
    break;
  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    middle = "U-Z";
    break;
  default:
    middle = "ZZZ";
    break;
  }

  sprintf(filename, "%s/%s/%s.%s", prefix, middle, name, suffix);
  return 1;
}


int num_pc_in_room(struct room_data *room)
{
  int i = 0;
  struct char_data *ch;

  for (ch = room->people; ch != NULL; ch = ch->next_in_room)
    if (!IS_NPC(ch))
      i++;

  return i;
}


/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int replace_str(char **string, char *pattern, char *replacement, int rep_all,
		int max_size) {
   char *replace_buffer = NULL;
   char *flow, *jetsam, temp;
   int len, i;
   
   if ((strlen(*string) - strlen(pattern)) + strlen(replacement) > max_size)
     return -1;
   
   CREATE(replace_buffer, char, max_size);
   i = 0;
   jetsam = *string;
   flow = *string;
   *replace_buffer = '\0';
   if (rep_all) {
      while ((flow = (char *)strstr(flow, pattern)) != NULL) {
	 i++;
	 temp = *flow;
	 *flow = '\0';
	 if ((strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
	    i = -1;
	    break;
	 }
	 strcat(replace_buffer, jetsam);
	 strcat(replace_buffer, replacement);
	 *flow = temp;
	 flow += strlen(pattern);
	 jetsam = flow;
      }
      strcat(replace_buffer, jetsam);
   }
   else {
      if ((flow = (char *)strstr(*string, pattern)) != NULL) {
	 i++;
	 flow += strlen(pattern);  
	 len = ((char *)flow - (char *)*string) - strlen(pattern);
   
	 strncpy(replace_buffer, *string, len);
	 strcat(replace_buffer, replacement);
	 strcat(replace_buffer, flow);
      }
   }
   if (i == 0) return 0;
   if (i > 0) {
      RECREATE(*string, char, strlen(replace_buffer) + 3);
      strcpy(*string, replace_buffer);
   }
   free(replace_buffer);
   return i;
}


/* re-formats message type formatted char * */
/* (for strings edited with d->str) (mostly olc and mail)     */
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen) {
   int total_chars, cap_next = TRUE, cap_next_next = FALSE;
   char *flow, *start = NULL, temp;
   /* warning: do not edit messages with max_str's of over this value */
   char formated[MAX_STRING_LENGTH];
   
   flow   = *ptr_string;
   if (!flow) return;

   if (IS_SET(mode, FORMAT_INDENT)) {
      strcpy(formated, "   ");
      total_chars = 3;
   }
   else {
      *formated = '\0';
      total_chars = 0;
   } 

   while (*flow != '\0') {
      while ((*flow == '\n') ||
	     (*flow == '\r') ||
	     (*flow == '\f') ||
	     (*flow == '\t') ||
	     (*flow == '\v') ||
	     (*flow == ' ')) flow++;

      if (*flow != '\0') {

	 start = flow++;
	 while ((*flow != '\0') &&
		(*flow != '\n') &&
		(*flow != '\r') &&
		(*flow != '\f') &&
		(*flow != '\t') &&
		(*flow != '\v') &&
		(*flow != ' ') &&
		(*flow != '.') &&
		(*flow != '?') &&
		(*flow != '!')) flow++;

	 if (cap_next_next) {
	    cap_next_next = FALSE;
	    cap_next = TRUE;
	 }

	 /* this is so that if we stopped on a sentance .. we move off the sentance delim. */
	 while ((*flow == '.') || (*flow == '!') || (*flow == '?')) {
	    cap_next_next = TRUE;
	    flow++;
	 }
	 
	 temp = *flow;
	 *flow = '\0';

	 if ((total_chars + strlen(start) + 1) > 70) {
	    strcat(formated, "\r\n");
	    total_chars = 0;
	 }

	 if (!cap_next) {
	    if (total_chars > 0) {
	       strcat(formated, " ");
	       total_chars++;
	    }
	 }
	 else {
	    cap_next = FALSE;
	    *start = UPPER(*start);
	 }

	 total_chars += strlen(start);
	 strcat(formated, start);

	 *flow = temp;
      }

      if (cap_next_next) {
	 if ((total_chars + 3) > 70) {
	    strcat(formated, "\r\n");
	    total_chars = 0;
	 }
	 else {
	    strcat(formated, "  ");
	    total_chars += 2;
	 }
      }
   }
   strcat(formated, "\r\n");

   if (strlen(formated) > maxlen) formated[maxlen] = '\0';
   RECREATE(*ptr_string, char, MIN(maxlen, strlen(formated)+3));
   strcpy(*ptr_string, formated);
}
  
char *lcase(char *str)
{
  char *p;
  
  for (p=str; *p; p++){
    *p=tolower(*p);
  }
  return str;
}

char *ucase(char *str)
{
  char *p;
  
  for (p=str; *p; p++){
    *p=toupper(*p);
  }
  return str;
}

/* Storm 7/28/98 -- warn of a multiplayer... */
int multi_check(struct char_data *ch) {

  struct char_data *tch;
  struct descriptor_data *d;
  char buf[1024], temp[1024];
  int found=0;

  sprintf(buf, "%sMultiplay:: %s has logged in on host %s. \r\n"
               "Multiplay:: Previous players:", 
              CCCYN(ch, C_NRM), GET_NAME(ch), ch->desc->host);
  for (d = descriptor_list; d; d = d->next) {
    if (!d->connected) {
      if (d->original)
        tch = d->original;
      else if (!(tch = d->character))
        continue;
      if (d->host && *d->host)
        if (!strcmp(d->host, ch->desc->host)) {
          if (d->original){
            if(!PRF_FLAGGED(d->original, PRF_MULTIPLAY))
              TOGGLE_BIT_AR(PRF_FLAGS(d->original), PRF_MULTIPLAY);
            sprintf(temp, " %s,", d->original->player.name);}
          else{
            if(!PRF_FLAGGED(d->character, PRF_MULTIPLAY))
              TOGGLE_BIT_AR(PRF_FLAGS(d->character), PRF_MULTIPLAY);
            sprintf(temp, " %s,", d->character->player.name);}
          strcat(buf, temp);
          found++;
          if (!(found % 5)) {
            sprintf(temp, "\r\nMultiplay:: Previous players (cont):");
            strcat(buf, temp);
          }
        }
    }
  }
  if (found) {
    sprintf(temp, "\r\nMultiplay:: total players from that host: %d%s\r\n",
            found, CCNRM(ch, C_NRM));
    strcat(buf, temp + 1);
    send_to_level(buf, LVL_ADMIN);
    TOGGLE_BIT_AR(PRF_FLAGS(ch), PRF_MULTIPLAY);
    return 1;
  }
  return 0;
}

int adjust_damage(struct char_data *ch, int damage) {

  float temp_damage = damage>>1;

  if (!GET_EQ(ch, WEAR_WIELD)) {
    if (GET_CLASS(ch)==CLASS_MONK ||
        GET_CLASS(ch)==CLASS_ACROBAT ||
        GET_CLASS(ch)==CLASS_COMBAT_MASTER ||
        GET_CLASS(ch)==CLASS_PSIONIST ||
        GET_CLASS(ch)==CLASS_STORR ||
        GET_CLASS(ch)==CLASS_SAGE ||
        GET_CLASS(ch)==CLASS_FIANNA)
      temp_damage *= 1.28;
    return temp_damage;
  }

  switch(GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3)-TYPE_HIT) {
    case TYPE_STING:    temp_damage *= 1.08; break;
    case TYPE_WHIP:     temp_damage *= 1.12; break;
    case TYPE_SLASH:    temp_damage *= 1.04; break;
    case TYPE_BITE:     temp_damage *= 1.10; break;
    case TYPE_BLUDGEON: temp_damage *= 1.08; break;
    case TYPE_CRUSH:    temp_damage *= 1.08; break;
    case TYPE_POUND:    temp_damage *= 1.05; break;
    case TYPE_CLAW:     temp_damage *= 1.23; break;
    case TYPE_MAUL:     temp_damage *= 1.20; break;
    case TYPE_THRASH:   temp_damage *= 1.16; break;
    case TYPE_PIERCE:   temp_damage *= 1.19; break;
    case TYPE_BLAST:    temp_damage *= 1.15; break;
    case TYPE_PUNCH:    temp_damage *= 1.03; break;
    case TYPE_STAB:     temp_damage *= 1.20; break;
    case TYPE_SPELL:    temp_damage *= 1.14; break;
    case TYPE_EMBRACE:  temp_damage *= 1.14; break;
    case TYPE_SUBMIT:   temp_damage  = 0; break;
  }
  return (int)temp_damage;
}

/*  Storm --8/23/98 */
extern int prac_params[4][NUM_CLASSES];
#define LEARNED_LEVEL   0       /* % known which is considered "learned" */
#define LEARNED(ch) (prac_params[LEARNED_LEVEL][(int)GET_CLASS(ch)])
/* A little function that will improve a skill, should be used
   when the skill fails. */
void improve_abil(struct char_data *ch, int num, byte type) {

  byte lvl;

  lvl = get_ability(ch, type, num);
  if (lvl >= LEARNED(ch))
    return;

  if (!number(0, 10)) {
    set_ability(ch, type, num, ++lvl);
    send_to_char("You feel yourself improve a little.\r\n", ch);
  }
}

struct char_data *is_playing(char *vict_name)
{
  extern struct descriptor_data *descriptor_list;
  struct descriptor_data *i, *next_i;

  for (i = descriptor_list; i; i = next_i) {
    next_i = i->next;
    if(i->connected == CON_PLAYING && !strcmp(i->character->player.name,CAP(vict_name)))
      return i->character;
  }
  return NULL;
}


int pkill_ok(struct char_data * ch, struct char_data * victim)
{ 
  if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
   return FALSE;
  }

  if (ROOM_FLAGGED(victim->in_room, ROOM_PEACEFUL)) {
   return FALSE;
  }

  if (IS_NPC(ch) || IS_NPC(victim))
    return TRUE;

  if (!victim->desc) {
    if (GET_LEVEL(ch) < LVL_IMMORT) {
      send_to_char("You are not allowed to attack linkdead players!\r\n", ch);
      return FALSE;
    } else
      return TRUE;
  }

  if (GET_PKSAFETIMER(ch) && !IS_NPC(victim)) {         
     sprintf(buf, "You are timed out from pkill for %d more %s.\r\n", GET_PKSAFETIMER(ch),
            (GET_PKSAFETIMER(ch) > 1 ? "ticks" : "tick"));
     send_to_char(buf, ch);
     return FALSE;
  }

  if (GET_PKSAFETIMER(victim) && !IS_NPC(ch)) {         
     sprintf(buf, "%s is timed out from pkilling for %d more %s.\r\n", GET_NAME(victim), 
            GET_PKSAFETIMER(victim), (GET_PKSAFETIMER(victim) > 1 ? "ticks" : "tick"));
     send_to_char(buf, ch);
     return FALSE;
  }

  if (!strcmp(ch->desc->host, victim->desc->host)) {
    send_to_char("You cannot kill anyone with your same ip.\r\n",ch);
    return FALSE;
  }

  if (PLR_FLAGGED(victim, PLR_KILLER))
    return TRUE; 
 
  if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) && 
      ROOM_FLAGGED(victim->in_room, ROOM_ARENA))
    return TRUE;


  if(PLR_FLAGGED(victim, PLR_BOUNTY)) { return TRUE; }

  if (PLR_FLAGGED(victim, PLR_EXILE)) { return TRUE; }

  if(GET_CLAN(ch) && GET_CLAN(victim)) {
    if(clan[find_clan_by_id(GET_CLAN(ch))].at_war == GET_CLAN(victim) &&
       clan[find_clan_by_id(GET_CLAN(victim))].at_war == GET_CLAN(ch)) {
      return TRUE;
    }
  }


 // * clan zone pkill 
/*
    if (((zone_table[world[ch->in_room].zone].owner == clan[find_clan_by_id(GET_CLAN(ch))].alliance)) && 
       ((GET_CLAN(ch)) != (GET_CLAN(victim))) &&
       (GET_CLAN(victim)) && (GET_CLAN(ch))) {
    return TRUE;

  }
*/

/*
    if (((zone_table[world[ch->in_room].zone].owner == (GET_CLAN(ch))) || (zone_table[world[ch->in_room].zone].owner == (GET_CLAN(victim)))) && 
       ((GET_CLAN(ch)) != (GET_CLAN(victim))) &&
       (GET_CLAN(victim)) && (GET_CLAN(ch))) {
    return TRUE;
  }
*/

/* Level-based pkill removed 5/5/01
    if(GET_CLASS(ch) >= 9 && GET_LEVEL(victim) >= 100) { return TRUE; }
    if(GET_CLASS(victim) >= 9 && GET_LEVEL(ch) >= 100) { return TRUE; }
    if (GET_CLASS(victim) >= CLASS_VAMPIRE && GET_CLASS(ch) >= CLASS_VAMPIRE) {
      return TRUE;
    }

    if((GET_LEVEL(victim) >= 100) && (GET_LEVEL(ch) >=100)) {
      return TRUE;
    }
*/
  /* pkill based on PLR_PKILL_OK */
  if (PLR_FLAGGED(ch, PLR_PKILL_OK) && PLR_FLAGGED(victim, PLR_PKILL_OK)) {
    return TRUE;
  }

  else {
    return FALSE;
  }
   
}

byte get_ability(struct char_data *ch, byte type, uint num) {
  int i;

  if (IS_NPC(ch))
    return 0; /* Mobs don't have 'em */

  if (!IS_NPC(ch) && GET_LEVEL(ch) >= LVL_IMMORT)
    return 100; /* Set all god abilities to 100% */

  for (i=0; i < MAX_ABILITIES; i++) {
    if (ch->player_specials->abilities[i].type == type &&
        ch->player_specials->abilities[i].num == num) {
      return ch->player_specials->abilities[i].pct;
    }
  }
  return 0; /* not learned at all */
}

void set_ability(struct char_data *ch, byte type, uint num, byte pct) {
  int i;

  if (IS_NPC(ch))
    return;

  for (i=0; i < MAX_ABILITIES && ch->player_specials->abilities[i].pct; i++) {
    if (ch->player_specials->abilities[i].type == type &&
        ch->player_specials->abilities[i].num == num) {
      ch->player_specials->abilities[i].pct = pct;
      if (!pct) {
        ch->player_specials->abilities[i].type = 0;
        ch->player_specials->abilities[i].num = 0;
      }  
      return;
    }
  }
  /* Not known.  Add it to the end of the list */
  ch->player_specials->abilities[i].type = type;
  ch->player_specials->abilities[i].num = num;
  ch->player_specials->abilities[i].pct = pct;
  return;
}

/* Count the number of bits that are set. */
int countbits(int n) {
  int pos, ct=0;

  for (pos=0; pos < (sizeof(n)*8)-1; pos++)
    if ((1<<pos) & n)
      ct++;
  return ct;
}

/*
void write_event(char *event)
{

  FILE *fp;
  FILE *tips;

  if (!(tips=fopen(EVENT_FILE, "r"))){
    printf("Can't open tip file '%s' for reading.\n", EVENT_FILE);
    exit(1);
  }


  fclose(tips);

  if ((fp = fopen("/home/dibrova/public_html/events.txt", "w")) == 0) {
    perror("Failed to open events.txt for writing.");
    return;
  }

  fprintf(fp, event);
  fclose(fp);
}
*/

/* Return a graphical representation of something */
void make_bar(char *b, int now, int max, int len) {

    int i, j, bchar = 0, c = 0, fill = (now * len) / max;
    bool newcolor = 0;
    
    const char *ccode[] = {
       "/cr",           /* Color code for RED    */
       "/cy",           /* Color code for YELLOW */
       "/cg",           /* Color code for GREEN  */
       "/cc"            /* Color code for CYAN   */
    };
    
    if (now > max)
        now = max;
        
    for (i = 0; i < len; i++) {
    
       if (i >= fill && c < 3) {
          c = 3;
          newcolor = 1;
       }
       
       if (((i % (len / 3)) == 0 && (i < fill)) || newcolor) {
          for (j = 0; j < strlen(ccode[c]); j++) {
             b[bchar] = ccode[c][j];
             bchar++;
             
          }
          if (c < 2)
              c++;
          newcolor = 0;
       }
       
       if (i < fill)
          b[bchar] = '#';
       else
          b[bchar] = '=';
       bchar++;
       
    }
    b[bchar] = '\0';
}

int load_percent(int current) {

   if(current < MAX_OBJ_LOAD_PERCENT) {
     return current;
   }
   else {
     return MAX_OBJ_LOAD_PERCENT; 
   }
}

