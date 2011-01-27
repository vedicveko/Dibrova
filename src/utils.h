/*************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* external declarations and prototypes **********************************/

extern struct weather_data weather_info;
extern FILE *logfile;   

#define log 	basic_mud_log

/* public functions in utils.c */
char	*str_dup(const char *source);
int	str_cmp(char *arg1, char *arg2);
int	strn_cmp(char *arg1, char *arg2, int n);
void	basic_mud_log(const char *format, ...) __attribute__ ((format (printf, 1, 2)));
int	touch(char *path);
void	mudlog(char *str, char type, int level, byte file);
void	log_death_trap(struct char_data *ch);
int	number(int from, int to);
int	dice(int number, int size);
void	sprintbit(long vektor, char *names[], char *result);
void	sprinttype(int type, char *names[], char *result);
int	get_line(FILE *fl, char *buf);
int	get_filename(char *orig_name, char *filename, int mode);
struct time_info_data age(struct char_data *ch);
int	num_pc_in_room(struct room_data *room);
void	sprintbitarray(int bitvector[], char *names[], int maxar, char *result);
int     replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
void    format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
void make_bar(char *b, int now, int max, int len);
char *lcase(char *str);
char *ucase(char *str);
int multi_check(struct char_data *ch);
int  pkill_ok(struct char_data *ch, struct char_data *victim);
void improve_abil(struct char_data *, int, byte); 
byte get_ability(struct char_data *, byte, uint);
void set_ability(struct char_data *, byte, uint, byte);
byte get_mob_ability(struct char_data *, byte, uint);
void set_mob_ability(struct char_data *, byte, uint, byte);
int countbits(int);
int is_class(struct char_data *ch, int chclass);
int can_bash(struct char_data *ch, struct char_data *victim);

/* random functions in random.c */
void circle_srandom(unsigned long initial_seed);
unsigned long circle_random(void);

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

int MAX(int a, int b);
int MIN(int a, int b);

/* in magic.c */
bool	circle_follow(struct char_data *ch, struct char_data * victim);

/* in act.informative.c */
void	look_at_room(struct char_data *ch, int mode);

/* in act.movmement.c */
int	do_simple_move(struct char_data *ch, int dir, int following);
int	perform_move(struct char_data *ch, int dir, int following);
int     mob_can_move(struct char_data *ch, int dir);

/* in limits.c */
int	mana_limit(struct char_data *ch);
int	hit_limit(struct char_data *ch);
int	move_limit(struct char_data *ch);
int	mana_gain(struct char_data *ch);
int	hit_gain(struct char_data *ch);
int	move_gain(struct char_data *ch);
void	advance_level(struct char_data *ch);
void	set_title(struct char_data *ch, char *title);
void	set_name(struct char_data *ch, char *title);
void	gain_exp(struct char_data *ch, int gain);
void	gain_exp_regardless(struct char_data *ch, int gain);
void	gain_condition(struct char_data *ch, int condition, int value);
void	check_idling(struct char_data *ch);
void	point_update(void);
void	update_pos(struct char_data *victim);

/* From races.c */
extern int min_stat_table[NUM_RACES][NUM_STATS];
extern int max_stat_table[NUM_RACES][NUM_STATS];

/* mob_skills.c */
void do_mob_bash(struct char_data *ch, struct char_data *vict);
void do_mob_disarm(struct char_data *ch, struct char_data *vict);
void do_generic_skill(struct char_data *ch, struct char_data *vict, int type, int dam);
void do_generic_knock_down(struct char_data *ch, struct char_data *vict, int type);
void mob_steal_eq(struct char_data *ch, struct char_data *vict);
void mob_steal_from_inv(struct char_data *ch, struct char_data *vict);

#define MIN_STAT(ch, stat)  (min_stat_table[(int)GET_RACE(ch)][stat])
#define MAX_STAT(ch, stat)  (max_stat_table[(int)GET_RACE(ch)][stat])


/* various constants *****************************************************/


/* defines for mudlog() */
#define OFF	0
#define BRF	1
#define NRM	2
#define CMP	3

/* get_filename() */
#define CRASH_FILE	0
#define ETEXT_FILE	1
#define SCRIPT_VARS_FILE 2
/* breadth-first searching */
#define BFS_ERROR		-1
#define BFS_ALREADY_THERE	-2
#define BFS_NO_PATH		-3

/* mud-life time */
#define SECS_PER_MUD_HOUR	75
#define SECS_PER_MUD_DAY	(24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH	(35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR	(17*SECS_PER_MUD_MONTH)

/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN	60
#define SECS_PER_REAL_HOUR	(60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY	(24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR	(365*SECS_PER_REAL_DAY)


/* string utils **********************************************************/


#define YESNO(a) ((a) ? "YES" : "NO")
#define ONOFF(a) ((a) ? "ON" : "OFF")

#define LOWER(c)   (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))
#define UPPER(c)   (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r') 
#define IF_STR(st) ((st) ? (st) : "\0")
#define CAP(st)  (*(st) = UPPER(*(st)), st)

#define AN(string) (strchr("aeiouAEIOU", *string) ? "an" : "a")


/* memory utils **********************************************************/


#define CREATE(result, type, number)  do {\
	if (!((result) = (type *) calloc ((number), sizeof(type))))\
		{ perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
		{ perror("realloc failure"); abort(); } } while(0)

/*
 * the source previously used the same code in many places to remove an item
 * from a list: if it's the list head, change the head, else traverse the
 * list looking for the item before the one to be removed.  Now, we have a
 * macro to do this.  To use, just make sure that there is a variable 'temp'
 * declared as the same type as the list to be manipulated.  BTW, this is
 * a great application for C++ templates but, alas, this is not C++.  Maybe
 * CircleMUD 4.0 will be...
 */
#define REMOVE_FROM_LIST(item, head, next)	\
   if ((item) == (head))		\
      head = (item)->next;		\
   else {				\
      temp = head;			\
      while (temp && (temp->next != (item))) \
	 temp = temp->next;		\
      if (temp)				\
         temp->next = (item)->next;	\
   }					\


/* basic bitvector utils *************************************************/

#define Q_FIELD(x)  ((int) (x) / 32)
#define Q_BIT(x)    (1 << ((x) % 32))

#define IS_SET_AR(var, bit)       ((var)[Q_FIELD(bit)] & Q_BIT(bit))
#define SET_BIT_AR(var, bit)      ((var)[Q_FIELD(bit)] |= Q_BIT(bit))
#define REMOVE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] &= ~Q_BIT(bit))
#define TOGGLE_BIT_AR(var, bit)   ((var)[Q_FIELD(bit)] = \
                                   (var)[Q_FIELD(bit)] ^ Q_BIT(bit))
#define IS_SET(flag,bit)  ((flag) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit))

#define MOB_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PLR_FLAGS(ch) ((ch)->char_specials.saved.act)
#define PRF_FLAGS(ch) ((ch)->player_specials->saved.pref)
#define AFF_FLAGS(ch) ((ch)->char_specials.saved.affected_by)
#define ROOM_FLAGS(loc) (world[(loc)].room_flags)

#define IS_NPC(ch)  (IS_SET_AR(MOB_FLAGS(ch), MOB_ISNPC))
#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define MOB_FLAGGED(ch, flag) (IS_NPC(ch) && IS_SET_AR(MOB_FLAGS(ch), (flag)))
#define PLR_FLAGGED(ch, flag) (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), (flag)))
#define AFF_FLAGGED(ch, flag) (IS_SET_AR(AFF_FLAGS(ch), (flag)))
#define PRF_FLAGGED(ch, flag) (IS_SET_AR(PRF_FLAGS(ch), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET_AR(ROOM_FLAGS(loc), (flag)))
#define EXIT_FLAGGED(exit, flag) (IS_SET((exit)->exit_info, (flag)))

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED((ch), (skill)))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PLR_FLAGS(ch), (flag))) & Q_BIT(flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT_AR(PRF_FLAGS(ch), (flag))) & Q_BIT(flag))


/* room utils ************************************************************/


#define SECT(room)	(world[(room)].sector_type)

#define IS_IMPEN(room)  (ROOM_FLAGGED(room, ROOM_IMPEN))

#define IS_DARK(room)  ( (!world[room].light && IS_IMPEN(room)) &&\
                         (ROOM_FLAGGED(room, ROOM_DARK) || \
                          ( ( SECT(room) != SECT_INSIDE && \
                              SECT(room) != SECT_CITY ) && \
                            (weather_info.sunlight == SUN_SET || \
			     weather_info.sunlight == SUN_DARK)) ) )


#define IS_LIGHT(room)  (!IS_DARK(room))

#define GET_ROOM_SPEC(room) ((room) >= 0 ? world[(room)].func : NULL)

/* char utils ************************************************************/


#define IN_ROOM(ch)	((ch)->in_room)
#define GET_WAS_IN(ch)	((ch)->was_in_room)
#define GET_AGE(ch)     (age(ch).year)

#define GET_NAME(ch)    (IS_NPC(ch) ? \
			 (ch)->player.short_descr : (ch)->player.name)
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_PASSWD(ch)	((ch)->player.passwd)
#define GET_PFILEPOS(ch)((ch)->pfilepos)

#define GET_QUEST(ch)   ((ch)->current_quest)
#define GET_QUEST_TYPE(ch) (aquest_table[real_quest((int)GET_QUEST((ch)))].type)
#define GET_NUM_QUESTS(ch)      ((ch)->num_completed_quests)

#define GET_AUTO_QUEST(ch) ((ch->quest_mob_number)

#define GET_QUESTMOB(ch)     ((ch)->player_specials->saved.questmob)
#define GET_QUESTOBJ(ch)     ((ch)->player_specials->saved.questobj)
#define GET_QUESTGIVER(ch)   ((ch)->player_specials->saved.questgiver)
#define GET_NEXTQUEST(ch)    ((ch)->player_specials->saved.nextquest)
#define GET_COUNTDOWN(ch)    ((ch)->player_specials->saved.countdown)
#define GET_PKSAFETIMER(ch)  ((ch)->player_specials->saved.pksafetimer)
#define GET_PIT_TIMER(ch)    ((ch)->player_specials->saved.uextra[0])
/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 */
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch))

#define GET_GOD_TYPE(ch) ((ch)->player_specials->saved.god_type)
#define GET_CLASS(ch)   ((ch)->player.class)
#define GET_RACE(ch)   ((ch)->player.race)
#define GET_HOME(ch)	((ch)->player.hometown)
#define GET_HEIGHT(ch)	((ch)->player.height)
#define GET_WEIGHT(ch)	((ch)->player.weight)
#define GET_SEX(ch)	((ch)->player.sex)

#define GET_STR(ch)     ((ch)->aff_stats.str)
#define GET_ADD(ch)     ((ch)->aff_stats.str_add)
#define GET_DEX(ch)     ((ch)->aff_stats.dex)
#define GET_INT(ch)     ((ch)->aff_stats.intel)
#define GET_WIS(ch)     ((ch)->aff_stats.wis)
#define GET_CON(ch)     ((ch)->aff_stats.con)
#define GET_CHA(ch)     ((ch)->aff_stats.cha)
#define GET_LUCK(ch)    ((ch)->aff_stats.luck)

#define GET_EXP(ch)	  ((ch)->points.exp)
#define GET_AC(ch)        ((ch)->points.armor)
#define GET_HIT(ch)	  ((ch)->points.hit)
#define GET_MAX_HIT(ch)	  ((ch)->points.max_hit)
#define GET_MOVE(ch)	  ((ch)->points.move)
#define GET_MAX_MOVE(ch)  ((ch)->points.max_move)
#define GET_MANA(ch)	  ((ch)->points.mana)
#define GET_MAX_MANA(ch)  ((ch)->points.max_mana)
#define GET_QI(ch)	  ((ch)->points.qi)
#define GET_MAX_QI(ch)    ((ch)->points.max_qi)
#define GET_VIM(ch)	  ((ch)->points.vim)
#define GET_MAX_VIM(ch)   ((ch)->points.max_vim)
#define GET_ARIA(ch)	  ((ch)->points.aria)
#define GET_MAX_ARIA(ch)  ((ch)->points.max_aria)
#define GET_ARENA_ROOM(ch) ((ch)->player_specials->saved.arena_room)
#define GET_GOLD(ch)	  ((ch)->points.gold)
#define GET_BANK_GOLD(ch) ((ch)->points.bank_gold)
#define GET_HITROLL(ch)	  ((ch)->points.hitroll)
#define GET_DAMROLL(ch)   ((ch)->points.damroll)
#define GET_SPELLPOWER(ch) ((ch)->points.spellpower)
#define GET_AMT_BET(ch) ((ch)->player_specials->saved.bet_amt)
#define GET_BETTED_ON(ch) ((ch)->player_specials->saved.betted_on)

#define GET_POS(ch)	  ((ch)->char_specials.position)
#define GET_IDNUM(ch)	  ((ch)->char_specials.saved.idnum)
#define GET_ID(x)         ((x)->id)
#define GET_TIMER_ONOFF(x) ((x)->timer_onoff)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)	  ((ch)->char_specials.fighting)
#define HUNTING(ch)	  ((ch)->char_specials.hunting)
#define GET_RIDER(ch)     ((ch)->char_specials.rider)
#define GET_MOUNT(ch)     ((ch)->char_specials.mount)
#define GET_SAVE(ch, i)	  ((ch)->char_specials.saved.apply_saving_throw[i])
#define GET_KILLVICTIM(ch)   ((ch)->char_specials.killvictim)
#define IS_SAFE(ch)       ((ch)->char_specials.safe)
#define GET_ALIGNMENT(ch) ((ch)->char_specials.saved.alignment)

#define GET_KILLVICTIMER(ch)   ((ch)->char_specials.killvictimer)

#define GET_COND(ch, i)		((ch)->player_specials->saved.conditions[(i)])
#define GET_LOADROOM(ch)	((ch)->player_specials->saved.load_room)
#define GET_PRACTICES(ch)	((ch)->player_specials->saved.spells_to_learn)
#define GET_INVIS_LEV(ch)	((ch)->player_specials->saved.invis_level)
#define GET_WIMP_LEV(ch)	((ch)->player_specials->saved.wimp_level)
#define GET_FREEZE_LEV(ch)	((ch)->player_specials->saved.freeze_level)
#define GET_BAD_PWS(ch)		((ch)->player_specials->saved.bad_pws)
#define GET_TALK(ch, i)		((ch)->player_specials->saved.talks[i])
#define POOFIN(ch)		((ch)->player_specials->poofin)
#define POOFOUT(ch)		((ch)->player_specials->poofout)
#define IHIDE(ch)               ((ch)->player_specials->ihide)
#define GET_LAST_OLC_TARG(ch)	((ch)->player_specials->last_olc_targ)
#define GET_LAST_OLC_MODE(ch)	((ch)->player_specials->last_olc_mode)
#define GET_PRIVATE(ch)         ((ch)->player_specials->priv)
#define GET_ALIASES(ch)		((ch)->player_specials->aliases)
#define GET_LAST_TELL(ch)	((ch)->player_specials->last_tell)
#define GET_QPOINTS(ch)         ((ch)->player_specials->saved.quest_points)
#define GET_NUM_KILLS(ch)       ((ch)->player_specials->saved.num_kills)
#define GET_NUM_DEATHS(ch)      ((ch)->player_specials->saved.num_deaths)
#define GET_TOAD_LEV(ch)        ((ch)->player_specials->saved.toad_level)
#define GET_RANK(ch)            ((ch)->player_specials->saved.rank)
#define IS_KNIGHT(ch)           ((ch)->player_specials->saved.knight)
#define GET_KINGDOM(ch)         ((ch)->player_specials->saved.king)
#define GET_TIER(ch)            ((ch)->player_specials->saved.tier)
#define GET_EQ(ch, i)		((ch)->equipment[i])
#define CORPSE_RETRIEVAL(ch)    ((ch)->player_specials->assigned)  /* Sam*/

#define GET_MOB_SPEC(ch) (IS_MOB(ch) ? (mob_index[(ch->nr)].func) : NULL)
#define GET_MOB_RNUM(mob)	((mob)->nr)
#define GET_MOB_VNUM(mob)	(IS_MOB(mob) ? \
				 mob_index[GET_MOB_RNUM(mob)].virtual : -1)

#define GET_MOB_WAIT(ch)	((ch)->mob_specials.wait_state)
#define GET_SKILL_WAIT(ch)      ((ch)->mob_specials.skill_wait)
#define GET_DEFAULT_POS(ch)	((ch)->mob_specials.default_pos)
#define MEMORY(ch)		((ch)->mob_specials.memory)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )
#define IS_IMMORT(ch) (GET_CLASS(ch) >= CLASS_VAMPIRE)
#define CAN_CARRY_W(ch) (IS_IMMORT(ch) ? (350+str_app[STRENGTH_APPLY_INDEX(ch)].carry_w) : str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (IS_IMMORT(ch) ? (35+(GET_TIER(ch) * 50)+(5+(GET_DEX(ch)>>1)+(GET_LEVEL(ch)>>1))) : (5+(GET_DEX(ch)>>1)+(GET_LEVEL(ch)>>1)))

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)
#define CAN_SEE_IN_DARK(ch) \
   (AFF_FLAGGED(ch, AFF_INFRAVISION) || PRF_FLAGGED(ch, PRF_HOLYLIGHT))

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* Abilities macro's for players */
#define GET_SKILL(ch, num)        get_ability(ch, ABT_SKILL, num)
#define SET_SKILL(ch, num, pct)   set_ability(ch, ABT_SKILL, num, pct)
#define GET_SPELL(ch, num)        get_ability(ch, ABT_SPELL, num)
#define SET_SPELL(ch, num, pct)   set_ability(ch, ABT_SPELL, num, pct)
#define GET_CHANT(ch, num)        get_ability(ch, ABT_CHANT, num)
#define SET_CHANT(ch, num, pct)   set_ability(ch, ABT_CHANT, num, pct)
#define GET_PRAYER(ch, num)       get_ability(ch, ABT_PRAYER, num)
#define SET_PRAYER(ch, num, pct)  set_ability(ch, ABT_PRAYER, num, pct)
#define GET_SONG(ch, num)         get_ability(ch, ABT_SONG, num)
#define SET_SONG(ch, num, pct)    set_ability(ch, ABT_SONG, num, pct)
/* Abilities macro's for mobiles */
#define GET_MOB_SKILL(ch, num)        get_mob_ability(ch, ABT_SKILL, num)
#define SET_MOB_SKILL(ch, num, pct)   set_mob_ability(ch, ABT_SKILL, num, pct)
#define GET_MOB_SPELL(ch, num)        get_mob_ability(ch, ABT_SPELL, num)
#define SET_MOB_SPELL(ch, num, pct)   set_mob_ability(ch, ABT_SPELL, num, pct)
#define GET_MOB_CHANT(ch, num)        get_mob_ability(ch, ABT_CHANT, num)
#define SET_MOB_CHANT(ch, num, pct)   set_mob_ability(ch, ABT_CHANT, num, pct)
#define GET_MOB_PRAYER(ch, num)       get_mob_ability(ch, ABT_PRAYER, num)
#define SET_MOB_PRAYER(ch, num, pct)  set_mob_ability(ch, ABT_PRAYER, num, pct)
#define GET_MOB_SONG(ch, num)         get_mob_ability(ch, ABT_SONG, num)
#define SET_MOB_SONG(ch, num, pct)    set_mob_ability(ch, ABT_SONG, num, pct)
/*
#define CAN_CAST(ch)  (GET_CLASS(ch)==CLASS_SORCERER || \
                       GET_CLASS(ch)==CLASS_DARK_KNIGHT || \
                       GET_CLASS(ch)==CLASS_PALADIN)
#define CAN_CHANT(ch) (GET_CLASS(ch)==CLASS_MONK)
#define CAN_PRAY(ch)  (GET_CLASS(ch)==CLASS_CLERIC)
#define CAN_SING(ch)  (GET_CLASS(ch)==CLASS_BARD)
*/
/* descriptor-based utils ************************************************/


#define WAIT_STATE(ch, cycle) { \
	if ((ch)->desc) (ch)->desc->wait = (cycle); \
	else if (IS_NPC(ch)) GET_MOB_WAIT(ch) = (cycle); }

#define CHECK_WAIT(ch)	(((ch)->desc) ? ((ch)->desc->wait > 1) : 0)
#define STATE(d)	((d)->connected)

/* object utils **********************************************************/


#define GET_OBJ_LEVEL(obj)      ((obj)->obj_flags.minlevel)
#define GET_OBJ_TYPE(obj)	((obj)->obj_flags.type_flag)
#define GET_OBJ_COST(obj)	((obj)->obj_flags.cost)
#define GET_OBJ_RENT(obj)	((obj)->obj_flags.cost_per_day)
#define GET_OBJ_EXTRA(obj)	((obj)->obj_flags.extra_flags)
#define GET_OBJ_EXTRA_AR(obj, i)   ((obj)->obj_flags.extra_flags[(i)])
#define GET_OBJ_WEAR(obj)	((obj)->obj_flags.wear_flags)
#define GET_OBJ_VAL(obj, val)	((obj)->obj_flags.value[(val)])
#define GET_OBJ_WEIGHT(obj)	((obj)->obj_flags.weight)
#define GET_OBJ_EXP(obj)	((obj)->obj_flags.exp) /* SAC */
#define GET_OBJ_SPELL(obj)	((obj)->obj_flags.spell) /* Persistent spell */
#define GET_OBJ_SPELL_EXTRA(obj) ((obj)->obj_flags.spell_extra)
#define GET_OBJ_TIMER(obj)	((obj)->obj_flags.timer)
#define GET_OBJ_VAMP(obj)       ((obj)->obj_flags.vamp_wear) /* Unholy 7 */
#define GET_OBJ_TITAN(obj)      ((obj)->obj_flags.titan_wear) /*  7 */
#define GET_OBJ_SAINT(obj)      ((obj)->obj_flags.saint_wear) /*  7 */
#define GET_OBJ_DEMON(obj)      ((obj)->obj_flags.demon_wear) /*  7 */
#define GET_OBJ_BID_NUM(obj)    ((obj)->obj_flags.obj_bid_num)
#define GET_OBJ_BID_AMNT(obj)   ((obj)->obj_flags.bid_amnt)
#define GET_OBJ_AUC_TIMER(obj)  ((obj)->obj_flags.auc_timer)
#define GET_OBJ_AUC_PLAYER(obj) ((obj)->obj_flags.auc_player)
#define GET_OBJ_AUC_SELLER(obj) ((obj)->obj_flags.auc_seller)
#define GET_OBJ_CLAN(obj)       ((obj)->obj_flags.clan)
#define GET_OBJ_PLAYER(obj)     ((obj)->obj_flags.player)
#define GET_OBJ_RNUM(obj)	((obj)->item_number)
#define GET_OBJ_VNUM(obj)	(GET_OBJ_RNUM(obj) >= 0 ? \
				 obj_index[GET_OBJ_RNUM(obj)].virtual : -1)
#define IS_OBJ_STAT(obj,stat)	(IS_SET_AR((obj)->obj_flags.extra_flags, \
                                 (stat)))
#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
	(obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET_AR((obj)->obj_flags.wear_flags, (part)))

//#define IS_CORPSE(obj) ((GET_OBJ_VAL(obj, 3) = 1) && (GET_OBJ_TYPE(obj) = ITEM_CONTAINER))

#define GET_BIDDER(obj)   ((obj)->bidder)

#define GET_PKLOOTER(obj)  ((obj)->pkiller)

/* compound utilities and other macros **********************************/


#define HSHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "his":"her") :"its")
#define HSSH(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "he" :"she") : "it")
#define HMHR(ch) (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "him":"her") : "it")

#define ANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")
#define SANA(obj) (strchr("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")


/* Various macros building up to CAN_SEE */

#define LIGHT_OK(sub)	(!IS_AFFECTED(sub, AFF_BLIND) && \
   (IS_LIGHT((sub)->in_room) || IS_AFFECTED((sub), AFF_INFRAVISION)))

#define INVIS_OK(sub, obj) \
 ((!IS_AFFECTED((obj),AFF_INVISIBLE) || IS_AFFECTED(sub,AFF_DETECT_INVIS)) && \
 (!IS_AFFECTED((obj), AFF_HIDE) || IS_AFFECTED(sub, AFF_SENSE_LIFE)))

#define MORT_CAN_SEE(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK(sub, obj))


#define IMM_CAN_SEE(sub, obj) \
   (MORT_CAN_SEE(sub, obj) || PRF_FLAGGED(sub, PRF_HOLYLIGHT) || \
    IS_AFFECTED(obj, AFF_BURROW))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"? */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
   ((GET_REAL_LEVEL(sub) >= GET_INVIS_LEV(obj)) && IMM_CAN_SEE(sub, obj) && \
     !IS_AFFECTED(obj, AFF_BURROW)))

/* End of CAN_SEE */


#define IS_BURIED(obj) (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_BURIED))

#define INVIS_OK_OBJ(sub, obj) \
  (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || IS_AFFECTED((sub), AFF_DETECT_INVIS))

#define MORT_CAN_SEE_OBJ(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && \
  (!IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_VAMP_CAN_SEE)) && \
  (!IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_TITAN_CAN_SEE)) && \
  (!IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_SAINT_CAN_SEE)) && \
  (!IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_DEMON_CAN_SEE)))

#define VAMP_CAN_SEE_OBJ(sub, obj) \
   (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_VAMP_CAN_SEE) && \
    IS_AFFECTED((sub), AFF_VAMP_AURA))

#define TITAN_CAN_SEE_OBJ(sub, obj) \
   (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_TITAN_CAN_SEE) && \
    GET_CLASS(sub) == CLASS_TITAN)

#define SAINT_CAN_SEE_OBJ(sub, obj) \
   (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_SAINT_CAN_SEE) && \
    GET_CLASS(sub) == CLASS_SAINT)

#define DEMON_CAN_SEE_OBJ(sub, obj) \
   (IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_DEMON_CAN_SEE) && \
    GET_CLASS(sub) == CLASS_DEMON)

#define CAN_SEE_OBJ(sub, obj) (!IS_BURIED(obj) && \
   (MORT_CAN_SEE_OBJ(sub, obj) || PRF_FLAGGED((sub), PRF_HOLYLIGHT) || \
    VAMP_CAN_SEE_OBJ(sub, obj) || TITAN_CAN_SEE_OBJ(sub, obj) || \
    SAINT_CAN_SEE_OBJ(sub, obj) || DEMON_CAN_SEE_OBJ(sub, obj) )) 

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
    CAN_SEE_OBJ((ch),(obj)))


#define PERS(ch, vict)   (CAN_SEE(vict, ch) ? GET_NAME(ch) : "someone")

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	(obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
	fname((obj)->name) : "something")


#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])
#define CAN_GO(ch, door) (EXIT(ch,door) && \
			 (EXIT(ch,door)->to_room != NOWHERE) && \
			 !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
#define DOOR_IS_OPENABLE(ch, obj, door)	((obj) ? \
			((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSEABLE))) :\
			(IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)))
#define DOOR_IS_OPEN(ch, obj, door)	((obj) ? \
			(!IS_SET(GET_OBJ_VAL(obj, 1), CONT_CLOSED)) :\
			(!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)))
#define DOOR_IS_PICKPROOF(ch, obj, door) ((obj) ? \
			(IS_SET(GET_OBJ_VAL(obj, 1), CONT_PICKPROOF)) : \
			(IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)))
#define DOOR_IS_CLOSED(ch, obj, door)	(!(DOOR_IS_OPEN(ch, obj, door)))
#define DOOR_IS_LOCKED(ch, obj, door)	(!(DOOR_IS_UNLOCKED(ch, obj, door)))
#define DOOR_KEY(ch, obj, door)		((obj) ? (GET_OBJ_VAL(obj, 2)) : \
					(EXIT(ch, door)->key))
#define DOOR_LOCK(ch, obj, door)	((obj) ? (GET_OBJ_VAL(obj, 1)) : \
					(EXIT(ch, door)->exit_info))
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


#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[(int)GET_CLASS(ch)])
#define RACE_ABBR(ch) (IS_NPC(ch) ? "--" : race_abbrevs[(int)GET_RACE(ch)])

#define IS_SORCERER(ch) 	((!IS_NPC(ch)) && \
				(GET_CLASS(ch) == CLASS_SORCERER))
#define IS_CLERIC(ch)		((!IS_NPC(ch)) && \
				(GET_CLASS(ch) == CLASS_CLERIC))
#define IS_THIEF(ch)		((!IS_NPC(ch)) && \
				(GET_CLASS(ch) == CLASS_THIEF))
#define IS_GLADIATOR(ch)	((!IS_NPC(ch)) && \
				(GET_CLASS(ch) == CLASS_GLADIATOR))
#define IS_DRUID(ch)		((!IS_NPC(ch)) && \
				(GET_CLASS(ch) == CLASS_DRUID))
#define IS_DARK_KNIGHT(ch)      ((!IS_NPC(ch)) && \
                                (GET_CLASS(ch) == CLASS_DARK_KNIGHT))
#define IS_MONK(ch)             ((!IS_NPC(ch)) && \
                                (GET_CLASS(ch) == CLASS_MONK))
#define IS_PALADIN(ch)          ((!IS_NPC(ch)) && \
                                (GET_CLASS(ch) == CLASS_PALADIN))
#define IS_BARD(ch)             (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_BARD))
#define IS_ASSASSIN(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ASSASSIN))
#define IS_SHADOWMAGE(ch)       (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SHADOWMAGE))
#define IS_INQUISITOR(ch)       (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_INQUISITOR))
#define IS_ACROBAT(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ACROBAT))
#define IS_RANGER(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_RANGER))
#define IS_WARLOCK(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_WARLOCK))
#define IS_CHAMPION(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_CHAMPION))
#define IS_REAPER(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_REAPER))
#define IS_COMBAT_MASTER(ch)    (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_COMBAT_MASTER))
#define IS_CRUSADER(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_CRUSADER))
#define IS_BEASTMASTER(ch)      (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_BEASTMASTER))
#define IS_ARCANIC(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_ARCANIC))
#define IS_MAGI(ch)             (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_MAGI))
#define IS_DARKMAGE(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_DARKMAGE))
#define IS_FORESTAL(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_FORESTAL))
#define IS_PSIONIST(ch)         (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_PSIONIST))
#define IS_TEMPLAR(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_TEMPLAR))
#define IS_STORR(ch)            (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_STORR))
#define IS_SAGE(ch)             (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SAGE))
#define IS_SAMURAI(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SAMURAI))
#define IS_FIANNA(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_FIANNA))
#define IS_SHAMAN(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SHAMAN))
#define IS_VAMPIRE(ch)          (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_VAMPIRE))
#define IS_TITAN(ch)            (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_TITAN))
#define IS_SAINT(ch)            (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_SAINT))
#define IS_DEMON(ch)            (!IS_NPC(ch) && \
                                (GET_CLASS(ch) == CLASS_DEMON))
#define IS_REMORT(ch)           (!IS_NPC(ch) && \
                                (GET_CLASS(ch) > CLASS_BARD))


#define CAN_CAST(ch)  (GET_CLASS(ch) == CLASS_SORCERER || \
                       GET_CLASS(ch) == CLASS_DARK_KNIGHT || \
                       GET_CLASS(ch) == CLASS_INQUISITOR || \
                       GET_CLASS(ch) == CLASS_ASSASSIN || \
                       GET_CLASS(ch) == CLASS_SHADOWMAGE || \
                       GET_CLASS(ch) == CLASS_WARLOCK || \
                       GET_CLASS(ch) == CLASS_REAPER || \
                       GET_CLASS(ch) == CLASS_CRUSADER || \
                       GET_CLASS(ch) == CLASS_ARCANIC || \
                       GET_CLASS(ch) == CLASS_MAGI || \
                       GET_CLASS(ch) == CLASS_DARKMAGE || \
                       GET_CLASS(ch) == CLASS_INQUISITOR || \
                       GET_CLASS(ch) == CLASS_TEMPLAR || \
                       GET_CLASS(ch) == CLASS_SAMURAI || \
                       GET_CLASS(ch) == CLASS_FIANNA || \
                       GET_CLASS(ch) == CLASS_DRUID || \
                       GET_CLASS(ch) == CLASS_SAGE || \
                       GET_CLASS(ch) == CLASS_RANGER || \
                       GET_CLASS(ch) == CLASS_BEASTMASTER || \
                       GET_CLASS(ch) == CLASS_VAMPIRE || \
                       GET_CLASS(ch) == CLASS_TITAN || \
                       GET_CLASS(ch) == CLASS_SAINT || \
                       GET_CLASS(ch) == CLASS_DEMON || \
                       GET_CLASS(ch) == CLASS_PALADIN)
#define CAN_CHANT(ch) (GET_CLASS(ch) == CLASS_MONK || \
                       GET_CLASS(ch) == CLASS_ACROBAT || \
                       GET_CLASS(ch) == CLASS_PSIONIST || \
                       GET_CLASS(ch) == CLASS_COMBAT_MASTER || \
                       GET_CLASS(ch) == CLASS_STORR || \
                       GET_CLASS(ch) == CLASS_SAMURAI || \
                       GET_CLASS(ch) == CLASS_VAMPIRE || \
                       GET_CLASS(ch) == CLASS_SAINT || \
                       GET_CLASS(ch) == CLASS_TITAN || \
                       GET_CLASS(ch) == CLASS_DEMON || \
                       GET_CLASS(ch) == CLASS_SHAMAN)
#define CAN_PRAY(ch)  (GET_CLASS(ch) == CLASS_CLERIC || \
                       GET_CLASS(ch) == CLASS_TEMPLAR || \
                       GET_CLASS(ch) == CLASS_CRUSADER || \
                       GET_CLASS(ch) == CLASS_INQUISITOR || \
                       GET_CLASS(ch) == CLASS_ARCANIC || \
                       GET_CLASS(ch) == CLASS_STORR || \
                       GET_CLASS(ch) == CLASS_VAMPIRE || \
                       GET_CLASS(ch) == CLASS_SAINT || \
                       GET_CLASS(ch) == CLASS_DEMON || \
                       GET_CLASS(ch) == CLASS_TITAN || \
                       GET_CLASS(ch) == CLASS_SAGE)
#define CAN_SING(ch)  (GET_CLASS(ch)==CLASS_BARD)


#define MINOR_PROTECT_OK(ch)   (!IS_AFFECTED(ch, AFF_ARMOR) && \
				!IS_AFFECTED(ch, AFF_HEAVY_SKIN) && \
				!IS_AFFECTED(ch, AFF_AEGIS) && \
				!IS_AFFECTED(ch, AFF_FIRESHIELD) && \
				!IS_AFFECTED(ch, AFF_CHAOSARMOR) && \
				!IS_AFFECTED(ch, AFF_PHANTOM_ARMOR) && \
				!IS_AFFECTED(ch, AFF_HOLYARMOR))

#define MAJOR_PROTECT_OK(ch)   (!IS_AFFECTED(ch, AFF_PROTFROMGOOD) && \
				!IS_AFFECTED(ch, AFF_PROTECT_EVIL) && \
				!IS_AFFECTED(ch, AFF_INDESTR_AURA) && \
				!IS_AFFECTED(ch, AFF_BLAZEWARD) && \
				!IS_AFFECTED(ch, AFF_SANCTUARY) && \
				!IS_AFFECTED(ch, AFF_SACRED_SHIELD) && \
				!IS_AFFECTED(ch, AFF_DIVINE_NIMBUS) && \
				!IS_AFFECTED(ch, AFF_ANCIENT_PROT) && \
				!IS_AFFECTED(ch, AFF_VAMP_AURA) && \
				!IS_AFFECTED(ch, AFF_NETHERBLAZE) && \
				!IS_AFFECTED(ch, AFF_EARTH_SHROUD) && \
				!IS_AFFECTED(ch, AFF_DARKWARD))


#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))


#define IS_DWARF(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_DWARF))
#define IS_ELDAR(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_ELDAR))
#define IS_ELF(ch)              (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_ELF))
#define IS_GIANT(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_GIANT))
#define IS_HALFLING(ch)         (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_HALFLING))
#define IS_HUMAN(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_HUMAN))
#define IS_KENDER(ch)           (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_KENDER))
#define IS_OGRE(ch)             (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_OGRE))
#define IS_TRITON(ch)           (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_TRITON))
#define IS_VALKYRIE(ch)         (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_VALKYRIE))
#define IS_WOLFEN(ch)           (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_WOLFEN))
#define IS_GOBLIN(ch)          	(!IS_NPC(ch) && \
           			(GET_RACE(ch) == RACE_GOBLIN))
#define IS_DRIDER(ch)           (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_DRIDER))
#define IS_TROLL(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_TROLL))
#define IS_PIXIE(ch)            (!IS_NPC(ch) && \
           			(GET_RACE(ch) == RACE_PIXIE))
#define IS_DROW(ch)             (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_DROW))
#define IS_GNOME(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_GNOME))
#define IS_NYMPH(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_NYMPH))
#define IS_CENTAUR(ch)          (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_CENTAUR))
#define IS_NAGA(ch)             (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_NAGA))
#define IS_GARGOYLE(ch)         (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_GARGOYLE))
#define IS_HARPY(ch)            (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_HARPY))
#define IS_DRACONIAN(ch)        (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_DRACONIAN))
#define IS_SCORPIUS(ch)         (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_SCORPIUS))
#define IS_SPRITE(ch)           (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_SPRITE))
#define IS_AVIADES(ch)          (!IS_NPC(ch) && \
                                (GET_RACE(ch) == RACE_AVIADES))

#define IS_SIZE_SMALL(ch)       (IS_GOBLIN(ch) || IS_GOBLIN(ch) || \
				 IS_NYMPH(ch) || IS_DROW(ch) || \
				 IS_HARPY(ch) || IS_AVIADES(ch) || \
 				 IS_ELF(ch))
#define IS_SIZE_MEDIUM(ch)      (IS_DRIDER(ch) || IS_VALKYRIE(ch) || \
 				 IS_ELDAR(ch) || IS_NAGA(ch) || \
				 IS_DRACONIAN(ch) || IS_HUMAN(ch) || \
				 IS_TRITON(ch))
#define IS_SIZE_LARGE(ch)       (IS_TROLL(ch) || IS_GIANT(ch) || \
				 IS_CENTAUR(ch) || IS_GARGOYLE(ch) || \
				 IS_SCORPIUS(ch) || IS_WOLFEN(ch))
#define IS_SIZE_TINY(ch)        (IS_KENDER(ch) || IS_GNOME(ch) || \
				 IS_PIXIE(ch) || IS_SPRITE(ch))

/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif
#define SENDOK(ch)	(((ch)->desc || SCRIPT_CHECK((ch), MTRIG_ACT)) && \
			(sleep || AWAKE(ch)) && \
			!PLR_FLAGGED((ch), PLR_WRITING))

#define GET_KILLS_VNUM(ch, a)   ((ch)->player_specials->saved.kills_vnum[a-1])
#define GET_KILLS_AMOUNT(ch, a) ((ch)->player_specials->saved.kills_amount[a-1])
#define GET_KILLS_CURPOS(ch)    ((ch)->player_specials->saved.kills_curpos)
#define GET_MOB_MAXFACTOR(ch)   ((ch)->mob_specials.maxfactor)
#define GET_OBJ_VROOM(obj)      ((obj)->vroom)

#define IS_CORPSE(obj)          (GET_OBJ_TYPE(obj) == ITEM_CONTAINER && \
                                        GET_OBJ_VAL((obj), 3) == 1 && \
                                (IS_OBJ_STAT(obj, ITEM_PC_CORPSE) || \
                                IS_OBJ_STAT(obj, ITEM_NPC_CORPSE)))
#define WEARALL    1  /*Used in perform_wear */
#define NOTWEARALL 0

// Stuff item materials
#define IS_METAL(obj)            (GET_OBJ_MATERIAL(obj) == MATERIAL_IRON || \
                                  GET_OBJ_MATERIAL(obj) == MATERIAL_SILVER || \
                                  GET_OBJ_MATERIAL(obj) == MATERIAL_BRONZE || \
                                  GET_OBJ_MATERIAL(obj) == MATERIAL_COPPER || \
                                  GET_OBJ_MATERIAL(obj) == MATERIAL_STEEL)
#define ITEM_RACE_BONUS(obj,stat)(IS_SET_AR((obj)->obj_flags.racial_bonus, \
                                 (stat)))
#define GET_OBJ_MATERIAL(obj)       ((obj)->obj_flags.material)

#define MAX_OBJ_LOAD_PERCENT          60
int load_percent(int current);

#define RM_BLOOD(rm)   ((int)world[rm].blood)

