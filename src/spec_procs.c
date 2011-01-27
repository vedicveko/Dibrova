/*  ***********************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

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
#include "clan.h"
#include "dg_scripts.h"

/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct command_info cmd_info[];
extern int drink_aff[][3];
extern int newbie_level;
extern struct zone_data *zone_table;
extern struct aq_data *aquest_table;
extern int top_of_aquestt;
extern struct obj_data *object_list;
extern int max_stat_table[NUM_RACES][7];
void mobsay(struct char_data *ch, const char *msg);
void do_stat_object(struct char_data *ch, struct obj_data *j);
extern int double_exp;

/* extern functions */
void add_follower(struct char_data *ch, struct char_data *leader);
extern int level_exp(int class, int level);
ACMD(do_say);
ACMD(do_flee);
ACMD(do_move);

struct social_type {
    char *cmd;
    int next_line;
};


/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */


SPECIAL(dump)
{
    struct obj_data *k;
    int value = 0;

    ACMD(do_drop);
    char *fname(char *namelist);

    for (k = world[ch->in_room].contents; k;
	 k = world[ch->in_room].contents) {
	act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
	extract_obj(k);
    }

    if (!CMD_IS("drop"))
	return 0;

    do_drop(ch, argument, cmd, 0);

    for (k = world[ch->in_room].contents; k;
	 k = world[ch->in_room].contents) {
	act("$p vanishes in a puff of smoke!", FALSE, 0, k, 0, TO_ROOM);
	value += MAX(1, MIN(50, GET_OBJ_COST(k) / 10));
	extract_obj(k);
    }

    if (value) {
	act("You are awarded for outstanding performance.", FALSE, ch, 0,
	    0, TO_CHAR);
	act("$n has been awarded for being a good citizen.", TRUE, ch, 0,
	    0, TO_ROOM);

	if (GET_LEVEL(ch) < 3)
	    gain_exp(ch, value);
	else
	    GET_GOLD(ch) += value;
    }
    return 1;
}


SPECIAL(mayor)
{
    ACMD(do_gen_door);

    static char open_path[] =
	"W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char close_path[] =
	"W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static char *path;
    static int index;
    static bool move = FALSE;

    if (!move) {
	if (time_info.hours == 6) {
	    move = TRUE;
	    path = open_path;
	    index = 0;
	} else if (time_info.hours == 20) {
	    move = TRUE;
	    path = close_path;
	    index = 0;
	}
    }
    if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
	(GET_POS(ch) == POS_FIGHTING)) return FALSE;

    switch (path[index]) {
    case '0':
    case '1':
    case '2':
    case '3':
	perform_move(ch, path[index] - '0', 1);
	break;

    case 'W':
	GET_POS(ch) = POS_STANDING;
	act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'S':
	GET_POS(ch) = POS_SLEEPING;
	act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0,
	    TO_ROOM);
	break;

    case 'a':
	act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
	act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'b':
	act
	    ("$n says 'What a view!  I must get something done about that dump!'",
	     FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'c':
	act
	    ("$n says 'Vandals!  Youngsters nowadays have no respect for anything!'",
	     FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'd':
	act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'e':
	act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0,
	    TO_ROOM);
	break;

    case 'E':
	act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0,
	    TO_ROOM);
	break;

    case 'O':
	do_gen_door(ch, "gate", 0, SCMD_UNLOCK);
	do_gen_door(ch, "gate", 0, SCMD_OPEN);
	break;

    case 'C':
	do_gen_door(ch, "gate", 0, SCMD_CLOSE);
	do_gen_door(ch, "gate", 0, SCMD_LOCK);
	break;

    case '.':
	move = FALSE;
	break;

    }

    index++;
    return FALSE;
}

SPECIAL(gym)
{
    int max;
    char buf[1024];
    char no_money[] = "You can't afford that.\r\n";
    char no_pracs[] = "You don't have enough practice sessions.\r\n";
    char maxed_out[] = "You can't improve that stat any more.\r\n";

    if (!CMD_IS("train"))
	return 0;		/* Ignore it and move along. */

    /* Really should keep in synch with affect_total (handler.c) */
    if (GET_LEVEL(ch) < 50)
	max = 18;
    else if (GET_LEVEL(ch) < 75)
	max = 20;
    else if (GET_LEVEL(ch) < 100)
	max = 22;
    else if (GET_LEVEL(ch) < 125)
	max = 23;
    else			/* 100+ */
	max = 25;

    one_argument(argument, arg);	/* Get the arg (the stat to train.) */

    if (!*arg) {
	/* List the trainable stats. */
	sprintf(buf,
		"/cwYou can Train the following stats for the following costs:/c0\r\n\r\n"
		"  Stat          Gold    Practices\r\n"
		"  -------------------------------\r\n"
		"  /cWStrength      /cy50,000      3/c0\r\n"
		"  /cWDexterity     /cy50,000      3/c0\r\n"
		"  /cWConstitution  /cy50,000      2/c0\r\n"
		"  /cWCharisma      /cy50,000      1/c0\r\n\r\n"
		"  Affect        Exp\r\n"
		"  -------------------------------\r\n"
		"  /cWNoHunger     /cy25,000,000/c0\r\n"
		"  /cWNoThirst     /cy25,000,000/c0\r\n\r\n"
		" /cgYou have %d practice sessions remaining./c0\r\n",
		GET_PRACTICES(ch));
	send_to_char(buf, ch);
	return 1;
    } else {
	/* Attempt to practice... */
	if (isname(arg, "strength")) {
	    /* Strength */
	    if (ch->real_stats.str >= max_stat_table[(int)GET_RACE(ch)][3]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 3) {
		    send_to_char
			("You hit the weights.  You feel much stronger.\r\n",
			 ch);
		    ch->real_stats.str++;
		    GET_STR(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 3;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	} else if (isname(arg, "dexterity")) {
	    /* Dexterity */
	    if (ch->real_stats.dex >= max_stat_table[(int)GET_RACE(ch)][2]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 3) {
		    send_to_char
			("You do something.  You feel much more dexterous.\r\n",
			 ch);
		    ch->real_stats.dex++;
		    GET_DEX(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 3;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	} else if (isname(arg, "constitution")) {
	    /* Constitution */
	    if (ch->real_stats.con >= max_stat_table[(int)GET_RACE(ch)][4]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 2) {
		    send_to_char
			("You feel different.  Your constitution improves.\r\n",
			 ch);
		    ch->real_stats.con++;
		    GET_CON(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 2;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	} else if (isname(arg, "charisma")) {
	    /* Charisma */
	    if (ch->real_stats.cha >= max_stat_table[(int)GET_RACE(ch)][5]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 1) {
		    send_to_char("You feel more charismatic.\r\n", ch);
		    ch->real_stats.cha++;
		    GET_CHA(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 1;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	} else if (isname(arg, "nohunger")) {
	    /* No Hunger */
	    if (GET_COND(ch, 1) == (char) -1) {
		send_to_char("You already have no hunger!\r\n", ch);
		return 1;
	    }
	    if (GET_LEVEL(ch) < 50) {
		send_to_char
		    ("You must be level 50 or above to purchase that.\r\n",
		     ch);
		return 1;
	    }
            if (GET_EXP(ch) <= 24999999) {
              send_to_char("You do not have enough experience to buy that!\r\n", ch);
              return 1;
            }
	    GET_COND(ch, 1) = (char) -1;
	    ch->points.exp = ch->points.exp - 25000000;
	    send_to_char
		("The trainer does something wierd to your stomache!\r\n",
		 ch);

	    return 1;
	} else if (isname(arg, "nothirst")) {
	    /* No Thirst */
	    if (GET_COND(ch, 2) == (char) -1) {
		send_to_char("You already have no thirst!\r\n", ch);
		return 1;
	    }
	    if (GET_LEVEL(ch) < 50) {
		send_to_char
		    ("You must be level 50 or above to purchase that.\r\n",
		     ch);
		return 1;
	    }
            if (GET_EXP(ch) <= 24999999) {
              send_to_char("You do not have enough experience to buy that!\r\n", ch);
              return 1;
            }
	    GET_COND(ch, 2) = (char) -1;
	    ch->points.exp = ch->points.exp - 25000000;
	    send_to_char
		("The trainer does something wierd to your stomache!\r\n",
		 ch);
	    return 1;
	}

    }
    return TRUE;
}


SPECIAL(library)
{
    int max;
    char buf[1024];
    char no_money[] = "You can't afford that.\r\n";
    char no_pracs[] = "You don't have enough practice sessions.\r\n";
    char maxed_out[] = "You can't improve that stat any more.\r\n";

    if (!CMD_IS("train"))
	return 0;		/* Ignore it and move along. */

    /* Really should keep in synch with affect_total (handler.c) */
    if (GET_LEVEL(ch) < 50)
	max = 18;
    else if (GET_LEVEL(ch) < 75)
	max = 20;
    else if (GET_LEVEL(ch) < 100)
	max = 22;
    else if (GET_LEVEL(ch) < 125)
	max = 23;
    else			/* 100+ */
	max = 25;

    one_argument(argument, arg);	/* Get the arg (the stat to train.) */

    if (!*arg) {
	/* List the trainable stats. */
	sprintf(buf,
		"/cwYou can Train the following stats for the following costs:/c0\r\n\r\n"
		"  Stat          Gold    Practices\r\n"
		"  -------------------------------\r\n"
		"  /cWIntelligence  /cy50,000      2/c0\r\n"
		"  /cWWisdom        /cy50,000      2/c0\r\n"
		" /cgYou have %d practice sessions remaining./c0\r\n",
		GET_PRACTICES(ch));
	send_to_char(buf, ch);
	return 1;
    } else {
	/* Attempt to practice... */
	if (isname(arg, "intelligence")) {
	    /* Intelligence */
	    if (ch->real_stats.intel >= max_stat_table[(int)GET_RACE(ch)][0]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 2) {
		    send_to_char
			("You hit the books.  You feel smarter.\r\n", ch);
		    ch->real_stats.intel++;
		    GET_INT(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 2;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	} else if (isname(arg, "wisdom")) {
	    /* Wisdom */
	    if (ch->real_stats.wis >= max_stat_table[(int)GET_RACE(ch)][1]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 2) {
		    send_to_char("You feel wiser.\r\n", ch);
		    ch->real_stats.wis++;
		    GET_WIS(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 2;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	}
    }
    return TRUE;
}




/* The lava beast should attack when a player tries to take his sleeves. */
SPECIAL(lava_beast)
{

    struct obj_data *obj2take;
    struct char_data *lava_beast;

    /* Don't mind the creepies... */
    if (IS_MOB(ch))
	return FALSE;

    if (!(lava_beast = get_char_room("beast", IN_ROOM(ch)))) {
	send_to_char("This is bad.  Report to a god.\r\n", ch);
	return TRUE;
    }


    if (CMD_IS("take") || CMD_IS("get")) {
	argument = one_argument(argument, buf);
	obj2take = get_obj_vis(ch, buf);

	if (!obj2take && !(buf == "all"))
	    return FALSE;
	else {
	    send_to_char("Pick a fight.\r\n", ch);
	    return TRUE;
	}
    }

    return FALSE;
}

SPECIAL(ankou_follower)
{

    struct char_data *master;

    /* players don't need to be here. */
    if (!IS_NPC(ch))
	return FALSE;

    /* Not the guy we are worried about.  Leave this mob alone. */
    if (GET_MOB_VNUM(ch) != 1404)
	return FALSE;

    switch (GET_MOB_VNUM(ch)) {
    case 1404:
	master = get_char_room("Ankou", IN_ROOM(ch));
	break;
    default:
	return FALSE;
	break;
    }

    if (master) {
	if (ch->master && IS_MOVE(cmd))
	    /* Leave while your lord is here?!?  No way! */
	    return TRUE;
	if (!ch->master) {
	    /* Master is here and you aren't following?  Now you are. */
	    add_follower(ch, master);
	    return TRUE;
	}
    }

    return FALSE;		/* You are probably following your master... */
}


SPECIAL(sergeant_follower)
{

    struct char_data *master;

    /* players don't need to be here. */
    if (!IS_NPC(ch))
	return FALSE;

    switch (GET_MOB_VNUM(ch)) {
    case 1800:
    case 1801:
    case 1802:
	master = get_char_room("823487", IN_ROOM(ch));
	break;
    default:
	return FALSE;
	break;
    }

    if (master) {
	if (ch->master && IS_MOVE(cmd))
	    /* Leave while your sergeant is here?!?  No way! */
	    return TRUE;
	if (!ch->master) {
	    /* Master is here and you aren't following?  Now you are. */
	    add_follower(ch, master);
	    return TRUE;
	}
    }

    return FALSE;		/* You are probably following your master... */
}

/* ********************************************************************
*  General special procedures for mobiles                             *
******************************************************************** */


void npc_steal(struct char_data *ch, struct char_data *victim)
{
    int gold;

    if (IS_NPC(victim))
	return;
    if (GET_LEVEL(victim) >= LVL_IMMORT)
	return;

    if (AWAKE(victim)
	&& (number(0, GET_LEVEL(ch) + (GET_LEVEL(ch) >> 1)) == 0)) {
	act("You discover that $n has $s hands in your wallet.", FALSE, ch,
	    0, victim, TO_VICT);
	act("$n tries to steal gold from $N.", TRUE, ch, 0, victim,
	    TO_NOTVICT);
    } else {
	/* Steal some gold coins */
	gold = (int) ((GET_LEVEL(ch) * number(1, 5)));
	if (gold > 0 && GET_GOLD(ch) > gold) {
	    GET_GOLD(ch) += gold;
	    GET_GOLD(victim) -= gold;
	    act("$n steals some gold from $N.", TRUE, ch, 0, victim,
		TO_NOTVICT);
	    sprintf(buf, "THIEF: %s steals %d gold from %s\r\n",
		    GET_NAME(ch), gold, GET_NAME(victim));
	    log(buf);
	}
    }
}


SPECIAL(snake)
{
    if (cmd)
	return FALSE;

    if (GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
	(number(0, 42 - GET_LEVEL(ch)) == 0)) {
	act("$n bites $N!", 1, ch, 0, FIGHTING(ch), TO_NOTVICT);
	act("$n bites you!", 1, ch, 0, FIGHTING(ch), TO_VICT);
	call_magic(ch, FIGHTING(ch), 0, SPELL_POISON, ABT_SPELL,
		   GET_LEVEL(ch), CAST_SPELL, FALSE);
	return TRUE;
    }
    return FALSE;
}


SPECIAL(thief)
{
    struct char_data *cons;

    if (cmd)
	return FALSE;

    if (GET_POS(ch) != POS_STANDING)
	return FALSE;

    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
	if (!IS_NPC(cons) && (GET_LEVEL(cons) < LVL_IMMORT)
	    && (!number(0, 4))) {
	    npc_steal(ch, cons);
	    return TRUE;
	}
    return FALSE;
}


SPECIAL(flaming_efreet)
{

    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (FIGHTING(vict) == ch && !number(0, 4))
	    break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
	vict = FIGHTING(ch);

    if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0))
	cast_spell(ch, vict, NULL, SPELL_FIREBALL, ABT_SPELL, FALSE);

    return TRUE;

}

/* Guard will keep non-newbies out of newbie zone. */
SPECIAL(newbie_guard)
{

    if (!cmd)
	return FALSE;

    if (!IS_MOB(ch) && IS_MOVE(cmd)) {
	if (CMD_IS("west") && GET_LEVEL(ch) >= newbie_level &&
	    GET_LEVEL(ch) < LVL_IMMORT) {
	    act
		("Lord Nelvaar jabs his finger in the middle of your chest and "
		 "growls \"You're not welcome here!\"", FALSE, ch, 0, 0,
		 TO_CHAR);
	    act
		("Lord Nelvaar jabs his finger in the middle of $n's chest and "
		 "growls \"You're not welcome here!\"", FALSE, ch, 0, 0,
		 TO_ROOM);
	    return TRUE;
	}
    }

    return FALSE;

}


SPECIAL(simple_heal)
{

    struct char_data *vict;

    if (cmd)
	return FALSE;

    /* pseudo-randomly choose someone in the room to help */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (!IS_MOB(vict) && !number(0, 1))
	    break;

    if (!vict)
	return FALSE;

    GET_MAX_MANA(ch) = 1000;

    switch (number(0, 25)) {
    case 0:
	cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
	break;
    case 1:
	cast_spell(ch, vict, NULL, SPELL_CURE_CRITIC, ABT_SPELL, FALSE);
	break;
    case 2:
	cast_spell(ch, vict, NULL, SPELL_HEAL, ABT_SPELL, FALSE);
	break;
    case 3:
	cast_spell(ch, vict, NULL, SPELL_BLESS, ABT_SPELL, FALSE);
	break;
    case 4:
	cast_spell(ch, vict, NULL, SPELL_HASTE, ABT_SPELL, FALSE);
	break;
    case 5:
	cast_spell(ch, vict, NULL, SPELL_REGENERATE, ABT_SPELL, FALSE);
	break;
    }
    return TRUE;

}


SPECIAL(newbie_helper)
{

    struct char_data *vict;

    if (cmd)
	return FALSE;

    /* pseudo-randomly choose someone in the room to help */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (!IS_MOB(vict) && !number(0, 4))
	    break;

    if (!vict)
	return FALSE;

    GET_MAX_MANA(ch) = 100;

    switch (number(0, 30)) {
    case 0:
    case 1:
    case 2:
	cast_spell(ch, vict, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
	break;
    case 3:
    case 4:
    case 5:
	cast_spell(ch, vict, NULL, SPELL_BLESS, ABT_SPELL, FALSE);
	break;
    case 6:
    case 7:
	cast_spell(ch, vict, NULL, SPELL_STRENGTH, ABT_SPELL, FALSE);
	break;
    case 8:
	cast_spell(ch, vict, NULL, SPELL_SANCTUARY, ABT_SPELL, FALSE);
	break;
    }
    return TRUE;

}


SPECIAL(magic_user)
{
    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (FIGHTING(vict) == ch && !number(0, 4))
	    break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
	vict = FIGHTING(ch);

    if ((GET_LEVEL(ch) > 13) && (number(0, 10) == 0))
	cast_spell(ch, vict, NULL, SPELL_SLEEP, ABT_SPELL, FALSE);

    if ((GET_LEVEL(ch) > 7) && (number(0, 8) == 0))
	cast_spell(ch, vict, NULL, SPELL_BLINDNESS, ABT_SPELL, FALSE);

    if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
	if (IS_EVIL(ch))
	    cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN, ABT_SPELL,
		       FALSE);
	else if (IS_GOOD(ch))
	    cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL, ABT_SPELL,
		       FALSE);
    }
    if (number(0, 4))
	return TRUE;

    switch (GET_LEVEL(ch)) {
    case 4:
    case 5:
	cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE, ABT_SPELL, FALSE);
	break;
    case 6:
    case 7:
	cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH, ABT_SPELL, FALSE);
	break;
    case 8:
    case 9:
	cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS, ABT_SPELL, FALSE);
	break;
    case 10:
    case 11:
	cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP, ABT_SPELL, FALSE);
	break;
    case 12:
    case 13:
	cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT, ABT_SPELL, FALSE);
	break;
    case 14:
    case 15:
    case 16:
    case 17:
	cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY, ABT_SPELL, FALSE);
	break;
    default:
	cast_spell(ch, vict, NULL, SPELL_FIREBALL, ABT_SPELL, FALSE);
	break;
    }
    return TRUE;

}


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

SPECIAL(guild_guard)
{
    int i;
    extern int guild_info[][3];
    struct char_data *guard = (struct char_data *) me;
    char *buf = "The guard humiliates you, and blocks your way.\r\n";
    char *buf2 = "The guard humiliates $n, and blocks $s way.";

    if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND))
	return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
	return FALSE;

    for (i = 0; guild_info[i][0] != -1; i++) {
	if (GET_CLASS(ch) >= 9) {
	    return FALSE;
	}
	if ((IS_NPC(ch) || GET_CLASS(ch) != guild_info[i][0]) &&
	    world[ch->in_room].number == guild_info[i][1] &&
	    cmd == guild_info[i][2]) {
	    send_to_char(buf, ch);
	    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
    }

    return FALSE;
}



SPECIAL(puff)
{

    if (cmd)
	return (0);

    switch (number(0, 60)) {
    case 0:
	do_say(ch, "My god!  It's full of stars!", 0, 0);
	return (1);
    case 1:
	do_say(ch, "How'd all those fish get up here?", 0, 0);
	return (1);
    case 2:
	do_say(ch, "I'm a very female dragon.", 0, 0);
	return (1);
    case 3:
	do_say(ch, "I've got a peaceful, easy feeling.", 0, 0);
	return (1);
    default:
	return (0);
    }
}



SPECIAL(fido)
{

    struct obj_data *i, *temp, *next_obj;

    if (cmd || !AWAKE(ch))
	return (FALSE);

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
	if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
	    if (GET_OBJ_EXP(i) < 0) {
		continue;
	    }
	    act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
	    for (temp = i->contains; temp; temp = next_obj) {
		next_obj = temp->next_content;
		obj_from_obj(temp);
		obj_to_room(temp, ch->in_room);
	    }
	    extract_obj(i);
	    return (TRUE);
	}
    }
    return (FALSE);
}



SPECIAL(janitor)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch) || FIGHTING(ch))
	return FALSE;

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
	if (!CAN_WEAR(i, ITEM_WEAR_TAKE))
	    continue;
	if (GET_OBJ_TYPE(i) != ITEM_DRINKCON && GET_OBJ_COST(i) >= 15)
	    continue;
	if (GET_OBJ_EXP(i) < 0)
	    continue;		/* It's a player corpse - hands off */
	act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
	obj_from_room(i);
	obj_to_char(i, ch);
	return TRUE;
    }

    return FALSE;
}

SPECIAL(cityguard)
{
    struct char_data *tch, *evil, *vict;
    int max_evil;
    int att_type = 0;


    if (cmd || !AWAKE(ch))
	return FALSE;

    if ((GET_POS(ch) == POS_STANDING) && !FIGHTING(ch)) {
	switch (number(0, 60)) {
	case 0:
	    act("$n eyes you suspiciously.", TRUE, ch, 0, 0, TO_ROOM);
	    break;
	case 1:
	case 2:
	case 3:
	    act
		("$n keeps his hand on the hilt of $s weapon while near you.",
		 TRUE, ch, 0, 0, TO_ROOM);
	    break;
	case 4:
	case 5:
	    act("$n stops suddenly as if having heard something odd.",
		TRUE, ch, 0, 0, TO_ROOM);
	    act("After a few moments, $n continues on $s way.", TRUE, ch,
		0, 0, TO_ROOM);

	    break;
	case 7:
	    act("$n casts you a wary glance.", TRUE, ch, 0, 0, TO_ROOM);
	    break;
	case 9:
	    act("$n growls at you.", TRUE, ch, 0, 0, TO_ROOM);
	    break;
	case 10:
	    act("$n glares icily at you.", TRUE, ch, 0, 0, TO_ROOM);
	    break;
	case 11:
	    act("$n gives you a casual glance.", TRUE, ch, 0, 0, TO_ROOM);
	    break;
	default:
	    return FALSE;
	    break;
	}
    }


    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {
	vict = FIGHTING(ch);

	if (number(1, 5) == 5) {
	    act("$n says, 'Yield dog!'.", 1, ch, 0, 0, TO_ROOM);
	}
	if (GET_POS(ch) > POS_SITTING) {

	    att_type = number(1, 11);
	    switch (att_type) {
	    case 1:
		do_mob_bash(ch, vict);
		break;
	    case 3:
		do_generic_skill(ch, vict, SKILL_KICK, 0);
		break;
	    case 5:
		do_generic_skill(ch, vict, SKILL_ROUNDHOUSE, 0);
		break;
	    case 6:
		do_mob_bash(ch, vict);
		break;
	    case 7:
		do_generic_skill(ch, vict, SKILL_BODYSLAM, 0);
		break;
	    default:
		return FALSE;
		break;
	    }
	}
    }

    if (!FIGHTING(ch)) {
	max_evil = 1000;
	evil = 0;

	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	    if (!IS_NPC(tch) && CAN_SEE(ch, tch)
		&& IS_SET_AR(PLR_FLAGS(tch), PLR_KILLER)) {
		act
		    ("$n screams 'HEY!!!  You're one of those PLAYER KILLERS!!!!!!'",
		     FALSE, ch, 0, 0, TO_ROOM);
		hit(ch, tch, TYPE_UNDEFINED);
		return (TRUE);
	    }
	}

	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	    if (!IS_NPC(tch) && CAN_SEE(ch, tch)
		&& IS_SET_AR(PLR_FLAGS(tch), PLR_THIEF)) {
		act
		    ("$n screams 'HEY!!!  You're one of those PLAYER THIEVES!!!!!!'",
		     FALSE, ch, 0, 0, TO_ROOM);
		hit(ch, tch, TYPE_UNDEFINED);
		return (TRUE);
	    }
	}

	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	    if (CAN_SEE(ch, tch) && FIGHTING(tch)) {
		if ((GET_ALIGNMENT(tch) < max_evil) &&
		    (IS_NPC(tch) || IS_NPC(FIGHTING(tch)))) {
		    max_evil = GET_ALIGNMENT(tch);
		    evil = tch;
		}
	    }
	}

	if (evil && (GET_ALIGNMENT(FIGHTING(evil)) >= 0)) {
	    act
		("$n screams 'PROTECT THE INNOCENT!  BANZAI!  CHARGE!  ARARARAGGGHH!'",
		 FALSE, ch, 0, 0, TO_ROOM);
	    hit(ch, evil, TYPE_UNDEFINED);
	    return (TRUE);
	}

	return (FALSE);
    }
    return FALSE;
}

#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)

SPECIAL(pet_shops)
{
    char buf[MAX_STRING_LENGTH], pet_name[256];
    int pet_room;
    struct char_data *pet;

    pet_room = ch->in_room + 1;

    if (CMD_IS("list")) {
	send_to_char("Available pets are:\r\n", ch);
	for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
	    sprintf(buf, "%8d - %s\r\n", PET_PRICE(pet), GET_NAME(pet));
	    send_to_char(buf, ch);
	}
	return (TRUE);
    } else if (CMD_IS("buy")) {

	argument = one_argument(argument, buf);
	argument = one_argument(argument, pet_name);

	if (!(pet = get_char_room(buf, pet_room))) {
	    send_to_char("There is no such pet!\r\n", ch);
	    return (TRUE);
	}
	if (GET_GOLD(ch) < PET_PRICE(pet)) {
	    send_to_char("You don't have enough gold!\r\n", ch);
	    return (TRUE);
	}
	GET_GOLD(ch) -= PET_PRICE(pet);

	pet = read_mobile(GET_MOB_RNUM(pet), REAL);
	GET_EXP(pet) = 0;
	SET_BIT_AR(AFF_FLAGS(pet), AFF_CHARM);

	if (*pet_name) {
	    sprintf(buf, "%s %s", pet->player.name, pet_name);
	    /* free(pet->player.name); don't free the prototype! */
	    pet->player.name = str_dup(buf);

	    sprintf(buf,
		    "%sA small sign on a chain around the neck says 'My name is %s'\r\n",
		    pet->player.description, pet_name);
	    /* free(pet->player.description); don't free the prototype! */
	    pet->player.description = str_dup(buf);
	}
	char_to_room(pet, ch->in_room);
	add_follower(pet, ch);
	load_mtrigger(pet);

	/* Be certain that pets can't get/carry/use/wield/wear items */
	IS_CARRYING_W(pet) = 1000;
	IS_CARRYING_N(pet) = 100;

	send_to_char("May you enjoy your pet.\r\n", ch);
	act("$n buys $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

	return 1;
    }
    /* All commands except list and buy */
    return 0;
}

/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
    int amount;

    if (CMD_IS("balance")) {
	if (GET_BANK_GOLD(ch) > 0)
	    sprintf(buf, "Your current balance is %d coins.\r\n",
		    GET_BANK_GOLD(ch));
	else
	    sprintf(buf, "You currently have no money deposited.\r\n");
	send_to_char(buf, ch);
	return 1;
    } else if (CMD_IS("deposit")) {
	if ((amount = atoi(argument)) <= 0) {
	    send_to_char("How much do you want to deposit?\r\n", ch);
	    return 1;
	}
	if (GET_GOLD(ch) < amount) {
	    send_to_char("You don't have that many coins!\r\n", ch);
	    return 1;
	}
	GET_GOLD(ch) -= amount;
	GET_BANK_GOLD(ch) += amount;
	sprintf(buf, "You deposit %d coins.\r\n", amount);
	send_to_char(buf, ch);
	act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
	return 1;
    } else if (CMD_IS("withdraw")) {
	if ((amount = atoi(argument)) <= 0) {
	    send_to_char("How much do you want to withdraw?\r\n", ch);
	    return 1;
	}
	if (GET_BANK_GOLD(ch) < amount) {
	    send_to_char("You don't have that many coins deposited!\r\n",
			 ch);
	    return 1;
	}
	GET_GOLD(ch) += amount;
	GET_BANK_GOLD(ch) -= amount;
	sprintf(buf, "You withdraw %d coins.\r\n", amount);
	send_to_char(buf, ch);
	act("$n makes a bank transaction.", TRUE, ch, 0, FALSE, TO_ROOM);
	return 1;
    } else
	return 0;
}

SPECIAL(kaan_altar)
{

    if (!CMD_IS("kneel"))
	return FALSE;

    argument = one_argument(argument, buf);

    if (!*buf) {
	send_to_char("Kneel where?\r\n", ch);
	return TRUE;
    }

    if (GET_CLASS(ch) != CLASS_THIEF) {
	switch (number(1, 5)) {
	case 1:
	    send_to_char
		("A robed priest rushes over and tells you to fuck off.\r\n",
		 ch);
	    act("A robed priest rushes in and tells $n to fuck off.\r\n",
		FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	case 2:
	    send_to_char
		("A robed priest rushes over and pisses on your feet.\r\n",
		 ch);
	    act("A robed priest rushes in and pisses $n's feet.\r\n",
		FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	case 3:
	    send_to_char
		("A robed priest rushes over and kicks you in the ass.\r\n",
		 ch);
	    act("A robed priest rushes in and kicks $n in the ass.\r\n",
		FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	case 4:
	    send_to_char
		("A robed priest rushes over and tells you to go away.\r\n",
		 ch);
	    act("A robed priest rushes in and tells $n to go away.\r\n",
		FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	case 5:
	    send_to_char
		("A robed priest rushes over, scratches his ass and jams his finger in your ear.\r\n",
		 ch);
	    act
		("A robed priest rushes in, scratches his ass and jams his finger in $n's ear.\r\n",
		 FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	default:
	    send_to_char("Fucking bugs...\r\n", ch);
	    act("Fucking bugs...\r\n", FALSE, ch, NULL, NULL, TO_ROOM);
	    break;
	}
	return TRUE;
    }

    if (!isname(buf, "altar")) {
	send_to_char("You can't kneel there.\r\n", ch);
	return TRUE;
    }

    send_to_char
	("As you kneel to pray to Holy Kaan, a robed priest comes \r\n"
	 "over to give you the sacriment.  He tilts the Holy Mug of \r\n"
	 "Beer to your lips, allowing you only one slurp, and places \r\n"
	 "a pretzel on your tongue.\r\n", ch);
    act("$n kneels to receive the Holy Sacriment.\r\n", FALSE, ch, NULL,
	NULL, TO_ROOM);

    /* drink_aff 2 is beer. */
    gain_condition(ch, DRUNK, (int) ((int) drink_aff[2][DRUNK] * 1) / 4);

    gain_condition(ch, FULL, (int) ((int) drink_aff[2][FULL] * 1) / 4);

    gain_condition(ch, THIRST, (int) ((int) drink_aff[2][THIRST] * 1) / 4);

    return TRUE;
}

#define BLOOD_BANK_HIT_COST 10
#define BLOOD_BANK_GOLD_PAY 10
SPECIAL(blood_bank)
{
    if (CMD_IS("give")) {
	one_argument(argument, arg);	/* Get the arg */
	if (*arg && !strcmp(arg, "blood")) {
	    if (GET_HIT(ch) < (BLOOD_BANK_HIT_COST + 1)) {	/* Don't let them go below 1 hp. */
		send_to_char
		    ("You are in no condition to give blood right now.\r\n",
		     ch);
		return (TRUE);
	    } else {
		send_to_char
		    ("The nurse takes your blood.  You feel somewhat weaker.\r\nThe nurse gives you a handful of coins and you start to feel better.\r\n",
		     ch);
		GET_HIT(ch) -= BLOOD_BANK_HIT_COST;
		GET_GOLD(ch) += BLOOD_BANK_GOLD_PAY;
		act("The nurse drains some blood from $n.", FALSE, ch, 0,
		    ch, TO_ROOM);
		return (TRUE);
	    }
	}
    }
    return (FALSE);
}

SPECIAL(street_tough)
{
    struct char_data *tch;

    if (!IS_NPC(ch) || GET_MOB_VNUM(ch) != 3031)
	return FALSE;

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	if ((!IS_NPC(tch)) && (!FIGHTING(tch)) &&
	    (GET_CLASS(tch) != CLASS_THIEF)
	    && (GET_LEVEL(tch) < LVL_IMMORT)) {
	    hit(ch, tch, TYPE_UNDEFINED);
	    return (TRUE);
	}
    }
    return FALSE;		/*  Didn't find anyone.  Oh well. */
}

SPECIAL(pool)
{

    if (!CMD_IS("jump"))
	return FALSE;

    argument = one_argument(argument, buf);

    if (!*buf) {
	send_to_char("Jump in what?\r\n", ch);
	return TRUE;
    }

    if (!isname(buf, "pool water")) {
	send_to_char("You can't jump in that.\r\n", ch);
	return TRUE;
    }

    send_to_char("\r\nYou watch as the cool water of the pond closes "
		 "around your body, and although you see the water "
		 "surrounding you, you can't feel it at all, all "
		 "can feel as the sensation of falling... falling..."
		 "\r\n\r\n", ch);

    act("$n leaps into the pool.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(6422));
    act("$n appears.  Seemingly from nowhere!", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, FALSE);

    return TRUE;

}

SPECIAL(chute)
{

    if (!CMD_IS("jump"))
	return FALSE;

    argument = one_argument(argument, buf);

    if (!*buf) {
	send_to_char("Jump in what?\r\n", ch);
	return TRUE;
    }

    if (!isname(buf, "chute")) {
	send_to_char("You can't jump in that.\r\n", ch);
	return TRUE;
    }

    send_to_char
	("\r\nYou jump into the chute and beginning sliding at first\r\n"
	 "slowly, then pick up speed and begin hurtling down, down,\r\n"
	 "down - landing finally with a bone-jarring /ccTHUMP!/c0"
	 "\r\n\r\n", ch);


    act("$n leaps into the chute.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, real_room(18117));
    act("$n slides into the room, falling from the chute.", FALSE, ch, 0,
	0, TO_ROOM);
    look_at_room(ch, FALSE);

    return TRUE;

}

SPECIAL(casino)
{
    int wager;
    char buf[128];

    if (!CMD_IS("gamble"))
	return 0;

    one_argument(argument, arg);	/* Get the arg (amount to wager.) */

    if (!*arg) {
	send_to_char("How much?\r\n", ch);
	return 1;
    } else
	wager = atoi(arg);

    if (wager <= 0) {
	send_to_char("Very funny, dick-head.\r\n", ch);
	return 1;
    } else if (wager > GET_GOLD(ch)) {
	send_to_char("You don't have that much gold to wager.\r\n", ch);
	return 1;
    } else {
	/*  Okay - gamble away! */
	send_to_char("You gamble.\r\n", ch);
	act("$n makes a wager.", FALSE, ch, 0, 0, TO_ROOM);
	if (number(1, 113 - GET_LUCK(ch)) >= 70) {	/* 30 % chance of winning. */
	    /* You win! */
	    sprintf(buf,
		    "You win!  The dealer hands you %d gold coins.\r\n",
		    wager);
	    send_to_char(buf, ch);
	    GET_GOLD(ch) += wager;
	} else {
	    /* You lose */
	    sprintf(buf,
		    "You lose your wager of %d coins.\r\n  The dealer takes your gold.\r\n",
		    wager);
	    send_to_char(buf, ch);
	    GET_GOLD(ch) -= wager;
	}
	return 1;
    }
}

SPECIAL(play_war)
{
    int wager;
    int player_card;
    int dealer_card;
    char buf[128];
    static char *cards[] =
	{ "One", "Two", "Three", "Four", "Five", "Six", "Seven",
	"Eight", "Nine", "Ten", "Prince", "Knight", "Wizard"
    };
    static char *suits[] = { "Wands", "Daggers", "Wings", "Fire" };

    if (!CMD_IS("gamble"))
	return 0;

    one_argument(argument, arg);	/* Get the arg (amount to wager.) */

    if (!*arg) {
	send_to_char("How much?\r\n", ch);
	return 1;
    } else
	wager = atoi(arg);

    if (wager <= 0) {
	send_to_char("Very funny, dick-head.\r\n", ch);
	return 1;
    } else if (wager > GET_GOLD(ch)) {
	send_to_char("You don't have that much gold to wager.\r\n", ch);
	return 1;
    } else {
	/*  Okay - gamble away! */
	player_card = number(0, 12);
	dealer_card = number(0, 12);
	sprintf(buf, "You are dealt a %s of %s.\r\n"
		"The dealer turns up a %s of %s.\r\n",
		cards[player_card], suits[number(0, 3)],
		cards[dealer_card], suits[number(0, 3)]);
	send_to_char(buf, ch);
	if (player_card > dealer_card) {
	    /* You win! */
	    sprintf(buf,
		    "You win!  The dealer hands you %d gold coins.\r\n",
		    wager);
	    act("$n makes a wager and wins!", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char(buf, ch);
	    GET_GOLD(ch) += wager;
	} else if (dealer_card > player_card) {
	    /* You lose */
	    sprintf(buf, "You lose your wager of %d coins.\r\n"
		    "The dealer takes your gold.\r\n", wager);
	    act("$n makes a wager and loses.", FALSE, ch, 0, 0, TO_ROOM);
	    send_to_char(buf, ch);
	    GET_GOLD(ch) -= wager;
	} else {
	    /* WAR! */
	    while (player_card == dealer_card) {
		send_to_char("/cRWar!/c0\r\n", ch);
		player_card = number(0, 12);
		dealer_card = number(0, 12);
		sprintf(buf, "You are dealt a %s of %s.\r\n"
			"The dealer turns of a %s of %s.\r\n",
			cards[player_card], suits[number(0, 3)],
			cards[dealer_card], suits[number(0, 3)]);
		send_to_char(buf, ch);
	    }
	    if (player_card > dealer_card) {
		/* You win! */
		sprintf(buf, "You win!  The dealer hands you %d gold "
			"coins.\r\n", wager);
		act("$n makes a wager and wins!", FALSE, ch, 0, 0,
		    TO_ROOM);
		send_to_char(buf, ch);
		GET_GOLD(ch) += wager;
	    } else if (dealer_card > player_card) {
		/* You lose */
		sprintf(buf, "You lose your wager of %d coins.\r\n"
			"The dealer takes your gold.\r\n", wager);
		act("$n makes a wager and loses.", FALSE, ch, 0, 0,
		    TO_ROOM);
		send_to_char(buf, ch);
		GET_GOLD(ch) -= wager;
	    }
	}
	return 1;
    }
}

SPECIAL(slot_machine)
{
    struct obj_data *obj = me;
    char buf[128];
    int hit[3], i, win = 0;
    char *pics[] = {
	"/cL7/c0",
	"/cR7/c0",
	"/cW7/c0",
	"/cwBAR/c0",
	"/cYLemon/c0",
	"/cMPlum/c0"
    };
    /* 18 slots on each reel */
    int reel[] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5 };

    if (!CMD_IS("gamble"))
	return 0;

    if (!GET_GOLD(ch)) {
	send_to_char("You don't have any gold to gamble.\r\n", ch);
	return TRUE;
    }

    GET_GOLD(ch)--;
    act("You try your luck with $p.\r\n", FALSE, ch, obj, 0, TO_CHAR);
    act("$n tries $s luck with $p.\r\n", FALSE, ch, obj, 0, TO_ROOM);

    for (i = 0; i < 3; i++)
	hit[i] = reel[number(0, 17)];

    sprintf(buf, "The reels rattle and spin for a moment:\r\n"
	    "\r\n %s %s %s\r\n\r\n", pics[hit[0]], pics[hit[1]],
	    pics[hit[2]]);
    send_to_char(buf, ch);

    if (hit[0] == hit[1] && hit[0] == hit[2]) {
	switch (hit[0]) {
	case 0:		/* Black 7's */
	    win = 1000;
	    break;
	case 1:		/* Red 7's */
	    win = 500;
	    break;
	case 2:		/* White 7's */
	    win = 100;
	    break;
	case 3:		/* Bars */
	    win = 5;
	    break;
	case 4:		/* Lemon's don't count */
	case 5:		/* Plum's don't count */
	    win = 0;
	    break;
	}
    } else {
	if (hit[0] < 3 && hit[1] < 3 && hit[2] < 3)
	    win = 20;		/* All 7's */
    }

    if (win) {
	sprintf(buf, "You win!  %d gold coins flow from $p.", win);
	act(buf, FALSE, ch, obj, 0, TO_CHAR);
	act("$n wins!  Gold pours from %p.", FALSE, ch, obj, 0, TO_ROOM);
	GET_GOLD(ch) += win;
    } else {
	act("You lose.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n loses.", FALSE, ch, 0, 0, TO_ROOM);
    }

    return TRUE;
}

SPECIAL(hospital)
{
    char buf[256];
    int full_heal;


    if (CMD_IS("list")) {

	full_heal = ((GET_MAX_HIT(ch) - GET_HIT(ch)) * 1.5) +
	    (GET_MAX_MANA(ch) - GET_MANA(ch)) +
	    (GET_MAX_VIM(ch) - GET_VIM(ch)) +
	    (GET_MAX_MOVE(ch) - GET_MOVE(ch)) +
	    (GET_MAX_QI(ch) - GET_QI(ch));

	/* List hospital services and prices... */
	send_to_char("\r\n/cY        Services:/c0\r\n", ch);
	send_to_char("/cW_________________________/c0\r\n\r\n", ch);
	sprintf(buf, "/cY F/cyull Heal   -  %-5d coins /c0\r\n",
		full_heal);
	send_to_char(buf, ch);
	send_to_char("/cY C/cyure Poison -   220  coins /c0\r\n", ch);
	send_to_char("/cY B/cyandage     -   360  coins /c0\r\n", ch);
	send_to_char("/cY R/cyefresh     -    30  coins /c0\r\n", ch);
	send_to_char("/cW_________________________/c0\r\n", ch);
	send_to_char("\r\n\"What'll it be?\"\r\n", ch);
	return TRUE;
    } else if (CMD_IS("buy")) {
	/* buy a service... */
	argument = one_argument(argument, buf);

	full_heal = ((GET_MAX_HIT(ch) - GET_HIT(ch)) * 1.5) +
	    (GET_MAX_MANA(ch) - GET_MANA(ch)) +
	    (GET_MAX_VIM(ch) - GET_VIM(ch)) +
	    (GET_MAX_MOVE(ch) - GET_MOVE(ch)) +
	    (GET_MAX_QI(ch) - GET_QI(ch));

	switch (tolower(*buf)) {
	case 'f':
	    /*  The works */
	    if (GET_GOLD(ch) >= full_heal) {
		GET_HIT(ch) = GET_MAX_HIT(ch);
		GET_MOVE(ch) = GET_MAX_MOVE(ch);
		GET_MANA(ch) = GET_MAX_MANA(ch);
		GET_QI(ch) = GET_MAX_QI(ch);
		GET_VIM(ch) = GET_MAX_VIM(ch);
		affect_from_char(ch, SPELL_POISON, ABT_SPELL);	/* Just in case... */
		REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_POISON);
		GET_GOLD(ch) = GET_GOLD(ch) - full_heal;
		send_to_char
		    ("The good doctor patches you up.  You feel like "
		     "a mountain of gold!\r\n", ch);
		act("The doctor goes to work on $n", FALSE, ch, NULL, NULL,
		    TO_ROOM);
	    } else
		send_to_char
		    ("This isn't a charity clinic.  Come back when "
		     "you can pay.\r\n", ch);
	    break;
	case 'c':
	    /* Cure Poison */
	    if (IS_AFFECTED(ch, AFF_POISON)) {
		if (GET_GOLD(ch) >= 120) {
		    affect_from_char(ch, SPELL_POISON, ABT_SPELL);
		    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_POISON);
		    send_to_char
			("The doctor extracts the poison from your body.\r\n",
			 ch);
		    act("The doctor extracts the poison from $n.", FALSE,
			ch, NULL, NULL, TO_ROOM);
		    GET_GOLD(ch) -= 120;
		} else
		    send_to_char
			("This isn't a charity clinic.  Come back when you can pay.\r\n",
			 ch);
	    } else
		send_to_char("But you aren't poisoned!\r\n", ch);
	    break;
	case 'b':
	    /* Bandaging (10HP) */
	    if (GET_GOLD(ch) >= 360) {
		GET_HIT(ch) += number(8, 12);
		if (GET_HIT(ch) > GET_MAX_HIT(ch))
		    GET_HIT(ch) = GET_MAX_HIT(ch);	/* Don't go past the max... */
		send_to_char("The doctor applies some bandages.\r\n", ch);
		act("The doctor applies some bandages to $n.", FALSE, ch,
		    NULL, NULL, TO_ROOM);
		GET_GOLD(ch) -= 60;
	    } else
		send_to_char
		    ("This isn't a charity clinic.  Come back when you can pay.\r\n",
		     ch);
	    break;
	case 'r':
	    /* Revitalize */
	    if (GET_GOLD(ch) >= 30) {
		GET_MOVE(ch) += number(24, 26);
		GET_GOLD(ch) -= 30;
		send_to_char
		    ("The nurse give you an exquisite foot rub, you fell like you could walk forever!\r\n",
		     ch);
		act("The nurse tends to $n.", FALSE, ch, NULL, NULL,
		    TO_ROOM);
	    } else
		send_to_char
		    ("This isn't a charity clinic.  Come back when you can pay.\r\n",
		     ch);
	    break;
	default:
	    send_to_char("Speak up, please.  What do you want to buy?\r\n",
			 ch);
	    break;
	}
	return TRUE;
    }
    return FALSE;
}

/* Special message when falling out of the maze. */
SPECIAL(thieven_maze)
{

    int from_zone;
    int to_zone;

    if (!cmd || !IS_MOVE(cmd))
	return FALSE;

    if (!EXIT(ch, cmd - 1) || EXIT(ch, cmd - 1)->to_room == NOWHERE)
	return FALSE;


    from_zone = (int) world[IN_ROOM(ch)].number / 100;
    to_zone = (int) world[EXIT(ch, cmd - 1)->to_room].number / 100;

    if (from_zone != to_zone)
	send_to_char
	    ("/cWYou see a blinding flash of light as you step through "
	     "the shrubbery.  You have been teleported!/c0\r\n", ch);

    return FALSE;		/* So the actually MOVE stuff still happens. */

}

/* Mob func for a newbie guy to teleport newbies to a newbie zone. */
SPECIAL(newbie_teleport)
{

    extern int newbie_level;
    int newbie_eq[] = { 3081, 3086, 1009, 1041, 1044, 1065, 1031,
	3077, 3078, 3079, 3047, 3027, 3028, 3029, -1
    };
    int i, weapon;
    struct obj_data *obj;

    if (IS_NPC(ch))
	return FALSE;

    if (CMD_IS("newbie")) {

	if (GET_LEVEL(ch) > newbie_level) {
	    act("$N gives $n a look of indifference.", FALSE, ch, 0, me,
		TO_ROOM);
	    act("$N gives you an indifferent look.", FALSE, ch, 0, me,
		TO_CHAR);
	    return TRUE;
	}

	act("$N teleports $n to the newbie zone!", FALSE, ch, 0, me,
	    TO_ROOM);
	act("$N teleports you to the newbie zone!", FALSE, ch, 0, me,
	    TO_CHAR);

	char_from_room(ch);
	char_to_room(ch, real_room(3066));
	look_at_room(ch, 0);

	act("$n arrives!", FALSE, ch, 0, 0, TO_ROOM);

	return TRUE;
    }
    if (CMD_IS("outfit")) {
	if (GET_LEVEL(ch) >= newbie_level) {
	    send_to_char("Your level is too high to use this command.\r\n",
			 ch);
	    return TRUE;
	}
	if (IS_SET_AR(PLR_FLAGS(ch), PLR_OUTFITTED)) {
	    send_to_char("You have already been outfitted once!\r\n", ch);
	    return TRUE;
	}


	for (i = 0; newbie_eq[i] != -1; i++) {
	    obj = read_object(newbie_eq[i], VIRTUAL);
	    obj_to_char(obj, ch);
	}

	/*  Give a weapon specific to the class... */
	switch GET_CLASS
	    (ch) {
	case CLASS_SORCERER:
	case CLASS_CLERIC:
	    weapon = 1024;	/* Mace */
	    break;
	default:
	    weapon = 3020;	/* Dagger */
	    break;
	    }
	obj = read_object(weapon, VIRTUAL);
	obj_to_char(obj, ch);

	act("$N outfits $n for adventuring.", FALSE, ch, 0, me, TO_ROOM);
	act("$N outfits you adventuring.", FALSE, ch, 0, me, TO_CHAR);
	SET_BIT_AR(PLR_FLAGS(ch), PLR_OUTFITTED);
	return TRUE;
    }
    return FALSE;
}

/* Sundhaven Specials... */
SPECIAL(silktrader)
{
    if (cmd)
	return 0;

    if (world[ch->in_room].sector_type == SECT_CITY)
	switch (number(0, 30)) {
	case 0:
	    act("$n eyes a passing woman.", FALSE, ch, 0, 0, TO_ROOM);
	    do_say(ch,
		   "Come, m'lady, and have a look at this precious silk!",
		   0, 0);
	    return (1);
	case 1:
	    act("$n says to you, 'Wouldn't you look lovely in this!'",
		FALSE, ch, 0, 0, TO_ROOM);
	    act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,
		TO_ROOM);
	    return (1);
	case 2:
	    act("$n holds a pair of silk gloves up for you to inspect.",
		FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 3:
	    act
		("$n cries out, 'Have at this fine silk from exotic corners of the world you will likely never see!",
		 FALSE, ch, 0, 0, TO_ROOM);
	    act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 4:
	    do_say(ch, "Step forward, my pretty locals!", 0, 0);
	    return (1);
	case 5:
	    act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,
		TO_ROOM);
	    return (1);
	case 6:
	    do_say(ch, "Have you ever seen an ogre in a silken gown?", 0,
		   0);
	    do_say(ch, "I didn't *think* so!", 0, 0);
	    act("$n throws his head back and cackles with insane glee!",
		FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 7:
	    act("$n hands you a glass of wine.", FALSE, ch, 0, 0, TO_ROOM);
	    do_say(ch, "Come, have a seat and view my wares.", 0, 0);
	    return (1);
	case 8:
	    act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
	    act("$n shakes his head sadly.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 9:
	    act("$n fiddles with some maps.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 10:
	    do_say(ch,
		   "Here here! Beggars and nobles alike come forward and make your bids!",
		   0, 0);
	    return (1);
	case 11:
	    do_say(ch,
		   "I am in this bourgeois hamlet for a limited time only!",
		   0, 0);
	    act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,
		TO_ROOM);
	    return (1);
	}

    if (world[ch->in_room].sector_type != SECT_CITY)
	switch (number(0, 20)) {
	case 0:
	    do_say(ch,
		   "Ah! Fellow travellers! Come have a look at the finest silk this side of the infamous Ched Razimtheth!",
		   0, 0);
	    return (1);
	case 1:
	    act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
	    do_say(ch,
		   "You are feebly attired for the danger that lies ahead.",
		   0, 0);
	    do_say(ch, "Silk is the way to go.", 0, 0);
	    act("$n smiles warmly.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 2:
	    do_say(ch, "Worthy adventurers, hear my call!", 0, 0);
	    return (1);
	case 3:
	    act("$n adjusts his cloak.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 4:
	    act
		("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'",
		 FALSE, ch, 0, 0, TO_ROOM);
	    act("$n bows respectfully.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 5:
	    do_say(ch, "Can you direct me to the nearest tavern?", 0, 0);
	    return (1);
	case 6:
	    do_say(ch, "Heard the latest ogre joke?", 0, 0);
	    act("$n snickers to himself.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 7:
	    do_say(ch,
		   "What ho, traveller! Rest your legs here for a spell and peruse the latest in fashion!",
		   0, 0);
	    return (1);
	case 8:
	    do_say(ch,
		   "Beware ye, traveller, lest ye come to live in Exile!",
		   0, 0);
	    act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);
	case 9:
	    act("$n touches your shoulder.", FALSE, ch, 0, 0, TO_ROOM);
	    do_say(ch,
		   "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.",
		   0, 0);
	    act("$n shivers uncomfortably.", FALSE, ch, 0, 0, TO_ROOM);
	    return (1);

	}
    return (0);
}


SPECIAL(athos)
{
    if (cmd)
	return 0;
    switch (number(0, 20)) {
    case 0:
	act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 1:
	act("$n grimaces.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 2:
	act
	    ("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'",
	     FALSE, ch, 0, 0, TO_ROOM);
	do_say(ch, "That monster will be the death of us all.", 0, 0);
	return (1);
    case 3:
	do_say(ch, "God save the King!", 0, 0);
	return (1);
    case 4:
	do_say(ch, "All for one and .. one for...", 0, 0);
	act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return (1);
    case 5:
	act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,
	    TO_ROOM);
	do_say(ch, "Women - God's eternal punishment on man.", 0, 0);
	return (1);
    case 6:
	act("$n downs his glass and leans heavily on the oaken table.",
	    FALSE, ch, 0, 0, TO_ROOM);
	do_say(ch,
	       "You know, we would best band together and wrestle the monstrous woman from her lair and home!",
	       0, 0);
	return (1);
    default:
	return (FALSE);
	break;
    }
    return (0);
}



SPECIAL(hangman)
{
    if (cmd)
	return 0;
    switch (number(0, 15)) {
    case 0:
	act
	    ("$n whirls his noose like a lasso and it lands neatly around your neck.",
	     FALSE, ch, 0, 0, TO_ROOM);
	do_say(ch, "You're next, you ugly rogue!", 0, 0);
	do_say(ch, "Just kidding.", 0, 0);
	act("$n pats you on your head.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 1:
	do_say(ch,
	       "I was conceived in Exile and have been integrated into society!",
	       0, 0);
	do_say(ch, "Muahaha!", 0, 0);
	return (1);
    case 2:
	do_say(ch, "Anyone have a butterknife I can borrow?", 0, 0);
	return (1);
    case 3:
	act("$n suddenly pulls a lever.", FALSE, ch, 0, 0, TO_ROOM);
	act
	    ("With the flash of light on metal a giant guillotine comes crashing down!",
	     FALSE, ch, 0, 0, TO_ROOM);
	act("A head drops to the ground from the platform.", FALSE, ch, 0,
	    0, TO_ROOM);
	act("$n looks up and shouts wildly.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 4:
	act("$n whistles a local tune.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    default:
	return (FALSE);
	break;
    }
    return (0);
}



SPECIAL(butcher)
{
    if (cmd)
	return 0;
    switch (number(0, 40)) {
    case 0:
	do_say(ch, "I need a Union.", 0, 0);
	act("$n glares angrily.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n rummages about for an axe.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 1:
	act("$n gnaws on a toothpick.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 2:
	act("$n runs a finger along the edge of a giant meat cleaver.",
	    FALSE, ch, 0, 0, TO_ROOM);
	act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 3:
	do_say(ch, "Pork for sale!", 0, 0);
	return (1);
    case 4:
	act
	    ("$n whispers to you, 'I've got some great damage eq in the back room. Wanna see?'",
	     FALSE, ch, 0, 0, TO_ROOM);
	act("$n throws back his head and cackles with insane glee!", FALSE,
	    ch, 0, 0, TO_ROOM);
	return (1);
    case 5:
	act("$n yawns.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 6:
	act
	    ("$n throws an arm around the headless body of an ogre and asks to have his picture taken.",
	     FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 7:
	act
	    ("$n listlessly grabs a cleaver and hurls it into the wall behind your head.",
	     FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 8:
	act("$n juggles some fingers.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 9:
	act("$n eyes your limbs.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n chuckles.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 10:
	do_say(ch, "Hi, Alice.", 0, 0);
	return (1);
    case 11:
	do_say(ch, "Everyone looks like food to me these days.", 0, 0);
	act("$n sighs loudly.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 12:
	act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,
	    TO_ROOM);
	act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 13:
	do_say(ch, "The worms crawl in, the worms crawl out..", 0, 0);
	return (1);
    case 14:
	act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,
	    TO_ROOM);
	act("$n whistles a tune.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 15:
	do_say(ch,
	       "Get Lurch to bring me over a case and I'll sport you a year's supply of grilled ogre.",
	       0, 0);
	return (1);
    default:
	return (FALSE);
	break;
    }
    return (0);
}



SPECIAL(stu)
{
    if (cmd)
	return 0;

    switch (number(0, 60)) {
    case 0:
	do_say(ch,
	       "I'm so damn cool, I'm too cool to hang out with myself!",
	       0, 0);
	break;
    case 1:
	do_say(ch, "I'm really the NICEST guy you ever MEET!", 0, 0);
	break;
    case 2:
	do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0,
	       0);
	break;
    case 3:
	do_say(ch,
	       "Mind if I upload 200 megs of pregnant XXX gifs with no descriptions to your bbs?",
	       0, 0);
	break;
    case 4:
	do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0, 0);
	break;
    case 5:
	do_say(ch, "I'll take you OUT!!   ...tomorrow", 0, 0);
	break;
    case 6:
	do_say(ch, "I invented Mud you know...", 0, 0);
	break;
    case 7:
	do_say(ch, "Can I have a cup of water?", 0, 0);
	break;
    case 8:
	do_say(ch,
	       "I'll be jogging down ventnor ave in 10 minutes if you want some!",
	       0, 0);
	break;
    case 9:
	do_say(ch,
	       "Just let me pull a few strings and I'll get ya a site, they love me! - doesnt everyone?",
	       0, 0);
	break;
    case 10:
	do_say(ch, "Pssst! Someone tell Mercy to sport me some levels.", 0,
	       0);
	act("$n nudges you with his elbow.", FALSE, ch, 0, 0, TO_ROOM);
	break;
    case 11:
	do_say(ch,
	       "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!",
	       0, 0);
	break;
    case 12:
	act("$n tells you, 'Skylar has bad taste in women!'", FALSE, ch, 0,
	    0, TO_ROOM);
	act("$n screams in terror!", FALSE, ch, 0, 0, TO_ROOM);
	do_flee(ch, 0, 0, 0);
	break;
    case 13:
	if (number(0, 32767) < 10) {
	    act
		("$n whispers to you, 'Dude! If you fucking say 'argle bargle' to the glowing fido he'll raise you a level!'",
		 FALSE, ch, 0, 0, TO_ROOM);
	    act("$n flexes.", FALSE, ch, 0, 0, TO_ROOM);
	}
	return (1);
    default:
	return (FALSE);
	break;
	return (1);
    }
    return 0;
}


SPECIAL(sund_earl)
{
    if (cmd)
	return (FALSE);
    switch (number(0, 20)) {
    case 0:
	do_say(ch, "Lovely weather today.", 0, 0);
	return (1);
    case 1:
	act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return (1);
    case 2:
	do_say(ch, "Hot performance at the gallows tonight.", 0, 0);
	act("$n winks suggestively.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 3:
	do_say(ch, "Must remember to up the taxes at my convenience.", 0,
	       0);
	return (1);
    case 4:
	do_say(ch, "Sundhaven is impermeable to the enemy!", 0, 0);
	act("$n growls menacingly.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 5:
	do_say(ch, "Decadence is the credence of the abominable.", 0, 0);
	return (1);
    case 6:
	do_say(ch,
	       "I look at you and get a wonderful sense of impending doom.",
	       0, 0);
	act("$n chortles merrily.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    case 7:
	act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return (1);
    case 8:
	do_say(ch, "It's Mexican Madness night at Maynards!", 0, 0);
	act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
	return (1);
    default:
	return (FALSE);
	break;
	return (0);
    }
}


SPECIAL(blinder)
{
    if (cmd)
	return FALSE;

    if (GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room) &&
	(number(0, 100) + GET_LEVEL(ch) >= 50)) {
	act("$n whispers, 'So, $N! You wouldst share my affliction!", 1,
	    ch, 0, FIGHTING(ch), TO_NOTVICT);
	act("$n whispers, 'So, $N! You wouldst share my affliction!", 1,
	    ch, 0, FIGHTING(ch), TO_VICT);
	act("$n's frayed cloak blows as he points at $N.", 1, ch, 0,
	    FIGHTING(ch), TO_NOTVICT);
	act("$n's frayed cloak blows as he aims a bony finger at you.", 1,
	    ch, 0, FIGHTING(ch), TO_VICT);
	act("A flash of pale fire explodes in $N's face!", 1, ch, 0,
	    FIGHTING(ch), TO_NOTVICT);
	act("A flash of pale fire explodes in your face!", 1, ch, 0,
	    FIGHTING(ch), TO_VICT);
	call_magic(ch, FIGHTING(ch), 0, SPELL_BLINDNESS, ABT_SPELL,
		   GET_LEVEL(ch), CAST_SPELL, FALSE);
	return TRUE;
    }
    return FALSE;
}


SPECIAL(idiot)
{
    if (cmd)
	return FALSE;
    switch (number(0, 40)) {
    case 0:
	do_say(ch, "even if idiot = god", 0, 0);
	do_say(ch, "and Stu = idiot", 0, 0);
	do_say(ch, "Stu could still not = god.", 0, 0);
	act("$n smiles.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 1:
	act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return TRUE;
    case 2:
	act("$n doesn't think you could stand up to him in a duel.", FALSE,
	    ch, 0, 0, TO_ROOM);
	return TRUE;
    case 3:
	do_say(ch, "Rome really was built in a day.", 0, 0);
	act("$n snickers.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 4:
	act("$n flips over and walks around on his hands.", FALSE, ch, 0,
	    0, TO_ROOM);
	return TRUE;
    case 5:
	act("$n cartwheels around the room.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 6:
	do_say(ch, "How many ogres does it take to screw in a light bulb?",
	       0, 0);
	act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return TRUE;
    case 7:
	do_say(ch, "Uh huh. Uh huh huh.", 0, 0);
	return TRUE;
    case 8:
	act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
	act("$n whistles quietly.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 9:
	act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return TRUE;
    case 10:
	act("$n has a battle of wits with himself and comes out unharmed.",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 11:
	do_say(ch, "All this and I am just a number.", 0, 0);
	act("$n cries on your shoulder.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 12:
	do_say(ch,
	       "A certain hunchback I know dresses very similar to you, very similar...",
	       0, 0);
	return TRUE;
    default:
	return FALSE;
    }
    return FALSE;
}


SPECIAL(marbles)
{
    struct obj_data *tobj = me;

    if (tobj->in_room == NOWHERE)
	return FALSE;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
	return FALSE;

    if (cmd && IS_MOVE(cmd)) {
	if (number(1, 100) > 60 + GET_DEX(ch)) {
	    act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
	    act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
	    GET_POS(ch) = POS_SITTING;
	    return TRUE;
	} else {
	    if (number(1, 3) == 1) {
		act("You slip on $p, but manage to keep your balance.",
		    FALSE, ch, tobj, 0, TO_CHAR);
		act("$n slips on $p, but manages to keep $s balance.",
		    FALSE, ch, tobj, 0, TO_ROOM);
	    }
	}
    }
    return FALSE;
}

SPECIAL(toll_guard)
{
    /* Toll guards will force a player to give them money in  ** 
       ** exchange for passage.  This was originally designed    **
       ** for McGintey Cove but could be used anywhere.  Just    ** 
       ** update the below structure.                            */

    int i, found = FALSE;

    /* mob vnum, dir to block, toll (in gold) */
    const int toll_guards[][3] = {
	{6705, SCMD_SOUTH, 10},
	{5803, SCMD_NORTH, 1500},
	{-1, -1, -1}
    };

    if (!IS_MOVE(cmd) && !CMD_IS("pay"))
	return FALSE;

    for (i = 0; toll_guards[i][0] > 0 && !found; i++)
	if (GET_MOB_VNUM((struct char_data *) me) == toll_guards[i][0] &&
	    IN_ROOM(ch) == IN_ROOM((struct char_data *) me))
	    found = TRUE;

    if (!found)
	return FALSE;
    i--;

    if (cmd == toll_guards[i][1] && !IS_AFFECTED(ch, AFF_SNEAK)) {
	switch (toll_guards[i][0]) {
	case 6705:
	    act
		("$N steps in front of $n and holds $S hand out for payment.",
		 FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
	    send_to_char("No pay, no way. No one gets in without paying "
			 "the toll!\r\n", ch);
	    break;
	case 5803:
	    act
		("$N crosses $S arms, standing solidly before $n with the hint of "
		 "a smile playing across $S face.", FALSE, ch, 0,
		 (struct char_data *) me, TO_ROOM);
	    act
		("$N blocks your way with a slight smirk and a folding of $S "
		 "arms. Seems $e wants you to pay before going into the city.",
		 FALSE, ch, 0, (struct char_data *) me, TO_CHAR);
	    break;
	}
	return TRUE;
    } else if (CMD_IS("pay")) {
	if (GET_GOLD(ch) >= toll_guards[i][2]) {
	    GET_GOLD(ch) -= toll_guards[i][2];
	    switch (toll_guards[i][0]) {
	    case 6705:
		break;
	    case 5803:
		act
		    ("$n hands $N the payoff and enters the town unmolested.",
		     FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
		act
		    ("You bow your head in acquiescance and pay the ridiculous "
		     "amount of 1500 coins.", FALSE, ch, 0, 0, TO_CHAR);
		break;
	    }
	    do_move(ch, "", toll_guards[i][1], 0);
	} else
	    switch (toll_guards[i][0]) {
	    case 6705:
		send_to_char("You haven't got the money.  Get lost!\r\n",
			     ch);
		break;
	    case 5803:
		act
		    ("$N grabs $n by the front of $s shirt and growls into $s face "
		     "that $e better pay OR ELSE!", FALSE, ch, 0,
		     (struct char_data *) me, TO_ROOM);
		act("$N roughly grabs you by your lapels, shaking you and "
		    "threatening that you had *better* pay the price or else $E "
		    "can't guarantee your safety.", FALSE, ch, 0,
		    (struct char_data *) me, TO_CHAR);
		break;
	    }
	return TRUE;
    }
    return FALSE;
}

SPECIAL(rattlesnake)
{
    if (cmd)
	return FALSE;
    if (!number(0, 4))
	act("$n shakes $s rattle.", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
}


SPECIAL(hidden_believer)
{
    if (cmd)
	return FALSE;
    if (!number(0, 15))
	act("Someone whispers quietly at you from the corner of the room.",
	    FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
}

SPECIAL(gibbering_man)
{
    if (cmd)
	return FALSE;
    if (!number(0, 5))
	act
	    ("The man gazes painfully up at you and gasps, /cr'Do not go in "
	     "there! They will cut your heart out'/c0.", FALSE, ch, 0, 0,
	     TO_ROOM);
    return TRUE;
}

SPECIAL(believer)
{
    if (cmd)
	return FALSE;
    if (!number(0, 23))
	act
	    ("The believer stares longingly at the blade in X'Raantra's fist, "
	     "absently licking a bit of sprayed blood from the corner of his "
	     "mouth.", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
}

SPECIAL(robedthief)
{
    if (cmd)
	return FALSE;
    if (!number(0, 33))
	act("$n suddenly stops, drops down on $s knees and touches $s\r\n"
	    "forehead to the floor. /crKaan/c0 must be near!.",
	    FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
}

SPECIAL(gunter)
{
    if (cmd)
	return (FALSE);
    switch (number(0, 20)) {
    case 0:
	do_say(ch, "And so we must ask ourselves every day... will we be "
	       "the next Chosen?", 0, 0);
	return TRUE;
    case 1:
	act
	    ("$n gives you a deep, penetrating glare, as if trying to decide "
	     "something.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 2:
	do_say(ch, "We must rejoice in the knowledge that the Saviour "
	       "X'Raantra is with us.", 0, 0);
	act("$n kneels down and touches $s head to the ground.",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 3:
	act("$n mumbles distractedly to himself.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return TRUE;
    case 4:
	do_say(ch, "X'Raantra is our one true link to God!", 0, 0);
	return TRUE;
    case 5:
	do_say(ch, "All praise to our saviour, X'Raantra!", 0, 0);
	return TRUE;
    case 6:
	act("$n chuckles quietly, as if enjoying a personal joke.",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}

SPECIAL(quiltclub)
{
    if (cmd)
	return FALSE;
    switch (number(0, 20)) {
    case 0:
	act("Beulah mumbles 'mmm hmm' without looking up. ",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 1:
	act
	    ("Nedis natters, 'I say these dang men are just lucky that we even "
	     "let them live in our houses!'", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 2:
	act
	    ("$n shakes her quilting needle in the air and says, 'All them men are "
	     "good for is one thing. Every woman knows THAT!'", FALSE, ch,
	     0, 0, TO_ROOM);
	return TRUE;
    case 3:
	act("Inga blushes.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 4:
	do_say(ch,
	       "All that talk of all them responsibilities - drinkin' and "
	       "workin' is all they do. THEY don't have to bear the children!",
	       0, 0);
	return TRUE;
    case 5:
	do_say(ch, "I say let them cook one night for a change!", 0, 0);
	return TRUE;
    case 6:
	act
	    ("Beulah shifts a bit in her chair, looks up with sleepy eyes and "
	     "murmurs, 'mmm hmm'.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}


SPECIAL(styxferry)
{

    int i;

    /* mob vnum, dir to block, toll (in gold) */
    const int styxferry[][3] = {
	{3219, SCMD_UP, 10000},
	{-1, -1, -1}
    };

    if (!IS_MOVE(cmd) && !CMD_IS("pay"))
	return FALSE;

    for (i = 0; styxferry[i][0] != -1; i++)
	if (IN_ROOM(ch) == IN_ROOM((struct char_data *) me) &&
	    cmd == styxferry[i][1]) {
	    act
		("$N steps in front of $n and holds $S hand out for payment.",
		 FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
	    send_to_char
		("You must pay the price if ever you wish to leave."
		 "\r\n", ch);
	    return TRUE;
	} else if (CMD_IS("pay")) {
	    if (GET_GOLD(ch) >= styxferry[i][2]) {
		GET_GOLD(ch) -= styxferry[i][2];
		send_to_char
		    ("You pay the toll and Chyron ushers you across."
		     "\r\n", ch);
		act("$n pays the toll.", FALSE, ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, real_room(3218));
		look_at_room(ch, FALSE);
	    } else
		send_to_char("The fee is 10000 gold or death. Choose.\r\n",
			     ch);
	    return TRUE;
	}
    return FALSE;
}


SPECIAL(class_cleric)
{
    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (FIGHTING(vict) == ch && !number(0, 4))
	    break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
	vict = FIGHTING(ch);

    if (number(0, GET_LEVEL(ch)) > (GET_LEVEL(ch) / 2))
	return FALSE;

    if ((GET_LEVEL(ch) > 17) && (number(0, 10) == 0))
	cast_spell(ch, vict, NULL, SPELL_HARM, ABT_SPELL, FALSE);

    if ((GET_LEVEL(ch) > 25) && (number(0, 8) == 0)) {
	if (IS_EVIL(ch))
	    cast_spell(ch, vict, NULL, SPELL_CURSE, ABT_SPELL, FALSE);
	else
	    cast_spell(ch, vict, NULL, SPELL_CALL_LIGHTNING, ABT_SPELL,
		       FALSE);
    }

    if ((GET_LEVEL(ch) > 23) && (number(0, 8) == 0))
	cast_spell(ch, vict, NULL, SPELL_EARTHQUAKE, ABT_SPELL, FALSE);

    if ((GET_LEVEL(ch) > 12) && (number(0, 12) == 0)) {
	if (IS_EVIL(ch))
	    cast_spell(ch, vict, NULL, SPELL_DISPEL_GOOD, ABT_SPELL,
		       FALSE);
	else if (IS_GOOD(ch))
	    cast_spell(ch, vict, NULL, SPELL_DISPEL_EVIL, ABT_SPELL,
		       FALSE);
    }
    if (number(0, 4))
	return TRUE;

    switch (number((GET_LEVEL(ch) >> 1), GET_LEVEL(ch))) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
	cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
	break;
    case 6:
    case 7:
	cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
	break;
    case 8:
    case 9:
	cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC, ABT_SPELL, FALSE);
	break;
    case 10:
    case 11:
	cast_spell(ch, vict, NULL, SPELL_DUMBNESS, ABT_SPELL, FALSE);
	break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 23:
	cast_spell(ch, ch, NULL, SPELL_CURE_LIGHT, ABT_SPELL, FALSE);
	break;
    case 27:
	cast_spell(ch, vict, NULL, SPELL_CALL_LIGHTNING, ABT_SPELL, FALSE);
	break;
    default:
	cast_spell(ch, ch, NULL, SPELL_CURE_CRITIC, ABT_SPELL, FALSE);
	break;
    }
    return TRUE;

}


SPECIAL(class_mage)
{
    struct char_data *vict;

    if (cmd || GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    /* pseudo-randomly choose someone in the room who is fighting me */
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (FIGHTING(vict) == ch && !number(0, 4))
	    break;

    /* if I didn't pick any of those, then just slam the guy I'm fighting */
    if (vict == NULL)
	vict = FIGHTING(ch);

    if (number(0, 200) > (GET_LEVEL(ch) + 10))
	return FALSE;

    switch (number((GET_LEVEL(ch) >> 1), GET_LEVEL(ch))) {
    case 0:;
    case 1:
	cast_spell(ch, vict, NULL, SPELL_MAGIC_MISSILE, ABT_SPELL, FALSE);
	break;
    case 2:;
    case 3:
	cast_spell(ch, vict, NULL, SPELL_CHILL_TOUCH, ABT_SPELL, FALSE);
	break;
    case 4:;
    case 5:
	cast_spell(ch, vict, NULL, SPELL_BURNING_HANDS, ABT_SPELL, FALSE);
	break;
    case 6:;
    case 7:
	cast_spell(ch, vict, NULL, SPELL_SHOCKING_GRASP, ABT_SPELL, FALSE);
	break;
    case 8:;
    case 9:
    case 10:;
    case 11:
    case 12:;
    case 13:;
    case 14:;
    case 15:
	cast_spell(ch, vict, NULL, SPELL_LIGHTNING_BOLT, ABT_SPELL, FALSE);
	break;
    case 16:;
    case 17:;
    case 18:;
    case 19:;
    case 20:
	cast_spell(ch, vict, NULL, SPELL_COLOR_SPRAY, ABT_SPELL, FALSE);
	break;
    case 21:
    case 22:
    case 23:;
    case 24:;
    case 25:
	cast_spell(ch, vict, NULL, SPELL_ENERGY_DRAIN, ABT_SPELL, FALSE);
	break;
    case 26:;
    case 27:;
    case 28:
	cast_spell(ch, vict, NULL, SPELL_FIREBALL, ABT_SPELL, FALSE);
	break;
    case 29:;
    case 30:
    case 31:;
    case 32:;
    case 33:;
    case 34:;
    case 35:
	cast_spell(ch, vict, NULL, SPELL_SONIC_WALL, ABT_SPELL, FALSE);
	break;
    case 36:;
    case 37:;
    case 38:;
    case 39:;
    case 40:
    case 41:;
    case 42:
    case 43:;
    case 44:;
    case 45:
    case 46:;
    case 47:;
    case 48:;
    case 49:;
    case 50:;
    case 51:
	cast_spell(ch, vict, NULL, SPELL_FIRESTORM, ABT_SPELL, FALSE);
	break;

    case 53:
    case 54:
    case 55:
    case 58:
    case 61:
	cast_spell(ch, vict, NULL, SPELL_STONE_HAIL, ABT_SPELL, FALSE);
	break;

    case 62:
    case 64:
    case 66:
    case 68:
    case 71:
	cast_spell(ch, vict, NULL, SPELL_FLYING_FIST, ABT_SPELL, FALSE);
	break;

    case 72:
    case 74:
    case 76:
    case 78:
    case 81:
	cast_spell(ch, vict, NULL, SPELL_SHOCK_SPHERE, ABT_SPELL, FALSE);
	break;

    case 82:
    case 84:
    case 86:
    case 88:
    case 91:
	cast_spell(ch, vict, NULL, SPELL_CAUSTIC_RAIN, ABT_SPELL, FALSE);
	break;

    case 92:
    case 94:
    case 96:
    case 98:
    case 101:
	cast_spell(ch, vict, NULL, SPELL_WITHER, ABT_SPELL, FALSE);
	break;

    case 102:
    case 104:
    case 106:
    case 108:
    case 110:
	cast_spell(ch, vict, NULL, SPELL_METEOR_SWARM, ABT_SPELL, FALSE);
	break;

    case 112:
    case 114:
    case 116:
    case 118:
    case 121:
	cast_spell(ch, vict, NULL, SPELL_BALEFIRE, ABT_SPELL, FALSE);
	break;

    case 122:
    case 124:
    case 126:
    case 128:
    case 131:
	cast_spell(ch, vict, NULL, SPELL_SCORCH, ABT_SPELL, FALSE);
	break;

    case 132:
    case 134:
    case 136:
    case 138:
    case 141:
	cast_spell(ch, vict, NULL, SPELL_IMMOLATE, ABT_SPELL, FALSE);
	break;

    default:
	cast_spell(ch, vict, NULL, SPELL_FIRESTORM, ABT_SPELL, FALSE);
	break;
    }
    return TRUE;


}

SPECIAL(rat)
{

    struct obj_data *i, *temp, *next_obj;

    if (cmd || !AWAKE(ch) || GET_POS(ch) != POS_STANDING)
	return (FALSE);


    switch (number(0, 3)) {
    case 0:			/* rat action */
	act("$n lifts its nose in the air and sniffs.", FALSE, ch, 0, 0,
	    TO_ROOM);
	return (1);
    case 1:			/* rat action */
	act("$n looks at you and skitters to the nearest wall.", FALSE, ch,
	    0, 0, TO_ROOM);
	return (1);
    case 2:			/* rat action */
	for (i = world[ch->in_room].contents; i; i = i->next_content) {
	    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER && GET_OBJ_VAL(i, 3)) {
		act
		    ("$n leaps onto a corpse and eats it all from the inside.",
		     FALSE, ch, 0, 0, TO_ROOM);
		for (temp = i->contains; temp; temp = next_obj) {
		    next_obj = temp->next_content;
		    obj_from_obj(temp);
		    obj_to_room(temp, ch->in_room);
		}
		extract_obj(i);
		return (TRUE);
	    }
	}
	return (FALSE);
	break;
    }
    return (FALSE);
}

SPECIAL(class_warrior)
{
    struct char_data *vict;
    int att_type = 0, hit_roll = 0, to_hit = 0;
    struct obj_data *weap;

    if (cmd || !AWAKE(ch))
	return FALSE;

    /* if two people are fighting in a room */
    if (FIGHTING(ch) && (FIGHTING(ch)->in_room == ch->in_room)) {

	vict = FIGHTING(ch);

	if (number(1, 5) == 5) {
	    act("$n foams at the mouth and growls in anger.", 1, ch, 0, 0,
		TO_ROOM);
	}
	if (GET_POS(ch) == POS_FIGHTING) {

	    att_type = number(1, 40);

	    hit_roll = number(1, 100) + GET_STR(ch);
	    to_hit = (100 - (int) (100 * GET_LEVEL(ch) / 250));
	    if (GET_LEVEL(vict) >= LVL_IMMORT)
		hit_roll = 0;

	    switch (att_type) {
	    case 1:
	    case 2:
	    case 3:
	    case 4:
		/* bash */
		if (hit_roll < to_hit) {
		    GET_POS(ch) = POS_SITTING;
		    damage(ch, vict, 0, SKILL_BASH, ABT_SKILL);
		} else {
		    GET_POS(vict) = POS_SITTING;
		    damage(ch, vict, GET_LEVEL(ch), SKILL_BASH, ABT_SKILL);
		    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 3);

		break;

	    case 5:
	    case 6:
	    case 7:
	    case 8:
	    case 9:
	    case 10:
		if (hit_roll < to_hit) {	/* kick */
		    damage(ch, vict, 0, SKILL_KICK, ABT_SKILL);
		} else {
		    damage(ch, vict, 2 * GET_LEVEL(ch), SKILL_KICK,
			   ABT_SKILL);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);

		break;

	    case 11:
		if (hit_roll < to_hit) {	/* deathblow */
		    damage(ch, vict, 0, SKILL_DEATHBLOW, ABT_SKILL);
		} else {
		    damage(ch, vict, 2 * GET_LEVEL(ch), SKILL_DEATHBLOW,
			   ABT_SKILL);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 4);
		break;
	    case 12:
	    case 13:
		if (!(weap = GET_EQ(FIGHTING(ch), WEAR_WIELD))) {
		    send_to_char("Nope, sorry\r\n", ch);
		    break;
		}
		if (hit_roll < to_hit) {	/* Disarm */
		    act("You knock $p from $N's hand!", FALSE, ch, weap,
			FIGHTING(ch), TO_CHAR);
		    act("$n knocks $p from your hand!", FALSE, ch, weap,
			FIGHTING(ch), TO_VICT);
		    act("$n knocks $p from $N's hand!", FALSE, ch, weap,
			FIGHTING(ch), TO_ROOM);
		    obj_to_char(unequip_char(FIGHTING(ch), WEAR_WIELD),
				FIGHTING(ch));
		} else {
		    act("You fail to disarm $N", FALSE, ch, weap,
			FIGHTING(ch), TO_CHAR);
		    act("$n fails to disarm you", FALSE, ch, weap,
			FIGHTING(ch), TO_VICT);
		    act("$n fails to disarm $N", FALSE, ch, weap,
			FIGHTING(ch), TO_ROOM);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE);
		break;
	    case 14:
	    case 15:
	    case 16:
		if (hit_roll < to_hit) {	/* Round House */
		    damage(ch, vict, 0, SKILL_ROUNDHOUSE, ABT_SKILL);
		} else {
		    damage(ch, vict, GET_LEVEL(ch), SKILL_ROUNDHOUSE,
			   ABT_SKILL);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 3);
		break;
	    case 17:
		if (hit_roll < to_hit) {
		    damage(ch, vict, GET_LEVEL(ch) + number(1, 10),
			   SKILL_STUN, ABT_SKILL);
		} else {
		    damage(ch, vict, 0, SKILL_STUN, ABT_SKILL);
		}
		WAIT_STATE(ch, PULSE_VIOLENCE * 3);
		break;
	    default:
		break;
	    }

	}
    }
    return FALSE;
}



SPECIAL(highwayman)
{

    struct obj_data *obj;
    room_rnum new_room;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if (cmd == SCMD_WEST && !IS_AFFECTED(ch, AFF_SNEAK)) {
	act
	    ("$N puts a quiet, dangerous hand on your chest and raises one eyebrow, "
	     "eyeing your purse.", FALSE, ch, 0, (struct char_data *) me,
	     TO_CHAR);
	act
	    ("$N puts a quiet, dangerous hand on $n's chest and raises one eyebrow, "
	     "eyeing your purse.", FALSE, ch, 0, (struct char_data *) me,
	     TO_ROOM);
	return TRUE;
    }

    if (!CMD_IS("give"))
	return FALSE;

    two_arguments(argument, arg1, arg2);
    if (!*arg1)
	return FALSE;

    if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	sprintf(buf, "You don't seem to have %s %s.\r\n", AN(arg1), arg1);
	send_to_char(buf, ch);
	return TRUE;
    }

    if (!*arg2)
	return FALSE;

    if (!((struct char_data *) me == get_char_room_vis(ch, arg2)))
	return FALSE;

    if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
	act("You can't let go of $p!!  Yeech!", FALSE, ch, obj, 0,
	    TO_CHAR);
	return TRUE;
    }

    obj_from_char(obj);
    extract_obj(obj);

    act
	("\r\n/cWYou give $N $p/cW. $E looks it over and ushers you west./c0\r\n",
	 FALSE, ch, obj, (struct char_data *) me, TO_CHAR);
    act
	("\r\n/cW$n gives $N $p/cW. $E looks it over and ushers $m west./c0\r\n",
	 FALSE, ch, obj, (struct char_data *) me, TO_ROOM);

    new_room = EXIT(ch, WEST)->to_room;
    char_from_room(ch);
    char_to_room(ch, new_room);
    look_at_room(ch, 0);
    act("$n has arrived.", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;

}
SPECIAL(arrest)
{

    if (!CMD_IS("west"))
	return FALSE;

    act
	("\r\n/cW$N shouts, 'Halt! In the name of the King, you have been placed under "
	 "arrest.'", FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
    act("You get roughly escorted through a small keep and down into an "
	"underground prison area, tossed into a cell./c0\r\n", FALSE, ch,
	0, 0, TO_CHAR);
    act("$n gets roughly escorted to prison./c0\r\n", FALSE, ch, 0, 0,
	TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, real_room(8412));
    look_at_room(ch, 0);
    act("$n is thrown roughly into the cell.", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
}

SPECIAL(elf_maiden)
{

    if (cmd)
	return FALSE;

    if (!number(0, 7)) {
	act
	    ("$n squints up at you and croaks, 'Aye, you may get out of this cell "
	     "a'right ya may, but there is no way in all that's Holy that you "
	     "will get past the guards out there.'", FALSE,
	     (struct char_data *) me, 0, 0, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}

SPECIAL(serf)
{

    if (cmd)
	return FALSE;

    if (!number(0, 4)) {
	act
	    ("A wide-eyed serf blasts out from between the cornstalks, sees you, "
	     "and dives back into the fields, gone in a flash.", FALSE, 0,
	     0, 0, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}

#define HORSE_PRICE(m) (GET_LEVEL(m) * 50)
SPECIAL(horse_trader)
{

    char horse_name[MAX_INPUT_LENGTH];
    struct char_data *horse;
    int horse_room;
    struct obj_data *obj;
    struct char_data *mount;
    int r_num;

    if (!cmd)
	return FALSE;

    horse_room = ch->in_room + 1;

    if (CMD_IS("list")) {
	send_to_char("Available mounts are:\r\n", ch);
	for (horse = world[horse_room].people; horse;
	     horse = horse->next_in_room) {
	    sprintf(buf, "%8d - %s\r\n", HORSE_PRICE(horse),
		    GET_NAME(horse));
	    send_to_char(buf, ch);
	}
	return TRUE;
    } else if (CMD_IS("buy")) {

	argument = one_argument(argument, buf);
	argument = one_argument(argument, horse_name);

	if (!(horse = get_char_room(buf, horse_room))) {
	    send_to_char("There is no such mount!\r\n", ch);
	    return TRUE;
	}
	if (GET_CLASS(ch) == CLASS_VAMPIRE) {
	    send_to_char("Why would you bother? Vampires do not ride.\r\n",
			 ch);
	    return TRUE;
	}
	if (GET_GOLD(ch) < HORSE_PRICE(horse)) {
	    send_to_char("You don't have enough gold!\r\n", ch);
	    return TRUE;
	}
	GET_GOLD(ch) -= HORSE_PRICE(horse);
	horse = read_mobile(GET_MOB_RNUM(horse), REAL);
	GET_EXP(horse) = 0;

	char_to_room(horse, ch->in_room);
	GET_RIDER(horse) = ch;
	GET_MOUNT(ch) = horse;

	IS_CARRYING_W(horse) = 1000;
	IS_CARRYING_N(horse) = 100;
	GET_MAX_MOVE(horse) = GET_LEVEL(horse) * 5 + 100;
	GET_MOVE(horse) = GET_MAX_MOVE(horse);

	send_to_char("She'll serve you well.\r\n", ch);
	sprintf(buf, "You mount %s\r\n", GET_NAME(GET_MOUNT(ch)));
	send_to_char(buf, ch);
	act("$n buys $N.", FALSE, ch, 0, horse, TO_ROOM);

	return TRUE;
    } else if (CMD_IS("stable")) {
	if (!(GET_MOUNT(ch))) {
	    send_to_char("The stable man says, 'Please stay mounted.'\r\n",
			 ch);
	    return 1;
	}
	mount = GET_MOUNT(ch);

	r_num = real_object(1260);	// token of mount recovery
	obj = read_object(r_num, REAL);
	GET_OBJ_VAL(obj, 1) = GET_MOB_VNUM(mount);
	obj_to_char(obj, ch);
	send_to_char
	    ("The stable man gives you a shiny gold token.  DON'T LOSE IT.\r\n",
	     ch);
	act("$n swiftly jumps off the back of $N.", FALSE, ch, 0,
	    GET_MOUNT(ch), TO_ROOM);
	act("You swiftly jump off the back of $N.", FALSE, ch, 0,
	    GET_MOUNT(ch), TO_CHAR);
	send_to_char
	    ("The stable man leads your mount away to a nice safe place ....\r\n",
	     ch);
	GET_RIDER(GET_MOUNT(ch)) = NULL;
	GET_MOUNT(ch) = NULL;
	extract_char(mount);
	return 1;


    } else if (CMD_IS("redeem")) {
	if (!(obj = get_obj_in_list_vis(ch, "mount_token", ch->carrying))) {
	    send_to_char
		("You don't seem to have any token to claim your mount.\r\n",
		 ch);
	    return 1;
	}
	r_num = GET_OBJ_VAL(obj, 1);
	if (!r_num) {
	    send_to_char("Ack, It's a bug squish it!\r\n", ch);
	    return 1;
	}
	mount = read_mobile(r_num, VIRTUAL);
	send_to_char("The stable man takes back the shiny gold token.\r\n",
		     ch);
	obj_from_char(obj);

	send_to_char("The stable man returns your mount back to you.\r\n",
		     ch);
	GET_EXP(mount) = 0;
	char_to_room(mount, ch->in_room);

	return 1;
    } else
	return FALSE;

}


SPECIAL(ferry_tickets)
{
    struct obj_data *o, *obj;
    bool has_token;
    int tokennum, transroom = 0, transroom1 = 0, transroom2 = 0;
    ACMD(do_look);
    o = 0;
    tokennum = 31300;		/* obj number of your tokens! */
    transroom1 = real_room(31371);	/* transporter rooms */
//   transroom2 = real_room(3044);


    if (cmd == 2) {
	/* transporter there? */
	if (world[ch->in_room].dir_option[1]->to_room != NOWHERE) {
	    /* if so, which one? */
	    if (world[ch->in_room].dir_option[1]->to_room == transroom1)
		transroom = transroom1;
	    else if (world[ch->in_room].dir_option[1]->to_room ==
		     transroom2) transroom = transroom2;
	    has_token = FALSE;
	    for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj_index[obj->item_number].virtual == tokennum) {
		    has_token = TRUE;
		    o = obj;
		}
	    if (!has_token) {
		send_to_char("You need a ticket to ride the ferry.\n\r",
			     ch);
		return (TRUE);
	    } else {
		send_to_char
		    ("The first mate takes your ticket, and you board the ferry\n\r",
		     ch);
		act
		    ("The first mate takes $n's ticket, and $e leaves east.\n\r",
		     FALSE, ch, 0, 0, TO_ROOM);
		obj_from_char(o);
		extract_obj(o);
		char_from_room(ch);
		char_to_room(ch, transroom);
		do_look(ch, "", 0, 0);
		return (TRUE);
	    }
	}			/* end if transporter is there */
    }				/* end if cmd = 2 */
    return (FALSE);		/* all other cmds */
}

/* List players in hometowns */
/* NOTE: The object MUST be in a hometown. */
SPECIAL(docket)
{

    FILE *pfile;
    int i = 1, hits = 0;
    sh_int ht;
    struct char_file_u vbuf;
    char buf[MAX_STRING_LENGTH];
    char buf2[512];

    extern const char *hometowns[];
    extern sh_int get_ht_zone(int room);

    if (!CMD_IS("docket"))
	return FALSE;

    ht = get_ht_zone(world[ch->in_room].number);
    if (ht < 0) {
	send_to_char
	    ("Error - Docket isn't in a hometown.  Please report.\r\n",
	     ch);
	return TRUE;
    }

    if (!(pfile = fopen(PLAYER_FILE, "r+b"))) {
	return TRUE;
    }
    sprintf(buf, "Citizens of %s:\r\n"
	    "-------------------------------------------------------------------\r\n",
	    hometowns[ht]);
    fseek(pfile, (long) (i * sizeof(struct char_file_u)), SEEK_SET);
    fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
    for (i = 2; !feof(pfile); i++) {
	if (vbuf.hometown == ht && vbuf.level < LVL_IMMORT) {
	    hits++;
	    sprintf(buf, "%s %-16s%s", buf, vbuf.name,
		    hits % 4 ? "" : "\r\n");
	}
	fseek(pfile, (long) (i * sizeof(struct char_file_u)), SEEK_SET);
	fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
    }
    fclose(pfile);

    if (!hits) {
	strcat(buf, " Nobody.\r\n");
	sprintf(buf2, "Welcome to the ghost town of %s.\r\n\r\n",
		hometowns[ht]);
    } else
	sprintf(buf2, "Welcome to %s, population %d\r\n\r\n",
		hometowns[ht], hits);
    strcat(buf, "\r\n");

    send_to_char(buf2, ch);
    page_string(ch->desc, buf, 1);

    return TRUE;
}

SPECIAL(yragg)
{
    if (cmd)
	return FALSE;
    switch (number(0, 20)) {
    case 0:
	act("Yragg attempts to get up from his chair but ends up falling "
	    "flat on his ass.\r\nThe bartender helps him back up.",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 1:
	act("Yragg looks your way blearily, belching a stenchful burp in "
	    "your general direction.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 2:
	act("Yragg looks at you and slurringly whispers, 'The Gods are\r\n"
	    "alive and roaming the streets of Khera Vale with Death.",
	    FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 3:
	act
	    ("Yragg slumps upon his barstool and begins shaking from head\r\n"
	     "to toe, a small bit of drool falling from his half-open "
	     "mouth.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    case 4:
	do_say(ch,
	       "Aye, and I have seen the icy countenances of the Gods.\r\n"
	       "  Aye, and they be unhappy, or I ain't... ain't... err, well\r\n"
	       "they's not happy - that be a fact shore 'nough!", 0, 0);
	return TRUE;
    case 5:
	do_say(ch, "And let no man say that Yragg Vondes has not the "
	       "courage of \r\na thousand men - but I *ain't* goin' back to "
	       "Khera Vale!", 0, 0);
	return TRUE;
    case 6:
	act("The bartender looks sadly at Yragg, shaking his head in "
	    "sympathy.", FALSE, ch, 0, 0, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}



SPECIAL(leprechaun)
{
    int max;
    char buf[1024];
    char no_money[] = "You can't afford that.\r\n";
    char no_pracs[] = "You don't have enough practice sessions.\r\n";
    char maxed_out[] = "You can't improve that stat any more.\r\n";

    if (!CMD_IS("train"))
	return 0;

    if (GET_LEVEL(ch) < 50)
	max = 18;
    else if (GET_LEVEL(ch) < 75)
	max = 20;
    else if (GET_LEVEL(ch) < 100)
	max = 22;
    else if (GET_LEVEL(ch) < 125)
	max = 23;
    else
	max = 25;

    one_argument(argument, arg);

    if (!*arg) {
	sprintf(buf,
		"/cwYou can Train the following stats for the following costs:/c0\r\n\r\n"
		"  Stat          Gold    Practices\r\n"
		"  -------------------------------\r\n"
		"  /cWLuck    /cy50,000      2/c0\r\n"
		" /cgYou have %d practice sessions remaining./c0\r\n",
		GET_PRACTICES(ch));
	send_to_char(buf, ch);
	return 1;
    } else {
	if (isname(arg, "luck")) {
	    if (ch->real_stats.luck >= (int) max_stat_table[(int)GET_RACE(ch)][6]) {
		send_to_char(maxed_out, ch);
		return 1;
	    }
	    if (GET_GOLD(ch) >= 50000) {
		if (GET_PRACTICES(ch) >= 2) {
		    send_to_char("Wow.  You feel really lucky.\r\n", ch);
		    ch->real_stats.luck++;
		    GET_LUCK(ch)++;
		    GET_GOLD(ch) -= 50000;
		    GET_PRACTICES(ch) -= 2;
		    return 1;
		} else
		    send_to_char(no_pracs, ch);
	    } else
		send_to_char(no_money, ch);
	}
    }
    return TRUE;
}


SPECIAL(netherman)
{

    int i;

    const int netherman[][3] = {
	{3599, SCMD_UP, 0},
	{-1, -1, -1}
    };

    if (!IS_MOVE(cmd) && !CMD_IS("nether"))
	return FALSE;

    for (i = 0; netherman[i][0] != -1; i++)
	if (IN_ROOM(ch) == IN_ROOM((struct char_data *) me) &&
	    cmd == netherman[i][1]) {
	    act
		("$N tells you, '/cNETHER take you where you wish to go./c0",
		FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
	    send_to_char
		("You must use the NETHER command to enter the Nether."
		 "\r\n", ch);
	    return TRUE;
	} else if (CMD_IS("nether")) {
	    if (GET_GOLD(ch) >= netherman[i][2]) {
		GET_GOLD(ch) -= netherman[i][2];
		send_to_char
		    ("The old man gives you a final nod before sending "
		     "you to your death.\r\n" "\r\n", ch);
		act("$n is sent to the Nether.", FALSE, ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch, real_room(13402));
		look_at_room(ch, FALSE);
	    } else
		send_to_char
		    ("To enter the Nether you must type NETHER.\r\n", ch);
	    return TRUE;
	}
    return FALSE;
}


SPECIAL(quest_mage)
{
    int room_num;

    if (CMD_IS("okay")) {
	room_num = GET_LEVEL(ch) + 40000;
	act
	    ("The Questmage bows to you as he sends you off to the Questor.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("The Questmage bows to $n as he sends $m off to the Questor.",
	    FALSE, ch, 0, 0, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, real_room(room_num));
	look_at_room(ch, FALSE);
    }
    return FALSE;
}

void send_to_zone_outdoor(int zone_rnum, char *messg)
{
    struct descriptor_data *i;

    if (!messg || !*messg)
	return;

    for (i = descriptor_list; i; i = i->next)
	if (!i->connected && i->character && AWAKE(i->character) &&
	    (IN_ROOM(i->character) != NOWHERE) &&
	    !IS_SET_AR(ROOM_FLAGS(i->character->in_room), ROOM_INDOORS) &&
	    (world[IN_ROOM(i->character)].zone == zone_rnum))
	    send_to_char(messg, i->character);
}
void send_to_zone_indoor(int zone_rnum, char *messg)
{
    struct descriptor_data *i;

    if (!messg || !*messg)
	return;

    for (i = descriptor_list; i; i = i->next)
	if (!i->connected && i->character && AWAKE(i->character) &&
	    (IN_ROOM(i->character) != NOWHERE) &&
	    IS_SET_AR(ROOM_FLAGS(i->character->in_room), ROOM_INDOORS) &&
	    (world[IN_ROOM(i->character)].zone == zone_rnum))
	    send_to_char(messg, i->character);


}

static int noiseact = 1;
static int noisezone = 1;

void city_noises(void)
{
    int tempzone;

    switch (noisezone) {
    case 1:
	tempzone = real_room(3000);
	break;
    case 2:
	tempzone = real_room(1000);
	break;
    case 3:
	tempzone = real_room(6700);
	break;
    case 4:
	tempzone = real_room(5300);
	break;
    default:
	tempzone = 0;
	break;
    }

    noiseact++;
    if (noiseact == 101)
	noiseact = 1;

    if ((tempzone != 0) && (tempzone != NOWHERE))
	tempzone = world[tempzone].zone;

    if (tempzone != 0) {
	switch (noiseact) {
	case 1:
	    send_to_zone_outdoor(tempzone,
				 "The sound of a horse's neigh can be heard nearby.\n");
	    send_to_zone_indoor(tempzone,
				"You can hear a rat screeching from somewhere.\n");
	    break;
	case 2:
	    send_to_zone_outdoor(tempzone,
				 "A child can be heard shouting in the distance.\n");
	    break;
	case 3:
	    send_to_zone_outdoor(tempzone,
				 "From somewhere close by, you can hear a woman's playful scream.\n");
	    break;
	case 4:
	    send_to_zone_outdoor(tempzone,
				 "The annoying sound of a dog's incessant barking can be heard nearby.\n");
	    break;
	case 5:
	    send_to_zone_outdoor(tempzone,
				 "Someone swears loudly a few streets over.\n");
	    break;
	case 6:
	    send_to_zone_outdoor(tempzone,
				 "Someone screams, 'Ouch! .. God Dammit!' from within a building near here.\n");
	    send_to_zone_indoor(tempzone, "You hear a creaking noise.\n");
	    break;
	case 7:
	    send_to_zone_outdoor(tempzone,
				 "The sounds of carriage wheels can be heard on a nearby street.\n");
	    break;
	case 8:
	    send_to_zone_outdoor(tempzone,
				 "There is a loud crashing noise from somewhere close by. *BANG*\n");
	    break;
	case 9:
	    send_to_zone_outdoor(tempzone,
				 "The sounds of laughter can be heard from the next building.\n");
	    break;
	case 10:
	    send_to_zone_outdoor(tempzone,
				 "A cat dashes past you at a full run, disappearing as quick as it came.\n");
	    break;
	case 11:
	    send_to_zone_outdoor(tempzone,
				 "The wonderful smell of charcoal and cooking meat wafts by. *YUM*\n");
	    break;
	case 12:
	    send_to_zone_indoor(tempzone,
				"You can hear a bird's loud call from outside.\n");
	    send_to_zone_outdoor(tempzone,
				 "The awful smell of fresh horse dung wafts by. \n");
	    break;
	case 13:
	    send_to_zone_outdoor(tempzone,
				 "A cityguard yells, 'Stop thief!'\n");
	    break;
	case 14:
	    send_to_zone_outdoor(tempzone,
				 "A child wails, 'MOM!' from somewhere close by.\n");
	    break;
	case 15:
	    send_to_zone_outdoor(tempzone,
				 "You can smell burnt meat coming from one of the nearby buildings.\n");
	    break;
	case 16:
	    send_to_zone_outdoor(tempzone,
				 "A man yells, 'God, I love this city!'\n");
	    break;
	case 17:
	    send_to_zone_outdoor(tempzone,
				 "The sound of someone dropping a metal pot rings through the area.\n");
	    break;
	case 18:
	    send_to_zone_indoor(tempzone,
				"From outside, you can hear a woman's scream..\n");
	    send_to_zone_outdoor(tempzone,
				 "A woman cries out in terror from inside a building near here!\n");
	    break;
	case 19:
	    send_to_zone_outdoor(tempzone,
				 "A cool breeze blows through, bringing some welcome fresh air.\n");
	    break;
	case 20:
	    send_to_zone_outdoor(tempzone,
				 "A wailing cry of a dog in pain can be heard in the distance.\n");
	    break;
	case 21:
	    send_to_zone_outdoor(tempzone,
				 "In the distance, a merchant can be heard loudly soliciting his goods.\n");
	    break;
	case 22:
	    send_to_zone_outdoor(tempzone,
				 "The sizzling sound of cooking meat can be heard.\n");
	    break;
	case 23:
	    send_to_zone_outdoor(tempzone,
				 "A strange smell wafts by, similar to hide or worked leather.\n");
	    break;
	case 24:
	    send_to_zone_indoor(tempzone,
				"You feel a presence .. almost as if you're not alone.\n");
	    send_to_zone_outdoor(tempzone,
				 "An unusually large crow flies overhead, croaking loudly, 'Caw! Caw!'\n");
	    break;
	case 25:
	    send_to_zone_outdoor(tempzone,
				 "A couple can be heard bartering over some purchased goods nearby.\n");
	    break;
	case 26:
	    send_to_zone_outdoor(tempzone,
				 "A sparrow dives on a piece of discarded meat, and flies off.\n");
	    break;
	case 27:
	    send_to_zone_outdoor(tempzone,
				 "From somewhere close, a man shouts, 'Yeah? Well piss off asshole!'\n");
	    break;
	case 28:
	    send_to_zone_outdoor(tempzone,
				 "The strong and appetizing smells of baking bread waft by. *Mmmm*\n");
	    break;
	case 29:
	    send_to_zone_outdoor(tempzone,
				 "The strong odor of a dead animal near here offends your nose.\n");
	    break;
	case 30:
	    send_to_zone_indoor(tempzone,
				"You can make out the sounds of two people arguing from outside.\n");
	    send_to_zone_outdoor(tempzone,
				 "The rising shouts of a lovers' quarrel can be heard near here.\n");
	    break;
	case 31:
	    send_to_zone_outdoor(tempzone,
				 "A dog can be heard chasing someone down a nearby street.\n");
	    break;
	case 32:
	    send_to_zone_outdoor(tempzone,
				 "The cloying smell of ladies perfume wafts by, enticing you.\n");
	    break;
	case 33:
	    send_to_zone_outdoor(tempzone,
				 "Several seagulls fly overhead, calling loudly.\n");
	    break;
	case 34:
	    send_to_zone_outdoor(tempzone,
				 "Several children can be heard playing and yelling loudly in the distance.\n");
	    break;
	case 35:
	    send_to_zone_outdoor(tempzone,
				 "A bird dropping lands in front of you, just missing your head.\n");
	    break;
	case 36:
	    send_to_zone_indoor(tempzone, "A cool draft blows by.\n");
	    send_to_zone_outdoor(tempzone,
				 "The banging sounds of a smithy hammer ring through the air.\n");
	    break;
	case 37:
	    send_to_zone_outdoor(tempzone,
				 "A loud splash can be heard, as if someone dumped a bucket of liquid out a window.\n");
	    break;
	case 38:
	    send_to_zone_outdoor(tempzone,
				 "From inside a building, someone lets out a tremendous belch!\n");
	    break;
	case 39:
	    send_to_zone_outdoor(tempzone,
				 "A rare and beautiful hawk circles overhead, drifting on the wind currents.\n");
	    break;
	case 40:
	    send_to_zone_outdoor(tempzone,
				 "The sound of breaking glass pierces the air, much like that of a fallen bottle.\n");
	    break;
	case 41:
	    send_to_zone_outdoor(tempzone,
				 "Someone yells, 'Hey, you little runt! Get back here!'\n");
	    break;
	case 42:
	    send_to_zone_indoor(tempzone,
				"The smells of cooking food from outside waft by.\n");
	    send_to_zone_outdoor(tempzone,
				 "The gentle and musical sound of singing songbirds fills the air.\n");
	    break;
	case 43:
	    send_to_zone_outdoor(tempzone,
				 "A dark figure dashes in and out of a shadow in your peripheral vision. \n");
	    break;
	case 44:
	    send_to_zone_outdoor(tempzone,
				 "Shiphands can be heard yelling instructions from somewhere in the harbor.\n");
	    break;
	case 45:
	    send_to_zone_outdoor(tempzone,
				 "A woman's erotic cry echoes from within a building near here.\n");
	    break;
	case 46:
	    send_to_zone_outdoor(tempzone,
				 "The 'neigh' of many horses can be heard riding down a nearby street.\n");
	    break;
	case 47:
	    send_to_zone_outdoor(tempzone,
				 "You hear a salesman peddling his goods to someone close by.\n");
	    break;
	case 48:
	    send_to_zone_indoor(tempzone,
				"The is a loud crash from outside!\n");
	    send_to_zone_outdoor(tempzone,
				 "Shouts and running feet can be heard, as if guards were pursuing someone.\n");
	    break;
	case 49:
	    send_to_zone_outdoor(tempzone,
				 "Several guards or soldiers can be heard marching near here.\n");
	    break;
	case 50:
	    send_to_zone_outdoor(tempzone,
				 "The bangs and thuds of loading cargo can be heard from within the harbor.\n");
	    break;
	case 51:
	    send_to_zone_outdoor(tempzone,
				 "A man yells, ' Hey! What are you doing in here, kid?! Get outta here!'\n");
	    break;
	case 52:
	    send_to_zone_outdoor(tempzone,
				 "The ringing of swords can be heard, as if someone were fighting close by!\n");
	    break;
	case 53:
	    send_to_zone_outdoor(tempzone,
				 "A merchant and customer can be heard bartering over a price.\n");
	    break;
	case 54:
	    send_to_zone_indoor(tempzone,
				"A shout can be heard from outside somewhere.\n");
	    send_to_zone_outdoor(tempzone,
				 "The thick smell of cigar smoke wafts by.  \n");
	    break;
	case 55:
	    send_to_zone_outdoor(tempzone,
				 "You can see smoke above the city, as if someone was burning something.\n");
	    break;
	case 56:
	    send_to_zone_outdoor(tempzone,
				 "A loud argument can be heard taking place near here.\n");
	    break;
	case 57:
	    send_to_zone_outdoor(tempzone,
				 "Cheers and jolly shouts can be heard from a tavern near here.\n");
	    break;
	case 58:
	    send_to_zone_outdoor(tempzone,
				 "Someone shouts several instructions over a bullhorn in the harbor.\n");
	    break;
	case 59:
	    send_to_zone_outdoor(tempzone,
				 "A gust of wind whips through the area, amplified by all the buildings.\n");
	    break;
	case 60:
	    send_to_zone_indoor(tempzone,
				"A dog's incessant barking can be heard from outside.\n");
	    send_to_zone_outdoor(tempzone,
				 "You hear someone's tortured groaning from inside a building nearby.\n");
	    break;
	case 61:
	    send_to_zone_outdoor(tempzone,
				 "A grubby boy with apples spilling from his pockets storms by.\n");
	    break;
	case 62:
	    send_to_zone_outdoor(tempzone,
				 "A commoner yells, 'Help! Help! I'm being repressed!'\n");
	    send_to_zone_outdoor(tempzone,
				 "Kaan yells, 'Shut up, you bloody peasant!'\n");
	    break;
	case 63:
	    send_to_zone_outdoor(tempzone,
				 "An old man pushing a two-wheeled cart yells, 'Bring out yer dead!'\n");
	    break;
	case 64:
	    send_to_zone_outdoor(tempzone,
				 "A *large* gooey bird dropping strikes the ground in front of you! *SPLAT*\n");
	    break;
	case 65:
	    send_to_zone_outdoor(tempzone,
				 "The long, eerie howl of a timber wolf can be heard echoing in the distance.\n");
	    break;
	case 66:
	    send_to_zone_outdoor(tempzone,
				 "Someone yells, 'Hey! Would you bring that over here, please?'\n");
	    send_to_zone_indoor(tempzone,
				"You can hear the faint sound of a mouse scrabbling in the walls.\n");
	    break;
	case 67:
	    send_to_zone_outdoor(tempzone,
				 "Someone lets off a resounding, rip-roaring fart about a block or so.away \n");
	    break;
	case 68:
	    send_to_zone_outdoor(tempzone,
				 "You hear the sounds of laughter nearby.\n");
	    break;
	case 69:
	    send_to_zone_outdoor(tempzone,
				 "A guard can be heard shouting instructions to someone.\n");
	    break;
	case 70:
	    send_to_zone_outdoor(tempzone,
				 "The pungent odor of rotting trash floats into the area.\n");
	    break;
	case 71:
	    send_to_zone_outdoor(tempzone,
				 "A child screams in pain from somewhere and begins wailing.\n");
	    break;
	case 72:
	    send_to_zone_outdoor(tempzone,
				 "From within a building, someone says loudly, 'Oops!'\n");
	    send_to_zone_indoor(tempzone,
				"You can hear shouting going on outdoors.\n");
	    break;
	case 73:
	    send_to_zone_outdoor(tempzone,
				 "A large flock of birds flies over this area of the city.\n");
	    send_to_zone_outdoor(tempzone,
				 "A small bird dropping strikes you squarely on the forearm. *SPLOCH*\n");
	    break;
	case 74:
	    send_to_zone_outdoor(tempzone,
				 "You hear a tortured death cry echoing in the distance!\n");
	    break;
	case 75:
	    send_to_zone_outdoor(tempzone,
				 "The sound of someone pounding metal with a smithy hammer echoes in the distance.\n");
	    break;
	case 76:
	    send_to_zone_outdoor(tempzone,
				 "The sounds of a man singing a loud religous hymn echoes throughout the city.\n");
	    break;
	case 77:
	    send_to_zone_outdoor(tempzone,
				 "You can hear the pleasant sound of someone playing a lyre nearby.\n");
	    break;
	case 78:
	    send_to_zone_outdoor(tempzone,
				 "Off in the distance, you can faintly hear the blast of a horn.\n");
	    send_to_zone_indoor(tempzone,
				"Something resembling the blast of a horn can faintly be heard outside.\n");
	    break;
	case 79:
	    send_to_zone_outdoor(tempzone,
				 "A woman yells, 'Tommy! Dinner time! Get yer butt home!'\n");
	    break;
	case 80:
	    send_to_zone_outdoor(tempzone,
				 "An unusually cold wind blows through the area.\n");
	    break;
	case 81:
	    send_to_zone_outdoor(tempzone,
				 "Far in the distance, the echo of an explosion can faintly be heard.\n");
	    break;
	case 82:
	    send_to_zone_outdoor(tempzone,
				 "A few streets down you hear a man yell, 'Keep the noise down, dammit!'\n");
	    break;
	case 83:
	    send_to_zone_outdoor(tempzone,
				 "The cheers of about a dozen people echo through the streets.\n");
	    break;
	case 84:
	    send_to_zone_outdoor(tempzone,
				 "You can hear the music of a snake charmer's flute from somewhere nearby.\n");
	    break;
	case 85:
	    send_to_zone_outdoor(tempzone,
				 "You hear a noise from a nearby rooftop, like that of running footsteps.\n");
	    send_to_zone_indoor(tempzone,
				"You hear someone walking across the rooftop above.\n");
	    break;
	case 86:
	    send_to_zone_outdoor(tempzone,
				 "You hear a the loud *slosh* of water hitting the ground somewhere.\n");
	    break;
	case 87:
	    send_to_zone_outdoor(tempzone,
				 "A salesman yells, 'Come one, come all! Fine goods for sale in the bazaar!'\n");
	    break;
	case 88:
	    send_to_zone_outdoor(tempzone,
				 "A man says, 'Awww, go piss on yourself. That price is too high!' from nearby.\n");
	    break;
	case 89:
	    send_to_zone_outdoor(tempzone,
				 "A dog comes out of nowhere, running quickly past you and out of sight.\n");
	    break;
	case 90:
	    send_to_zone_outdoor(tempzone,
				 "You hear the pleasant sounds of a balalaika from somewhere close by.\n");
	    send_to_zone_indoor(tempzone,
				"You hear the pleasant sounds of a balalaika from somewhere outside.\n");
	    break;
	case 91:
	    send_to_zone_outdoor(tempzone,
				 "You hear a dog barking from somewhere.\n");
	    break;
	case 92:
	    send_to_zone_outdoor(tempzone,
				 "You hear, 'No! No, don't touch that!' and then a loud explosion *BOOM*\n");
	    break;
	case 93:
	    send_to_zone_outdoor(tempzone,
				 "You hear the loud frightened neigh of a spooked horse from somewhere nearby.\n");
	    break;
	case 94:
	    send_to_zone_outdoor(tempzone,
				 "A child and his dog race by you, with the child giggling wildly. \n");
	    break;
	case 95:
	    send_to_zone_outdoor(tempzone,
				 "You hear the heated tones of an argument taking place nearby.\n");
	    send_to_zone_indoor(tempzone,
				"You hear the heated tones of an argument taking place outside.\n");
	    break;
	case 96:
	    send_to_zone_outdoor(tempzone,
				 "You hear the gleeful shouts of girls playing nearby.\n");
	    break;
	case 97:
	    send_to_zone_outdoor(tempzone,
				 "A small cloud of smoke floats into the area from someone's cooking fire.\n");
	    break;
	case 98:
	    send_to_zone_outdoor(tempzone,
				 "You hear a man yell, 'Be quiet! Or I shall taunt thee a second time!'\n");
	    break;
	case 99:
	    send_to_zone_outdoor(tempzone,
				 "You hear the sounds of dice hitting a wall nearby, as two men gamble.\n");
	    break;
	case 100:
	    send_to_zone_outdoor(tempzone,
				 "The stench of urine floats into the area, offending your nose.\n");
	    send_to_zone_indoor(tempzone,
				"The stench of urine floats in from outside, offending your nose.\n");
	    break;
	default:
	    break;
	}
    }
    if (noisezone == 4)
	noisezone = 0;
    noisezone++;

}

SPECIAL(bell)
{
    int zone = real_room(7261);

    if ((zone != 0) && (zone != NOWHERE))
	zone = world[zone].zone;



    if (CMD_IS("ring")) {
	send_to_zone_outdoor(zone,
			     "An alarm bell rings loudly in the distance!\r\n");
	send_to_zone_indoor(zone, "A loud alarm bell rings outside!\r\n");
	return TRUE;
    }
    return FALSE;
}

SPECIAL(bounty)
{
    struct char_data *victim;
    struct descriptor_data *i;

    if (CMD_IS("bounty")) {

	one_argument(argument, arg);

	if (!*arg) {
	    send_to_char
		("Who is it that you wish to place a bounty on?\r\n", ch);
	    return TRUE;
	}
	for (i = descriptor_list; i; i = i->next) {
	    if (!i->connected && i->character && i->character != ch) {
		victim = i->character;
		if (!(victim = get_char_vis(ch, arg))) {
		    send_to_char(NOPERSON, ch);
		    return TRUE;
		}
		if (victim == ch) {
		    send_to_char
			("You want to place a bounty on yourself? Nah.\r\n",
			 ch);
		    return TRUE;
		}
		if (GET_LEVEL(victim) >= LVL_IMMORT) {
		    send_to_char
			("Let's just call that person 'priviledged', okay?\r\n",
			 ch);
		    return TRUE;
		}
		if (GET_LEVEL(victim) < 100 && GET_CLASS(victim) < 9) {
		    send_to_char
			("You can't place bounties on players under level "
			 "100 unless they are remorts.\r\n", ch);
		    return TRUE;
		}
		if (GET_GOLD(ch) < 550000) {
		    send_to_char
			("You do not have the funds to place a bounty upon them.\r\n"
			 "The cost to place a bounty is 550,000 gold.\r\n",
			 ch);
		    return TRUE;
		}
		if (PLR_FLAGGED(victim, PLR_BOUNTY)) {
		    send_to_char
			("That person already has a bounty on them.\r\n",
			 ch);
		    return TRUE;
		} else
		    SET_BIT_AR(PLR_FLAGS(victim), PLR_BOUNTY);
		GET_GOLD(ch) = GET_GOLD(ch) - 550000;
		send_to_char
		    ("You pay the 550,000 gold to place the bounty.\r\n",
		     ch);
		send_to_char
		    ("A bounty has been placed upon your head - watch your "
		     "back!\r\n", victim);
		sprintf(buf,
			"\r\n/cRBOUNTY:: A bounty has been placed upon %s's "
			"life!/c0\r\n", GET_NAME(victim));
		send_to_all(buf);
		return TRUE;
	    }
	}
    }
    return FALSE;
}


SPECIAL(poshuls_tower)
{
    int collar;

    collar = 50034;

    if (!IS_MOVE(cmd) && !CMD_IS("chronocross"))
	return FALSE;

    if (CMD_IS("chronocross")) {
	if (GET_EQ(ch, WEAR_NECK_1) || GET_EQ(ch, WEAR_NECK_2)) {
	    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_NECK_1)) == collar) {
		send_to_char
		    ("/cc\r\nYou are blinded by a sudden flash of light!\r\n\r\n",
		     ch);
		char_from_room(ch);
		char_to_room(ch, real_room(50050));
		look_at_room(ch, 0);
		act("$n vanishes in a puff of smoke!", FALSE, ch, 0, 0,
		    TO_ROOM);
		return TRUE;
	    }
	    if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_NECK_2)) == collar) {
		send_to_char
		    ("/cc\r\nYou are blinded by a sudden flash of light!\r\n\r\n",
		     ch);
		char_from_room(ch);
		char_to_room(ch, real_room(50050));
		look_at_room(ch, 0);
		act("$n vanishes in a puff of smoke!", FALSE, ch, 0, 0,
		    TO_ROOM);
		return TRUE;
	    }
	}
	send_to_char
	    ("Nothing happens... but you are on the right track.\r\n", ch);
	return TRUE;
    }
    return FALSE;
}

SPECIAL(bridgeguard)
{
    struct obj_data *obj;

    if (cmd == SCMD_EAST) {
	if (!(obj = get_obj_in_list_vis(ch, "bridge_pass", ch->carrying))) {
	    act("The BridgeGuard steps in front of $n.\r\n"
		"'/cwYou cannot enter the castle without a Bridge Pass./c0'",
		FALSE, ch, 0, 0, TO_ROOM);
	    act("The BridgeGuard steps in front of you.\r\n"
		"'/cwYou cannot enter the castle without a Bridge Pass./c0'",
		FALSE, ch, 0, 0, TO_CHAR);
	    return TRUE;
	} else
	    act("The BridgeGuard bows as you pass.",
		FALSE, ch, 0, 0, TO_ROOM);
    }
    return FALSE;
}


SPECIAL(undertaker)
{
    struct obj_data *i;
    int num, gold;

    num = 0;

    if (!CMD_IS("retrieve"))
	return 0;

    gold = GET_LEVEL(ch) * 5000;

    for (i = object_list; i; i = i->next) {
	if (!isname(GET_NAME(ch), i->name))
	    continue;

	if (i->in_room != NOWHERE) {
	    if (GET_OBJ_TYPE(i) == ITEM_CONTAINER) {
		if (GET_OBJ_VAL(i, 3) == 1) {
		    if (GET_GOLD(ch) <= gold) {
			send_to_char
			    ("The undertaker says, '/cwYou haven't the funds for this service.  Come back when you can pay me./c0'\r\n",
			     ch);
			return TRUE;
		    }
		    if (num == 0) {
			obj_from_room(i);
			obj_to_room(i, ch->in_room);
			num = 1;
			GET_GOLD(ch) = GET_GOLD(ch) - gold;
			send_to_char
			    ("The undertaker bows as you pay him the money.\r\n",
			     ch);
			send_to_char
			    ("Your corpse is brought in by two burly men who leave immediately.\r\n",
			     ch);
			return TRUE;
		    }
		}
	    }
	} else
	    send_to_char
		("The undertaker says, '/cwI cannot find your corpse./c0'\r\n",
		 ch);
	return TRUE;
    }
    send_to_char
	("The undertaker says, '/cwI cannot find your corpse./c0'\r\n",
	 ch);
    return TRUE;
}

void reimb(char *arg)
{
/* Let's assume all of the good stuff is set up by a wrapper
    char *middle, *name;
 
    name = LOWER(*arg);

    switch (name) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
	middle = "A-E";
	break;
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
	middle = "F-J";
	break;
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
	middle = "K-O";
	break;
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
	middle = "P-T";
	break;
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
	middle = "U-Z";
	break;
    default:
	middle = "ZZZ";
	break;
    }

    sprintf(buf,
	    "cp -f /home/dibrova/bk/plrobjs/%s/%s.objs /home/dibrova/prod/plrobjs/%s",
	    middle, name, middle);
    system(buf);
*/
}

SPECIAL(make_vamp)
{
    if (!CMD_IS("supplicate"))
	return FALSE;

    if (GET_LEVEL(ch) <= 149) {
	send_to_char
	    ("Only level 150 remorts can be turned to the darkness.\r\n",
	     ch);
	return FALSE;
    }

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
	send_to_char("Trust me, you don't wanna do that.\r\n", ch);
	send_to_char("Kaan did that once... wasn't pretty.\r\n", ch);
	return FALSE;
    }

    if (!IS_REMORT(ch)) {
	send_to_char
	    ("Only level 150 remorts can be turned to the darkness.\r\n",
	     ch);
	return FALSE;
    }

    else if (CMD_IS("supplicate")) {
	GET_CLASS(ch) = CLASS_VAMPIRE;
	GET_LEVEL(ch) = 1;
	GET_TIER(ch) = 1;
	GET_EXP(ch) = 0;
	GET_WIMP_LEV(ch) = 0;
	SET_SKILL(ch, SKILL_REND, 75);
	SET_SKILL(ch, SKILL_SECOND_ATTACK, 95);
	SET_SKILL(ch, SKILL_THIRD_ATTACK, 95);
	SET_SKILL(ch, SKILL_FOURTH_ATTACK, 95);
	SET_SKILL(ch, SKILL_FIFTH_ATTACK, 95);
	SET_SKILL(ch, SKILL_SIXTH_ATTACK, 95);
	act("You get an overpowering sensation of falling through ages "
	    "of life and death,\r\n"
	    "torment and ecstasy. You feel your vision grow sharp, your "
	    "incisors elongate.\r\n"
	    "You have become what most of the civilized world hunts and "
	    "abhors.\r\n\r\n"
	    "/crYou are vampire. Creature of Eternal Night./c0\r\n",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("A shroud of darkness suddenly blankets the room. There are "
	    "dark workings within this place.\r\n", FALSE, ch, 0, 0,
	    TO_ROOM);
	affect_total(ch);
	double_exp = 20;
	sprintf(buf, "/crA shroud of darkness suddenly blankets the world.\r\n%s has become a Vampire, Creature of Eternal Night./c0\r\n", GET_NAME(ch));
	sprintf(buf, "%s/cRDouble Experience is declared for the next 20 minutes, \r\nin honour of %s./c0\r\n", buf, GET_NAME(ch));
	send_to_all(buf);
	return TRUE;
    }
    return FALSE;
}

SPECIAL(immort_teacher)
{

    int i, found = FALSE;

    /* mob vnum, skill */
    const int teacher_nums[][4] = {
  /* Vnum | Ability               |     Class         | Type      */
	/*-----[ Vampires ]-----*/
	{26700, SKILL_TEAR,					CLASS_VAMPIRE,	ABT_SKILL},
	{26701, SKILL_HOVER,				CLASS_VAMPIRE,	ABT_SKILL},
	{26702, SKILL_VANISH,				CLASS_VAMPIRE,	ABT_SKILL},
	{26703, SKILL_FIND_FLESH,			CLASS_VAMPIRE,	ABT_SKILL},
	{26704, SKILL_MESMERIZE,			CLASS_VAMPIRE,	ABT_SKILL},
	{26705, SKILL_HISS,					CLASS_VAMPIRE,	ABT_SKILL},
	{26706, SKILL_SWOOP,				CLASS_VAMPIRE,	ABT_SKILL},
	{26707, SKILL_SACRED_EARTH,			CLASS_VAMPIRE,	ABT_SKILL},
	{26708, SKILL_DEATH_CALL,			CLASS_VAMPIRE,	ABT_SKILL},
	{26709, SKILL_SPIT_BLOOD,			CLASS_VAMPIRE,	ABT_SKILL},
	/*-----[ Titans ]-----*/
	{26710, SKILL_SCORCH,				CLASS_TITAN,	ABT_SKILL},
	{26711, SKILL_ABSORB,				CLASS_TITAN,	ABT_SKILL},
	{26712, SKILL_CLOUD,				CLASS_TITAN,	ABT_SKILL},
	{26713, SKILL_SUMMON_ELEM,			CLASS_TITAN,	ABT_SKILL},
	{26714, SKILL_INTIMIDATE,			CLASS_TITAN,	ABT_SKILL},
	{26715, SKILL_BELLOW,				CLASS_TITAN,	ABT_SKILL},
	/*-----[ Saints ]-----*/
	{26716, PRAY_HOLY_WATER,			CLASS_SAINT,	ABT_PRAYER},
	{26717, PRAY_EXORCISM,				CLASS_SAINT,	ABT_PRAYER},
	{26718, PRAY_AFTERLIFE,				CLASS_SAINT,	ABT_PRAYER},
	{26719, PRAY_HEAVENSBEAM,			CLASS_SAINT,	ABT_PRAYER},
	{26720, PRAY_DIVINESHIELD,			CLASS_SAINT,	ABT_PRAYER},
	{26721, PRAY_ETHEREAL,				CLASS_SAINT,	ABT_PRAYER},
	{26722, PRAY_CHOIR,					CLASS_SAINT,	ABT_PRAYER},
	{26723, PRAY_ENLIGHTENMENT,			CLASS_SAINT,	ABT_PRAYER},
	/*-----[ Demons ]-----*/
	{26724, SKILL_DECEIVE,				CLASS_DEMON,	ABT_SKILL},
	{26725, SKILL_DARK_ALLEGIANCE,		CLASS_DEMON,	ABT_SKILL},
	{26726, SKILL_ABSORB_FIRE,			CLASS_DEMON,	ABT_SKILL},
	{26727, SKILL_SOULSUCK,				CLASS_DEMON,	ABT_SKILL},
	{26728, SKILL_BESTOW_WINGS,			CLASS_DEMON,	ABT_SKILL},
	{26729, SPELL_NETHERCLAW,			CLASS_DEMON,	ABT_SPELL},
	{26730, SPELL_CONJURE_FIREDRAKE,	CLASS_DEMON,	ABT_SPELL},
	{26731, SPELL_RAIN_OF_FIRE,			CLASS_DEMON,	ABT_SPELL},
	{-1, -1, -1}
    };

    for (i = 0; teacher_nums[i][0] > 0 && !found; i++)
	if (GET_MOB_VNUM((struct char_data *) me) == teacher_nums[i][0] &&
	    IN_ROOM(ch) == IN_ROOM((struct char_data *) me))
	    found = TRUE;

    if (!found)
	return FALSE;
    i--;

    if (CMD_IS("kneel")) {
	if (GET_CLASS(ch) != teacher_nums[i][2]) {
	    send_to_char
		("You kneel, but this is not one who can teach you.\r\n",
		 ch);
	    return TRUE;
	} else if (GET_SKILL(ch, teacher_nums[i][1]) >= 75) {
	    send_to_char
		("You have already learned all of that which can be "
		 "taught here.\r\n", ch);
	    return TRUE;
	} else {
		if (teacher_nums[i][3] == ABT_SKILL)
			SET_SKILL(ch, teacher_nums[i][1], 95);
		else if (teacher_nums[i][3] == ABT_SPELL)
			SET_SPELL(ch, teacher_nums[i][1], 95);
		else if (teacher_nums[i][3] == ABT_PRAYER)
			SET_PRAYER(ch, teacher_nums[i][1], 95);
		else if (teacher_nums[i][3] == ABT_CHANT)
			SET_CHANT(ch, teacher_nums[i][1], 95);
		else if (teacher_nums[i][3] == ABT_SONG)
			SET_SONG(ch, teacher_nums[i][1], 95);
	}
	act("You kneel down on one knee, closing your eyes.\r\n"
	    "You feel a very light touch on your right shoulder and that is all.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act
	    ("$n kneels down on one knee as $N touches $m lightly on one shoulder.",
	     FALSE, ch, 0, (struct char_data *) me, TO_ROOM);
	return TRUE;
    }
    return FALSE;
}




#define DTI_PRICE(ch)  ((GET_LEVEL(ch) * 50) + (GET_TIER(ch) * 7500))

SPECIAL(broker)
{
    if (CMD_IS("offer")) {

	sprintf(buf,
		"\r\nA one-time processing fee of %d is due upon purchase.\r\n"
		"Equipment insurance will cost you %d gold per day.\r\n"
		"First installment due immediately, all subsequent "
		"payments \r\nwill be auto-withdrawn"
		"from your bank account.\r\n", (DTI_PRICE(ch) * 10),
		DTI_PRICE(ch));
	send_to_char(buf, ch);

	return (TRUE);
    }

    if (CMD_IS("cancel")) {

	if (!PLR_FLAGGED(ch, PLR_INSURED)) {
	    send_to_char("\r\nYou have no policy to cancel.\r\n"
			 "You are currently uninsured.\r\n", ch);
	    return TRUE;
	} else
	    send_to_char
		("\r\nAfter a thorough check of your records, we find that you\r\n"
		 "still owe an outstanding amount to reconcile your account.\r\n"
		 "However, since you have been such a good customer in the\r\n"
		 "past, we would like to give you every reason to come back.\r\n"
		 "Your outstanding debt has been erased and your policy cleared.\r\n"
		 "You no longer are insured through the Midgaard Equipment Underwriters, Inc.\r\n"
		 "\r\nHave a nice day.\r\n", ch);
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_INSURED);
	return TRUE;

    } else if (CMD_IS("buy")) {

	if (IS_NPC(ch)) {
	    send_to_char("Mobs dont get Eq Insurance!\r\n", ch);
	    return (TRUE);
	}
	if (GET_GOLD(ch) < DTI_PRICE(ch)) {
	    send_to_char("\r\nYou don't have enough gold!\r\n", ch);
	    return (TRUE);
	}
	GET_GOLD(ch) -= DTI_PRICE(ch);

	SET_BIT_AR(PLR_FLAGS(ch), PLR_INSURED);
	send_to_char("Your equipment is now insured.\r\n", ch);

	return (TRUE);
    }
    return (FALSE);
}

SPECIAL(seven_wards)
{
    struct obj_data *ward1, *ward2, *ward3, *ward4, *ward5, *ward6, *ward7,
	*staff;
    int r_num;

    if (!CMD_IS("kneel"))
	return 0;

    if (GET_RACE(ch) == RACE_ELDAR) {
	if (get_obj_in_list_vis(ch, "one_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "two_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "three_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "four_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "five_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "six_ward", ch->carrying) &&
	    get_obj_in_list_vis(ch, "seven_ward", ch->carrying)) {

	    ward1 = get_obj_in_list_vis(ch, "one_ward", ch->carrying);
	    ward2 = get_obj_in_list_vis(ch, "two_ward", ch->carrying);
	    ward3 = get_obj_in_list_vis(ch, "three_ward", ch->carrying);
	    ward4 = get_obj_in_list_vis(ch, "four_ward", ch->carrying);
	    ward5 = get_obj_in_list_vis(ch, "five_ward", ch->carrying);
	    ward6 = get_obj_in_list_vis(ch, "six_ward", ch->carrying);
	    ward7 = get_obj_in_list_vis(ch, "seven_ward", ch->carrying);

	    r_num = real_object(41799);
	    staff = read_object(r_num, REAL);

	    send_to_char
		("You kneel upon the temple floor, offering the Seven Wards "
		 "as tribute to the Eldar Gods.\r\n", ch);
	    send_to_char
		("You are immediately engulfed in a flash of arcane light!\r\n\r\n",
		 ch);
	    send_to_char
		("You hear the voice of the God Vedic, token God of the Eldar race, say:\r\n",
		 ch);
	    send_to_char
		("'/cwYou have done well, my subject. Take with you this great token of power.'\r\n",
		 ch);
	    send_to_char("'/cwUse its power wisely./c0'\r\n\r\n", ch);
	    send_to_char
		("As the light that is Vedic withdraws, you realize that the Seven Wards \r\n",
		 ch);
	    send_to_char
		("in your possession have been replaced with a single item.\r\n",
		 ch);
	    act
		("$n is engulfed in /cWbrilliant white light/c0 as $e kneels upon the temple floor,\r\n",
		 FALSE, ch, 0, 0, TO_ROOM);

	    obj_from_char(ward1);
	    obj_from_char(ward2);
	    obj_from_char(ward3);
	    obj_from_char(ward4);
	    obj_from_char(ward5);
	    obj_from_char(ward6);
	    obj_from_char(ward7);
	    obj_to_char(staff, ch);
	    save_char(ch, NOWHERE);
	    return (TRUE);
	} else
	    send_to_char("You kneel upon the floor of the temple.\r\n",
			 ch);
	send_to_char
	    ("You do not have all seven wards in your possession.\r\n",
	     ch);
	send_to_char("Find the rest before trying again.\r\n", ch);
	act
	    ("$n kneels upon the temple floor, praying to what Gods may listen.\r\n",
	     FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_RESTING;
	return (TRUE);
    } else
	send_to_char
	    ("You kneel on temple floor, praying to what Gods may listen.\r\n",
	     ch);
    act
	("$n kneels upon the temple floor, praying to what Gods may listen.\r\n",
	 FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_RESTING;
    return (TRUE);
}

SPECIAL(pkill_allow)
{
    if (!CMD_IS("pkill"))
	return 0;

    if (PLR_FLAGGED(ch, PLR_PKILL_OK)) {
	send_to_char("You are already a pkiller.\r\n", ch);
	send_to_char("This is irreversable.\r\n", ch);
	return (TRUE);
    }

    SET_BIT_AR(PLR_FLAGS(ch), PLR_PKILL_OK);
    send_to_char
	("You may now kill other players who also have chosen to pkill.\r\n",
	 ch);
    send_to_char("There is no way to reverse your decision.\r\n", ch);
    return (TRUE);
}

const int grid_trans[30][4] = {

  {   1,   5, 26750, 26000 },
  {   6,  10, 26751, 26000 },
  {  11,  15, 26752, 26000 },
  {  16,  20, 26753, 26000 },
  {  21,  25, 26754, 26100 },
  {  26,  30, 26755, 26100 },
  {  31,  35, 26756, 26100 },
  {  36,  40, 26757, 26100 },
  {  41,  45, 26758, 26200 },
  {  46,  50, 26759, 26200 },
  {  51,  55, 26760, 26200 },
  {  56,  60, 26761, 26200 },
  {  61,  65, 26762, 26300 },
  {  66,  70, 26763, 26300 },
  {  71,  75, 26764, 26300 },
  {  76,  80, 26765, 26300 },
  {  81,  85, 26766, 26400 },
  {  86,  90, 26767, 26400 },
  {  91,  95, 26768, 26400 },
  {  96, 100, 26769, 26400 },
  { 101, 105, 26770, 26500 },
  { 106, 110, 26771, 26500 },
  { 111, 115, 26772, 26500 },
  { 116, 120, 26773, 26500 },
  { 121, 125, 26774, 26600 },
  { 126, 130, 26775, 26600 },
  { 131, 135, 26776, 26600 },
  { 136, 140, 26777, 26600 },
  { 141, 145, 26778, 26700 },
  { 146, 150, 26779, 26700 }

};

#define GRID_TOKEN        25020

SPECIAL(grid_dude)
{
  void reset_zone(int zone);
  struct obj_data *item;
  struct char_data *i;
  bool found = 0;
  int j=0;

  if (CMD_IS("trade")) {
    for (item = ch->carrying; item; item = item->next_content) {
      if (GET_OBJ_VNUM(item) == GRID_TOKEN) {
        found = 1;
        obj_from_char(item);
        extract_obj(item);
        send_to_char("You receive one quest point in trade for your token.\r\n)", ch);
        act("The Grid Master takes $n's token in trade.\r\n", 
             FALSE, ch, 0, 0, TO_ROOM);
        GET_QPOINTS(ch) += 1;
        return 1;
      }
    }
    if (!found) {
        send_to_char("You don't have a Grid Token to trade.\r\n", ch);
        return 1;
    }
  } else
    if (!CMD_IS("grid"))
	return FALSE;

    if (GET_CLASS(ch) >= 9) {
	send_to_char("You cannot use the Grid after attaining remort.\r\n",
		     ch);
	return 1;
    }
    if (GET_LEVEL(ch) >= LVL_IMMORT) {
	send_to_char
	    ("Use goto, jackass. You don't need to use tokens to get to the Grid.\r\n",
	     ch);
	return 1;
    }

    /* Make sure there is no one in the Grid already */
    for (i = character_list; i; i = i->next)
      if (!PRF_FLAGGED(ch, PRF_NOGRID)) {
        if (!IS_NPC(i) && GET_LEVEL(i) < LVL_IMMORT) {
 	  if (zone_table[world[real_room(26000)].zone].number ==
	      zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26100)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26200)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26300)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26400)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26500)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26600)].zone].number ==
              zone_table[world[IN_ROOM(i)].zone].number ||
              zone_table[world[real_room(26700)].zone].number ==
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

    for (item = ch->carrying; item; item = item->next_content) {
      if (GET_OBJ_VNUM(item) == GRID_TOKEN) {
        found = 1;
        obj_from_char(item);
        extract_obj(item);
        act("$n is whisked away to the Grid.\r\n", FALSE, ch, 0, 0, TO_ROOM);
      }
    }
    if (!found) {
	send_to_char("You need a Grid Token to enter the Grid.\r\n", ch);
	return 1;
    }
    for (j=0; j <= 30; j++)
      if (GET_LEVEL(ch) <= grid_trans[j][1] && 
          GET_LEVEL(ch) >= grid_trans[j][0]) {
        reset_zone(world[real_room(grid_trans[j][3])].zone);
        char_from_room(ch);
        char_to_room(ch, real_room(grid_trans[j][2]));
      }

    if (!PRF_FLAGGED(ch, PRF_NOGRID)) {
	look_at_room(ch, 0);
	if (ch->affected) {
	    while (ch->affected)
		affect_remove(ch, ch->affected);
	    send_to_char("All spells removed.\r\n", ch);
	}
    }
    return TRUE;
}

//Moves to random room (26k-26104) at tick.
SPECIAL(greenber) 
{
  int num = 0;
   
  if(cmd)
    return 0;

  if (FIGHTING(ch)) return 0;
  
  num = real_room(number(27100, 27199));
  if(num) {
  act("A greenber wrinkles its furry nose and vanishes from your sight.",FALSE,ch,0, 0 , TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, num);  
  act("A greenber pops out of nowhere.",FALSE,ch,0, 0 , TO_ROOM);
  }
  return 1;    
} 
 
#define ROOM_HARRIED_MAID_TARGET 27160
SPECIAL(harried_maid)
{
  struct char_data *vict=0, *vict2;

  if(cmd) 
    return 0;
  
  act("A harried maid says, 'Leave me alone.'",FALSE,ch,0,vict,TO_NOTVICT);

  for (vict = world[ch->in_room].people; vict; vict = vict2) {
    vict2 = vict->next_in_room;
    if ((!number(0,3)) && (vict != ch)){
      act("A harried maid pushes $N west!",FALSE,ch,0,vict,TO_NOTVICT);
      act("A harried maid pushes you west!",FALSE,ch,0,vict,TO_VICT);
      char_from_room(vict);
      char_to_room(vict, real_room(ROOM_HARRIED_MAID_TARGET));
      look_at_room(vict, 0);
      break;
    }  
  }
  if (vict) return 1;
  return 0;
}

SPECIAL(Maniac_rapist_with_money)
{
  if (cmd) return FALSE;
  command_interpreter(ch,"cackle");
  command_interpreter(ch,"hide");
  return 1;
}

//Block player from going north.
SPECIAL(Brutal_guard)
{
  struct char_data *guard = (struct char_data *) me;
  char *buf = "A brutal guard pokes his scythe in your face.\r\n";
  char *buf2 = "A brutal guard pokes his scythe in $n's face.";
   
  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND) ||
      (GET_LEVEL(ch) >= LVL_IMMORT))
    return FALSE;

  if (!IS_NPC(ch)) {
      send_to_char(buf, ch);
      act(buf2, FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
  }

  return FALSE;
}

SPECIAL(Burly_gruff_guard)   
{
  struct char_data *guard = (struct char_data *) me;
  char *buf = "A burly, gruff guard pokes his axe in your face.\r\n";
  char *buf2 = "A burly, gruff guard pokes his axe in $n's face.";

  if (!IS_MOVE(cmd) || IS_AFFECTED(guard, AFF_BLIND) || (GET_LEVEL(ch) >= LVL_IMMORT))
    return FALSE;


  if (IS_NPC(ch)) {
    send_to_char(buf, ch);
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
  }

  return FALSE;

}

SPECIAL(scaredy_cat)
{
  int num = 0;
  char *buf = "A phantom ghost suddenly appears before you causing you to flee.\r\n";
  char *buf2 = "The sudden appearance of a phantom ghost causes $n to flee.";

  if (!IS_NPC(ch))  
  {
	num = number(0, 4);
	if (num == 1)
	{
	   FIGHTING(ch) = ch;
	   GET_POS(ch) = POS_FIGHTING;
       send_to_char(buf, ch);
       act(buf2, FALSE, ch, 0, 0, TO_ROOM);
	   do_flee(ch,0,0,0);
	   FIGHTING(ch) = NULL;
	   GET_POS(ch) = POS_STANDING;
       return TRUE;
	}
  }
  return FALSE;
}
