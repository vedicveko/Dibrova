/*
**    File: quest.h
**      By: Cj Stremick
**     For: Dibrova Quests
** History: -CMS- 12/20/1998 Created
**          -CMS- 11/10/99 Ownership abondoned.  If this code is going to 
**          be subject to fucking shit-assed style, I will take no more 
**          credit or blame for what happens to it.
*/

/* Function prototypes for quest.c */
void list_quests_old(struct char_data *ch);

int  can_see_quest_obj(struct char_data *ch, struct obj_data *obj);
void open_quest(struct char_data *ch, int qn);
void begin_quest();
void close_quest();
int can_join_quest(struct char_data *ch);
void quest_purge(struct obj_data *obj_list);

/* General information structure for quests */
struct quest_definition {
  char *quest_name;  /* Name for the lists and echo's and stuff              */
  int   minlevel;    /* Minimum level to join a particular quest             */
  int   quest_flags; /* Various flags for this quest  See the QF_ constants. */
  int   quest_rating; /* Diffilculty rating for quests - determines award    */
};

/* Quest flag constants */
#define QF_SAFE       (1 << 0) /* Aggro mobs won't attack these questors.    */
#define QF_REJOIN     (1 << 1) /* Player can rejoin this quest (quit/die...) */
/* Add other QF_'s here... */

/* Quest states (synch with qstates in quest.c) */
#define QS_CLOSED     0   /* No quest running                                */
#define QS_ACCEPTING  1   /* Quest about to start - accepting players        */
#define QS_RUNNING    2   /* Quest is running                                */

/* Other macros */
#define NO_QUEST     -1   /* No quest running! */




