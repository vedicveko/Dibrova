/*************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "arena.h"
#include "dg_scripts.h"
#include "clan.h"

/* Structures */
struct char_data *combat_list = NULL;	/* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;

/* External structures */
extern struct room_data *world;
void save_corpses(void);
extern struct message_list weapon_messages[MAX_MESSAGES];
extern struct message_list skill_messages[MAX_MESSAGES];
extern struct message_list spell_messages[MAX_MESSAGES];
extern struct message_list chant_messages[MAX_MESSAGES];
extern struct message_list prayer_messages[MAX_MESSAGES];
extern struct message_list song_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern int pk_allowed;		/* see config.c */
extern int auto_save;		/* see config.c */
extern int max_exp_gain;	/* see config.c */
extern int max_exp_loss;	/* see config.c */
extern int double_exp;
extern room_rnum r_immort_start_room;
extern room_rnum r_frozen_start_room;
extern struct descriptor_data *descriptor_list;
extern int newbie_level;
extern struct aq_data *aquest_table;
extern struct abil_info_type abil_info[5][MAX_ABILITIES + 1];
extern struct index_data *mob_index;
extern int newbie_level;
extern int glad_bet_amnt;
extern int pit_mob_vnum;
void raw_kill(struct char_data *ch, struct char_data *killer);
void die(struct char_data *ch, struct char_data *killer);
void pc_death_return(struct char_data *ch, struct char_data *killer);


/* External procedures */
void save_corpses(void);
char *fread_action(FILE * fl, int nr);
char *fread_string(FILE * fl, char *error);
void stop_follower(struct char_data *ch);
ACMD(do_flee);
void hit(struct char_data *ch, struct char_data *victim, int type);
void forget(struct char_data *ch, struct char_data *victim);
void clearMemory(struct char_data *ch);
void remember(struct char_data *ch, struct char_data *victim);
int ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim);
void arena_kill(struct char_data *ch);
void saint_kill(struct char_data *ch);
int adjust_damage(struct char_data *ch, int damage);
void diag_char_to_char(struct char_data *i, struct char_data *ch);
int level_exp(int class, int level);
ACMD(do_split);
ACMD(do_get);
ACMD(do_sac);
extern void send_to_arena(char *);
extern char *rank[][3];
void advanced_mobile_combat(struct char_data *ch, struct char_data *vict);
void monk_technique(struct char_data *ch);
void hunt_victim(struct char_data *ch);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] = {
    {"hit", "hits"},		/* 0 */
    {"sting", "stings"},
    {"whip", "whips"},
    {"slash", "slashes"},
    {"bite", "bites"},
    {"bludgeon", "bludgeons"},	/* 5 */
    {"crush", "crushes"},
    {"pound", "pounds"},
    {"claw", "claws"},
    {"maul", "mauls"},
    {"thrash", "thrashes"},	/* 10 */
    {"pierce", "pierces"},
    {"blast", "blasts"},
    {"punch", "punches"},
    {"stab", "stabs"},
    {"spell", "burns"},		/* 15 */
    {"embrace", "embraces"},
    {"jab", "jabs"},
    {"cleave", "cleaves"}
};

#define IS_WEAPON(type) (((type) >= TYPE_HIT) && ((type) < TYPE_SUFFERING))

// Funcs to control mob reaction to ranged combat
#define TOROOM(x, y)    (world[(x)].dir_option[(y)]->to_room)

int react_first_step(struct char_data *ch, struct char_data *victim)
{
    int dir = 0;
    room_rnum x = victim->in_room;

    for (dir = 0; dir < NUM_OF_DIRS; dir++) {
	if (world[x].dir_option[dir]) {
	    if (TOROOM(x, dir) == NOWHERE || !CAN_GO(victim, dir))
		continue;

	    if (EXIT(victim, dir)->to_room == IN_ROOM(ch)) {
		return (dir);
	    }
	} else
	    continue;
    }
    return (-1);
}

void summon_ranged_helpers(struct char_data *ch, struct char_data *victim,
			   int dir, room_rnum was_in)
{
    struct follow_type *k, *next;
    struct char_data *tch, *next_tch;

    for (k = victim->followers; k; k = next) {
	next = k->next;
	if ((k->follower->in_room == was_in) &&
	    (GET_POS(k->follower) >= POS_STANDING)) {
	    perform_move(k->follower, dir, 1);
	    hit(k->follower, ch, TYPE_UNDEFINED);
	}
    }
    for (tch = world[was_in].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (!MOB_FLAGGED(tch, MOB_HELPER))
	    continue;
	if (AFF_FLAGGED(tch, AFF_BLIND | AFF_CHARM))
	    continue;
	if (GET_POS(tch) <= POS_SLEEPING)
	    continue;
	if (FIGHTING(tch) && FIGHTING(tch) != ch)
	    continue;
	perform_move(tch, dir, 1);
	hit(tch, ch, TYPE_UNDEFINED);
    }

    return;
}

void mob_reaction(struct char_data *ch, struct char_data *victim)
{
    int dir;
    room_rnum was_in = victim->in_room;

    if (AFF_FLAGGED(victim, AFF_BLIND) || AFF_FLAGGED(victim, AFF_CHARM))
	return;
    if (GET_POS(victim) <= POS_STUNNED)
	return;
    if (FIGHTING(victim) && FIGHTING(victim) != ch)
	return;

    if (IS_NPC(victim) && (FIGHTING(victim) == ch)) {

	if (MOB_FLAGGED(victim, MOB_MEMORY))
	    remember(victim, ch);

	sprintf(buf, "$n bellows in pain!");
	act(buf, FALSE, victim, 0, 0, TO_ROOM);
	GET_POS(victim) = POS_STANDING;

	if ((dir = react_first_step(ch, victim)) < 0) {
	    sprintf(buf, "$n looks around, utterly confused...");
	    act(buf, FALSE, victim, 0, 0, TO_ROOM);
	    return;
	} else {
	    if (!do_simple_move(victim, dir, 1))
		return;
	}

	if (victim->in_room == ch->in_room) {
	    hit(victim, ch, TYPE_UNDEFINED);
	    summon_ranged_helpers(ch, victim, dir, was_in);
	} else {
	    if (!MOB_FLAGGED(victim, MOB_MEMORY)) {
		SET_BIT_AR(MOB_FLAGS(victim), MOB_MEMORY);
		remember(victim, ch);
	    }
	    HUNTING(victim) = ch;
	    hunt_victim(victim);
	}
    }

}




/* damage dice for monk bare hand  -Rapax 11/18/99 */

const int monk_bare_hand[][2] = {
    {0, 0},
    {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3},
    {1, 3}, {2, 2}, {2, 2}, {2, 2}, {2, 2},	/* 10 */
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {2, 2}, {2, 2}, {2, 3}, {2, 3}, {2, 3},	/* 20 */
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 4}, {2, 4},	/* 30 */
    {2, 4}, {2, 4}, {2, 4}, {2, 4}, {2, 4},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 4},	/* 40 */
    {3, 4}, {3, 4}, {3, 4}, {3, 4}, {3, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},	/* 50 */
    {5, 3}, {5, 3}, {5, 3}, {5, 3}, {5, 3},
    {4, 5}, {4, 5}, {4, 5}, {4, 5}, {4, 5},	/* 60 */
    {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4},
    {4, 6}, {4, 6}, {4, 6}, {4, 6}, {4, 6},	/* 70 */
    {5, 5}, {5, 5}, {5, 5}, {5, 5}, {5, 5},
    {5, 5}, {5, 5}, {5, 5}, {5, 5}, {4, 7},	/* 80 */
    {4, 7}, {4, 7}, {4, 7}, {5, 6}, {5, 6},
    {5, 6}, {5, 6}, {5, 6}, {5, 6}, {5, 6},	/* 90 */
    {5, 6}, {5, 6}, {5, 6}, {5, 6}, {5, 6},
    {4, 8}, {4, 8}, {4, 8}, {4, 8}, {5, 7},	/* 100 */
    {5, 7}, {5, 7}, {5, 7}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6}, {6, 6}, {5, 8},	/* 110 */
    {5, 8}, {5, 8}, {5, 8}, {5, 8}, {5, 8},
    {6, 7}, {6, 7}, {6, 7}, {6, 7}, {5, 9},	/* 120 */
    {5, 9}, {5, 9}, {5, 9}, {5, 9}, {7, 6},
    {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},	/* 130 */
    {6, 8}, {6, 8}, {6, 8}, {6, 8}, {7, 7},
    {7, 7}, {7, 7}, {7, 7}, {8, 6}, {8, 6},	/* 140 */
    {8, 6}, {8, 6}, {8, 6}, {7, 8}, {7, 8},
    {7, 8}, {7, 8}, {8, 7}, {8, 7}, {8, 8},	/* 150 */
    {8, 8}, {8, 8}, {8, 8}, {8, 8}, {8, 8},
    {20, 20}, {25, 25}, {25, 25}, {25, 25}, {50, 50},
    {50, 50}
};				/* monk_bare_hand */

const int weapon_dam_dice[][2] = {
    {0, 0},
    {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 3},
    {1, 3}, {2, 2}, {2, 2}, {2, 2}, {2, 2},	/* 10 */
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {2, 2}, {2, 2}, {2, 3}, {2, 3}, {2, 3},	/* 20 */
    {2, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {2, 3}, {2, 3}, {2, 3}, {2, 4}, {2, 4},	/* 30 */
    {2, 4}, {2, 4}, {2, 4}, {2, 4}, {2, 4},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 4},	/* 40 */
    {3, 4}, {3, 4}, {3, 4}, {3, 4}, {3, 4},
    {4, 4}, {4, 4}, {4, 4}, {4, 4}, {4, 4},	/* 50 */
    {5, 3}, {5, 3}, {5, 3}, {5, 3}, {5, 3},
    {4, 5}, {4, 5}, {4, 5}, {4, 5}, {4, 5},	/* 60 */
    {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4},
    {4, 6}, {4, 6}, {4, 6}, {4, 6}, {4, 6},	/* 70 */
    {5, 5}, {5, 5}, {5, 5}, {5, 5}, {5, 5},
    {5, 5}, {5, 5}, {5, 5}, {5, 5}, {4, 7},	/* 80 */
    {4, 7}, {4, 7}, {4, 7}, {5, 6}, {5, 6},
    {5, 6}, {5, 6}, {5, 6}, {5, 6}, {5, 6},	/* 90 */
    {5, 6}, {5, 6}, {5, 6}, {5, 6}, {5, 6},
    {4, 8}, {4, 8}, {4, 8}, {4, 8}, {5, 7},	/* 100 */
    {5, 7}, {5, 7}, {5, 7}, {6, 6}, {6, 6},
    {6, 6}, {6, 6}, {6, 6}, {6, 6}, {5, 8},	/* 110 */
    {5, 8}, {5, 8}, {5, 8}, {5, 8}, {5, 8},
    {6, 7}, {6, 7}, {6, 7}, {6, 7}, {5, 9},	/* 120 */
    {5, 9}, {5, 9}, {5, 9}, {5, 9}, {7, 6},
    {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},	/* 130 */
    {6, 8}, {6, 8}, {6, 8}, {6, 8}, {7, 7},
    {7, 7}, {7, 7}, {7, 7}, {8, 6}, {8, 6},	/* 140 */
    {8, 6}, {8, 6}, {8, 6}, {7, 8}, {7, 8},
    {7, 8}, {7, 8}, {8, 7}, {8, 7}, {8, 8},	/* 150 */
    {8, 9}, {8, 9}, {9, 9}, {9,10}, {10, 10},
    {20, 20}, {25, 25}, {25, 25}, {25, 25}, {50, 50},
    {50, 50}
};				/* weapon dam dice */

/* The Fight related routines */

void appear(struct char_data *ch)
{
    if (affected_by_spell(ch, SPELL_INVISIBLE, ABT_SPELL))
	affect_from_char(ch, SPELL_INVISIBLE, ABT_SPELL);

    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INCOGNITO);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_SNEAK);
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_VAMP_AURA);

    if (GET_LEVEL(ch) < LVL_IMMORT)
	act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
    else
	act
	    ("You feel a strange presence as $n appears, seemingly from nowhere.",
	     FALSE, ch, 0, 0, TO_ROOM);
}

void load_messages(struct message_list *msg_list, char *msg_file)
{
    FILE *fl;
    int i, type;
    struct message_type *message;
    char chk[128];

    if (!(fl = fopen(msg_file, "r"))) {
	sprintf(buf2, "SYSERR: Error reading message file %s", msg_file);
	perror(buf2);
	exit(1);
    }

    for (i = 0; i < MAX_MESSAGES; i++) {
	msg_list[i].a_type = 0;
	msg_list[i].number_of_attacks = 0;
	msg_list[i].msg = 0;
    }

    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
	fgets(chk, 128, fl);

    while (*chk == 'M') {
	fgets(chk, 128, fl);
	sscanf(chk, " %d\n", &type);
	for (i = 0; (i < MAX_MESSAGES) && (msg_list[i].a_type != type) &&
	     (msg_list[i].a_type); i++);
	if (i >= MAX_MESSAGES) {
	    fprintf(stderr, "SYSERR: Too many messages.  Increase "
		    "MAX_MESSAGES and recompile.");
	    exit(1);
	}
	CREATE(message, struct message_type, 1);
	msg_list[i].number_of_attacks++;
	msg_list[i].a_type = type;
	message->next = msg_list[i].msg;
	msg_list[i].msg = message;

	message->die_msg.attacker_msg = fread_action(fl, i);
	message->die_msg.victim_msg = fread_action(fl, i);
	message->die_msg.room_msg = fread_action(fl, i);
	message->miss_msg.attacker_msg = fread_action(fl, i);
	message->miss_msg.victim_msg = fread_action(fl, i);
	message->miss_msg.room_msg = fread_action(fl, i);
	message->hit_msg.attacker_msg = fread_action(fl, i);
	message->hit_msg.victim_msg = fread_action(fl, i);
	message->hit_msg.room_msg = fread_action(fl, i);
	message->god_msg.attacker_msg = fread_action(fl, i);
	message->god_msg.victim_msg = fread_action(fl, i);
	message->god_msg.room_msg = fread_action(fl, i);

	fgets(chk, 128, fl);
	while (!feof(fl) && (*chk == '\n' || *chk == '*'))
	    fgets(chk, 128, fl);
    }

    fclose(fl);
}


void update_pos(struct char_data *victim)
{
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
	return;
    else if (GET_HIT(victim) > 0)
	GET_POS(victim) = POS_STANDING;
    else if (GET_HIT(victim) <= -11)
	GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -6)
	GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -3)
	GET_POS(victim) = POS_INCAP;
    else
	GET_POS(victim) = POS_STUNNED;
}


void check_killer(struct char_data *ch, struct char_data *vict)
{
    if (!PLR_FLAGGED(vict, PLR_KILLER) && !PLR_FLAGGED(vict, PLR_THIEF)
	&& !PLR_FLAGGED(ch, PLR_KILLER) && !IS_NPC(ch) && !IS_NPC(vict) &&
	!ROOM_FLAGGED(ch->in_room, ROOM_ARENA) && (ch != vict)) {
	char buf[256];

/*
    SET_BIT_AR(PLR_FLAGS(ch), PLR_KILLER); */
	sprintf(buf,
		"PC Killer bit set on %s for initiating attack on %s at %s.",
		GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
	mudlog(buf, BRF, LVL_IMMORT, TRUE);
	send_to_char("If you want to be a PLAYER KILLER, so be it...\r\n",
		     ch);
    }
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{

    char msg[512];

    if (ch == vict)
	return;

    assert(!FIGHTING(ch));

    ch->next_fighting = combat_list;
    combat_list = ch;

    if (IS_AFFECTED(ch, AFF_SLEEP))
	affect_from_char(ch, SPELL_SLEEP, ABT_SPELL);

    FIGHTING(ch) = vict;
    GET_POS(ch) = POS_FIGHTING;

/*
  if (!pk_allowed)
    check_killer(ch, vict);
*/

    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) &&
	ROOM_FLAGGED(vict->in_room, ROOM_ARENA)) {
	sprintf(msg, "%s has engaged %s!\r\n", GET_NAME(ch),
		GET_NAME(vict));
	send_to_arena(msg);
    }
    if (IS_MOB(vict) && MOB_FLAGGED(vict, MOB_HUNTER))
	HUNTING(vict) = ch;
    if (GET_CLAN(ch) && !IS_NPC(vict) && GET_CLAN(vict)) {
	if (clan[find_clan_by_id(GET_CLAN(ch))].at_war == GET_CLAN(vict)
	    && clan[find_clan_by_id(GET_CLAN(vict))].at_war ==
	    GET_CLAN(ch)) {
	    clan[find_clan_by_id(GET_CLAN(ch))].war_timer = 30;
	}

    }
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
    struct char_data *temp;

    if (ch == next_combat_list)
	next_combat_list = ch->next_fighting;

    REMOVE_FROM_LIST(ch, combat_list, next_fighting);
    ch->next_fighting = NULL;
    FIGHTING(ch) = NULL;
    GET_POS(ch) = POS_STANDING;
    update_pos(ch);
}



void make_corpse(struct char_data *ch, struct char_data *killer)
{
    struct obj_data *corpse, *o, *next_obj;
    struct obj_data *money;
    int i, x, y;
    extern int max_npc_corpse_time, max_pc_corpse_time;

    struct obj_data *create_money(int amount);

    corpse = create_obj();

    corpse->item_number = NOTHING;
    corpse->in_room = NOWHERE;
    sprintf(buf2, "corpse %s", GET_NAME(ch));
    corpse->name = str_dup(buf2);

    sprintf(buf2, "The corpse of %s is lying here.", GET_NAME(ch));
    corpse->description = str_dup(buf2);

    sprintf(buf2, "the corpse of %s", GET_NAME(ch));
    corpse->short_description = str_dup(buf2);

    GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;
    for (x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
	if (x < EF_ARRAY_MAX)
	    GET_OBJ_EXTRA_AR(corpse, x) = 0;
	if (y < TW_ARRAY_MAX)
	    corpse->obj_flags.wear_flags[y] = 0;
    }
    SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
    SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
    GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
    if (!IS_NPC(ch))
	GET_OBJ_VAL(corpse, 2) = -2;	/* -2 means player corpse */
    if (GET_CLASS(ch) == CLASS_VAMPIRE) {
	GET_OBJ_VAL(corpse, 3) = 2;	/* vamp corpse identifier */
    } else
	GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */

    if (IS_NPC(ch)) {
	GET_OBJ_EXP(corpse) = (GET_LEVEL(ch) * 2);
	corpse->obj_flags.pid = 0;
	SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NPC_CORPSE);
	GET_OBJ_VROOM(corpse) = NOWHERE;
    } else {
	GET_OBJ_EXP(corpse) = -1;
	corpse->obj_flags.pid = GET_IDNUM(ch);
	SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_PC_CORPSE);
	GET_OBJ_VROOM(corpse) = world[ch->in_room].number;
    }

    GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
    GET_OBJ_RENT(corpse) = 100000;
    if (IS_NPC(ch))
	GET_OBJ_TIMER(corpse) = max_npc_corpse_time;
    else {
	GET_OBJ_TIMER(corpse) = max_pc_corpse_time;
	GET_PKLOOTER(corpse) = killer;
    }

    // Corpse Saving
    SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_UNIQUE_SAVE);



    if (!PLR_FLAGGED(ch, PLR_INSURED)) {

	/* transfer character's inventory to the corpse */
	corpse->contains = ch->carrying;
	for (o = corpse->contains; o != NULL; o = o->next_content)
	    o->in_obj = corpse;
	object_list_new_owner(corpse, NULL);

	/* transfer character's equipment to the corpse */
	for (i = 0; i < NUM_WEARS; i++)
	    if (GET_EQ(ch, i)) {
		remove_otrigger(GET_EQ(ch, i), ch);
		obj_to_obj(unequip_char(ch, i), corpse);
	    }


	/* transfer gold */
	if (GET_GOLD(ch) > 0) {
	    /* following 'if' clause added to fix gold duplication loophole */
	    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
		money = create_money(number(1, (int) GET_GOLD(ch)));
		money->obj_flags.player = -1;
		obj_to_obj(money, corpse);
		SET_BIT_AR(GET_OBJ_EXTRA(money), ITEM_UNIQUE_SAVE);

	    }
	    GET_GOLD(ch) = 0;
	}
	ch->carrying = NULL;
	IS_CARRYING_N(ch) = 0;
	IS_CARRYING_W(ch) = 0;
    }


    /* grab any !trans eq from the corpse and place it back in player inv */
    if (!IS_NPC(ch)) {
	for (o = corpse->contains; o != NULL; o = next_obj) {
	    next_obj = o->next_content;
	    if (IS_OBJ_STAT(o, ITEM_NOTRANSFER)) {
		obj_from_obj(o);
		obj_to_char(o, ch);
	    }
	}
    }

    obj_to_room(corpse, ch->in_room);
    save_corpses();
}


/* When ch kills victim */
/* Ok, redoing this to suit Dibrova better and to remove a few bugs. */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
	// Coder: Raiker - Don't change (Testperiod).
	int a, b, c;
	int a2, b2;

	a = GET_ALIGNMENT(ch);
	b = GET_ALIGNMENT(victim);
	a2 = a;
	b2 = b;

	if (a < 0)
		a2 += ((-a)*2);
	if (b < 0)
		b2 += ((-b)*2);

	c = (((a2+b2)/2)/100);

	if (b <= -300 || b >= 300)
	{
		if (a < 0 && b < 0)
			GET_ALIGNMENT(ch) += c;
		else if (a < 0 && b > 0)
			GET_ALIGNMENT(ch) -= c;
		else if (a > 0 && b > 0)
			GET_ALIGNMENT(ch) -= c;
		else if (a > 0 && b < 0)
			GET_ALIGNMENT(ch) += c;
		if (GET_ALIGNMENT(ch) >= 1000)
			GET_ALIGNMENT(ch) = 1000;
		if (GET_ALIGNMENT(ch) <= -1000)
			GET_ALIGNMENT(ch) = -1000;
	}
// Testing done and it works good, so I removed debug output to shadowmaster and eldorian.
//  Old code below:
//	if (GET_ALIGNMENT(victim) <= -300 || GET_ALIGNMENT(victim) >= 300)
//		GET_ALIGNMENT(ch) += ((-GET_ALIGNMENT(victim) - GET_ALIGNMENT(ch)) / 100);
}

void pc_death_return(struct char_data *ch, struct char_data *killer)
{
    room_rnum r_mortal_start_room = real_room(3001);

    char_from_room(ch);

    if (!PLR_FLAGGED(ch, PLR_FROZEN)) {
	if (GET_LEVEL(ch) >= LVL_IMMORT)
	    char_to_room(ch, r_immort_start_room);
	else
	    char_to_room(ch, r_mortal_start_room);
    } else {
	char_to_room(ch, r_frozen_start_room);
    }


    if (GET_PKSAFETIMER(ch) <= 0) {
	if (!IS_NPC(killer))
	    GET_PKSAFETIMER(ch) = 20;
	else
	    GET_PKSAFETIMER(ch) = 10;
    }

    GET_POS(ch) = POS_RESTING;
    GET_HIT(ch) = 1;
    GET_MANA(ch) = 1;
    GET_VIM(ch) = 1;
    GET_QI(ch) = 1;
    GET_ARIA(ch) = 1;
    act("$n arrives from a boiling black cloud.", FALSE, ch, 0, 0,
	TO_ROOM);
    look_at_room(ch, 1);

    if ((GET_PKSAFETIMER(ch) > 0) && PLR_FLAGGED(ch, PLR_PKILL_OK)) {
	sprintf(buf,
		"You may not engage in pkilling for the next %d ticks\r\n",
		GET_PKSAFETIMER(ch));
	send_to_char(buf, ch);
	sprintf(buf, "PK TIMEOUT:: %d ticks for %s\r\n",
		GET_PKSAFETIMER(ch), GET_NAME(ch));
	send_to_pkillers(buf);
    }

}


/* Killing same mobs XP gain limit function */

/* You can change formulas below if you wish */

#define GET_PERCENT(num, from_v) ((int)((float)(from_v) / 100 * (num)))

/* This formula is responsible for very slow increase of XP penalties   */
#define KILL_UNDER_MF_FORMULA(exp, num_of_kills)                        \
        (MAX(1, GET_PERCENT((101 - num_of_kills), exp)                  \
            )                                                           \
        )

/* This formula is responsible for fast increase of XP penalties        */
#define KILL_ABOVE_MF_FORMULA(exp, num_of_kills)                        \
        (MAX(1, GET_PERCENT((float)100/((float)num_of_kills/2+1), exp)  \
            )                                                           \
        )

int kills_limit_xpgain(struct char_data *ch, struct char_data *victim,
		       int exp)
{
    byte current_entry;
    int victim_vnum = GET_MOB_VNUM(victim);
    int new_exp;
    int i;

    /* Find corresponding to victim's vnum entry in array or create one */

    i = GET_KILLS_CURPOS(ch);
    while (i >= 1 && GET_KILLS_VNUM(ch, i) != victim_vnum)
	--i;
    if (!i) {
	if (!GET_KILLS_VNUM(ch, GET_KILLS_CURPOS(ch))) {
	    /* Array still isn't full - don't scan second part */
	    i = GET_KILLS_CURPOS(ch);
	} else {
	    /* Array is full - must scan second part too */
	    i = 100;
	    while (i > GET_KILLS_CURPOS(ch)
		   && GET_KILLS_VNUM(ch, i) != victim_vnum)
		--i;
	}
	if (i == GET_KILLS_CURPOS(ch)) {
	    /* We came back to search starting point: */
	    /* New type of mob killed, add it to list */
	    GET_KILLS_VNUM(ch, i) = victim_vnum;
	    GET_KILLS_AMOUNT(ch, i) = 0;
	    if (GET_KILLS_CURPOS(ch) < 100)
		++GET_KILLS_CURPOS(ch);
	    else
		GET_KILLS_CURPOS(ch) = 1;
	}
    }
    current_entry = i;

    /* Ok, now we have current_entry, pointing to current mob's entry in array */
    /* Lets increment number of kills and reduce XP gain */

//    if (GET_KILLS_AMOUNT(ch, current_entry) < 255)
    ++GET_KILLS_AMOUNT(ch, current_entry);

	/*
    if (GET_KILLS_AMOUNT(ch, current_entry) > GET_MOB_MAXFACTOR(victim)) {
	new_exp = KILL_UNDER_MF_FORMULA(exp, GET_MOB_MAXFACTOR(victim));
	new_exp = KILL_ABOVE_MF_FORMULA(new_exp,
					(GET_KILLS_AMOUNT
					 (ch,
					  current_entry) -
					 GET_MOB_MAXFACTOR(victim)));
    } else if (GET_KILLS_AMOUNT(ch, current_entry) > 1)
	new_exp =
	    KILL_UNDER_MF_FORMULA(exp,
				  GET_KILLS_AMOUNT(ch, current_entry));
    else*/
	new_exp = exp;		/* MOB was killed for first time, just give the EXP */

    return new_exp;
}


void death_cry(struct char_data *ch)
{
    int door, was_in;

    act("Your blood freezes as you hear $n's death cry.", FALSE, ch, 0, 0,
	TO_ROOM);
    was_in = ch->in_room;

    for (door = 0; door < NUM_OF_DIRS; door++) {
	if (CAN_GO(ch, door)) {
	    ch->in_room = world[was_in].dir_option[door]->to_room;
	    act("Your blood freezes as you hear someone's death cry.",
		FALSE, ch, 0, 0, TO_ROOM);
	    ch->in_room = was_in;
	}
    }
}



void raw_kill(struct char_data *ch, struct char_data *killer)
{
    if (FIGHTING(ch))
	stop_fighting(ch);

    while (ch->affected)
	affect_remove(ch, ch->affected);

    if (killer) {
	if (death_mtrigger(ch, killer))
	    death_cry(ch);
    } else
	death_cry(ch);

    make_corpse(ch, killer);

/*
  if (IS_NPC(killer)) {
    GET_KILLVICTIM(killer) = ch;
    GET_KILLVICTIMER(killer) = 15;
  }
*/
    if (IS_NPC(ch))
	extract_char(ch);
    else
	pc_death_return(ch, killer);

}

void increase_blood(int rm)
{
  RM_BLOOD(rm) = MIN(RM_BLOOD(rm) + 1, 10);
}


void die(struct char_data *ch, struct char_data *killer)
{
    int exp_jump;

    increase_blood(killer->in_room);

    if (!IS_NPC(ch) && ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
	arena_kill(ch);
    else if (!IS_NPC(ch) && GET_CLASS(ch) == CLASS_SAINT &&
	     IS_AFFECTED(ch, AFF_AFTERLIFE)) saint_kill(ch);
    else {
	exp_jump = level_exp(GET_CLASS(ch), GET_LEVEL(ch) + 1) -
	    level_exp(GET_CLASS(ch), GET_LEVEL(ch));
	if (GET_LEVEL(ch) > newbie_level)
	    exp_jump >>= 2;
	else
	    exp_jump >>= 3;
	if (GET_EXP(ch) > exp_jump)
	    gain_exp(ch, -exp_jump);	/* take out 1/4 of their exp */
	else
	    gain_exp(ch, -(GET_EXP(ch) - 1));	/* down to 1 */

	GET_NUM_DEATHS(ch)++;
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
	REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);

	/* If he was riding, free up the mount. */
	if (GET_MOUNT(ch))
	    GET_RIDER(GET_MOUNT(ch)) = NULL;
	/* If he was a mount, free up the rider. */
	if (GET_RIDER(ch))
	    GET_MOUNT(GET_RIDER(ch)) = NULL;

	raw_kill(ch, killer);
    }

}

void perform_group_gain(struct char_data *ch, int base,
			struct char_data *victim)
{
  int share = 0;
  int exp_after_lim = 0;

  if (!IS_NPC(ch) && IS_NPC(victim))
    exp_after_lim = kills_limit_xpgain(ch, victim, base);

  if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
    return;

  share = MIN(max_exp_gain, MAX(1, exp_after_lim));

  share = MAX(share, 1) + 14;

  if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
    share = MAX(share, 1) * .65;
  } else if (GET_CLASS(ch) >= 9) {
    share = MAX(share, 1) * .75;
  } else
    share = MAX(share, 1) + 14;

  /* Calculate level-difference bonus */
  if (IS_NPC(ch))
    share += MAX(0, (share * MIN(4,(GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
  else {
    if (((GET_LEVEL(ch) - GET_LEVEL(victim)) >= 20 &&
          GET_CLASS(ch) < CLASS_VAMPIRE) ||
         (GET_CLASS(ch) >= CLASS_VAMPIRE && GET_LEVEL(victim) <= 139)) {
      send_to_char("Your level is too high to garner any experience "
                   "from this mob.\r\n", ch);
      share = 0;
    } else
      share += MAX(0,(share*MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
      if (GET_LEVEL(ch) < 51 && GET_CLASS(ch) < CLASS_VAMPIRE)
        share *= 1;
      else if (GET_LEVEL(ch) < 101 && GET_CLASS(ch) < CLASS_VAMPIRE)
        share *= .8;
      else
        share *= .6;
    }

    if (share > 1) {
      sprintf(buf2, "You receive your share of experience -- %d points.\r\n",
              share);
      send_to_char(buf2, ch);
      if (double_exp >= 1) {
        sprintf(buf2, "You receive your share of bonus experience -- "
                      "%d points.\r\n", share);
        send_to_char(buf2, ch);
      }
    } else {
      send_to_char("You receive your share of experience -- one measly "
                   "little point!\r\n", ch);
      if (double_exp >= 1) {
        send_to_char("You receive bonus experience -- one measly little "
                     "point!\r\n", ch);
      }
    }

    gain_exp(ch, share);
    change_alignment(ch, victim);
}


void group_gain(struct char_data *ch, struct char_data *victim)
{
    int tot_members, base, tot_gain;
    struct char_data *k;
    struct follow_type *f;

    if (!(k = ch->master))
	k = ch;

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
	tot_members = 1;
    } else
	tot_members = 0;

    for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	    f->follower->in_room == ch->in_room) tot_members++;

    /* round up to the next highest tot_members */
    tot_gain = (int) (GET_EXP(victim) / 4);

    /* prevent illegal xp creation when killing players */
    if (!IS_NPC(victim))
	tot_gain = MIN(max_exp_loss * 2 / 3, tot_gain);

    if (tot_members >= 2)
	base = ((tot_gain / tot_members) * 1.25);
    else
	base = tot_gain;

    if (IS_AFFECTED(k, AFF_GROUP) && k->in_room == ch->in_room)
	perform_group_gain(k, base, victim);

    for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	    f->follower->in_room == ch->in_room)
		perform_group_gain(f->follower, base, victim);
}


void solo_gain(struct char_data *ch, struct char_data *victim)
{
    int exp = 0;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
	return;

    if (!IS_NPC(ch) && IS_NPC(victim))
	exp = kills_limit_xpgain(ch, victim, exp);


    if (GET_CLASS(ch) >= CLASS_VAMPIRE) {
		exp = MIN(max_exp_gain, (GET_EXP(victim) / 4) * .65);
    } else if (GET_CLASS(ch) >= 9) {
	exp = MIN(max_exp_gain, (GET_EXP(victim) / 4) * .75);
    } else
	exp = MIN(max_exp_gain, GET_EXP(victim) / 4);

    /* Calculate level-difference bonus */
    if (IS_NPC(ch))
	exp += MAX(0, (exp * MIN(4, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
    else {
      if (((GET_LEVEL(ch) - GET_LEVEL(victim)) >= 20 && 
            GET_CLASS(ch) < CLASS_VAMPIRE) ||
           (GET_CLASS(ch) >= CLASS_VAMPIRE && GET_LEVEL(victim) <= 139)) {
        send_to_char("Your level is too high to garner any experience "
                     "from this mob.\r\n", ch);
        exp = 0;
      } else
        exp += MAX(0, (exp * MIN(8, (GET_LEVEL(victim) - GET_LEVEL(ch)))) / 8);
	if (GET_LEVEL(ch) < 51 && GET_CLASS(ch) < CLASS_VAMPIRE)
	    exp *= 1;
	else if (GET_LEVEL(ch) < 101 && GET_CLASS(ch) < CLASS_VAMPIRE)
	    exp *= .8;
	else
	    exp *= .6;
    }

    exp = MAX(exp, 1) + 4;

    if (exp > 1) {
	sprintf(buf2, "You receive %d experience points.\r\n", exp);
	send_to_char(buf2, ch);
	if (double_exp >= 1) {
	    sprintf(buf2, "You receive %d bonus experience points.\r\n",
		    exp);
	    send_to_char(buf2, ch);
	}
    } else {
      send_to_char("You receive one lousy experience point.\r\n", ch);
      if (double_exp >= 1) {
        send_to_char("You receive one lousy bonus experience point.\r\n", ch);
      }
    }

    gain_exp(ch, exp);
    change_alignment(ch, victim);
}


char *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
    static char buf[256];
    char *cp;

    cp = buf;

    for (; *str; str++) {
	if (*str == '#') {
	    switch (*(++str)) {
	    case 'W':
		for (; *weapon_plural; *(cp++) = *(weapon_plural++));
		break;
	    case 'w':
		for (; *weapon_singular; *(cp++) = *(weapon_singular++));
		break;
	    default:
		*(cp++) = '#';
		break;
	    }
	} else
	    *(cp++) = *str;

	*cp = 0;
    }				/* For */

    return (buf);
}


/* message for doing damage with a weapon */
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type)
{
    char *buf;
    int msgnum;

    static struct dam_weapon_type {
	char *to_room;
	char *to_char;
	char *to_victim;
    } dam_weapons[] = {

	/* use #w for singular (i.e. "slash") and #W for plural (i.e. "slashes") */

	{
	    "$n tries to #w $N, but misses.",	/* 0     */
		"You try to #w $N, but miss.",
		"$n tries to #w you, but misses."}, {
	    "$n tickles $N as $e #W $M.",	/* 1  */
		"You tickle $N as you #w $M.",
		"$n tickles you as $e #W you."}, {
	    "$n barely #W $N.",	/* 2  */
	"You barely #w $N.", "$n barely #W you."}, {
	    "$n #W $N.",	/* 3 */
	"You #w $N.", "$n #W you."}, {
	    "$n #W $N hard.",	/* 4  */
	"You #w $N hard.", "$n #W you hard."}, {
	    "$n #W $N very hard.",	/* 5  */
	"You #w $N very hard.", "$n #W you very hard."}, {
	    "$n #W $N extremely hard.",	/* 6  */
	"You #w $N extremely hard.", "$n #W you extremely hard."}, {
	    "$n massacres $N to small fragments with $s #w.",	/* 7 */
		"You massacre $N to small fragments with your #w.",
		"$n massacres you to small fragments with $s #w."}, {
	    "$n /cWOBLITERATES/c0 $N with $s deadly #w!!/c0",	/* 8   */
		"You /cWOBLITERATE/cy $N with your deadly #w!!/c0",
		"$n/cW OBLITERATES/cr you with $s deadly #w!!/c0"}, {
	    "$n /cG**RAZES**/c0 $N with $s deadly #w!!",	/* 9 */
		"You /cG**RAZE**/cy $N with your deadly #w!!/c0",
		"$n /cG**RAZES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cM**WRECKS**/c0 $N with $s deadly #w!!",	/* 10 */
		"You /cM**WRECK**/cy $N with your deadly #w!!/c0",
		"$n /cM**WRECKS**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cC**WASTES**/c0 $N with $s deadly #w!!",	/* 11 */
		"You /cC**WASTE**/cy $N with your deadly #w!!/c0",
		"$n /cC**WASTES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cY**DESOLATES**/c0 $N with $s deadly #w!!",	/* 12 */
		"You /cY**DESOLATE**/cy $N with your deadly #w!!/c0",
		"$n /cY**DESOLATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cW**MAIMS**/c0 $N with $s deadly #w!!",	/* 13 */
		"You /cW**MAIM**/cy $N with your deadly #w!!/c0",
		"$n /cW**MAIMS**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cB**PULVERIZES**/c0 $N with $s deadly #w!!",	/* 14 */
		"You /cB**PULVERIZE**/cy $N with your deadly #w!!/c0",
		"$n /cB**PULVERIZES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cL**RUINS**/c0 $N with $s deadly #w!!",	/* 15 */
		"You /cL**RUIN**/cy $N with your deadly #w!!/c0",
		"$n /cL**RUINS**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cW**DEMOLISHES**/c0 $N with $s deadly #w!!/c0",	/* 16 */
		"You /cW**DEMOLISH**/cy $N with your deadly #w!!/c0",
		"$n /cW**DEMOLISHES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cR**MANGLES**/c0 $N with $s deadly #w!!",	/* 17 */
		"You /cR**MANGLE**/cy $N with your deadly #w!!/c0",
		"$n /cR**MANGLES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cM**ANNIHILATES**/c0 $N with $s deadly #w!!",	/* 18   */
		"You /cM**ANNIHILATE**/cy $N with your deadly #w!!/c0",
		"$n /cM**ANNIHILATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cY**DEVASTATES**/c0 $N with $s deadly #w!!",	/* 19   */
		"You /cY**DEVASTATE**/cy $N with your deadly #w!!/c0",
		"$n /cY**DEVASTATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cB**DECIMATES**/c0 $N with $s deadly #w!!",	/* 20 */
		"You /cB**DECIMATE**/cy $N with your deadly #w!!/c0",
		"$n /cB**DECIMATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cG**ERADICATES**/c0 $N with $s deadly #w!!",	/* 21 */
		"You /cG**ERADICATE**/cy $N with your deadly #w!!/c0",
		"$n /cG**ERADICATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cL**CREMATES**/c0 $N with $s deadly #w!!",	/* 22  */
		"You /cL**CREMATE**/cy $N with your deadly #w!!/c0",
		"$n /cL**CREMATES**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cR**BUTCHERS**/c0 $N with $s deadly #w!!",	/* 23 */
		"You /cR**BUTCHER**/cy $N with your deadly #w!!/c0",
		"$n /cR**BUTCHERS**/cr you with $s deadly #w!!/c0"}, {
	    "$n /cC**LIQUEFIES**/c0 $N with $s deadly #w!!/c0",	/* 24   */
		"You /cC**LIQUEFY**/cy $N with your deadly #w!!/c0",
		"$n /cC**LIQUEFIES**/cr you with $s deadly #w!!/c0"}

    };


    w_type -= TYPE_HIT;		/* Change to base of table with text */

    if (dam == 0)
	msgnum = 0;
    else if (dam <= 1)
	msgnum = 1;
    else if (dam <= 2)
	msgnum = 2;
    else if (dam <= 3)
	msgnum = 3;
    else if (dam <= 4)
	msgnum = 4;
    else if (dam <= 6)
	msgnum = 5;
    else if (dam <= 8)
	msgnum = 6;
    else if (dam <= 11)
	msgnum = 7;
    else if (dam <= 14)
	msgnum = 8;
    else if (dam <= 18)
	msgnum = 9;
    else if (dam <= 22)
	msgnum = 10;
    else if (dam <= 27)
	msgnum = 11;
    else if (dam <= 34)
	msgnum = 12;
    else if (dam <= 39)
	msgnum = 13;
    else if (dam <= 44)
	msgnum = 14;
    else if (dam <= 55)
	msgnum = 15;
    else if (dam <= 63)
	msgnum = 16;
    else if (dam <= 75)
	msgnum = 17;
    else if (dam <= 84)
	msgnum = 18;
    else if (dam <= 100)
	msgnum = 19;
    else if (dam <= 130)
	msgnum = 20;
    else if (dam <= 165)
	msgnum = 21;
    else if (dam <= 200)
	msgnum = 22;
    else if (dam <= 220)
	msgnum = 23;
    else
	msgnum = 24;

    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
			 attack_hit_text[w_type].singular,
			 attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

    /* damage message to damager */
    send_to_char(CCYEL(ch, C_CMP), ch);
    buf = replace_string(dam_weapons[msgnum].to_char,
			 attack_hit_text[w_type].singular,
			 attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, NULL, victim, TO_CHAR);
    send_to_char(CCNRM(ch, C_CMP), ch);

    /* damage message to damagee */
    send_to_char(CCRED(victim, C_CMP), victim);
    buf = replace_string(dam_weapons[msgnum].to_victim,
			 attack_hit_text[w_type].singular,
			 attack_hit_text[w_type].plural);
    act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
    send_to_char(CCNRM(victim, C_CMP), victim);
}


/*
 * message for doing damage with a spell or skill
 */
int skill_message(int dam, struct char_data *ch, struct char_data *vict,
		  int attacktype, int abil_type)
{
    int i, j, nr;
    struct message_type *msg;
    struct obj_data *weap = GET_EQ(ch, WEAR_WIELD);
    struct message_list *msg_list;

    switch (abil_type) {
    case ABT_WEAPON:
	msg_list = weapon_messages;
	break;
    case ABT_SKILL:
	msg_list = skill_messages;
	break;
    case ABT_SPELL:
	msg_list = spell_messages;
	break;
    case ABT_CHANT:
	msg_list = chant_messages;
	break;
    case ABT_PRAYER:
	msg_list = prayer_messages;
	break;
    case ABT_SONG:
	msg_list = song_messages;
	break;
    default:
	send_to_char("Something fucked up.\r\n", ch);
	return 0;
	break;
    }

    for (i = 0; i < MAX_MESSAGES; i++) {
	if (msg_list[i].a_type == attacktype) {
	    nr = dice(1, msg_list[i].number_of_attacks);
	    for (j = 1, msg = msg_list[i].msg; (j < nr) && msg; j++)
		msg = msg->next;
	    if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
		act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict,
		    TO_CHAR);
		act(msg->god_msg.victim_msg, FALSE, ch, weap, vict,
		    TO_VICT);
		act(msg->god_msg.room_msg, FALSE, ch, weap, vict,
		    TO_NOTVICT);
	    } else if (dam != 0) {
		if (GET_POS(vict) == POS_DEAD) {
		    send_to_char(CCYEL(ch, C_CMP), ch);
		    act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict,
			TO_CHAR);
		    send_to_char(CCNRM(ch, C_CMP), ch);
		    send_to_char(CCRED(vict, C_CMP), vict);
		    act(msg->die_msg.victim_msg, FALSE, ch, weap, vict,
			TO_VICT | TO_SLEEP);
		    send_to_char(CCNRM(vict, C_CMP), vict);
		    act(msg->die_msg.room_msg, FALSE, ch, weap, vict,
			TO_NOTVICT);
		} else {
		    send_to_char(CCYEL(ch, C_CMP), ch);
		    act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict,
			TO_CHAR);
		    send_to_char(CCNRM(ch, C_CMP), ch);
		    send_to_char(CCRED(vict, C_CMP), vict);
		    act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict,
			TO_VICT | TO_SLEEP);
		    send_to_char(CCNRM(vict, C_CMP), vict);
		    act(msg->hit_msg.room_msg, FALSE, ch, weap, vict,
			TO_NOTVICT);
		}
	    } else if (ch != vict) {	/* dam == 0 */
		send_to_char(CCYEL(ch, C_CMP), ch);
		act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict,
		    TO_CHAR);
		send_to_char(CCNRM(ch, C_CMP), ch);
		send_to_char(CCRED(vict, C_CMP), vict);
		act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict,
		    TO_VICT | TO_SLEEP);
		send_to_char(CCNRM(vict, C_CMP), vict);
		act(msg->miss_msg.room_msg, FALSE, ch, weap, vict,
		    TO_NOTVICT);
	    }
	    return 1;
	}
    }
    return 0;
}


void damage(struct char_data *ch, struct char_data *victim, int dam,
	    int attacktype, int type)
{

//  struct descriptor_data *k;
    char tmpstr[128];
    int clan_num;
    int had_coins = 0, clan_coins = 0, coins = 0;
	
	if (ch->in_room != victim->in_room)
		return;

    if (GET_POS(victim) <= POS_DEAD)
	{
		log("SYSERR: Attempt to damage a corpse.");
		die(victim, ch);
		return;			/* -je, 7/7/92 */
    }

    /* peaceful rooms */
/*  if (ch->nr != real_mobile(DG_CASTER_PROXY) && */
    if (ch != victim && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
	if (GET_CLASS(ch) == CLASS_VAMPIRE)
	    send_to_char
		("You stand upon sacred ground. No blood can be spilt here "
		 "today.\r\n", ch);
	else
	    send_to_char
		("This room just has such a peaceful, easy feeling...\r\n",
		 ch);
	return;
    }

    /* cone of cold players */
    if (AFF_FLAGGED(victim, AFF_COLD)) {
	send_to_char
	    ("They are encased in solid ice! You can't attack them!\r\n",
	     ch);
	return;
    }
    if (AFF_FLAGGED(victim, AFF_BURROW)) {
	send_to_char("You do not see that here.\r\n", ch);
	return;
    }
    /* MESMERIZED PLAYERS */
    if (AFF_FLAGGED(victim, AFF_MESMERIZED))
	REMOVE_BIT_AR(AFF_FLAGS(victim), AFF_MESMERIZED);

    /* Titans using absorb */
    if (AFF_FLAGGED(victim, AFF_ABSORB))
	REMOVE_BIT_AR(AFF_FLAGS(victim), AFF_ABSORB);

    /* shopkeeper protection */
    if (!ok_damage_shopkeeper(ch, victim)) {
	stop_fighting(ch);
	stop_fighting(victim);
	return;
    }

    if (dam > 0 && type == ABT_WEAPON && GET_EQ(ch, WEAR_WIELD) &&
	GET_EQ(ch, WEAR_WIELD)->obj_flags.special) {
	dam += GET_EQ(ch, WEAR_WIELD)->obj_flags.special >> 1;
	GET_EQ(ch, WEAR_WIELD)->obj_flags.special >>= 2;
    }

    /* You can't damage an immortal! */
    if (!IS_NPC(victim) && (GET_LEVEL(victim) >= LVL_IMMORT))
	dam = 0;

    /* Vampires only get damaged by ITEM_SILVER weapons */
    if (IS_NPC(victim) && GET_CLASS(victim) == CLASS_NPC_VAMPIRE &&
	GET_EQ(ch, WEAR_WIELD) &&
	!IS_OBJ_STAT(GET_EQ(ch, WEAR_WIELD), ITEM_SILVER)) {
	send_to_char
	    ("You can only hurt vampires with /ccsilver/c0 weapons.\r\n",
	     ch);
	send_to_char("Your weapon has no affect! Flee!\r\n", ch);
	dam = 0;
    }

    if (victim != ch) {
	/* Start the attacker fighting the victim */
	if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL))
	    set_fighting(ch, victim);

	/* Start the victim fighting the attacker */
	if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == NULL)) {
	    set_fighting(victim, ch);
	    if (MOB_FLAGGED(victim, MOB_MEMORY) && !IS_NPC(ch))
		remember(victim, ch);
	    if (MOB_FLAGGED(victim, MOB_VENOMOUS) && !IS_NPC(ch)) {
		send_to_char
		    ("/cr\r\nYou recoil as you are infected with your foe's deadly poison!/c0\r\n",
		     ch);
		call_magic(victim, FIGHTING(victim), 0, SPELL_POISON, ABT_SPELL,
			   250, CAST_SPELL, FALSE);
		call_magic(victim, FIGHTING(victim), NULL, SPELL_POISON, 
                           ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE);

	    }
	}
    }

    /* If you attack a pet, it hates your guts */
    if (victim->master == ch)
	stop_follower(victim);

    /* If the attacker is invisible, he becomes visible */
    if (IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_HIDE) ||
	IS_AFFECTED(ch, AFF_INCOGNITO))
	appear(ch);

    /* give some meaning to Prot-good and prot evil */
    if (AFF_FLAGGED(victim, AFF_PROTFROMGOOD) && (IS_GOOD(ch)))
	dam = (dam * 4 / 5);

    if (AFF_FLAGGED(victim, AFF_PROTECT_EVIL) && (IS_EVIL(ch)))
	dam = (dam * 4 / 5);

    if (AFF_FLAGGED(victim, AFF_PROTECT_EVIL) && (IS_EVIL(victim)))
	dam = (dam * 5 / 4);

    if (AFF_FLAGGED(victim, AFF_PROTFROMGOOD) && (IS_GOOD(victim)))
	dam = (dam * 5 / 4);

    if (IS_AFFECTED(victim, AFF_WEAKENEDFLESH))
	dam = ((float) dam * 1.25);

    if (IS_AFFECTED(victim, AFF_GAUGE))
        dam = ((float) dam * 1.15);

    if (IS_AFFECTED(victim, AFF_SANCTUARY) && dam >= 2)
	dam >>= 1;

    if (IS_AFFECTED(victim, AFF_SOFTEN_FLESH) && dam >= 4)	/* Crysist */
	dam *= 2;

    if (PLR_FLAGGED(victim, PLR_FISHING) && dam >= 4)
	dam = ((float) dam * 1.5);

    if (IS_AFFECTED(victim, AFF_ANCIENT_PROT) && dam >= 2)
	dam = (dam * 3) >> 2;

    if (IS_AFFECTED(victim, AFF_BLAZEWARD) && dam >= 4)
	dam = (dam * 2) >> 2;

    if (IS_AFFECTED(victim, AFF_INDESTR_AURA) && dam >= 8)
	dam >>= 2;

    if (IS_AFFECTED(victim, AFF_VAMP_AURA) && dam >= 2)
	dam >>= 1;

    if (IS_AFFECTED(victim, AFF_NETHERBLAZE) && dam >= 2)
	dam >>= 1;

    if (IS_AFFECTED(victim, AFF_EARTH_SHROUD) && dam >= 2)
	dam >>= 1;

    if (IS_AFFECTED(victim, AFF_SACRED_SHIELD) && dam >= 4)
	dam = (dam * 3) >> 2;

    if (IS_AFFECTED(victim, AFF_DIVINE_NIMBUS) && dam >= 4)
	dam = (dam * 3) >> 2;

    if (IS_AFFECTED(victim, AFF_DARKWARD) && dam >= 4)
	dam = (dam * 3) >> 2;

    if (IS_AFFECTED(victim, AFF_DEATHCALL)
	&& GET_CLASS(ch) == CLASS_VAMPIRE && dam >= 4)
	dam *= 2;



    // Add bonuses for mounted people
    if (GET_MOUNT(ch) != NULL && !GET_MOUNT(victim) &&
	GET_SKILL(ch, SKILL_RIDE) > 1) {
	if (IS_PALADIN(ch) || IS_GLADIATOR(ch) || IS_DARK_KNIGHT(ch) ||
	    IS_ASSASSIN(ch) || IS_INQUISITOR(ch) || IS_WARLOCK(ch) ||
	    IS_CHAMPION(ch) || IS_REAPER(ch) || IS_COMBAT_MASTER(ch) ||
	    IS_CRUSADER(ch) || IS_BEASTMASTER(ch) || IS_MAGI(ch) ||
	    IS_DARKMAGE(ch) || IS_TEMPLAR(ch) || IS_SAMURAI(ch) ||
	    IS_FIANNA(ch) ||
		GET_CLASS(ch) == CLASS_MERCENARY) {
	    dam = dam * 1.1;
	}
    }

    if (GET_SKILL(ch, SKILL_CRITICAL_HIT) && number(1, 100) <= 5) {
	send_to_char("/cRYou score a critical hit!/cy\r\n", ch);
	dam = dam * 2;
    }

    /* Check for PK if this is not a PK MUD 
       if (!pk_allowed) {
       check_killer(ch, victim);
       if (PLR_FLAGGED(ch, PLR_KILLER) && (ch != victim))
       dam = 0;
       } */

    if (GET_POS(victim) == POS_FIGHTING)
	{
		if (type == ABT_WEAPON)
		{
			if (GET_SKILL(victim, SKILL_BLOCK))
			{
				if (number(1, 640) < GET_SKILL(victim,
					SKILL_BLOCK) + (2 * (GET_DEX(victim) - GET_DEX(ch))))
				{
					act("You block $N's vicious attack!", FALSE, victim, 0, ch, TO_CHAR);
					act("$n blocks your vicious attack!", FALSE, victim, 0, ch, TO_VICT);
					act("$n blocks $N's vicious attack!", FALSE, victim, 0, ch, TO_NOTVICT);
					return;
				}
			}

			if (GET_SKILL(victim, SKILL_DODGE))
			{
				if (number(1, 640) < GET_SKILL(victim, 
					SKILL_DODGE) + (2 * (GET_DEX(victim) - GET_DEX(ch))))
				{
					act("You dodge $N's vicious attack!", FALSE, victim, 0, ch, TO_CHAR);
					act("$n dodges your vicious attack!", FALSE, victim, 0, ch, TO_VICT);
					act("$n dodges $N's vicious attack!", FALSE, victim, 0, ch, TO_NOTVICT);
					return;
				}
			}
		}
    }

    /* Shadow Mobs */
    if ((IS_NPC(victim)) && (GET_RACE(victim) ==
			     RACE_NPC_SHADOW) && (number(0, 2))) {
	act("Your attack passes right through $n with no effect!",
	    FALSE, victim, 0, ch, TO_VICT);
	act("$N's attack passes right through $n with no effect!",
	    FALSE, victim, 0, ch, TO_NOTVICT);
	return;
    }

    /* Defensive - that's why victim & ch are switched. */
    if (IS_AFFECTED(victim, AFF_MENTAL_BARRIER)) {
	if (!number(0, 40 - GET_WIS(victim))) {
	    act
		("Your mental barrier transforms $N's negative energy into your "
		 "positive energy!", FALSE, victim, 0, ch, TO_CHAR);
	    act
		("$n's mental barrier transforms your negative energy into $s "
		 "positive energy!", FALSE, victim, 0, ch, TO_VICT);
	    act
		("$n's mental barrier transforms $N's negative energy into $s own, "
		 "positive energy!", FALSE, victim, 0, ch, TO_ROOM);
	    GET_HIT(victim) =
		MIN(GET_HIT(victim) + dam, GET_MAX_HIT(victim));
//        GET_HIT(victim) += (GET_HIT(victim) + number(35, 50));  
	    dam = 0;
	}
    }

    /* Set the maximum damage per round and subtract the hit points */
    if (type == ABT_WEAPON)
	dam = adjust_damage(ch, dam);

	if (GET_CLASS(ch) == CLASS_DEMON)
	{
		if (IS_NEUTRAL(ch))
			dam /= 4;
		else if (IS_GOOD(ch))
		{
			if (type != ABT_WEAPON)
				dam /= 10;
		}
	}

	if (GET_CLASS(ch) == CLASS_SAINT)
	{
		if (IS_NEUTRAL(ch))
			dam /= 2;
		else if (IS_EVIL(ch))
		{
			if (type != ABT_WEAPON)
				dam /= 10;
		}
	}

    if (dam)
	dam = MAX(MIN(dam, 500), 2);	/* Min and max damage ranges */

    if (!IS_NPC(ch) && !IS_NPC(victim))	/* If this is pkill cut dam in 1/4 */
	GET_HIT(victim) -= (dam / 4);
    else
	GET_HIT(victim) -= dam;

    /* Gain exp for the hit */
    if (dam && ch != victim)
	gain_exp(ch, MAX(1, dam * 3));

    update_pos(victim);

    /*
     * skill_message sends a message from the messages file in lib/misc.
     * dam_message just sends a generic "You hit $n extremely hard.".
     * skill_message is preferable to dam_message because it is more
     * descriptive.
     * 
     * If we are _not_ attacking with a weapon (i.e. a spell), always use
     * skill_message. If we are attacking with a weapon: If this is a miss or a
     * death blow, send a skill_message if one exists; if not, default to a
     * dam_message. Otherwise, always send a dam_message.
     */
    if (type != ABT_WEAPON) {
	skill_message(dam, ch, victim, attacktype, type);
    } else {
	if (GET_POS(victim) == POS_DEAD || dam == 0) {
	    if (!skill_message(dam, ch, victim, attacktype, type)) {
		if (!IS_NPC(victim) && !IS_NPC(ch)) {
		    dam_message((dam * 4), ch, victim, attacktype);
		} else
		    dam_message(dam, ch, victim, attacktype);
	    }
	} else {
	    if (!IS_NPC(victim) && !IS_NPC(ch)) {
		dam_message((dam), ch, victim, attacktype);
	    } else
		dam_message(dam, ch, victim, attacktype);
	}
    }

    /* Use send_to_char -- act() doesn't send message if you are DEAD. */
    switch (GET_POS(victim)) {
    case POS_MORTALLYW:
	act("$n is mortally wounded, and will die soon, if not aided.",
	    TRUE, victim, 0, 0, TO_ROOM);
	send_to_char
	    ("You are mortally wounded, and will die soon, if not aided.\r\n",
	     victim);
	break;
    case POS_INCAP:
	act("$n is incapacitated and will slowly die, if not aided.", TRUE,
	    victim, 0, 0, TO_ROOM);
	send_to_char
	    ("You are incapacitated an will slowly die, if not aided.\r\n",
	     victim);
	break;
    case POS_STUNNED:
	act("$n is stunned, but will probably regain consciousness again.",
	    TRUE, victim, 0, 0, TO_ROOM);
	send_to_char
	    ("You're stunned, but will probably regain consciousness again.\r\n",
	     victim);
	break;
    case POS_DEAD:
	act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
	send_to_char
	    ("Life is but a rest before the journey of death begins.\r\n"
	     "You are dead. Sorry...\r\n", victim);
	break;

    default:			/* >= POSITION SLEEPING */
	if (dam > (GET_MAX_HIT(victim) / 4))
	    act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

	if (GET_HIT(victim) < (GET_MAX_HIT(victim) / 4)) {
	    sprintf(buf2,
		    "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
		    CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
	    send_to_char(buf2, victim);
	    if (MOB_FLAGGED(victim, MOB_WIMPY) && (ch != victim) &&
		(GET_POS(victim) == POS_FIGHTING) && (number(1, 20) < 8)) {
              if (AFF_FLAGGED(victim, AFF_SNARE)) {
                act("You try to flee, but your feet are stuck to the ground!",
                     FALSE, victim, 0, 0, TO_CHAR);
                act("$n tries to flee, but $s feet are stuck to the ground!",
                     FALSE, victim, 0, 0, TO_ROOM);
              } else
		do_flee(victim, "", 0, SCMD_FLEE);
            }
	}
	if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && (victim != ch) &&
	    GET_HIT(victim) < GET_WIMP_LEV(victim)) {
	    send_to_char("You wimp out, and attempt to flee!\r\n", victim);
	    if (GET_SKILL(victim, SKILL_RETREAT) > number(1, 90))
		do_flee(victim, "", 0, SCMD_RETREAT);
	    else
		do_flee(victim, "", 0, SCMD_FLEE);
	}
	break;
    }

    /* Help out poor linkless people who are attacked */
    if (!IS_NPC(victim) && !(victim->desc)) {
	do_flee(victim, "", 0, SCMD_FLEE);
	if (!FIGHTING(victim)) {
	    act("$n is rescued by divine forces.", FALSE, victim, 0, 0,
		TO_ROOM);
	    GET_WAS_IN(victim) = victim->in_room;
	    char_from_room(victim);
	    char_to_room(victim, 0);
	}
    }

    /* stop someone from fighting if they're stunned or worse */
    if ((GET_POS(victim) <= POS_STUNNED) && (FIGHTING(victim) != NULL))
	stop_fighting(victim);
    if ((GET_HIT(victim) <= 10) && (GET_POS(victim) >= POS_STUNNED) &&
	(FIGHTING(victim) != NULL) && (IS_NPC(ch)) &&
	(GET_LUCK(victim) > number(15, 2000))) {

	act("$N walks away. It must be your lucky day!",
	    FALSE, victim, 0, ch, TO_CHAR);
	act("$N stops fighting and walks away. It must be $n's lucky day!",
	    FALSE, victim, 0, ch, TO_NOTVICT);
	stop_fighting(victim);
	stop_fighting(ch);
    }


    /* Uh oh.  Victim died. */
    if (GET_POS(victim) == POS_DEAD) {
	if ((ch != victim) && (IS_NPC(victim) || victim->desc)) {
	    /*    if(!IS_NPC(ch) && !IS_NPC(victim) {
	       send_to_char("You gain no experience for this kill.\r\n", ch); 
	       return;

	       } */
//      else { 
	    if (IS_AFFECTED(ch, AFF_GROUP))
		group_gain(ch, victim);
	    else
		solo_gain(ch, victim);
	    //     }
	    if (++GET_NUM_KILLS(ch) % 1000 == 0) {
		sprintf(tmpstr,
			"/cGCongrats to %s on %s %dth kill!/c0\r\n",
			GET_NAME(ch), HSHR(ch), (int) GET_NUM_KILLS(ch));
		GET_HIT(ch) = GET_MAX_HIT(ch);
		GET_MANA(ch) = GET_MAX_MANA(ch);
		GET_MOVE(ch) = GET_MAX_MOVE(ch);
		GET_QI(ch) = GET_MAX_QI(ch);
		GET_VIM(ch) = GET_MAX_VIM(ch);
		GET_ARIA(ch) = GET_MAX_ARIA(ch);

		if (!PLR_FLAGGED(ch, PLR_KEEPRANK)
		    && GET_RANK(ch) < RANK_DUKE) {
		    GET_RANK(ch)++;
		}
		send_to_all(tmpstr);
	    }
	}

	if (!IS_NPC(victim) && !IS_NPC(ch) &&
	    !ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
	    sprintf(buf2, "%s killed by %s at %s", GET_NAME(victim),
		    GET_NAME(ch), world[victim->in_room].name);
	    send_to_all(buf2);
	    mudlog(buf2, BRF, LVL_IMMORT, TRUE);
	    if (GET_CLAN(ch) != GET_CLAN(victim)) {
		clan_num = find_clan_by_id(GET_CLAN(ch));
		clan[clan_num].pkwin += 1;
		clan_num = find_clan_by_id(GET_CLAN(victim));
		clan[clan_num].pklose += 1;

	    }
	    if ((GET_RANK(ch) > 12)
		&& (!ROOM_FLAGGED(ch->in_room, ROOM_ARENA))
		&& (!ROOM_FLAGGED(victim->in_room, ROOM_ARENA))
		&& !PLR_FLAGGED(victim, PLR_KEEPRANK)
		&& !PLR_FLAGGED(ch, PLR_KEEPRANK)) {
		GET_RANK(ch)++;
		sprintf(buf, "You achieve the rank of %s.\r\n",
			rank[(int) GET_RANK(ch)][(int) GET_SEX(ch)]);
		send_to_char(buf, ch);
		sprintf(buf,
			"\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n",
			GET_NAME(ch),
			rank[(int) GET_RANK(ch)][(int) GET_SEX(ch)]);
		send_to_all(buf);
	    }
	    if (PLR_FLAGGED(victim, PLR_BOUNTY)) {
		REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_BOUNTY);
		GET_GOLD(ch) = GET_GOLD(ch) + 500000;
		sprintf(buf,
			"\r\n/cRBOUNTY:: The bounty on %s has been collected "
			"by %s!/c0\r\n", GET_NAME(victim), GET_NAME(ch));
		send_to_all(buf);
	    }

	    if (ROOM_FLAGGED(ch->in_room, ROOM_ARENA) &&
		ROOM_FLAGGED(victim->in_room, ROOM_ARENA)) {
		sprintf(tmpstr, "%s has killed %s!\r\n",
			GET_NAME(ch), GET_NAME(victim));
		send_to_arena(tmpstr);
	    }
	    if (MOB_FLAGGED(ch, MOB_MEMORY)) {
		MEMORY(ch) = NULL;
		clearMemory(ch);
		forget(ch, victim);
		forget(victim, ch);
		MEMORY(victim) = NULL;
		clearMemory(victim);
	    }
	    if (MOB_FLAGGED(ch, MOB_HUNTER))
		HUNTING(ch) = 0;
	}

	if (!IS_NPC(ch) && !IS_NPC(victim)
	    && GET_RANK(victim) > GET_RANK(ch) && GET_RANK(victim) != 0
	    && !PLR_FLAGGED(ch, PLR_KEEPRANK)
	    && !PLR_FLAGGED(victim, PLR_KEEPRANK)
		&& !ROOM_FLAGGED(victim->in_room, ROOM_ARENA)
		&& !ROOM_FLAGGED(ch->in_room,  ROOM_ARENA)) {
		if (GET_RANK(ch) < GET_RANK(victim))
			GET_RANK(ch) = GET_RANK(victim);
		if (!PLR_FLAGGED(victim, PLR_GLAD_BET_QP) &&
				!PLR_FLAGGED(victim, PLR_GLAD_BET_GOLD))
				GET_RANK(victim) -= number(1, 2);
		if (!GET_RANK(victim))
				GET_RANK(victim) = 0;

	    send_to_char
		("/cRYou lose your rank! You hide your face in shame!/c0\r\n",
		 victim);
	    sprintf(buf, "You achieve the rank of %s.\r\n",
		    rank[(int) GET_RANK(ch)][(int) GET_SEX(ch)]);
	    send_to_char(buf, ch);
	    sprintf(buf,
		    "\r\n/cGGRATS:: %s has achieved the rank of %s!/c0\r\n",
		    GET_NAME(ch),
		    rank[(int) GET_RANK(ch)][(int) GET_SEX(ch)]);
	    send_to_all(buf);
	}
	    else if (IS_NPC(ch) && !IS_NPC(victim) && GET_RANK(victim) != 0
		     && !ROOM_FLAGGED(victim->in_room, ROOM_GRID)
		     && !PLR_FLAGGED(ch, PLR_KEEPRANK)
			 && !ROOM_FLAGGED(victim->in_room, ROOM_ARENA)
			 && !ROOM_FLAGGED(ch->in_room,  ROOM_ARENA)) {
			send_to_char
				("/cRYou lose your rank! You hide your face in shame!/c0\r\n",
				victim);
			if (!PLR_FLAGGED(victim, PLR_GLAD_BET_QP) &&
				!PLR_FLAGGED(victim, PLR_GLAD_BET_GOLD))
				GET_RANK(victim) -= number(1, 2);

			if (!GET_RANK(victim))
				GET_RANK(victim) = 0;
	}

        /* PIT KILLS */
        if (PLR_FLAGGED(ch, PLR_GLAD_BET_QP) && 
            GET_MOB_VNUM(victim) == pit_mob_vnum) {
          sprintf(buf, "\r\nYou receive %d quest points for your victory!\r\n"
                           "You may fight again in 90 minutes.\r\n",
                  glad_bet_amnt);
          send_to_char(buf, ch);
          sprintf(buf, "\r\n/cR%s is victorious in defeating %s in the Pit!/c0\r\n\r\n", GET_NAME(ch), GET_NAME(victim));
          send_to_all(buf);
          GET_QPOINTS(ch) += glad_bet_amnt;
          GET_PIT_TIMER(ch) = 90;
          glad_bet_amnt = 0;
          REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_QP);
          char_from_room(ch);
          char_to_room(ch, real_room(6836));
          look_at_room(ch, 0);
          GET_HIT(ch) = GET_MAX_HIT(ch);
          GET_MANA(ch) = GET_MAX_MANA(ch);
          make_corpse(victim, ch);
          extract_char(victim);
          pit_mob_vnum = 0;
          return;
        }
        if (PLR_FLAGGED(ch, PLR_GLAD_BET_GOLD) &&
            GET_MOB_VNUM(victim) == pit_mob_vnum) {
          sprintf(buf, "\r\nYou receive %d gold for your victory!\r\n"
                           "YOu may fight again in 90 minutes.\r\n",
                  glad_bet_amnt);
          send_to_char(buf, ch);
          sprintf(buf, "\r\n/cR%s is victorious in defeating %s in the Pit!/c0\r\n\r\n", GET_NAME(ch), GET_NAME(victim));
          send_to_all(buf);
          GET_GOLD(ch) += glad_bet_amnt;
          GET_PIT_TIMER(ch) = 90;
          glad_bet_amnt = 0;
          REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_GLAD_BET_GOLD);
          char_from_room(ch);
          char_to_room(ch, real_room(6836));
          look_at_room(ch, 0);
          GET_HIT(ch) = GET_MAX_HIT(ch);
          GET_MANA(ch) = GET_MAX_MANA(ch);
          make_corpse(victim, ch);
          extract_char(victim);
          pit_mob_vnum = 0;
          return;
        }
        if (PLR_FLAGGED(victim, PLR_GLAD_BET_QP)) {
          sprintf(buf, "\r\nYou lose %d quest points for your death!\r\n"
                           "You may fight again in 90 minutes.\r\n",
                  glad_bet_amnt);
          send_to_char(buf, victim);
          sprintf(buf, "\r\n/cR%s has been defeated in the Pit!/c0\r\n",
                  GET_NAME(victim));
          send_to_all(buf);
          GET_QPOINTS(victim) -= glad_bet_amnt;
          GET_PIT_TIMER(victim) = 90;
          glad_bet_amnt = 0;
          REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_GLAD_BET_QP);
          GET_HIT(victim) = GET_MAX_HIT(victim);
          GET_MANA(victim) = GET_MAX_MANA(victim);
          GET_POS(victim) = POS_STANDING;
          char_from_room(victim);
          char_to_room(victim, real_room(6836));
          look_at_room(victim, 0);
          extract_char(ch);
          pit_mob_vnum = 0;
          return;
        }
        if (PLR_FLAGGED(victim, PLR_GLAD_BET_GOLD)) {
          sprintf(buf, "\r\nYou lose %d gold for your death!\r\n"
                           "You may fight again in 90 minutes.\r\n",
                  glad_bet_amnt);
          send_to_char(buf, victim);
          sprintf(buf, "\r\n/cR%s has been defeated in the Pit!/c0\r\n",
                  GET_NAME(victim));
          send_to_all(buf);
          GET_GOLD(victim) -= glad_bet_amnt;
          GET_PIT_TIMER(victim) = 90;
          glad_bet_amnt = 0;
          REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_GLAD_BET_GOLD);
          GET_HIT(victim) = GET_MAX_HIT(victim);
          GET_MANA(victim) = GET_MAX_MANA(victim);
          GET_POS(victim) = POS_STANDING;
          char_from_room(victim);
          char_to_room(victim, real_room(6836));
          look_at_room(victim, 0);
          extract_char(ch);
          pit_mob_vnum = 0;
          return;
        }
        /* END PIT KILLS */

	if (IS_NPC(victim) && !IS_NPC(ch) && GET_QUESTMOB(ch)) {
	    if (GET_QUESTMOB(ch) == GET_MOB_VNUM(victim)) {
		send_to_char
		    ("/cw\r\nYou have almost completed your quest!\r\nHead back to your Quest Master to complete it fully!\r\n\r\n/c0",
		     ch);
		GET_QUESTMOB(ch) = 1;
	    }
	}

	if (IS_NPC(ch) && !IS_NPC(victim))
	{
		if (MOB_FLAGGED(ch, MOB_MEMORY))
		{
			forget(ch, victim);
	    }
		if (MOB_FLAGGED(ch, MOB_HUNTER))
			HUNTING(ch) = 0;
	}

	if (!IS_NPC(victim)) {
	    sprintf(buf, "DEATH: %s killed by %s.", GET_NAME(victim), GET_NAME(ch));
	    log(buf);
		// Below: Mobs will now forget who they are hunting when the vicim goes dead. R.I.P
		if (IS_NPC(ch))
			if (MOB_FLAGGED(ch, MOB_MEMORY))
				forget(ch, victim);
	}

	die(victim, ch);
        save_corpses();

	autosplit(ch);

	if (IS_NPC(victim) && !IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOLOOT)) {

	    had_coins = GET_GOLD(ch);
	    do_get(ch, "coins corpse", 0, 0);
	    coins = GET_GOLD(ch) - had_coins;
	    clan_num = find_clan_by_id(GET_CLAN(ch));
	    clan_coins = (coins * (clan[clan_num].tax * .01));
	    clan[clan_num].treasure += clan_coins;
	    GET_GOLD(ch) -= clan_coins;
	    coins -= clan_coins;
	    sprintf(buf, "%d", coins);
	    sprintf(buf2, "You lose %d coins to clan taxes.\r\n",
		    clan_coins);
	    send_to_char(buf2, ch);
	    do_get(ch, "all corpse", 0, 0);
	}
	/* autoloot */

	if (IS_NPC(victim) && !IS_NPC(ch) && PRF_FLAGGED(ch, PRF_AUTOSAC)) {
	    do_sac(ch, "corpse", 0, 0);
	}
    }
}



void hit(struct char_data *ch, struct char_data *victim, int type)
{
    struct obj_data *wielded = GET_EQ(ch, WEAR_WIELD);
    int w_type, victim_ac, calc_thaco, dam, diceroll;

    extern int thaco[NUM_CLASSES][LVL_IMPL + 1];
    extern struct str_app_type str_app[];
    extern struct dex_app_type dex_app[];

    int backstab_mult(int level);
    int swoop_mult(int level);

    /* check if the character has a fight trigger */
    fight_mtrigger(ch);

    /* Do some sanity checking, in case someone flees, etc. */
    if (ch->in_room != victim->in_room) {
	if (FIGHTING(ch) && FIGHTING(ch) == victim)
	    stop_fighting(ch);
	return;
    }

    if (type == HIT_DUAL)
	wielded = GET_EQ(ch, WEAR_HOLD);

    /* Find the weapon type (for display purposes only) */
    if (type != HIT_DUAL) {
	if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
	    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
	else {
	    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
		w_type = ch->mob_specials.attack_type + TYPE_HIT;
	    else
		w_type = TYPE_HIT;
	}
    } else {
	if (wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON)
	    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
	else {
	    if (IS_NPC(ch) && (ch->mob_specials.attack_type != 0))
		w_type = ch->mob_specials.attack_type + TYPE_HIT;
	    else
		w_type = TYPE_HIT;
	}
    }



    if (!IS_NPC(ch))
	calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
    else
	calc_thaco = thaco[CLASS_DRUID][MIN(150, (int) GET_LEVEL(ch))];
    calc_thaco -= (int) GET_HITROLL(ch);
    calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
    calc_thaco -= (GET_INT(ch) - 14);	/* Intelligence helps! */
    calc_thaco -= (GET_WIS(ch) - 14);	/* So does wisdom      */
    calc_thaco -= (GET_LUCK(ch) - 14);	/* So does luck        */
    if (GET_MOUNT(ch) != NULL && !GET_MOUNT(victim)
	&& GET_SKILL(ch, SKILL_RIDE) > 1) {
	if (IS_PALADIN(ch) || IS_GLADIATOR(ch) || IS_DARK_KNIGHT(ch) || GET_CLASS(ch) == CLASS_MERCENARY) {
	    calc_thaco -= 5;

	}
    }

    /* Calculate the raw armor including magic armor.  Lower AC is better. */
    victim_ac = GET_AC(victim);

    if (AWAKE(victim)) {
	victim_ac += dex_app[GET_DEX(victim)].defensive;
    }

    victim_ac = MAX(-100, victim_ac);	/* -100 is lowest */
    victim_ac += 100;

    /* roll the die and take your chances... */
    diceroll = number(1, 210);

    if (IS_NPC(ch) && GET_LEVEL(ch) < 19)
	diceroll += 20 - GET_LEVEL(ch);	/* Mobs get it a little harder... */

    /* decide whether this is a hit or a miss */
    if ((((diceroll < 200) && AWAKE(victim)) &&
	 ((diceroll < 10) || ((calc_thaco - diceroll) > victim_ac)))) {
	/* the attacker missed the victim */
	if (type == SKILL_BACKSTAB)
	    damage(ch, victim, 0, SKILL_BACKSTAB, ABT_SKILL);
	else if (type == SKILL_SWOOP)
	    damage(ch, victim, 0, SKILL_SWOOP, ABT_SKILL);
	else
	    damage(ch, victim, 0, w_type, ABT_WEAPON);
    } else {
	/* okay, we know the guy has been hit.  now calculate damage. */

	/* Start with the damage bonuses: the damroll and strength apply */
	dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
	if (!IS_NPC(ch))
	    dam += ((int) (GET_DAMROLL(ch) * 1.50) >> 1);
	else
	    dam += ((GET_DAMROLL(ch) * 3) >> 4);

	if (wielded) {
	    /* Add weapon-based damage if a weapon is being wielded */
		if (!IS_NPC(ch))
	    dam += (dice(weapon_dam_dice[wielded->obj_flags.minlevel][0], weapon_dam_dice[wielded->obj_flags.minlevel][1]));
		else if (IS_NPC(ch)) {
			dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
		}
	} else {
	    /* Mobs always use bare hand damage */
	    if (IS_NPC(ch)) {
			if (!wielded) {
				dam += dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice);
				dam *= ((float)0.75);
			}
		} else
		/* Monk barehand advantage */
		if ((GET_CLASS(ch) == CLASS_MONK ||
		     GET_CLASS(ch) == CLASS_COMBAT_MASTER ||
		     GET_CLASS(ch) == CLASS_ACROBAT ||
		     GET_CLASS(ch) == CLASS_PSIONIST ||
		     GET_CLASS(ch) == CLASS_STORR ||
		     GET_CLASS(ch) == CLASS_SAMURAI ||
		     GET_CLASS(ch) == CLASS_SHAMAN)
		    && (GET_LEVEL(ch) < LVL_IMMORT)) dam +=
		    (dice
		     (monk_bare_hand[GET_LEVEL(ch) + 1][0],
		      monk_bare_hand[GET_LEVEL(ch) + 1][1]));
	    else if (GET_CLASS(ch) == CLASS_VAMPIRE)
		dam +=
		    (dice(monk_bare_hand[151][0], monk_bare_hand[151][1]));
	    else
		dam += number(0, 2);	/* Max 2 bare hand damage for players */

	}

	/*
	 * Include a damage multiplier if victim isn't ready to fight:
	 *
	 * Position sitting  1.33 x normal
	 * Position resting  1.66 x normal
	 * Position sleeping 2.00 x normal
	 * Position stunned  2.33 x normal
	 * Position incap    2.66 x normal
	 * Position mortally 3.00 x normal
	 *
	 * Note, this is a hack because it depends on the particular
	 * values of the POSITION_XXX constants.
	 */
	if (GET_POS(victim) < POS_FIGHTING)
	    dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;

	/* at least 1 hp damage min per hit */
	dam = MAX(1, dam);

	/* Saint enlightenment gives them better damage */
	if (GET_CLASS(ch) == CLASS_SAINT && IS_AFFECTED(ch, AFF_ENLIGHT))
	    dam *= 1.25;

	/* Demons with a Netherclaw get 25% more damage */
	if (GET_CLASS(ch) == CLASS_DEMON
	    && IS_AFFECTED(ch, AFF_NETHERCLAW)) dam *= 1.25;

	if (type == SKILL_BACKSTAB) {
	    dam *= backstab_mult(GET_LEVEL(ch));
	    damage(ch, victim, dam, SKILL_BACKSTAB, ABT_SKILL);
	} else if (type == SKILL_SWOOP) {
	    dam *= swoop_mult(GET_LEVEL(ch));
	    damage(ch, victim, dam, SKILL_SWOOP, ABT_SKILL);
	} else {
	    damage(ch, victim, dam, w_type, ABT_WEAPON);
	}
    }

    /* check if the victim has a hitprcnt trigger */
    hitprcnt_mtrigger(victim);
}



/* control the fights going on.  Called every 2 seconds from comm.c. */
extern ACMD(do_assist);
void perform_violence(void)
{
    struct char_data *ch, *vict = NULL, *tch = NULL;
    extern struct index_data *mob_index;
    struct char_data *k;
    struct follow_type *f;
    int apr;
    int percent, prob;

    for (ch = combat_list; ch; ch = next_combat_list) {
	next_combat_list = ch->next_fighting;
	apr = 1;

	if (FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room) {
	    stop_fighting(ch);
	    continue;
	}

	if (IS_NPC(ch)) {
	    if (GET_MOB_WAIT(ch) > 0) {
		GET_MOB_WAIT(ch) -= PULSE_VIOLENCE;
		continue;
	    }
	    if (GET_SKILL_WAIT(ch) != 0) {
		GET_SKILL_WAIT(ch) -= PULSE_VIOLENCE;
	    }
	    if (GET_MOB_WAIT(ch) == 0) {

		if ((GET_POS(ch) < POS_FIGHTING)) {
		    if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2))
			act("$n quickly stands up.", 1, ch, 0, 0, TO_ROOM);
		    else if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 6))
			act("$n slowly stands up.", 1, ch, 0, 0, TO_ROOM);
		    else
			act("$n gets to $s feet very slowly.", 1, ch, 0, 0,
			    TO_ROOM);
		    GET_POS(ch) = POS_FIGHTING;
		}
	    }
	}
	/* If the attacker is invisible, he becomes visible */
	if (IS_AFFECTED(ch, AFF_INVISIBLE))
	    appear(ch);
	if (IS_AFFECTED(ch, AFF_HIDE))
	    appear(ch);
	if (IS_AFFECTED(ch, AFF_INCOGNITO))
	    appear(ch);

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
	    affect_from_char(ch, SPELL_SLEEP, ABT_SPELL);
	    affect_from_char(ch, SONG_LULLABY, ABT_SONG);
	}
	if (GET_POS(ch) < POS_FIGHTING) {
	    send_to_char("You can't fight while sitting!!\r\n", ch);
	    continue;
	}

	if (FIGHTING(ch)) {
	    monk_technique(ch);
	}


	if (IS_NPC(ch)) {
	    /* Give the poor mobs some extra attacks */
	    if (GET_LEVEL(ch) <= 13)
		apr = 1;
	    else if (GET_LEVEL(ch) <= 35)
		apr = 2;
	    else if (GET_LEVEL(ch) <= 80)
		apr = 3;
	    else if (GET_LEVEL(ch) <= 130)
		apr = 4;
	    else if (GET_LEVEL(ch) <= 200)
		apr = 5;
		else if (GET_LEVEL(ch) <= 250)
		apr = 6;
	} else {
	    if (GET_SKILL(ch, SKILL_SECOND_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_SECOND_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	    if (GET_SKILL(ch, SKILL_THIRD_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_THIRD_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	    if (GET_SKILL(ch, SKILL_FOURTH_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_FOURTH_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	    if (GET_SKILL(ch, SKILL_FIFTH_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_FIFTH_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	    if (GET_SKILL(ch, SKILL_SIXTH_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_SIXTH_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	    if (GET_SKILL(ch, SKILL_SEVENTH_ATTACK) >= number(1, 121)) {
		if (abil_info[ABT_SKILL][SKILL_SEVENTH_ATTACK].min_level
		    [(int) GET_CLASS(ch)] > GET_LEVEL(ch)
		    && GET_CLASS(ch) < CLASS_VAMPIRE) {
		} else
		    apr++;
	    }
	}

	if (AFF_FLAGGED(ch, AFF_SLOW))
	    apr = apr - 1;

	apr = MAX(1, MIN(apr, 9));	/* Don't let things get too out of hand */

	if (AFF_FLAGGED(ch, AFF_HASTE))
	    apr++;

	if (IS_AFFECTED(ch, AFF_UNFURLED))
	    apr++;

	for (; apr > 0 && FIGHTING(ch); apr--)
	    hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
	if (MOB_FLAGGED(ch, MOB_SPEC)
	    && mob_index[GET_MOB_RNUM(ch)].func != NULL)
	    (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");

	/* If your weapon has a spell, this is when it will be used. */
	if (!IS_NPC(ch)) {
		if (FIGHTING(ch) && GET_EQ(ch, WEAR_WIELD)) {
			if (GET_OBJ_SPELL(GET_EQ(ch, WEAR_WIELD))) {
				if (GET_OBJ_SPELL_EXTRA(GET_EQ(ch, WEAR_WIELD)) > number(1, 101)) {
					call_magic(ch, FIGHTING(ch), NULL, GET_OBJ_SPELL(GET_EQ(ch, WEAR_WIELD)), ABT_SPELL, GET_EQ(ch,	WEAR_WIELD)->obj_flags.minlevel, TYPE_SPELL, FALSE);
				}
			}
		}
	}

	/* Dual Wield (MJ) */

	if (GET_SKILL(ch, SKILL_DUAL_WIELD) && FIGHTING(ch)) {
	    struct obj_data *holding = GET_EQ(ch, WEAR_HOLD);

	    if (holding && GET_OBJ_TYPE(holding) == ITEM_WEAPON) {
		percent = number(1, 101);
		prob = GET_SKILL(ch, SKILL_DUAL_WIELD);
		if (percent < prob) {

		    hit(ch, FIGHTING(ch), HIT_DUAL);

		    if (MOB_FLAGGED(ch, MOB_SPEC) &&
			mob_index[GET_MOB_RNUM(ch)].func != NULL)

			(mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
		}
	    }
	}

	/* Handle Fire Shield damage */
	if (FIGHTING(ch) && IS_AFFECTED(ch, AFF_FIRESHIELD)) {
	    if (!number(0, 25)) {

		if (GET_SKILL(FIGHTING(ch), SKILL_ABSORB_FIRE)) {
		    act("$n's fire shield flares up, but $N absorbs it!",
			FALSE, ch, 0, FIGHTING(ch), TO_NOTVICT);
		    act("$n's fire shield flares up, but you absorb it!",
			FALSE, ch, 0, FIGHTING(ch), TO_VICT);
		    act("Your fire shield flares up, but $N absorbs it!",
			FALSE, ch, 0, FIGHTING(ch), TO_CHAR);
		    GET_HIT(FIGHTING(ch)) += number(25, 50);

		} else {
		    act("$n's fire shield flares up and burns $N!", FALSE,
			ch, 0, FIGHTING(ch), TO_ROOM);
		    act("$n's fire shield flares up and burns you!", FALSE,
			ch, 0, FIGHTING(ch), TO_VICT);
		    act("Your fire shield flares up and burns $N!", FALSE,
			ch, 0, FIGHTING(ch), TO_CHAR);
		    call_magic(ch, FIGHTING(ch), NULL, SPELL_FIREBALL,
			       ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL,
			       FALSE);
		}
	    }
	}

	/* Handle Mental Barrier */
	if (FIGHTING(ch) && IS_AFFECTED(ch, AFF_MENTAL_BARRIER)) {
	    if (!number(0, 50 - GET_WIS(ch))) {
		act
		    ("Your mental barrier transforms $N's negative energy into your "
		     "positive energy!", FALSE, ch, 0, FIGHTING(ch),
		     TO_CHAR);
		act
		    ("$n's mental barrier transforms your negative energy into $s "
		     "positive energy!", FALSE, ch, 0, FIGHTING(ch),
		     TO_VICT);
		act
		    ("$n'smental barrier transforms $N's negative energy into $s own, "
		     "positive energy!", FALSE, ch, 0, FIGHTING(ch),
		     TO_ROOM);
	    }
	}

	/* NPC Giants are Biotches to fight.... */


	if (IS_NPC(ch) && GET_POS(ch) <= POS_STANDING) {
	    if (GET_SKILL_WAIT(ch) == 0 && FIGHTING(ch)) {
		advanced_mobile_combat(ch, FIGHTING(ch));

		switch (GET_CLASS(ch)) {

		case CLASS_NPC_MAGE:
		    if (number(0, 3)) {
			break;
		    }

		    switch (number((GET_LEVEL(ch) >> 1), GET_LEVEL(ch))) {
		    case 0:;
		    case 1:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_MAGIC_MISSILE, ABT_SPELL, FALSE);
			break;
		    case 2:;
		    case 3:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_CHILL_TOUCH, ABT_SPELL, FALSE);
			break;
		    case 4:;
		    case 5:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_BURNING_HANDS, ABT_SPELL, FALSE);
			break;
		    case 6:;
		    case 7:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_SHOCKING_GRASP, ABT_SPELL, FALSE);
			break;
		    case 8:;
		    case 9:
		    case 10:;
		    case 11:
		    case 12:;
		    case 13:;
		    case 14:;
		    case 15:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_LIGHTNING_BOLT, ABT_SPELL, FALSE);
			break;
		    case 16:;
		    case 17:;
		    case 18:;
		    case 19:;
		    case 20:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_COLOR_SPRAY, ABT_SPELL, FALSE);
			break;
		    case 21:
		    case 22:
		    case 23:;
		    case 24:;
		    case 25:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_ENERGY_DRAIN, ABT_SPELL, FALSE);
			break;
		    case 26:;
		    case 27:;
		    case 28:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_FIREBALL,
				   ABT_SPELL, FALSE);
			break;
		    case 29:;
		    case 30:
		    case 31:;
		    case 32:;
		    case 33:;
		    case 34:;
		    case 35:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_SONIC_WALL, ABT_SPELL, FALSE);
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
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_FIRESTORM,
				   ABT_SPELL, FALSE);
			break;
		    case 53:
		    case 54:
		    case 55:
		    case 58:
		    case 61:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_STONE_HAIL, ABT_SPELL, FALSE);
			break;

		    case 62:
		    case 64:
		    case 66:
		    case 68:
		    case 71:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_FLYING_FIST, ABT_SPELL, FALSE);
			break;

		    case 72:
		    case 74:
		    case 76:
		    case 78:
		    case 81:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_SHOCK_SPHERE, ABT_SPELL, FALSE);
			break;

		    case 82:
		    case 84:
		    case 86:
		    case 88:
		    case 91:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_CAUSTIC_RAIN, ABT_SPELL, FALSE);
			break;

		    case 92:
		    case 94:
		    case 96:
		    case 98:
		    case 101:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_WITHER,
				   ABT_SPELL, FALSE);
			break;

		    case 102:
		    case 104:
		    case 106:
		    case 108:
		    case 110:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_METEOR_SWARM, ABT_SPELL, FALSE);
			break;

		    case 112:
		    case 114:
		    case 116:
		    case 118:
		    case 121:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_BALEFIRE,
				   ABT_SPELL, FALSE);
			break;

		    case 122:
		    case 124:
		    case 126:
		    case 128:
		    case 131:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_SCORCH,
				   ABT_SPELL, FALSE);
			break;

		    case 132:
		    case 134:
		    case 136:
		    case 138:
		    case 141:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_IMMOLATE,
				   ABT_SPELL, FALSE);
			break;
		    case 150:
			cast_spell(ch, FIGHTING(ch), NULL, SPELL_IMMOLATE,
				   ABT_SPELL, FALSE);
			break;

		    default:
			cast_spell(ch, FIGHTING(ch), NULL,
				   SPELL_COLOR_SPRAY, ABT_SPELL, FALSE);
			break;
		    }
		    break;

		default:
		    break;
		}
	    }
	}			// End of NPC Classes  

	if (IS_NPC(ch) && GET_INT(ch) > 15 && !number(0, 5)) {
	    vict = NULL;
	    for (tch = world[ch->in_room].people; tch;
		 tch = tch->next_in_room) if (FIGHTING(tch) == ch
					      && !IS_NPC(tch))
		    if (vict == NULL || GET_HIT(tch) < GET_HIT(vict))
			vict = tch;

	    if (vict != NULL && FIGHTING(ch) != vict) {
		stop_fighting(ch);
		act("$n wises up and attacks $N", FALSE, ch, 0, vict,
		    TO_ROOM);
		hit(ch, vict, TYPE_UNDEFINED);
	    }
	}
	if (IS_AFFECTED(ch, AFF_GROUP)) {
	    k = (ch->master ? ch->master : ch);	/* get the leader */
	    if (IS_AFFECTED(k, AFF_GROUP)
		&& FIGHTING(ch) && !FIGHTING(k)
		&& IN_ROOM(ch) == IN_ROOM(k)
		&& PRF_FLAGGED(k, PRF_AUTOASSIST))
		do_assist(k, GET_NAME(ch), 0, 0);
	    for (f = k->followers; f; f = f->next) {
		if (IS_AFFECTED(f->follower, AFF_GROUP)
		    && FIGHTING(ch) && !FIGHTING(f->follower)
		    && IN_ROOM(ch) == IN_ROOM(f->follower)
		    && PRF_FLAGGED(f->follower, PRF_AUTOASSIST))
		    do_assist(f->follower, GET_NAME(ch), 0, 0);
	    }
	}
	if (FIGHTING(ch) && IS_SET_AR(PRF_FLAGS(ch), PRF_AUTODIAG)) {
	    send_to_char("\r\n/cY", ch);
	    diag_char_to_char(FIGHTING(ch), ch);
	    send_to_char("/c0", ch);
	}
    }
}

/* Perform autosplit */
void autosplit(struct char_data *character)
{

    struct char_data *k;	/* Leader. */
    struct follow_type *f;	/* Follower. */

    int clan_coins = 0, clan_num;
    int had_coins = 0;
    int coins = 0;
    char buf[40];

    /* I killed it.  I get first crack at it. */
    if (PRF_FLAGGED(character, PRF_AUTOSPLIT)) {
	had_coins = GET_GOLD(character);
	do_get(character, "coins corpse", 0, 0);
	coins = GET_GOLD(character) - had_coins;
	clan_num = find_clan_by_id(GET_CLAN(character));
	clan_coins = (coins * (clan[clan_num].tax * .01));
	clan[clan_num].treasure += clan_coins;
	GET_GOLD(character) -= clan_coins;
	coins -= clan_coins;
	sprintf(buf, "%d", coins);
	sprintf(buf2, "You lose %d coins to clan taxes.\r\n", clan_coins);
	send_to_char(buf2, character);
    }
    /*  If grouped, then split it.  Otherwise, just put in pocket... */
    if (IS_AFFECTED(character, AFF_GROUP) && coins > 0)
	do_split(character, buf, 0, 0);
    return;

    /* Autogold - happens AFTER autosplit check */
    if (PRF_FLAGGED(character, PRF_AUTOGOLD) &&
	!PRF_FLAGGED(character, PRF_AUTOSPLIT)) {

	had_coins = GET_GOLD(character);
	do_get(character, "coins corpse", 0, 0);
	coins = GET_GOLD(character) - had_coins;
	clan_num = find_clan_by_id(GET_CLAN(character));
	clan_coins = (coins * (clan[clan_num].tax * .01));
	clan[clan_num].treasure += clan_coins;
	GET_GOLD(character) -= clan_coins;
	coins -= clan_coins;
	sprintf(buf, "%d", coins);
        if(GET_CLAN_RANK(character) && GET_CLAN(character) && clan_coins) {
	  sprintf(buf2, "You lose %d coins to clan taxes.\r\n", clan_coins);
          send_to_char(buf2, character);
        }
	return;
    }

    if (character->master)
	k = character->master;
    else
	k = character;

    /* k is the head of the group.  May or may not be the killer. */

    if (!k->followers)
	return;

    for (f = k->followers; f; f = f->next) {
	if (IS_AFFECTED(f->follower, AFF_GROUP)
	    && PRF_FLAGGED(f->follower, PRF_AUTOSPLIT)
	    && IN_ROOM(character) == IN_ROOM(f->follower)) {
	    had_coins = GET_GOLD(f->follower);
	    do_get(f->follower, "coins corpse", 0, 0);
	    coins = GET_GOLD(f->follower) - had_coins;	/* You got the difference.) */
	    if (coins > 0)
		sprintf(buf, "%d", coins);
	    do_split(f->follower, buf, 0, 0);
	    return;
	}
    }
}

void update_blood(void)
{
  int i;
  extern int top_of_world;

  for (i = 0; i < top_of_world; i++)
    if (RM_BLOOD(i) > 0)
      RM_BLOOD(i) -= 1;
}

