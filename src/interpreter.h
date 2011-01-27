/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define ACMD(name)  \
   void name(struct char_data *ch, char *argument, int cmd, int subcmd)

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strcmp(cmd_name, cmd_info[cmd].command))
#define IS_MOVE(cmdnum) (cmdnum >= 1 && cmdnum <= 6)

/* necessary for CMD_IS macro */
#ifndef __INTERPRETER_C__
extern struct command_info cmd_info[];
#endif


void	command_interpreter(struct char_data *ch, char *argument);
int	search_block(char *arg, char **list, int exact);
char	lower( char c );
char	*one_argument(char *argument, char *first_arg);
char	*one_word(char *argument, char *first_arg);
char	*any_one_arg(char *argument, char *first_arg);
char	*two_arguments(char *argument, char *first_arg, char *second_arg);
int	fill_word(char *argument);
void	half_chop(char *string, char *arg1, char *arg2);
void	nanny(struct descriptor_data *d, char *arg);
int	is_abbrev(char *arg1, char *arg2);
int	is_number(char *str);
int	find_command(char *command);
void	skip_spaces(char **string);
char	*delete_doubledollar(char *string);

/* for compatibility with 2.20: */
#define argument_interpreter(a, b, c) two_arguments(a, b, c)


struct command_info {
   char *command;
   byte minimum_position;
   void	(*command_pointer)
   (struct char_data *ch, char * argument, int cmd, int subcmd);
   sh_int minimum_level;
   int	subcmd;
};

struct alias {
  char *alias;
  char *replacement;
  int type;
  struct alias *next;
};

#define ALIAS_SIMPLE	0
#define ALIAS_COMPLEX	1

#define ALIAS_SEP_CHAR	';'
#define ALIAS_VAR_CHAR	'$'
#define ALIAS_GLOB_CHAR	'*'

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH	1
#define SCMD_EAST	2
#define SCMD_SOUTH	3
#define SCMD_WEST	4
#define SCMD_UP		5
#define SCMD_DOWN	6
#define SCMD_NORTHEAST  7
#define SCMD_NORTHWEST  8
#define SCMD_SOUTHEAST  9
#define SCMD_SOUTHWEST  10

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1 
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_WIZLIST    4
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD	8
#define SCMD_IMOTD	9
#define SCMD_CLEAR	10
#define SCMD_WHOAMI	11

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL	4
#define SCMD_NOAUCTION	5
#define SCMD_DEAF	6
#define SCMD_NOGOSSIP	7
#define SCMD_NOGRATZ	8
#define SCMD_NOWIZ	9
#define SCMD_QUEST	10
#define SCMD_ROOMFLAGS	11
#define SCMD_NOREPEAT	12
#define SCMD_HOLYLIGHT	13
#define SCMD_SLOWNS	14
#define SCMD_AUTOEXIT	15
#define SCMD_AUTOASSIST	16
#define SCMD_AUTOSPLIT	17
#define SCMD_SHOWTIPS	18
#define SCMD_AFK	19
#define SCMD_ARENA      20
#define SCMD_AUTOLOOT   21
#define SCMD_NOINFO     22
#define SCMD_AUTODIAG   23
#define SCMD_AUTOSAC    24
#define SCMD_AUTOTITLE  25
#define SCMD_AUTOMAP    26
#define SCMD_NORANK     27
#define SCMD_NOGRID     28
#define SCMD_AUTOGOLD   29
#define SCMD_MOBFLAGS   30
#define SCMD_NOFOLLOW   31

/* do_wizutil */
#define SCMD_PARDON     0
#define SCMD_NOTITLE    1
#define SCMD_SQUELCH    2
#define SCMD_FREEZE	3
#define SCMD_THAW	4
#define SCMD_UNAFFECT	5
#define SCMD_TOAD       6
#define SCMD_TOADOFF    7

/* do_spec_com */
#define SCMD_WHISPER	0
#define SCMD_ASK	1

/* do_gen_com */
//#define SCMD_HOLLER	0
#define SCMD_COMMUNE	0
#define SCMD_SHOUT	1
#define SCMD_GOSSIP	2
#define SCMD_AUCTION	3
#define SCMD_GRATZ	4
#define SCMD_PRIVATE    5
#define SCMD_SING       6
#define SCMD_GMOTE      7

/* do_shutdown */
#define SCMD_SHUTDOW	0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI	0
#define SCMD_QUIT	1

/* do_date */
#define SCMD_DATE	0
#define SCMD_UPTIME	1

/* do_commands */
#define SCMD_COMMANDS	0
#define SCMD_SOCIALS	1
#define SCMD_WIZHELP	2

/* do_drop */
#define SCMD_DROP	0
#define SCMD_JUNK	1
#define SCMD_DONATE	2

/* do_gen_write */
#define SCMD_BUG	0
#define SCMD_TYPO	1
#define SCMD_IDEA	2

/* do_look */
#define SCMD_LOOK	0
#define SCMD_READ	1

/* do_qcomm */
#define SCMD_QSAY	0
#define SCMD_QECHO	1

/* do_pour */
#define SCMD_POUR	0
#define SCMD_FILL	1

/* do_poof */
// #define SCMD_POOFIN	0
// #define SCMD_POOFOUT	1

/* do_hit */
#define SCMD_HIT	0
#define SCMD_MURDER	1

/* do_eat */
#define SCMD_EAT	0
#define SCMD_TASTE	1
#define SCMD_DRINK	2
#define SCMD_SIP	3

/* do_use */
#define SCMD_USE	0
#define SCMD_QUAFF	1
#define SCMD_RECITE	2

/* do_echo */
#define SCMD_ECHO	0
#define SCMD_EMOTE	1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4
#define SCMD_DOORBASH   5

/* do_olc */
#define SCMD_OLC_REDIT		0
#define SCMD_OLC_OEDIT		1
#define SCMD_OLC_ZEDIT		2
#define SCMD_OLC_MEDIT		3
#define SCMD_OLC_SEDIT		4
#define SCMD_OLC_TRIGEDIT	5
#define SCMD_OLC_SAVEINFO	7
#define SCMD_OLC_QEDIT		6
#define SCMD_OLC_HEDIT		7

/* Other SCMD's... */
#define SCMD_FLEE    0
#define SCMD_RETREAT 1

/* stuff for portals... */
/* storm 7/23/98*/
#define SCMD_JUMP 1

/* do_private_channel */
#define PRIVATE_HELP    0
#define PRIVATE_OPEN    1
#define PRIVATE_CLOSE   2
#define PRIVATE_OFF     3
#define PRIVATE_ADD     4
#define PRIVATE_REMOVE  5
#define PRIVATE_WHO     6
#define PRIVATE_CHECK   7

/* do_qpmanage (quest.c) */
#define SCMD_ADD 0
#define SCMD_REMOVE 1

/* do_spells (act.informative.c) and also do_abilset (modify.c).  Since
   these are used with other functions, be sure to keep these SCMD's in 
   synch with the ABT's in spells.h */
#define SCMD_SKILLS  ABT_SKILL
#define SCMD_SPELLS  ABT_SPELL
#define SCMD_CHANTS  ABT_CHANT
#define SCMD_PRAYERS ABT_PRAYER
#define SCMD_SONGS   ABT_SONG

/* do_get       */
#define SCMD_GET     0
#define SCMD_SLEIGHT 1

// for skill quickdraw 7.4.01 Rapideye
#define SCMD_REAL_QUICKDRAW       0
#define SCMD_PRACTICE_QUICKDRAW   1
