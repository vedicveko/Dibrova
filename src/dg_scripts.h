/**************************************************************************
*  File: scripts.h                                                        *
*  Usage: header file for script structures and contstants, and           *
*         function prototypes for scripts.c                               *
*                                                                         *
*                                                                         *
*  $Author: dibrova $
*  $Date: 2000/06/29 17:34:03 $
*  $Revision: 1.2 $
**************************************************************************/

#define DG_SCRIPT_VERSION "DG Scripts Version 0.99 Patch Level 7   12/98"



#define    MOB_TRIGGER   0
#define    OBJ_TRIGGER   1
#define    WLD_TRIGGER   2

#define DG_NO_TRIG         256     /* don't check act trigger   */

/* unless you change this, Puff casts all your dg spells */
#define DG_CASTER_PROXY 1

/* mob trigger types */
#define MTRIG_GLOBAL           (1 << 0)      /* check even if zone empty   */
#define MTRIG_RANDOM           (1 << 1)      /* checked randomly           */
#define MTRIG_COMMAND          (1 << 2)	     /* character types a command  */
#define MTRIG_SPEECH           (1 << 3)	     /* a char says a word/phrase  */
#define MTRIG_ACT              (1 << 4)      /* word or phrase sent to act */
#define MTRIG_DEATH            (1 << 5)      /* character dies             */
#define MTRIG_GREET            (1 << 6)      /* something enters room seen */
#define MTRIG_GREET_ALL        (1 << 7)      /* anything enters room       */
#define MTRIG_ENTRY            (1 << 8)      /* the mob enters a room      */
#define MTRIG_RECEIVE          (1 << 9)      /* character is given obj     */
#define MTRIG_FIGHT            (1 << 10)     /* each pulse while fighting  */
#define MTRIG_HITPRCNT         (1 << 11)     /* fighting and below some hp */
#define MTRIG_BRIBE	       (1 << 12)     /* coins are given to mob     */
#define MTRIG_LOAD             (1 << 13)     /* the mob is loaded          */
#define MTRIG_MEMORY           (1 << 14)     /* mob see's someone remembered */

/* obj trigger types */
#define OTRIG_GLOBAL           (1 << 0)	     /* unused                     */
#define OTRIG_RANDOM           (1 << 1)	     /* checked randomly           */
#define OTRIG_COMMAND          (1 << 2)      /* character types a command  */

#define OTRIG_TIMER            (1 << 5)     /* item's timer expires       */
#define OTRIG_GET              (1 << 6)     /* item is picked up          */
#define OTRIG_DROP             (1 << 7)     /* character trys to drop obj */
#define OTRIG_GIVE             (1 << 8)     /* character trys to give obj */
#define OTRIG_WEAR             (1 << 9)     /* character trys to wear obj */
#define OTRIG_REMOVE           (1 << 11)    /* character trys to remove obj */

#define OTRIG_LOAD             (1 << 13)    /* the object is loaded       */

/* wld trigger types */
#define WTRIG_GLOBAL           (1 << 0)      /* check even if zone empty   */
#define WTRIG_RANDOM           (1 << 1)	     /* checked randomly           */
#define WTRIG_COMMAND          (1 << 2)	     /* character types a command  */
#define WTRIG_SPEECH           (1 << 3)      /* a char says word/phrase    */

#define WTRIG_RESET            (1 << 5)      /* zone has been reset        */
#define WTRIG_ENTER            (1 << 6)	     /* character enters room      */
#define WTRIG_DROP             (1 << 7)      /* something dropped in room  */


/* obj command trigger types */
#define OCMD_EQUIP             (1 << 0)	     /* obj must be in char's equip */
#define OCMD_INVEN             (1 << 1)	     /* obj must be in char's inven */
#define OCMD_ROOM              (1 << 2)	     /* obj must be in char's room  */

#define TRIG_NEW                0	     /* trigger starts from top  */
#define TRIG_RESTART            1	     /* trigger restarting       */


/*
 * These are slightly off of PULSE_MOBILE so
 * everything isnt happening at the same time 
 */
#define PULSE_DG_SCRIPT         (13 RL_SEC)


#define MAX_SCRIPT_DEPTH      10          /* maximum depth triggers can
					     recurse into each other */


/* one line of the trigger */
struct cmdlist_element {
  char *cmd;				/* one line of a trigger */
  struct cmdlist_element *original;
  struct cmdlist_element *next;
};

struct trig_var_data {
  char *name;				/* name of variable  */
  char *value;				/* value of variable */
  long context;				/* 0: global context */
  
  struct trig_var_data *next;
};

/* structure for triggers */
struct trig_data {
    int nr;		                /* trigger's rnum                  */
    byte attach_type;			/* mob/obj/wld intentions          */
    byte data_type;		        /* type of game_data for trig      */
    char *name;			        /* name of trigger                 */
    long trigger_type;			/* type of trigger (for bitvector) */
    struct cmdlist_element *cmdlist;	/* top of command list             */
    struct cmdlist_element *curr_state;	/* ptr to current line of trigger  */
    int narg;				/* numerical argument              */
    char *arglist;			/* argument list                   */
    int depth;				/* depth into nest ifs/whiles/etc  */
    int loops;				/* loop iteration counter          */
    struct event_info *wait_event;	/* event to pause the trigger      */
    ubyte purged;			/* trigger is set to be purged     */
    struct trig_var_data *var_list;	/* list of local vars for trigger  */
    
    struct trig_data *next;  
    struct trig_data *next_in_world;    /* next in the global trigger list */
};


/* a complete script (composed of several triggers) */
struct script_data {
  long types;				/* bitvector of trigger types */
  struct trig_data *trig_list;	/* list of triggers           */
  struct trig_var_data *global_vars;	/* list of global variables   */
  ubyte purged;				/* script is set to be purged */
  long context;				/* current context for statics */

  struct script_data *next;		/* used for purged_scripts    */
};

/* used for actor memory triggers */
struct script_memory {
  long id;				/* id of who to remember */
  char *cmd;				/* command, or NULL for generic */
  struct script_memory *next;
};


/* function prototypes from triggers.c (and others) */
void act_mtrigger(const struct char_data *ch, char *str,
	struct char_data *actor,
	struct char_data *victim, struct obj_data *object,
	struct obj_data *target, char *arg);  
void speech_mtrigger(struct char_data *actor, char *str);
void speech_wtrigger(struct char_data *actor, char *str);
void greet_memory_mtrigger(struct char_data *ch);
int greet_mtrigger(struct char_data *actor, int dir);
int entry_mtrigger(struct char_data *ch);
void entry_memory_mtrigger(struct char_data *ch);
int enter_wtrigger(struct room_data *room, struct char_data *actor, int dir);
int drop_otrigger(struct obj_data *obj, struct char_data *actor);
void timer_otrigger(struct obj_data *obj);
int get_otrigger(struct obj_data *obj, struct char_data *actor);
int drop_wtrigger(struct obj_data *obj, struct char_data *actor);
int give_otrigger(struct obj_data *obj, struct char_data *actor,
         struct char_data *victim);
int receive_mtrigger(struct char_data *ch, struct char_data *actor,
         struct obj_data *obj);
void bribe_mtrigger(struct char_data *ch, struct char_data *actor,
         int amount);
int wear_otrigger(struct obj_data *obj, struct char_data *actor, int where);
int remove_otrigger(struct obj_data *obj, struct char_data *actor);
int command_mtrigger(struct char_data *actor, char *cmd, char *argument);
int command_otrigger(struct char_data *actor, char *cmd, char *argument);
int command_wtrigger(struct char_data *actor, char *cmd, char *argument);
int death_mtrigger(struct char_data *ch, struct char_data *actor);
void fight_mtrigger(struct char_data *ch);
void hitprcnt_mtrigger(struct char_data *ch);

void random_mtrigger(struct char_data *ch);
void random_otrigger(struct obj_data *obj);
void random_wtrigger(struct room_data *ch);
void reset_wtrigger(struct room_data *ch);

void load_mtrigger(struct char_data *ch);
void load_otrigger(struct obj_data *obj);

/* function prototypes from scripts.c */
void script_trigger_check(void);
void add_trigger(struct script_data *sc, struct trig_data *t, int loc);

void do_stat_trigger(struct char_data *ch, struct trig_data *trig);
void do_sstat_room(struct char_data * ch);
void do_sstat_object(struct char_data * ch, struct obj_data * j);
void do_sstat_character(struct char_data * ch, struct char_data * k);

void script_log(char *msg);
void dg_read_trigger(FILE *fp, void *i, int type);
void dg_obj_trigger(char *line, struct obj_data *obj);
void assign_triggers(void *i, int type);
void parse_trigger(FILE *trig_f, int nr);
int real_trigger(int vnum);
void extract_script(struct script_data *sc);
void extract_script_mem(struct script_memory *sc);

struct trig_data *read_trigger(int nr);
void add_var(struct trig_var_data **var_list, char *name, char *value, long id);
struct room_data *dg_room_of_obj(struct obj_data *obj);
void do_dg_cast(void *go, struct script_data *sc, struct trig_data *trig, 
    int type, char *cmd);
void do_dg_affect(void *go, struct script_data *sc, struct trig_data *trig, 
    int type, char *cmd);




/* Macros for scripts */

#define UID_CHAR   '\x1b'

#define GET_TRIG_NAME(t)          ((t)->name)
#define GET_TRIG_RNUM(t)          ((t)->nr)
#define GET_TRIG_VNUM(t)	  (trig_index[(t)->nr]->virtual)
#define GET_TRIG_TYPE(t)          ((t)->trigger_type)
#define GET_TRIG_DATA_TYPE(t)	  ((t)->data_type)
#define GET_TRIG_NARG(t)          ((t)->narg)
#define GET_TRIG_ARG(t)           ((t)->arglist)
#define GET_TRIG_VARS(t)	  ((t)->var_list)
#define GET_TRIG_WAIT(t)	  ((t)->wait_event)
#define GET_TRIG_DEPTH(t)         ((t)->depth)
#define GET_TRIG_LOOPS(t)         ((t)->loops)

/* player id's: 0 to ROOM_ID_BASE - 1            */
/* room id's: ROOM_ID_BASE to MOBOBJ_ID_BASE - 1 */
/* mob/object id's: MOBOBJ_ID_BASE and higher    */
#define ROOM_ID_BASE    50000
#define MOBOBJ_ID_BASE	200000

#define SCRIPT(o)		  ((o)->script)
#define SCRIPT_MEM(c)             ((c)->memory)

#define SCRIPT_TYPES(s)		  ((s)->types)				  
#define TRIGGERS(s)		  ((s)->trig_list)

#define GET_SHORT(ch)    ((ch)->player.short_descr)


#define SCRIPT_CHECK(go, type)   (SCRIPT(go) && \
				  IS_SET(SCRIPT_TYPES(SCRIPT(go)), type))
#define TRIGGER_CHECK(t, type)   (IS_SET(GET_TRIG_TYPE(t), type) && \
				  !GET_TRIG_DEPTH(t))

#define ADD_UID_VAR(buf, trig, go, name, context) { \
		         sprintf(buf, "%c%ld", UID_CHAR, GET_ID(go)); \
                         add_var(&GET_TRIG_VARS(trig), name, buf, context); }




/* typedefs that the dg functions rely on */
typedef struct index_data index_data;
typedef struct room_data room_data;
typedef struct obj_data obj_data;
typedef struct trig_data trig_data;
typedef struct char_data char_data;
