/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "house.h"
#include "screen.h"
#include "olc.h"
#include "arena.h"
#include "hometowns.h"
#include "spec_names.h"
#include "teleport.h"
#include "clan.h"
#include "dg_scripts.h"
#include "diskio.h"
#include "pfdefaults.h"

/*   external vars  */
extern int DFLT_PORT;
extern FILE *player_fl;
extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct teleport_shop *teleport_shops;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];
extern struct zone_data *zone_table;
extern struct char_data *mob_proto;
extern int top_of_zone_table;
extern int restrict;
extern int top_of_world;
extern int top_of_mobt;
extern int top_of_objt;
extern int top_of_p_table;
extern int double_exp;
extern char *class_abbrevs[];
extern char *skills[];
extern char *spells[];
extern char *chants[];
extern char *prayers[];
extern char *songs[];
extern char *prac_types[];
extern char *tactions[];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern char *connected_types[];
extern int tip_count;
extern int load_rooms[];
extern char *hometowns[];
extern struct shop_data *shop_index;
extern int spell_sort_info[MAX_ABILITIES+1];
extern const int weapon_dam_dice[][2];
extern int zedit_save_to_disk(int zone);
extern int medit_save_to_disk(int zone);
extern int redit_save_to_disk(int zone);
extern int oedit_save_to_disk(int zone);
extern int sedit_save_to_disk(int zone);
void vnum_illegal(struct char_data * ch);
int special_loading(byte type);


/* for objects */
extern char *item_types[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *container_bits[];
extern char *drinks[];

/* for rooms */
extern char *dirs[];
extern char *room_bits[];
extern char *exit_bits[];
extern char *sector_types[];

/* for chars */
extern char *spells[];
extern char *equipment_types[];
extern char *affected_bits[];
extern char *affected_bits2[];
extern char *apply_types[];
extern char *pc_class_types[];
extern char *pc_race_types[];
extern char *npc_class_types[];
extern char *action_bits[];
extern char *player_bits[];
extern char *preference_bits[];
extern char *preference_bits2[];
extern char *position_types[];
extern char *connected_types[];
extern char *npc_race_types[];

/* extern functions */
int level_exp(int class, int level);
char *get_role_text(int level);
int real_shop(int vshop_num);
struct char_data *find_char(int n);
int find_eq_pos(struct char_data * ch, struct obj_data * obj, char *arg);
void Read_Invalid_List(void);

void make_fingerlist_html(void);
void make_facelist_html(void);
void make_eyelist_html(void);
void make_headlist_html(void);
void make_earlist_html(void);
void make_necklist_html(void);
void make_holdlist_html(void);
void make_armlist_html(void);
void make_bodylist_html(void);
void make_aboutlist_html(void);
void make_floatlist_html(void);
void make_handlist_html(void);
void make_thumblist_html(void);
void make_waistlist_html(void);
void make_leglist_html(void);
void make_wieldlist_html(void);
void make_shieldlist_html(void);
void make_feetlist_html(void);
void make_wristlist_html(void);
void save_corpses(void);

void reimb(char *arg);

int vnum_weapon(int attacktype, struct char_data *ch);
int find_attack_type(char *arg);


#define FILE_XNAME	"misc/xnames"

ACMD(do_xname)
{
  FILE *fl;
  char *filename = FILE_XNAME;
  if (!*argument)
  {
	  send_to_char("How about actually adding a name and not a blank line?\r\n", ch);
	  return;
  }
  else if (!(fl = fopen(filename, "a")))
  {
	  send_to_char("Could not open the file.  Sorry.\r\n", ch);
	  return;
  }
  else {
	  skip_spaces(&argument);
	  delete_doubledollar(argument);
	  fprintf(fl, "%s\n", argument);
	  fclose(fl);
	  Read_Invalid_List();
	  send_to_char("You have added another illegal name.\r\n", ch);
	  return;
  }
}


ACMD(do_echo)
{
  skip_spaces(&argument);

  if (!*argument)
    send_to_char("Yes.. but what?\r\n", ch);
  else {
    if (subcmd == SCMD_EMOTE) {
      if (ROOM_FLAGGED(ch->in_room, ROOM_COURT)) {
        send_to_char("Not in a courtroom.\r\n", ch);
        return;
      } else
      sprintf(buf, "$n %s", argument);
    } else
      strcpy(buf, argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      act(buf, FALSE, ch, 0, 0, TO_CHAR);
  }
}

// This function is done, except that I
// need to add some more stats that the players
// of dibrova needs in their playerfiles. Other than
// that, the command is done. Works without problems, converts
// all pfiles. No argument is given, it automatically
// converts all binary pfiles into ascii ones.
// This one currently saves to /eldorian/pfiles since
// I, Eldorian, do not have write access to /lib.
#define PLR_INDEX_FILE2		"../eldorian/pfiles/plr_index"
ACMD(do_pconvert)
{
  FILE *pfile;
  int pnum=0;
  struct char_file_u vbuf;
  struct char_file_u player;
  struct char_special_data_saved *csds;
  struct player_special_data_saved *psds;
  struct char_point_data *cpd;
  struct affected_type *aff;
  struct char_stat_data *cad;
  FBFILE *outfile, *index_file = 0;
  char outname[40], bits[127];
  char index_buf[100000]; // Hey, how am I suppose to know how many letters 847 players becomes :P
  int i;

  *index_buf = '\0';

  send_to_char("Opening binary playerfile...\r\n", ch);
  if (!(pfile = fopen(PLAYER_FILE, "r+b"))) {
    return;
  }
  send_to_char("Opening plr_index...\r\n", ch);
  if (!(index_file = fbopen(PLR_INDEX_FILE, FB_WRITE))) {
    send_to_char("Error opening index file...\r\n", ch);
   return;
  }

  send_to_char("Looping players in binary file, writing them to ascii-files and closing them...\r\n", ch);
  fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
  fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
for(pnum=1;!feof(pfile);pnum++) {
  for(i = 0;
      (*(bits + i) = LOWER(*(vbuf.name + i))); i++);
    sprintf(outname, "../eldorian/pfiles%s%c%s%s", SLASH, *bits, SLASH, bits);
    printf("writing: %s\n", outname);

	sprintf(index_buf + strlen(index_buf), "%d %s %d 0 %s",
		(int)vbuf.char_specials_saved.idnum, bits, vbuf.level,
		ctime(&vbuf.last_logon));

	/******************************************************
	 * Lots of places to crash the game here, be carefull *
	 ******************************************************/

    if (!(outfile = fbopen(outname, FB_WRITE))) {
      printf("error opening output file");
      exit(1);
	}
	if(vbuf.name)
      fbprintf(outfile, "Name: %s\n", vbuf.name);
    if(vbuf.pwd)
      fbprintf(outfile, "Pass: %s\n", vbuf.pwd);
    if(vbuf.title)
      fbprintf(outfile, "Titl: %s\n", vbuf.title);
    if(vbuf.description && *vbuf.description)
      fbprintf(outfile, "Desc:\n%s~\n", vbuf.description);
    if(player.sex != PFDEF_SEX)
      fbprintf(outfile, "Sex : %d\n", (int)vbuf.sex); 
    if(vbuf.class != PFDEF_CLASS)
      fbprintf(outfile, "Clas: %d\n", (int)vbuf.class); 
	if(vbuf.race != PFDEF_RACE)
      fbprintf(outfile, "Race: %d\n", (int)vbuf.race); 
    if(vbuf.level != PFDEF_LEVEL)
      fbprintf(outfile, "Levl: %d\n", (int)vbuf.level); 
    if(vbuf.hometown != PFDEF_HOMETOWN)
      fbprintf(outfile, "Home: %d\n", (int)vbuf.hometown); 
    fbprintf(outfile, "Brth: %d\n", (int)vbuf.birth);
    fbprintf(outfile, "Plyd: %d\n", (int)vbuf.played);
    fbprintf(outfile, "Last: %d\n", (int)vbuf.last_logon);
    fbprintf(outfile, "Host: %s\n", vbuf.host);
    if(vbuf.height != PFDEF_HEIGHT)
      fbprintf(outfile, "Hite: %d\n", (int)vbuf.height);
    if(vbuf.weight != PFDEF_WEIGHT)
      fbprintf(outfile, "Wate: %d\n", (int)vbuf.weight);

	/*
	 * I could have saved all skill, chants, etc, etc
	 * into one Abil: list, but I liked it better this way.
	 */
	/* Skills, chants, spells, prayers and songs. */
	if(player.level < LVL_IMMORT) {
      fbprintf(outfile, "Skil:\n");
		for (i=0; i < MAX_ABILITIES; i++) {
			if (vbuf.abilities[i].type == ABT_SKILL) {
				if(vbuf.abilities[i].pct)
					fbprintf(outfile, "%d %d\n", i, (int)vbuf.abilities[i].pct);
			}
		}
		fbprintf(outfile, "0 0\n");
	  }
	if(player.level < LVL_IMMORT) {
      fbprintf(outfile, "Spel:\n");
		for (i=0; i < MAX_ABILITIES; i++) {
			if (vbuf.abilities[i].type == ABT_SPELL) {
				if(vbuf.abilities[i].pct)
					fbprintf(outfile, "%d %d\n", i, (int)vbuf.abilities[i].pct);
			}
		}
		fbprintf(outfile, "0 0\n");
	  }
	if(player.level < LVL_IMMORT) {
      fbprintf(outfile, "Pray:\n");
		for (i=0; i < MAX_ABILITIES; i++) {
			if (vbuf.abilities[i].type == ABT_PRAYER) {
				if(vbuf.abilities[i].pct)
					fbprintf(outfile, "%d %d\n", i, (int)vbuf.abilities[i].pct);
			}
		}
		fbprintf(outfile, "0 0\n");
	  }
	if(player.level < LVL_IMMORT) {
      fbprintf(outfile, "Chan:\n");
		for (i=0; i < MAX_ABILITIES; i++) {
			if (vbuf.abilities[i].type == ABT_CHANT) {
				if(vbuf.abilities[i].pct)
					fbprintf(outfile, "%d %d\n", i, (int)vbuf.abilities[i].pct);
			}
		}
		fbprintf(outfile, "0 0\n");
	  }
	if(player.level < LVL_IMMORT) {
      fbprintf(outfile, "Song:\n");
		for (i=0; i < MAX_ABILITIES; i++) {
			if (vbuf.abilities[i].type == ABT_SONG) {
				if(vbuf.abilities[i].pct)
					fbprintf(outfile, "%d %d\n", i, (int)vbuf.abilities[i].pct);
			}
		}
		fbprintf(outfile, "0 0\n");
	  }
	/* char_special_data_saved */
    csds = &(vbuf.char_specials_saved);
    if(csds->alignment != PFDEF_ALIGNMENT)
      fbprintf(outfile, "Alin: %d\n", csds->alignment);
    fbprintf(outfile, "Id  : %d\n", (int)csds->idnum);
    if(csds->act != PFDEF_PLRFLAGS)
      fbprintf(outfile, "Act : %d\n", (int)csds->act);
	sprintbitarray(csds->act, player_bits, PM_ARRAY_MAX, buf2);
    sprintf(buf, "Plrf: %s\r\n", buf2);
    if(csds->apply_saving_throw[0] != PFDEF_SAVETHROW)
      fbprintf(outfile, "Thr1: %d\n", csds->apply_saving_throw[0]);
    if(csds->apply_saving_throw[1] != PFDEF_SAVETHROW)
      fbprintf(outfile, "Thr2: %d\n", csds->apply_saving_throw[1]);
    if(csds->apply_saving_throw[2] != PFDEF_SAVETHROW)
      fbprintf(outfile, "Thr3: %d\n", csds->apply_saving_throw[2]);
    if(csds->apply_saving_throw[3] != PFDEF_SAVETHROW)
      fbprintf(outfile, "Thr4: %d\n", csds->apply_saving_throw[3]);
    if(csds->apply_saving_throw[4] != PFDEF_SAVETHROW)
      fbprintf(outfile, "Thr5: %d\n", csds->apply_saving_throw[4]);

/* player_special_data_saved */
    psds = &(vbuf.player_specials_saved);
    if(psds->wimp_level != PFDEF_WIMPLEV)
      fbprintf(outfile, "Wimp: %d\n", psds->wimp_level);
    if(psds->freeze_level != PFDEF_FREEZELEV)
      fbprintf(outfile, "Frez: %d\n", (int)psds->freeze_level);
    if(psds->invis_level != PFDEF_INVISLEV)
      fbprintf(outfile, "Invs: %d\n", (int)psds->invis_level);
    if(psds->load_room != PFDEF_LOADROOM)
      fbprintf(outfile, "Room: %d\n", (int)psds->load_room);
    sprintbitarray(psds->pref, preference_bits, PR_ARRAY_MAX, buf2);
    sprintf(buf, "Prff: %s\r\n", buf2);
    if(psds->conditions[FULL] && player.level < LVL_IMMORT &&
	psds->conditions[FULL] != PFDEF_HUNGER)
      fbprintf(outfile, "Hung: %d\n", (int)psds->conditions[0]);
    if(psds->conditions[THIRST] && player.level < LVL_IMMORT &&
	psds->conditions[THIRST] != PFDEF_THIRST)
      fbprintf(outfile, "Thir: %d\n", (int)psds->conditions[1]);
    if(psds->conditions[2] && player.level < LVL_IMMORT &&
	psds->conditions[DRUNK] != PFDEF_DRUNK)
      fbprintf(outfile, "Drnk: %d\n", (int)psds->conditions[2]);
    if(psds->spells_to_learn != PFDEF_PRACTICES)
      fbprintf(outfile, "Lern: %d\n", (int)psds->spells_to_learn);

    /* char_ability_data */
    cad = &(vbuf.stats);
    if(cad->str != PFDEF_STR || cad->str_add != PFDEF_STRADD)
      fbprintf(outfile, "Str : %d/%d\n", cad->str, cad->str_add);
    if(cad->intel != PFDEF_INT)
      fbprintf(outfile, "Int : %d\n", cad->intel);
    if(cad->wis != PFDEF_WIS)
      fbprintf(outfile, "Wis : %d\n", cad->wis);
    if(cad->dex != PFDEF_DEX)
      fbprintf(outfile, "Dex : %d\n", cad->dex);
    if(cad->con != PFDEF_CON)
      fbprintf(outfile, "Con : %d\n", cad->con);
    if(cad->cha != PFDEF_CHA)
      fbprintf(outfile, "Cha : %d\n", cad->cha);
	if(cad->cha != PFDEF_LUCK)
      fbprintf(outfile, "Luck: %d\n", cad->luck);

	// Player Points data
	cpd = &(vbuf.points);
    if(cpd->hit != PFDEF_HIT || cpd->max_hit != PFDEF_MAXHIT)
      fbprintf(outfile, "Hitp: %d/%d\n", cpd->hit, cpd->max_hit);
    if(cpd->mana != PFDEF_MANA || cpd->max_mana != PFDEF_MAXMANA)
      fbprintf(outfile, "Mana: %d/%d\n", cpd->mana, cpd->max_mana);
    if(cpd->move != PFDEF_MOVE || cpd->max_move != PFDEF_MAXMOVE)
      fbprintf(outfile, "Move: %d/%d\n", cpd->move, cpd->max_move);
	if(cpd->qi != PFDEF_QI || cpd->max_qi != PFDEF_MAXQI)
      fbprintf(outfile, "Qi  : %d/%d\n", cpd->qi, cpd->max_qi);
	if(cpd->vim != PFDEF_VIM || cpd->max_vim != PFDEF_MAXVIM)
      fbprintf(outfile, "Vim : %d/%d\n", cpd->vim, cpd->max_vim);
	if(cpd->aria != PFDEF_ARIA || cpd->max_aria != PFDEF_MAXARIA)
      fbprintf(outfile, "Aria: %d/%d\n", cpd->aria, cpd->max_aria);
    if(cpd->armor != PFDEF_AC)
      fbprintf(outfile, "Ac  : %d\n", cpd->armor);
    if(cpd->gold != PFDEF_GOLD)
      fbprintf(outfile, "Gold: %d\n", cpd->gold);
    if(cpd->bank_gold != PFDEF_BANK)
      fbprintf(outfile, "Bank: %d\n", cpd->bank_gold);
    if(cpd->exp != PFDEF_EXP)
      fbprintf(outfile, "Exp : %d\n", cpd->exp);
    if(cpd->hitroll != PFDEF_HITROLL)
      fbprintf(outfile, "Hrol: %d\n", cpd->hitroll);
    if(cpd->damroll != PFDEF_DAMROLL)
      fbprintf(outfile, "Drol: %d\n", cpd->damroll);
	if(cpd->spellpower != PFDEF_SPELLPOWER)
      fbprintf(outfile, "Spwr: %d\n", cpd->spellpower);

	/* affected_type */
    fbprintf(outfile, "Affs:\n");
	//for (aff = &(vbuf->affected); aff; aff = aff->next) {
    for(i = 0; i < MAX_AFFECT; i++) {
    aff = &(vbuf.affected[i]);
      if(aff->type)
	fbprintf(outfile, "%d %d %d %d %d\n", aff->type, aff->duration,
	  aff->modifier, aff->location, (int)aff->bitvector);
	}
	fbprintf(outfile, "0 0 0 0 0\n");
    fbclose(outfile);
	fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
    fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
}
	send_to_char("Writing plr_index...\r\n", ch);
	fbprintf(index_file, index_buf);
	send_to_char("Closing plr_index...\r\n", ch);
	fbclose(index_file);
	send_to_char("Convert: Binary->Ascii completed.\r\n", ch);
}


ACMD(do_send)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send what to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  send_to_char(buf, vict);
  send_to_char("\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data * ch, char *rawroomstr)
{
  int tmp;
  int location;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  char roomstr[MAX_INPUT_LENGTH];

  one_argument(rawroomstr, roomstr);

  if (!*roomstr) {
    send_to_char("You must supply a room number or name.\r\n", ch);
    return NOWHERE;
  }
  if (isdigit(*roomstr) && !strchr(roomstr, '.')) {
    tmp = atoi(roomstr);
    if ((location = real_room(tmp)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return NOWHERE;
    }
  } else if ((target_mob = get_char_vis(ch, roomstr)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, roomstr))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("That object is not available.\r\n", ch);
      return NOWHERE;
    }
  } else {
    send_to_char("No such creature or object around.\r\n", ch);
    return NOWHERE;
  }

  /* a location has been found -- if you're < GRGOD, check restrictions. */
  if (GET_LEVEL(ch) < LVL_BUILDER) {
    if (ROOM_FLAGGED(location, ROOM_GODROOM)) {
      send_to_char("You are not godly enough to use that room!\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
	world[location].people && world[location].people->next_in_room) {
      send_to_char("There's a private conversation going on in that room.\r\n", ch);
      return NOWHERE;
    }
    if (ROOM_FLAGGED(location, ROOM_HOUSE) &&
	!House_can_enter(ch, world[location].number)) {
      send_to_char("That's private property -- no trespassing!\r\n", ch);
      return NOWHERE;
    }
  }
  return location;
}



ACMD(do_at)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
}


ACMD(do_goto)
{
  int location;

  if ((location = find_target_room(ch, argument)) < 0)
    return;

  if (POOFOUT(ch))
    sprintf(buf, "$n%s", POOFOUT(ch));
  else
    strcpy(buf, "$n disappears in a puff of smoke.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, location);
  look_at_room(ch, 0);
  if (GET_MOUNT(ch)) {
    char_from_room(GET_MOUNT(ch));
    char_to_room(GET_MOUNT(ch), location);
  }

  if (POOFIN(ch))
    sprintf(buf, "$n%s", POOFIN(ch));
  else
    strcpy(buf, "$n appears with an ear-splitting bang.");

  act(buf, TRUE, ch, 0, 0, TO_ROOM);
}



ACMD(do_trans)
{
  struct descriptor_data *i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf)
    send_to_char("Whom do you wish to transfer?\r\n", ch);
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis(ch, buf)))
      send_to_char(NOPERSON, ch);
    else if (victim == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
    else {
      if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim)) {
	send_to_char("Go transfer someone your own size.\r\n", ch);
	return;
      }
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      char_from_room(victim);
      char_to_room(victim, ch->in_room);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      if (PLR_FLAGGED(victim, PLR_FISHING))
        REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_FISHING);
      look_at_room(victim, 0);
    }
  } else {			/* Trans All */
    if (GET_LEVEL(ch) < LVL_BUILDER) {
      send_to_char("I think not.\r\n", ch);
      return;
    }

    for (i = descriptor_list; i; i = i->next)
      if (!i->connected && i->character && i->character != ch) {
	victim = i->character;
	if (GET_LEVEL(victim) >= GET_LEVEL(ch))
	  continue;
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, ch->in_room);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	look_at_room(victim, 0);
      }
    send_to_char(OK, ch);
  }
}



ACMD(do_teleport)
{
  struct char_data *victim;
  int target;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to teleport?\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (victim == ch)
    send_to_char("Use 'goto' to teleport yourself.\r\n", ch);
  else if (!IS_NPC(victim) && GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("Where do you wish to send this person?\r\n", ch);
  else if ((target = find_target_room(ch, buf2)) >= 0) {
    send_to_char(OK, ch);
    act("$n disappears in a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, target);
    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
    act("$n has teleported you!", FALSE, ch, 0, (char *) victim, TO_VICT);
    if (PLR_FLAGGED(victim, PLR_FISHING))
      REMOVE_BIT_AR(PLR_FLAGS(victim), PLR_FISHING);
    look_at_room(victim, 0);
  }
}


ACMD(do_vwear)
{
  one_argument(argument, buf);

   if (!*buf) {
     send_to_char("Usage: vwear <wear position>\r\n"
           "Wear positions are:\r\n"
           "finger  neck   body   head   legs   feet   hands\r\n"
           "shield  arms   about  waist  wrist  ankle  ears\r\n"
           "face    eyes   thumb  float  back   wield           \r\n"
           "---------------------------------------------------\r\n"
           "Item types are:\r\n"
           "light   scroll   wand   staff     treasure potion \r\n"
           "worn    other    trash  container liquid   key \r\n"
           "food    money    pen    boat      fountain\r\n", ch);
    return;
  }
   if (is_abbrev(buf, "finger"))
     vwear_wearpos(ITEM_WEAR_FINGER, ch);
else if (is_abbrev(buf, "neck"))
     vwear_wearpos(ITEM_WEAR_NECK, ch);
else if (is_abbrev(buf, "back"))
     vwear_wearpos(ITEM_WEAR_BACK, ch);
else if (is_abbrev(buf, "body"))
     vwear_wearpos(ITEM_WEAR_BODY, ch);
else if (is_abbrev(buf, "head"))
     vwear_wearpos(ITEM_WEAR_HEAD, ch);
else if (is_abbrev(buf, "legs"))
     vwear_wearpos(ITEM_WEAR_LEGS, ch);
else if (is_abbrev(buf, "feet"))
     vwear_wearpos(ITEM_WEAR_FEET, ch);
else if (is_abbrev(buf, "hands"))
     vwear_wearpos(ITEM_WEAR_HANDS, ch);
else if (is_abbrev(buf, "arms"))
     vwear_wearpos(ITEM_WEAR_ARMS, ch);
else if (is_abbrev(buf, "shield"))
     vwear_wearpos(ITEM_WEAR_SHIELD, ch);
else if (is_abbrev(buf, "wield"))
     vwear_wearpos(ITEM_WEAR_WIELD, ch);
else if (is_abbrev(buf, "hold"))
     vwear_wearpos(ITEM_WEAR_HOLD, ch);
else if (is_abbrev(buf, "about body"))
     vwear_wearpos(ITEM_WEAR_ABOUT, ch);
else if (is_abbrev(buf, "waist"))
     vwear_wearpos(ITEM_WEAR_WAIST, ch);
else if (is_abbrev(buf, "wrist"))
     vwear_wearpos(ITEM_WEAR_WRIST, ch);
else if (is_abbrev(buf, "ankles"))
     vwear_wearpos(ITEM_WEAR_ANKLE, ch);
else if (is_abbrev(buf, "ears"))
     vwear_wearpos(ITEM_WEAR_EAR, ch);
else if (is_abbrev(buf, "eyes"))
     vwear_wearpos(ITEM_WEAR_EYES, ch);
else if (is_abbrev(buf, "face"))
     vwear_wearpos(ITEM_WEAR_FACE, ch);
else if (is_abbrev(buf, "thumb"))
     vwear_wearpos(ITEM_WEAR_THUMB, ch);
else if (is_abbrev(buf, "floating"))
     vwear_wearpos(ITEM_WEAR_FLOAT_1, ch);
else if (is_abbrev(buf, "light")) /* Start of new vwear- Item types */
    vwear_itemtype(ITEM_LIGHT, ch);
else if (is_abbrev(buf, "scroll"))
    vwear_itemtype(ITEM_SCROLL, ch);
else if (is_abbrev(buf, "wand"))
    vwear_itemtype(ITEM_WAND, ch);
else if (is_abbrev(buf, "staff"))
    vwear_itemtype(ITEM_STAFF, ch);
else if (is_abbrev(buf, "treasure"))
    vwear_itemtype(ITEM_TREASURE, ch);
else if (is_abbrev(buf, "potion"))
    vwear_itemtype(ITEM_POTION, ch);
else if (is_abbrev(buf, "worn"))
    vwear_itemtype(ITEM_WORN, ch);
else if (is_abbrev(buf, "other"))
    vwear_itemtype(ITEM_OTHER, ch);
else if (is_abbrev(buf, "trash"))
    vwear_itemtype(ITEM_TRASH, ch);
else if (is_abbrev(buf, "container"))
    vwear_itemtype(ITEM_CONTAINER, ch);
else if (is_abbrev(buf, "liquid"))
    vwear_itemtype(ITEM_DRINKCON, ch);
else if (is_abbrev(buf, "key"))
    vwear_itemtype(ITEM_KEY, ch);
else if (is_abbrev(buf, "food"))
    vwear_itemtype(ITEM_FOOD, ch);
else if (is_abbrev(buf, "money"))
    vwear_itemtype(ITEM_MONEY, ch);
else if (is_abbrev(buf, "pen"))
    vwear_itemtype(ITEM_PEN, ch);
else if (is_abbrev(buf, "boat"))
    vwear_itemtype(ITEM_BOAT, ch);
else if (is_abbrev(buf, "fountain"))
    vwear_itemtype(ITEM_FOUNTAIN, ch);
  else
          send_to_char("Possible positions are:\r\n"
          "Wear positions are:\r\n"
          "finger  neck   body   head   legs    feet    hands\r\n"
          "shield  arms   about  waist  wrist   wield   hold\r\n"
          "face    thumb  eyes   float  back                     \r\n"
          "------------------------------------------------------\r\n" /* new t
ypes */
         "Item types are:\r\n"
          "light   scroll  wand   staff     treasure  potion\r\n"
          "worn    other   trash  container liquid    key\r\n"
          "food    money   pen    boat      fountain\r\n", ch);     
return;
}

int find_attack_type(char *arg)
{
	extern struct attack_hit_type attack_hit_text[];
	int nr;
	
	for (nr=0; nr < NUM_ATTACK_TYPES; nr++)
	{
		if (is_abbrev(arg, attack_hit_text[nr].singular))
			break;
	}
	return (nr);
}

ACMD(do_vnum)
{
  two_arguments(argument, buf, buf2);

  if (!*buf || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "weapon") && !is_abbrev(buf, "obj") && !is_abbrev(buf, "illegal"))) {
    send_to_char("Usage: vnum { obj | mob | weapon | illegal} <name/type> (none for illegal)\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob"))
  {
	  if (!*buf2)
	  {
		  send_to_char("Usage: vnum { obj | mob | weapon | illegal} <name/type> (none for illegal)\r\n", ch);
		  return;
	  } else {
		  vnum_mobile(buf2, ch);
	  }
  }
  else if (is_abbrev(buf, "obj"))
  {
	  if (!*buf2)
	  {
		  send_to_char("Usage: vnum { obj | mob | weapon | illegal} <name/type> (none for illegal)\r\n", ch);
		  return;
	  } else {
		  vnum_object(buf2, ch);
	  }
  }
  else if (is_abbrev(buf, "illegal"))
  {
	  vnum_illegal(ch);
  }
  else if (is_abbrev(buf, "weapon"))
  {
	  if (!*buf2)
	  {
		  send_to_char("Usage: vnum { obj | mob | weapon | illegal} <name/type> (none for illegal)\r\n", ch);
		  return;
	  } else {
		  vnum_weapon(find_attack_type(buf2), ch);
	  }
  }
  else
	  return;
}



void do_stat_room(struct char_data * ch)
{
  struct extra_descr_data *desc;
  struct room_data *rm = &world[ch->in_room];
  int i, found = 0;
  struct obj_data *j = 0;
  struct char_data *k = 0;

  sprintf(buf, "Room name: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name,
	  CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprinttype(rm->sector_type, sector_types, buf2);
  sprintf(buf, "Zone: [%3d], VNum: [%s%5d%s], RNum: [%5d], Type: %s\r\n",
	  zone_table[rm->zone].number, CCGRN(ch, C_NRM), rm->number,
	  CCNRM(ch, C_NRM), ch->in_room, buf2);
  send_to_char(buf, ch);

  sprintbitarray(rm->room_flags, room_bits, RF_ARRAY_MAX, buf2);
  sprintf(buf, "SpecProc: %s, Flags: %s\r\n",
	  (rm->func == NULL) ? "None" : "Exists", buf2);
  send_to_char(buf, ch);

  send_to_char("Description:\r\n", ch);
  if (rm->description)
    send_to_char(rm->description, ch);
  else
    send_to_char("  None.\r\n", ch);

  if (rm->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = rm->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  sprintf(buf, "Chars present:%s", CCYEL(ch, C_NRM));
  for (found = 0, k = rm->people; k; k = k->next_in_room) {
    if (!CAN_SEE(ch, k))
      continue;
    sprintf(buf2, "%s %s(%s)", found++ ? "," : "", GET_NAME(k),
	    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (k->next_in_room)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);
  send_to_char(CCNRM(ch, C_NRM), ch);

  if (rm->contents) {
    sprintf(buf, "Contents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j = rm->contents; j; j = j->next_content) {
      if (!CAN_SEE_OBJ(ch, j))
	continue;
      sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  for (i = 0; i < NUM_OF_DIRS; i++) {
    if (rm->dir_option[i]) {
      if (rm->dir_option[i]->to_room == NOWHERE)
	sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
      else
	sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
		world[rm->dir_option[i]->to_room].number, CCNRM(ch, C_NRM));
      sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
      sprintf(buf, "Exit %s%-5s%s:  To: [%s], Key: [%5d], Keywrd: %s, Type: %s\r\n ",
	      CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
	   rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "None",
	      buf2);
      send_to_char(buf, ch);
      if (rm->dir_option[i]->general_description)
	strcpy(buf, rm->dir_option[i]->general_description);
      else
	strcpy(buf, "  No exit description.\r\n");
      send_to_char(buf, ch);
    }
  }

  /* check the room for a script */
  do_sstat_room(ch);
}



void do_stat_object(struct char_data * ch, struct obj_data * j)
{
  int i, virtual, found;
  struct obj_data *j2;
  struct extra_descr_data *desc;

  virtual = GET_OBJ_VNUM(j);
  sprintf(buf, "Name: '%s%s%s', Aliases: %s\r\n", CCYEL(ch, C_NRM),
	  ((j->short_description) ? j->short_description : "<None>"),
	  CCNRM(ch, C_NRM), j->name);
  send_to_char(buf, ch);
  sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
  if (GET_OBJ_RNUM(j) >= 0)
    strcpy(buf2, (obj_index[GET_OBJ_RNUM(j)].func ? "Exists" : "None"));
  else
    strcpy(buf2, "None");
  sprintf(buf, "VNum: [%s%5d%s], RNum: [%5d], Type: %s, SpecProc: %s, Level: %d\r\n",
   CCGRN(ch, C_NRM), virtual, CCNRM(ch, C_NRM), GET_OBJ_RNUM(j), buf1, buf2, j->obj_flags.minlevel);
  send_to_char(buf, ch);
  sprintf(buf, "L-Des: %s\r\n", ((j->description) ? j->description : "None"));
  send_to_char(buf, ch);

  if (j->ex_description) {
    sprintf(buf, "Extra descs:%s", CCCYN(ch, C_NRM));
    for (desc = j->ex_description; desc; desc = desc->next) {
      strcat(buf, " ");
      strcat(buf, desc->keyword);
    }
    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(strcat(buf, "\r\n"), ch);
  }
  send_to_char("Can be worn on: ", ch);
  sprintbitarray(j->obj_flags.wear_flags, wear_bits, TW_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  send_to_char("Set char bits : ", ch);
  sprintbitarray(j->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Player ID number: %d\r\n", j->obj_flags.player);
  send_to_char(buf, ch);

  send_to_char("Extra flags   : ", ch);
  sprintbitarray(GET_OBJ_EXTRA(j), extra_bits, EF_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\r\n",
     GET_OBJ_WEIGHT(j), GET_OBJ_COST(j), GET_OBJ_RENT(j), GET_OBJ_TIMER(j));
  send_to_char(buf, ch);

  strcpy(buf, "In room: ");
  if (j->in_room == NOWHERE)
    strcat(buf, "Nowhere");
  else {
    sprintf(buf2, "%d", world[j->in_room].number);
    strcat(buf, buf2);
  }
  strcat(buf, ", In object: ");
  strcat(buf, j->in_obj ? j->in_obj->short_description : "None");
  strcat(buf, ", Carried by: ");
  strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Nobody");
  strcat(buf, ", Worn by: ");
  strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Nobody");
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  switch (GET_OBJ_TYPE(j)) {
  case ITEM_LIGHT:
    if (GET_OBJ_VAL(j, 2) == -1)
      strcpy(buf, "Hours left: Infinite");
    else
      sprintf(buf, "Hours left: [%d]", GET_OBJ_VAL(j, 2));
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    sprintf(buf, "Spells: (Level %d) %s, %s, %s", GET_OBJ_VAL(j, 0),
	    skill_name(GET_OBJ_VAL(j, 1)), skill_name(GET_OBJ_VAL(j, 2)),
	    skill_name(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf(buf, "Spell: %s at level %d, %d (of %d) charges remaining",
	    skill_name(GET_OBJ_VAL(j, 3)), GET_OBJ_VAL(j, 0),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_WEAPON:
    sprintf(buf, "Todam: %dd%d, Message type: %d, MinLevel: %d",
            weapon_dam_dice[j->obj_flags.minlevel][0],
            weapon_dam_dice[j->obj_flags.minlevel][1],
	    GET_OBJ_VAL(j, 3), j->obj_flags.minlevel);
    if (GET_OBJ_SPELL(j)) {
      sprintf(buf, "%s\r\nThis weapon casts %s %d percent of the time.", buf,
              spells[GET_OBJ_SPELL(j)], GET_OBJ_SPELL_EXTRA(j));
    }
    break;
  case ITEM_ARMOR:
    sprintf(buf, "AC-apply: [%d]", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_TRAP:
    sprintf(buf, "Spell: %d, - Hitpoints: %d",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1));
    break;
  case ITEM_CONTAINER:
    sprintbit(GET_OBJ_VAL(j, 1), container_bits, buf2);
    sprintf(buf, "Weight capacity: %d, Lock Type: %s, Key Num: %d, Corpse: %s",
	    GET_OBJ_VAL(j, 0), buf2, GET_OBJ_VAL(j, 2),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    sprinttype(GET_OBJ_VAL(j, 2), drinks, buf2);
    sprintf(buf, "Capacity: %d, Contains: %d, Poisoned: %s, Liquid: %s",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), YESNO(GET_OBJ_VAL(j, 3)),
	    buf2);
    break;
  case ITEM_NOTE:
    sprintf(buf, "Tongue: %d", GET_OBJ_VAL(j, 0));
    break;
  case ITEM_KEY:
    strcpy(buf, "");
    break;
  case ITEM_FOOD:
    sprintf(buf, "Makes full: %d, Poisoned: %s", GET_OBJ_VAL(j, 0),
	    YESNO(GET_OBJ_VAL(j, 3)));
    break;
  case ITEM_MONEY:
    sprintf(buf, "Coins: %d", GET_OBJ_VAL(j, 0));
    break;
  default:
    sprintf(buf, "Values 0-3: [%d] [%d] [%d] [%d]",
	    GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1),
	    GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    break;
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  /*
   * I deleted the "equipment status" code from here because it seemed
   * more or less useless and just takes up valuable screen space.
   */

  if (j->contains) {
    sprintf(buf, "\r\nContents:%s", CCGRN(ch, C_NRM));
    for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
      sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
      strcat(buf, buf2);
      if (strlen(buf) >= 62) {
	if (j2->next_content)
	  send_to_char(strcat(buf, ",\r\n"), ch);
	else
	  send_to_char(strcat(buf, "\r\n"), ch);
	*buf = found = 0;
      }
    }

    if (*buf)
      send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
  }
  found = 0;
  send_to_char("Affections:", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (j->affected[i].modifier) {
      sprinttype(j->affected[i].location, apply_types, buf2);
      sprintf(buf, "%s %+d to %s", found++ ? "," : "",
	      j->affected[i].modifier, buf2);
      send_to_char(buf, ch);
    }
  if (!found)
    send_to_char(" None", ch);

  send_to_char("\r\n", ch);
   if (GET_PKLOOTER(j)) {
     sprintf(buf, "PkLooter: %s\r\n", GET_NAME(GET_PKLOOTER(j)));
     send_to_char(buf, ch);
   }

  /* check the object for a script */
  do_sstat_object(ch, j);
}


void do_stat_character(struct char_data * ch, struct char_data * k)
{
  int i, i2, found = 0;
  struct obj_data *j;
  struct follow_type *fol;
  struct affected_type *aff;
  char **abil_list;
  extern struct attack_hit_type attack_hit_text[];

  switch (GET_SEX(k)) {
  case SEX_NEUTRAL:    strcpy(buf, "NEUTRAL-SEX");   break;
  case SEX_MALE:       strcpy(buf, "MALE");          break;
  case SEX_FEMALE:     strcpy(buf, "FEMALE");        break;
  default:             strcpy(buf, "ILLEGAL-SEX!!"); break;
  }

  sprintf(buf2, " %s '%s'  IDNum: [%5ld], In room [%5d]\r\n",
	  (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	  GET_NAME(k), GET_IDNUM(k), world[k->in_room].number);
  send_to_char(strcat(buf, buf2), ch);
  if (IS_MOB(k)) {
    sprintf(buf, "Alias: %s, VNum: [%5d], RNum: [%5d]\r\n",
	    k->player.name, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    send_to_char(buf, ch);
  }
  sprintf(buf, "Title: %s\r\n", (k->player.title ? k->player.title : "<None>"));
  send_to_char(buf, ch);

  sprintf(buf, "L-Des: %s", (k->player.long_descr ? k->player.long_descr : "<None>\r\n"));
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintf(buf, "Class: %s,  Race: %s",
            npc_class_types[(int)GET_CLASS(k)],
            npc_race_types[(int)GET_RACE(k)]);
  } else {
    sprintf(buf, "Class: %s,  Race: %s", 
            pc_class_types[(int)GET_CLASS(k)],
            pc_race_types[(int)GET_RACE(k)]);
  }

  sprintf(buf2, ", Lev: [%s%2d%s], XP: [%s%7d%s], Align: [%4d]\r\n",
	  CCYEL(ch, C_NRM), GET_LEVEL(k), CCNRM(ch, C_NRM),
	  CCYEL(ch, C_NRM), GET_EXP(k), CCNRM(ch, C_NRM),
	  GET_ALIGNMENT(k));
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (!IS_NPC(k)) {
    strcpy(buf1, (char *) asctime(localtime(&(k->player.time.birth))));
    strcpy(buf2, (char *) asctime(localtime(&(k->player.time.logon))));
    buf1[10] = buf2[10] = '\0';

    sprintf(buf, "Created: [%s], Last Logon: [%s], Played [%dh %dm], Age [%d]\r\n",
	    buf1, buf2, k->player.time.played / 3600,
	    ((k->player.time.played / 3600) % 60), age(k).year);
    send_to_char(buf, ch);

    sprintf(buf, "Hometown: [%s], Load in: [%d], (STL[%d]/per[%d]/NSTL[%d])",
	    hometowns[k->player.hometown], GET_LOADROOM(k), GET_PRACTICES(k),
	    int_app[GET_INT(k)].learn, wis_app[GET_WIS(k)].bonus);
    /*. Display OLC zone for immorts .*/
    if(GET_LEVEL(k) >= LVL_IMMORT)
      sprintf(buf, "%s, OLC[%d]", buf, GET_OLC_ZONE(k));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
  }
  sprintf(buf, "Str: [%s%d/%d%s]  Int: [%s%d%s]  Wis: [%s%d%s]  "
	  "Dex: [%s%d%s]  Con: [%s%d%s]  Cha: [%s%d%s]  Luck: [%s%d%s]\r\n",
	  CCCYN(ch, C_NRM), GET_STR(k), GET_ADD(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_INT(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_WIS(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_DEX(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CON(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_CHA(k), CCNRM(ch, C_NRM),
	  CCCYN(ch, C_NRM), GET_LUCK(k), CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Hit p.:[%s%d/%d+%d%s] Mana p.:[%s%d/%d+%d%s] Move p.:[%s%d/%d+%d%s] Vim.:[%s%d/%d%s] Qi:[%s%d/%d%s] Aria:[%s%d/%d%s]\r\n",
	  CCGRN(ch, C_NRM), GET_HIT(k), GET_MAX_HIT(k), hit_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MANA(k), GET_MAX_MANA(k), mana_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_MOVE(k), GET_MAX_MOVE(k), move_gain(k), CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_VIM(k), GET_MAX_VIM(k),CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_QI(k), GET_MAX_QI(k),CCNRM(ch, C_NRM),
	  CCGRN(ch, C_NRM), GET_ARIA(k), GET_MAX_ARIA(k),CCNRM(ch, C_NRM));
  send_to_char(buf, ch);

  sprintf(buf, "Coins: [%d], Bank: [%d] (Total: %d), Quest points: (%d) Rank: %d\r\n",
	  GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k),
          GET_QPOINTS(k), GET_RANK(k));
  send_to_char(buf, ch);

  sprintf(buf, "AC: [%d/100], HR: [%d], DR: [%d], SP: [%d], Saving throws: [%d/%d/%d/%d/%d]\r\n",
	  GET_AC(k), k->points.hitroll, k->points.damroll, k->points.spellpower, GET_SAVE(k, 0),
	  GET_SAVE(k, 1), GET_SAVE(k, 2), GET_SAVE(k, 3), GET_SAVE(k, 4));
  send_to_char(buf, ch);

  sprintf(buf, "Killed /cy%d/c0 mobs and died /cy%d/c0 times.\r\n", (int)GET_NUM_KILLS(k), (int)GET_NUM_DEATHS(k));
  send_to_char(buf, ch);

  sprinttype(GET_POS(k), position_types, buf2);
  sprintf(buf, "Pos: %s, Fighting: %s", buf2,
	  (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Nobody"));

  if (IS_NPC(k)) {
    strcat(buf, ", Attack type: ");
    strcat(buf, attack_hit_text[k->mob_specials.attack_type].singular);
  }
  if (k->desc) {
    sprinttype(k->desc->connected, connected_types, buf2);
    strcat(buf, ", Connected: ");
    strcat(buf, buf2);
  }
  send_to_char(strcat(buf, "\r\n"), ch);

  strcpy(buf, "Default position: ");
  sprinttype((k->mob_specials.default_pos), position_types, buf2);
  strcat(buf, buf2);

  sprintf(buf2, ", Idle Timer (in tics) [%d]\r\n", k->char_specials.timer);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  if (IS_NPC(k)) {
    sprintbitarray(MOB_FLAGS(k), action_bits, PM_ARRAY_MAX, buf2);
    sprintf(buf, "NPC flags: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  } else {
    sprintbitarray(PLR_FLAGS(k), player_bits, PM_ARRAY_MAX, buf2);
    sprintf(buf, "PLR: %s%s%s\r\n", CCCYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    sprintbitarray(PRF_FLAGS(k), preference_bits, PR_ARRAY_MAX, buf2);
    sprintf(buf, "PRF: %s%s%s\r\n", CCGRN(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
  }

  if (IS_MOB(k)) {
    sprintf(buf, "Mob Spec-Proc: %s, NPC Bare Hand Dam: %dd%d\r\n",
	    (mob_index[GET_MOB_RNUM(k)].func ? "Exists" : "None"),
	    k->mob_specials.damnodice, k->mob_specials.damsizedice);
    send_to_char(buf, ch);
  }
  sprintf(buf, "Carried: weight: %d, items: %d; ",
	  IS_CARRYING_W(k), IS_CARRYING_N(k));

  for (i = 0, j = k->carrying; j; j = j->next_content, i++);
  sprintf(buf, "%sItems in: inventory: %d, ", buf, i);

  for (i = 0, i2 = 0; i < NUM_WEARS; i++)
    if (GET_EQ(k, i))
      i2++;
  sprintf(buf2, "eq: %d\r\n", i2);
  strcat(buf, buf2);
  send_to_char(buf, ch);

  sprintf(buf, "Hunger: %d, Thirst: %d, Drunk: %d\r\n",
	  GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK));
  send_to_char(buf, ch);

  sprintf(buf, "Master is: %s, Followers are:",
	  ((k->master) ? GET_NAME(k->master) : "<none>"));

  for (fol = k->followers; fol; fol = fol->next) {
    sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch));
    strcat(buf, buf2);
    if (strlen(buf) >= 62) {
      if (fol->next)
	send_to_char(strcat(buf, ",\r\n"), ch);
      else
	send_to_char(strcat(buf, "\r\n"), ch);
      *buf = found = 0;
    }
  }

  if (*buf)
    send_to_char(strcat(buf, "\r\n"), ch);

  /* Showing the bitvector */
  sprintbitarray(AFF_FLAGS(k), affected_bits, AF_ARRAY_MAX, buf2);
  sprintf(buf, "AFF: %s%s%s\r\n", CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
/*
  sprintbit(AFF_FLAGS2(k), affected_bits2, buf2);
  sprintf(buf, "%sAFF2: %s%s%s\r\n", buf, CCYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
*/
  send_to_char(buf, ch);

  /* Routine to show what spells a char is affected by */
  if (k->affected) {
    for (aff = k->affected; aff; aff = aff->next) {
      *buf2 = '\0';
      switch (aff->src) {
      case ABT_SPELL: abil_list = spells;  break;
      case ABT_CHANT: abil_list = chants;  break;
      case ABT_PRAYER:abil_list = prayers; break;
      case ABT_SONG:  abil_list = songs;   break;
      default: return; /* Damn.  Shouldn't happen! */
      }
      sprintf(buf, "SPL: (%3dhr) %s%-21s%s ", aff->duration + 1,
	      CCCYN(ch, C_NRM), (aff->type >= 0 && aff->type <= MAX_ABILITIES) ?
	      abil_list[aff->type] : "TYPE UNDEFINED", CCNRM(ch, C_NRM));
      if (aff->modifier) {
	sprintf(buf2, "%+d to %s", aff->modifier, apply_types[(int) aff->location]);
	strcat(buf, buf2);
      }
      if (aff->bitvector) {
	if (*buf2)
	  strcat(buf, ", sets ");
	else
	  strcat(buf, "sets ");
	strcpy(buf2, affected_bits[aff->bitvector]);
	strcat(buf, buf2);
      }
      send_to_char(strcat(buf, "\r\n"), ch);
    }
  }

  if (!IS_NPC(k)) {
    sprintf(buf, "Pkill timeout: %d\r\n", GET_PKSAFETIMER(k));
    send_to_char(buf, ch);
  }

  if (IS_NPC(k)) {
   sprintf(buf, "Killvictimer: %d  Killvictim: %s\r\n", GET_KILLVICTIMER(k),
                (GET_KILLVICTIM(k) ? GET_NAME(GET_KILLVICTIM(k)) : "None"));
   send_to_char(buf, ch);
  }

  /* check mobiles for a script */
  if (IS_NPC(k)) {
    do_sstat_character(ch, k);
    if (SCRIPT_MEM(k)) {
      struct script_memory *mem = SCRIPT_MEM(k);
      send_to_char("Script memory:\r\n  Remember             Command\r\n", ch);
      while (mem) {
        struct char_data *mc = find_char(mem->id);
        if (!mc) send_to_char("  ** Corrupted!\r\n", ch);
        else {
          if (mem->cmd) sprintf(buf,"  %-20.20s%s\r\n",GET_NAME(mc),mem->cmd);
          else sprintf(buf,"  %-20.20s <default>\r\n",GET_NAME(mc));
          send_to_char(buf, ch);
        }
      mem = mem->next;
      }
    }
  } else {
    /* this is a PC, display their global variables */
    if (k->script && k->script->global_vars) {
      struct trig_var_data *tv;
      char name[MAX_INPUT_LENGTH];
      void find_uid_name(char *uid, char *name);

      send_to_char("Global Variables:\r\n", ch);

      /* currently, variable context for players is always 0, so it is */
      /* not displayed here. in the future, this might change */
      for (tv = k->script->global_vars; tv; tv = tv->next) {
        if (*(tv->value) == UID_CHAR) {
          find_uid_name(tv->value, name);
          sprintf(buf, "    %10s:  [UID]: %s\r\n", tv->name, name);
        } else
          sprintf(buf, "    %10s:  %s\r\n", tv->name, tv->value);
        send_to_char(buf, ch);
      }
    }
  }

}


ACMD(do_stat)
{
  struct char_data *victim = 0;
  struct obj_data *object = 0;
  struct char_file_u tmp_store;
  int tmp;

  half_chop(argument, buf1, buf2);

  if (!*buf1) {
    send_to_char("Stats on who or what?\r\n", ch);
    return;
  } else if (is_abbrev(buf1, "room")) {
    do_stat_room(ch);
  } else if (is_abbrev(buf1, "mob")) {
    if (!*buf2)
      send_to_char("Stats on which mobile?\r\n", ch);
    else {
      if ((victim = get_char_vis(ch, buf2)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such mobile around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "player")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      if ((victim = get_player_vis(ch, buf2, 0)))
	do_stat_character(ch, victim);
      else
	send_to_char("No such player around.\r\n", ch);
    }
  } else if (is_abbrev(buf1, "file")) {
    if (!*buf2) {
      send_to_char("Stats on which player?\r\n", ch);
    } else {
      CREATE(victim, struct char_data, 1);
      clear_char(victim);
      if (load_char(buf2, &tmp_store) > -1) {
	store_to_char(&tmp_store, victim);
	if (GET_LEVEL(victim) > GET_LEVEL(ch) && GET_IDNUM(ch) != 1127)
	  send_to_char("Sorry, you can't do that.\r\n", ch);
	else
	  do_stat_character(ch, victim);
	free_char(victim);
      } else {
	send_to_char("There is no such player.\r\n", ch);
	free(victim);
      }
    }
  } else if (is_abbrev(buf1, "object")) {
    if (!*buf2)
      send_to_char("Stats on which object?\r\n", ch);
    else {
      if ((object = get_obj_vis(ch, buf2)))
	do_stat_object(ch, object);
      else
	send_to_char("No such object around.\r\n", ch);
    }
  } else {
    if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)))
      do_stat_object(ch, object);
    else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)))
      do_stat_object(ch, object);
    else if ((victim = get_char_room_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)))
      do_stat_object(ch, object);
    else if ((victim = get_char_vis(ch, buf1)))
      do_stat_character(ch, victim);
    else if ((object = get_obj_vis(ch, buf1)))
      do_stat_object(ch, object);
    else
      send_to_char("Nothing around by that name.\r\n", ch);
  }
}


ACMD(do_shutdown)
{
  extern int circle_shutdown, circle_reboot;
  struct descriptor_data *i, *next_desc;

  if (subcmd != SCMD_SHUTDOWN) {
    send_to_char("If you want to shut something down, say so!\r\n", ch);
    return;
  }
  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down.\r\n");
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "reboot")) {
    sprintf(buf, "(GC) Reboot by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    touch(FASTBOOT_FILE);
    circle_shutdown = circle_reboot = 1;
  } else if (!str_cmp(arg, "now")) {
    sprintf(buf, "(GC) Shutdown NOW by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting.. come back in a minute or two.\r\n");
    circle_shutdown = 1;
    circle_reboot = 2;
  } else if (!str_cmp(arg, "die")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch(KILLSCRIPT_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "pause")) {
    sprintf(buf, "(GC) Shutdown by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Shutting down for maintenance.\r\n");
    touch(PAUSE_FILE);
    circle_shutdown = 1;
  } else if (!str_cmp(arg, "update")) {
    sprintf(buf, "(GC) Shutdown UPDATE by %s.", GET_NAME(ch));
    log(buf);
    send_to_all("Rebooting...  Back in a flash!\r\n");
    touch(UPDATE_FILE);
	system("rm -f /home/dibrova/prod/bin/circle");
	system("cp /home/dibrova/dibrova~/bin/circle /home/dibrova/prod/bin/circle");
    circle_shutdown = circle_reboot = 1;
  } else
    send_to_char("Unknown shutdown option.\r\n", ch);
  
  if(circle_shutdown) {

      for (i = descriptor_list; i; i = next_desc) {
        next_desc = i->next;

        if (i->connected)
          continue;
        command_interpreter(i->character, "save");
      }
  }

}


void stop_snooping(struct char_data * ch)
{
  if (!ch->desc->snooping)
    send_to_char("You aren't snooping anyone.\r\n", ch);
  else {
    send_to_char("You stop snooping.\r\n", ch);
    ch->desc->snooping->snoop_by = NULL;
    ch->desc->snooping = NULL;
  }
}


ACMD(do_snoop)
{
  struct char_data *victim, *tch;

  if (!ch->desc)
    return;

  one_argument(argument, arg);

  if (!*arg)
    stop_snooping(ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such person around.\r\n", ch);
  else if (!victim->desc)
    send_to_char("There's no link.. nothing to snoop.\r\n", ch);
  else if (victim == ch)
    stop_snooping(ch);
  else if (victim->desc->snoop_by)
    send_to_char("Busy already. \r\n", ch);
  else if (victim->desc->snooping == ch->desc)
    send_to_char("Don't be stupid.\r\n", ch);
  else {
    if (victim->desc->original)
      tch = victim->desc->original;
    else
      tch = victim;

    if (GET_LEVEL(tch) >= GET_LEVEL(ch) && GET_IDNUM(ch) != 1 && GET_IDNUM(ch) != 2) {
      send_to_char("You can't.\r\n", ch);
      return;
    }
    send_to_char(OK, ch);

    if (ch->desc->snooping)
      ch->desc->snooping->snoop_by = NULL;

    ch->desc->snooping = victim->desc;
    victim->desc->snoop_by = ch->desc;
  }
}



ACMD(do_switch)
{
  struct char_data *victim;

  one_argument(argument, arg);

  if (ch->desc->original)
    send_to_char("You're already switched.\r\n", ch);
  else if (!*arg)
    send_to_char("Switch with who?\r\n", ch);
  else if (!(victim = get_char_vis(ch, arg)))
    send_to_char("No such character.\r\n", ch);
  else if (ch == victim)
    send_to_char("Hee hee... we are jolly funny today, eh?\r\n", ch);
  else if (victim->desc)
    send_to_char("You can't do that, the body is already in use!\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_IMPL) && !IS_NPC(victim))
    send_to_char("You aren't holy enough to use a mortal's body.\r\n", ch);
  else {
    send_to_char(OK, ch);

    ch->desc->character = victim;
    ch->desc->original = ch;

    victim->desc = ch->desc;
    ch->desc = NULL;
  }
}


ACMD(do_return)
{
  if (ch->desc && ch->desc->original) {
    send_to_char("You return to your original body.\r\n", ch);

    /* JE 2/22/95 */
    /* if someone switched into your original body, disconnect them */
    if (ch->desc->original->desc)
      close_socket(ch->desc->original->desc);

    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;

    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
  }
}



ACMD(do_load)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: load { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	0, 0, TO_ROOM);
    act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
    act("You create $N.", FALSE, ch, 0, mob, TO_CHAR);
    load_mtrigger(mob);
    sprintf(buf, "(GC) %s has LOADED %s.", GET_NAME(ch), GET_NAME(mob));
    mudlog(buf, BRF, LVL_ADMIN, TRUE); 
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You create $p.", FALSE, ch, obj, 0, TO_CHAR);
    load_otrigger(obj);
    sprintf(buf, "(GC) %s has LOADED %s.", GET_NAME(ch),obj->short_description);
    mudlog(buf, BRF, LVL_ADMIN, TRUE); 
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}



ACMD(do_vstat)
{
  struct char_data *mob;
  struct obj_data *obj;
  int number, r_num;

  two_arguments(argument, buf, buf2);

  if (!*buf || !*buf2 || !isdigit(*buf2)) {
    send_to_char("Usage: vstat { obj | mob } <number>\r\n", ch);
    return;
  }
  if ((number = atoi(buf2)) < 0) {
    send_to_char("A NEGATIVE number??\r\n", ch);
    return;
  }
  if (is_abbrev(buf, "mob")) {
    if ((r_num = real_mobile(number)) < 0) {
      send_to_char("There is no monster with that number.\r\n", ch);
      return;
    }
    mob = read_mobile(r_num, REAL);
    char_to_room(mob, 0);
    do_stat_character(ch, mob);
    extract_char(mob);
  } else if (is_abbrev(buf, "obj")) {
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\r\n", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    do_stat_object(ch, obj);
    extract_obj(obj);
  } else
    send_to_char("That'll have to be either 'obj' or 'mob'.\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  one_argument(argument, buf);

  if (*buf) {			/* argument supplied. destroy single object
				 * or char */
    if ((vict = get_char_room_vis(ch, buf))) {
      if (!IS_NPC(vict) && (GET_LEVEL(ch) <= GET_LEVEL(vict))) {
	send_to_char("Fuuuuuuuuu!\r\n", ch);
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (!IS_NPC(vict)) {
	sprintf(buf, "(GC) %s has purged %s.", GET_NAME(ch), GET_NAME(vict));
	mudlog(buf, BRF, LVL_ADMIN, TRUE);
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = NULL;
	}
      }
      extract_char(vict);
    } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      save_corpses();
      extract_obj(obj);
    } else {
      send_to_char("Nothing here by that name.\r\n", ch);
      return;
    }

    send_to_char(OK, ch);
  } else {			/* no argument. clean out the room */
    act("$n gestures... You are surrounded by scorching flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_room("The world seems a little cleaner.\r\n", ch->in_room);

    for (vict = world[ch->in_room].people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict))
	extract_char(vict);
    }

    for (obj = world[ch->in_room].contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}



static char *logtypes[] = {
"off", "brief", "normal", "complete", "\n"};

ACMD(do_syslog)
{
  int tp;

  one_argument(argument, arg);

  if (!*arg) {
    tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) +
	  (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
    sprintf(buf, "Your syslog is currently %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
    return;
  }
  if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
    send_to_char("Usage: syslog { Off | Brief | Normal | Complete }\r\n", ch);
    return;
  }
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
  REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);
  if (tp & 1) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG1);
  if (tp & 2) SET_BIT_AR(PRF_FLAGS(ch), PRF_LOG2);

  sprintf(buf, "Your syslog is now %s.\r\n", logtypes[tp]);
  send_to_char(buf, ch);
}



ACMD(do_advance)
{
  struct char_data *victim;
  char *name = arg, *level = buf2;
  int newlevel, oldlevel;
  void do_start(struct char_data *ch);

/*  void gain_exp(struct char_data * ch, int gain); */

  two_arguments(argument, name, level);

  if (*name) {
    if (!(victim = get_char_vis(ch, name))) {
      send_to_char("That player is not here.\r\n", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\r\n", ch);
    return;
  }

  if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
    send_to_char("Maybe that's not such a great idea.\r\n", ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("NO!  Not on NPC's.\r\n", ch);
    return;
  }
  if (!*level || (newlevel = atoi(level)) <= 0) {
    send_to_char("That's not a level!\r\n", ch);
    return;
  }
  if (newlevel > LVL_IMPL) {
    sprintf(buf, "%d is the highest possible level.\r\n", LVL_IMPL);
    send_to_char(buf, ch);
    return;
  }
  if (newlevel > GET_LEVEL(ch)) {
    send_to_char("Yeah, right.\r\n", ch);
    return;
  }
  if (newlevel == GET_LEVEL(victim)) {
    send_to_char("They are already at that level.\r\n", ch);
    return;
  }
  oldlevel = GET_LEVEL(victim);
  if (newlevel < GET_LEVEL(victim)) {
    do_start(victim);
    GET_LEVEL(victim) = newlevel;
    send_to_char("You are momentarily enveloped by darkness!\r\n"
		 "You feel somewhat diminished.\r\n", victim);
  } else {
    act("$n makes some strange gestures.\r\n"
	"A strange feeling comes upon you,\r\n"
	"Like a giant hand, light comes down\r\n"
	"from above, grabbing your body, that\r\n"
	"begins to pulse with colored lights\r\n"
	"from inside.\r\n\r\n"
	"Your head seems to be filled with demons\r\n"
	"from another plane as your body dissolves\r\n"
	"to the elements of time and space itself.\r\n"
	"Suddenly a silent explosion of light\r\n"
	"snaps you back to reality.\r\n\r\n"
	"You feel slightly different.", FALSE, ch, 0, victim, TO_VICT);
  }

  send_to_char(OK, ch);

  sprintf(buf, "(GC) %s has advanced %s to level %d (from %d)",
	  GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);
  log(buf);
  gain_exp_regardless(victim,
	 level_exp(GET_CLASS(victim), newlevel) - GET_EXP(victim));
  save_char(victim, NOWHERE);
}



ACMD(do_restore)
{
  struct char_data *vict;

  one_argument(argument, buf);
  if (!*buf) {
    send_to_char("Whom do you wish to restore?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, buf))) {
    send_to_char(NOPERSON, ch);
    return;
  } else {
    GET_HIT(vict)  = GET_MAX_HIT(vict);
    GET_MANA(vict) = GET_MAX_MANA(vict);
    GET_MOVE(vict) = GET_MAX_MOVE(vict);
    GET_QI(vict)   = GET_MAX_QI(vict);
    GET_VIM(vict)  = GET_MAX_VIM(vict);
    GET_ARIA(vict) = GET_MAX_ARIA(vict);

      if (GET_LEVEL(vict) >= LVL_BUILDER) {
      vict->real_stats.str_add = 100;
      vict->real_stats.intel = 25;
      vict->real_stats.wis = 25;
      vict->real_stats.dex = 25;
      vict->real_stats.str = 25;
      vict->real_stats.con = 25;
      vict->real_stats.cha = 25;
      vict->real_stats.luck = 25;
      }
    }
    update_pos(vict);
    send_to_char(OK, ch);
    act("You have been fully healed by $N!", FALSE, vict, 0, ch, TO_CHAR);

}


void perform_immort_vis(struct char_data *ch)
{
  void appear(struct char_data *ch);

  if (GET_INVIS_LEV(ch) == 0 && !IS_AFFECTED(ch, AFF_HIDE | AFF_INVISIBLE)) {
    send_to_char("You are already fully visible.\r\n", ch);
    return;
  }
   
  GET_INVIS_LEV(ch) = 0;
  appear(ch);
  send_to_char("You are now fully visible.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
  struct char_data *tch;

  if (IS_NPC(ch))
    return;

  for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch == ch)
      continue;
    if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
      act("You blink and suddenly realize that $n is gone.", FALSE, ch, 0,
	  tch, TO_VICT);
    if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
      act("You suddenly realize that $n is standing beside you.", FALSE, ch, 0,
	  tch, TO_VICT);
  }

  GET_INVIS_LEV(ch) = level;
  sprintf(buf, "Your invisibility level is %d.\r\n", level);
  send_to_char(buf, ch);
}
  

ACMD(do_invis)
{
  int level;

  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\r\n", ch);
    return;
  }

  one_argument(argument, arg);
  if (!*arg) {
    if (GET_INVIS_LEV(ch) > 0)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, GET_LEVEL(ch));
  } else {
    level = atoi(arg);
    if (level < 151)
      send_to_char("You can't go visible below level 151.\r\n", ch);
    if (level > GET_LEVEL(ch))
      send_to_char("You can't go invisible above your own level.\r\n", ch);
    else if (level < 1)
      perform_immort_vis(ch);
    else
      perform_immort_invis(ch, level);
  }
}


ACMD(do_gecho)
{
  struct descriptor_data *pt;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument)
    send_to_char("That must be a mistake...\r\n", ch);
  else {
    sprintf(buf, "%s/c0\r\n", argument);
    for (pt = descriptor_list; pt; pt = pt->next)
      if (!pt->connected && pt->character && pt->character != ch)
	send_to_char(buf, pt->character);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
      send_to_char(OK, ch);
    else
      strcat(buf, "/c0");
      send_to_char(buf, ch);
  }
}


/*
ACMD(do_poofset)
{
  char **msg;

  switch (subcmd) {
  case SCMD_POOFIN:    msg = &(POOFIN(ch));    break;
  case SCMD_POOFOUT:   msg = &(POOFOUT(ch));   break;
  default:    return;    break;
  }

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  send_to_char(OK, ch);
}
*/

ACMD(do_poof)
{
 sprintf(buf, "Current Poofin: %s\r\n",POOFIN(ch));
 send_to_char(buf, ch);
 sprintf(buf2, "Current Poofout: %s\r\n",POOFOUT(ch));
 send_to_char(buf2, ch);
}

ACMD(do_poofout)
{
  if (POOFOUT(ch))
    free(POOFOUT(ch));
  POOFOUT(ch) = str_dup(argument);
  sprintf(buf, "Poofout set to: %s\r\n", POOFOUT(ch));
  send_to_char(buf, ch);
}

ACMD(do_poofin)
{
  if (POOFIN(ch))
    free(POOFIN(ch));
  POOFIN(ch) = str_dup(argument);
  sprintf(buf, "Poofin set to: %s\r\n", POOFIN(ch));
  send_to_char(buf, ch);
}



ACMD(do_dc)
{
  struct descriptor_data *d;
  int num_to_dc;

  one_argument(argument, arg);
  if (!(num_to_dc = atoi(arg))) {
    send_to_char("Usage: DC <user number> (type USERS for a list)\r\n", ch);
    return;
  }
  for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

  if (!d) {
    send_to_char("No such connection.\r\n", ch);
    return;
  }
  if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch)) {
    if (!CAN_SEE(ch, d->character))
      send_to_char("No such connection.\r\n", ch);
    else
      send_to_char("Umm.. maybe that's not such a good idea...\r\n", ch);
    return;
  }

  /* We used to just close the socket here using close_socket(), but
   * various people pointed out this could cause a crash if you're
   * closing the person below you on the descriptor list.  Just setting
   * to CON_CLOSE leaves things in a massively inconsistent state so I
   * had to add this new flag to the descriptor.
   */
  d->close_me = 1;
  sprintf(buf, "Connection #%d closed.\r\n", num_to_dc);
  send_to_char(buf, ch);
  sprintf(buf, "(GC) Connection closed by %s.", GET_NAME(ch));
  log(buf);
}



ACMD(do_wizlock)
{
  int value;
  char *when;

  one_argument(argument, arg);
  if (*arg) {
    value = atoi(arg);
    if (value < 0 || value > GET_LEVEL(ch)) {
      send_to_char("Invalid wizlock value.\r\n", ch);
      return;
    }
    restrict = value;
    when = "now";
  } else
    when = "currently";

  switch (restrict) {
  case 0:
    sprintf(buf, "The game is %s completely open.\r\n", when);
    break;
  case 1:
    sprintf(buf, "The game is %s closed to new players.\r\n", when);
    break;
  default:
    sprintf(buf, "Only level %d and above may enter the game %s.\r\n",
	    restrict, when);
    break;
  }
  send_to_char(buf, ch);
}


ACMD(do_date)
{
  char *tmstr;
  time_t mytime;
  int d, h, m;
  extern time_t boot_time;

  if (subcmd == SCMD_DATE)
    mytime = time(0);
  else
    mytime = boot_time;

  tmstr = (char *) asctime(localtime(&mytime));
  *(tmstr + strlen(tmstr) - 1) = '\0';

  if (subcmd == SCMD_DATE)
    sprintf(buf, "Current machine time: %s\r\n", tmstr);
  else {
    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    sprintf(buf, "Up since %s: %d day%s, %d:%02d\r\n", tmstr, d,
	    ((d == 1) ? "" : "s"), h, m);
  }

  send_to_char(buf, ch);
}



ACMD(do_last)
{
  struct char_file_u chdata;
  extern char *class_abbrevs[];

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("For whom do you wish to search?\r\n", ch);
    return;
  }
  if (load_char(arg, &chdata) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if ((chdata.level > GET_LEVEL(ch)) && (GET_LEVEL(ch) < LVL_IMPL)) {
    send_to_char("You are not sufficiently godly for that!\r\n", ch);
    return;
  }
  sprintf(buf, "[%5ld] [%2d %s] %-12s : %-18s : %-20s\r\n",
	  chdata.char_specials_saved.idnum, (int) chdata.level,
	  class_abbrevs[(int) chdata.class], chdata.name, chdata.host,
	  ctime(&chdata.last_logon));
  send_to_char(buf, ch);
}


ACMD(do_force)
{
  struct descriptor_data *i, *next_desc;
  struct char_data *vict, *next_force;
  char to_force[MAX_INPUT_LENGTH + 2];

  half_chop(argument, arg, to_force);

  sprintf(buf1, "$n has forced you to '%s'.", to_force);

  if (!*arg || !*to_force)
    send_to_char("Whom do you wish to force do what?\r\n", ch);
  else if ((GET_LEVEL(ch) < LVL_BUILDER) ||
           (str_cmp("all", arg) && str_cmp("room", arg))) {
    if (!(vict = get_char_vis(ch, arg)))
      send_to_char(NOPERSON, ch);
    else if ((GET_LEVEL(ch) <= GET_LEVEL(vict)) && !IS_NPC(vict) &&
             GET_IDNUM(ch) != 1)
      send_to_char("No, no, no!\r\n", ch);
    else {
      send_to_char(OK, ch);
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      sprintf(buf, "(GC) %s forced %s to %s", GET_NAME(ch), GET_NAME(vict), to_force);
      mudlog(buf, NRM, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      command_interpreter(vict, to_force);
    }
  } else if (!str_cmp("room", arg)) {
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced room %d to %s", GET_NAME(ch), world[ch->in_room].number, to_force);
    mudlog(buf, NRM, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);

    for (vict = world[ch->in_room].people; vict; vict = next_force) {
      next_force = vict->next_in_room;
      if (GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    send_to_char(OK, ch);
    sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
    mudlog(buf, NRM, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);

    for (i = descriptor_list; i; i = next_desc) {
      next_desc = i->next;

      if (i->connected || !(vict = i->character) || GET_LEVEL(vict) >= GET_LEVEL(ch))
	continue;
      act(buf1, TRUE, ch, NULL, vict, TO_VICT);
      command_interpreter(vict, to_force);
    }
  }
}



ACMD(do_wiznet)
{
  struct descriptor_data *d;
  char emote = FALSE;
  char any = FALSE;
  int level = LVL_IMMORT;

  skip_spaces(&argument);
  delete_doubledollar(argument);

  if (!*argument) {
    send_to_char("Usage: wiznet <text> | #<level> <text> | *<emotetext> |\r\n "
		 "       wiznet @<level> *<emotetext> | wiz @\r\n", ch);
    return;
  }
  switch (*argument) {
  case '*':
    emote = TRUE;
  case '#':
    one_argument(argument + 1, buf1);
    if (is_number(buf1)) {
      half_chop(argument+1, buf1, argument);
      level = MAX(atoi(buf1), LVL_IMMORT);
      if (level > GET_LEVEL(ch)) {
	send_to_char("You can't wizline above your own level.\r\n", ch);
	return;
      }
    } else if (emote)
      argument++;
    break;
  case '@':
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  (CAN_SEE(ch, d->character) || GET_LEVEL(ch) == LVL_IMPL)) {
	if (!any) {
	  sprintf(buf1, "Gods online:\r\n");
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s", buf1, GET_NAME(d->character));
	if (PLR_FLAGGED(d->character, PLR_WRITING))
	  sprintf(buf1, "%s (Writing)\r\n", buf1);
	else if (PLR_FLAGGED(d->character, PLR_MAILING))
	  sprintf(buf1, "%s (Writing mail)\r\n", buf1);
	else
	  sprintf(buf1, "%s\r\n", buf1);

      }
    }
    any = FALSE;
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && GET_LEVEL(d->character) >= LVL_IMMORT &&
	  PRF_FLAGGED(d->character, PRF_NOWIZ) &&
	  CAN_SEE(ch, d->character)) {
	if (!any) {
	  sprintf(buf1, "%sGods offline:\r\n", buf1);
	  any = TRUE;
	}
	sprintf(buf1, "%s  %s\r\n", buf1, GET_NAME(d->character));
      }
    }
    send_to_char(buf1, ch);
    return;
    break;
  case '\\':
    ++argument;
    break;
  default:
    break;
  }
  if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
    send_to_char("You are offline!\r\n", ch);
    return;
  }
  skip_spaces(&argument);

  if (!*argument) {
    send_to_char("Don't bother the gods like that!\r\n", ch);
    return;
  }
  if (level > LVL_IMMORT) {
    sprintf(buf1, "%s: <%d> %s%s\r\n", GET_NAME(ch), level,
	    emote ? "<--- " : "", argument);
    sprintf(buf2, "Someone: <%d> %s%s\r\n", level, emote ? "<--- " : "",
	    argument);
  } else {
    sprintf(buf1, "%s: %s%s\r\n", GET_NAME(ch), emote ? "<--- " : "",
	    argument);
    sprintf(buf2, "Someone: %s%s\r\n", emote ? "<--- " : "", argument);
  }

  for (d = descriptor_list; d; d = d->next) {
    if ((!d->connected) && (GET_LEVEL(d->character) >= level) &&
	(!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
	(!PLR_FLAGGED(d->character, PLR_MAILING) ||
	 !PLR_FLAGGED(d->character, PLR_WRITING))
	&& (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
      send_to_char(CCCYN(d->character, C_NRM), d->character);
      if (CAN_SEE(d->character, ch))
	send_to_char(buf1, d->character);
      else
	send_to_char(buf2, d->character);
      send_to_char(CCNRM(d->character, C_NRM), d->character);
    }
  }

  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(OK, ch);
}



ACMD(do_zreset)
{
  void reset_zone(int zone);

  int i, j;

  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("You must specify a zone.\r\n", ch);
    return;
  }
  if (*arg == '*') {
    for (i = 0; i <= top_of_zone_table; i++)
      reset_zone(i);
    send_to_char("Reset world.\r\n", ch);
    sprintf(buf, "(GC) %s reset entire world.", GET_NAME(ch));
    mudlog(buf, NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
    return;
  } else if (*arg == '.')
    i = world[ch->in_room].zone;
  else {
    j = atoi(arg);
    for (i = 0; i <= top_of_zone_table; i++)
      if (zone_table[i].number == j)
	break;
  }
  if (i >= 0 && i <= top_of_zone_table) {
    reset_zone(i);
    sprintf(buf, "Reset zone %d (#%d): %s.\r\n", i, zone_table[i].number,
	    zone_table[i].name);
    send_to_char(buf, ch);
    sprintf(buf, "(GC) %s reset zone %d (%s)", GET_NAME(ch), i, zone_table[i].name);
    mudlog(buf, NRM, MAX(LVL_BUILDER, GET_INVIS_LEV(ch)), TRUE);
  } else
    send_to_char("Invalid zone number.\r\n", ch);
}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
  struct char_data *vict;
  long result;
  int i = 0;

  one_argument(argument, arg);

  if (!*arg)
    send_to_char("Yes, but for whom?!?\r\n", ch);
  else if (!(vict = get_char_vis(ch, arg)))
    send_to_char("There is no such player.\r\n", ch);
  else if (IS_NPC(vict))
    send_to_char("You can't do that to a mob!\r\n", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    send_to_char("Hmmm...you'd better not.\r\n", ch);
  else {
    switch (subcmd) {
    case SCMD_PARDON:
      if (!PLR_FLAGGED(vict, PLR_THIEF) && !PLR_FLAGGED(vict, PLR_KILLER)) {
	send_to_char("Your victim is not flagged.\r\n", ch);
	return;
      }
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_THIEF);
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_KILLER);
      send_to_char("Pardoned.\r\n", ch);
      send_to_char("You have been pardoned by the Gods!\r\n", vict);
      sprintf(buf, "(GC) %s pardoned by %s", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_NOTITLE:
      result = PLR_TOG_CHK(vict, PLR_NOTITLE);
      sprintf(buf, "(GC) Notitle %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, NRM, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_SQUELCH:
      result = PLR_TOG_CHK(vict, PLR_NOSHOUT);
      sprintf(buf, "(GC) Squelch %s for %s by %s.", ONOFF(result),
	      GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
      break;
    case SCMD_FREEZE:
      if (ch == vict) {
	send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
	return;
      }
      if (PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Your victim is already pretty cold.\r\n", ch);
	return;
      }
      SET_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
      send_to_char("A bitter wind suddenly rises and drains every erg of heat from your body!\r\nYou feel frozen!\r\n", vict);
      send_to_char("Frozen.\r\n", ch);
      act("A sudden cold wind conjured from nowhere freezes $n!", FALSE, vict, 0, 0, TO_ROOM);
      sprintf(buf, "(GC) %s frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      break;
    case SCMD_THAW:
      if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
	send_to_char("Sorry, your victim is not morbidly encased in ice at the moment.\r\n", ch);
	return;
      }
      if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
	sprintf(buf, "Sorry, a level %d God froze %s... you can't unfreeze %s.\r\n",
	   GET_FREEZE_LEV(vict), GET_NAME(vict), HMHR(vict));
	send_to_char(buf, ch);
	return;
      }
      sprintf(buf, "(GC) %s un-frozen by %s.", GET_NAME(vict), GET_NAME(ch));
      mudlog(buf, BRF, MAX(LVL_ADMIN, GET_INVIS_LEV(ch)), TRUE);
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_FROZEN);
      send_to_char("A fireball suddenly explodes in front of you, melting the ice!\r\nYou feel thawed.\r\n", vict);
      send_to_char("Thawed.\r\n", ch);
      act("A sudden fireball conjured from nowhere thaws $n!", FALSE, vict, 0, 0, TO_ROOM);
      break;
    case SCMD_UNAFFECT:
		if (vict->char_specials.saved.affected_by)
		{
			for (i = 0;i < NUM_WEARS;i++)
			{
				if (GET_EQ(ch, i))
					obj_to_char(unequip_char(ch, i), ch);
			}
			for(i = 0; i <= 150;i++)
			{
				if (AFF_FLAGGED(ch, i))
				{
					REMOVE_BIT_AR(AFF_FLAGS(ch), i);
					affect_total(ch);
				}
				else
					continue;
			}
			send_to_char("There is a brief flash of light!\r\n"
				"You feel slightly different.\r\n", vict);
			send_to_char("All spells removed.\r\n", ch);
		} else {
			send_to_char("Your victim does not have any affections!\r\n", ch);
			return;
		}
	break;

   case SCMD_TOAD:
      if (ch == vict) {
        send_to_char("Oh, yeah, THAT'S real smart...\r\n", ch);
        return;
      }
      if (PLR_FLAGGED(vict, PLR_TOAD)) {
        send_to_char("Your victim is already hopping mad.\r\n", ch);
        return;
      }
      if(isname("Vedic", GET_NAME(vict))) {
        send_to_char("Kiss my hairy white ass.\r\n", ch);
      }
      SET_BIT_AR(PLR_FLAGS(vict), PLR_TOAD);
      send_to_char("You have pissed off the gods. Zap! You're a toad!\r\nYou have an odd longing for flies...\r\n", vict);
      send_to_char("Cool. You've got a toad.\r\n", ch);
      act("The gods are pissed off and have turned $n into a toad!", FALSE, vict, 0, 0, TO_ROOM);

      if (GET_LEVEL(ch) != LVL_IMPL) {
        sprintf(buf, "(GC) %s has achieved toadhood, courtesy of %s.", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
      }
        
      break;   
    case SCMD_TOADOFF:
      if (!PLR_FLAGGED(vict, PLR_TOAD)) {
        send_to_char("Sorry, your victim is not a toad at this moment...should he be...?\r\n", ch);
        return;
      }
      
      if (GET_LEVEL(ch) != LVL_IMPL) {
       sprintf(buf, "(GC) %s returned to original state by %s.", GET_NAME(vict), GET_NAME(ch));
        mudlog(buf, BRF, MAX(LVL_IMPL, GET_INVIS_LEV(ch)), TRUE);
      }
       
      
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_TOAD);
      send_to_char("The gods have shown you mercy, you wretch.\r\nYour opposable thumbs have returned.\r\n", vict);
      send_to_char("Weeeelllllll...alright. If you wish.\r\n", ch);
      act("$n stops hopping around and returns to human form!", FALSE, vict, 0, 0, TO_ROOM);
      break;

    default:
      log("SYSERR: Unknown subcmd passed to do_wizutil (act.wizard.c)");
      break;
    }
    save_char(vict, NOWHERE);
  }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, int zone)
{
/* REMOVED 11/3/99 to make it shorter.  I think 
         this is overflowing "buf".*/
  sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name,
	  zone_table[zone].age, zone_table[zone].lifespan,
	  zone_table[zone].reset_mode, zone_table[zone].top);
/* 
  sprintf(bufptr, "%s%3d %-30.30s\r\n",
	  bufptr, zone_table[zone].number, zone_table[zone].name);*/
}


/*  Added by Storm for show f <arg>   */

void print_one_record(FILE *filen, sh_int search_for, struct char_data *ch) {
  char input;
  int /* bool */ found = 0;
  int vn = 0;  /* temp var for intermediate records in the file. */
  char buf[MAX_INPUT_LENGTH];

  while (!found) {
    fscanf(filen, "%c", &input);
    if (input == '#') {
      fscanf(filen, "%d", &vn);
      if (vn == search_for) found = 1;
    } else if (input == '$') {
      send_to_char("That vnum does not exist.", ch);
      return;
    }
  }
  /* We should only get here if the search_for number has been found. */
  fscanf(filen, "%c", &input);
  sprintf(buf, "#%d\r\n", search_for);
  while ((input != '#') && (input != '$')) {
    if (input == '\n') {
      send_to_char(buf, ch);
      sprintf(buf, "%c", input);
    } else
      sprintf(buf, "%s%c", buf, input);
    fscanf(filen, "%c", &input);
  }
}

#define LOAD_CARDS		1
#define LOAD_LETTERS	2
#define LOAD_IMMORTEQ	3
ACMD(do_show)
{
  struct char_file_u vbuf;
  int i, j, k, l, con;
  char self = 0;
  struct char_data *vict;
  struct obj_data *obj;
  char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH], birth[80];
  extern char *class_abbrevs[];
  extern char *genders[];
  extern int buf_switches, buf_largecount, buf_overflows;
  void show_shops(struct char_data * ch, char *value);
  void hcontrol_list_houses(struct char_data *ch);
  FILE *showfile;
  extern int port;
  struct teleport_shop *tshop;
  struct teleport_dest *tdest;
  char buf5[MAX_STRING_LENGTH * 2];

  struct show_struct {
    char *cmd;
    char level;
  } fields[] = {
    { "nothing",	0  },				/* 0 */
    { "zones",		LVL_IMMORT },			/* 1 */
    { "player",		LVL_ADMIN },
    { "rent",		LVL_ADMIN },
    { "stats",		LVL_IMMORT },
    { "errors",		LVL_BUILDER },			/* 5 */
    { "death",		LVL_ADMIN },
    { "godrooms",	LVL_ADMIN },
    { "shops",		LVL_IMMORT },
    { "houses",		LVL_ADMIN },
    { "f", 		LVL_BUILDER },
    { "development",	LVL_BUILDER },
    { "arena",          LVL_QUESTOR },
    { "builders",       LVL_BUILDER_H },
    { "teleporters",    LVL_BUILDER_H },
    { "\n", 0 }
  };


  *buf5 ='\0';

  skip_spaces(&argument);

  if (!*argument) {
    strcpy(buf, "Show options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	sprintf(buf, "%s%-15s%s", buf, fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;

  if (GET_LEVEL(ch) < fields[l].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return;
  }
  if (!strcmp(value, "."))
    self = 1;
  buf[0] = '\0';
  switch (l) {
  case 1:			/* zone */
    /* tightened up by JE 4/6/93 */
    if (self)
      print_zone_to_buf(buf5, world[ch->in_room].zone);
    else if (*value && is_number(value)) {
      for (j = atoi(value), i = 0; zone_table[i].number != j && i <= top_of_zone_table; i++);
      if (i <= top_of_zone_table)
	print_zone_to_buf(buf5, i);
      else {
	send_to_char("That is not a valid zone.\r\n", ch);
	return;
      }
    } else
      for (i = 0; i <= top_of_zone_table; i++)
	print_zone_to_buf(buf5, i);
    page_string(ch->desc, buf5, TRUE);
    break;
  case 2:			/* player */
    if (!*value) {
      send_to_char("A name would help.\r\n", ch);
      return;
    }

    if (load_char(value, &vbuf) < 0) {
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
    sprintf(buf, "Player: %-12s (%s) [%2d %s]\r\n", vbuf.name,
      genders[(int) vbuf.sex], vbuf.level, class_abbrevs[(int) vbuf.class]);
    sprintf(buf,
	 "%sAu: %-8d  Bal: %-8d  Exp: %-8d  Align: %-5d  Lessons: %-3d\r\n",
	    buf, vbuf.points.gold, vbuf.points.bank_gold, vbuf.points.exp,
	    vbuf.char_specials_saved.alignment,
	    vbuf.player_specials_saved.spells_to_learn);
    strcpy(birth, ctime(&vbuf.birth));
    sprintf(buf,
	    "%sStarted: %-20.16s  Last: %-20.16s  Played: %3dh %2dm\r\n",
	    buf, birth, ctime(&vbuf.last_logon), (int) (vbuf.played / 3600),
	    (int) (vbuf.played / 60 % 60));
    send_to_char(buf, ch);
    break;
  case 3:
    Crash_listrent(ch, value);
    break;
  case 4:
    i = 0;
    j = 0;
    k = 0;
    con = 0;
    for (vict = character_list; vict; vict = vict->next) {
      if (IS_NPC(vict))
	j++;
      else if (CAN_SEE(ch, vict)) {
	i++;
	if (vict->desc)
	  con++;
      }
    }
    for (obj = object_list; obj; obj = obj->next)
      k++;
    sprintf(buf, "Current stats /cb(port %d)/c0:\r\n", port);
    sprintf(buf, "%s  %5d players in game  %5d connected\r\n", buf, i, con);
    sprintf(buf, "%s  %5d registered\r\n", buf, top_of_p_table + 1);
    sprintf(buf, "%s  %5d mobiles          %5d prototypes\r\n",
	    buf, j, top_of_mobt + 1);
    sprintf(buf, "%s  %5d objects          %5d prototypes\r\n",
	    buf, k, top_of_objt + 1);
    sprintf(buf, "%s  %5d rooms            %5d zones\r\n",
	    buf, top_of_world + 1, top_of_zone_table + 1);
    sprintf(buf, "%s  %5d large bufs\r\n", buf, buf_largecount);
    sprintf(buf, "%s  %5d buf switches     %5d overflows\r\n", buf,
	    buf_switches, buf_overflows);
	    sprintf(buf, "%s  %5d tips.\r\n", buf, tip_count);
	sprintf(buf, "%s  %5d cards loading.   %5d letters loading.\r\n", buf, special_loading(LOAD_CARDS), special_loading(LOAD_LETTERS));
	sprintf(buf, "%s  %5d immortequipment loading.\r\n", buf, special_loading(LOAD_IMMORTEQ));
    send_to_char(buf, ch);
    break;
  case 5:
    strcpy(buf, "Errant Rooms\r\n------------\r\n");
    for (i = 0, k = 0; i <= top_of_world; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
	if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
	  sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++k, world[i].number,
		  world[i].name);
    send_to_char(buf, ch);
    break;
  case 6:
    strcpy(buf, "Death Traps\r\n-----------\r\n");
    for (i = 0, j = 0; i <= top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_DEATH))
	sprintf(buf, "%s%2d: [%5d] %s\r\n", buf, ++j,
		world[i].number, world[i].name);
    send_to_char(buf, ch);
    break;
  case 7:
    strcpy(buf, "Godrooms\r\n--------------------------\r\n");
    for (i = 0, j = 0; i < top_of_world; i++)
    if (ROOM_FLAGGED(i, ROOM_GODROOM))
      sprintf(buf,"%s%2d: [%5d] %s\r\n",buf,++j,world[i].number,world[i].name);
    send_to_char(buf, ch);
    break;
  case 8:
    show_shops(ch, value);
    break;
  case 9:
    hcontrol_list_houses(ch);
    break;
  case 10:
    j = atoi(arg);
    if (!strcmp(value, "mob")) {
      for (i = 0; i <= top_of_zone_table; i++)
        if ((j >= (zone_table[i].number * 100)) &&
            (j <= zone_table[i].top))
           break;

      sprintf(field, "%d.mob", zone_table[i].number);
      sprintf(buf2, "world/mob/%s", field);
      if (!(showfile = fopen(buf2, "r"))) {
        sprintf(buf1, "Error opening obj file '%s'", buf2);
        perror(buf1);
        return;
      }
      print_one_record(showfile, j, ch);
    } else if (!strcmp(value, "obj")) {
      for (i = 0; i <= top_of_zone_table; i++){
        if ((j >= (zone_table[i].number * 100)) &&
            (j <= (zone_table[i].top)))
           break;
      }
      sprintf(field, "%d.obj", zone_table[i].number);
      sprintf(buf2, "world/obj/%s", field);
      if (!(showfile = fopen(buf2, "r"))) {
        sprintf(buf1, "Error opening mob file '%s'", buf2);
        perror(buf1);
        return;
      }
      print_one_record(showfile, j, ch);
    }
    break;
  case 11: /* development */
    send_to_char("Portal-proof zones:\r\n------------------------\r\n", ch);
    for (i = 0; i <= top_of_zone_table; i++){
      if (zone_table[i].development){
        sprintf(buf, "%s%3d - %s\r\n",
                     buf, zone_table[i].number, zone_table[i].name);
      }
    }
    if (strlen(buf))
      send_to_char(buf, ch);
    else
      send_to_char("-none-\r\n", ch);
    break;
  case 12:      /* arena */
    if (in_arena == ARENA_OFF) {
        sprintf(buf, "The Arena is closed right now.\r\n");
    } else if (in_arena == ARENA_START) {
        sprintf(buf, "Arena will start in %d hour(s)\r\n", time_to_start);
        sprintf(buf, "%sIt will last for %d hour(s)\r\n", buf, game_length);
    } else if (in_arena == ARENA_RUNNING) {
        sprintf(buf, "Arena will end in %d hour(s)\r\n", time_left_in_game);
    }
    send_to_char(buf, ch);
    break;
  case 13:      /* builders */
    send_to_char("Under construction.  Try later.\r\n", ch);
    break;

  case 14:
    for (tshop = teleport_shops; tshop; tshop = tshop->next) {
      if (real_mobile((int)tshop->operator)>0) {
        sprintf(buf, "\r\nTeleport operator: %s\r\n", 
            mob_proto[real_mobile((int)tshop->operator)].player.short_descr);
        sprintf(buf, "%s           Action: %s\r\n"
                    "   Destination(s):\r\n",buf1, tactions[tshop->action]);
        send_to_char(buf, ch);
        for (tdest = tshop->destinations; tdest; tdest = tdest->next) {
          sprintf(buf, "  Room %hd for %d gold coins.\r\n",
                        tdest->dest, tdest->gold_cost);
          send_to_char(buf, ch);
        }
      } else {
        sprintf(buf1, "Unknown teleport operator! (%hd)\r\n", tshop->operator);
        send_to_char(buf, ch);
      }
    }
    break;

  default:
    send_to_char("Sorry, I don't understand that.\r\n", ch);
    break;


  }
}


/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC	0
#define BINARY	1
#define NUMBER	2

#define SET_OR_REMOVE(flagset, flags) { \
	if (on) SET_BIT_AR(flagset, flags); \
	else if (off) REMOVE_BIT_AR(flagset, flags); }

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


/* The set options available */
  struct set_struct {
    char *cmd;
    char level;
    char pcnpc;
    char type;
  } set_fields[] = {
   { "brief",		LVL_ADMIN, 	PC, 	BINARY },  /* 0 */
   { "invstart", 	LVL_ADMIN, 	PC, 	BINARY },  /* 1 */
   { "title",		LVL_ADMIN, 	PC, 	MISC },
   { "nosummon", 	LVL_BUILDER, 	PC, 	BINARY },
   { "maxhit",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "maxmana", 	LVL_BUILDER, 	BOTH, 	NUMBER },  /* 5 */
   { "maxmove", 	LVL_BUILDER, 	BOTH, 	NUMBER },
   { "hit", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "mana",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "move",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "align",		LVL_ADMIN, 	BOTH, 	NUMBER },  /* 10 */
   { "str",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "stradd",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "int", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "wis", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "dex", 		LVL_BUILDER, 	BOTH, 	NUMBER },  /* 15 */
   { "con", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "cha",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "ac", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "gold",		LVL_ADMIN, 	BOTH, 	NUMBER },
   { "bank",		LVL_ADMIN, 	PC, 	NUMBER },  /* 20 */
   { "exp", 		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "hitroll", 	LVL_BUILDER, 	BOTH, 	NUMBER },
   { "damroll", 	LVL_BUILDER, 	BOTH, 	NUMBER },
   { "invis",		LVL_IMPL, 	PC, 	NUMBER },
   { "nohassle", 	LVL_BUILDER, 	PC, 	BINARY },  /* 25 */
   { "frozen",		LVL_FREEZE, 	PC, 	BINARY },
   { "practices", 	LVL_BUILDER, 	PC, 	NUMBER },
   { "lessons", 	LVL_BUILDER, 	PC, 	NUMBER },
   { "drunk",		LVL_BUILDER, 	BOTH, 	MISC },
   { "hunger",		LVL_BUILDER, 	BOTH, 	MISC },    /* 30 */
   { "thirst",		LVL_BUILDER, 	BOTH, 	MISC },
   { "killer",		LVL_ADMIN, 	PC, 	BINARY },
   { "thief",		LVL_ADMIN, 	PC, 	BINARY },
   { "level",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "room",		LVL_IMPL, 	BOTH, 	NUMBER },  /* 35 */
   { "roomflag", 	LVL_BUILDER, 	PC, 	BINARY },
   { "siteok",		LVL_BUILDER, 	PC, 	BINARY },
   { "deleted", 	LVL_IMPL, 	PC, 	BINARY },
   { "class",		LVL_IMPL, 	BOTH, 	MISC },
   { "nowizlist", 	LVL_ADMIN, 	PC, 	BINARY },  /* 40 */
   { "quest",		LVL_ADMIN, 	PC, 	BINARY },
   { "loadroom", 	LVL_BUILDER, 	PC, 	MISC },
   { "color",		LVL_ADMIN, 	PC, 	BINARY },
   { "idnum",		LVL_IMPL, 	PC, 	NUMBER },
   { "passwd",		LVL_IMPL, 	PC, 	MISC },    /* 45 */
   { "nodelete", 	LVL_ADMIN, 	PC, 	BINARY },
   { "sex", 		LVL_IMPL, 	BOTH, 	MISC },
   { "olc",		LVL_BUILDER_H,	PC,	NUMBER },
   { "hometown",        LVL_ADMIN,      PC,     NUMBER },
   { "department", 	LVL_CODER, 	PC, 	MISC },    /* 50 */
   { "race",            LVL_IMPL, 	PC, 	MISC },
   { "luck",		LVL_BUILDER, 	BOTH, 	NUMBER },
   { "private",		LVL_ADMIN, 	PC, 	NUMBER },
   { "clan",		LVL_IMPL, 	PC, 	MISC },
   { "spellpower", 	LVL_CODER, 	BOTH, 	NUMBER }, /* 55 */
   { "qi",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "maxqi",		LVL_IMPL, 	BOTH, 	NUMBER },
   { "vim",     	LVL_IMPL, 	BOTH, 	NUMBER },
   { "maxvim",	        LVL_IMPL, 	BOTH, 	NUMBER },
   { "aria",     	LVL_IMPL, 	BOTH, 	NUMBER }, /* 60 */
   { "maxaria",	        LVL_IMPL, 	BOTH, 	NUMBER }, 
   { "rank",            LVL_ADMIN,      PC,     NUMBER },
   { "remort",          LVL_IMPL,       PC,     NUMBER },
   { "keeprank",        LVL_IMPL,       PC,     NUMBER },
   { "killnums",        LVL_IMPL,       PC,     NUMBER },
   { "multi",           LVL_ADMIN,        PC,     BINARY },
   { "carry",           LVL_IMPL,       PC,     NUMBER },
   { "clanexile",           158,       PC,     NUMBER },
   { "\n", 0, BOTH, MISC }
  };


int perform_set(struct char_data *ch, struct char_data *vict, int mode,
		char *val_arg)
{
  int i, on = 0, off = 0, value = 0;
  sh_int clan_num;
  char output[MAX_STRING_LENGTH];
  int parse_class(char arg);
  int parse_race(char *arg);

  /* Check to make sure all the levels are correct */
  if (GET_LEVEL(ch) != LVL_IMPL) {
    if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && vict != ch) {
      send_to_char("Maybe that's not such a great idea...\r\n", ch);
      return 0;
    }
  }
  if (GET_LEVEL(ch) < set_fields[mode].level) {
    send_to_char("You are not godly enough for that!\r\n", ch);
    return 0;
  }

  /* Make sure the PC/NPC is correct */
  if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
    send_to_char("You can't do that to a beast!\r\n", ch);
    return 0;
  } else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
    send_to_char("That can only be done to a beast!\r\n", ch);
    return 0;
  }

  /* Find the value of the argument */
  if (set_fields[mode].type == BINARY) {
    if (!strcmp(val_arg, "on") || !strcmp(val_arg, "yes"))
      on = 1;
    else if (!strcmp(val_arg, "off") || !strcmp(val_arg, "no"))
      off = 1;
    if (!(on || off)) {
      send_to_char("Value must be 'on' or 'off'.\r\n", ch);
      return 0;
    }
    sprintf(output, "%s %s for %s.", set_fields[mode].cmd, ONOFF(on),
	    GET_NAME(vict));
  } else if (set_fields[mode].type == NUMBER) {
    value = atoi(val_arg);
    sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
	    set_fields[mode].cmd, value);
    sprintf(buf, "(GC) %s has SET %s's %s to %d.",
                  GET_NAME(ch), GET_NAME(vict), set_fields[mode].cmd, value);
    mudlog(buf, BRF, LVL_IMMORT, TRUE);  
  } else {
    strcpy(output, "Okay.");  /* can't use OK macro here 'cause of \r\n */
  }

  switch (mode) {
  case 0:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_BRIEF);
    break;
  case 1:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_INVSTART);
    break;
  case 2:
    set_title(vict, val_arg);
    sprintf(output, "%s's title is now: %s", GET_NAME(vict), GET_TITLE(vict));
    break;
  case 3:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_SUMMONABLE);
    sprintf(output, "Nosummon %s for %s.\r\n", ONOFF(!on), GET_NAME(vict));
    break;
  case 4:
    vict->points.max_hit = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 5:
    vict->points.max_mana = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 6:
    vict->points.max_move = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 7:
    vict->points.hit = RANGE(-9, vict->points.max_hit);
    affect_total(vict);
    break;
  case 8:
    vict->points.mana = RANGE(0, vict->points.max_mana);
    affect_total(vict);
    break;
  case 9:
    vict->points.move = RANGE(0, vict->points.max_move);
    affect_total(vict);
    break;
  case 10:
    GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
    affect_total(vict);
    break;
  case 11:
    RANGE(3, 25);
    vict->real_stats.str = value;
    vict->real_stats.str_add = 0;
    affect_total(vict);
    break;
  case 12:
    vict->real_stats.str_add = RANGE(0, 100);
    if (value > 0)
      vict->real_stats.str = 25;
    affect_total(vict);
    break;
  case 13:
    RANGE(3, 25);
    vict->real_stats.intel = value;
    affect_total(vict);
    break;
  case 14:
    RANGE(3, 25);
    vict->real_stats.wis = value;
    affect_total(vict);
    break;
  case 15:
    RANGE(3, 25);
    vict->real_stats.dex = value;
    affect_total(vict);
    break;
  case 16:
    RANGE(3, 25);
    vict->real_stats.con = value;
    affect_total(vict);
    break;
  case 17:
    RANGE(3, 25);
    vict->real_stats.cha = value;
    affect_total(vict);
    break;
  case 18:
    vict->points.armor = RANGE(-100, 100);
    affect_total(vict);
    break;
  case 19:
    GET_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 20:
    GET_BANK_GOLD(vict) = RANGE(0, 100000000);
    break;
  case 21:
    vict->points.exp = RANGE(0, 2000000000);
    break;
  case 22:
    vict->points.hitroll = RANGE(-50, 500);
    affect_total(vict);
    break;
  case 23:
    vict->points.damroll = RANGE(-50, 500);
    affect_total(vict);
    break;
  case 48:
    GET_OLC_ZONE(vict) = value;
    break;
  case 24:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    GET_INVIS_LEV(vict) = RANGE(0, GET_LEVEL(vict));
    break;
  case 25:
    if (GET_LEVEL(ch) < LVL_IMPL && ch != vict) {
      send_to_char("You aren't godly enough for that!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_NOHASSLE);
    break;
  case 26:
    if (ch == vict) {
      send_to_char("Better not -- could be a long winter!\r\n", ch);
      return 0;
    }
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_FROZEN);
    break;
  case 27:
  case 28:
    GET_PRACTICES(vict) = RANGE(0, 100);
    break;
  case 29:
  case 30:
  case 31:
    if (!str_cmp(val_arg, "off")) {
      GET_COND(vict, (mode - 29)) = (char) -1; /* warning: magic number here */
      sprintf(output, "%s's %s now off.", GET_NAME(vict), set_fields[mode].cmd);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      RANGE(0, 24);
      GET_COND(vict, (mode - 29)) = (char) value; /* and here too */
      sprintf(output, "%s's %s set to %d.", GET_NAME(vict),
	      set_fields[mode].cmd, value);
    } else {
      send_to_char("Must be 'off' or a value from 0 to 24.\r\n", ch);
      return 0;
    }
    break;
  case 32:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_KILLER);
    break;
  case 33:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_THIEF);
    break;
  case 34:
    if (value > GET_LEVEL(ch) || value > LVL_IMPL) {
      send_to_char("You can't do that.\r\n", ch);
      return 0;
    }
    RANGE(0, LVL_IMPL);
    vict->player.level = (byte) value;
    break;
  case 35:
    if ((i = real_room(value)) < 0) {
      send_to_char("No room exists with that number.\r\n", ch);
      return 0;
    }
    char_from_room(vict);
    char_to_room(vict, i);
    break;
  case 36:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_ROOMFLAGS);
    break;
  case 37:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_SITEOK);
    break;
  case 38:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_DELETED);
    break;
  case 39:
    if ((i = parse_class(*val_arg)) == CLASS_UNDEFINED) {
      send_to_char("That is not a class.\r\n", ch);
      return 0;
    }
    GET_CLASS(vict) = i;
    break;
  case 40:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NOWIZLIST);
    break;
  case 41:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_QUEST);
    break;
  case 42:
    if (!str_cmp(val_arg, "off")) {
      REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
    } else if (is_number(val_arg)) {
      value = atoi(val_arg);
      if (real_room(value) != NOWHERE) {
        SET_BIT_AR(PLR_FLAGS(vict), PLR_LOADROOM);
	GET_LOADROOM(vict) = value;
	sprintf(output, "%s will enter at room #%d.", GET_NAME(vict),
		GET_LOADROOM(vict));
      } else {
	send_to_char("That room does not exist!\r\n", ch);
	return 0;
      }
    } else {
      send_to_char("Must be 'off' or a room's virtual number.\r\n", ch);
      return 0;
    }
    break;
  case 43:
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_1);
    SET_OR_REMOVE(PRF_FLAGS(vict), PRF_COLOR_2);
    break;
  case 44:
    if (GET_IDNUM(ch) != 1 || !IS_NPC(vict))
      return 0;
    GET_IDNUM(vict) = value;
    break;
  case 45:
    if (GET_IDNUM(ch) > 1) {
      send_to_char("Please don't use this command, yet.\r\n", ch);
      return 0;
    }
    if (GET_LEVEL(vict) >= LVL_BUILDER) {
      send_to_char("You cannot change that.\r\n", ch);
      return 0;
    }
    strncpy(GET_PASSWD(vict), CRYPT(val_arg, GET_NAME(vict)), MAX_PWD_LENGTH);
    *(GET_PASSWD(vict) + MAX_PWD_LENGTH) = '\0';
    sprintf(output, "Password changed to '%s'.", val_arg);
    break;
  case 46:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_NODELETE);
    break;
  case 47:
    if (!str_cmp(val_arg, "male"))
      vict->player.sex = SEX_MALE;
    else if (!str_cmp(val_arg, "female"))
      vict->player.sex = SEX_FEMALE;
    else if (!str_cmp(val_arg, "neutral"))
      vict->player.sex = SEX_NEUTRAL;
    else {
      send_to_char("Must be 'male', 'female', or 'neutral'.\r\n", ch);
      return 0;
    }
    break;
   case 49:
     RANGE(0, NUM_STARTROOMS-1);
     GET_HOME(vict) = value;
     send_to_char("  NUM  ROOM  ZONE\r\n", ch);
     for (i=0; i<NUM_STARTROOMS; i++) {
       sprintf(output, " %2d)  %-5d  %s\r\n", i, load_rooms[i], hometowns[i]);
       send_to_char(output, ch);
     }
     sprintf(output, "%ss hometown set to %s.", GET_NAME(vict),
             hometowns[GET_HOME(vict)]);
     break;

  case 50:
    if (!strncmp(val_arg, "imm", 3)) 
      GET_GOD_TYPE(vict) = GOD_ALL;
    else if (is_abbrev(val_arg, "coder"))
      GET_GOD_TYPE(vict) = GOD_CODER;
     else if (is_abbrev(val_arg, "builder"))
      GET_GOD_TYPE(vict) = GOD_BUILDER;
     else if (is_abbrev(val_arg, "administrator"))
      GET_GOD_TYPE(vict) = GOD_ADMIN;
    else if (is_abbrev(val_arg, "questor"))
      GET_GOD_TYPE(vict) = GOD_QUESTOR;
    else if (is_abbrev(val_arg, "implementor"))
      GET_GOD_TYPE(vict) = GOD_IMPL;
    else 
      send_to_char("That department does not currently exist.\r\n", ch);
    break;
  case 51:
    if ((i = parse_race(val_arg)) == RACE_UNDEFINED) {
      send_to_char("That is not a race.\r\n", ch);
      return 0;
    }
    GET_RACE(vict) = i;
    break;
  case 52:
    RANGE(3, 25);
    vict->real_stats.luck = value;
    affect_total(vict);
    break;
  case 53:
    GET_PRIVATE(vict) = RANGE(0, 32000);
    break;
  case 54:
    if ((clan_num=find_clan(val_arg))!=-1) {
      GET_CLAN(vict) = clan[clan_num].id;
      GET_CLAN_RANK(vict)=1;
    } else {
      if (isname(val_arg, "off")) {
        GET_CLAN(vict)=0;
        GET_CLAN_RANK(vict)=0;
      } else {
        send_to_char("That doesn't look like a clan name.\r\nTry CLAN INFO "
                     "for a list of clans or SET <char> CLAN OFF to\r\n"
                     "remove a player from their clan.\r\n", ch);
        return 0;
      }
    }
    save_clans();
    break;
  case 55:
    GET_SPELLPOWER(vict) = RANGE(0, 200);
    affect_total(vict);
    break;
  case 56:
    vict->points.qi = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 57:
    vict->points.max_qi = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 58:
    vict->points.vim = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 59:
    vict->points.max_vim = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 60:
    vict->points.max_aria = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 61:
    vict->points.max_aria = RANGE(1, 5000);
    affect_total(vict);
    break;
  case 62:
   GET_RANK(vict) = RANGE(0, MAX_RANK - 1);
   break;
  case 63:
   GET_CLASS(vict) = RANGE(9, NUM_CLASSES -1);
   break;
  case 64:
if(value == 1)
 SET_BIT_AR(PLR_FLAGS(vict), PLR_KEEPRANK);
else
REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_KEEPRANK);
  case 65:
   GET_NUM_KILLS(vict) = RANGE(1, 100000);
   break;
  case 66:
    SET_OR_REMOVE(PLR_FLAGS(vict), PLR_MULTI_OK);
    break;
  break;
  case 67:
    IS_CARRYING_W(vict) = value;
    break;
  case 68:
	  GET_CLAN(vict)=0;
	  GET_CLAN_RANK(vict)=0;
	  GET_EXP(vict) -= 50000000;
	  SET_BIT_AR(PLR_FLAGS(vict), PLR_EXILE);
      send_to_char("You have exiled %s from their clan.\r\n", ch);
	  save_clans();
	  break;
  default:
    send_to_char("Can't set that!\r\n", ch);
    return 0;
    break;
  }

  strcat(output, "\r\n");
  send_to_char(CAP(output), ch);
  return 1;
}


ACMD(do_set)
{
  struct char_data *vict = NULL, *cbuf = NULL;
  struct char_file_u tmp_store;
  char field[MAX_INPUT_LENGTH], name[MAX_INPUT_LENGTH],
	val_arg[MAX_INPUT_LENGTH];
  int mode = -1, len = 0, player_i = 0, retval;
  char is_file = 0, is_mob = 0, is_player = 0;

  half_chop(argument, name, buf);

  if (!strcmp(name, "file")) {
    is_file = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "player")) {
    is_player = 1;
    half_chop(buf, name, buf);
  } else if (!str_cmp(name, "mob")) {
    is_mob = 1;
    half_chop(buf, name, buf);
  }
  half_chop(buf, field, buf);
  strcpy(val_arg, buf);

  if (!*name || !*field) {
    send_to_char("Usage: set <victim> <field> <value>\r\n", ch);
    return;
  }

  /* find the target */
  if (!is_file) {
    if (is_player) {
      if (!(vict = get_player_vis(ch, name, 0))) {
	send_to_char("There is no such player.\r\n", ch);
	return;
      }
    } else {
      if (!(vict = get_char_vis(ch, name))) {
	send_to_char("There is no such creature.\r\n", ch);
	return;
      }
    }
  } else if (is_file) {
    /* try to load the player off disk */
    CREATE(cbuf, struct char_data, 1);
    clear_char(cbuf);
    if ((player_i = load_char(name, &tmp_store)) > -1) {
      store_to_char(&tmp_store, cbuf);
      if (GET_LEVEL(cbuf) >= GET_LEVEL(ch) && GET_IDNUM(ch) > 2 &&
          GET_IDNUM(ch) != 1127 && GET_IDNUM(ch) != 158) {
	free_char(cbuf);
	send_to_char("Sorry, you can't do that.\r\n", ch);
	return;
      }
      vict = cbuf;
    } else {
      free(cbuf);
      send_to_char("There is no such player.\r\n", ch);
      return;
    }
  }

  /* find the command in the list */
  len = strlen(field);
  for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
    if (!strncmp(field, set_fields[mode].cmd, len))
      break;

  /* perform the set */
  retval = perform_set(ch, vict, mode, val_arg);

  /* save the character if a change was made */
  if (retval) {
    if (!is_file && !IS_NPC(vict))
      save_char(vict, NOWHERE);
    if (is_file) {
      char_to_store(vict, &tmp_store);
      fseek(player_fl, (player_i) * sizeof(struct char_file_u), SEEK_SET);
      fwrite(&tmp_store, sizeof(struct char_file_u), 1, player_fl);
      send_to_char("Saved in file.\r\n", ch);
    }
  }

  /* free the memory if we allocated it earlier */
  if (is_file)
    free_char(cbuf);
}



/* Healing Flux.  Heals all the PC's */
ACMD(do_flux)
{
  struct char_data *character;
  char msg[256];
  int heal_count=0;

 
  sprintf(buf, "(GC) Healing FLUX by %s.", GET_NAME(ch));
  log(buf);

  skip_spaces(&argument);
  for (character = character_list; character; character = character->next) {
    if (!IS_NPC(character)){
      /*  Heal the player. */
      if (GET_POS(character)==POS_STUNNED) GET_POS(character)=POS_RESTING;
      if (GET_POS(character)==POS_INCAP) GET_POS(character)=POS_RESTING;
      GET_HIT(character)  = GET_MAX_HIT(character);
      GET_MANA(character) = GET_MAX_MANA(character);
      GET_MOVE(character) = GET_MAX_MOVE(character);
      GET_QI(character)   = GET_MAX_QI(character);
      GET_VIM(character)  = GET_MAX_VIM(character);
      GET_ARIA(character) = GET_MAX_ARIA(character);
      if (!*argument)
       if(isname("Vedic", GET_NAME(ch))) {
          send_to_char("\r\n/cRA wee turtle begins gnawing at your leg!/c0/c0", character);
      }
      else if(isname("Kaan", GET_NAME(ch))) {
          send_to_char("\r\n/cWA blaze of /cYgolden light/cW shines down from the heavens, washing away all your pains./c0/c0", character);
 
      }
      else if(isname("Inindo", GET_NAME(ch))) {
          send_to_char("\r\n/cgFrom the heavens, a wind reeking of/cg pot smoke/cg blasts into you.\r\n/cyInindo has... healed you.\r\n\r\n\r\n/cCIndeed.\r\n/c0/c0", character);
      }
      else {
          send_to_char("\r\nA /cWchilling/c0 breeze touches your spine.", character);
      }
      else {
        strcat(argument, "/c0");
        send_to_char(argument, character);
      }
      send_to_char("\r\nYou are fully healed!\r\n", character);
      if(!CAN_SEE(ch, character)) continue;
      heal_count++;
    }
  }
  sprintf(msg, "Okay.  You healed %d player(s).\r\n", heal_count);
  send_to_char(msg, ch);
  
}
  
  
ACMD(do_bitchslap)
{
  
  struct char_data *vict;
  
  one_argument(argument, arg);
  
  if (!*arg){
    send_to_char("Who do you want to bitchslap?\r\n", ch);
    return;
  }
  
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char("You don't see that here.\r\n", ch);
    return;
  }
  if (isname(GET_NAME(vict), "Vedic")) {
    act("$N says, '/cwNo, fuck YOU!/c0', as $E turns your bitchslap around, slapping you instead!\r\n", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    act("You give a smile and smirk as $n attempts to bitchslap you.\r\n", FALSE, ch, 0, vict, TO_VICT);
    act("$N gives a smile and smirk as $n attempts to bitchslap $M.\r\n", FALSE, ch, 0, vict, TO_NOTVICT);
    GET_HIT(ch) = -1;
    GET_POS(ch) = POS_INCAP;
   } else

  act("You bitchslap $N.\r\n", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  act("$n bitchslaps you!\r\nYou are incapacitated.\r\n", FALSE, ch, 0, vict, TO_VICT);
  act("$N is bitchslapped by $n!\r\n", FALSE, ch, 0, vict, TO_NOTVICT);
  GET_HIT(vict) = -1; /* Fuck the vict up! */
  GET_POS(vict) = POS_INCAP;
  
}
  
extern struct zone_data *zone_table;
extern int real_zone(int number);
ACMD(do_rlist)
{
    
  int i;
  int zone_num;
  int room_num;
  int room_count=0;
  int curr_room;
  char tmp[128];
    
  one_argument(argument, arg);

  tmp[0]='\0';
    
  if (!*arg) 
    zone_num = (int)world[IN_ROOM(ch)].number/100;
  else  
    zone_num = atoi(arg);
    
  for (i = 0; i <= top_of_zone_table; i++)
    if (zone_table[i].number == zone_num) {
      
      if (i >= 0 && i <= top_of_zone_table) {
        sprintf(buf, "\r\nRoom List For Zone # %d: %s.\r\n\r\nRoom #    Name\r\n---------------------------------------------------------------\r\n", zone_table[i].number, zone_table[i].name);
        send_to_char(buf, ch);
        *buf='\0';
        for (curr_room = 0; curr_room <= top_of_world; curr_room++){
          room_num = world[curr_room].number;
          if (room_num >= (zone_num*100) && room_num <= ((zone_num*100)+99)){
            if (real_room(room_num)>NOWHERE){
    sprintbitarray(ROOM_FLAGS(real_room(room_num)), room_bits, RF_ARRAY_MAX, buf2);
              sprintf(tmp, " %-5d %s/c0 (%s)\r\n", room_num, world[curr_room].name, buf2);
              strcat(buf, tmp);
              room_count++;
            }
          }
        }
      sprintf(tmp, "\r\n%d rooms listed for zone %d\r\n\r\n", room_count, zone_num);
      strcat(buf, tmp);
      page_string(ch->desc, buf, TRUE);
    } else
      send_to_char("Zone not found.\r\n", ch);
  }
}
  
  
extern struct obj_data *obj_proto ;
ACMD(do_olist)
{
  
  int zone_num;
  int nr;
  int obj_count=0;
  int i;
  char tmp[128];
  
  one_argument(argument, arg);
  
  tmp[0]='\0';
  
  if (!*arg) 
    zone_num = (int)world[IN_ROOM(ch)].number/100;
  else
    zone_num = atoi(arg);
  
  for (i = 0; i <= top_of_zone_table; i++) {
    if (zone_table[i].number == zone_num) {
      if (i >= 0 && i <= top_of_zone_table) {
        sprintf(buf, "\r\nObject List For Zone # %d: %s.\r\n\r\n"
              " Obj #   Level Name\r\n"
              "-----------------------------------------------------\r\n",
              zone_table[i].number, zone_table[i].name);
        send_to_char(buf, ch);
        *buf='\0';
        for (nr = zone_num*100; nr<=((zone_num*100)+99); nr++) {
          if (real_object(nr)>0) {
            sprintf(tmp, " %-5d %5d   %s (%d) (Cost: %d)\r\n", nr,
            obj_proto[real_object(nr)].obj_flags.minlevel,
            obj_proto[real_object(nr)].short_description, 
            obj_proto[real_object(nr)].obj_flags.wear_flags[0],
            obj_proto[real_object(nr)].obj_flags.cost);
            strcat(buf, tmp);
            obj_count++;
          } else {
            /* sprintf(tmp, " /cB%-5d Unused./c0\r\n", nr); */
            /* strcat(buf, tmp); */
          }
        }
      }
    }
  }
  sprintf(tmp, "-----------\r\n  %d slots used.\r\n\r\n", obj_count);
  strcat(buf, tmp);
  page_string(ch->desc, buf, TRUE);
}
  
ACMD(do_mlist)
{
  
  int zone_num;
  int mob_count=0;
  int i, nr;
  char tmp[128];
  
  one_argument(argument, arg);
  
  tmp[0]='\0';
  if (!*arg) 
    zone_num = (int)world[IN_ROOM(ch)].number/100;
  else
    zone_num = atoi(arg);
  
  for (i = 0; i <= top_of_zone_table; i++) {
    if (zone_table[i].number == zone_num) {
      if (i >= 0 && i <= top_of_zone_table) {
        sprintf(buf, "\r\nMobile List For Zone # %d: %s.\r\n\r\n MOB #  Level  Name\r\n---------------------------------------------------------------\r\n", zone_table[i].number, zone_table[i].name);
        send_to_char(buf, ch);
        *buf='\0';
        for (nr = zone_num*100; nr<=((zone_num*100)+99); nr++) {
          if (real_mobile(nr)>0) {
            sprintf(tmp, " %-5d %5d   %s\r\n", nr,
                         mob_proto[real_mobile(nr)].player.level,
                         mob_proto[real_mobile(nr)].player.short_descr);
            strcat(buf, tmp);
            mob_count++;
          } else {
            /* sprintf(tmp, " /cB%-5d Unused./c0\r\n", nr); */
            /* strcat(buf, tmp); */
          }
        }
      }
    }
  }
  sprintf(tmp, "-----------\r\n  %d slots used.\r\n", mob_count);
  strcat(buf, tmp);
  page_string(ch->desc, buf, TRUE);
}


/*
#define S_NUM(i)		((i)->virtual)
#define S_KEEPER(i)		((i)->keeper)
#define S_ROOM(i, num)		((i)->in_room[(num)])
ACMD(do_slist)
{
  
  int zone_num, i, rshop, shp_count=0;
  char tmp[128];
  struct shop_data *shop;
  
  one_argument(argument, arg);
  
  tmp[0]='\0';
  if (!*arg) {
    send_to_char("Specify a zone number.\r\n", ch);
    return;
  }
  
  zone_num = atoi(arg);
  
  for (i = 0; i <= top_of_zone_table; i++) {
    if (zone_table[i].number == zone_num) {
      if (i >= 0 && i <= top_of_zone_table) {
        sprintf(buf, "\r\nShop List For Zone # %d: %s.\r\n\r\n"
            " SHOP #    MOBILE    ROOM \r\n"
            "--------------------------\r\n",
            zone_table[i].number, zone_table[i].name);
        send_to_char(buf, ch);
        *buf='\0';
        for (i = zone_num * 100; i <= zone_table[zone_num].top; i++) {
          if ((rshop = real_shop(i)) != -1) {
            shop = shop_index[rshop];
            sprintf(tmp, " %-5d     %-5d     %-5d\r\n",
                         S_NUM(shop),
                         mob_index[S_KEEPER(shop)].virtual,
                         S_ROOM(shop, 0));
            strcat(buf, tmp);
            shp_count++;
          }
        }
      }
    }
  }
  sprintf(tmp, "-----------\r\n  %d slots used.\r\n", shp_count);
  strcat(buf, tmp);
  page_string(ch->desc, buf, TRUE);
}
*/


void do_newbie(struct char_data *vict)
{
  extern int port;
  struct obj_data *obj;
  int i, weapon, map, w;
  int newbie_eq[] = {3081, 3086, 1009, 1041, 1044, 1065, 1031,
                    3077, 3078, 3079, 3047, 3027, 3028, 3029, 3019, -1};
  
  /*  Give some basic eq to this person (vict) */
  for (i = 0; newbie_eq[i] != -1; i++) {
    obj = read_object(newbie_eq[i], VIRTUAL);
    obj_to_char(obj, vict);
  }
  
  /*  Give a weapon specific to the class... */
  switch GET_CLASS(vict){
  case CLASS_SORCERER:
  case CLASS_CLERIC:
    weapon = 1024; /* Mace */
    break;
  default:
    weapon = 3020; /* Dagger */
    break;
  }
  obj = read_object(weapon, VIRTUAL);
  obj_to_char(obj, vict);
  
  /*  Give the poor schmuck some change. */
  GET_GOLD(vict) = number(250, 320);

  if (port!=DFLT_PORT)
    return; /* Don't worry about the map unless the real game port */


  switch(GET_HOME(vict)) {
    case HOME_MIDGAARD:     map = 1244; break;
/*    case HOME_JARETH:       map = 1243; break;
    case HOME_MCGINTEY:     map = 1245; break;
    case HOME_SILVERTHORNE: map = 1240; break;
    case HOME_ELVENCITY:    map = 1242; break;
    case HOME_OFINGIA:      map = 1241; break;
    case HOME_MALATHAR:     map = 1246; break; */
    default: map = 1244; break;
  }
  obj = read_object(map, VIRTUAL);
  obj_to_char(obj, vict);

  for (w = 0; w < NUM_WEARS; w++) {
    if(!GET_EQ(vict, w)) {
      for (obj = vict->carrying; obj; obj = obj-> next_content) {
        if (GET_OBJ_TYPE(obj) == ITEM_ARMOR || GET_OBJ_TYPE(obj) == ITEM_WORN) {
          if(find_eq_pos(vict, obj, 0) == w) {
            obj_from_char(obj);
            equip_char(vict, obj, w);
          }
        }
      }
    }
  }

}

ACMD(do_olcsave)
{
	int save_nr=0,olc_save=0;
	if (!*argument) {
		send_to_char("Save what zone?\r\n", ch);
		return;
	} else {
		save_nr = atoi(argument);
		if ((olc_save = real_zone(save_nr)) == -1) {
			send_to_char("Sorry, there is no zone for that number!\r\n", ch);
			return;
		} else {
			   redit_save_to_disk(olc_save);
			   zedit_save_to_disk(olc_save);
			   oedit_save_to_disk(olc_save);
			   medit_save_to_disk(olc_save);
			   sedit_save_to_disk(olc_save);
			   sprintf(buf, "(GC) %s saved all olc information.", GET_NAME(ch));
			   mudlog(buf, BRF, MAX(154, GET_INVIS_LEV(ch)), TRUE);
			   return;
		}
	} 
}

ACMD(do_dupe)
{
  FILE *pfile;
  int pnum=0, plr_host=0, playing=0;
//  int playing_multi=0;
  struct char_file_u vbuf;
  struct char_file_u vtbuf;
  struct descriptor_data *d=0;
  char buf[MAX_STRING_LENGTH*4];
  bool found = FALSE;
  char status[MAX_STRING_LENGTH];
  char multiplaying[MAX_STRING_LENGTH];
  char yes_no[MAX_STRING_LENGTH];
  //char char_multi[MAX_STRING_LENGTH];
  if (!(pfile = fopen(PLAYER_FILE, "r+b"))) {
	  send_to_char("Error, can not open player files.\r\n", ch);
    return;
  }
  skip_spaces(&argument);
  if (!*argument) {
	  send_to_char("/ccCheck what character for existing dupes?/c0\r\n", ch);
	  return;
  }
  if (load_char(argument, &vtbuf) < 0) {
    send_to_char("/ccThere is no such player./c0\r\n", ch);
    return;
  }
  if (vtbuf.host)
  //*char_multi = '\0';
  sprintf(buf, "/ccChecking for existing dupes of /cy%s:/c0\r\n", vtbuf.name);
  send_to_char(buf, ch);
  fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
  fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  for(pnum=1;!feof(pfile);pnum++) {
	  if (!IS_SET_AR(vbuf.char_specials_saved.act, PLR_DELETED)) {
		  if (vbuf.host && vtbuf.host) {
			  if (!strcmp(vbuf.host, vtbuf.host)) {
				  sprintf(status, "/crOffline/c0");
				  for (d = descriptor_list; d; d = d->next) {
					  if (d->connected >= CON_PLAYING) {
						  if (!strcmp(GET_NAME(d->character), vbuf.name)) {
							  sprintf(status, "/cROnline/c0");
							  //sprintf(char_multi + strlen(char_multi), "/cR%s/cr(/cc%s/cr)/c0  ", vbuf.name, connected_types[d->connected]);
							  playing++;
							  //if (d->connected == CON_PLAYING) {
	  							  //send_to_char("Found a playing character connected...\r\n", ch);
								  //playing_multi++;
							  //}
						  }
					  }
				  }
				  if ((GET_LEVEL(ch) >= 1) || (vbuf.last_logon > 0))
					  sprintf(buf, "/cy%-20.20s:/cc %-3d %-3.3s %-3.3s  %-13s  /cyLast login: /cc%s/c0", vbuf.name, vbuf.level, race_abbrevs[(int) vbuf.race], class_abbrevs[(int) vbuf.class], status, ctime(&vbuf.last_logon));
				  else if (!vbuf.last_logon)
					  sprintf(buf, "/cy%-20.20s:/cc %-3d %-3.3s %-3.3s  %-13s  /cyLast login: /cc-/c0", vbuf.name, vbuf.level, race_abbrevs[(int) vbuf.race], class_abbrevs[(int) vbuf.class], status);
				  send_to_char(buf, ch);
				  found = TRUE;
			  }
		  }
	  }
	  fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
	  fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  }
  fclose(pfile);
  if (found) {
	  if (playing) {
		  //sprintf(yes_no, "/cRYes/cc,/cy %s/cc./cy Total:/cc %d/cy./c0", char_multi, playing);
		  sprintf(yes_no, "/cRYes/cc./cy Total:/cc %d/cy./c0", playing);
		  sprintf(multiplaying, "/crNo/c0");
		  if (playing > 1) {
			  //sprintf(yes_no, "/cRYes/cc,/cy %s/cc./cy Total:/cc %d/cy./c0", char_multi, playing);
			  sprintf(yes_no, "/cRYes/cc./cy Total:/cc %d/cy./c0", playing);
		//	  if (playing_multi)
		//		  sprintf(multiplaying, "/cRYes/cc,/cC %d/cy characters/c0", playing_multi);
			  sprintf(multiplaying, "/cRYes, %d characters/c0", playing);
		  }
	  } else {
		  sprintf(multiplaying, "/crNo/c0");
		  sprintf(yes_no, "/crNo/c0");
	  }
	                sprintf(buf, "/ccHost          :/cy %-20s/c0\r\n", vtbuf.host);
	  sprintf(buf + strlen(buf), "/ccTotal Players :/cy %-3d/c0\r\n", plr_host);
	  sprintf(buf + strlen(buf), "/ccOnline        :/cy %-s/c0\r\n", yes_no);
	  sprintf(buf + strlen(buf), "/ccMultiplaying  :/cy %-s/c0\r\n", multiplaying);
	  send_to_char(buf, ch);
	  return;
  } 
  else if (!found) {
	  send_to_char("/ccNo dupes found./c0\r\n", ch);
	  return;
  }
  return;
}


ACMD(do_new_dupe)
{
	FILE *pfile;
	struct char_file_u vbuf;
	struct char_file_u vtbuf;
	struct descriptor_data *d;
	char output[MAX_STRING_LENGTH];
	char status[MAX_STRING_LENGTH];
    char multi_str[MAX_STRING_LENGTH];
	int pnum = 0, nr_chars[2];
	bool found = FALSE;

	if (!(pfile = fopen(PLAYER_FILE, "r+b")))
		return;
	skip_spaces(&argument);
	if (!*argument)
	{
		send_to_char("Usage: dupe name | ip.\r\n", ch);
		return;
	}
	else if (isdigit(*argument))
	{
		fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
		fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
		for (pnum = 1; !feof(pfile); pnum++)
		{
			if (IS_SET_AR(vbuf.char_specials_saved.act, PLR_DELETED))
					continue;
			else if (vbuf.level < 1)
				continue;
			else if (vbuf.host <= 0)
				continue;
			else if (vbuf.last_logon <= 0)
				continue;
			else if (is_abbrev(argument, vbuf.host))
			{
				sprintf(status, "/crOffline/c0");
				nr_chars[0]++;
				for (d = descriptor_list; d; d = d->next)
				{
					if (d->connected >= CON_PLAYING)
						if (!strcmp(GET_NAME(d->character), vbuf.name))
						{
							nr_chars[1]++;
							sprintf(status, "/cROnline/c0");
						}
				}
				sprintf(output, "%s/cy%-20.20s:/cc %-3d %-3.3s %-3.3s  %-13s  /cyLast login: /cc%s/c0\r\n", output, vbuf.name, vbuf.level, race_abbrevs[(int) vbuf.race], class_abbrevs[(int) vbuf.class], status, ctime(&vbuf.last_logon));				
				found = TRUE;
			}
			fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
			fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
		}
		fclose(pfile);
	}
	else if (!isdigit(*argument))
	{
		if (load_char(argument, &vtbuf) < 0) {
			send_to_char("/ccThere is no such player./c0\r\n", ch);
			return;
		}
		else {
			fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
			fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
			for (pnum = 1; !feof(pfile); pnum++)
			{
				if (IS_SET_AR(vbuf.char_specials_saved.act, PLR_DELETED))
					continue;
				if (IS_SET_AR(vtbuf.char_specials_saved.act, PLR_DELETED))
					continue;
				else if (vbuf.level < 1)
					continue;
				else if (vbuf.host <= 0)
					continue;
				else if (vbuf.last_logon <= 0)
					continue;
				else if (vtbuf.host <= 0)
					continue;
				else if (vtbuf.level <= 0)
					continue;
				else if (vtbuf.last_logon <= 0)
					continue;
				else if (!strcmp(vtbuf.host, vbuf.host))
				{
					sprintf(status, "/crOffline/c0");
					nr_chars[0]++;
					for (d = descriptor_list; d; d = d->next)
					{
					  if (d->connected >= CON_PLAYING)
						  if (!strcmp(GET_NAME(d->character), vbuf.name))
						  {
							  nr_chars[1]++;
							  sprintf(status, "/cROnline/c0");
						  }
					}
					sprintf(output, "%s/cy%-20.20s:/cc %-3d %-3.3s %-3.3s  %-13s  /cyLast login: /cc%s/c0\r\n", output, vbuf.name, vbuf.level, race_abbrevs[(int) vbuf.race], class_abbrevs[(int) vbuf.class], status, ctime(&vbuf.last_logon));				
					found = TRUE;
				}
				fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
				fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
			}
			fclose(pfile);
		}
	} else {
		send_to_char("No such player.\r\n", ch);
	}
	if (found == TRUE)
	{
		send_to_char("/cyThe following characters were found:\r\n/c0", ch);
		if (nr_chars[1] > 1)
			sprintf(multi_str, "/cRYes/c0");
		else
			sprintf(multi_str, "/crNo/c0");
		sprintf(output, "%s/ccHost           :/cy %-20s/c0\r\n", output, vtbuf.host);
		sprintf(output, "%s/ccTotal Players  :/cy %-3d/c0\r\n", output, nr_chars[0]);
		sprintf(output, "%s/ccOnline Players :/cy %-3d/c0\r\n", output, nr_chars[1]);
		sprintf(output, "%s/ccMultiplaying   :/cy %-s/c0\r\n", output, multi_str);
		send_to_char(output, ch);
		return;
	} else {
		send_to_char("/ccNo dupes found./c0\r\n", ch);
		return;
	}
}

ACMD(do_plist)
{

  FILE *pfile;
  int pnum=0, minlvl=0, cnt=0;
  bool hostname = FALSE, level = FALSE;
  struct char_file_u vbuf;
  char buf[MAX_STRING_LENGTH*4];

  if (!(pfile = fopen(PLAYER_FILE, "r+b"))) {
    return;
  }

  skip_spaces(&argument);
  if (*argument) {
    if (!(minlvl = atoi(argument))) hostname = TRUE;
    else level=TRUE;
  }

  sprintf(buf, "/cbPlayers:\r\n------------------------------------/c0\r\n");
  fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
  fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  for(pnum=1;!feof(pfile);pnum++) {
    if (level) { /* If a level was specified *//* Storm 1/31/99 */
     if (vbuf.level >= minlvl) {
      sprintf(buf, "%s/cg%-5ld:/cy[%3d/cy]/c0 %-13s/cw %-26s - %s\r",
                  buf, vbuf.char_specials_saved.idnum,vbuf.level,vbuf.name, 
                  vbuf.host,ctime(&vbuf.last_logon));
      cnt++;
     }
    } else if (hostname) { /* If a string was specified *//* Storm 1/31/99 */
     if (strstr(vbuf.host, argument)) {
      sprintf(buf, "%s/cg%-5ld:/cy[%3d/cy]/c0 %-13s/cw %-26s - %s\r",
                  buf, vbuf.char_specials_saved.idnum,vbuf.level,vbuf.name, 
                  vbuf.host,ctime(&vbuf.last_logon));
      cnt++;
     }
    } else {/* if nothing was specified just list everyone.. *//* Storm 1/31/99 */
      sprintf(buf, "%s/cg%-5ld:/cy[%3d/cy]/c0 %-13s/cw %-26s - %s\r",
                  buf, vbuf.char_specials_saved.idnum,vbuf.level,vbuf.name, 
                  vbuf.host,ctime(&vbuf.last_logon));
      cnt++;
    }
    fseek(pfile, (long) (pnum * sizeof(struct char_file_u)), SEEK_SET);
    fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  }

  sprintf(buf, "%s%d players in the database (%d listed).\r\n", buf,
          pnum-1, cnt);
  fclose(pfile);
  page_string(ch->desc, buf, TRUE);

}


ACMD(do_file)
{
  FILE *req_file;
  int cur_line = 0,
      num_lines = 0,
      req_lines = 0,
      i, 
      j;  
  int l;
  char field[MAX_INPUT_LENGTH], 
       value[MAX_INPUT_LENGTH];

  struct file_struct {
    char *cmd;
    char level;
    char *file;
  } fields[] = {
    { "none",           LVL_IMPL,    "Does Nothing" },     
    { "bug",	        LVL_CODER,   "..//lib//misc//bugs"},   
    { "typo",		LVL_BUILDER, "..//lib//misc//typos"},  
    { "ideas",		LVL_CODER,   "..//lib//misc//ideas"},
    { "xnames",		LVL_IMPL,    "..//lib//misc//xnames"},
    { "syslog",         LVL_ADMIN,   "..//syslog" }, 
    { "crash",          LVL_ADMIN,   "..//syslog.CRASH" }, 
    { "levels",         LVL_ADMIN,   "..//log//levels" }, 
    { "rip",            LVL_ADMIN,   "..//log//rip" }, 
    { "players",        LVL_ADMIN,   "..//log//newplayers" },
    { "rentgone",       LVL_ADMIN,   "..//log//rentgone" }, 
    { "godcmds",        LVL_IMPL,    "..//log//godcmds" }, 
    { "olc",            LVL_BUILDER_H,"..//log//olc" },  
    { "flux",           LVL_IMPL,    "..//log//flux" },  
    { "cheaters",       LVL_IMPL,    "..//log//cheaters" }, 
    { "quest",          LVL_ADMIN,   "..//log//quest" },
    { "thief",           LVL_ADMIN,   "..//log//thief" },
    { "clan",		LVL_ADMIN,    "..//log//clan" }, 
    { "errors",         LVL_ADMIN,    "..//log//errors" },
    { "death",           LVL_ADMIN,    "..//log//death" },
    { "\n", 0, "\n" }
  };

  skip_spaces(&argument);

  if (!*argument) 
  {
    strcpy(buf, "USAGE: file <option> <num lines>\r\n\r\nFile options:\r\n");
    for (j = 0, i = 1; fields[i].level; i++)
      if (fields[i].level <= GET_LEVEL(ch))
	sprintf(buf, "%s%-15s%s\r\n", buf, fields[i].cmd, fields[i].file);
    // send_to_char(buf, ch);
    SEND_TO_Q(buf, ch->desc);
    return;
  }

  strcpy(arg, two_arguments(argument, field, value));

  for (l = 0; *(fields[l].cmd) != '\n'; l++)
    if (!strncmp(field, fields[l].cmd, strlen(field)))
      break;
      
  if(*(fields[l].cmd) == '\n')
  {
    send_to_char("That is not a valid option!\r\n", ch);
    return;
  }     

  if (GET_LEVEL(ch) < fields[l].level) 
  {
    send_to_char("You are not godly enough to view that file!\r\n", ch);
    return;
  }
 
  if(!*value)
     req_lines = 15; /* default is the last 15 lines */
  else
     req_lines = atoi(value);

  req_lines = 100;

  /* open the requested file */
  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "SYSERR: Error opening file %s using 'file' command.", 
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }

  /* count lines in requested file */
  get_line(req_file,buf);
  while (!feof(req_file))
  {
     num_lines++;
     get_line(req_file,buf);
  }
  fclose(req_file);
  
  
  /* Limit # of lines printed to # requested or # of lines in file or
     150 lines */
  if(req_lines > num_lines) req_lines = num_lines;
  if(req_lines > 150) req_lines = 150;


  /* close and re-open */
  if (!(req_file=fopen(fields[l].file,"r")))
  {
     sprintf(buf2, "SYSERR: Error opening file %s using 'file' command.", 
             fields[l].file);
     mudlog(buf2, BRF, LVL_IMPL, TRUE);
     return;
  }
  
  buf2[0] = '\0';

  /* and print the requested lines */
  get_line(req_file,buf);
  while (!feof(req_file))
  {
     cur_line++;
     if(cur_line > (num_lines - req_lines))
     {
        sprintf(buf2,"%s%s\r\n",buf2, buf);
     }
     get_line(req_file,buf);
   }
   page_string(ch->desc, buf2, 1);
   
   
   fclose(req_file);
}

ACMD(do_peace) {

void clearMemory(struct char_data * ch);

  struct char_data *vict, *next_v;

  act ("/cYAs $n makes a strange arcane gesture, a golden light descends\r\n"
       "from the heavens stopping all the fighting./c0\r\n",
       FALSE, ch, 0, 0, TO_ROOM);
  send_to_room("Everything is quite peaceful now.\r\n", ch->in_room);
  for(vict=world[ch->in_room].people; vict; vict=next_v) {
   next_v = vict->next_in_room;
   if (FIGHTING(vict))
     stop_fighting(vict);
   if (IS_NPC(vict))
     clearMemory(vict);
   HUNTING(vict)=0;
  }
}

ACMD(do_assign)
{
  struct char_data *mob = NULL;
  struct obj_data *obj = NULL;
  struct room_data *room = NULL;
  char name[MAX_INPUT_LENGTH];
  int r_num, number, num;
  char *Usage = "Usage: spec_assign < room | mob | obj > <name of target | vnum> <spec_proc>";

  half_chop(argument, name, buf); /* find out what type of spec_proc to add */

  if (!*name)                               /* make sure the args are correct */
    send_to_char(Usage, ch);
  else {
    if (is_abbrev(name, "list")) {
      list_spec_procs(ch);
      return;
    } else if (is_abbrev(name, "room")) {                /* room type of spec_proc */
      two_arguments(buf, name, buf2);
      if (!isdigit(*name)) {                    /* more input checking...  */
        send_to_char(Usage, ch);
        return;
      } else if ((number = atoi(name)) < 0) {  /* even more input checking  */
        send_to_char("You must use a positive vnum.", ch);
        return;
      }
      if (!*buf2) {                    /* and check for the spec_proc name */
        list_spec_procs(ch);
        return;
      }
      if ((r_num = real_room(number)) < 0) {      /* make sure room exists. */
        send_to_char("There is no room with that number.\r\n", ch);
        return;
      }
      room = &world[r_num];                         /* get room data... */
      if ((room->func = get_spec_proc(buf2, 2))) {            /* set new spec_proc. */
        sprintf(buf, "Room number %d's Spec Proc has been changed to %s",
                room->number, get_spec_name(room->func));
        send_to_char(buf, ch);
      } else {
        sprintf(buf, "Room number %d's Spec Proc has not been changed, spec_proc %s does not exist.\r\n", 
                room->number, buf2);
        send_to_char(buf, ch);
      }
    } else if (is_abbrev(name, "obj")) {          /* ok, obj type spec_proc.  */
      two_arguments(buf, name, buf2);
      if (!*name) {                     /*  more input checking  */
        send_to_char(Usage, ch);
        return;
      } else if ((num = atoi(name))) {
        if (num < 0) {
          send_to_char("There are no negative objects.\r\n\r\n", ch);
          return; 
        }
        if (!(num = real_object(num))) {
          send_to_char("That is not a valid vnum!\r\n", ch);
          return;
        }
        obj = read_object(num, REAL); 
      } else if (!*buf2) {
        list_spec_procs(ch);
        return;
      } else {
       if (!(obj = get_obj_vis(ch, name))) {        /* Make sure obj exists  */
         send_to_char("That object does not exist.\r\n\r\n", ch);
         return;
       }
      }
      if (GET_OBJ_RNUM(obj) >= 0) {          /* Obj is real and permanent */
        if (!(obj_index[GET_OBJ_RNUM(obj)].func = get_spec_proc(buf2, 1))) 
        { 
          sprintf(buf, "Object number %d's Spec Proc has not been changed, spec_proc %s does not exist.\r\n",
                   GET_OBJ_VNUM(obj), buf2);
          send_to_char(buf, ch);
        } else {
          sprintf(buf, "Object number %d's Spec Proc has been changed to %s",
                  GET_OBJ_VNUM(obj),
                  get_spec_name(obj_index[GET_OBJ_RNUM(obj)].func));
          send_to_char(buf, ch);
        }
      } else {
        send_to_char("That obj cannot have a spec proc.", ch);
      }
    } else if (!strcmp(name, "mob")) {
      two_arguments(buf, name, buf2);
      if (!*name) {                     /*  more input checking  */
        send_to_char(Usage, ch);
        return;
      } else if (!*buf2) {
        list_spec_procs(ch);
        return;
      } else if ((num = atoi(name))) {
        if (num < 0) {
          send_to_char("There are no negative mobs.\r\n\r\n", ch);
          return;
        }
        if (!(num = real_mobile(num))) {
          send_to_char("There is no mob with that number.\r\n\r\n", ch);
          return;
        }
        mob = read_mobile(num, REAL); 
      } else {
       if (!(mob = get_char_vis(ch, name))) {
         send_to_char("That mobile does not exits.\r\n\r\n", ch);
         return;
       }
      }
      if (GET_MOB_RNUM(mob) >= 0) {
        if (!(mob_index[GET_MOB_RNUM(mob)].func = get_spec_proc(buf2, 0)))  {
          sprintf(buf, "Mobile number %d's Spec Proc has not been changed.  Spec_proc %s does not exist.\r\n",
                  GET_MOB_VNUM(mob), buf2);
          send_to_char(buf, ch);
        } else {
          sprintf(buf, "Mobile number %d's Spec Proc has been changed to %s",
                  GET_MOB_VNUM(mob),
                  get_spec_name(mob_index[GET_MOB_RNUM(mob)].func));
          send_to_char(buf, ch);
        }
      } else
          send_to_char("That mob cannot have a spec proc.", ch);
      }
    }
  }


ACMD(do_omni) {

  struct char_data *tch;
  char tmp[256];
  int count = 0;

  send_to_char("Lvl  Class  Room   Name                "
               "Health  Position\r\n", ch);
  for (tch = character_list; tch; tch = tch->next)
    if (!IS_NPC(tch) && GET_LEVEL(tch) < LVL_IMMORT) {
      sprintf(tmp, "%3d  %4s    %-5d  %-20s  %3d%%  %s\r\n",
              GET_LEVEL(tch), CLASS_ABBR(tch), world[IN_ROOM(tch)].number,
              GET_NAME(tch), (int)(GET_HIT(tch)*100)/GET_MAX_HIT(tch),
              position_types[(int)GET_POS(tch)]);
      send_to_char(tmp, ch);
      count++;
    }
  if (!count)
    send_to_char("  No mortals are on!\r\n", ch);

}


ACMD(do_lag)
{

  struct char_data *vict;

  one_argument(argument, buf);

   if (!(vict = get_char_room_vis(ch, buf))) {
    send_to_char("Lag who?\r\n", ch);
    return;
  }

  if (vict == ch) {
    send_to_char("Okay, you asked for it.\r\n", ch);
    return;
  }

  if (GET_LEVEL(vict) >= GET_LEVEL(ch)) {
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
    return;
  }

  send_to_char("Okay.\r\n", ch);
  send_to_char("The evil lag monster walks into the room and gives you a big hug!\r\n", vict);
  WAIT_STATE(vict, PULSE_VIOLENCE * 10);


}

char *abil_name(byte type, int num) {

  switch(type) {
  case ABT_SKILL:
    return skills[num];
    break;
  case ABT_SPELL:
    return spells[num];
    break;
  case ABT_CHANT:
    return chants[num];
    break;
  case ABT_PRAYER:
    return prayers[num];
    break;
  case ABT_SONG:
    return songs[num];
    break;
  }
  return NULL;
}

ACMD(do_abilstat) {

  int i;
  struct char_data *victim;

  one_argument(argument, buf);
  if (!*buf) {
    send_to_char("For whom?\r\n", ch);
    return;
  }
  if (!(victim = get_char_vis(ch, buf))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if (IS_NPC(victim)) {
    send_to_char("Not for mobs.\r\n", ch);
    return;
  }

  strcpy(buf2, "Type    Ability              Value\r\n"
               "----------------------------------\r\n");
  for (i = 0; i < MAX_ABILITIES; i++) {
    if (victim->player_specials->abilities[i].pct)
      sprintf(buf2, "%s%-7s %-20s %d\r\n", buf2,
              prac_types[(int)victim->player_specials->abilities[i].type],
              abil_name(victim->player_specials->abilities[i].type,
                        victim->player_specials->abilities[i].num),
              victim->player_specials->abilities[i].pct);
  }
  sprintf(buf2, "%s%s has %d practice%s remaining.\r\n", buf2,
          GET_NAME(victim), GET_PRACTICES(victim),
          GET_PRACTICES(victim) != 1 ? "s" : "");
  page_string(ch->desc, buf2, TRUE);

}

ACMD(do_chown)
{
  struct char_data *victim;
  struct obj_data *obj;
  char buf2[80];
  char buf3[80];
  int i, k = 0;

  two_arguments(argument, buf2, buf3);

  if (!*buf2)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else if (!(victim = get_char_vis(ch, buf3)))
    send_to_char("No one by that name here.\r\n", ch);
  else if (victim == ch)
    send_to_char("Are you sure you're feeling ok?\r\n", ch);
  else if (GET_LEVEL(victim) >= GET_LEVEL(ch))
    send_to_char("That's really not such a good idea.\r\n", ch);
  else if (!*buf3)
    send_to_char("Syntax: chown <object> <character>.\r\n", ch);
  else {
    for (i = 0; i < NUM_WEARS; i++) {
      if (GET_EQ(victim, i) && CAN_SEE_OBJ(ch, GET_EQ(victim, i)) &&
         isname(buf2, GET_EQ(victim, i)->name)) {
        obj_to_char(unequip_char(victim, i), victim);
        k = 1;
      }
    }

  if (!(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
    if (!k && !(obj = get_obj_in_list_vis(victim, buf2, victim->carrying))) {
      sprintf(buf, "%s does not appear to have the %s.\r\n", GET_NAME(victim),
buf2);
      send_to_char(buf, ch);
      return;
    }
  }

  act("/cW$n makes a slight gesture and $p /cWflies from $N to $m./c0",FALSE,ch,obj
,
       victim,TO_NOTVICT);
  act("/cW$n makes a slight gesture and $p /cWflies away from you to $m./c0",FALSE,
ch,
       obj,victim,TO_VICT);
  act("/cWYou make a slight gesture and $p /cWflies away from $N to you./c0",FALSE,
ch,
       obj, victim,TO_CHAR);

  obj_from_char(obj);
  obj_to_char(obj, ch);
  save_char(ch, NOWHERE);
  save_char(victim, NOWHERE);
  }

}

ACMD(do_addprac)
{
  struct char_data *victim;
  char *name = arg, *amnt = buf2;
  int prac_amnt;

  two_arguments(argument, name, amnt);

  if (!*name) {
    send_to_char("Who do you want to add practices to?\r\n", ch);
    return;
  }

  if (!(victim = get_char_vis(ch, name))) {
    send_to_char("Doesn't look like they are on right now...\r\n", ch);
    return;
  } 

  if (!*amnt) {
    send_to_char("How many practices do you want to add?\r\n", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("You can't add practices to mobs.\r\n", ch);
    return;
  }

  if ((prac_amnt = atoi(amnt)) <= 0) {
    send_to_char("The amount must be more than 0!\r\n"
                 "Syntax is: addprac <char> <amnt>\r\n", ch);
    return;
  }

  GET_PRACTICES(victim) = GET_PRACTICES(victim) + prac_amnt;
  send_to_char("Practices added.\r\n", ch);
}


ACMD(do_addexp)
{
  struct char_data *victim;
  char *name = arg, *amnt = buf2;
  int exp;

  two_arguments(argument, name, amnt);

  if (!*name) {
    send_to_char("Who do you want to add exp to?\r\n", ch);
    return;
  }

  if (!(victim = get_char_vis(ch, name))) {
    send_to_char("Doesn't look like they are on right now...\r\n", ch);
    return;
  }
  
  if (!*amnt) {
    send_to_char("How much exp do you want to add?\r\n", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("You can't add experience to mobs.\r\n", ch);
    return;
  }

  if ((exp = atoi(amnt)) <= 0) {
    send_to_char("The amount must be more than 0!\r\n"
                 "Syntax is: addexp <char> <amnt>\r\n", ch);
    return;
  }

  GET_EXP(victim) = GET_EXP(victim) + exp;
  send_to_char("Experience added.\r\n", ch);
  act("You have just been granted additional experience.", 
       FALSE, ch, 0, 0, TO_VICT);
}


ACMD(do_addlag)
{
  struct char_data *victim;
  char *name = arg, *time = buf2;
  int lag; two_arguments(argument, name, time);

  if (!*name) {
    send_to_char("Add lag to who?\r\n", ch);
    return;
  }

  if (!(victim = get_char_vis(ch, name))) {
    send_to_char("It doesn't look like they're here.\r\n", ch);
    return;
  }

  if (!*time) {
    send_to_char("You need to specify how long to lag them.\r\n", ch);
    return;
    }

  if ((lag = atoi(time)) <= 0) {
    send_to_char("The time must be more than 0!\r\n"
                 "Syntax is: addlag <char> <amnt>\r\n", ch);
    return;
  }

  if (lag > 100) {
    send_to_char("There is a limit to cruel and unusual punishment\r\n", ch);
    return;
  }

  WAIT_STATE(victim, lag);
  send_to_char("Adding lag now.\r\n", ch);
  return;
}

#define IN_ZONE(zone, room)	(((100*zone) <= room) && (zone_table[real_zone(zone*100)].top >= room))

ACMD(do_links)
{
  int zonenum;
  int i,j,r, room;
  

  skip_spaces(&argument);
  half_chop(argument, buf1, buf);
  if (!argument || !*argument) {
    send_to_char("Syntax: links <zone_vnum> ('.' for zone you are standing in)\r\n", ch);
    return;
  }
  
  if (strcmp(buf1,".")==0) {
    zonenum = zone_table[world[ch->in_room].zone].number;
  }
  else zonenum = atoi(buf1);

  sprintf(buf,"Zone: /cM%d/cw is linked to the following zones:\r\n",zonenum);
  for (i = zonenum*100; i<= zone_table[real_zone(zonenum*100)].top; i++) {
    r = real_room(i);
    if (r != -1) {
      for (j = 0; j < NUM_OF_DIRS; j++) {
        if (world[r].dir_option[j]) {
          room = world[r].dir_option[j]->to_room;
          if (room != NOWHERE && (!IN_ZONE(zonenum, world[room].number)))
          sprintf(buf,"%s/cC%3d /cc%-28s/cw at /cr%5d/cw (/cm%-5s/cw) ---> /cg%5d/c0\r\n", buf, 
            zone_table[world[room].zone].number, 
            zone_table[world[room].zone].name, i, dirs[j], world[room].number);
        }
      }
    }
  }
  send_to_char(buf, ch);
}


ACMD(do_ihide)
{

      TOGGLE_BIT_AR(PRF_FLAGS(ch), PRF_IHIDE);
      send_to_char("So be it.\r\n", ch);
}

ACMD(do_imsg)
{
  char **msg;

  msg = &(IHIDE(ch));    

  skip_spaces(&argument);

  if (*msg)
    free(*msg);

  if (!*argument)
    *msg = NULL;
  else
    *msg = str_dup(argument);

  send_to_char(OK, ch);
}


ACMD(do_mobat)
{
  char command[MAX_INPUT_LENGTH];
  int location, original_loc;

  if(!IS_NPC(ch)) { send_to_char("You don't look like a mob to me.\r\n", ch); 
  return; }

  half_chop(argument, buf, command);
  if (!*buf) {
    send_to_char("You must supply a room number or a name.\r\n", ch);
    return;
  }

  if (!*command) {
    send_to_char("What do you want to do there?\r\n", ch);
    return;
  }

  if ((location = find_target_room(ch, buf)) < 0)
    return;

  /* a location has been found. */
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);

  command_interpreter(ch, command);

  /* check if the char is still there */
  if (ch->in_room == location) {
    char_from_room(ch);
    char_to_room(ch, original_loc);
  }
} 

ACMD(do_random)
{
  int i, random;
  
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Random up to what number?\r\n", ch);
    return;
  }
  i=atoi(arg);

  random = number(1, i);

  sprintf(buf, "You're number is %d.\r\n", random);
  send_to_char(buf, ch);
}

ACMD(do_iweather)
{
        int i;
 
	one_argument(argument, arg);

	if(!*arg) {
		sprintf(buf, "Current Pressure is %d\r\n", weather_info.pressure);	
		send_to_char(buf, ch);
		return;
	}

        i = atoi(arg);
	
	sprintf(buf, "Sky was %d\r\n", weather_info.sky);
	send_to_char(buf, ch);
        
	weather_info.sky = i;

	sprintf(buf, "Sky is now %d\r\n", weather_info.sky);
	send_to_char(buf, ch);

}
        
ACMD(do_immbless)
{
  struct char_data *vict;
  int bonus;
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Who do you want to bless?\r\n", ch);
    return;
  }
  
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char("Doesn't look like they are online.\r\n", ch);
    return;
  }

  if (IS_NPC(vict)) {
    send_to_char("Not on mobs.\r\n", ch);
    return;
  }

  if (AFF_FLAGGED(vict, AFF_IMMBLESSED)) {
    send_to_char("That player has already been blessed.\r\n", ch);
    return;
  }

  bonus = number(1, 3);

  if (bonus == 1) {

    call_magic(ch, vict, NULL, SPELL_SANCTUARY,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE);
    call_magic(ch, vict, NULL, SPELL_HASTE,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE);
  }
  if (bonus == 2) {
    call_magic(ch, vict, NULL, SPELL_BLAZEWARD,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE); 
    call_magic(ch, vict, NULL, SPELL_GROWTH,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE); 
  } 
  if (bonus == 3) {
    call_magic(ch, vict, NULL, SPELL_INDESTR_AURA,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE);
    call_magic(ch, vict, NULL, SPELL_MIST_FORM,
               ABT_SPELL, GET_LEVEL(ch), TYPE_SPELL, FALSE);
  } 
  sprintf(buf, "/cC%s has been declared as blessed by %s!/c0\r\n", 
          GET_NAME(vict), GET_NAME(ch));
  send_to_all(buf);
  return;
}

ACMD(do_doublexp)
{
  int amount;
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("You'll need to specify an amount. The norm is 10.\r\n", ch);
    return;
  }

  amount = atoi(arg);

  double_exp = amount;
  sprintf(buf, "/cR%s has declared double experience for the next %d minutes!/c0\r\n", GET_NAME(ch), double_exp);
  send_to_all(buf);
  return;
}

ACMD(do_objlist)
{
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("You need to specify which wear position to reload.\r\n"
                 "Valid options are:\r\n"
                 "face\r\neyes\r\near\r\nneck\r\nhead\r\narms\r\nshield\r\n"
                 "body\r\nabout\r\nwaist\r\nlegs\r\nfeet\r\nwrist\r\n"
                 "hands\r\nfinger\r\nthumb\r\nhold\r\nwield\r\nfloat\r\n", ch);
    return;
  }
  if (is_abbrev(arg, "face"))
     make_facelist_html();
  else if (is_abbrev(arg, "eyes"))
     make_eyelist_html();
  else if (is_abbrev(arg, "ear"))
     make_earlist_html();
  else if (is_abbrev(arg, "neck"))
     make_necklist_html();
  else if (is_abbrev(arg, "head"))
     make_headlist_html();
  else if (is_abbrev(arg, "arms"))
     make_armlist_html();
  else if (is_abbrev(arg, "body"))
     make_bodylist_html();
  else if (is_abbrev(arg, "about"))
     make_aboutlist_html();
  else if (is_abbrev(arg, "waist"))
     make_waistlist_html();
  else if (is_abbrev(arg, "legs"))
     make_leglist_html();
  else if (is_abbrev(arg, "feet"))
     make_feetlist_html();
  else if (is_abbrev(arg, "wrist"))
     make_wristlist_html();
  else if (is_abbrev(arg, "hands"))
     make_handlist_html();
  else if (is_abbrev(arg, "finger"))
     make_fingerlist_html();
  else if (is_abbrev(arg, "thumb"))
     make_thumblist_html();
  else if (is_abbrev(arg, "hold"))
     make_holdlist_html();
  else if (is_abbrev(arg, "wield"))
     make_wieldlist_html();
  else if (is_abbrev(arg, "float"))
     make_floatlist_html();
  else if (is_abbrev(arg, "shield"))
     make_shieldlist_html();
  else {
     send_to_char("That is not a valid wear position.\r\n"
                  "Valid options are:\r\n"
                  "face\r\neyes\r\near\r\nneck\r\nhead\r\narms\r\nshield\r\n"
                  "body\r\nabout\r\nwaist\r\nlegs\r\nfeet\r\nwrist\r\n"
                  "hands\r\nfinger\r\nthumb\r\nhold\r\nwield\r\nfloat\r\n", ch);
     return;
  }
  return;
}

ACMD(do_reimb)
{
  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Who do you want to bless?\r\n", ch);
    return;
  }

  if (get_char_vis(ch, arg)) {
    send_to_char("The character must completely log off.\r\n", ch);
    return;
  }

  reimb(arg);
  sprintf(buf, "You reimburse %s.", arg);
  send_to_char(buf, ch);
}

ACMD(do_enable)
{
  struct char_data *vict;
  one_argument(argument, arg);

  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char("Doesn't look like they are online.\r\n", ch);
    return;
  } 

  if (!ROOM_FLAGGED(ch->in_room, ROOM_COURT)) {
    send_to_char("You can only use this command in courtrooms.\r\n", ch);
    return;
  }
  if (!IS_AFFECTED(vict, AFF_SPEAK)) {
    sprintf(buf, "You allow %s to speak.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    send_to_char("You have been allowed to speak.\r\n", vict);
    SET_BIT_AR(AFF_FLAGS(vict), AFF_SPEAK);
    return;
  } if (IS_AFFECTED(vict, AFF_SPEAK)) {
    sprintf(buf, "You silence %s.\r\n", GET_NAME(vict));
    send_to_char(buf, ch);
    send_to_char("You have been silenced for a short time.\r\n", vict); 
    REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_SPEAK);
    return;
  }
  send_to_char("There is no one in the room by that name.\r\n", ch);
  return;
}

ACMD(do_passwd)
{
  struct char_file_u vbuf;

  skip_spaces(&argument);
  if (load_char(argument, &vbuf) < 0) {
    send_to_char("There is no such player.\r\n", ch);
    return;
  }
  if (vbuf.level >= LVL_IMMORT) {
    send_to_char("That's none of your damn business.\r\n", ch);
    return;
  }
  if (GET_IDNUM(ch) != 1 && GET_IDNUM(ch) != 3 && GET_IDNUM(ch) != 2) {
    send_to_char("Huh?!?\r\n", ch);
    return;
  } else {
    sprintf(buf, "/cw%s's password is %s./c0\r\n",
                 vbuf.name, vbuf.pwd);
    send_to_char(buf, ch);
  }
}

ACMD(do_warning)
{
  struct char_data *vict;

  half_chop(argument, arg, buf);

  if (!*arg) {
    send_to_char("Send a warning to who?\r\n", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, arg))) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char(NOPERSON, ch);
    return;
  }
  if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
    send_to_char("You should not warn people who are holier than yourself.\r\n", ch);
    return;
  }
  if (ch == vict) {
    send_to_char("Warning yourself, eh?.\r\n", ch);
    return;
  }
  if (!*buf) {
    send_to_char("You must enter a warning message.\r\n", ch);
    return;
  }
  send_to_char("/cR[WARNING]:/c0 ", vict);
  send_to_char(buf, vict);
  send_to_char("/cR[WARNING]\r\n", vict);
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char("Sent.\r\n", ch);
  else {
    sprintf(buf2, "You send '%s' to %s.\r\n", buf, GET_NAME(vict));
    send_to_char(buf2, ch);
  }
}

ACMD(do_rembit)
{
  struct char_data *vict;

  two_arguments(argument, buf, buf2);

  if (!*buf)
    send_to_char("Whom do you wish to remove an affect from?\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf)))
    send_to_char(NOPERSON, ch);
  else if (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch))
    send_to_char("Maybe you shouldn't do that.\r\n", ch);
  else if (!*buf2)
    send_to_char("What affect is it you wish to remove?\r\n", ch);

  else if (is_abbrev(buf2, "sanctuary")) {
    send_to_char("Affect removed.\r\n", ch);
    REMOVE_BIT_AR(AFF_FLAGS(vict), AFF_SANCTUARY);
  } else
    send_to_char("Unrecognized affect.\r\n", ch);
}
