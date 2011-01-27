/************************************************************************
 * OasisOLC - medit.c						v1.5	*
 * Copyright 1996 Harvey Gilpin.					*
 ************************************************************************/

#include <math.h>

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "shop.h"
#include "olc.h"
#include "handler.h"
#include "dg_olc.h"
#include "mounts.h"

/*
 * Set this to 1 for debugging logs in medit_save_internally.
 */
#if 0
#define DEBUG
#endif

/*
 * Set this to 1 as a last resort to save mobiles.
 */
#if 1
#define I_CRASH
#endif

/*-------------------------------------------------------------------*/

/*
 * External variable declarations.
 */
extern struct index_data *mob_index;
extern struct char_data *mob_proto;
extern struct char_data *character_list;
extern int top_of_mobt;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct player_special_data dummy_mob;
extern struct attack_hit_type attack_hit_text[];
extern char *action_bits[];
extern char *affected_bits[];
extern char *position_types[];
extern char *genders[];
extern int top_shop;
extern struct shop_data *shop_index;
extern struct descriptor_data *descriptor_list;
extern char *npc_class_types[];
extern char *npc_race_types[];
#if defined(OASIS_MPROG)
extern const char *mobprog_types[];
#endif
extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
extern char *skills[];
extern char *spells[];
extern char *chants[];
extern char *prayers[];
extern char *songs[];

extern int nr_mount_types;

struct mount_types mountt[MAX_MOUNT_TYPES];

/*-------------------------------------------------------------------*/

/*
 * Handy internal macros.
 */

#define GET_NDD(mob) ((mob)->mob_specials.damnodice)
#define GET_SDD(mob) ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)
#define GET_ATTACK(mob) ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop) ((shop)->keeper)
#if defined(OASIS_MPROG)
#define GET_MPROG(mob)		(mob_index[(mob)->nr].mobprogs)
#define GET_MPROG_TYPE(mob)	(mob_index[(mob)->nr].progtypes)
#endif

/*-------------------------------------------------------------------*/

/*
 * Function prototypes.
 */
void medit_parse(struct descriptor_data *d, char *arg);
void medit_disp_menu(struct descriptor_data *d);
void medit_setup_new(struct descriptor_data *d);
void medit_setup_existing(struct descriptor_data *d, int rmob_num);
void medit_save_internally(struct descriptor_data *d);
void medit_save_to_disk(int zone_num);
void init_mobile(struct char_data *mob);
void copy_mobile(struct char_data *tmob, struct char_data *fmob);
void medit_disp_positions(struct descriptor_data *d);
void medit_disp_mob_flags(struct descriptor_data *d);
void medit_disp_aff_flags(struct descriptor_data *d);
void medit_disp_attack_types(struct descriptor_data *d);
#if defined(OASIS_MPROG)
void medit_disp_mprog(struct descriptor_data *d);
void medit_change_mprog(struct descriptor_data *d);
const char *medit_get_mprog_type(struct mob_prog_data *mprog);
#endif
void disp_stats_menu(struct descriptor_data *);
void show_mob_abilities(struct descriptor_data *d, int type);
void show_mob_skills(struct descriptor_data *d, int medit_num);
void show_mob_spells(struct descriptor_data *d, int medit_num);
void show_mob_chants(struct descriptor_data *d, int medit_num);
void show_mob_prayers(struct descriptor_data *d, int medit_num);
const char show_mob_abil_x(struct char_data *mob, int skill_nr, int type);
void medit_disp_prayer_menu(struct descriptor_data *d);
void medit_disp_spell_menu(struct descriptor_data *d);
void medit_disp_chant_menu(struct descriptor_data *d);
void medit_disp_skill_menu(struct descriptor_data *d);
void medit_disp_abil_menu(struct descriptor_data *d);
void medit_disp_paladin_menu(struct descriptor_data *d);
void medit_disp_dkn_menu(struct descriptor_data *d);
void medit_disp_monk_menu(struct descriptor_data *d);
void disp_mtype_menu(struct descriptor_data *d);

/* Mob damage dice tables for autostatting */
const int mob_dam_dice[][2]={
  {1, 1}, {1, 1}, {1, 1}, {1, 2}, {1, 2},
  {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
  {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 3},
  {1, 3}, {1, 3}, {1, 3}, {1, 3}, {1, 4},
  {1, 4}, {1, 4}, {1, 4}, {2, 3}, {2, 3}, /* 25 */
  {2, 3}, {2, 4}, {2, 4}, {3, 3}, {3, 3},
  {3, 4}, {3, 4}, {3, 4}, {3, 4}, {3, 5},
  {3, 5}, {3, 6}, {3, 6}, {4, 5}, {4, 5},
  {4, 5}, {4, 5}, {4, 5}, {4, 5}, {4, 5},
  {5, 5}, {5, 5}, {5, 5}, {5, 5}, {5, 5}, /* 50 */
  {5, 5}, {5, 5}, {5, 6}, {5, 6}, {5, 6},
  {5, 6}, {5, 6}, {5, 6}, {5, 6}, {5, 6},
  {5, 6}, {5, 6}, {6, 6}, {6, 6}, {6, 6},
  {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
  {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 7}, /* 75 */
  {6, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7},
  {6, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7},
  {6, 7}, {6, 7}, {6, 7}, {6, 7}, {6, 7},
  {6, 7}, {6, 7}, {6, 8}, {6, 8}, {6, 8},
  {6, 8}, {6, 8}, {6, 8}, {6, 8}, {9, 10}, /* 100 */
  {9, 10}, {9, 10}, {10, 10}, {10, 10}, {10, 11},
  {10, 11}, {11, 11}, {11, 11}, {11, 12}, {11, 12},
  {12, 12}, {12, 12}, {12, 13}, {12, 13}, {13, 13},
  {13, 13}, {13, 14}, {13, 14}, {14, 14}, {14, 14},
  {14, 14}, {14, 14}, {14, 15}, {14, 15}, {15, 15}, /* 125 */
  {15, 15}, {15, 15}, {15, 15}, {15, 15}, {15, 16},
  {15, 15}, {15, 16}, {16, 16}, {16, 16}, {16, 17},
  {16, 17}, {16, 17}, {16, 17}, {16, 17}, {17, 17},
  {17, 17}, {17, 18}, {17, 18}, {18, 18}, {18, 18},
  {18, 18}, {18, 19}, {18, 19}, {18, 19}, {20, 20}, /* 150 */
  {21, 21}, {21, 22}, {21, 22}, {21, 22}, {21, 22},
  {22, 22}, {22, 22}, {22, 23}, {22, 23}, {22, 23},
  {24, 24}, {24, 24}, {24, 24}, {24, 25}, {24, 25},
  {25, 25}, {25, 25}, {25, 26}, {25, 26}, {25, 26},
  {26, 26}, {26, 26}, {26, 27}, {27, 28}, {28, 28}, /* 175 */
  {28, 28}, {28, 28}, {28, 28}, {28, 29}, {29, 29},
  {29, 30}, {30, 31}, {31, 31}, {31, 32}, {32, 32},
  {32, 33}, {33, 33}, {33, 34}, {34, 34}, {34, 35},
  {35, 35}, {35, 36}, {36, 36}, {36, 37}, {37, 37},
  {37, 38}, {38, 38}, {38, 39}, {39, 40}, {40, 45}, /* 200 */
  {41, 45}, {42, 45}, {43, 45}, {44, 45}, {45, 45},
  {45, 46}, {46, 46}, {46, 47}, {47, 47}, {47, 48},
  {48, 48}, {48, 49}, {49, 49}, {49, 49}, {49, 49},
  {49, 50}, {49, 50}, {49, 50}, {50, 50}, {50, 50},
  {50, 50}, {50, 50}, {50, 51}, {50, 51}, {50, 51}, /* 225 */
  {50, 51}, {51, 51}, {51, 51}, {52, 52}, {52, 52}, 
  {52, 53}, {52, 53}, {53, 53}, {53, 53}, {53, 54}, 
  {53, 54}, {53, 54}, {53, 54}, {54, 54}, {54, 54}, 
  {54, 54}, {54, 54}, {54, 55}, {54, 55}, {54, 55}, 
  {54, 55}, {55, 55}, {55, 55}, {55, 55}, {55, 75}  /* 250 */
}; /* mob_dam_dice */

/* Mob gold for autostatting */
const int mob_gold[] = {
     5,     5,     5,     5,     5,    10,    10,    10,    15,    15,
    25,    25,    25,    50,    50,    75,    75,   100,   100,   150,
   150,   150,   150,   150,   200,   200,   200,   250,   250,   250,
   300,   300,   300,   300,   350,   350,   400,   400,   450,   500,
   525,   550,   575,   600,   625,   650,   675,   700,   725,   750,
   775,   800,   825,   850,   875,   900,   925,   950,   975,  1000,
  1025,  1050,  1075,  1100,  1125,  1150,  1175,  1200,  1125,  1150,
  1175,  1200,  1225,  1250,  1275,  1300,  1325,  1350,  1375,  1400,
  1425,  1450,  1475,  1500,  1525,  1550,  1575,  1600,  1675,  1700,
  1725,  1750,  1775,  1800,  1825,  1850,  1875,  1900,  1925,  1950,
  1975,  2000,  2025,  2050,  2075,  2100,  2125,  2150,  2175,  2200,
  2225,  2250,  2275,  2300,  2325,  2350,  2375,  2400,  2425,  2450,
  2475,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,
  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,
  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,  2500,
  2500,  2500,  2500,  2500,  2550,  2550,  2550,  2550,  2550,  2550,
  2550,  2600,  2600,  2600,  2600,  2600,  2600,  2600,  2650,  2650,
  2700,  2700,  2700,  2700,  2700,  2700,  2700,  2700,  2700,  2700,
  2750,  2750,  2750,  2750,  2750,  2800,  2800,  2800,  2800,  2800,
  2850,  2850,  2850,  2850,  2850,  2900,  2900,  2900,  2900,  2900,
  2950,  2950,  2950,  2950,  2950,  3000,  3000,  3000,  3000,  3000,
  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,  3100,
  3200,  3200,  3200,  3200,  3200,  3200,  3200,  3200,  3200,  3200,
  3300,  3300,  3300,  3300,  3300,  3300,  3300,  3300,  3300,  3300,
  3400,  3400,  3400,  3400,  3400,  3500,  3500,  3500,  3500,  3500
};

/* Mob armor class for autostatting */
const int mob_ac[] = {
   96,  92,  88,  84,  80,  76,  72,  68,  64,  60,
   56,  52,  48,  44,  40,  36,  32,  28,  24,  20,
   16,  12,   8,   4,   0,  -4,  -8, -12, -16, -20,
  -20, -20, -20, -20, -20, -20, -20, -20, -20, -20,
  -24, -24, -24, -24, -24, -24, -28, -28, -28, -28,
  -32, -32, -32, -32, -32, -32, -32, -36, -36, -36,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
  -60, -60, -60, -60, -60, -60, -60, -60, -60, -60,
  -60, -60, -60, -60, -60, -60, -60, -60, -60, -60,
  -60, -60, -60, -60, -60, -60, -60, -60, -60, -60,
  -60, -60, -60, -60, -60, -60, -60, -60, -60, -60,
  -60, -60, -60, -60, -60, -60, -60, -60, -60, -60,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100,
  -100, -100, -100, -100, -100, -100, -100, -100, -100, -100
};

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/

/*
 * Free a mobile structure that has been edited.
 * Take care of existing mobiles and their mob_proto!
 */

void medit_free_mobile(struct char_data *mob)
{
  int i;
  /*
   * Non-prototyped mobile.  Also known as new mobiles.
   */
  if (!mob)
    return;
  else if (GET_MOB_RNUM(mob) == -1) {
    if (mob->player.name)
      free(mob->player.name);
    if (mob->player.title)
      free(mob->player.title);
    if (mob->player.short_descr)
      free(mob->player.short_descr);
    if (mob->player.long_descr)
      free(mob->player.long_descr);
    if (mob->player.description)
      free(mob->player.description);
  } else if ((i = GET_MOB_RNUM(mob)) > -1) {	/* Prototyped mobile. */
    if (mob->player.name && mob->player.name != mob_proto[i].player.name)
      free(mob->player.name);
    if (mob->player.title && mob->player.title != mob_proto[i].player.title)
      free(mob->player.title);
    if (mob->player.short_descr && mob->player.short_descr != mob_proto[i].player.short_descr)
      free(mob->player.short_descr);
    if (mob->player.long_descr && mob->player.long_descr != mob_proto[i].player.long_descr)
      free(mob->player.long_descr);
    if (mob->player.description && mob->player.description != mob_proto[i].player.description)
      free(mob->player.description);
  }
  while (mob->affected)
    affect_remove(mob, mob->affected);

  free(mob);
}

void medit_setup_new(struct descriptor_data *d)
{
  struct char_data *mob;

  /*
   * Allocate a scratch mobile structure.  
   */
  CREATE(mob, struct char_data, 1);

  init_mobile(mob);

  GET_MOB_RNUM(mob) = -1;
  /*
   * Set up some default strings.
   */
  GET_ALIAS(mob) = str_dup("mob unfinished");
  GET_SDESC(mob) = str_dup("the unfinished mob");
  GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
  GET_DDESC(mob) = str_dup("It looks unfinished.\r\n");
#if defined(OASIS_MPROG)
  OLC_MPROGL(d) = NULL;
  OLC_MPROG(d) = NULL;
#endif

  OLC_MOB(d) = mob;
  OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;

  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

void medit_setup_existing(struct descriptor_data *d, int rmob_num)
{
  struct char_data *mob;
#if defined(OASIS_MPROG)
  MPROG_DATA *temp;
  MPROG_DATA *head;
#endif

  /*
   * Allocate a scratch mobile structure. 
   */
  CREATE(mob, struct char_data, 1);

  copy_mobile(mob, mob_proto + rmob_num);

#if defined(OASIS_MPROG)
  /*
   * I think there needs to be a brace from the if statement to the #endif
   * according to the way the original patch was indented.  If this crashes,
   * try it with the braces and report to greerga@van.ml.org on if that works.
   */
  if (GET_MPROG(mob))
    CREATE(OLC_MPROGL(d), MPROG_DATA, 1);
  head = OLC_MPROGL(d);
  for (temp = GET_MPROG(mob); temp;temp = temp->next) {
    OLC_MPROGL(d)->type = temp->type;
    OLC_MPROGL(d)->arglist = str_dup(temp->arglist);
    OLC_MPROGL(d)->comlist = str_dup(temp->comlist);
    if (temp->next) {
      CREATE(OLC_MPROGL(d)->next, MPROG_DATA, 1);
      OLC_MPROGL(d) = OLC_MPROGL(d)->next;
    }
  }
  OLC_MPROGL(d) = head;
  OLC_MPROG(d) = OLC_MPROGL(d);
#endif

  OLC_MOB(d) = mob;
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;
  dg_olc_script_copy(d);
  medit_disp_menu(d);
}

/*-------------------------------------------------------------------*/

/*
 * Copy one mob struct to another.
 */
void copy_mobile(struct char_data *tmob, struct char_data *fmob)
{
   struct trig_proto_list *proto, *fproto;
  /*
   * Free up any used strings.
   */
  if (GET_ALIAS(tmob))
    free(GET_ALIAS(tmob));
  if (GET_SDESC(tmob))
    free(GET_SDESC(tmob));
  if (GET_LDESC(tmob))
    free(GET_LDESC(tmob));
  if (GET_DDESC(tmob))
    free(GET_DDESC(tmob));

  /* delete the old script list */
  proto = tmob->proto_script;
  while (proto) {
    fproto = proto;
    proto = proto->next;
    free(fproto);
  }

  /*
   * Copy mob over.
   */
  *tmob = *fmob;

  /*
   * Reallocate strings.
   */
  GET_ALIAS(tmob) = str_dup((GET_ALIAS(fmob) && *GET_ALIAS(fmob)) ? GET_ALIAS(fmob) : "undefined");
  GET_SDESC(tmob) = str_dup((GET_SDESC(fmob) && *GET_SDESC(fmob)) ? GET_SDESC(fmob) : "undefined");
  GET_LDESC(tmob) = str_dup((GET_LDESC(fmob) && *GET_LDESC(fmob)) ? GET_LDESC(fmob) : "undefined");
  GET_DDESC(tmob) = str_dup((GET_DDESC(fmob) && *GET_DDESC(fmob)) ? GET_DDESC(fmob) : "undefined");
  /* copy the new script list */
  if (fmob->proto_script) {
    fproto = fmob->proto_script;
    CREATE(proto, struct trig_proto_list, 1);
    tmob->proto_script = proto;
    do {
      proto->vnum = fproto->vnum;
      fproto = fproto->next;
      if (fproto) { 
        CREATE(proto->next, struct trig_proto_list, 1);
        proto = proto->next;
      }
    } while (fproto);
  }
}

/*-------------------------------------------------------------------*/

/*
 * Ideally, this function should be in db.c, but I'll put it here for
 * portability.
 */
void init_mobile(struct char_data *mob)
{
  clear_char(mob);

  GET_HIT(mob) = GET_MANA(mob) = 1;
  GET_MAX_MANA(mob) = GET_MAX_MOVE(mob) = 100;
  GET_NDD(mob) = GET_SDD(mob) = 1;
  GET_WEIGHT(mob) = 200;
  GET_HEIGHT(mob) = 198;

  mob->real_stats.str = mob->real_stats.intel = mob->real_stats.wis = 11;
  mob->real_stats.dex = mob->real_stats.con = mob->real_stats.cha = 11;
  mob->real_stats.luck = 11;
  mob->aff_stats = mob->real_stats;

  SET_BIT_AR(MOB_FLAGS(mob), MOB_ISNPC);
  mob->player_specials = &dummy_mob;
}

/*-------------------------------------------------------------------*/

#define ZCMD zone_table[zone].cmd[cmd_no]

/*
 * Save new/edited mob to memory.
 */
void medit_save_internally(struct descriptor_data *d)
{
  int rmob_num, found = 0, new_mob_num = 0, zone, cmd_no, shop;
  struct char_data *new_proto;
  struct index_data *new_index;
  struct char_data *live_mob;
  struct descriptor_data *dsc;

  /* put the script into proper position */
  OLC_MOB(d)->proto_script = OLC_SCRIPT(d);

  /*
   * Mob exists? Just update it.
   */
  if ((rmob_num = real_mobile(OLC_NUM(d))) != -1) {
    copy_mobile((mob_proto + rmob_num), OLC_MOB(d));
    /*
     * Update live mobiles.
     */
    for (live_mob = character_list; live_mob; live_mob = live_mob->next)
      if (IS_MOB(live_mob) && GET_MOB_RNUM(live_mob) == rmob_num) {
        /*
	 * Only really need to update the strings, since these can
	 * cause protection faults.  The rest can wait till a reset/reboot.
	 */
	GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
	GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
	GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
	GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
      }
  }
  /*
   * Mob does not exist, we have to add it.
   */
  else {
#if defined(DEBUG)
    fprintf(stderr, "top_of_mobt: %d, new top_of_mobt: %d\n", top_of_mobt, top_of_mobt + 1);
#endif

    CREATE(new_proto, struct char_data, top_of_mobt + 2);
    CREATE(new_index, struct index_data, top_of_mobt + 2);

    for (rmob_num = 0; rmob_num <= top_of_mobt; rmob_num++) {
      if (!found) {		/* Is this the place? */
/*	if ((rmob_num > top_of_mobt) || (mob_index[rmob_num].virtual > OLC_NUM(d))) {*/
	if (mob_index[rmob_num].virtual > OLC_NUM(d)) {
	  /*
	   * Yep, stick it here.
	   */
	  found = TRUE;
#if defined(DEBUG)
	  fprintf(stderr, "Inserted: rmob_num: %d\n", rmob_num);
#endif
	  new_index[rmob_num].virtual = OLC_NUM(d);
	  new_index[rmob_num].number = 0;
	  new_index[rmob_num].func = NULL;
	  new_mob_num = rmob_num;
	  GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
	  copy_mobile((new_proto + rmob_num), OLC_MOB(d));
	  /*
	   * Copy the mob that should be here on top.
	   */
	  new_index[rmob_num + 1] = mob_index[rmob_num];
	  new_proto[rmob_num + 1] = mob_proto[rmob_num];
	  GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
	} else {	/* Nope, copy over as normal. */
	  new_index[rmob_num] = mob_index[rmob_num];
	  new_proto[rmob_num] = mob_proto[rmob_num];
	}
      } else { /* We've already found it, copy the rest over. */
	new_index[rmob_num + 1] = mob_index[rmob_num];
	new_proto[rmob_num + 1] = mob_proto[rmob_num];
	GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
      }
    }
#if defined(DEBUG)
    fprintf(stderr, "rmob_num: %d, top_of_mobt: %d, array size: 0-%d (%d)\n",
		rmob_num, top_of_mobt, top_of_mobt + 1, top_of_mobt + 2);
#endif
    if (!found) { /* Still not found, must add it to the top of the table. */
#if defined(DEBUG)
      fprintf(stderr, "Append.\n");
#endif
      new_index[rmob_num].virtual = OLC_NUM(d);
      new_index[rmob_num].number = 0;
      new_index[rmob_num].func = NULL;
      new_mob_num = rmob_num;
      GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
      copy_mobile((new_proto + rmob_num), OLC_MOB(d));
    }
    /*
     * Replace tables.
     */
#if defined(DEBUG)
    fprintf(stderr, "Attempted free.\n");
#endif
#if !defined(I_CRASH)
    free(mob_index);
    free(mob_proto);
#endif
    mob_index = new_index;
    mob_proto = new_proto;
    top_of_mobt++;
#if defined(DEBUG)
    fprintf(stderr, "Free ok.\n");
#endif

    /*
     * Update live mobile rnums.
     */
    for (live_mob = character_list; live_mob; live_mob = live_mob->next)
      if (GET_MOB_RNUM(live_mob) > new_mob_num)
	GET_MOB_RNUM(live_mob)++;

    /*
     * Update zone table.
     */
    for (zone = 0; zone <= top_of_zone_table; zone++)
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
	if (ZCMD.command == 'M')
	  if (ZCMD.arg1 >= new_mob_num)
	    ZCMD.arg1++;

    /*
     * Update shop keepers.
     */
    if (shop_index)
      for (shop = 0; shop <= top_shop; shop++)
 	if (SHOP_KEEPER(shop) >= new_mob_num)
	  SHOP_KEEPER(shop)++;

    /*
     * Update keepers in shops being edited and other mobs being edited.
     */
    for (dsc = descriptor_list; dsc; dsc = dsc->next)
      if (dsc->connected == CON_SEDIT) {
	if (S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
	  S_KEEPER(OLC_SHOP(dsc))++;
      } else if (dsc->connected == CON_MEDIT) {
	if (GET_MOB_RNUM(OLC_MOB(dsc)) >= new_mob_num)
	  GET_MOB_RNUM(OLC_MOB(dsc))++;
      }
  }

#if defined(OASIS_MPROG)
  GET_MPROG(OLC_MOB(d)) = OLC_MPROGL(d);
  GET_MPROG_TYPE(OLC_MOB(d)) = (OLC_MPROGL(d) ? OLC_MPROGL(d)->type : 0);
  while (OLC_MPROGL(d)) {
    GET_MPROG_TYPE(OLC_MOB(d)) |= OLC_MPROGL(d)->type;
    OLC_MPROGL(d) = OLC_MPROGL(d)->next;
  }
#endif

  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/*-------------------------------------------------------------------*/

/*
 * Save ALL mobiles for a zone to their .mob file, mobs are all 
 * saved in Extended format, regardless of whether they have any
 * extended fields.  Thanks to Sammy for ideas on this bit of code.
 */
void medit_save_to_disk(int zone_num)
{
  int i, rmob_num, zone, top;
  FILE *mob_file;
  char fname[64];
  struct char_data *mob;
#if defined(OASIS_MPROG)
  MPROG_DATA *mob_prog = NULL;
#endif

  zone = zone_table[zone_num].number;
  top = zone_table[zone_num].top;

  sprintf(fname, "%s/%d.new", MOB_PREFIX, zone);
  if (!(mob_file = fopen(fname, "w"))) {
    mudlog("SYSERR: OLC: Cannot open mob file!", BRF, LVL_BUILDER, TRUE);
    return;
  }

  /*
   * Seach the database for mobs in this zone and save them.
   */
  for (i = zone * 100; i <= top; i++) {
    if ((rmob_num = real_mobile(i)) != -1) {
      if (fprintf(mob_file, "#%d\n", i) < 0) {
	mudlog("SYSERR: OLC: Cannot write mob file!\r\n", BRF, LVL_BUILDER, TRUE);
	fclose(mob_file);
	return;
      }
      mob = (mob_proto + rmob_num);

      /*
       * Clean up strings.
       */
      strcpy(buf1, (GET_LDESC(mob) && *GET_LDESC(mob)) ? GET_LDESC(mob) : "undefined");
      strip_string(buf1);
      strcpy(buf2, (GET_DDESC(mob) && *GET_DDESC(mob)) ? GET_DDESC(mob) : "undefined");
      strip_string(buf2);

      fprintf(mob_file, "%s~\n"
			"%s~\n"
			"%s~\n"
			"%s~\n"
			"%d %d %d %d %d %d %d %d %d E\n"
			"%d %d %d %dd%d+%d %dd%d+%d\n"
			"%d %d\n"
			"%d %d %d\n",
	      (GET_ALIAS(mob) && *GET_ALIAS(mob)) ? GET_ALIAS(mob) : "undefined",
	      (GET_SDESC(mob) && *GET_SDESC(mob)) ? GET_SDESC(mob) : "undefined",
	      buf1, buf2, MOB_FLAGS(mob)[0], MOB_FLAGS(mob)[1],
              MOB_FLAGS(mob)[2], MOB_FLAGS(mob)[3], AFF_FLAGS(mob)[0],
              AFF_FLAGS(mob)[1], AFF_FLAGS(mob)[2], AFF_FLAGS(mob)[3],
              GET_ALIGNMENT(mob),
	      GET_LEVEL(mob), GET_HITROLL(mob),
	      GET_AC(mob), GET_HIT(mob), GET_MANA(mob), GET_MOVE(mob),
	      GET_NDD(mob), GET_SDD(mob), GET_DAMROLL(mob), GET_GOLD(mob),
	      GET_EXP(mob), GET_POS(mob), GET_DEFAULT_POS(mob), GET_SEX(mob)
	      );

      /*
       * Deal with Extra stats in case they are there.
       */
      if (GET_ATTACK(mob) != 0)
	fprintf(mob_file, "BareHandAttack: %d\n", GET_ATTACK(mob));
      if (GET_STR(mob) != 11)
	fprintf(mob_file, "Str: %d\n", GET_STR(mob));
      if (GET_ADD(mob) != 0)
	fprintf(mob_file, "StrAdd: %d\n", GET_ADD(mob));
      if (GET_DEX(mob) != 11)
	fprintf(mob_file, "Dex: %d\n", GET_DEX(mob));
      if (GET_INT(mob) != 11)
	fprintf(mob_file, "Int: %d\n", GET_INT(mob));
      if (GET_WIS(mob) != 11)
	fprintf(mob_file, "Wis: %d\n", GET_WIS(mob));
      if (GET_CON(mob) != 11)
	fprintf(mob_file, "Con: %d\n", GET_CON(mob));
      if (GET_CHA(mob) != 11)
	fprintf(mob_file, "Cha: %d\n", GET_CHA(mob));
      if (GET_LUCK(mob) != 11)
	fprintf(mob_file, "Luck: %d\n", GET_LUCK(mob));
      if(GET_CLASS(mob) != CLASS_NPC_OTHER)
        fprintf(mob_file, "Class: %d\n", GET_CLASS(mob));
      if(GET_RACE(mob) != RACE_NPC_OTHER)
        fprintf(mob_file, "Race: %d\n", GET_RACE(mob));
	  if(mob->mob_specials.skills[MOB_SKILL_ONE] != 0)
        fprintf(mob_file, "Skill1: %d\n", mob->mob_specials.skills[MOB_SKILL_ONE]);
	  if(mob->mob_specials.skills[MOB_SKILL_TWO] != 0)
        fprintf(mob_file, "Skill2: %d\n", mob->mob_specials.skills[MOB_SKILL_TWO]);
	  if(mob->mob_specials.skills[MOB_SKILL_THREE] != 0)
        fprintf(mob_file, "Skill3: %d\n", mob->mob_specials.skills[MOB_SKILL_THREE]);
	  if(mob->mob_specials.skills[MOB_SKILL_FOUR] != 0)
        fprintf(mob_file, "Skill4: %d\n", mob->mob_specials.skills[MOB_SKILL_FOUR]);
	  if(mob->mob_specials.skills[MOB_SKILL_FIVE] != 0)
        fprintf(mob_file, "Skill5: %d\n", mob->mob_specials.skills[MOB_SKILL_FIVE]);
	  if(mob->mob_specials.spells[MOB_SKILL_ONE] != 0)
        fprintf(mob_file, "Spell1: %d\n", mob->mob_specials.spells[MOB_SKILL_ONE]);
	  if(mob->mob_specials.spells[MOB_SKILL_TWO] != 0)
        fprintf(mob_file, "Spell2: %d\n", mob->mob_specials.spells[MOB_SKILL_TWO]);
	  if(mob->mob_specials.spells[MOB_SKILL_THREE] != 0)
        fprintf(mob_file, "Spell3: %d\n", mob->mob_specials.spells[MOB_SKILL_THREE]);
	  if(mob->mob_specials.spells[MOB_SKILL_FOUR] != 0)
        fprintf(mob_file, "Spell4: %d\n", mob->mob_specials.spells[MOB_SKILL_FOUR]);
	  if(mob->mob_specials.spells[MOB_SKILL_FIVE] != 0)
        fprintf(mob_file, "Spell5: %d\n", mob->mob_specials.spells[MOB_SKILL_FIVE]);
	  if(mob->mob_specials.chants[MOB_SKILL_ONE] != 0)
        fprintf(mob_file, "Chant1: %d\n", mob->mob_specials.chants[MOB_SKILL_ONE]);
	  if(mob->mob_specials.chants[MOB_SKILL_TWO] != 0)
        fprintf(mob_file, "Chant2: %d\n", mob->mob_specials.chants[MOB_SKILL_TWO]);
	  if(mob->mob_specials.chants[MOB_SKILL_THREE] != 0)
        fprintf(mob_file, "Chant3: %d\n", mob->mob_specials.chants[MOB_SKILL_THREE]);
	  if(mob->mob_specials.chants[MOB_SKILL_FOUR] != 0)
        fprintf(mob_file, "Chant4: %d\n", mob->mob_specials.chants[MOB_SKILL_FOUR]);
	  if(mob->mob_specials.chants[MOB_SKILL_FIVE] != 0)
        fprintf(mob_file, "Chant5: %d\n", mob->mob_specials.chants[MOB_SKILL_FIVE]);
	  if(mob->mob_specials.prayers[MOB_SKILL_ONE] != 0)
        fprintf(mob_file, "Prayer1: %d\n", mob->mob_specials.prayers[MOB_SKILL_ONE]);
	  if(mob->mob_specials.prayers[MOB_SKILL_TWO] != 0)
        fprintf(mob_file, "Prayer2: %d\n", mob->mob_specials.prayers[MOB_SKILL_TWO]);
	  if(mob->mob_specials.prayers[MOB_SKILL_THREE] != 0)
        fprintf(mob_file, "Prayer3: %d\n", mob->mob_specials.prayers[MOB_SKILL_THREE]);
	  if(mob->mob_specials.prayers[MOB_SKILL_FOUR] != 0)
        fprintf(mob_file, "Prayer4: %d\n", mob->mob_specials.prayers[MOB_SKILL_FOUR]);
	  if(mob->mob_specials.prayers[MOB_SKILL_FIVE] != 0)
        fprintf(mob_file, "Prayer5: %d\n", mob->mob_specials.prayers[MOB_SKILL_FIVE]);
	  if(mob->mob_specials.mount_type != 0)
        fprintf(mob_file, "MountType: %d\n", mob->mob_specials.mount_type);

      /*
       * XXX: Add E-mob handlers here.
       */

      fprintf(mob_file, "E\n");
	  script_save_to_disk(mob_file, mob, MOB_TRIGGER);

#if defined(OASIS_MPROG)
      /*
       * Write out the MobProgs.
       */
      mob_prog = GET_MPROG(mob);
      while(mob_prog) {
	strcpy(buf1, mob_prog->arglist);
	strip_string(buf1);
	strcpy(buf2, mob_prog->comlist);
	strip_string(buf2);
	fprintf(mob_file, "%s %s~\n%s", medit_get_mprog_type(mob_prog),
					buf1, buf2);
	mob_prog = mob_prog->next;
	fprintf(mob_file, "~\n%s", (!mob_prog ? "|\n" : ""));
      }
#endif
    }
  }
  fprintf(mob_file, "$\n");
  fclose(mob_file);
  sprintf(buf2, "%s/%d.mob", MOB_PREFIX, zone);
  /*
   * We're fubar'd if we crash between the two lines below.
   */
  remove(buf2);
  rename(fname, buf2);

  olc_remove_from_save_list(zone_table[zone_num].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * Display positions. (sitting, standing, etc)
 */
void medit_disp_positions(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 0; *position_types[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, position_types[i]);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter position number : ", d->character);
}

/*-------------------------------------------------------------------*/

#if defined(OASIS_MPROG)
/*
 * Get the type of MobProg.
 */
const char *medit_get_mprog_type(struct mob_prog_data *mprog)
{
  switch (mprog->type) {
  case IN_FILE_PROG:	return ">in_file_prog";
  case ACT_PROG:	return ">act_prog";
  case SPEECH_PROG:	return ">speech_prog";
  case RAND_PROG:	return ">rand_prog";
  case FIGHT_PROG:	return ">fight_prog";
  case HITPRCNT_PROG:	return ">hitprcnt_prog";
  case DEATH_PROG:	return ">death_prog";
  case ENTRY_PROG:	return ">entry_prog";
  case GREET_PROG:	return ">greet_prog";
  case ALL_GREET_PROG:	return ">all_greet_prog";
  case GIVE_PROG:	return ">give_prog";
  case BRIBE_PROG:	return ">bribe_prog";
  }
  return ">ERROR_PROG";
}

/*-------------------------------------------------------------------*/

/*
 * Display the MobProgs.
 */
void medit_disp_mprog(struct descriptor_data *d)
{
  struct mob_prog_data *mprog = OLC_MPROGL(d);

  OLC_MTOTAL(d) = 1;

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  while (mprog) {
    sprintf(buf, "%d) %s %s\r\n", OLC_MTOTAL(d), medit_get_mprog_type(mprog),
		(mprog->arglist ? mprog->arglist : "NONE"));
    send_to_char(buf, d->character);
    OLC_MTOTAL(d)++;
    mprog = mprog->next;
  }
  sprintf(buf,  "%d) Create New Mob Prog\r\n"
		"%d) Purge Mob Prog\r\n"
		"Enter number to edit [0 to exit]:  ",
		OLC_MTOTAL(d), OLC_MTOTAL(d) + 1);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProgs.
 */
void medit_change_mprog(struct descriptor_data *d)
{
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  sprintf(buf,  "1) Type: %s\r\n"
		"2) Args: %s\r\n"
		"3) Commands:\r\n%s\r\n\r\n"
		"Enter number to edit [0 to exit]: ",
	medit_get_mprog_type(OLC_MPROG(d)),
	(OLC_MPROG(d)->arglist ? OLC_MPROG(d)->arglist: "NONE"),
	(OLC_MPROG(d)->comlist ? OLC_MPROG(d)->comlist : "NONE"));

  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_CHANGE_MPROG;
}

/*-------------------------------------------------------------------*/

/*
 * Change the MobProg type.
 */
void medit_disp_mprog_types(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif

  for (i = 0; i < NUM_PROGS-1; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, mobprog_types[i]);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter mob prog type : ", d->character);
  OLC_MODE(d) = MEDIT_MPROG_TYPE;
}
#endif

/*-------------------------------------------------------------------*/

/*
 * Display the gender of the mobile.
 */
void medit_disp_sex(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 0; i < NUM_GENDERS; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, genders[i]);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter gender number : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display attack types menu.
 */
void medit_disp_attack_types(struct descriptor_data *d)
{
  int i;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 0; i < NUM_ATTACK_TYPES; i++) {
    sprintf(buf, "%s%2d%s) %s\r\n", grn, i, nrm, attack_hit_text[i].singular);
    send_to_char(buf, d->character);
  }
  send_to_char("Enter attack type : ", d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display mob-flags menu.
 */
void medit_disp_mob_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 0; i < NUM_MOB_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, action_bits[i],
		!(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(MOB_FLAGS(OLC_MOB(d)), action_bits, AF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nCurrent flags : %s%s%s\r\nEnter mob flags (0 to quit) : ",
		  cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/*-------------------------------------------------------------------*/

/*
 * Display affection flags menu.
 */
void medit_disp_aff_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 1; i < NUM_AFF_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i, nrm, affected_bits[i],
			!(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(AFF_FLAGS(OLC_MOB(d)), affected_bits, AF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nCurrent flags   : %s%s%s\r\nEnter aff flags (0 to quit) : ",
			  cyn, buf1, nrm);
  send_to_char(buf, d->character);
  send_to_char("\r\n", d->character);
}

/* Do I get a cookie if this works? */
/* Nope.  Sorry. */
void medit_disp_class_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  for (i = 0; i < NUM_NPC_CLASS; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i, nrm, npc_class_types[i],
                        !(++columns % 3) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintf(buf, "\r\nEnter class type: "); 
  SEND_TO_Q(buf, d);
}
void medit_disp_race_flags(struct descriptor_data *d)
{
  int i, columns = 0;

  for (i = 0; i < NUM_NPC_RACE; i++) {
    sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i, nrm, npc_race_types[i],
                        !(++columns % 3) ? "\r\n" : "");
    SEND_TO_Q(buf, d);
  }
  sprintf(buf, "\r\nEnter race type: "); 
  SEND_TO_Q(buf, d);
}
 
/* Str, Dex, Wis, etc... */
void disp_stats_menu(struct descriptor_data *d) {

#if defined(CLEAR_SCREEN)
"[H[J"
#endif

  sprintf(buf, "\r\n-[Mobile Stats]---------------------\r\n"
	  "%s1%s) Strength    : %s%d\r\n"
	  "%s2%s) Dexterity   : %s%d\r\n"
	  "%s3%s) Constitution: %s%d\r\n"
	  "%s4%s) Intelligence: %s%d\r\n"
	  "%s5%s) Wisdom      : %s%d\r\n"
	  "%s6%s) Charisma    : %s%d\r\n"
	  "%s7%s) Luck        : %s%d\r\n"
	  "%sQ%s) Quit\r\n\r\nEnter choice :",
	  grn, nrm, cyn, GET_STR(OLC_MOB(d)),
	  grn, nrm, cyn, GET_DEX(OLC_MOB(d)),
	  grn, nrm, cyn, GET_CON(OLC_MOB(d)),
	  grn, nrm, cyn, GET_INT(OLC_MOB(d)),
	  grn, nrm, cyn, GET_WIS(OLC_MOB(d)),
	  grn, nrm, cyn, GET_CHA(OLC_MOB(d)),
	  grn, nrm, cyn, GET_LUCK(OLC_MOB(d)),
	  grn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_STATS_MENU;
}


/*-------------------------------------------------------------------*/

/*
 * Display main menu.
 */
void medit_disp_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  int exp_add=0;
  int mtype = 0;
  mob = OLC_MOB(d);
  get_char_cols(d->character);

  mtype = mob->mob_specials.mount_type;

  /* Automatically compute experience. */
  GET_EXP(OLC_MOB(d)) = SUGGEST_EXP(OLC_MOB(d));
  if (IS_AFFECTED(OLC_MOB(d), AFF_SANCTUARY))
    exp_add = GET_EXP(OLC_MOB(d)) * .1;
  if (IS_AFFECTED(OLC_MOB(d), AFF_FIRESHIELD))
    exp_add += GET_EXP(OLC_MOB(d)) * .1;
  if (IS_AFFECTED(OLC_MOB(d), AFF_CHAOSARMOR))
    exp_add += GET_EXP(OLC_MOB(d)) * .05;
  if (IS_AFFECTED(OLC_MOB(d), AFF_DARKWARD))
    exp_add += GET_EXP(OLC_MOB(d)) * .15;
  if (IS_AFFECTED(OLC_MOB(d), AFF_HOLYARMOR))
    exp_add += GET_EXP(OLC_MOB(d)) * .05;

  /* Add bonuses for affections */
  GET_EXP(OLC_MOB(d)) += exp_add;

  sprintf(buf,
#if defined(CLEAR_SCREEN)
"[H[J"
#endif
	  "-- Mob Number:  [%s%d%s]\r\n"
	  "%s1%s) Sex: %s%-7.7s%s	         %s2%s) Alias: %s%s\r\n"
	  "%s3%s) S-Desc: %s%s\r\n"
	  "%s4%s) L-Desc:-\r\n%s%s"
	  "%s5%s) D-Desc:-\r\n%s%s"
     "%s6%s) Level:       [%s%4d%s],  %s7%s) Alignment:    [%s%4d%s]\r\n"
     "%s8%s) Hitroll:     [%s%4d%s],  %s9%s) Damroll:      [%s%4d%s]\r\n"
     "%sA%s) NumDamDice:  [%s%4d%s],  %sB%s) SizeDamDice:  [%s%4d%s]\r\n"
/*     "%sC%s) Num HP Dice: [%s%4d%s],  %sD%s) Size HP Dice: [%s%4d%s],  %sE%s) HP Bonus: [%s%5d%s]\r\n" */
     "%sF%s) Armor Class: [%s%4d%s],  %sG%s) Exp:     [%s%9d%s],  %sH%s) Gold:  [%s%8d%s]\r\n",

	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, genders[(int)GET_SEX(mob)], nrm,
	  grn, nrm, yel, GET_ALIAS(mob),
	  grn, nrm, yel, GET_SDESC(mob),
	  grn, nrm, yel, GET_LDESC(mob),
	  grn, nrm, yel, GET_DDESC(mob),
	  grn, nrm, cyn, GET_LEVEL(mob), nrm,
	  grn, nrm, cyn, GET_ALIGNMENT(mob), nrm,
	  red, nrm, cyn, GET_HITROLL(mob), nrm,
	  red, nrm, cyn, GET_DAMROLL(mob), nrm,
	  red, nrm, cyn, GET_NDD(mob), nrm,
	  red, nrm, cyn, GET_SDD(mob), nrm,
/*	  red, nrm, cyn, GET_HIT(mob), nrm,
	  red, nrm, cyn, GET_MANA(mob), nrm,
	  red, nrm, cyn, GET_MOVE(mob), nrm, */
	  red, nrm, cyn, GET_AC(mob), nrm,
	  red, nrm, cyn, GET_EXP(mob), nrm,
	  grn, nrm, cyn, GET_GOLD(mob), nrm
	  );
  send_to_char(buf, d->character);

  sprintbitarray(MOB_FLAGS(mob), action_bits, AF_ARRAY_MAX, buf1);
  sprintbitarray(AFF_FLAGS(mob), affected_bits, AF_ARRAY_MAX, buf2);
  sprintf(buf,
	  "%sI%s) Position  : %s%s\r\n"
	  "%sJ%s) Default   : %s%s\r\n"
	  "%sK%s) Attack    : %s%s\r\n"
	  "%sL%s) NPC Flags : %s%s\r\n"
	  "%sM%s) AFF Flags : %s%s\r\n"
#if defined(OASIS_MPROG)
	  "%sP%s) Mob Progs : %s%s\r\n"
#endif
	  "%sN%s) Class     : %s%s\r\n"
	  "%sR%s) Race      : %s%s\r\n"
	  "%sO%s) Mob Stats Menu\r\n"
	  "%sP) Mount-Type: /cc%s%s\r\n"
//	  "%sP%s) Abilities Menu%s\r\n"
          "%sS%s) Script    : %s%s\r\n"
	  "%sQ%s) Quit\r\n"

	  "Enter choice : ",

	  grn, nrm, yel, position_types[(int)GET_POS(mob)],
	  grn, nrm, yel, position_types[(int)GET_DEFAULT_POS(mob)],
	  grn, nrm, yel, attack_hit_text[GET_ATTACK(mob)].singular,
	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, buf2,
#if defined(OASIS_MPROG)
	  grn, nrm, cyn, (OLC_MPROGL(d) ? "Set." : "Not Set."),
#endif
          grn, nrm, cyn, npc_class_types[(int)GET_CLASS(mob)], 
          grn, nrm, cyn, npc_race_types[(int)GET_RACE(mob)], 
          grn, nrm,
		  ((IS_SET_AR(MOB_FLAGS(OLC_MOB(d)), MOB_MOUNT)) ? "\x1B[0;32m" : "\x1B[0;31m"), mountt[mtype].name, yel,
          grn, nrm, cyn, mob->proto_script?"Set.":"Not Set.",
	  grn, nrm
	  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = MEDIT_MAIN_MENU;
}

/************************************************************************
 *			The GARGANTAUN event handler			*
 ************************************************************************/

void medit_parse(struct descriptor_data *d, char *arg)
{
  int i;

  if (OLC_MODE(d) > MEDIT_NUMERICAL_RESPONSE) {
    if (!*arg || (!isdigit(arg[0]) && ((*arg == '-') && (!isdigit(arg[1]))))) {
      send_to_char("Field must be numerical, try again : ", d->character);
      return;
    }
  }
  switch (OLC_MODE(d)) {
/*-------------------------------------------------------------------*/
  case MEDIT_CONFIRM_SAVESTRING:
    /*
     * Ensure mob has MOB_ISNPC set or things will go pair shaped.
     */
    SET_BIT_AR(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
    switch (*arg) {
    case 'y':
    case 'Y':
      /*
       * Save the mob in memory and to disk.
       */
      send_to_char("Saving mobile to memory.\r\n", d->character);
      medit_save_internally(d);
      sprintf(buf, "OLC: %s edits mob %d", GET_NAME(d->character), OLC_NUM(d));
      mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      /* FALL THROUGH */
    case 'n':
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      return;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save the mobile? : ", d->character);
      return;
    }
    break;

/*-------------------------------------------------------------------*/
  case MEDIT_MAIN_MENU:
    i = 0;
    switch (*arg) {
    case 'q':
    case 'Q':
      if (OLC_VAL(d)) {	/* Anything been changed? */
	send_to_char("Do you wish to save the changes to the mobile? (y/n) : ", d->character);
	OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
      } else
	cleanup_olc(d, CLEANUP_ALL);
      return;
    case '1':
      OLC_MODE(d) = MEDIT_SEX;
      medit_disp_sex(d);
      return;
    case '2':
      OLC_MODE(d) = MEDIT_ALIAS;
      i--;
      break;
    case '3':
      OLC_MODE(d) = MEDIT_S_DESC;
      i--;
      break;
    case '4':
      OLC_MODE(d) = MEDIT_L_DESC;
      i--;
      break;
    case '5':
      OLC_MODE(d) = MEDIT_D_DESC;
      SEND_TO_Q("Enter mob description: (/s saves /h for help)\r\n\r\n", d);
      d->backstr = NULL;
      if (OLC_MOB(d)->player.description) {
	SEND_TO_Q(OLC_MOB(d)->player.description, d);
	d->backstr = str_dup(OLC_MOB(d)->player.description);
      }
      d->str = &OLC_MOB(d)->player.description;
      d->max_str = MAX_MOB_DESC;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      return;
    case '6':
      OLC_MODE(d) = MEDIT_LEVEL;
      i++;
      break;
    case '7':
      OLC_MODE(d) = MEDIT_ALIGNMENT;
      i++;
      break;
    case 'f':
    case 'F':
      OLC_MODE(d) = MEDIT_AC;
      i++;
      break;
    case 'h':
    case 'H':
      OLC_MODE(d) = MEDIT_GOLD;
      i++;
      break;
    case 'i':
    case 'I':
      OLC_MODE(d) = MEDIT_POS;
      medit_disp_positions(d);
      return;
    case 'j':
    case 'J':
      OLC_MODE(d) = MEDIT_DEFAULT_POS;
      medit_disp_positions(d);
      return;
    case 'k':
    case 'K':
      OLC_MODE(d) = MEDIT_ATTACK;
      medit_disp_attack_types(d);
      return;
    case 'l':
    case 'L':
      OLC_MODE(d) = MEDIT_NPC_FLAGS;
      medit_disp_mob_flags(d);
      return;
    case 'm':
    case 'M':
      OLC_MODE(d) = MEDIT_AFF_FLAGS;
      medit_disp_aff_flags(d);
      return;/*
	case 'p':
    case 'P':
      OLC_MODE(d) = MEDIT_ABILITIES;
      medit_disp_abil_menu(d);
      return;*/
    case 'n':
    case 'N':
      OLC_MODE(d) = MEDIT_CLASS;
      medit_disp_class_flags(d);
      return;
    case 'r':
    case 'R':
      OLC_MODE(d) = MEDIT_RACE;
      medit_disp_race_flags(d);
      return;
    case 'o':
    case 'O':
      disp_stats_menu(d);
      return;
	case 'p':
    case 'P':
		if (IS_SET_AR(MOB_FLAGS(OLC_MOB(d)), MOB_MOUNT))
		{
			OLC_MODE(d) = MEDIT_MOUNTTYPE;
			disp_mtype_menu(d);
		} else
			medit_disp_menu(d);
      return;
#if defined(OASIS_MPROG)
    case 'p':
    case 'P':
      OLC_MODE(d) = MEDIT_MPROG;
      medit_disp_mprog(d);
      return;
#endif
    case 's':
    case 'S':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      return;
    default:
      medit_disp_menu(d);
      return;
    }
    if (i != 0) {
      send_to_char(i == 1 ? "\r\nEnter new value : " :
		   i == -1 ? "\r\nEnter new text :\r\n] " :
			"\r\nOops...:\r\n", d->character);
      return;
    }
    break;

/*-------------------------------------------------------------------*/
  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_ALIAS:
    if (GET_ALIAS(OLC_MOB(d)))
      free(GET_ALIAS(OLC_MOB(d)));
    GET_ALIAS(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_S_DESC:
    if (GET_SDESC(OLC_MOB(d)))
      free(GET_SDESC(OLC_MOB(d)));
    GET_SDESC(OLC_MOB(d)) = str_dup((arg && *arg) ? arg : "undefined");
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_L_DESC:
    if (GET_LDESC(OLC_MOB(d)))
      free(GET_LDESC(OLC_MOB(d)));
    if (arg && *arg) {
      strcpy(buf, arg);
      strcat(buf, "\r\n");
      GET_LDESC(OLC_MOB(d)) = str_dup(buf);
    } else
      GET_LDESC(OLC_MOB(d)) = str_dup("undefined");

    break;
/*-------------------------------------------------------------------*/
  case MEDIT_D_DESC:
    /*
     * We should never get here.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached D_DESC case!",
			BRF, LVL_BUILDER, TRUE);
    send_to_char("Oops...\r\n", d->character);
    break;
/*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
  case MEDIT_MPROG_COMLIST:
    /*
     * We should never get here, but if we do, bail out.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached MPROG_COMLIST case!",
			BRF, LVL_BUILDER, TRUE);
    break;
#endif
/*-------------------------------------------------------------------*/
  case MEDIT_NPC_FLAGS:
    if ((i = atoi(arg)) == 0)
      break;
    else if (!((i < 0) || (i > NUM_MOB_FLAGS)))
	{
      TOGGLE_BIT_AR(MOB_FLAGS(OLC_MOB(d)), i-1);
	  if (i == 19 && !IS_SET_AR(MOB_FLAGS(OLC_MOB(d)), MOB_MOUNT))
		  OLC_MOB(d)->mob_specials.mount_type = 0;
	}
    medit_disp_mob_flags(d);
    return;
/*-------------------------------------------------------------------*/
  case MEDIT_AFF_FLAGS:
    if ((i = atoi(arg)) == 0)
      break;
    else if (!((i < 0) || (i > NUM_AFF_FLAGS-1)))
      TOGGLE_BIT_AR(AFF_FLAGS(OLC_MOB(d)), i);
    medit_disp_aff_flags(d);
    return;
/*-------------------------------------------------------------------*/
#if defined(OASIS_MPROG)
  case MEDIT_MPROG:
    if ((i = atoi(arg)) == 0)
      medit_disp_menu(d);
    else if (i == OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      CREATE(temp, struct mob_prog_data, 1);
      temp->next = OLC_MPROGL(d);
      temp->type = -1;
      temp->arglist = NULL;
      temp->comlist = NULL;
      OLC_MPROG(d) = temp;
      OLC_MPROGL(d) = temp;
      OLC_MODE(d) = MEDIT_CHANGE_MPROG;
      medit_change_mprog (d);
    } else if (i < OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      int x = 1;
      for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
        x++;
      OLC_MPROG(d) = temp;
      OLC_MODE(d) = MEDIT_CHANGE_MPROG;
      medit_change_mprog (d);
    } else if (i == OLC_MTOTAL(d) + 1) {
      send_to_char("Which mob prog do you want to purge? ", d->character);
      OLC_MODE(d) = MEDIT_PURGE_MPROG;
    } else
      medit_disp_menu(d);
    return;

  case MEDIT_PURGE_MPROG:
    if ((i = atoi(arg)) > 0 && i < OLC_MTOTAL(d)) {
      struct mob_prog_data *temp;
      int x = 1;

      for (temp = OLC_MPROGL(d); temp && x < i; temp = temp->next)
	x++;
      OLC_MPROG(d) = temp;
      REMOVE_FROM_LIST(OLC_MPROG(d), OLC_MPROGL(d), next);
      free(OLC_MPROG(d)->arglist);
      free(OLC_MPROG(d)->comlist);
      free(OLC_MPROG(d));
      OLC_MPROG(d) = NULL;
      OLC_VAL(d) = 1;
    }
    medit_disp_mprog(d);
    return;

  case MEDIT_CHANGE_MPROG:
    if ((i = atoi(arg)) == 1)
      medit_disp_mprog_types(d);
    else if (i == 2) {
      send_to_char ("Enter new arg list: ", d->character);
      OLC_MODE(d) = MEDIT_MPROG_ARGS;
    } else if (i == 3) {
      send_to_char("Enter new mob prog commands:\r\n", d->character);
      /*
       * Pass control to modify.c for typing.
       */
      OLC_MODE(d) = MEDIT_MPROG_COMLIST;
      d->backstr = NULL;
      if (OLC_MPROG(d)->comlist) {
        SEND_TO_Q(OLC_MPROG(d)->comlist, d);
        d->backstr = str_dup(OLC_MPROG(d)->comlist);
      }
      d->str = &OLC_MPROG(d)->comlist;
      d->max_str = MAX_STRING_LENGTH;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
    } else
      medit_disp_mprog(d);
    return;
#endif

/*-------------------------------------------------------------------*/

/*
 * Numerical responses.
 */

#if defined(OASIS_MPROG)
/*
  David Klasinc suggests for MEDIT_MPROG_TYPE:
    switch (atoi(arg)) {
      case 0: OLC_MPROG(d)->type = 0; break;
      case 1: OLC_MPROG(d)->type = 1; break;
      case 2: OLC_MPROG(d)->type = 2; break;
      case 3: OLC_MPROG(d)->type = 4; break;
      case 4: OLC_MPROG(d)->type = 8; break;
      case 5: OLC_MPROG(d)->type = 16; break;
      case 6: OLC_MPROG(d)->type = 32; break;
      case 7: OLC_MPROG(d)->type = 64; break;
      case 8: OLC_MPROG(d)->type = 128; break;
      case 9: OLC_MPROG(d)->type = 256; break;
      case 10: OLC_MPROG(d)->type = 512; break;
      case 11: OLC_MPROG(d)->type = 1024; break;
      default: OLC_MPROG(d)->type = -1; break;
    }
*/

  case MEDIT_MPROG_TYPE:
    OLC_MPROG(d)->type = (1 << MAX(0, MIN(atoi(arg), NUM_PROGS - 1)));
    OLC_VAL(d) = 1;
    medit_change_mprog(d);
    return;

  case MEDIT_MPROG_ARGS:
    OLC_MPROG(d)->arglist = str_dup(arg);
    OLC_VAL(d) = 1;
    medit_change_mprog(d);
    return;
#endif

  case MEDIT_SEX:
    GET_SEX(OLC_MOB(d)) = MAX(0, MIN(NUM_GENDERS - 1, atoi(arg)));
    break;

  case MEDIT_AC:
    GET_AC(OLC_MOB(d)) = MAX(-200, MIN(200, atoi(arg)));
    break;

  case MEDIT_GOLD:
    /* restrist to 25% bonus over suggested. */
    GET_GOLD(OLC_MOB(d)) =
           MAX(0, MIN((int)mob_gold[GET_LEVEL(OLC_MOB(d))-1]*1.25, atoi(arg)));
    break;

  case MEDIT_POS:
    GET_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
    break;

  case MEDIT_DEFAULT_POS:
    GET_DEFAULT_POS(OLC_MOB(d)) = MAX(0, MIN(NUM_POSITIONS - 1, atoi(arg)));
    break;

  case MEDIT_ATTACK:
    GET_ATTACK(OLC_MOB(d)) = MAX(0, MIN(NUM_ATTACK_TYPES - 1, atoi(arg)));
    break;

  case MEDIT_LEVEL:
    /* Autostat based on mob level */
    GET_LEVEL(OLC_MOB(d)) = MAX(1, MIN(250, atoi(arg)));
    GET_GOLD(OLC_MOB(d)) = mob_gold[GET_LEVEL(OLC_MOB(d))-1];
    GET_HITROLL(OLC_MOB(d)) = SUGGEST_HR(OLC_MOB(d));
    GET_DAMROLL(OLC_MOB(d)) = (int)(GET_LEVEL(OLC_MOB(d))/1.5);
    GET_NDD(OLC_MOB(d)) = mob_dam_dice[GET_LEVEL(OLC_MOB(d))-1][0];
    GET_SDD(OLC_MOB(d)) = mob_dam_dice[GET_LEVEL(OLC_MOB(d))-1][1];
    GET_AC(OLC_MOB(d)) = mob_ac[GET_LEVEL(OLC_MOB(d))-1];
    break;

  case MEDIT_ALIGNMENT:
    GET_ALIGNMENT(OLC_MOB(d)) = MAX(-1000, MIN(1000, atoi(arg)));
    break;

  case MEDIT_CLASS:
    GET_CLASS(OLC_MOB(d)) = MAX(0, MIN(NUM_NPC_CLASS, atoi(arg)));
	if (atoi(arg) == CLASS_NPC_MAGE || atoi(arg) == CLASS_NPC_DOLL_MAGE) {
		medit_disp_spell_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_SPELLS;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_WARRIOR) {
		medit_disp_skill_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_CLERIC) {
		medit_disp_prayer_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_PRAYERS;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_PALADIN) {
		medit_disp_paladin_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_PALADIN;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_DARK_KNIGHT) {
		medit_disp_dkn_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_DARK_KNIGHT;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_MONK) {
		medit_disp_monk_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_MONK;
		return;
	}
	else if (atoi(arg) == CLASS_NPC_DRUID) {
		medit_disp_skill_menu(d);
		OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	}
    break;

  case MEDIT_RACE:
    GET_RACE(OLC_MOB(d)) = MAX(0, MIN(NUM_NPC_RACE, atoi(arg)));
    break;

  case MEDIT_MOUNTTYPE:
	  if (atoi(arg) > (nr_mount_types-1))
		  disp_mtype_menu(d);
	  OLC_MOB(d)->mob_specials.mount_type = atoi(arg);
	  if (OLC_MOB(d)->mob_specials.mount_type < 0)
		  OLC_MOB(d)->mob_specials.mount_type = 0;
	  medit_disp_menu(d);
	  break;

  case MEDIT_ABIL_PALADIN:
	  switch (*arg) {
	  case '1':
		  medit_disp_skill_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	  case '2':
		  medit_disp_spell_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SPELLS;
		return;
		case 'q':
      case 'Q':
		OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
		return;
      default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_DARK_KNIGHT:
	  switch (*arg) {
	  case '1':
		  medit_disp_skill_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	  case '2':
		  medit_disp_spell_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SPELLS;
		return;
		case 'q':
      case 'Q':
		OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
		return;
      default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_MONK:
	  switch (*arg) {
	  case '1':
		  medit_disp_skill_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	  case '2':
		  medit_disp_chant_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_CHANTS;
		return;
		case 'q':
      case 'Q':
		OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
		return;
      default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABILITIES:
	  switch (*arg) {
	  case '1':
		  medit_disp_skill_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SKILLS;
		return;
	  case '2':
		  medit_disp_spell_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_SPELLS;
		return;
	  case '3':
		  medit_disp_chant_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_CHANTS;
		return;
	  case '4':
		  medit_disp_prayer_menu(d);
		  OLC_MODE(d) = MEDIT_ABIL_PRAYERS;
		return;
	  case 'q':
      case 'Q':
      default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_SKILLS:
	  switch (*arg) {
			case '1':
				show_mob_skills(d, MEDIT_ABIL_SKILL1);
				OLC_MODE(d) = MEDIT_ABIL_SKILL1;
			 return;
			case '2':
				show_mob_skills(d, MEDIT_ABIL_SKILL2);
				OLC_MODE(d) = MEDIT_ABIL_SKILL2;
			 return;
			case '3':
				show_mob_skills(d, MEDIT_ABIL_SKILL3);
				OLC_MODE(d) = MEDIT_ABIL_SKILL3;
			 return;
			case '4':
				show_mob_skills(d, MEDIT_ABIL_SKILL4);
				OLC_MODE(d) = MEDIT_ABIL_SKILL4;
			 return;
			case '5':
				show_mob_skills(d, MEDIT_ABIL_SKILL5);
				OLC_MODE(d) = MEDIT_ABIL_SKILL5;
			 return;
			case 'q':
			case 'Q':
				OLC_VAL(d) = 1;
				medit_disp_menu(d);
				return;
			default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_SPELLS:
	  switch (*arg) {
			case '1':
				show_mob_spells(d, MEDIT_ABIL_SPELL1);
				OLC_MODE(d) = MEDIT_ABIL_SPELL1;
			 return;
			case '2':
				show_mob_spells(d, MEDIT_ABIL_SPELL2);
				OLC_MODE(d) = MEDIT_ABIL_SPELL2;
			 return;
			case '3':
				show_mob_spells(d, MEDIT_ABIL_SPELL3);
				OLC_MODE(d) = MEDIT_ABIL_SPELL3;
			 return;
			case '4':
				show_mob_spells(d, MEDIT_ABIL_SPELL4);
				OLC_MODE(d) = MEDIT_ABIL_SPELL4;
			 return;
			case '5':
				show_mob_spells(d, MEDIT_ABIL_SPELL5);
				OLC_MODE(d) = MEDIT_ABIL_SPELL5;
			 return;
	  		case 'q':
			case 'Q':
				OLC_VAL(d) = 1;
				medit_disp_menu(d);
				return;
			default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_CHANTS:
	  switch (*arg) {
			case '1':
				show_mob_chants(d, MEDIT_ABIL_CHANT1);
				OLC_MODE(d) = MEDIT_ABIL_CHANT1;
			 return;
			case '2':
				show_mob_chants(d, MEDIT_ABIL_CHANT2);
				OLC_MODE(d) = MEDIT_ABIL_CHANT2;
			 return;
			case '3':
				show_mob_chants(d, MEDIT_ABIL_CHANT3);
				OLC_MODE(d) = MEDIT_ABIL_CHANT3;
			 return;
			case '4':
				show_mob_chants(d, MEDIT_ABIL_CHANT4);
				OLC_MODE(d) = MEDIT_ABIL_CHANT4;
			 return;
			case '5':
				show_mob_chants(d, MEDIT_ABIL_CHANT5);
				OLC_MODE(d) = MEDIT_ABIL_CHANT5;
			 return;
	  		case 'q':
			case 'Q':
				OLC_VAL(d) = 1;
				medit_disp_menu(d);
				return;
			default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_PRAYERS:
	  switch (*arg) {
			case '1':
				show_mob_prayers(d, MEDIT_ABIL_PRAYER1);
				OLC_MODE(d) = MEDIT_ABIL_PRAYER1;
			 return;
			case '2':
				show_mob_prayers(d, MEDIT_ABIL_PRAYER2);
				OLC_MODE(d) = MEDIT_ABIL_PRAYER2;
			 break;
			case '3':
				show_mob_prayers(d, MEDIT_ABIL_PRAYER3);
				OLC_MODE(d) = MEDIT_ABIL_PRAYER3;
			 return;
			case '4':
				show_mob_prayers(d, MEDIT_ABIL_PRAYER4);
				OLC_MODE(d) = MEDIT_ABIL_PRAYER4;
			 return;
			case '5':
				show_mob_prayers(d, MEDIT_ABIL_PRAYER5);
				OLC_MODE(d) = MEDIT_ABIL_PRAYER5;
			 return;
	  		case 'q':
			case 'Q':
				OLC_VAL(d) = 1;
				medit_disp_menu(d);
				return;
			default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
	  }
  case MEDIT_ABIL_SKILL1:
	  OLC_MOB(d)->mob_specials.skills[1] = atoi(arg);
	  medit_disp_skill_menu(d);
	  return;
  case MEDIT_ABIL_SKILL2:
	  OLC_MOB(d)->mob_specials.skills[2] = atoi(arg);
	  medit_disp_skill_menu(d);
	  return;
  case MEDIT_ABIL_SKILL3:
	  OLC_MOB(d)->mob_specials.skills[3] = atoi(arg);
	  medit_disp_skill_menu(d);
	  return;
  case MEDIT_ABIL_SKILL4:
	  OLC_MOB(d)->mob_specials.skills[4] = atoi(arg);
	  medit_disp_skill_menu(d);
	  return;
  case MEDIT_ABIL_SKILL5:
	  OLC_MOB(d)->mob_specials.skills[5] = atoi(arg);
	  medit_disp_skill_menu(d);
	  return;
  case MEDIT_ABIL_SPELL1:
	  OLC_MOB(d)->mob_specials.spells[MOB_SKILL_ONE] = atoi(arg);
	  medit_disp_spell_menu(d);
	  return;
  case MEDIT_ABIL_SPELL2:
	  OLC_MOB(d)->mob_specials.spells[MOB_SKILL_TWO] = atoi(arg);
	  medit_disp_spell_menu(d);
	  return;
  case MEDIT_ABIL_SPELL3:
	  OLC_MOB(d)->mob_specials.spells[MOB_SKILL_THREE] = atoi(arg);
	  medit_disp_spell_menu(d);
	  return;
  case MEDIT_ABIL_SPELL4:
	  OLC_MOB(d)->mob_specials.spells[MOB_SKILL_FOUR] = atoi(arg);
	  medit_disp_spell_menu(d);
	  return;
  case MEDIT_ABIL_SPELL5:
	  OLC_MOB(d)->mob_specials.spells[MOB_SKILL_FIVE] = atoi(arg);
	  medit_disp_spell_menu(d);
	  return;
  case MEDIT_ABIL_CHANT1:
	  OLC_MOB(d)->mob_specials.chants[MOB_SKILL_ONE] = atoi(arg);
	  medit_disp_chant_menu(d);
	  return;
  case MEDIT_ABIL_CHANT2:
	  OLC_MOB(d)->mob_specials.chants[MOB_SKILL_TWO] = atoi(arg);
	  medit_disp_chant_menu(d);
	  return;
  case MEDIT_ABIL_CHANT3:
	  OLC_MOB(d)->mob_specials.chants[MOB_SKILL_THREE] = atoi(arg);
	  medit_disp_chant_menu(d);
	  return;
  case MEDIT_ABIL_CHANT4:
	  OLC_MOB(d)->mob_specials.chants[MOB_SKILL_FOUR] = atoi(arg);
	  medit_disp_chant_menu(d);
	  return;
  case MEDIT_ABIL_CHANT5:
	  OLC_MOB(d)->mob_specials.chants[MOB_SKILL_FIVE] = atoi(arg);
	  medit_disp_chant_menu(d);
	  return;
  case MEDIT_ABIL_PRAYER1:
	  OLC_MOB(d)->mob_specials.prayers[MOB_SKILL_ONE] = atoi(arg);
	  medit_disp_prayer_menu(d);
	  return;
  case MEDIT_ABIL_PRAYER2:
	  OLC_MOB(d)->mob_specials.prayers[MOB_SKILL_TWO] = atoi(arg);
	  medit_disp_prayer_menu(d);
	  return;
  case MEDIT_ABIL_PRAYER3:
	  OLC_MOB(d)->mob_specials.prayers[MOB_SKILL_THREE] = atoi(arg);
	  medit_disp_prayer_menu(d);
	  return;
  case MEDIT_ABIL_PRAYER4:
	  OLC_MOB(d)->mob_specials.prayers[MOB_SKILL_FOUR] = atoi(arg);
	  medit_disp_prayer_menu(d);
	  return;
  case MEDIT_ABIL_PRAYER5:
	  OLC_MOB(d)->mob_specials.prayers[MOB_SKILL_FIVE] = atoi(arg);
	  medit_disp_prayer_menu(d);
	  return;
  case MEDIT_STATS_MENU:
    switch (*arg) {
      case '1':
        send_to_char("Strength : ", d->character);
        OLC_MODE(d) = MEDIT_STR;
        return;
      case '2':
        send_to_char("Dexterity : ", d->character);
        OLC_MODE(d) = MEDIT_DEX;
        return;
      case '3':
        send_to_char("Constitution : ", d->character);
        OLC_MODE(d) = MEDIT_CON;
        return;
      case '4':
        send_to_char("Intelligence : ", d->character);
        OLC_MODE(d) = MEDIT_INT;
        return;
      case '5':
        send_to_char("Wisdom : ", d->character);
        OLC_MODE(d) = MEDIT_WIS;
        return;
      case '6':
        send_to_char("Charisma : ", d->character);
        OLC_MODE(d) = MEDIT_CHA;
        return;
      case '7':
        send_to_char("Luck : ", d->character);
        OLC_MODE(d) = MEDIT_LUCK;
        return;
      case 'q':
      case 'Q':
      default:
        OLC_VAL(d) = 1;
        medit_disp_menu(d); /* Return to main menu */
        return;
    }
  case MEDIT_STR:
    GET_STR(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_DEX:
    GET_DEX(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_CON:
    GET_CON(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_INT:
    GET_INT(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_WIS:
    GET_WIS(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_CHA:
    GET_CHA(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
  case MEDIT_LUCK:
    GET_LUCK(OLC_MOB(d)) = MIN(25, (MAX(6, atoi(arg))));
    disp_stats_menu(d);
    return;
/*-------------------------------------------------------------------*/
  default:
    /*
     * We should never get here.
     */
    cleanup_olc(d, CLEANUP_ALL);
    mudlog("SYSERR: OLC: medit_parse(): Reached default case!", BRF, LVL_BUILDER, TRUE);
    send_to_char("Oops...\r\n", d->character);
    break;
  }
/*-------------------------------------------------------------------*/

/*
 * END OF CASE 
 * If we get here, we have probably changed something, and now want to
 * return to main menu.  Use OLC_VAL as a 'has changed' flag  
 */

  OLC_VAL(d) = 1;
  medit_disp_menu(d);
}
/*
 * End of medit_parse(), thank god.
 */
/* Below: Functions for mob skills... */

void medit_disp_abil_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  mob = OLC_MOB(d);
  sprintf(buf,
	  "%s--==={%s Mob Abilities Menu%s }===--\r\n"
	  "%s1%s)%s Skills %s\r\n"
	  "%s2%s)%s Spells %s\r\n"
	  "%s3%s)%s Chants %s\r\n"
	  "%s4%s)%s Prayers %s\r\n"
	  "%sQ%s)%s Exit abilities menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel,
	  cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABILITIES;
}

void medit_disp_paladin_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  mob = OLC_MOB(d);
  sprintf(buf,
	  "%s--==={%s Paladin Abilities Menu%s }===--\r\n"
	  "%s1%s)%s Skills %s\r\n"
	  "%s2%s)%s Spells %s\r\n"
	  "%sQ%s)%s Exit abilities menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel,
	  cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_PALADIN;
}
void medit_disp_dkn_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  mob = OLC_MOB(d);
  sprintf(buf,
	  "%s--==={%s Dark-Knight Abilities Menu%s }===--\r\n"
	  "%s1%s)%s Skills %s\r\n"
	  "%s2%s)%s Spells %s\r\n"
	  "%sQ%s)%s Exit dark-knight abilities menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel,
	  cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_PALADIN;
}
void medit_disp_monk_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  mob = OLC_MOB(d);
  sprintf(buf,
	  "%s--==={%s Monk Abilities Menu%s }===--\r\n"
	  "%s1%s)%s Skills %s\r\n"
	  "%s2%s)%s Chants %s\r\n"
	  "%sQ%s)%s Exit monk abilities menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel, nrm,
	  cyn, grn, yel,
	  cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_MONK;
}
void medit_disp_skill_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  char **abil_list;
  char buf1[40], buf2[40], buf3[40], buf4[40], buf5[40];
  int nr1=0,nr2=0,nr3=0,nr4=0,nr5=0;
  mob = OLC_MOB(d);
  abil_list = skills;
    if (mob->mob_specials.skills[MOB_SKILL_ONE] != 0) {
	  nr1 = mob->mob_specials.skills[MOB_SKILL_ONE];
	  sprintf(buf1, "%s", abil_list[nr1]);
  } else {
	  sprintf(buf1, "<none>");
  }
  if (mob->mob_specials.skills[MOB_SKILL_TWO] != 0) {
	  nr2 = mob->mob_specials.skills[MOB_SKILL_TWO];
	  sprintf(buf2, "%s", abil_list[nr2]);
  } else {
	  sprintf(buf2, "<none>");
  }
  if (mob->mob_specials.skills[MOB_SKILL_THREE] != 0) {
	  nr3 = mob->mob_specials.skills[MOB_SKILL_THREE];
	  sprintf(buf3, "%s", abil_list[nr3]);
  } else {
	  sprintf(buf3, "<none>");
  }
  if (mob->mob_specials.skills[MOB_SKILL_FOUR] != 0) {
	  nr4 = mob->mob_specials.skills[MOB_SKILL_FOUR];
	  sprintf(buf4, "%s", abil_list[nr4]);
  } else {
	  sprintf(buf4, "<none>");
  }
  if (mob->mob_specials.skills[MOB_SKILL_FIVE] != 0) {
	  nr5 = mob->mob_specials.skills[MOB_SKILL_FIVE];
	  sprintf(buf5, "%s", abil_list[nr5]);
  } else {
	  sprintf(buf5, "<none>");
  }
  sprintf(buf,
	  "%s--==={%s Mob Skills Menu%s }===--\r\n"
	  "%s1%s)%s Skill 1 : %s%s\r\n"
	  "%s2%s)%s Skill 2 : %s%s\r\n"
	  "%s3%s)%s Skill 3 : %s%s\r\n"
	  "%s4%s)%s Skill 4 : %s%s\r\n"
	  "%s5%s)%s Skill 5 : %s%s\r\n"
	  "%sQ%s)%s Exit skill menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, nrm, yel, cyn, buf1,
	  cyn, nrm, yel, cyn, buf2,
	  cyn, nrm, yel, cyn, buf3,
	  cyn, nrm, yel, cyn, buf4,
	  cyn, nrm, yel, cyn, buf5,
	  cyn, nrm, yel, cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_SKILLS;
}

void medit_disp_spell_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  char **abil_list;
  char buf1[40], buf2[40], buf3[40], buf4[40], buf5[40];
  int nr1=0,nr2=0,nr3=0,nr4=0,nr5=0;
  mob = OLC_MOB(d);
  abil_list = spells;
  if (mob->mob_specials.spells[MOB_SKILL_ONE] != 0) {
	  nr1 = mob->mob_specials.spells[MOB_SKILL_ONE];
	  sprintf(buf1, "%s", abil_list[nr1]);
  } else {
	  sprintf(buf1, "<none>");
  }
  if (mob->mob_specials.spells[MOB_SKILL_TWO] != 0) {
	  nr2 = mob->mob_specials.spells[MOB_SKILL_TWO];
	  sprintf(buf2, "%s", abil_list[nr2]);
  } else {
	  sprintf(buf2, "<none>");
  }
  if (mob->mob_specials.spells[MOB_SKILL_THREE] != 0) {
	  nr3 = mob->mob_specials.spells[MOB_SKILL_THREE];
	  sprintf(buf3, "%s", abil_list[nr3]);
  } else {
	  sprintf(buf3, "<none>");
  }
  if (mob->mob_specials.spells[MOB_SKILL_FOUR] != 0) {
	  nr4 = mob->mob_specials.spells[MOB_SKILL_FOUR];
	  sprintf(buf4, "%s", abil_list[nr4]);
  } else {
	  sprintf(buf4, "<none>");
  }
  if (mob->mob_specials.spells[MOB_SKILL_FIVE] != 0) {
	  nr5 = mob->mob_specials.spells[MOB_SKILL_FIVE];
	  sprintf(buf5, "%s", abil_list[nr5]);
  } else {
	  sprintf(buf5, "<none>");
  }
  sprintf(buf,
	  "%s--==={%s Mob Spells Menu%s }===--\r\n"
	  "%s1%s)%s Spell 1 : %s%s\r\n"
	  "%s2%s)%s Spell 2 : %s%s\r\n"
	  "%s3%s)%s Spell 3 : %s%s\r\n"
	  "%s4%s)%s Spell 4 : %s%s\r\n"
	  "%s5%s)%s Spell 5 : %s%s\r\n"
	  "%sQ%s)%s Exit spell menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, nrm, yel, cyn, buf1,
	  cyn, nrm, yel, cyn, buf2,
	  cyn, nrm, yel, cyn, buf3,
	  cyn, nrm, yel, cyn, buf4,
	  cyn, nrm, yel, cyn, buf5,
	  cyn, nrm, yel, cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_SPELLS;
}

void medit_disp_chant_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  char **abil_list;
  char buf1[40], buf2[40], buf3[40], buf4[40], buf5[40];
  int nr1=0,nr2=0,nr3=0,nr4=0,nr5=0;
  mob = OLC_MOB(d);
  abil_list = chants;
  if (mob->mob_specials.chants[MOB_SKILL_ONE] != 0) {
	  nr1 = mob->mob_specials.chants[MOB_SKILL_ONE];
	  sprintf(buf1, "%s", abil_list[nr1]);
  } else {
	  sprintf(buf1, "<none>");
  }
  if (mob->mob_specials.chants[MOB_SKILL_TWO] != 0) {
	  nr2 = mob->mob_specials.chants[MOB_SKILL_TWO];
	  sprintf(buf2, "%s", abil_list[nr2]);
  } else {
	  sprintf(buf2, "<none>");
  }
  if (mob->mob_specials.chants[MOB_SKILL_THREE] != 0) {
	  nr3 = mob->mob_specials.chants[MOB_SKILL_THREE];
	  sprintf(buf3, "%s", abil_list[nr3]);
  } else {
	  sprintf(buf3, "<none>");
  }
  if (mob->mob_specials.chants[MOB_SKILL_FOUR] != 0) {
	  nr4 = mob->mob_specials.chants[MOB_SKILL_FOUR];
	  sprintf(buf4, "%s", abil_list[nr4]);
  } else {
	  sprintf(buf4, "<none>");
  }
  if (mob->mob_specials.chants[MOB_SKILL_FIVE] != 0) {
	  nr5 = mob->mob_specials.chants[MOB_SKILL_FIVE];
	  sprintf(buf5, "%s", abil_list[nr5]);
  } else {
	  sprintf(buf5, "<none>");
  }
  sprintf(buf,
	  "%s--==={%s Mob Chants Menu%s }===--\r\n"
	  "%s1%s)%s Chant 1 : %s%s\r\n"
	  "%s2%s)%s Chant 2 : %s%s\r\n"
	  "%s3%s)%s Chant 3 : %s%s\r\n"
	  "%s4%s)%s Chant 4 : %s%s\r\n"
	  "%s5%s)%s Chant 5 : %s%s\r\n"
	  "%sQ%s)%s Exit chant menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, nrm, yel, cyn, buf1,
	  cyn, nrm, yel, cyn, buf2,
	  cyn, nrm, yel, cyn, buf3,
	  cyn, nrm, yel, cyn, buf4,
	  cyn, nrm, yel, cyn, buf5,
	  cyn, nrm, yel, cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_CHANTS;
}

void medit_disp_prayer_menu(struct descriptor_data *d)
{
  struct char_data *mob;
  char **abil_list;
  char buf1[40], buf2[40], buf3[40], buf4[40], buf5[40];
  int nr1=0,nr2=0,nr3=0,nr4=0,nr5=0;
  mob = OLC_MOB(d);
  abil_list = prayers;
    if (mob->mob_specials.prayers[MOB_SKILL_ONE] != 0) {
	  nr1 = mob->mob_specials.prayers[MOB_SKILL_ONE];
	  sprintf(buf1, "%s", abil_list[nr1]);
  } else {
	  sprintf(buf1, "<none>");
  }
  if (mob->mob_specials.prayers[MOB_SKILL_TWO] != 0) {
	  nr2 = mob->mob_specials.prayers[MOB_SKILL_TWO];
	  sprintf(buf2, "%s", abil_list[nr2]);
  } else {
	  sprintf(buf2, "<none>");
  }
  if (mob->mob_specials.prayers[MOB_SKILL_THREE] != 0) {
	  nr3 = mob->mob_specials.prayers[MOB_SKILL_THREE];
	  sprintf(buf3, "%s", abil_list[nr3]);
  } else {
	  sprintf(buf3, "<none>");
  }
  if (mob->mob_specials.prayers[MOB_SKILL_FOUR] != 0) {
	  nr4 = mob->mob_specials.prayers[MOB_SKILL_FOUR];
	  sprintf(buf4, "%s", abil_list[nr4]);
  } else {
	  sprintf(buf4, "<none>");
  }
  if (mob->mob_specials.prayers[MOB_SKILL_FIVE] != 0) {
	  nr5 = mob->mob_specials.prayers[MOB_SKILL_FIVE];
	  sprintf(buf5, "%s", abil_list[nr5]);
  } else {
	  sprintf(buf5, "<none>");
  }
  sprintf(buf,
	  "%s--==={%s Mob Prayers Menu%s }===--\r\n"
	  "%s1%s)%s Prayer 1 : %s%s\r\n"
	  "%s2%s)%s Prayer 2 : %s%s\r\n"
	  "%s3%s)%s Prayer 3 : %s%s\r\n"
	  "%s4%s)%s Prayer 4 : %s%s\r\n"
	  "%s5%s)%s Prayer 5 : %s%s\r\n"
	  "%sQ%s)%s Exit prayer menu\r\n"
	  "%sEnter choice : %s",
	  cyn, yel, cyn,
	  cyn, nrm, yel, cyn, buf1,
	  cyn, nrm, yel, cyn, buf2,
	  cyn, nrm, yel, cyn, buf3,
	  cyn, nrm, yel, cyn, buf4,
	  cyn, nrm, yel, cyn, buf5,
	  cyn, nrm, yel, cyn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = MEDIT_ABIL_PRAYERS;
}

void show_mob_skills(struct descriptor_data *d, int medit_num)
{
	char **abil_list;
	struct char_data *ch;
	int i=0, ct=0, sortpos=0, columns=0;
	abil_list = skills;
	ch = d->character;
	send_to_char("\r\n", ch);
	for (ct=0, sortpos = 1; sortpos < MAX_SKILLS; sortpos++) {
      i = sortpos;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
		  strcat(buf2, "**OVERFLOW**\r\n");
		  break;
	  }
      sprintf(buf2, "%s%3d%s)%s %-20.20s    %s%s", cyn, i, grn, yel, abil_list[i], nrm, !(++columns % 3) ? "\r\n" : "");
	  send_to_char(buf2, d->character);
	  ct++;
	}
  send_to_char("\r\n", d->character);
  send_to_char("/ccSelect a skill (/cy0 for none/cc) : /c0", d->character);
  send_to_char("\r\n", d->character);
  OLC_MODE(d) = medit_num;
}

void show_mob_spells(struct descriptor_data *d, int medit_num)
{
	char **abil_list;
	struct char_data *ch;
	int i=0, ct=0, sortpos=0, columns=0;
	abil_list = spells;
	ch = d->character;
	send_to_char("\r\n", ch);
	for (ct=0, sortpos = 1; sortpos < MAX_SPELLS; sortpos++) {
      i = sortpos;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
		  strcat(buf2, "**OVERFLOW**\r\n");
		  break;
	  }
	  if (IS_SET(abil_info[ABT_SPELL][i].routines, MAG_DAMAGE) || IS_SET(abil_info[ABT_SPELL][i].routines, MAG_AREAS)) {
		  sprintf(buf2, "%s%3d%s)%s %-20.20s    %s%s", cyn, i, grn, yel, abil_list[i], nrm, !(++columns % 3) ? "\r\n" : "");
		  send_to_char(buf2, d->character);
		  ct++;
	  }
	}
  send_to_char("\r\n", d->character);
  send_to_char("/ccSelect a spell (/cy0 for none/cc) : /c0", d->character);
  send_to_char("\r\n", d->character);
  OLC_MODE(d) = medit_num;
}

void show_mob_chants(struct descriptor_data *d, int medit_num)
{
	char **abil_list;
	struct char_data *ch;
	int i=0, ct=0, sortpos=0, columns=0;
	abil_list = chants;
	ch = d->character;
	send_to_char("\r\n", ch);
	for (ct=0, sortpos = 1; sortpos < MAX_CHANTS; sortpos++) {
      i = sortpos;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
		  strcat(buf2, "**OVERFLOW**\r\n");
		  break;
	  }
      if (IS_SET(abil_info[ABT_CHANT][i].routines, MAG_DAMAGE) || IS_SET(abil_info[ABT_CHANT][i].routines, MAG_AREAS)) {
		  sprintf(buf2, "%s%3d%s)%s %-20.20s    %s%s", cyn, i, grn, yel, abil_list[i], nrm, !(++columns % 3) ? "\r\n" : "");
		  send_to_char(buf2, d->character);
		  ct++;
	  }
	}
  send_to_char("\r\n", ch);
  send_to_char("/ccSelect a chant (/cy0 for none/cc) : /c0", d->character);
  send_to_char("\r\n", d->character);
  OLC_MODE(d) = medit_num;
}

void show_mob_prayers(struct descriptor_data *d, int medit_num)
{
	char **abil_list;
	struct char_data *ch;
	int i=0, ct=0, sortpos=0, columns=0;
	abil_list = prayers;
	ch = d->character;
	send_to_char("\r\n", ch);
	for (ct=0, sortpos = 1; sortpos < MAX_PRAYERS; sortpos++) {
      i = sortpos;
      if (strlen(buf2) >= MAX_STRING_LENGTH - 32) {
		  strcat(buf2, "**OVERFLOW**\r\n");
		  break;
	  }
      if (IS_SET(abil_info[ABT_PRAYER][i].routines, MAG_DAMAGE) || IS_SET(abil_info[ABT_PRAYER][i].routines, MAG_AREAS)) {
		  sprintf(buf2, "%s%3d%s)%s %-20.20s    %s%s", cyn, i, grn, yel, abil_list[i], nrm, !(++columns % 3) ? "\r\n" : "");
		  send_to_char(buf2, d->character);
		  ct++;
	  }
	}
  send_to_char("\r\n", ch);
  send_to_char("/ccSelect a prayer (/cy0 for none/cc) : /c0", d->character);
  send_to_char("\r\n", d->character);
  OLC_MODE(d) = medit_num;
}

void show_mob_abilities(struct descriptor_data *d, int type)
{
	char **abil_list;
	char buf1[10];
	int i=0, columns = 0;
	switch (type) {
    case ABT_SKILL:
		abil_list = skills;
		sprintf(buf1, "skill");
		break;
    case ABT_SPELL:
		abil_list = spells;
		sprintf(buf1, "spell");
		break;
    case ABT_CHANT:
		abil_list = chants;
		sprintf(buf1, "chant");
		break;
    case ABT_PRAYER:
		abil_list = prayers;
		sprintf(buf1, "prayer");
		break;
    default:
      send_to_char("Something fucked up.\r\n", d->character);
      return;
      break;
  }
	for(i=0; i < MAX_ABILITIES; i++) {
			sprintf(buf, "%2d) %-20.20s  %s", i, abil_list[i],
			!(++columns % 3) ? "\r\n" : "");
			send_to_char(buf, d->character);
	}
	sprintf(buf, "What %s do you wish to set?(0 to quit) : ", buf1);
  send_to_char(buf, d->character);
  send_to_char("\r\n", d->character);
}

void disp_mtype_menu(struct descriptor_data *d)
{
	int i = 0;
	char output[MAX_STRING_LENGTH];
	*output = '\0';
	for(i = 1; i < (nr_mount_types-1);i++)
		sprintf(output, "%s/cc%3d)/cy %-15.15s/c0\r\n", output, i, mountt[i].name);
	send_to_char(output, d->character);
	send_to_char("/ccWhat type of mount should this mobile be? :  /c0", d->character);
}
