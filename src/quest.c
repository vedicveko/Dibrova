/*
**    File: quest.c
**      By: Cj Stremick
**     For: All Dibrova Quest-related code
** History: -CMS- 12/20/1998 Created
**          -CMS- 11/10/99 Ownership abondoned.  If this code is going to 
**          be subject to fucking shit-assed style, I will take no more 
**          credit or blame for what happens to it.
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "handler.h"
#include "hometowns.h"
#include "quest.h"

/* Global variables for quests... */
int active_quest = NO_QUEST;
int active_quest_state = QS_CLOSED;

/* External variables... */
extern struct descriptor_data *descriptor_list;
extern struct obj_data *object_list;
extern char *OK;

/* Quest definitions.  Add as many as desired but be sure to update
   the following TOP_QUEST macro.  */
const struct quest_definition quests[] = {
  {"Maze",            20, 0,         3},
  {"Recall Tag",      1,  QF_SAFE,   1},
  {"Object Hunt",     1,  QF_REJOIN, 3},
  {"Marker",          1,  0,         5},
  {"Riddle",          1,  0,         5},
  {"Portal Jump",     1,  0,         5},
  {"Keyword",         1,  0,         4},
  {"Trivia Quest",    1,  0,         3},
};
#define TOP_QUEST 7
 

/* Printable descriptions for the quest states */
const char *qstates[] = {
  "closed",
  "accepting players",
  "running"
};


const char *qflags[] = {
  "SAFE ",
  "REJOIN "
};

ACMD(do_qadmin)
{
  char param[MAX_INPUT_LENGTH];
  int qn, num_pots, obj_num, random_obj;
  struct descriptor_data *d;
  struct obj_data *potion;
  int num_questing=0;

  half_chop(argument, param, argument);
  if (isname(param, "list")) {
    list_quests_old(ch);

  } else if (isname(param, "open")) {
    one_argument(argument, param);
    if (*param) {
      qn = atoi(param);
      if (qn < 0 || qn > TOP_QUEST || !isdigit(*param))
        send_to_char("That quest number doesn't exist.\r\n", ch);
      else {
        open_quest(ch, qn);
      }
    } else {
      send_to_char("Missing argument - expecting 'OPEN #'.\r\n", ch);
    }
  } else if (isname(param, "begin")) {
    if (active_quest_state == QS_ACCEPTING) {

      for (d=descriptor_list; d; d = d->next) {
        if (!d->connected && d->character && d->character != ch &&
            PRF_FLAGGED(d->character, PRF_QUEST))
          num_questing++;
      }
      if (num_questing <= 4) {
        send_to_char("You must have a minimum of five entries to run a quest.\r\n", ch);
        return;
      }
      qn = quests[active_quest].quest_rating;
      if (qn == 1) {
        for (num_pots = 0; num_pots<5; num_pots++) {
          random_obj = number(26142, 26143);
          obj_num = real_object(random_obj);
          potion = read_object(obj_num, REAL);
          obj_to_char(potion, ch); 
        }
      }
      if (qn == 5) {
        random_obj = number(25075, 25089);
        obj_num = real_object(random_obj);
        potion = read_object(obj_num, REAL);
        obj_to_char(potion, ch);
      } 
      begin_quest();
    } else {
      send_to_char("A quest must be opened and accepting players..\r\n", ch);
      return;
    }
  } else if (isname(param, "close")) {
    close_quest();
  } else {
    send_to_char("Usage:  QADMIN (list | open # | begin | close)\r\n", ch);
  }
  return;
}

ACMD(do_qpmanage)
{
  struct char_data *victim;
  char tmp[128], tmp2[128], buf[512];
  int points;

  two_arguments(argument, tmp, tmp2);
  if (!*tmp) {
    send_to_char("Change quest points for whom?\r\n", ch);
    return;
  } else if (!*tmp2 || !(points=atoi(tmp2))) {
    send_to_char("How many quest points?\r\n", ch);
    return;
  }
  if (!(victim = get_char_vis(ch, tmp))) {
    send_to_char("You can't find that player.\r\n", ch);
    return;
  } 
  if (subcmd == SCMD_ADD)
    GET_QPOINTS(victim) += points;
  else if (subcmd == SCMD_REMOVE) {
    if (GET_QPOINTS(victim) < points) {
      sprintf(buf, "%s doesn't have that many quest points to give!\r\n",
              GET_NAME(victim));
      send_to_char(buf, ch);
      return;
    } else
      GET_QPOINTS(victim) -= points;
  }
  sprintf(buf, "(GC) %s has given %s %d QPOINTS.", GET_NAME(ch), 
          GET_NAME(victim), points);
  log(buf);
  sprintf(buf, "%s now has %d quest points.\r\n",
             GET_NAME(victim), GET_QPOINTS(victim));
  send_to_char(buf, ch);
}

/* Special transport for questors - bring all questing players to you. */
ACMD(do_qtransport) 
{
  struct descriptor_data *d;
  struct char_data *victim;

  for (d=descriptor_list; d; d = d->next)
    if (!d->connected && d->character && d->character != ch &&
        PRF_FLAGGED(d->character, PRF_QUEST)) { 
      victim = d->character;
      act("$n disappears in a /cGgreen/c0 cloud!", 
           FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of /cGgreen/c0 smoke.", 
          FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      look_at_room(victim, 0);
    }
  send_to_char(OK, ch);
}

/* Special teleport for questors -sends all questing players to a room. */
ACMD(do_qteleport) {
  struct descriptor_data *d;
  char msg[128], arg1[MAX_INPUT_LENGTH];
  int target;

  one_argument(argument, arg1);
  if (!*arg1) {
    send_to_char("Teleport the questers to where?\r\n", ch);
    return;
  } else if ((target = real_room(atoi(arg1))) == NOWHERE) {
    send_to_char("That room doesn't exist.\r\n", ch);
    return;
  }

  sprintf(msg, "/cG%s teleports you./c0\r\n", GET_NAME(ch));
  for (d=descriptor_list; d; d = d->next)
    if (d->connected) { continue; }
    if (d->character != ch && PRF_FLAGGED(d->character, PRF_QUEST)) {
      act("$n dissapers in a /cGgreen/c0 cloud!", FALSE, d->character, 0, 0, TO_ROOM);
      char_from_room(d->character);
      char_to_room(d->character, target);
      send_to_char(msg, d->character);
      look_at_room(d->character, 0);
      act("$n appears in a puff of /cGgreen/c0 smoke!", FALSE, d->character, 0, 0, TO_ROOM);
    }
  send_to_char(OK, ch);
}

/* List all the available quests */
void list_quests_old(struct char_data *ch) {
  int qn, i;
  char tmp[128];
  char qf[128];

  send_to_char("Defined quests:\r\n", ch);
  send_to_char(" #  Quest Name                  Lvl  Rating  Flags  \r\n", ch);
  send_to_char("----------------------------------------------------\r\n", ch);
  for (qn = 0; qn <= TOP_QUEST; qn++) {
    *qf='\0';
    for (i = 0; i < 2; i++)
      if (IS_SET(quests[qn].quest_flags, (1 << i)))
        strcat(qf, qflags[i]);
    sprintf(tmp, "%2d  %-24s   %3d   %4d    %s \r\n", qn,
            quests[qn].quest_name, quests[qn].minlevel,
            quests[qn].quest_rating, qf);
    send_to_char(tmp, ch);
  }
  return;
}

/* only questing players should be able to see these */
int can_see_quest_obj(struct char_data *ch, struct obj_data *obj) {
  if (!IS_OBJ_STAT(obj, ITEM_QUEST) || GET_LEVEL(ch) >= LVL_IMMORT)
    return TRUE;
  else if (PRF_FLAGGED(ch, PRF_QUEST))
    return TRUE;
  else
    return FALSE;
}

/* ch is opening quest # qn */
void open_quest(struct char_data *ch, int qn) {
  char buf[1024];
  if (active_quest != NO_QUEST) {
    send_to_char("A quest is already running.\r\n", ch);
    return;
  }
  sprintf(buf, "/cG-[QUEST]----------------------------------------\r\n"
               " %s has opened the %s quest.\r\n"
               " Type QUEST to join.\r\n"
               "------------------------------------------------/c0\r\n",
          GET_NAME(ch), quests[qn].quest_name);
  send_to_all(buf);
  active_quest = qn;
  active_quest_state = QS_ACCEPTING;
  SET_BIT_AR(PRF_FLAGS(ch), PRF_QUEST);
}

void begin_quest() {
  char buf[128];

  sprintf(buf, "/cGThe %s quest has begun./c0\r\n",
          quests[active_quest].quest_name);
  if (!(quests[active_quest].quest_flags & QF_REJOIN))
    strcat(buf, "/cGNo more entries./c0\r\n");
  send_to_all(buf);
  active_quest_state = QS_RUNNING;
}

void close_quest() {
  struct descriptor_data *d;
  char buf[128];

//  quest_purge(object_list);
  if (active_quest == NO_QUEST) {
    return; /* Nothing to close */
  }
  for (d=descriptor_list; d; d = d->next)
    if (d->character && PRF_FLAGGED(d->character, PRF_QUEST)) {
      char_from_room(d->character);
      char_to_room(d->character, real_room(1223));
      look_at_room(d->character, 0);
      REMOVE_BIT_AR(PRF_FLAGS(d->character), PRF_QUEST);
    }
  sprintf(buf, "/cGThe %s quest has been closed./c0\r\n",
          quests[active_quest].quest_name);
  send_to_all(buf);
  active_quest = NO_QUEST;
  active_quest_state = QS_CLOSED;
}

/* Checks if the player can join the current quest */
int can_join_quest(struct char_data *ch) {

  if (active_quest == NO_QUEST)
    return FALSE;

  if (GET_LEVEL(ch) < quests[active_quest].minlevel)
    return FALSE;

  if (active_quest_state==QS_CLOSED ||
      (active_quest_state==QS_RUNNING &&
       !IS_SET(quests[active_quest].quest_flags, QF_REJOIN)))
    return FALSE;

  return TRUE; /* Everything looks okay - you can join now. */
}

/* Extract all the quest objects from the given object list */
void quest_purge(struct obj_data *obj_list) {

  if (!obj_list)
    return;

/*  for (obj = obj_list; obj; obj = obj->next) {
    if (IS_OBJ_STAT(obj, ITEM_QUEST)) {
      if (obj->carried_by)
        obj_from_char(obj);
      if (obj->in_obj)
        obj_from_obj(obj);
      extract_obj(obj);
    }
  }*/

}

