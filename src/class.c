/*************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */


#include <math.h>

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "interpreter.h"
#include "comm.h"


/* Names first */

const char *class_abbrevs[] = {
    "Sor",
    "Cle",
    "Thi",
    "Gla",
    "Dru",
    "DKn",
    "Mon",
    "Pal",
    "Bar",
    "/cCAss/c0",
    "/cCShw/c0",
    "/cCInq/c0",
    "/cCAcr/c0",
    "/cCRan/c0",
    "/cCWar/c0",
    "/cCChm/c0",
    "/cCRpr/c0",
    "/cCCom/c0",
    "/cCCru/c0",
    "/cCBea/c0",
    "/cCArc/c0",
    "/cCMgi/c0",
    "/cCDkM/c0",
    "/cCFor/c0",
    "/cCPsi/c0",
    "/cCTem/c0",
    "/cCSto/c0",
    "/cCSag/c0",
    "/cCSam/c0",
    "/cCFia/c0",
    "/cCShm/c0",
    "/cCMer/c0",
    "/cCDes/c0",
    "/cCRog/c0",
    "/cCStx/c0",
    "/cCN/A/c0",
    "/cCN/A/c0",
    "/cCN/A/c0",
    "/cRVam/c0",
    "/cYTtn/c0",
    "/cWSnt/c0",
    "/crDmn/c0",
    "\n"
};


const char *pc_class_types[] = {
    "Sorcerer",
    "Cleric",
    "Thief",
    "Gladiator",
    "Druid",
    "Dark Knight",
    "Monk",
    "Paladin",
    "Bard",
    "Assassin",
    "Shadowmage",
    "Inquisitor",
    "Acrobat",
    "Ranger",
    "Warlock",
    "Champion",
    "Reaper",
    "Combat Master",
    "Crusader",
    "Beastmaster",
    "Arcanic",
    "Magi",
    "Darkmage",
    "Forestal",
    "Psionist",
    "Templar",
    "Storr",
    "Sage",
    "Samurai",
    "Fianna",
    "Shaman",
    "Mercenary",
    "Destroyer",
    "Rogue",
    "Styxian",
    "UNUSED",
    "UNUSED",
    "UNUSED",
    "Vampire",
    "Titan",
    "Saint",
    "Demon",
    "\n"
};


/* The menu for choosing a class in interpreter.c: */
const char *class_menu =
    "\r\n"
    " /cWSelect a Class:\r\n\r\n"
    " /cwCasting Classes         Melee Classes        Hybrid Classes\r\n"
    " /c0-----------------------------------------------------------\r\n"
    " /cr[S]/cworcerer              /cr[G]/cwladiator          /cr[D]/cwark Knight\r\n"
    " /cr[C]/cwleric                /cr[T]/cwhief              /cr[P]/cwaladin\r\n"
    "                                              /cr[M]/cwonk\r\n"
    "                                             /cwD/cr[R]/cwuid\r\n";

/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_class(char arg)
{
    arg = LOWER(arg);

    switch (arg) {
    case 's':
    case 'S':
	return CLASS_SORCERER;
	break;
    case 'c':
    case 'C':
	return CLASS_CLERIC;
	break;
    case 'g':
    case 'G':
	return CLASS_GLADIATOR;
	break;
    case 't':
    case 'T':
	return CLASS_THIEF;
	break;
    case 'r':
    case 'R':
	return CLASS_DRUID;
	break;
    case 'd':
    case 'D':
	return CLASS_DARK_KNIGHT;
	break;
    case 'm':
    case 'M':
	return CLASS_MONK;
	break;
    case 'p':
    case 'P':
	return CLASS_PALADIN;
	break;
/*
  case 'b':
  case 'B': 
    return CLASS_BARD;
    break;
*/
    default:
	return CLASS_UNDEFINED;
	break;
    }
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

long find_class_bitvector(char arg)
{
    arg = LOWER(arg);

    switch (arg) {
    case 's':
    case 'S':
	return (1 << 0);
	break;
    case 'c':
    case 'C':
	return (1 << 1);
	break;
    case 't':
    case 'T':
	return (1 << 2);
	break;
    case 'g':
    case 'G':
	return (1 << 3);
	break;
    case 'r':
    case 'R':
	return (1 << 4);
	break;
    case 'd':
    case 'D':
	return (1 << 5);
	break;
    case 'm':
    case 'M':
	return (1 << 6);
	break;
    case 'p':
    case 'P':
	return (1 << 7);
	break;
    default:
	return 0;
	break;
    }
}


/*
 * These are definitions which control the guildmasters for each class.
 *
 * The first field (top line) controls the highest percentage skill level
 * a character of the class is allowed to attain in any skill.  (After
 * this level, attempts to practice will say "You are already learned in
 * this area."
 * 
 * The second line controls the maximum percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out higher than this number, the gain will only be
 * this number instead.
 *
 * The third line controls the minimu percent gain in learnedness a
 * character is allowed per practice -- in other words, if the random
 * die throw comes out below this number, the gain will be set up to
 * this number.
 * 
 * The fourth line simply sets whether the character knows 'spells'
 * or 'skills'.  This does not affect anything except the message given
 * to the character when trying to practice (i.e. "You know of the
 * following spells" vs. "You know of the following skills"
 */

#define SKILL   0
#define SPELL   1
#define CHANT   2
#define PRAYER  3
#define SONG    4

/* #define LEARNED_LEVEL	0  % known which is considered "learned" */
/* #define MAX_PER_PRAC		1  max percent gain in skill per practice */
/* #define MIN_PER_PRAC		2  min percent gain in skill per practice */
/* #define PRAC_TYPE		3  should it say 'spell', 'skill'? or...  */

int prac_params[4][NUM_CLASSES] = {
    /* SOR    CLE   THE   GLA   RAN   DKN   MON   PAL   BAR */
    {95, 95, 95, 95, 95, 95, 95, 95, 95,
     95, 95, 95, 95, 95, 95, 95, 95, 95,
     95, 95, 95, 95, 95, 95, 95, 95, 95,
     95, 95, 95, 95, 100, 100, 100, 100},	/* learned level */
    {30, 30, 20, 20, 20, 25, 24, 25, 24,
     20, 20, 20, 20, 20, 20, 20, 20, 20,
     20, 20, 20, 20, 20, 20, 20, 20, 20,
     20, 20, 20, 20, 15, 15, 15, 15},	/* max per prac  */
    {15, 15, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 10, 10, 10, 10, 10,
     10, 10, 10, 10, 5, 5, 5, 5},	/* min per pac   */
    {SPELL, PRAYER, SKILL, SKILL, SKILL, SPELL, CHANT, SPELL, SONG,
     SPELL, SPELL, SKILL, CHANT, SKILL, SPELL, SPELL, SPELL, CHANT,
     PRAYER, SKILL, PRAYER, SPELL, SPELL, SKILL, CHANT, SPELL, CHANT,
     PRAYER, CHANT, SPELL, CHANT, SPELL, SPELL, SPELL, SPELL}	/* prac name     */
};


/*
 * ...And the appropriate rooms for each guildmaster/guildguard; controls
 * which types of people the various guildguards let through.  i.e., the
 * first line shows that from room 3017, only MAGIC_USERS are allowed
 * to go south.
 */
int guild_info[][3] = {

/* Midgaard */
    {CLASS_SORCERER, 3017, SCMD_SOUTH},
    {CLASS_CLERIC, 3004, SCMD_NORTH},
    {CLASS_THIEF, 3027, SCMD_EAST},
    {CLASS_GLADIATOR, 3021, SCMD_EAST},
    {CLASS_DRUID, 3076, SCMD_NORTH},
    {CLASS_DARK_KNIGHT, 3077, SCMD_SOUTH},
    {CLASS_MONK, 3082, SCMD_NORTH},
    {CLASS_PALADIN, 3085, SCMD_EAST},

/* Jareth */
    {CLASS_SORCERER, 1037, SCMD_WEST},
    {CLASS_CLERIC, 1077, SCMD_WEST},
    {CLASS_GLADIATOR, 1074, SCMD_WEST},
    {CLASS_DRUID, 1066, SCMD_SOUTH},
    {CLASS_MONK, 1071, SCMD_EAST},
    {CLASS_PALADIN, 1060, SCMD_EAST},
    {CLASS_DARK_KNIGHT, 1058, SCMD_WEST},

/* McGintey */
    {CLASS_SORCERER, 6808, SCMD_EAST},
    {CLASS_THIEF, 6821, SCMD_NORTH},
    {CLASS_DRUID, 6818, SCMD_SOUTH},
    {CLASS_GLADIATOR, 6827, SCMD_SOUTH},
    {CLASS_PALADIN, 6869, SCMD_WEST},
    {CLASS_CLERIC, 6846, SCMD_NORTH},
    {CLASS_DARK_KNIGHT, 6857, SCMD_WEST},
    {CLASS_MONK, 6875, SCMD_SOUTH},

/* Silverthorne */
    {CLASS_SORCERER, 5722, SCMD_EAST},
    {CLASS_PALADIN, 5723, SCMD_WEST},
    {CLASS_GLADIATOR, 5720, SCMD_EAST},
    {CLASS_CLERIC, 5721, SCMD_WEST},
    {CLASS_THIEF, 5719, SCMD_EAST},
    {CLASS_DARK_KNIGHT, 5726, SCMD_WEST},
    {CLASS_MONK, 5724, SCMD_EAST},
    {CLASS_DRUID, 5725, SCMD_WEST},

/* Ofingia */
    {CLASS_SORCERER, 8019, SCMD_SOUTH},
    {CLASS_PALADIN, 8008, SCMD_SOUTH},
    {CLASS_GLADIATOR, 8014, SCMD_EAST},
    {CLASS_CLERIC, 8003, SCMD_EAST},
    {CLASS_THIEF, 8011, SCMD_EAST},
    {CLASS_DARK_KNIGHT, 8015, SCMD_SOUTH},
    {CLASS_MONK, 8018, SCMD_EAST},
    {CLASS_DRUID, 8012, SCMD_EAST},

/* Elven City */
    {CLASS_SORCERER, 19063, SCMD_WEST},
    {CLASS_PALADIN, 19068, SCMD_EAST},
    {CLASS_GLADIATOR, 19026, SCMD_NORTH},
    {CLASS_CLERIC, 19024, SCMD_EAST},
    {CLASS_THIEF, 19022, SCMD_EAST},
    {CLASS_DARK_KNIGHT, 19070, SCMD_WEST},
    {CLASS_MONK, 19072, SCMD_NORTH},
    {CLASS_DRUID, 19066, SCMD_SOUTH},

/* New Thalos */
    {CLASS_CLERIC, 17612, SCMD_SOUTH},
    {CLASS_THIEF, 17632, SCMD_SOUTH},
    {CLASS_GLADIATOR, 17626, SCMD_SOUTH},
    {CLASS_SORCERER, 17625, SCMD_SOUTH},

    /* this must go last -- add new guards above! */
    {-1, -1, -1}
};

int thaco[NUM_CLASSES][LVL_IMPL + 1];

/* {@ level 1, @ level 150} */
const int thaco_mods[NUM_CLASSES][2] = {
    {184, 9},			/* Sorcerer    */
    {182, 4},			/* Cleric      */
    {181, 2},			/* Thief       */
    {180, 1},			/* Gladiator   */
    {181, 3},			/* Druid       */
    {182, 1},			/* Dark Knight */
    {182, 4},			/* Monk        */
    {182, 1},			/* Paladin     */
    {182, 2},			/* Bard        */
    {182, 2},			/* All remorts */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/*         */
    {182, 2},			/* Vamps   */
    {182, 2},			/* Titans  */
    {182, 2},			/* Saints  */
    {182, 2}			/* Demons  */
};

void load_thaco_table()
{

    int cls, lvl;

    for (cls = 0; cls < NUM_CLASSES; cls++) {
	thaco[cls][0] = 100;
	for (lvl = 1; lvl < LVL_IMMORT; lvl++)
	    thaco[cls][lvl] =
		(int) (((LVL_IMMORT - lvl) / LVL_IMMORT - 1) *
		       (thaco_mods[cls][1] - thaco_mods[cls][0])) +
		thaco_mods[cls][1];
    }
}


/* Some initializations for characters, including initial skills */
void do_start(struct char_data *ch)
{
    void advance_level(struct char_data *ch);

    GET_LEVEL(ch) = 1;
    GET_EXP(ch) = 1;

    if (!PRF_FLAGGED(ch, PRF_AUTOTITLE))
	set_title(ch, NULL);
    /*roll_real_stats(ch); */

    SET_BIT_AR(PRF_FLAGS(ch), PRF_AUTOEXIT);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPHP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMANA);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOVE);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPGOLD);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPMOBP);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPPCT);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPXTOL);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPTANK);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPVIM);
    SET_BIT_AR(PRF_FLAGS(ch), PRF_DISPARIA);

    ch->points.max_hit = 10;

    switch (GET_CLASS(ch)) {
    case CLASS_SORCERER:
	break;
    case CLASS_CLERIC:
	break;
    case CLASS_THIEF:
	break;
    case CLASS_GLADIATOR:
	break;
    case CLASS_DRUID:
	break;
    case CLASS_DARK_KNIGHT:
	break;
    case CLASS_MONK:
	break;
    case CLASS_PALADIN:
	break;
    case CLASS_BARD:
	break;
    }

    advance_level(ch);

    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
    GET_COND(ch, DRUNK) = 0;

    ch->player.time.played = 0;
    ch->player.time.logon = time(0);
}



/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */
void advance_level(struct char_data *ch)
{
    int add_hp = 0;
    int add_mana = 0;
    int add_move = 0;
    int add_qi = 0;
    int add_vim = 0;
    int add_aria = 0;
    int i;

    extern struct wis_app_type wis_app[];
    extern struct con_app_type con_app[];

    add_hp = con_app[GET_CON(ch)].hitp;

    switch (GET_CLASS(ch)) {

    case CLASS_SORCERER:
	add_hp += number(4, 10);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 10);
	add_move = number(1, (GET_DEX(ch) / 5));
	break;

    case CLASS_CLERIC:
	add_hp += number(6, 12);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 10);
	add_move = number(1, (GET_DEX(ch) / 5));
	break;

    case CLASS_THIEF:
	add_hp += number(8, 15);
	add_move = number(1, (GET_DEX(ch) / 4));
	break;

    case CLASS_GLADIATOR:
	add_hp += number(11, 17);
	add_move = number(1, (GET_DEX(ch) / 4));
	break;

    case CLASS_DRUID:
	add_hp += number(9, 15);
	add_move = number(2, 3);
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 3;
	break;

    case CLASS_DARK_KNIGHT:
	add_hp += number(10, 16);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	break;

    case CLASS_MONK:
	add_hp += number(11, 16);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_PALADIN:
	add_hp += number(10, 16);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	break;

    case CLASS_BARD:
	add_hp += number(9, 15);
	add_aria = number(GET_CHA(ch) - 1, GET_CHA(ch) + 1);
	add_move = number(1, (GET_DEX(ch) / 5));
	break;

    case CLASS_ASSASSIN:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_SHADOWMAGE:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_INQUISITOR:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

	case CLASS_MERCENARY:
	add_hp += number(6, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_mana = MIN(add_mana, 8);
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_ACROBAT:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_RANGER:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 4;
	break;

    case CLASS_WARLOCK:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_CHAMPION:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_REAPER:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_COMBAT_MASTER:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_CRUSADER:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_BEASTMASTER:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 4;
	break;

    case CLASS_ARCANIC:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_MAGI:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_DARKMAGE:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_FORESTAL:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 4;
	break;

    case CLASS_PSIONIST:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_TEMPLAR:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_STORR:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_SAGE:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_SAMURAI:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_FIANNA:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 4;
	break;

    case CLASS_SHAMAN:
	add_hp += number(6, 8);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	add_vim = number(GET_LEVEL(ch), (GET_LEVEL(ch) + 1)) >> 4;
	break;

    case CLASS_VAMPIRE:
	add_hp += number(5, 7);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_TITAN:
	add_hp += number(5, 7);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_SAINT:
	add_hp += number(5, 7);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    case CLASS_DEMON:
	add_hp += number(5, 7);
	add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
	add_mana = MIN(add_mana, 8);
	add_move = number(1, (GET_DEX(ch) / 5));
	add_qi = dice((GET_WIS(ch) / 2), 2);
	break;

    default:
        add_hp += number(5, 7);
        add_mana = number(GET_LEVEL(ch), (int) (1.5 * GET_LEVEL(ch)));
        add_mana = MIN(add_mana, 8);
        add_move = number(1, (GET_DEX(ch) / 5));
        add_qi = dice((GET_WIS(ch) / 2), 2);
        break; 
    }

    ch->points.max_hit += MAX(1, add_hp);
    ch->points.max_move += MAX(1, add_move);
    ch->points.max_qi += MAX(1, add_qi);
    ch->points.max_vim += MAX(1, add_vim);
    ch->points.max_aria += MAX(1, add_aria);
    if (GET_LEVEL(ch) > 1)
	ch->points.max_mana += add_mana;

    if (GET_CLASS(ch) <= 8 && GET_LEVEL(ch) <= 100) {
	GET_PRACTICES(ch) += MAX(1, wis_app[GET_WIS(ch)].bonus);
    } else if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
	GET_PRACTICES(ch) = GET_PRACTICES(ch);
    } else
	GET_PRACTICES(ch) += number(1, 2);

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
	for (i = 0; i < 3; i++)
	    GET_COND(ch, i) = (char) -1;
	SET_BIT_AR(PRF_FLAGS(ch), PRF_HOLYLIGHT);
    }
    GET_GOD_TYPE(ch) = 0;

    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_VIM(ch) = GET_MAX_VIM(ch);
    GET_QI(ch) = GET_MAX_QI(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    sprintf(buf, "\r\n/cw%s has gained to level %d!/c0\r\n", 
            GET_NAME(ch), GET_LEVEL(ch));
    send_to_all(buf);

    save_char(ch, NOWHERE);

    sprintf(buf, "%s advanced to level %d", GET_NAME(ch), GET_LEVEL(ch));
    mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);
}


/*
 * This simply calculates the backstab multiplier based on a character's
 * level.  This used to be an array, but was changed to be a function so
 * that it would be easier to add more levels to your MUD.  This doesn't
 * really create a big performance hit because it's not used very often.
 */
int backstab_mult(int level)
{
    if (level <= 0)
	return 3;
    else if (level <= 10)
	return 3.5;
    else if (level <= 20)
	return 4.5;
    else if (level <= 40)
	return 5.5;
    else if (level <= 80)
	return 7.5;
    else if (level <= 100)
	return 9;
    else if (level <= 120)
	return 11;
    else if (level <= 140)
	return 13;
    else if (level < LVL_IMMORT)
	return 15;		/* all remaining mortal levels */
    else
	return 20;		/* immortals */
}

int swoop_mult(int tier)
{
    if (tier < 0)
	return 1;
    else if (tier <= 2)
	return 2;
    else if (tier <= 5)
	return 3;
    else if (tier <= 10)
	return 4;
    else if (tier <= 15)
	return 6;
    else if (tier <= 20)
	return 7;
    else if (tier <= 30)
	return 8;
    else if (tier > 31)
	return 10;		/* all remaining mortal levels */
    else
	return 20;		/* immortals */
}


/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_class(struct char_data *ch, struct obj_data *obj)
{
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_SORCERER) && IS_SORCERER(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) && IS_CLERIC(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_GLADIATOR) && IS_GLADIATOR(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) && IS_THIEF(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_DRUID) && IS_DRUID(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_DARK_KNIGHT) && IS_DARK_KNIGHT(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_MONK) && IS_MONK(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_PALADIN) && IS_PALADIN(ch)))
	return 1;
    else
	return 0;
}




/*
 * SPELLS AND SKILLS.  This area defines which spells are assigned to
 * which classes, and the minimum level the character must be to use
 * the spell or skill.
 */
void init_abil_levels(void)
{
    /* SORCERER */
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_SORCERER, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_SHADOWMAGE, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_DARKMAGE, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_WARLOCK, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_ARCANIC, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_MAGI, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_FORESTAL, 1);
    abil_level(ABT_SPELL, SPELL_MAGIC_MISSILE, CLASS_PSIONIST, 1);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_SORCERER, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_SHADOWMAGE, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_DARKMAGE, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_WARLOCK, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_ARCANIC, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_MAGI, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_FORESTAL, 2);
    abil_level(ABT_SPELL, SPELL_DETECT_MAGIC, CLASS_PSIONIST, 2);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_SORCERER, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_SHADOWMAGE, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_DARKMAGE, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_WARLOCK, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_ARCANIC, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_MAGI, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_FORESTAL, 5);
    abil_level(ABT_SPELL, SPELL_CHILL_TOUCH, CLASS_PSIONIST, 5);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_SORCERER, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_SHADOWMAGE, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_DARKMAGE, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_WARLOCK, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_ARCANIC, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_MAGI, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_FORESTAL, 10);
    abil_level(ABT_SPELL, SPELL_BURNING_HANDS, CLASS_PSIONIST, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_SORCERER, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_SHADOWMAGE, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_DARKMAGE, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_WARLOCK, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_ARCANIC, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_MAGI, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_FORESTAL, 6);
    abil_level(ABT_SPELL, SPELL_DETECT_INVIS, CLASS_PSIONIST, 6);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_SORCERER, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_SHADOWMAGE, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_DARKMAGE, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_WARLOCK, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_ARCANIC, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_MAGI, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_FORESTAL, 7);
    abil_level(ABT_SPELL, SPELL_ARMOR, CLASS_PSIONIST, 7);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_SORCERER, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_SHADOWMAGE, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_WARLOCK, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_ARCANIC, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_MAGI, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_FORESTAL, 8);
    abil_level(ABT_SPELL, SPELL_FIND_FAMILIAR, CLASS_PSIONIST, 8);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_SORCERER, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_SHADOWMAGE, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_DARKMAGE, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_WARLOCK, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_ARCANIC, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_MAGI, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_FORESTAL, 9);
    abil_level(ABT_SPELL, SPELL_INVISIBLE, CLASS_PSIONIST, 9);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_SORCERER, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_SHADOWMAGE, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_DARKMAGE, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_WARLOCK, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_ARCANIC, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_MAGI, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_FORESTAL, 10);
    abil_level(ABT_SPELL, SPELL_DETECT_POISON, CLASS_PSIONIST, 10);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_SORCERER, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_SHADOWMAGE, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_DARKMAGE, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_WARLOCK, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_ARCANIC, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_MAGI, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_FORESTAL, 11);
    abil_level(ABT_SPELL, SPELL_MINOR_STRENGTH, CLASS_PSIONIST, 11);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_SORCERER, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_SHADOWMAGE, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_DARKMAGE, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_WARLOCK, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_ARCANIC, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_MAGI, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_FORESTAL, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_PSIONIST, 12);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_SORCERER, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_SHADOWMAGE, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_DARKMAGE, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_WARLOCK, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_ARCANIC, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_MAGI, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_FORESTAL, 13);
    abil_level(ABT_SPELL, SPELL_SLEEP, CLASS_PSIONIST, 13);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_SORCERER, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_SHADOWMAGE, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_DARKMAGE, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_WARLOCK, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_ARCANIC, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_MAGI, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_FORESTAL, 15);
    abil_level(ABT_SPELL, SPELL_SHOCKING_GRASP, CLASS_PSIONIST, 15);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_SORCERER, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_SHADOWMAGE, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_DARKMAGE, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_WARLOCK, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_ARCANIC, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_MAGI, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_FORESTAL, 16);
    abil_level(ABT_SPELL, SPELL_LIGHTNING_BOLT, CLASS_PSIONIST, 16);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_SORCERER, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_SHADOWMAGE, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_DARKMAGE, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_WARLOCK, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_ARCANIC, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_MAGI, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_FORESTAL, 17);
    abil_level(ABT_SPELL, SPELL_BLINDNESS, CLASS_PSIONIST, 17);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_SORCERER, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_SHADOWMAGE, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_DARKMAGE, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_WARLOCK, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_ARCANIC, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_MAGI, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_FORESTAL, 18);
    abil_level(ABT_SKILL, SKILL_BREW, CLASS_PSIONIST, 18);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SORCERER, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_DARKMAGE, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_WARLOCK, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_ARCANIC, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_MAGI, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_FORESTAL, 19);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_PSIONIST, 19);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_SORCERER, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_SHADOWMAGE, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_DARKMAGE, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_WARLOCK, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_ARCANIC, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_MAGI, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_FORESTAL, 21);
    abil_level(ABT_SPELL, SPELL_COLOR_SPRAY, CLASS_PSIONIST, 21);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_SORCERER, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_SHADOWMAGE, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_DARKMAGE, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_WARLOCK, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_ARCANIC, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_MAGI, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_FORESTAL, 23);
    abil_level(ABT_SPELL, SPELL_ENERGY_DRAIN, CLASS_PSIONIST, 23);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_SORCERER, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_SHADOWMAGE, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_DARKMAGE, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_WARLOCK, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_ARCANIC, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_MAGI, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_FORESTAL, 25);
    abil_level(ABT_SPELL, SPELL_FLY, CLASS_PSIONIST, 25);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_SORCERER, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_SHADOWMAGE, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_DARKMAGE, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_WARLOCK, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_ARCANIC, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_MAGI, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_FORESTAL, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_PSIONIST, 27);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_SORCERER, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_SHADOWMAGE, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_DARKMAGE, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_WARLOCK, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_ARCANIC, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_MAGI, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_FORESTAL, 29);
    abil_level(ABT_SPELL, SPELL_SONIC_WALL, CLASS_PSIONIST, 29);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_SORCERER, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_SHADOWMAGE, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_DARKMAGE, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_WARLOCK, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_ARCANIC, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_MAGI, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_FORESTAL, 31);
    abil_level(ABT_SPELL, SPELL_CREATESPRING, CLASS_PSIONIST, 31);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_SORCERER, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_SHADOWMAGE, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_DARKMAGE, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_WARLOCK, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_ARCANIC, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_MAGI, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_FORESTAL, 27);
    abil_level(ABT_SPELL, SPELL_CREATE_FOOD, CLASS_PSIONIST, 27);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_SORCERER, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_SHADOWMAGE, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_DARKMAGE, 37); 
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_WARLOCK, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_ARCANIC, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_MAGI, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_FORESTAL, 37);
//  abil_level(ABT_SPELL, SPELL_CHARM, CLASS_PSIONIST, 37);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SORCERER, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_DARKMAGE, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_WARLOCK, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_ARCANIC, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_MAGI, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_FORESTAL, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_PSIONIST, 39);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_SORCERER, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_SHADOWMAGE, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_DARKMAGE, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_WARLOCK, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_ARCANIC, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_MAGI, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_FORESTAL, 43);
    abil_level(ABT_SPELL, SPELL_LOCATE_OBJECT, CLASS_PSIONIST, 43);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_SORCERER, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_SHADOWMAGE, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_DARKMAGE, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_WARLOCK, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_ARCANIC, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_MAGI, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_FORESTAL, 45);
    abil_level(ABT_SPELL, SPELL_INFRAVISION, CLASS_PSIONIST, 45);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_SORCERER, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_SHADOWMAGE, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_DARKMAGE, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_WARLOCK, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_ARCANIC, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_MAGI, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_FORESTAL, 46);
    abil_level(ABT_SPELL, SPELL_ENCHANT_WEAPON, CLASS_PSIONIST, 46);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_SORCERER, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_SHADOWMAGE, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_DARKMAGE, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_WARLOCK, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_ARCANIC, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_MAGI, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_FORESTAL, 47);
    abil_level(ABT_SPELL, SPELL_FIRESHIELD, CLASS_PSIONIST, 47);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_SORCERER, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_SHADOWMAGE, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_DARKMAGE, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_WARLOCK, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_ARCANIC, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_MAGI, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_FORESTAL, 48);
    abil_level(ABT_SPELL, SPELL_STRENGTH, CLASS_PSIONIST, 48);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_SORCERER, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_SHADOWMAGE, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_DARKMAGE, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_WARLOCK, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_ARCANIC, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_MAGI, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_FORESTAL, 49);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_PSIONIST, 49);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_SHADOWMAGE, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_DARKMAGE, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_WARLOCK, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_ARCANIC, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_MAGI, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_FORESTAL, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_PSIONIST, 50);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_SORCERER, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_SHADOWMAGE, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_DARKMAGE, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_WARLOCK, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_ARCANIC, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_MAGI, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_FORESTAL, 54);
    abil_level(ABT_SPELL, SPELL_IDENTIFY, CLASS_PSIONIST, 54);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_SORCERER, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_SHADOWMAGE, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_DARKMAGE, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_WARLOCK, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_ARCANIC, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_MAGI, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_FORESTAL, 61);
    abil_level(ABT_SPELL, SPELL_STONE_HAIL, CLASS_PSIONIST, 61);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_SORCERER, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_SHADOWMAGE, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_DARKMAGE, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_WARLOCK, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_ARCANIC, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_MAGI, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_FORESTAL, 65);
    abil_level(ABT_SPELL, SPELL_HASTE, CLASS_PSIONIST, 65);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_SORCERER, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_SHADOWMAGE, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_DARKMAGE, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_WARLOCK, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_ARCANIC, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_MAGI, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_FORESTAL, 70);
    abil_level(ABT_SPELL, SPELL_DIMENSION_DOOR, CLASS_PSIONIST, 70);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_SORCERER, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_SHADOWMAGE, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_DARKMAGE, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_WARLOCK, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_ARCANIC, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_MAGI, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_FORESTAL, 71);
    abil_level(ABT_SPELL, SPELL_FLYING_FIST, CLASS_PSIONIST, 71);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_SORCERER, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_SHADOWMAGE, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_DARKMAGE, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_WARLOCK, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_ARCANIC, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_MAGI, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_FORESTAL, 65);
    abil_level(ABT_SPELL, SPELL_SUMMON, CLASS_PSIONIST, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SORCERER, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_DARKMAGE, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_WARLOCK, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_ARCANIC, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_MAGI, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_FORESTAL, 80);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_PSIONIST, 80);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_SORCERER, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_SHADOWMAGE, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_DARKMAGE, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_WARLOCK, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_ARCANIC, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_MAGI, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_FORESTAL, 81);
    abil_level(ABT_SPELL, SPELL_SHOCK_SPHERE, CLASS_PSIONIST, 81);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_SORCERER, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_SHADOWMAGE, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_DARKMAGE, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_WARLOCK, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_ARCANIC, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_MAGI, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_FORESTAL, 85);
    abil_level(ABT_SPELL, SPELL_CONFUSE, CLASS_PSIONIST, 85);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_SORCERER, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_SHADOWMAGE, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_DARKMAGE, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_WARLOCK, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_ARCANIC, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_MAGI, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_FORESTAL, 89);
    abil_level(ABT_SPELL, SPELL_SUMMON_DHALIN, CLASS_PSIONIST, 89);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_SORCERER, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_SHADOWMAGE, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_DARKMAGE, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_WARLOCK, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_ARCANIC, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_MAGI, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_FORESTAL, 90);
    abil_level(ABT_SPELL, SPELL_GROWTH, CLASS_PSIONIST, 90);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_SORCERER, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_SHADOWMAGE, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_DARKMAGE, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_WARLOCK, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_ARCANIC, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_MAGI, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_FORESTAL, 91);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_PSIONIST, 91);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_SORCERER, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_SHADOWMAGE, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_DARKMAGE, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_WARLOCK, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_ARCANIC, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_MAGI, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_FORESTAL, 100);
    abil_level(ABT_SPELL, SPELL_WITHER, CLASS_PSIONIST, 100);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_SORCERER, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_SHADOWMAGE, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_DARKMAGE, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_WARLOCK, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_ARCANIC, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_MAGI, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_FORESTAL, 101);
    abil_level(ABT_SPELL, SPELL_MAJOR_STRENGTH, CLASS_PSIONIST, 101);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_SORCERER, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_SHADOWMAGE, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_DARKMAGE, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_WARLOCK, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_ARCANIC, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_MAGI, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_FORESTAL, 102);
    abil_level(ABT_SPELL, SPELL_MANA_ECONOMY, CLASS_PSIONIST, 102);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_SORCERER, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_SHADOWMAGE, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_DARKMAGE, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_WARLOCK, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_ARCANIC, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_MAGI, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_FORESTAL, 110);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_PSIONIST, 110);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_SORCERER, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_SHADOWMAGE, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_DARKMAGE, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_WARLOCK, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_ARCANIC, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_MAGI, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_FORESTAL, 115);
    abil_level(ABT_SPELL, SPELL_SUMMON_FEHZOU, CLASS_PSIONIST, 115);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_SORCERER, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_SHADOWMAGE, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_DARKMAGE, 125);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_WARLOCK, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_ARCANIC, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_MAGI, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_FORESTAL, 120);
    abil_level(ABT_SPELL, SPELL_AGITATION, CLASS_PSIONIST, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_SORCERER, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_SHADOWMAGE, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_DARKMAGE, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_WARLOCK, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_ARCANIC, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_MAGI, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_FORESTAL, 120);
    abil_level(ABT_SPELL, SPELL_BALEFIRE, CLASS_PSIONIST, 120);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_SORCERER, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_SHADOWMAGE, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_DARKMAGE, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_WARLOCK, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_ARCANIC, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_MAGI, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_FORESTAL, 127);
    abil_level(ABT_SPELL, SPELL_MIST_FORM, CLASS_PSIONIST, 127);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_SORCERER, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_SHADOWMAGE, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_DARKMAGE, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_WARLOCK, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_ARCANIC, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_MAGI, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_FORESTAL, 130);
    abil_level(ABT_SPELL, SPELL_SCORCH, CLASS_PSIONIST, 130);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_SORCERER, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_SHADOWMAGE, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_DARKMAGE, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_WARLOCK, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_ARCANIC, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_MAGI, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_FORESTAL, 140);
    abil_level(ABT_SPELL, SPELL_IMMOLATE, CLASS_PSIONIST, 140);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_SORCERER, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_SHADOWMAGE, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_DARKMAGE, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_WARLOCK, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_ARCANIC, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_MAGI, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_FORESTAL, 135);
    abil_level(ABT_SPELL, SPELL_CONE_COLD, CLASS_PSIONIST, 135);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_SORCERER, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_SHADOWMAGE, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_DARKMAGE, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_WARLOCK, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_ARCANIC, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_MAGI, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_FORESTAL, 142);
    abil_level(ABT_SPELL, SPELL_SUMMON_NECROLIEU, CLASS_PSIONIST, 142);

    /* CLERICS */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_CLERIC, 1);
	abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_CLERIC, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_TEMPLAR, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_CRUSADER, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_ARCANIC, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_SAGE, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_STORR, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVY_SKIN, CLASS_INQUISITOR, 1);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_CLERIC, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_TEMPLAR, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_CRUSADER, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_ARCANIC, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_SAGE, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_STORR, 3);
    abil_level(ABT_PRAYER, PRAY_CURE_LIGHT, CLASS_INQUISITOR, 3);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_CLERIC, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_TEMPLAR, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_CRUSADER, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_ARCANIC, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_SAGE, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_STORR, 5);
    abil_level(ABT_PRAYER, PRAY_FOOD, CLASS_INQUISITOR, 5);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_CLERIC, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_TEMPLAR, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_CRUSADER, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_ARCANIC, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_SAGE, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_STORR, 6);
    abil_level(ABT_PRAYER, PRAY_CURE_BLIND, CLASS_INQUISITOR, 6);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_CLERIC, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_TEMPLAR, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_CRUSADER, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_ARCANIC, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_SAGE, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_STORR, 7);
    abil_level(ABT_PRAYER, PRAY_SECOND_SIGHT, CLASS_INQUISITOR, 7);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_CLERIC, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_TEMPLAR, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_CRUSADER, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_ARCANIC, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_SAGE, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_STORR, 8);
    abil_level(ABT_PRAYER, PRAY_DETECT_POISON, CLASS_INQUISITOR, 8);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_CLERIC, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_TEMPLAR, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_CRUSADER, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_ARCANIC, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SAGE, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_STORR, 9);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_CLERIC, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_TEMPLAR, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_CRUSADER, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_ARCANIC, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_SAGE, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_STORR, 10);
    abil_level(ABT_PRAYER, PRAY_REMOVE_POISON, CLASS_INQUISITOR, 10);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_CLERIC, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_TEMPLAR, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_CRUSADER, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_ARCANIC, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_SAGE, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_STORR, 11);
    abil_level(ABT_PRAYER, PRAY_WATER, CLASS_INQUISITOR, 11);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_CLERIC, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_TEMPLAR, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_CRUSADER, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_ARCANIC, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_SAGE, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_STORR, 12);
    abil_level(ABT_PRAYER, PRAY_BLESSING, CLASS_INQUISITOR, 12);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_CLERIC, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_TEMPLAR, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_CRUSADER, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_ARCANIC, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_SAGE, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_STORR, 15);
    abil_level(ABT_PRAYER, PRAY_CURE_SERIOUS, CLASS_INQUISITOR, 15);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_CLERIC, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_TEMPLAR, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_CRUSADER, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_ARCANIC, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_SAGE, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_STORR, 16);
    abil_level(ABT_PRAYER, PRAY_REFRESH, CLASS_INQUISITOR, 16);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_CLERIC, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_TEMPLAR, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_CRUSADER, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_ARCANIC, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_SAGE, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_STORR, 18);
    abil_level(ABT_PRAYER, PRAY_EARTHQUAKE, CLASS_INQUISITOR, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_CLERIC, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_TEMPLAR, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_CRUSADER, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_ARCANIC, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_SAGE, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_STORR, 18);
    abil_level(ABT_SKILL, SKILL_SCRIBE, CLASS_INQUISITOR, 18);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_CLERIC, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_TEMPLAR, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_CRUSADER, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_ARCANIC, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_SAGE, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_STORR, 22);
    abil_level(ABT_PRAYER, PRAY_LIFT_CURSE, CLASS_INQUISITOR, 22);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_CLERIC, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_TEMPLAR, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_CRUSADER, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_ARCANIC, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_SAGE, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_STORR, 24);
    abil_level(ABT_PRAYER, PRAY_HARM, CLASS_INQUISITOR, 24);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_CLERIC, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_TEMPLAR, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_CRUSADER, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_ARCANIC, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_SAGE, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_STORR, 27);
    abil_level(ABT_PRAYER, PRAY_INFRAVISION, CLASS_INQUISITOR, 27);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_CLERIC, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_TEMPLAR, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_CRUSADER, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_ARCANIC, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_SAGE, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_STORR, 29);
    abil_level(ABT_PRAYER, PRAY_BLINDNESS, CLASS_INQUISITOR, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_CLERIC, 31);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_TEMPLAR, 31);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_CRUSADER, 31);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_ARCANIC, 31);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SAGE, 31);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_STORR, 31);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_CLERIC, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_TEMPLAR, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_CRUSADER, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_ARCANIC, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_SAGE, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_STORR, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_GOOD, CLASS_INQUISITOR, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_CLERIC, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_TEMPLAR, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_CRUSADER, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_ARCANIC, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_SAGE, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_STORR, 32);
    abil_level(ABT_PRAYER, PRAY_DISPEL_EVIL, CLASS_INQUISITOR, 32);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_CLERIC, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_TEMPLAR, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_CRUSADER, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_ARCANIC, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_SAGE, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_STORR, 33);
    abil_level(ABT_PRAYER, PRAY_CURE_CRITICAL, CLASS_INQUISITOR, 33);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_CLERIC, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_TEMPLAR, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_CRUSADER, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_ARCANIC, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_SAGE, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_STORR, 35);
    abil_level(ABT_PRAYER, PRAY_CALL_LIGHTNING, CLASS_INQUISITOR, 35);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_CLERIC, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_TEMPLAR, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_CRUSADER, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_ARCANIC, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_SAGE, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_STORR, 38);
    abil_level(ABT_PRAYER, PRAY_GROUP_HEAL, CLASS_INQUISITOR, 38);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_CLERIC, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_TEMPLAR, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_CRUSADER, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_ARCANIC, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_SAGE, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_STORR, 40);
    abil_level(ABT_PRAYER, PRAY_GUIDING_LIGHT, CLASS_INQUISITOR, 40);
	abil_level(ABT_SKILL, SKILL_DODGE, CLASS_CLERIC, 43);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_TEMPLAR, 43);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_CRUSADER, 43);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_ARCANIC, 43);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_SAGE, 43);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_STORR, 43);
	abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_CLERIC, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_TEMPLAR, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_CRUSADER, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_ARCANIC, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_SAGE, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_STORR, 45);
    abil_level(ABT_PRAYER, PRAY_HASTE, CLASS_INQUISITOR, 45);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_CLERIC, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_TEMPLAR, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_CRUSADER, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_ARCANIC, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_SAGE, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_STORR, 47);
    abil_level(ABT_PRAYER, PRAY_ELEMENTAL_BURST, CLASS_INQUISITOR, 47);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_TEMPLAR, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_CRUSADER, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_SAGE, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_STORR, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_INQUISITOR, 50);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_CLERIC, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_TEMPLAR, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_CRUSADER, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_ARCANIC, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_SAGE, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_STORR, 55);
    abil_level(ABT_PRAYER, PRAY_HOLY_ARMOR, CLASS_INQUISITOR, 55);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_CLERIC, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_TEMPLAR, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_CRUSADER, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_ARCANIC, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_SAGE, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_STORR, 60);
    abil_level(ABT_PRAYER, PRAY_WINDS_PAIN, CLASS_INQUISITOR, 60);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_CLERIC, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_TEMPLAR, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_CRUSADER, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_ARCANIC, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SAGE, 65);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_STORR, 65);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_CLERIC, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_TEMPLAR, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_CRUSADER, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_ARCANIC, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_SAGE, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_STORR, 55);
    abil_level(ABT_PRAYER, PRAY_HEAL, CLASS_INQUISITOR, 55);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_CLERIC, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_TEMPLAR, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_CRUSADER, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_ARCANIC, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_SAGE, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_STORR, 70);
    abil_level(ABT_PRAYER, PRAY_PACIFY, CLASS_INQUISITOR, 70);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_CLERIC, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_TEMPLAR, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_CRUSADER, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_ARCANIC, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_SAGE, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_STORR, 72);
    abil_level(ABT_PRAYER, PRAY_SACRED_SHIELD, CLASS_INQUISITOR, 72);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_CLERIC, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_TEMPLAR, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_CRUSADER, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_ARCANIC, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_SAGE, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_STORR, 75);
    abil_level(ABT_PRAYER, PRAY_SUMMON, CLASS_INQUISITOR, 75);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_CLERIC, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_TEMPLAR, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_CRUSADER, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_ARCANIC, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_SAGE, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_STORR, 77);
    abil_level(ABT_PRAYER, PRAY_HAND_BALANCE, CLASS_INQUISITOR, 77);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_CLERIC, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_TEMPLAR, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_CRUSADER, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_ARCANIC, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_SAGE, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_STORR, 85);
    abil_level(ABT_PRAYER, PRAY_DEFLECT, CLASS_INQUISITOR, 85);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_CLERIC, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_TEMPLAR, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_CRUSADER, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_ARCANIC, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_SAGE, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_STORR, 90);
    abil_level(ABT_PRAYER, PRAY_SPIRIT_STRIKE, CLASS_INQUISITOR, 90);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_CLERIC, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_TEMPLAR, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_CRUSADER, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_ARCANIC, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_SAGE, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_STORR, 100);
    abil_level(ABT_PRAYER, PRAY_ABOMINATION, CLASS_INQUISITOR, 100);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_CLERIC, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_TEMPLAR, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_CRUSADER, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_ARCANIC, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_SAGE, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_STORR, 105);
    abil_level(ABT_PRAYER, PRAY_SANCTUARY, CLASS_INQUISITOR, 105);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_CLERIC, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_TEMPLAR, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_CRUSADER, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_ARCANIC, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_SAGE, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_STORR, 110);
    abil_level(ABT_PRAYER, PRAY_WINDS_RECKONING, CLASS_INQUISITOR, 110);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_CLERIC, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_TEMPLAR, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_CRUSADER, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_ARCANIC, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SAGE, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_STORR, 115);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_INQUISITOR, 115);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_CLERIC, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_TEMPLAR, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_CRUSADER, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_ARCANIC, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_SAGE, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_STORR, 117);
    abil_level(ABT_PRAYER, PRAY_DROWSE, CLASS_INQUISITOR, 117);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_CLERIC, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_TEMPLAR, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_CRUSADER, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_ARCANIC, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_SAGE, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_STORR, 120);
    abil_level(ABT_PRAYER, PRAY_GODS_FURY, CLASS_INQUISITOR, 120);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_CLERIC, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_TEMPLAR, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_CRUSADER, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_ARCANIC, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_SAGE, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_STORR, 125);
    abil_level(ABT_PRAYER, PRAY_VITALITY, CLASS_INQUISITOR, 125);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_CLERIC, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_TEMPLAR, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_CRUSADER, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_ARCANIC, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_SAGE, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_STORR, 130);
    abil_level(ABT_PRAYER, PRAY_ANGEL_BREATH, CLASS_INQUISITOR, 130);
/*  REMOVED UNTIL COMPLETE FIX INSTALLED 12-12-01 ******************
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_CLERIC, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_TEMPLAR, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_CRUSADER, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_ARCANIC, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_SAGE, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_STORR, 135);
    abil_level(ABT_PRAYER, PRAY_RETRIEVE_CORPSE, CLASS_INQUISITOR, 135);
*********************************************************************/
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_CLERIC, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_TEMPLAR, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_CRUSADER, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_ARCANIC, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_SAGE, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_STORR, 140);
    abil_level(ABT_PRAYER, PRAY_PLAGUE, CLASS_INQUISITOR, 140);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_CLERIC, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_TEMPLAR, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_CRUSADER, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_ARCANIC, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_SAGE, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_STORR, 150);
    abil_level(ABT_PRAYER, PRAY_SOUL_SCOURGE, CLASS_INQUISITOR, 150);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_CLERIC, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_TEMPLAR, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_CRUSADER, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_ARCANIC, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_SAGE, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_STORR, 135);
    abil_level(ABT_PRAYER, PRAY_WEAKENED_FLESH, CLASS_INQUISITOR, 135);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_CLERIC, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_TEMPLAR, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_CRUSADER, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_ARCANIC, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_SAGE, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_STORR, 130);
    abil_level(ABT_PRAYER, PRAY_REVERSE_ALIGN, CLASS_INQUISITOR, 130);

    /* THIEVES */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_THIEF, 1);
    abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_THIEF, 1);
    abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_ASSASSIN, 1);
    abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_SHADOWMAGE, 1);
    abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_ACROBAT, 1);
    abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_RANGER, 1);
	abil_level(ABT_SKILL, SKILL_SNEAK, CLASS_MERCENARY, 1);
    abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_THIEF, 2);
    abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_ASSASSIN, 2);
    abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_SHADOWMAGE, 2);
    abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_ACROBAT, 2);
    abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_RANGER, 2);
	abil_level(ABT_SKILL, SKILL_TRICKPUNCH, CLASS_MERCENARY, 2);
    abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_THIEF, 3);
    abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_ASSASSIN, 3);
    abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_SHADOWMAGE, 3);
    abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_ACROBAT, 3);
    abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_RANGER, 3);
	abil_level(ABT_SKILL, SKILL_PICK_LOCK, CLASS_MERCENARY, 3);
    abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_THIEF, 5);
    abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_ASSASSIN, 5);
    abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_SHADOWMAGE, 5);
    abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_ACROBAT, 5);
    abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_RANGER, 5);
	abil_level(ABT_SKILL, SKILL_BACKSTAB, CLASS_MERCENARY, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_THIEF, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_ASSASSIN, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SHADOWMAGE, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_ACROBAT, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_RANGER, 7);
	abil_level(ABT_SKILL, SKILL_TRACK, CLASS_MERCENARY, 7);
    abil_level(ABT_SKILL, SKILL_STEAL, CLASS_THIEF, 9);
    abil_level(ABT_SKILL, SKILL_STEAL, CLASS_ASSASSIN, 9);
    abil_level(ABT_SKILL, SKILL_STEAL, CLASS_SHADOWMAGE, 9);
    abil_level(ABT_SKILL, SKILL_STEAL, CLASS_ACROBAT, 9);
    abil_level(ABT_SKILL, SKILL_STEAL, CLASS_RANGER, 9);
	abil_level(ABT_SKILL, SKILL_STEAL, CLASS_MERCENARY, 9);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_THIEF, 11);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_ASSASSIN, 11);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_SHADOWMAGE, 11);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_ACROBAT, 11);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_RANGER, 11);
	abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_MERCENARY, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_THIEF, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_ASSASSIN, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SHADOWMAGE, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_ACROBAT, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_RANGER, 13);
    abil_level(ABT_SKILL, SKILL_HIDE, CLASS_THIEF, 15);
    abil_level(ABT_SKILL, SKILL_HIDE, CLASS_ASSASSIN, 15);
    abil_level(ABT_SKILL, SKILL_HIDE, CLASS_SHADOWMAGE, 15);
    abil_level(ABT_SKILL, SKILL_HIDE, CLASS_ACROBAT, 15);
    abil_level(ABT_SKILL, SKILL_HIDE, CLASS_RANGER, 15);
	abil_level(ABT_SKILL, SKILL_HIDE, CLASS_MERCENARY, 15);
    abil_level(ABT_SKILL, SKILL_SPY, CLASS_THIEF, 17);
    abil_level(ABT_SKILL, SKILL_SPY, CLASS_ASSASSIN, 17);
    abil_level(ABT_SKILL, SKILL_SPY, CLASS_SHADOWMAGE, 17);
    abil_level(ABT_SKILL, SKILL_SPY, CLASS_ACROBAT, 17);
    abil_level(ABT_SKILL, SKILL_SPY, CLASS_RANGER, 17);
	abil_level(ABT_SKILL, SKILL_SPY, CLASS_MERCENARY, 17);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_THIEF, 19);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_ASSASSIN, 19);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_SHADOWMAGE, 19);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_ACROBAT, 19);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_RANGER, 19);
	abil_level(ABT_SKILL, SKILL_DODGE, CLASS_MERCENARY, 19);
    abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_THIEF, 21);
    abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_ASSASSIN, 21);
    abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_SHADOWMAGE, 21);
    abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_ACROBAT, 21);
    abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_RANGER, 21);
	abil_level(ABT_SKILL, SKILL_EYEGOUGE, CLASS_MERCENARY, 21);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_THIEF, 23);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_ASSASSIN, 23);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_SHADOWMAGE, 23);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_ACROBAT, 23);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_RANGER, 23);
	abil_level(ABT_SKILL, SKILL_TRIP, CLASS_MERCENARY, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_THIEF, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_ASSASSIN, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_SHADOWMAGE, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_ACROBAT, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_RANGER, 25);
	abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_MERCENARY, 25);
    abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_THIEF, 27);
    abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_ASSASSIN, 27);
    abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_SHADOWMAGE, 27);
    abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_ACROBAT, 27);
    abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_RANGER, 27);
	abil_level(ABT_SKILL, SKILL_INCOGNITO, CLASS_MERCENARY, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_THIEF, 29);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_ASSASSIN, 29);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_SHADOWMAGE, 29);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_ACROBAT, 29);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_RANGER, 29);
	abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_MERCENARY, 29);
    abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_THIEF, 33);
    abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_ASSASSIN, 33);
    abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_SHADOWMAGE, 33);
    abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_ACROBAT, 33);
    abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_RANGER, 33);
	abil_level(ABT_SKILL, SKILL_GROINKICK, CLASS_MERCENARY, 33);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_THIEF, 35);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SHADOWMAGE, 35);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_ACROBAT, 35);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_RANGER, 35);
    abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_THIEF, 36);
    abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_ASSASSIN, 36);
    abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_SHADOWMAGE, 36);
    abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_ACROBAT, 36);
    abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_RANGER, 36);
	abil_level(ABT_SKILL, SKILL_GAUGE, CLASS_MERCENARY, 36);
    abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_THIEF, 41);
    abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_ASSASSIN, 41);
    abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_SHADOWMAGE, 41);
    abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_ACROBAT, 41);
    abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_RANGER, 41);
	abil_level(ABT_SKILL, SKILL_BRAIN, CLASS_MERCENARY, 41);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_THIEF, 47);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SHADOWMAGE, 47);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_ACROBAT, 47);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_RANGER, 47);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_ASSASSIN, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_RANGER, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_ACROBAT, 50);
    abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_THIEF, 53);
    abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_SHADOWMAGE, 53);
    abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_ASSASSIN, 53);
    abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_RANGER, 53);
    abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_ACROBAT, 53);
	abil_level(ABT_SKILL, SKILL_CLIMB, CLASS_MERCENARY, 53);
    abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_THIEF, 55);
    abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_ASSASSIN, 55);
    abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_SHADOWMAGE, 55);
    abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_ACROBAT, 55);
    abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_RANGER, 55);
	abil_level(ABT_SKILL, SKILL_LOWBLOW, CLASS_MERCENARY, 55);
    abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_THIEF, 65);
    abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_ASSASSIN, 65);
    abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_SHADOWMAGE, 65);
    abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_ACROBAT, 65);
    abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_RANGER, 65);
	abil_level(ABT_SKILL, SKILL_SLEIGHT, CLASS_MERCENARY, 65);
    abil_level(ABT_SKILL, SKILL_BEG, CLASS_THIEF, 1);
    abil_level(ABT_SKILL, SKILL_BEG, CLASS_SHADOWMAGE, 1);
    abil_level(ABT_SKILL, SKILL_BEG, CLASS_ACROBAT, 1);
    abil_level(ABT_SKILL, SKILL_BEG, CLASS_RANGER, 1);
	abil_level(ABT_SKILL, SKILL_BEG, CLASS_MERCENARY, 1);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_THIEF, 75);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SHADOWMAGE, 75);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_ACROBAT, 75);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_RANGER, 75);
    abil_level(ABT_SKILL, SKILL_KNEE, CLASS_THIEF, 90);
    abil_level(ABT_SKILL, SKILL_KNEE, CLASS_ASSASSIN, 90);
    abil_level(ABT_SKILL, SKILL_KNEE, CLASS_SHADOWMAGE, 90);
    abil_level(ABT_SKILL, SKILL_KNEE, CLASS_ACROBAT, 90);
    abil_level(ABT_SKILL, SKILL_KNEE, CLASS_RANGER, 90);
	abil_level(ABT_SKILL, SKILL_KNEE, CLASS_MERCENARY, 90);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_THIEF, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_SHADOWMAGE, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_ACROBAT, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_RANGER, 100);
    abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_THIEF, 105);
    abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_ASSASSIN, 105);
    abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_SHADOWMAGE, 105);
    abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_ACROBAT, 105);
    abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_RANGER, 105);
	abil_level(ABT_SKILL, SKILL_TAINT_FLUID, CLASS_MERCENARY, 105);
    abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_THIEF, 115);
    abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_ASSASSIN, 115);
    abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_SHADOWMAGE, 115);
    abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_ACROBAT, 115);
    abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_RANGER, 115);
	abil_level(ABT_SKILL, SKILL_PSYCHOTIC_STABBING, CLASS_MERCENARY, 115);
    abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_THIEF, 120);
    abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_ASSASSIN, 50);
    abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_SHADOWMAGE, 50);
    abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_ACROBAT, 50);
    abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_RANGER, 50);
	abil_level(ABT_SKILL, SKILL_HAGGLE, CLASS_MERCENARY, 50);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_THIEF, 130);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SHADOWMAGE, 130);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_ACROBAT, 130);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_RANGER, 130);
    abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_THIEF, 140);
    abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_SHADOWMAGE, 140);
    abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_ACROBAT, 140);
    abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_RANGER, 140);
    abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_ASSASSIN, 140);
	abil_level(ABT_SKILL, SKILL_SECOND_BSTAB, CLASS_MERCENARY, 140);
    abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_THIEF, 145);
    abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_SHADOWMAGE, 145);
    abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_ACROBAT, 145);
    abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_RANGER, 145);
    abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_ASSASSIN, 145);
	abil_level(ABT_SKILL, SKILL_THIRD_BSTAB, CLASS_MERCENARY, 145);
    abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_THIEF, 150);
    abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_SHADOWMAGE, 150);
    abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_ACROBAT, 150);
    abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_RANGER, 150);
    abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_ASSASSIN, 150);
	abil_level(ABT_SKILL, SKILL_FOURTH_BSTAB, CLASS_MERCENARY, 150);

    /* GLADIATORS */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_GLADIATOR, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_GLADIATOR, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_WARLOCK, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_CHAMPION, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_REAPER, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_COMBAT_MASTER, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_CRUSADER, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_BEASTMASTER, 1);
	abil_level(ABT_SKILL, SKILL_KICK, CLASS_MERCENARY, 1);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_GLADIATOR, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_WARLOCK, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_CHAMPION, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_REAPER, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_COMBAT_MASTER, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_CRUSADER, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_BEASTMASTER, 3);
	abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_MERCENARY, 3);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_GLADIATOR, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_WARLOCK, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_CHAMPION, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_REAPER, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_COMBAT_MASTER, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_CRUSADER, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_BEASTMASTER, 5);
	abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_MERCENARY, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_GLADIATOR, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_WARLOCK, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_CHAMPION, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_REAPER, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_COMBAT_MASTER, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_CRUSADER, 7);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_BEASTMASTER, 7);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_GLADIATOR, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_WARLOCK, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_CHAMPION, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_REAPER, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_COMBAT_MASTER, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_CRUSADER, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_BEASTMASTER, 9);
	abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_MERCENARY, 9);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_GLADIATOR, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_WARLOCK, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_CHAMPION, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_REAPER, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_COMBAT_MASTER, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_CRUSADER, 10);
    abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_BEASTMASTER, 10);
	abil_level(ABT_SKILL, SKILL_DOORBASH, CLASS_MERCENARY, 10);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_GLADIATOR, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_WARLOCK, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_CHAMPION, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_REAPER, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_COMBAT_MASTER, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_CRUSADER, 11);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_BEASTMASTER, 11);
	abil_level(ABT_SKILL, SKILL_BASH, CLASS_MERCENARY, 11);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_GLADIATOR, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_WARLOCK, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_CHAMPION, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_REAPER, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_COMBAT_MASTER, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_CRUSADER, 13);
    abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_BEASTMASTER, 13);
	abil_level(ABT_SKILL, SKILL_DEATHBLOW, CLASS_MERCENARY, 13);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_GLADIATOR, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_WARLOCK, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_CHAMPION, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_REAPER, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_COMBAT_MASTER, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_CRUSADER, 15);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_BEASTMASTER, 15);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_GLADIATOR, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_WARLOCK, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_CHAMPION, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_REAPER, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_COMBAT_MASTER, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_CRUSADER, 17);
    abil_level(ABT_SKILL, SKILL_DISARM, CLASS_BEASTMASTER, 17);
	abil_level(ABT_SKILL, SKILL_DISARM, CLASS_MERCENARY, 17);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_GLADIATOR, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_WARLOCK, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_CHAMPION, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_REAPER, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_COMBAT_MASTER, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_CRUSADER, 19);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_BEASTMASTER, 19);
	abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_MERCENARY, 19);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_GLADIATOR, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_WARLOCK, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_CHAMPION, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_REAPER, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_COMBAT_MASTER, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_CRUSADER, 20);
    abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_BEASTMASTER, 20);
	abil_level(ABT_SKILL, SKILL_HEADBUT, CLASS_MERCENARY, 20);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_GLADIATOR, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_WARLOCK, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_CHAMPION, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_REAPER, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_COMBAT_MASTER, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_CRUSADER, 25);
    abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_BEASTMASTER, 25);
	abil_level(ABT_SKILL, SKILL_BERSERK, CLASS_MERCENARY, 25);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_GLADIATOR, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_WARLOCK, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_CHAMPION, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_REAPER, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_COMBAT_MASTER, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_CRUSADER, 27);
    abil_level(ABT_SKILL, SKILL_SWEEP, CLASS_BEASTMASTER, 27);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_GLADIATOR, 30);	/* Vedic 7/6/99 */
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_WARLOCK, 30);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_CHAMPION, 30);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_REAPER, 30);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_COMBAT_MASTER, 30);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_CRUSADER, 30);
    abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_BEASTMASTER, 30);
	abil_level(ABT_SKILL, SKILL_KNOCKOUT, CLASS_MERCENARY, 30);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_GLADIATOR, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_WARLOCK, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_CHAMPION, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_REAPER, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_COMBAT_MASTER, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_CRUSADER, 35);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_BEASTMASTER, 35);
	abil_level(ABT_SKILL, SKILL_STUN, CLASS_MERCENARY, 35);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_GLADIATOR, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_WARLOCK, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_CHAMPION, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_REAPER, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_COMBAT_MASTER, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_CRUSADER, 40);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_BEASTMASTER, 40);
	abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_MERCENARY, 40);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_GLADIATOR, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_WARLOCK, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_CHAMPION, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_REAPER, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_COMBAT_MASTER, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_CRUSADER, 45);
    abil_level(ABT_SKILL, SKILL_TURN, CLASS_BEASTMASTER, 45);
	abil_level(ABT_SKILL, SKILL_TURN, CLASS_MERCENARY, 45);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_GLADIATOR, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_WARLOCK, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_CHAMPION, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_REAPER, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_COMBAT_MASTER, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_CRUSADER, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_BEASTMASTER, 50);
	abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_MERCENARY, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_CHAMPION, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_REAPER, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_COMBAT_MASTER, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_BEASTMASTER, 50);
	abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_MERCENARY, 50);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_GLADIATOR, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_WARLOCK, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_CHAMPION, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_REAPER, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_COMBAT_MASTER, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_CRUSADER, 55);
    abil_level(ABT_SKILL, SKILL_HONE, CLASS_BEASTMASTER, 55);
	abil_level(ABT_SKILL, SKILL_HONE, CLASS_MERCENARY, 55);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_GLADIATOR, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_WARLOCK, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_CHAMPION, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_REAPER, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_COMBAT_MASTER, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_CRUSADER, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_BEASTMASTER, 70);
	abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_MERCENARY, 70);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_GLADIATOR, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_WARLOCK, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_CHAMPION, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_REAPER, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_COMBAT_MASTER, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_CRUSADER, 71);
    abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_BEASTMASTER, 71);
	abil_level(ABT_SKILL, SKILL_BEARHUG, CLASS_MERCENARY, 71);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_GLADIATOR, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_WARLOCK, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_CHAMPION, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_REAPER, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_COMBAT_MASTER, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_CRUSADER, 80);
    abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_BEASTMASTER, 80);
	abil_level(ABT_SKILL, SKILL_BODYSLAM, CLASS_MERCENARY, 80);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_GLADIATOR, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_WARLOCK, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_CHAMPION, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_REAPER, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_COMBAT_MASTER, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_CRUSADER, 90);
    abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_BEASTMASTER, 90);
	abil_level(ABT_SKILL, SKILL_CRITICAL_HIT, CLASS_MERCENARY, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_GLADIATOR, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_WARLOCK, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_CHAMPION, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_REAPER, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_COMBAT_MASTER, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_CRUSADER, 90);
    abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_BEASTMASTER, 90);
	abil_level(ABT_SKILL, SKILL_QUICKDRAW, CLASS_MERCENARY, 90);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_GLADIATOR, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_WARLOCK, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_CHAMPION, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_REAPER, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_COMBAT_MASTER, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_CRUSADER, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_BEASTMASTER, 100);
	abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_MERCENARY, 100);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_GLADIATOR, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_WARLOCK, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_CHAMPION, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_REAPER, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_COMBAT_MASTER, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_CRUSADER, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_BEASTMASTER, 105);
	abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_MERCENARY, 105);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_GLADIATOR, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_WARLOCK, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_CHAMPION, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_REAPER, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_COMBAT_MASTER, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_CRUSADER, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_BEASTMASTER, 125);
	abil_level(ABT_SKILL, SKILL_PUSH, CLASS_MERCENARY, 125);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_GLADIATOR, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_WARLOCK, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_CHAMPION, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_REAPER, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_COMBAT_MASTER, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_CRUSADER, 150);
    abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_BEASTMASTER, 150);
	abil_level(ABT_SKILL, SKILL_SEVENTH_ATTACK, CLASS_MERCENARY, 150);


    /* DRUIDS */
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_DRUID, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_RANGER, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_FORESTAL, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_BEASTMASTER, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_SAGE, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_SHAMAN, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_FIANNA, 1);
	abil_level(ABT_SKILL, SKILL_FOG, CLASS_DRUID, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_RANGER, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_FORESTAL, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_BEASTMASTER, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_SAGE, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_SHAMAN, 3);
    abil_level(ABT_SKILL, SKILL_FOG, CLASS_FIANNA, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_DRUID, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_RANGER, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_FORESTAL, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_BEASTMASTER, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_SAGE, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_SHAMAN, 3);
    abil_level(ABT_SKILL, SKILL_FOG_MIST_AURA, CLASS_FIANNA, 3);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_DRUID, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_RANGER, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_FORESTAL, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_BEASTMASTER, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SAGE, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SHAMAN, 5);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_FIANNA, 5);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_DRUID, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_RANGER, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_FORESTAL, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_BEASTMASTER, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_SAGE, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_SHAMAN, 6);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DEARTH, CLASS_FIANNA, 6);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_DRUID, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_RANGER, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_FORESTAL, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_BEASTMASTER, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_SAGE, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_SHAMAN, 7);
    abil_level(ABT_SKILL, SKILL_FORAGE, CLASS_FIANNA, 7);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_DRUID, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_RANGER, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_FORESTAL, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_BEASTMASTER, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_SAGE, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_SHAMAN, 9);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_FIANNA, 9);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_DRUID, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_RANGER, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_FORESTAL, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_BEASTMASTER, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_SAGE, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_SHAMAN, 10);
    abil_level(ABT_SKILL, SKILL_BEFRIEND, CLASS_FIANNA, 10);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_DRUID, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_RANGER, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_FORESTAL, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_BEASTMASTER, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SAGE, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SHAMAN, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_FIANNA, 11);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_DRUID, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_RANGER, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_FORESTAL, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_BEASTMASTER, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_SAGE, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_SHAMAN, 12);
    abil_level(ABT_SKILL, SKILL_CIRCLE, CLASS_FIANNA, 12);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_DRUID, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_RANGER, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_FORESTAL, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_BEASTMASTER, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_SAGE, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_SHAMAN, 13);
    abil_level(ABT_SKILL, SKILL_BANDAGE, CLASS_FIANNA, 13);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_DRUID, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_RANGER, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_FORESTAL, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_BEASTMASTER, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_SAGE, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_SHAMAN, 14);
    abil_level(ABT_SPELL, SPELL_TREEWALK_GLANDS, CLASS_FIANNA, 14);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_DRUID, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_RANGER, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_FORESTAL, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_BEASTMASTER, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_SAGE, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_SHAMAN, 15);
    abil_level(ABT_SKILL, SKILL_DODGE, CLASS_FIANNA, 15);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_DRUID, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_RANGER, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_FORESTAL, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_BEASTMASTER, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_SAGE, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_SHAMAN, 16);
    abil_level(ABT_SPELL, SPELL_BLUR, CLASS_FIANNA, 16);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_DRUID, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_RANGER, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_FORESTAL, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_BEASTMASTER, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_SAGE, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_SHAMAN, 17);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_FIANNA, 17);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_DRUID, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_RANGER, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_FORESTAL, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_BEASTMASTER, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_SAGE, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_SHAMAN, 19);
    abil_level(ABT_SKILL, SKILL_RUN, CLASS_FIANNA, 19);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_DRUID, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_RANGER, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_FORESTAL, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_BEASTMASTER, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_SAGE, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_SHAMAN, 20);
    abil_level(ABT_SPELL, SPELL_TREEWALK_TWOOD, CLASS_FIANNA, 20);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_DRUID, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_RANGER, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_FORESTAL, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_BEASTMASTER, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_SAGE, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_SHAMAN, 21);
    abil_level(ABT_SKILL, SKILL_MOTHEREARTH, CLASS_FIANNA, 21);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_DRUID, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_RANGER, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_FORESTAL, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_BEASTMASTER, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_SAGE, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_SHAMAN, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_FIANNA, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_DRUID, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_RANGER, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_FORESTAL, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_BEASTMASTER, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SAGE, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SHAMAN, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_FIANNA, 29);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_DRUID, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_RANGER, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_FORESTAL, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_BEASTMASTER, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_SAGE, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_SHAMAN, 30);
    abil_level(ABT_SPELL, SPELL_TREEWALK_VALLEY, CLASS_FIANNA, 30);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_DRUID, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_RANGER, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_FORESTAL, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_BEASTMASTER, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_SAGE, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_SHAMAN, 31);
    abil_level(ABT_SKILL, SKILL_CALL_WILD, CLASS_FIANNA, 31);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_DRUID, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_RANGER, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_FORESTAL, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_BEASTMASTER, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_SAGE, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_SHAMAN, 33);
    abil_level(ABT_SPELL, SPELL_EARTHQUAKE, CLASS_FIANNA, 33);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_DRUID, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_RANGER, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_FORESTAL, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_BEASTMASTER, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_SAGE, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_SHAMAN, 35);
    abil_level(ABT_SKILL, SKILL_SWARM, CLASS_FIANNA, 35);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_DRUID, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_RANGER, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_FORESTAL, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_BEASTMASTER, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_SAGE, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_SHAMAN, 43);
    abil_level(ABT_SPELL, SPELL_TREEWALK_DWOOD, CLASS_FIANNA, 43);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_DRUID, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_RANGER, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_FORESTAL, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_BEASTMASTER, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_SAGE, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_SHAMAN, 45);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_FIANNA, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_DRUID, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_RANGER, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_FORESTAL, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_BEASTMASTER, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SAGE, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SHAMAN, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_FIANNA, 49);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_DRUID, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_RANGER, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_FORESTAL, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_BEASTMASTER, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_SAGE, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_SHAMAN, 50);
    abil_level(ABT_SKILL, SKILL_TANGLEWEED, CLASS_FIANNA, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_SHAMAN, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_FIANNA, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_RANGER, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_FORESTAL, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_BEASTMASTER, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_SAGE, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_SHAMAN, 50);
    abil_level(ABT_SPELL, SPELL_CLEAR_SKY, CLASS_FIANNA, 50);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_DRUID, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_RANGER, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_FORESTAL, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_BEASTMASTER, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_SAGE, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_SHAMAN, 55);
    abil_level(ABT_SKILL, SKILL_AURA_OF_NATURE, CLASS_FIANNA, 55);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_DRUID, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_RANGER, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_FORESTAL, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_BEASTMASTER, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_SAGE, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_SHAMAN, 59);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SINISTER, CLASS_FIANNA, 59);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_DRUID, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_RANGER, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_FORESTAL, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_BEASTMASTER, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_SAGE, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_SHAMAN, 60);
    abil_level(ABT_SKILL, SKILL_ELEMENTAL, CLASS_FIANNA, 60);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_DRUID, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_RANGER, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_FORESTAL, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_BEASTMASTER, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_SAGE, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_SHAMAN, 65);
    abil_level(ABT_SKILL, SKILL_MIX, CLASS_FIANNA, 65);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_DRUID, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_RANGER, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_FORESTAL, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_BEASTMASTER, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_SAGE, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_SHAMAN, 69);
    abil_level(ABT_SPELL, SPELL_TREEWALK_HERMIT, CLASS_FIANNA, 69);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_DRUID, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_RANGER, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_FORESTAL, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_BEASTMASTER, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_SAGE, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_SHAMAN, 70);
    abil_level(ABT_SKILL, SKILL_MONSOON, CLASS_FIANNA, 70);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_DRUID, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_RANGER, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_FORESTAL, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_BEASTMASTER, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_SAGE, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_SHAMAN, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_FIANNA, 75);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_DRUID, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_RANGER, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_FORESTAL, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_BEASTMASTER, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_SAGE, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_SHAMAN, 79);
    abil_level(ABT_SPELL, SPELL_HEAL, CLASS_FIANNA, 79);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_DRUID, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_RANGER, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_FORESTAL, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_BEASTMASTER, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_SAGE, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_SHAMAN, 80);
    abil_level(ABT_SPELL, SPELL_TREEWALK_CMIR, CLASS_FIANNA, 80);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_DRUID, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_RANGER, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_FORESTAL, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_BEASTMASTER, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SAGE, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SHAMAN, 85);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_FIANNA, 85);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_DRUID, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_RANGER, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_FORESTAL, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_BEASTMASTER, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_SAGE, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_SHAMAN, 90);
    abil_level(ABT_SPELL, SPELL_SENSE_SPAWN, CLASS_FIANNA, 90);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_DRUID, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_RANGER, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_FORESTAL, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_BEASTMASTER, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_SAGE, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_SHAMAN, 99);
    abil_level(ABT_SPELL, SPELL_CAUSTIC_RAIN, CLASS_FIANNA, 99);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_DRUID, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_RANGER, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_FORESTAL, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_BEASTMASTER, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_SAGE, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_SHAMAN, 90);
    abil_level(ABT_SPELL, SPELL_TREEWALK_THEW, CLASS_FIANNA, 90);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_DRUID, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_RANGER, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_FORESTAL, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_BEASTMASTER, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_SAGE, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_SHAMAN, 100);
    abil_level(ABT_SKILL, SKILL_FOG_MIND,	   CLASS_FIANNA, 100);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_DRUID, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_RANGER, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_FORESTAL, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_BEASTMASTER, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_SAGE, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_SHAMAN, 110);
    abil_level(ABT_SPELL, SPELL_SNARE, CLASS_FIANNA, 110);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_DRUID, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_RANGER, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_FORESTAL, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_BEASTMASTER, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_SAGE, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_SHAMAN, 115);
    abil_level(ABT_SPELL, SPELL_TREEWALK_KAI, CLASS_FIANNA, 115);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_DRUID, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_RANGER, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_FORESTAL, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_BEASTMASTER, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_SAGE, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_SHAMAN, 125);
    abil_level(ABT_SPELL, SPELL_METEOR_SWARM, CLASS_FIANNA, 125);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_DRUID, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_RANGER, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_FORESTAL, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_BEASTMASTER, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_SAGE, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_SHAMAN, 130);
    abil_level(ABT_SPELL, SPELL_TREEWALK_SISTER, CLASS_FIANNA, 130);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_DRUID, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_RANGER, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_FORESTAL, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_BEASTMASTER, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SAGE, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SHAMAN, 140);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_FIANNA, 140);

    /* DEATH KNIGHTS */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_DARK_KNIGHT, 1); 
    abil_level(ABT_SPELL, SPELL_CAUSE_LIGHT, CLASS_DARK_KNIGHT, 1);
    abil_level(ABT_SPELL, SPELL_CAUSE_LIGHT, CLASS_REAPER, 1);
    abil_level(ABT_SPELL, SPELL_CAUSE_LIGHT, CLASS_DARKMAGE, 1);
    abil_level(ABT_SPELL, SPELL_CAUSE_LIGHT, CLASS_ASSASSIN, 1);
    abil_level(ABT_SPELL, SPELL_CAUSE_LIGHT, CLASS_INQUISITOR, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_DARK_KNIGHT, 3);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_REAPER, 3);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_DARKMAGE, 3);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_ASSASSIN, 3);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_INQUISITOR, 3);
    abil_level(ABT_SPELL, SPELL_PROTFROMGOOD, CLASS_DARK_KNIGHT, 5);
    abil_level(ABT_SPELL, SPELL_PROTFROMGOOD, CLASS_REAPER, 5);
    abil_level(ABT_SPELL, SPELL_PROTFROMGOOD, CLASS_DARKMAGE, 5);
    abil_level(ABT_SPELL, SPELL_PROTFROMGOOD, CLASS_ASSASSIN, 5);
    abil_level(ABT_SPELL, SPELL_PROTFROMGOOD, CLASS_INQUISITOR, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_DARK_KNIGHT, 7);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_REAPER, 7);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_DARKMAGE, 7);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_ASSASSIN, 7);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_INQUISITOR, 7);
    abil_level(ABT_SPELL, SPELL_POISON, CLASS_DARK_KNIGHT, 9);
    abil_level(ABT_SPELL, SPELL_POISON, CLASS_REAPER, 9);
    abil_level(ABT_SPELL, SPELL_POISON, CLASS_DARKMAGE, 9);
    abil_level(ABT_SPELL, SPELL_POISON, CLASS_ASSASSIN, 9);
    abil_level(ABT_SPELL, SPELL_POISON, CLASS_INQUISITOR, 9);
    abil_level(ABT_SPELL, SPELL_BLACKDART, CLASS_DARK_KNIGHT, 11);
    abil_level(ABT_SPELL, SPELL_BLACKDART, CLASS_REAPER, 11);
    abil_level(ABT_SPELL, SPELL_BLACKDART, CLASS_DARKMAGE, 11);
    abil_level(ABT_SPELL, SPELL_BLACKDART, CLASS_ASSASSIN, 11);
    abil_level(ABT_SPELL, SPELL_BLACKDART, CLASS_INQUISITOR, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_DARK_KNIGHT, 12);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_REAPER, 12);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_DARKMAGE, 12);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_INQUISITOR, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_DARK_KNIGHT, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_REAPER, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_ASSASSIN, 12);
    abil_level(ABT_SPELL, SPELL_INVIS_STALKER, CLASS_INQUISITOR, 12);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_DARK_KNIGHT, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_REAPER, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_DARKMAGE, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_INQUISITOR, 13);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_DARK_KNIGHT, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_REAPER, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_DARKMAGE, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_ASSASSIN, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_INQUISITOR, 14);
    abil_level(ABT_SPELL, SPELL_CAUSE_CRITIC, CLASS_DARK_KNIGHT, 15);
    abil_level(ABT_SPELL, SPELL_CAUSE_CRITIC, CLASS_REAPER, 15);
    abil_level(ABT_SPELL, SPELL_CAUSE_CRITIC, CLASS_DARKMAGE, 15);
    abil_level(ABT_SPELL, SPELL_CAUSE_CRITIC, CLASS_ASSASSIN, 15);
    abil_level(ABT_SPELL, SPELL_CAUSE_CRITIC, CLASS_INQUISITOR, 15);
    abil_level(ABT_SPELL, SPELL_PHANTOM_ARMOR, CLASS_DARK_KNIGHT, 16);
    abil_level(ABT_SPELL, SPELL_PHANTOM_ARMOR, CLASS_REAPER, 16);
    abil_level(ABT_SPELL, SPELL_PHANTOM_ARMOR, CLASS_DARKMAGE, 16);
    abil_level(ABT_SPELL, SPELL_PHANTOM_ARMOR, CLASS_ASSASSIN, 16);
    abil_level(ABT_SPELL, SPELL_PHANTOM_ARMOR, CLASS_INQUISITOR, 16);
    abil_level(ABT_SPELL, SPELL_CURSE, CLASS_DARK_KNIGHT, 18);
    abil_level(ABT_SPELL, SPELL_CURSE, CLASS_REAPER, 18);
    abil_level(ABT_SPELL, SPELL_CURSE, CLASS_DARKMAGE, 18);
    abil_level(ABT_SPELL, SPELL_CURSE, CLASS_ASSASSIN, 18);
    abil_level(ABT_SPELL, SPELL_CURSE, CLASS_INQUISITOR, 18);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_DARK_KNIGHT, 19);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_REAPER, 19);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_DARKMAGE, 19);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_ASSASSIN, 19);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_INQUISITOR, 19);
    abil_level(ABT_SPELL, SPELL_SPECTRAL_WINGS, CLASS_DARK_KNIGHT, 20);
    abil_level(ABT_SPELL, SPELL_SPECTRAL_WINGS, CLASS_REAPER, 20);
    abil_level(ABT_SPELL, SPELL_SPECTRAL_WINGS, CLASS_DARKMAGE, 20);
    abil_level(ABT_SPELL, SPELL_SPECTRAL_WINGS, CLASS_ASSASSIN, 20);
    abil_level(ABT_SPELL, SPELL_SPECTRAL_WINGS, CLASS_INQUISITOR, 20);
    abil_level(ABT_SPELL, SPELL_EYESOFTHEDEAD, CLASS_DARK_KNIGHT, 21);
    abil_level(ABT_SPELL, SPELL_EYESOFTHEDEAD, CLASS_REAPER, 21);
    abil_level(ABT_SPELL, SPELL_EYESOFTHEDEAD, CLASS_DARKMAGE, 21);
    abil_level(ABT_SPELL, SPELL_EYESOFTHEDEAD, CLASS_ASSASSIN, 21);
    abil_level(ABT_SPELL, SPELL_EYESOFTHEDEAD, CLASS_INQUISITOR, 21);
    abil_level(ABT_SPELL, SPELL_BLACKBREATH, CLASS_DARK_KNIGHT, 23);
    abil_level(ABT_SPELL, SPELL_BLACKBREATH, CLASS_REAPER, 23);
    abil_level(ABT_SPELL, SPELL_BLACKBREATH, CLASS_DARKMAGE, 23);
    abil_level(ABT_SPELL, SPELL_BLACKBREATH, CLASS_ASSASSIN, 23);
    abil_level(ABT_SPELL, SPELL_BLACKBREATH, CLASS_INQUISITOR, 23);
//  abil_level(SPELL_UNDEAD_MOUNT, CLASS_DARK_KNIGHT, 24); /* Vedic 7/6/99 */
//    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_DARK_KNIGHT, 25);
//    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_REAPER, 25);
//    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_DARKMAGE, 25);
//    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_ASSASSIN, 25);
//    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_INQUISITOR, 25); Raiker september 2002.
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_DARK_KNIGHT, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_REAPER, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_DARKMAGE, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_ASSASSIN, 27);
    abil_level(ABT_SPELL, SPELL_FIREBALL, CLASS_INQUISITOR, 27);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_DARK_KNIGHT, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_REAPER, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_DARKMAGE, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_ASSASSIN, 29);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_INQUISITOR, 29);
    abil_level(ABT_SKILL, SKILL_GUT, CLASS_DARK_KNIGHT, 30);
    abil_level(ABT_SKILL, SKILL_GUT, CLASS_REAPER, 30);
    abil_level(ABT_SKILL, SKILL_GUT, CLASS_DARKMAGE, 30);
    abil_level(ABT_SKILL, SKILL_GUT, CLASS_ASSASSIN, 30);
    abil_level(ABT_SKILL, SKILL_GUT, CLASS_INQUISITOR, 30);
    abil_level(ABT_SPELL, SPELL_DEMONFIRE, CLASS_DARK_KNIGHT, 37);
    abil_level(ABT_SPELL, SPELL_DEMONFIRE, CLASS_REAPER, 37);
    abil_level(ABT_SPELL, SPELL_DEMONFIRE, CLASS_DARKMAGE, 37);
    abil_level(ABT_SPELL, SPELL_DEMONFIRE, CLASS_ASSASSIN, 37);
    abil_level(ABT_SPELL, SPELL_DEMONFIRE, CLASS_INQUISITOR, 37);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_DARK_KNIGHT, 41);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_REAPER, 41);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_DARKMAGE, 41);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_ASSASSIN, 41);
    abil_level(ABT_SPELL, SPELL_FIRESTORM, CLASS_INQUISITOR, 41);
    abil_level(ABT_SPELL, SPELL_CHAOSARMOR, CLASS_DARK_KNIGHT, 45);
    abil_level(ABT_SPELL, SPELL_CHAOSARMOR, CLASS_REAPER, 45);
    abil_level(ABT_SPELL, SPELL_CHAOSARMOR, CLASS_DARKMAGE, 45);
    abil_level(ABT_SPELL, SPELL_CHAOSARMOR, CLASS_ASSASSIN, 45);
    abil_level(ABT_SPELL, SPELL_CHAOSARMOR, CLASS_INQUISITOR, 45);  
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_DARK_KNIGHT, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_REAPER, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_DARKMAGE, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_ASSASSIN, 49);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_INQUISITOR, 49);
    abil_level(ABT_SKILL, SKILL_TRAMPLE, CLASS_DARK_KNIGHT, 50);
    abil_level(ABT_SKILL, SKILL_TRAMPLE, CLASS_REAPER, 50);
    abil_level(ABT_SKILL, SKILL_TRAMPLE, CLASS_DARKMAGE, 50);
    abil_level(ABT_SKILL, SKILL_TRAMPLE, CLASS_ASSASSIN, 50);
    abil_level(ABT_SKILL, SKILL_TRAMPLE, CLASS_INQUISITOR, 50);
    abil_level(ABT_SPELL, SPELL_UNDEAD_MOUNT, CLASS_DARK_KNIGHT, 50);
    abil_level(ABT_SPELL, SPELL_UNDEAD_MOUNT, CLASS_REAPER, 50);
    abil_level(ABT_SPELL, SPELL_UNDEAD_MOUNT, CLASS_DARKMAGE, 50);
    abil_level(ABT_SPELL, SPELL_UNDEAD_MOUNT, CLASS_ASSASSIN, 50);
    abil_level(ABT_SPELL, SPELL_UNDEAD_MOUNT, CLASS_INQUISITOR, 50);
    abil_level(ABT_SPELL, SPELL_HELLFIRE, CLASS_DARK_KNIGHT, 60);
    abil_level(ABT_SPELL, SPELL_HELLFIRE, CLASS_REAPER, 60);
    abil_level(ABT_SPELL, SPELL_HELLFIRE, CLASS_DARKMAGE, 60);
    abil_level(ABT_SPELL, SPELL_HELLFIRE, CLASS_ASSASSIN, 60);
    abil_level(ABT_SPELL, SPELL_HELLFIRE, CLASS_INQUISITOR, 60);
    abil_level(ABT_SPELL, SPELL_FROST_BLADE, CLASS_DARK_KNIGHT, 70);
    abil_level(ABT_SPELL, SPELL_FROST_BLADE, CLASS_REAPER, 70);
    abil_level(ABT_SPELL, SPELL_FROST_BLADE, CLASS_DARKMAGE, 70);
    abil_level(ABT_SPELL, SPELL_FROST_BLADE, CLASS_ASSASSIN, 70);
    abil_level(ABT_SPELL, SPELL_FROST_BLADE, CLASS_INQUISITOR, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_DARK_KNIGHT, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_REAPER, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_DARKMAGE, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_ASSASSIN, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_INQUISITOR, 73);
    abil_level(ABT_SPELL, SPELL_ANIMATE_CORPSE, CLASS_DARK_KNIGHT, 75);
    abil_level(ABT_SPELL, SPELL_ANIMATE_CORPSE, CLASS_REAPER, 75);
    abil_level(ABT_SPELL, SPELL_ANIMATE_CORPSE, CLASS_DARKMAGE, 75);
    abil_level(ABT_SPELL, SPELL_ANIMATE_CORPSE, CLASS_ASSASSIN, 75);
    abil_level(ABT_SPELL, SPELL_ANIMATE_CORPSE, CLASS_INQUISITOR, 75);
    abil_level(ABT_SPELL, SPELL_FLAME_BLADE, CLASS_DARK_KNIGHT, 80);
    abil_level(ABT_SPELL, SPELL_FLAME_BLADE, CLASS_REAPER, 80);
    abil_level(ABT_SPELL, SPELL_FLAME_BLADE, CLASS_DARKMAGE, 80);
    abil_level(ABT_SPELL, SPELL_FLAME_BLADE, CLASS_ASSASSIN, 80);
    abil_level(ABT_SPELL, SPELL_FLAME_BLADE, CLASS_INQUISITOR, 80);
    abil_level(ABT_SPELL, SPELL_ACID_STREAM, CLASS_DARK_KNIGHT, 90);
    abil_level(ABT_SPELL, SPELL_ACID_STREAM, CLASS_REAPER, 90);
    abil_level(ABT_SPELL, SPELL_ACID_STREAM, CLASS_DARKMAGE, 90);
    abil_level(ABT_SPELL, SPELL_ACID_STREAM, CLASS_ASSASSIN, 90);
    abil_level(ABT_SPELL, SPELL_ACID_STREAM, CLASS_INQUISITOR, 90);
    abil_level(ABT_SPELL, SPELL_DARKWARD, CLASS_DARK_KNIGHT, 95);
    abil_level(ABT_SPELL, SPELL_DARKWARD, CLASS_REAPER, 95);
    abil_level(ABT_SPELL, SPELL_DARKWARD, CLASS_DARKMAGE, 95);
    abil_level(ABT_SPELL, SPELL_DARKWARD, CLASS_ASSASSIN, 95);
    abil_level(ABT_SPELL, SPELL_DARKWARD, CLASS_INQUISITOR, 95);
    abil_level(ABT_SPELL, SPELL_FLAME_STRIKE, CLASS_DARK_KNIGHT, 100);
    abil_level(ABT_SPELL, SPELL_FLAME_STRIKE, CLASS_REAPER, 100);
    abil_level(ABT_SPELL, SPELL_FLAME_STRIKE, CLASS_DARKMAGE, 100);
    abil_level(ABT_SPELL, SPELL_FLAME_STRIKE, CLASS_ASSASSIN, 100);
    abil_level(ABT_SPELL, SPELL_FLAME_STRIKE, CLASS_INQUISITOR, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_DARK_KNIGHT, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_REAPER, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_DARKMAGE, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_ASSASSIN, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_INQUISITOR, 100);
    abil_level(ABT_SPELL, SPELL_SEAL, CLASS_DARK_KNIGHT, 105);
    abil_level(ABT_SPELL, SPELL_SEAL, CLASS_REAPER, 105);
    abil_level(ABT_SPELL, SPELL_SEAL, CLASS_DARKMAGE, 105);
    abil_level(ABT_SPELL, SPELL_SEAL, CLASS_ASSASSIN, 105);
    abil_level(ABT_SPELL, SPELL_SEAL, CLASS_INQUISITOR, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_DARK_KNIGHT, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_REAPER, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_DARKMAGE, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_ASSASSIN, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_INQUISITOR, 108);
    abil_level(ABT_SPELL, SPELL_FINGER_OF_DEATH, CLASS_DARK_KNIGHT, 110);
    abil_level(ABT_SPELL, SPELL_FINGER_OF_DEATH, CLASS_REAPER, 110);
    abil_level(ABT_SPELL, SPELL_FINGER_OF_DEATH, CLASS_DARKMAGE, 110);
    abil_level(ABT_SPELL, SPELL_FINGER_OF_DEATH, CLASS_ASSASSIN, 110);
    abil_level(ABT_SPELL, SPELL_FINGER_OF_DEATH, CLASS_INQUISITOR, 110);
    abil_level(ABT_SPELL, SPELL_INSOMNIA, CLASS_DARK_KNIGHT, 115);
    abil_level(ABT_SPELL, SPELL_INSOMNIA, CLASS_REAPER, 115);
    abil_level(ABT_SPELL, SPELL_INSOMNIA, CLASS_DARKMAGE, 115);
    abil_level(ABT_SPELL, SPELL_INSOMNIA, CLASS_ASSASSIN, 115);
    abil_level(ABT_SPELL, SPELL_INSOMNIA, CLASS_INQUISITOR, 115);
    abil_level(ABT_SPELL, SPELL_SCOURGE, CLASS_DARK_KNIGHT, 120);
    abil_level(ABT_SPELL, SPELL_SCOURGE, CLASS_REAPER, 120);
    abil_level(ABT_SPELL, SPELL_SCOURGE, CLASS_DARKMAGE, 120);
    abil_level(ABT_SPELL, SPELL_SCOURGE, CLASS_ASSASSIN, 120);
    abil_level(ABT_SPELL, SPELL_SCOURGE, CLASS_INQUISITOR, 120);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_DARK_KNIGHT, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_REAPER, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_DARKMAGE, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_ASSASSIN, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_INQUISITOR, 125);
    abil_level(ABT_SPELL, SPELL_SOUL_RIP, CLASS_DARK_KNIGHT, 130);
    abil_level(ABT_SPELL, SPELL_SOUL_RIP, CLASS_REAPER, 130);
    abil_level(ABT_SPELL, SPELL_SOUL_RIP, CLASS_DARKMAGE, 130);
    abil_level(ABT_SPELL, SPELL_SOUL_RIP, CLASS_ASSASSIN, 130);
    abil_level(ABT_SPELL, SPELL_SOUL_RIP, CLASS_INQUISITOR, 130);
    abil_level(ABT_SPELL, SPELL_FELLBLADE, CLASS_DARK_KNIGHT, 140);
    abil_level(ABT_SPELL, SPELL_FELLBLADE, CLASS_REAPER, 140);
    abil_level(ABT_SPELL, SPELL_FELLBLADE, CLASS_DARKMAGE, 140);
    abil_level(ABT_SPELL, SPELL_FELLBLADE, CLASS_ASSASSIN, 140);
    abil_level(ABT_SPELL, SPELL_FELLBLADE, CLASS_INQUISITOR, 140);
    abil_level(ABT_SPELL, SPELL_EXTERMINATE, CLASS_DARK_KNIGHT, 150);
    abil_level(ABT_SPELL, SPELL_EXTERMINATE, CLASS_REAPER, 150);
    abil_level(ABT_SPELL, SPELL_EXTERMINATE, CLASS_DARKMAGE, 150);
    abil_level(ABT_SPELL, SPELL_EXTERMINATE, CLASS_ASSASSIN, 150);
    abil_level(ABT_SPELL, SPELL_EXTERMINATE, CLASS_INQUISITOR, 150);

    /* MONKS */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_MONK, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_MONK, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_ACROBAT, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_PSIONIST, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_COMBAT_MASTER, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_STORR, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_SAMURAI, 1);
    abil_level(ABT_SKILL, SKILL_TRIP, CLASS_SHAMAN, 1);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_MONK, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_ACROBAT, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_PSIONIST, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_COMBAT_MASTER, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_STORR, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_SAMURAI, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_SHAMAN, 5);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_MONK, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_ACROBAT, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_PSIONIST, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_COMBAT_MASTER, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_STORR, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SAMURAI, 9);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SHAMAN, 9);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_MONK, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_ACROBAT, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_PSIONIST, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_COMBAT_MASTER, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_STORR, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SAMURAI, 11);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SHAMAN, 11);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_MONK, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_ACROBAT, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_PSIONIST, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_COMBAT_MASTER, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_STORR, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_SAMURAI, 13);
    abil_level(ABT_SKILL, SKILL_DIRTTHROW, CLASS_SHAMAN, 13);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_MONK, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_ACROBAT, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_PSIONIST, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_COMBAT_MASTER, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_STORR, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_SAMURAI, 16);
    abil_level(ABT_CHANT, CHANT_HEALING_HAND, CLASS_SHAMAN, 16);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_MONK, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_ACROBAT, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_PSIONIST, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_COMBAT_MASTER, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_STORR, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_SAMURAI, 20);
    abil_level(ABT_SKILL, SKILL_SHADOW, CLASS_SHAMAN, 20);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_MONK, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_ACROBAT, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_PSIONIST, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_COMBAT_MASTER, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_STORR, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_SAMURAI, 22);
    abil_level(ABT_SKILL, SKILL_AWE, CLASS_SHAMAN, 22);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_MONK, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_ACROBAT, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_PSIONIST, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_COMBAT_MASTER, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_STORR, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_SAMURAI, 24);
    abil_level(ABT_SKILL, SKILL_ROUNDHOUSE, CLASS_SHAMAN, 24);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_MONK, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_ACROBAT, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_PSIONIST, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_COMBAT_MASTER, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_STORR, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_SAMURAI, 27);
    abil_level(ABT_CHANT, CHANT_FIND_CENTER, CLASS_SHAMAN, 27);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_MONK, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_ACROBAT, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_PSIONIST, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_COMBAT_MASTER, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_STORR, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_SAMURAI, 30);
    abil_level(ABT_CHANT, CHANT_YIN_XU, CLASS_SHAMAN, 30);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_MONK, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_ACROBAT, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_PSIONIST, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_COMBAT_MASTER, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_STORR, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_SAMURAI, 33);
    abil_level(ABT_CHANT, CHANT_LEND_HEALTH, CLASS_SHAMAN, 33);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_MONK, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_ACROBAT, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_PSIONIST, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_COMBAT_MASTER, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_STORR, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SAMURAI, 39);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SHAMAN, 39);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_MONK, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_ACROBAT, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_PSIONIST, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_COMBAT_MASTER, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_STORR, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_SAMURAI, 40);
    abil_level(ABT_SKILL, SKILL_KICKFLIP, CLASS_SHAMAN, 40);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_MONK, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_ACROBAT, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_PSIONIST, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_COMBAT_MASTER, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_STORR, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_SAMURAI, 42);
    abil_level(ABT_CHANT, CHANT_OMM, CLASS_SHAMAN, 42);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_MONK, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_ACROBAT, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_PSIONIST, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_COMBAT_MASTER, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_STORR, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_SAMURAI, 46);
    abil_level(ABT_CHANT, CHANT_PSIONIC_DRAIN, CLASS_SHAMAN, 46);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_MONK, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_ACROBAT, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_PSIONIST, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_COMBAT_MASTER, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_STORR, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SAMURAI, 50);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SHAMAN, 50);
    abil_level(ABT_SKILL, SKILL_FAST_HEAL, CLASS_SAMURAI, 50);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_MONK, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_ACROBAT, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_PSIONIST, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_COMBAT_MASTER, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_STORR, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_SAMURAI, 55);
    abil_level(ABT_SKILL, SKILL_CHOP, CLASS_SHAMAN, 55);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_MONK, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_ACROBAT, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_PSIONIST, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_COMBAT_MASTER, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_STORR, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SAMURAI, 70);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SHAMAN, 70);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_MONK, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_ACROBAT, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_PSIONIST, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_COMBAT_MASTER, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_STORR, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_SAMURAI, 75);
    abil_level(ABT_SKILL, SKILL_TACKLE, CLASS_SHAMAN, 75);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_MONK, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_ACROBAT, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_PSIONIST, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_COMBAT_MASTER, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_STORR, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_SAMURAI, 90);
    abil_level(ABT_SKILL, SKILL_PRESSURE, CLASS_SHAMAN, 90);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_MONK, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_ACROBAT, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_PSIONIST, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_COMBAT_MASTER, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_STORR, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_SAMURAI, 95);
    abil_level(ABT_CHANT, CHANT_ANCIENT_PROT, CLASS_SHAMAN, 95);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_MONK, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_ACROBAT, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_PSIONIST, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_COMBAT_MASTER, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_STORR, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_SAMURAI, 100);
    abil_level(ABT_CHANT, CHANT_YANG_XU, CLASS_SHAMAN, 100);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_MONK, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_ACROBAT, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_PSIONIST, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_COMBAT_MASTER, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_STORR, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SAMURAI, 105);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SHAMAN, 105);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_MONK, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_ACROBAT, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_PSIONIST, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_COMBAT_MASTER, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_STORR, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_SAMURAI, 110);
    abil_level(ABT_CHANT, CHANT_BALANCE, CLASS_SHAMAN, 110);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_MONK, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_ACROBAT, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_PSIONIST, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_COMBAT_MASTER, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_STORR, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_SAMURAI, 120);
    abil_level(ABT_CHANT, CHANT_MENTAL_BARRIER, CLASS_SHAMAN, 120);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_MONK, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_ACROBAT, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_PSIONIST, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_COMBAT_MASTER, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_STORR, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_SAMURAI, 121);
    abil_level(ABT_SKILL, SKILL_KI, CLASS_SHAMAN, 121);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_MONK, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_ACROBAT, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_PSIONIST, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_COMBAT_MASTER, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_STORR, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_SAMURAI, 130);
    abil_level(ABT_SKILL, SKILL_HEADCUT, CLASS_SHAMAN, 130);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_MONK, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_ACROBAT, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_PSIONIST, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_COMBAT_MASTER, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_STORR, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_SAMURAI, 140);
    abil_level(ABT_SKILL, SKILL_KAMIKAZE, CLASS_SHAMAN, 140);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_MONK, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_ACROBAT, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_PSIONIST, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_COMBAT_MASTER, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_STORR, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_SAMURAI, 148);
    abil_level(ABT_CHANT, CHANT_PSYCHIC_FURY, CLASS_SHAMAN, 148);

    /* PALADIN */
//  abil_level(ABT_SKILL, SKILL_THROW, CLASS_PALADIN, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_PALADIN, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_CHAMPION, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_MAGI, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_TEMPLAR, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_SAMURAI, 1);
    abil_level(ABT_SKILL, SKILL_KICK, CLASS_FIANNA, 1);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_PALADIN, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_CHAMPION, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_MAGI, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_TEMPLAR, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_SAMURAI, 3);
    abil_level(ABT_SKILL, SKILL_RESCUE, CLASS_FIANNA, 3);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_PALADIN, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_CHAMPION, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_MAGI, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_TEMPLAR, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_SAMURAI, 5);
    abil_level(ABT_SKILL, SKILL_BASH, CLASS_FIANNA, 5);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_PALADIN, 7);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_CHAMPION, 7);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_MAGI, 7);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_TEMPLAR, 7);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_SAMURAI, 7);
    abil_level(ABT_SKILL, SKILL_BLOCK, CLASS_FIANNA, 7);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_PALADIN, 9);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_CHAMPION, 9);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_MAGI, 9);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_TEMPLAR, 9);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_SAMURAI, 9);
    abil_level(ABT_SPELL, SPELL_CURE_LIGHT, CLASS_FIANNA, 9);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_PALADIN, 10);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_CHAMPION, 10);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_MAGI, 10);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_TEMPLAR, 10);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_SAMURAI, 10);
    abil_level(ABT_SKILL, SKILL_TRACK, CLASS_FIANNA, 10);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_PALADIN, 11);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_CHAMPION, 11);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_MAGI, 11);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_TEMPLAR, 11);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_SAMURAI, 11);
    abil_level(ABT_SPELL, SPELL_LAY_HANDS, CLASS_FIANNA, 11);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_PALADIN, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_CHAMPION, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_MAGI, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_TEMPLAR, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_SAMURAI, 13);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_FIANNA, 13);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_PALADIN, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_CHAMPION, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_MAGI, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_TEMPLAR, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_SAMURAI, 14);
    abil_level(ABT_SPELL, SPELL_DETECT_ALIGN, CLASS_FIANNA, 14);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_PALADIN, 15);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_CHAMPION, 15);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_MAGI, 15);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_TEMPLAR, 15);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_SAMURAI, 15);
    abil_level(ABT_SPELL, SPELL_CURE_CRITIC, CLASS_FIANNA, 15);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_PALADIN, 17);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_CHAMPION, 17);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_MAGI, 17);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_TEMPLAR, 17);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_SAMURAI, 17);
    abil_level(ABT_SPELL, SPELL_BLESS, CLASS_FIANNA, 17);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_PALADIN, 20);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_CHAMPION, 20);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_MAGI, 20);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_TEMPLAR, 20);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_SAMURAI, 20);
    abil_level(ABT_SKILL, SKILL_UPPERCUT, CLASS_FIANNA, 20);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_PALADIN, 21);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_CHAMPION, 21);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_MAGI, 21);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_TEMPLAR, 21);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_SAMURAI, 21);
    abil_level(ABT_SKILL, SKILL_SHIELDPUNCH, CLASS_FIANNA, 21);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_PALADIN, 22);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_CHAMPION, 22);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_MAGI, 22);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_TEMPLAR, 22);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_SAMURAI, 22);
    abil_level(ABT_SPELL, SPELL_PROT_FROM_EVIL, CLASS_FIANNA, 22);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_PALADIN, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_CHAMPION, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_MAGI, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_TEMPLAR, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_SAMURAI, 23);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_FIANNA, 23);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_PALADIN, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_CHAMPION, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_MAGI, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_TEMPLAR, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_SAMURAI, 25);
    abil_level(ABT_SKILL, SKILL_RETREAT, CLASS_FIANNA, 25);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_PALADIN, 27);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_CHAMPION, 27);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_MAGI, 27);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_TEMPLAR, 27);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_SAMURAI, 27);
    abil_level(ABT_SKILL, SKILL_CHARGE, CLASS_FIANNA, 27);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_PALADIN, 33);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_CHAMPION, 33);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_MAGI, 33);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_TEMPLAR, 33);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_SAMURAI, 33);
    abil_level(ABT_SPELL, SPELL_AEGIS, CLASS_FIANNA, 33);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_PALADIN, 35);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_CHAMPION, 35);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_MAGI, 35);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_TEMPLAR, 35);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_SAMURAI, 35);
    abil_level(ABT_SPELL, SPELL_HOLY_FURY, CLASS_FIANNA, 35);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_PALADIN, 37);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_CHAMPION, 37);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_MAGI, 37);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_TEMPLAR, 37);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_SAMURAI, 37);
    abil_level(ABT_SKILL, SKILL_WARCRY, CLASS_FIANNA, 37);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_PALADIN, 40);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_CHAMPION, 40);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_MAGI, 40);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_TEMPLAR, 40);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_SAMURAI, 40);
    abil_level(ABT_SKILL, SKILL_STUN, CLASS_FIANNA, 40);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_PALADIN, 41);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_CHAMPION, 41);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_MAGI, 41);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_TEMPLAR, 41);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_SAMURAI, 41);
    abil_level(ABT_SKILL, SKILL_BACKSLASH, CLASS_FIANNA, 41);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_PALADIN, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_CHAMPION, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_MAGI, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_TEMPLAR, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_SAMURAI, 45);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_FIANNA, 45);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_PALADIN, 47);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_CHAMPION, 47);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_MAGI, 47);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_TEMPLAR, 47);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_SAMURAI, 47);
    abil_level(ABT_SPELL, SPELL_CHAMPION_STRENGTH, CLASS_FIANNA, 47);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_PALADIN, 48);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_CHAMPION, 48);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_MAGI, 48);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_TEMPLAR, 48);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_SAMURAI, 48);
    abil_level(ABT_SPELL, SPELL_HOLY_STEED, CLASS_FIANNA, 48);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_PALADIN, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_CHAMPION, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_MAGI, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_TEMPLAR, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_SAMURAI, 50);
    abil_level(ABT_SKILL, SKILL_SWORD_THRUST, CLASS_FIANNA, 50);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_PALADIN, 51);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_CHAMPION, 51);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_MAGI, 51);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_TEMPLAR, 51);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_SAMURAI, 51);
    abil_level(ABT_SPELL, SPELL_HOLY_MACE, CLASS_FIANNA, 51);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_PALADIN, 61);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_CHAMPION, 61);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_MAGI, 61);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_TEMPLAR, 61);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_SAMURAI, 61);
    abil_level(ABT_SPELL, SPELL_RAY_OF_LIGHT, CLASS_FIANNA, 61);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_PALADIN, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_CHAMPION, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_MAGI, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_TEMPLAR, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_SAMURAI, 73);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_FIANNA, 73);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_PALADIN, 75);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_CHAMPION, 75);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_MAGI, 75);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_TEMPLAR, 75);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_SAMURAI, 75);
    abil_level(ABT_SKILL, SKILL_POWERSLASH, CLASS_FIANNA, 75);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_PALADIN, 95);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_CHAMPION, 95);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_MAGI, 95);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_TEMPLAR, 95);
    abil_level(ABT_SPELL, SPELL_DIVINE_NIMBUS, CLASS_FIANNA, 95);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_PALADIN, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_CHAMPION, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_MAGI, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_TEMPLAR, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_SAMURAI, 108);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_FIANNA, 108);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_PALADIN, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_CHAMPION, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_MAGI, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_TEMPLAR, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_SAMURAI, 100);
    abil_level(ABT_SKILL, SKILL_DUAL_WIELD, CLASS_FIANNA, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_PALADIN, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_CHAMPION, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_MAGI, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_TEMPLAR, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_SAMURAI, 100);
    abil_level(ABT_SPELL, SPELL_BLADE_OF_LIGHT, CLASS_FIANNA, 100);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_PALADIN, 110);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_CHAMPION, 110);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_MAGI, 110);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_TEMPLAR, 110);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_SAMURAI, 110);
    abil_level(ABT_SPELL, SPELL_FLAMES_PURE, CLASS_FIANNA, 110);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_PALADIN, 120);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_CHAMPION, 120);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_MAGI, 120);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_TEMPLAR, 120);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_SAMURAI, 120);
    abil_level(ABT_SPELL, SPELL_PRISMATIC_BEAM, CLASS_FIANNA, 120);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_PALADIN, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_CHAMPION, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_MAGI, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_TEMPLAR, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_SAMURAI, 125);
    abil_level(ABT_SKILL, SKILL_PUSH, CLASS_FIANNA, 125);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_PALADIN, 130);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_CHAMPION, 130);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_MAGI, 130);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_TEMPLAR, 130);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_SAMURAI, 130);
    abil_level(ABT_SKILL, SKILL_BLADED_FURY, CLASS_FIANNA, 130);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_PALADIN, 140);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_CHAMPION, 140);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_MAGI, 140);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_TEMPLAR, 140);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_SAMURAI, 140);
    abil_level(ABT_SPELL, SPELL_HOLOCAUST, CLASS_FIANNA, 140);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_PALADIN, 150);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_CHAMPION, 150);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_MAGI, 150);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_TEMPLAR, 150);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_SAMURAI, 150);
    abil_level(ABT_SPELL, SPELL_CRY_FOR_JUSTICE, CLASS_FIANNA, 150);

    /* Bards */
    abil_level(ABT_SONG, SONG_BRAVERY, CLASS_BARD, 1);
    abil_level(ABT_SONG, SONG_HONOR, CLASS_BARD, 3);
    abil_level(ABT_SONG, SONG_HERO, CLASS_BARD, 12);
    abil_level(ABT_SONG, SONG_LULLABY, CLASS_BARD, 36);
    abil_level(ABT_SKILL, SKILL_SECOND_ATTACK, CLASS_BARD, 16);
    abil_level(ABT_SKILL, SKILL_THIRD_ATTACK, CLASS_BARD, 34);
    abil_level(ABT_SKILL, SKILL_FOURTH_ATTACK, CLASS_BARD, 58);
    abil_level(ABT_SKILL, SKILL_FIFTH_ATTACK, CLASS_BARD, 88);
    abil_level(ABT_SKILL, SKILL_SIXTH_ATTACK, CLASS_BARD, 124);

    /* VAMPIRES */
    abil_level(ABT_SKILL, SKILL_SWOOP, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_TEAR, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_HOVER, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_REND, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_MESMERIZE, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_HISS, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_VANISH, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_FIND_FLESH, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_SACRED_EARTH, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_DEATH_CALL, CLASS_VAMPIRE, 1);
    abil_level(ABT_SKILL, SKILL_SPIT_BLOOD, CLASS_VAMPIRE, 1);

    /* TITANS */
    abil_level(ABT_SKILL, SKILL_HARNESS_WIND, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_HARNESS_WATER, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_HARNESS_FIRE, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_HARNESS_EARTH, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_SCORCH, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_ABSORB, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_CLOUD, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_SUMMON_ELEM, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_INTIMIDATE, CLASS_TITAN, 1);
    abil_level(ABT_SKILL, SKILL_BELLOW, CLASS_TITAN, 1);


    /* SAINTS */
    abil_level(ABT_PRAYER, PRAY_HOLY_WATER, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_EXORCISM, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_AFTERLIFE, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_HEAVENSBEAM, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_DIVINESHIELD, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_ETHEREAL, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_CHOIR, CLASS_SAINT, 1);
    abil_level(ABT_PRAYER, PRAY_ENLIGHTENMENT, CLASS_SAINT, 1);

    /* DEMONS */
    abil_level(ABT_SPELL, SPELL_NETHERCLAW, CLASS_DEMON, 1);
    abil_level(ABT_SPELL, SPELL_CONJURE_FIREDRAKE, CLASS_DEMON, 1);
    abil_level(ABT_SPELL, SPELL_RAIN_OF_FIRE, CLASS_DEMON, 1);
    abil_level(ABT_SKILL, SKILL_DECEIVE, CLASS_DEMON, 1);
    abil_level(ABT_SKILL, SKILL_DARK_ALLEGIANCE, CLASS_DEMON, 1);
    abil_level(ABT_SKILL, SKILL_ABSORB_FIRE, CLASS_DEMON, 1);
    abil_level(ABT_SKILL, SKILL_SOULSUCK, CLASS_DEMON, 1);
    abil_level(ABT_SKILL, SKILL_BESTOW_WINGS, CLASS_DEMON, 1);

}


/*
 * This is the exp given to implementors -- it must always be greater
 * than the exp required for immortality, plus at least 20,000 or so.
 */
#define EXP_MAX  2000000000

uint exp_table[NUM_CLASSES][LVL_IMMORT - 1];

/* Experience modifiers       */
/*  {Exponent, Multiplier}... */
const float exp_mods[NUM_CLASSES][2] = {
    {2.70, 8.4},		/* Sorcerer    */
    {2.65, 10.0},		/* Cleric      */
    {2.65, 11.0},		/* Thief       */
    {2.65, 11.0},		/* Gladiator   */
    {2.65, 11.0},		/* Druid       */
    {2.65, 11.0},		/* Dark Knight */
    {2.65, 11.0},		/* Monk        */
    {2.65, 11.0},		/* Paladin     */
    {2.65, 10.0}		/* Bard        */
};

void load_exp_table()
{

    int cls;
    int lvl;
    float exp, mlt;

    for (cls = 0; cls < NUM_CLASSES; cls++) {
	exp = exp_mods[cls][0];
	mlt = exp_mods[cls][1];
	exp_table[cls][0] = 1;
	for (lvl = 1; lvl <= LVL_IMMORT - 1; lvl++) {
	    if (cls >= CLASS_VAMPIRE) {
		exp_table[cls][lvl] = exp_table[cls][lvl - 1] + 50000000;
	    } else if (cls >= 9 && cls < CLASS_VAMPIRE) {
		if (lvl < 49)
		    exp_table[cls][lvl] = exp_table[cls][lvl - 1];
		if (lvl >= 50 && lvl <= 74)
		    exp_table[cls][lvl] =
			exp_table[cls][lvl - 1] + 5000000;
		if (lvl >= 75 && lvl <= 99)
		    exp_table[cls][lvl] =
			exp_table[cls][lvl - 1] + 10000000;
		if (lvl >= 100 && lvl <= 124)
		    exp_table[cls][lvl] =
			exp_table[cls][lvl - 1] + 20000000;
		if (lvl >= 125 && lvl <= 139)
		    exp_table[cls][lvl] =
			exp_table[cls][lvl - 1] + 30000000;
		if (lvl >= 140 && lvl <= 150)
		    exp_table[cls][lvl] =
			exp_table[cls][lvl - 1] + 65000000;
	    } else
		exp_table[cls][lvl] =
		    exp_table[cls][lvl - 1] +
		    (uint) (pow(lvl * 2, exp)) * mlt;
	}
    }
}

uint level_exp(int class, int level)
{

    if (level > LVL_IMPL || level < 0) {
	log("SYSERR: Requesting exp for invalid level!");
	return 0;
    }

    if (level == 0)
	return 0;

    /*
     * Gods have exp close to EXP_MAX.  This statement should never have to
     * be changed, regardless of how many mortal or immortal levels exist.
     */
    if (level >= LVL_IMMORT) {
	return EXP_MAX - ((LVL_IMPL - level) * 1000);
    }

    return (exp_table[class][level - 1]);

}

/* 
 * Default titles of male characters.
 */
char *title_male(int class, int level)
{
    if (level <= 0 || level > LVL_IMPL)
	return "the Man";
    if (level == LVL_IMPL)
	return "the Implementor";

    switch (class) {

    case CLASS_SORCERER:
	switch (level) {
	case 1:
	    return "the Apprentice of Magic";
	    break;
	case 2:
	    return "the Spell Student";
	    break;
	case 3:
	    return "the Scholar of Magic";
	    break;
	case 4:
	    return "the Delver in Spells";
	    break;
	case 5:
	    return "the Medium of Magic";
	    break;
	case 6:
	    return "the Scribe of Magic";
	    break;
	case 7:
	    return "the Seer";
	    break;
	case 8:
	    return "the Sage";
	    break;
	case 9:
	    return "the Illusionist";
	    break;
	case 10:
	    return "the Abjurer";
	    break;
	case 11:
	    return "the Invoker";
	    break;
	case 12:
	    return "the Enchanter";
	    break;
	case 13:
	    return "the Conjurer";
	    break;
	case 14:
	    return "the Magician";
	    break;
	case 15:
	    return "the Creator";
	    break;
	case 16:
	    return "the Savant";
	    break;
	case 17:
	    return "the Magus";
	    break;
	case 18:
	    return "the Wizard";
	    break;
	case 19:
	    return "the Warlock";
	    break;
	case 20:
	    return "the Sorcerer";
	    break;
	case 21:
	    return "the Necromancer";
	    break;
	case 22:
	    return "the Thaumaturge";
	    break;
	case 23:
	    return "the Student of the Occult";
	    break;
	case 24:
	    return "the Disciple of the Uncanny";
	    break;
	case 25:
	    return "the Minor Elemental";
	    break;
	case 26:
	    return "the Greater Elemental";
	    break;
	case 27:
	    return "the Crafter of Magics";
	    break;
	case 28:
	    return "the Shaman";
	    break;
	case 29:
	    return "the Keeper of Talismans";
	    break;
	case 30:
	    return "the Archmage";
	    break;
	case LVL_IMMORT:
	    return "the Immortal Warlock";
	    break;
	case LVL_ADMIN:
	    return "the Avatar of Magic";
	    break;
	case LVL_BUILDER:
	    return "the God of Magic";
	    break;
	default:
	    return "the Mage";
	    break;
	}
	break;

    case CLASS_CLERIC:
	switch (level) {
	case 1:
	    return "the Believer";
	    break;
	case 2:
	    return "the Attendant";
	    break;
	case 3:
	    return "the Acolyte";
	    break;
	case 4:
	    return "the Novice";
	    break;
	case 5:
	    return "the Missionary";
	    break;
	case 6:
	    return "the Adept";
	    break;
	case 7:
	    return "the Deacon";
	    break;
	case 8:
	    return "the Vicar";
	    break;
	case 9:
	    return "the Priest";
	    break;
	case 10:
	    return "the Minister";
	    break;
	case 11:
	    return "the Canon";
	    break;
	case 12:
	    return "the Levite";
	    break;
	case 13:
	    return "the Curate";
	    break;
	case 14:
	    return "the Monk";
	    break;
	case 15:
	    return "the Healer";
	    break;
	case 16:
	    return "the Chaplain";
	    break;
	case 17:
	    return "the Expositor";
	    break;
	case 18:
	    return "the Bishop";
	    break;
	case 19:
	    return "the Arch Bishop";
	    break;
	case 20:
	    return "the Patriarch";
	    break;
	    /* no one ever thought up these titles 21-30 */
	case LVL_IMMORT:
	    return "the Immortal Cardinal";
	    break;
	case LVL_ADMIN:
	    return "the Inquisitor";
	    break;
	case LVL_BUILDER:
	    return "the God of good and evil";
	    break;
	default:
	    return "the Cleric";
	    break;
	}
	break;

    case CLASS_THIEF:
	switch (level) {
	case 1:
	    return "the Wanna-Be";
	    break;
	case 2:
	    return "the Scoundrel";
	    break;
	case 3:
	    return "the Sly";
	    break;
	case 4:
	    return "the Sneak";
	    break;
	case 5:
	    return "the Crafty";
	    break;
	case 6:
	    return "the Pilferer";
	    break;
	case 7:
	    return "the Swindler";
	    break;
	case 8:
	    return "the Brigand";
	    break;
	case 9:
	    return "the Looter";
	    break;
	case 10:
	    return "the Footpad";
	    break;
	case 11:
	    return "the Lock-pick";
	    break;
	case 12:
	    return "the Filcher";
	    break;
	case 13:
	    return "the Pick-Pocket";
	    break;
	case 14:
	    return "the Sneak";
	    break;
	case 15:
	    return "the Pincher";
	    break;
	case 16:
	    return "the Klepto";
	    break;
	case 17:
	    return "the Bandit";
	    break;
	case 18:
	    return "the Bandit";
	    break;
	case 19:
	    return "the Marauder";
	    break;
	case 20:
	    return "the Cut-Throat";
	    break;
	    /* no one ever thought up these titles 21-30 */
	case LVL_IMMORT:
	    return "the Immortal Assasin";
	    break;
	case LVL_ADMIN:
	    return "the Demi God of thieves";
	    break;
	case LVL_BUILDER:
	    return "the God of thieves and tradesmen";
	    break;
	default:
	    return "the Thief";
	    break;
	}
	break;

    case CLASS_GLADIATOR:
	switch (level) {
	case 1:
	    return "the Swordpupil";
	    break;
	case 2:
	    return "the Recruit";
	    break;
	case 3:
	    return "the Sentry";
	    break;
	case 4:
	    return "the Fighter";
	    break;
	case 5:
	    return "the Soldier";
	    break;
	case 6:
	    return "the Warrior";
	    break;
	case 7:
	    return "the Veteran";
	    break;
	case 8:
	    return "the Swordsman";
	    break;
	case 9:
	    return "the Fencer";
	    break;
	case 10:
	    return "the Combatant";
	    break;
	case 11:
	    return "the Hero";
	    break;
	case 12:
	    return "the Myrmidon";
	    break;
	case 13:
	    return "the Swashbuckler";
	    break;
	case 14:
	    return "the Mercenary";
	    break;
	case 15:
	    return "the Swordmaster";
	    break;
	case 16:
	    return "the Lieutenant";
	    break;
	case 17:
	    return "the Champion";
	    break;
	case 18:
	    return "the Dragoon";
	    break;
	case 19:
	    return "the Cavalier";
	    break;
	case 20:
	    return "the Knight";
	    break;
	    /* no one ever thought up these titles 21-30 */
	case LVL_IMMORT:
	    return "the Immortal Warlord";
	    break;
	case LVL_ADMIN:
	    return "the Extirpator";
	    break;
	case LVL_BUILDER:
	    return "the God of war";
	    break;
	default:
	    return "the Warrior";
	    break;
	}
	break;

    case CLASS_DRUID:
	switch (level) {
	case 1:
	    return "the Druid";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Druid";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Druid";
	    break;
	}
	break;
    case CLASS_DARK_KNIGHT:
	switch (level) {
	case 1:
	    return "the Dark Knight";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Dark Knight";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Dark Knight";
	    break;
	}
	break;
    case CLASS_MONK:
	switch (level) {
	case 1:
	    return "the Monk";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Monk";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Monk";
	    break;
	}
	break;
    case CLASS_PALADIN:
	switch (level) {
	case 1:
	    return "the Paladin";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Paladin";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Paladin";
	    break;
	}
	break;
    case CLASS_BARD:
	switch (level) {
	case 1:
	    return "the Bard";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Bard";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	case LVL_CODER:
	    return "the coder";
	    break;
	default:
	    return "the Bard";
	    break;
	}
	break;
    }

    /* Default title for classes which do not have titles defined */
    return "the Classless";
}


/* 
 * Default titles of female characters.
 */
char *title_female(int class, int level)
{
    if (level <= 0 || level > LVL_IMPL)
	return "the Woman";
    if (level == LVL_IMPL)
	return "the Implementress";

    switch (class) {

    case CLASS_SORCERER:
	switch (level) {
	case 1:
	    return "the Apprentice of Magic";
	    break;
	case 2:
	    return "the Spell Student";
	    break;
	case 3:
	    return "the Scholar of Magic";
	    break;
	case 4:
	    return "the Delveress in Spells";
	    break;
	case 5:
	    return "the Medium of Magic";
	    break;
	case 6:
	    return "the Scribess of Magic";
	    break;
	case 7:
	    return "the Seeress";
	    break;
	case 8:
	    return "the Sage";
	    break;
	case 9:
	    return "the Illusionist";
	    break;
	case 10:
	    return "the Abjuress";
	    break;
	case 11:
	    return "the Invoker";
	    break;
	case 12:
	    return "the Enchantress";
	    break;
	case 13:
	    return "the Conjuress";
	    break;
	case 14:
	    return "the Witch";
	    break;
	case 15:
	    return "the Creator";
	    break;
	case 16:
	    return "the Savant";
	    break;
	case 17:
	    return "the Craftess";
	    break;
	case 18:
	    return "the Wizard";
	    break;
	case 19:
	    return "the War Witch";
	    break;
	case 20:
	    return "the Sorceress";
	    break;
	case 21:
	    return "the Necromancress";
	    break;
	case 22:
	    return "the Thaumaturgess";
	    break;
	case 23:
	    return "the Student of the Occult";
	    break;
	case 24:
	    return "the Disciple of the Uncanny";
	    break;
	case 25:
	    return "the Minor Elementress";
	    break;
	case 26:
	    return "the Greater Elementress";
	    break;
	case 27:
	    return "the Crafter of Magics";
	    break;
	case 28:
	    return "Shaman";
	    break;
	case 29:
	    return "the Keeper of Talismans";
	    break;
	case 30:
	    return "Archwitch";
	    break;
	case LVL_IMMORT:
	    return "the Immortal Enchantress";
	    break;
	case LVL_ADMIN:
	    return "the Empress of Magic";
	    break;
	case LVL_BUILDER:
	    return "the Goddess of Magic";
	    break;
	default:
	    return "the Witch";
	    break;
	}
	break;

    case CLASS_CLERIC:
	switch (level) {
	case 1:
	    return "the Believer";
	    break;
	case 2:
	    return "the Attendant";
	    break;
	case 3:
	    return "the Acolyte";
	    break;
	case 4:
	    return "the Novice";
	    break;
	case 5:
	    return "the Missionary";
	    break;
	case 6:
	    return "the Adept";
	    break;
	case 7:
	    return "the Deaconess";
	    break;
	case 8:
	    return "the Vicaress";
	    break;
	case 9:
	    return "the Priestess";
	    break;
	case 10:
	    return "the Lady Minister";
	    break;
	case 11:
	    return "the Canon";
	    break;
	case 12:
	    return "the Levitess";
	    break;
	case 13:
	    return "the Curess";
	    break;
	case 14:
	    return "the Nunne";
	    break;
	case 15:
	    return "the Healess";
	    break;
	case 16:
	    return "the Chaplain";
	    break;
	case 17:
	    return "the Expositress";
	    break;
	case 18:
	    return "the Bishop";
	    break;
	case 19:
	    return "the Arch Lady of the Church";
	    break;
	case 20:
	    return "the Matriarch";
	    break;
	    /* no one ever thought up these titles 21-30 */
	case LVL_IMMORT:
	    return "the Immortal Priestess";
	    break;
	case LVL_ADMIN:
	    return "the Inquisitress";
	    break;
	case LVL_BUILDER:
	    return "the Goddess of good and evil";
	    break;
	default:
	    return "the Cleric";
	    break;
	}
	break;

    case CLASS_THIEF:
	switch (level) {
	case 1:
	    return "the Pilferess";
	    break;
	case 2:
	    return "the Footpad";
	    break;
	case 3:
	    return "the Filcheress";
	    break;
	case 4:
	    return "the Pick-Pocket";
	    break;
	case 5:
	    return "the Sneak";
	    break;
	case 6:
	    return "the Pincheress";
	    break;
	case 7:
	    return "the Cut-Purse";
	    break;
	case 8:
	    return "the Snatcheress";
	    break;
	case 9:
	    return "the Sharpress";
	    break;
	case 10:
	    return "the Rogue";
	    break;
	case 11:
	    return "the Robber";
	    break;
	case 12:
	    return "the Magswoman";
	    break;
	case 13:
	    return "the Highwaywoman";
	    break;
	case 14:
	    return "the Burglaress";
	    break;
	case 15:
	    return "the Thief";
	    break;
	case 16:
	    return "the Knifer";
	    break;
	case 17:
	    return "the Quick-Blade";
	    break;
	case 18:
	    return "the Murderess";
	    break;
	case 19:
	    return "the Brigand";
	    break;
	case 20:
	    return "the Cut-Throat";
	    break;
	    /* no one ever thought up these titles 21-50 */
	case LVL_IMMORT:
	    return "the Immortal Assasin";
	    break;
	case LVL_ADMIN:
	    return "the Demi Goddess of thieves";
	    break;
	case LVL_BUILDER:
	    return "the Goddess of thieves and tradesmen";
	    break;
	default:
	    return "the Thief";
	    break;
	}
	break;

    case CLASS_GLADIATOR:
	switch (level) {
	case 1:
	    return "the Swordpupil";
	    break;
	case 2:
	    return "the Recruit";
	    break;
	case 3:
	    return "the Sentress";
	    break;
	case 4:
	    return "the Fighter";
	    break;
	default:
	    return "the Warrior";
	    break;
	}
	break;

    case CLASS_DRUID:
	switch (level) {
	case 1:
	    return "the Druid";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Druid";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Druid";
	    break;
	}
	break;
    case CLASS_DARK_KNIGHT:
	switch (level) {
	case 1:
	    return "the Dark Knight";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Dark Knight";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Dark Knight";
	    break;
	}
	break;
    case CLASS_MONK:
	switch (level) {
	case 1:
	    return "the Monk";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Monk";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Monk";
	    break;
	}
	break;
    case CLASS_PALADIN:
	switch (level) {
	case 1:
	    return "the Paladin";
	    break;
	    /* No more descriptions... */
	case LVL_IMMORT:
	    return "the immortal Paladin";
	    break;
	case LVL_BUILDER:
	    return "the builder";
	    break;
	default:
	    return "the Paladin";
	    break;
	}
	break;
    }

    /* Default title for classes which do not have titles defined */
    return "the Classless";
}
