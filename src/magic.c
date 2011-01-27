/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
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
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "dg_scripts.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *obj_index;

extern struct weather_data weather_info;
extern struct descriptor_data *descriptor_list;
extern struct zone_data *zone_table;

extern int mini_mud;
extern int pk_allowed;
extern int cha_max_followers[26];

struct message_list spell_messages[MAX_MESSAGES]; /* spell messages    */

extern struct default_mobile_stats *mob_defaults;
extern char weapon_verbs[];
extern int *max_ac_applys;
extern struct apply_mod_defaults *apmd;
extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];

void clearMemory(struct char_data * ch);
void old_act(char *str, int i, struct char_data * c, struct obj_data * o,
	      void *vict_obj, int j);
void act(const char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, const void *vict_obj, int type);

void weight_change_object(struct obj_data * obj, int weight);
void add_follower(struct char_data * ch, struct char_data * leader);
int dice(int number, int size);
extern struct spell_info_type spell_info[];


struct char_data *read_mobile(int, int);


/*
 * Saving throws for:
 * MCTW
 *   PARA, ROD, PETRI, BREATH, SPELL
 *     Levels 0-60 plus one more (for zero)
 */

const byte saving_throws[NUM_CLASSES][5][61] = {

  { /* Sorcerer */
           {90, 80, 79, 77, 75, 72, 69, 67, 64, 62, 60,	        /*  0 - 10 */
/* PARA */	58, 56, 54, 53, 53, 52, 52, 51, 51, 50,		/* 11 - 20 */
		49, 48, 47, 46, 45, 44, 43, 42, 40, 39,		/* 21 - 30 */
                39, 38, 38, 37, 37, 36, 36, 35, 34, 33,	        /* 31 - 40 */
                33, 32, 32, 31, 31, 30, 30, 29, 29, 28,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,         /*  0 - 10 */
/* ROD */	48, 47, 47, 46, 45, 44, 43, 42, 41, 40,		/* 11 - 20 */
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30,		/* 21 - 30 */
                29, 28, 27, 26, 25, 24, 23, 22, 21, 20,	        /* 31 - 40 */
                19, 18, 17, 16, 15, 14, 13, 12, 11,  9,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 69, 68, 67, 65, 64,	        /*  0 - 10 */
/* PETRI */	63, 62, 61, 60, 59, 58, 57, 56, 55, 54,		/* 11 - 20 */
		53, 52, 51, 50, 48, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 37, 36, 35, 34, 32, 30, 28, 27,	        /* 31 - 40 */
                26, 25, 23, 22, 20, 18, 17, 16, 15, 13,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 81, 79, 78, 76, 75, 74, 73,	        /*  0 - 10 */
/* BREATH */	57, 56, 55, 54, 53, 52, 51, 50, 49, 48,		/* 11 - 20 */
		47, 46, 45, 43, 42, 41, 40, 39, 38, 36,		/* 21 - 30 */
                35, 34, 33, 32, 31, 31, 30, 30, 29, 29,	        /* 31 - 40 */
                29, 28, 27, 26, 25, 25, 24, 24, 24, 22,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,         /*  0 - 10 */
/* SPELL */	60, 59, 58, 57, 56, 55, 52, 50, 49, 47,		/* 11 - 20 */
		46, 44, 42, 41, 40, 38, 36, 35, 33, 31,		/* 21 - 30 */
                29, 28, 27, 26, 26, 25, 24, 23, 22, 21,	        /* 31 - 40 */
                20, 19, 18, 16, 14, 12, 11, 10,  9,  8,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */
  },

  { /* Clerics */
           {90, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,	        /*  0 - 10 */
/* PARA */	60, 59, 58, 57, 56, 55, 54, 54, 52, 51,		/* 11 - 20 */
		50, 49, 48, 46, 45, 44, 43, 41, 40, 39,		/* 21 - 30 */
                38, 37, 35, 34, 32, 31, 30, 28, 27, 26,	        /* 31 - 40 */
                25, 23, 21, 19, 17, 15, 13, 11,  9,  7,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,         /*  0 - 10 */
/* ROD */	65, 64, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		55, 54, 53, 52, 51, 50, 49, 48, 47, 46,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 39, 38, 37, 36,	        /* 31 - 40 */
                35, 34, 34, 33, 32, 31, 30, 29, 28, 27,	        /* 41 - 50 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,         /*  0 - 10 */
/* PETRI */	64, 63, 62, 61, 60, 59, 58, 57, 56, 55,		/* 11 - 20 */
		54, 53, 52, 51, 49, 47, 45, 44, 43, 42,		/* 21 - 30 */
                40, 39, 37, 36, 35, 34, 33, 32, 31, 30,	        /* 31 - 40 */
                29, 28, 27, 27, 26, 25, 24, 24, 23, 22,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81,         /*  0 - 10 */
/* BREATH */	80, 79, 78, 77, 76, 75, 74, 73, 72, 70,		/* 11 - 20 */
		69, 68, 67, 65, 64, 64, 63, 63, 62, 60,		/* 21 - 30 */
                59, 58, 58, 57, 56, 55, 54, 53, 52, 51,	        /* 31 - 40 */
                50, 49, 47, 46, 44, 42, 41, 39, 38, 37,	        /* 41 - 50 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76,         /*  0 - 10 */
/* SPELL */	75, 74, 73, 72, 71, 70, 69, 68, 68, 67,		/* 11 - 20 */
		66, 65, 63, 62, 60, 59, 58, 57, 56, 55,		/* 21 - 30 */
                54, 53, 52, 51, 50, 49, 48, 47, 45, 44,	        /* 31 - 40 */
                43, 42, 41, 40, 39, 38, 36, 35, 34, 32,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */
  },

  { /* Thieves */
           {90, 75, 75, 74, 74, 73, 73, 72, 71, 70, 69,         /*  0 - 10 */
/* PARA */	68, 67, 66, 65, 64, 63, 62, 61, 60, 59,		/* 11 - 20 */
		58, 57, 56, 55, 54, 53, 52, 51, 50, 49,		/* 21 - 30 */
                48, 47, 46, 45, 44, 43, 42, 42, 41, 41,	        /* 31 - 40 */
                40, 39, 38, 37, 37, 38, 38, 37, 37, 36,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 80, 79, 78, 76, 74, 73, 72, 71, 70, 69,         /*  0 - 10 */
/* ROD */	68, 66, 64, 63, 61, 59, 57, 55, 53, 52,		/* 11 - 20 */
		51, 50, 48, 46, 44, 43, 41, 40, 38, 37,		/* 21 - 30 */
                35, 33, 31, 29, 28, 27, 26, 25, 24, 24,	        /* 31 - 40 */
                23, 22, 21, 20, 19, 18, 17, 16, 15, 13,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 70, 69, 68, 68, 67, 66, 65, 64, 63, 62,         /*  0 - 10 */
/* PETRI */	63, 62, 61, 60, 59, 59, 58, 57, 57, 56,		/* 11 - 20 */
		55, 55, 54, 54, 53, 52, 52, 51, 51, 50,		/* 21 - 30 */
                49, 49, 48, 47, 46, 44, 43, 42, 41, 40,	        /* 31 - 40 */
                41, 40, 39, 38, 37, 35, 34, 33, 32, 31,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80,         /*  0 - 10 */
/* BREATH */	80, 79, 79, 79, 78, 78, 77, 76, 76, 75,		/* 11 - 20 */
		74, 73, 72, 71, 70, 69, 68, 68, 67, 66,		/* 21 - 30 */
                65, 64, 63, 62, 61, 60, 59, 58, 57, 56,	        /* 31 - 40 */
                55, 55, 54, 54, 54, 53, 53, 52, 52, 51,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 81, 80, 79, 78, 76, 75, 74,         /*  0 - 10 */
/* SPELL */	73, 73, 72, 71, 70, 68, 67, 66, 63, 62,		/* 11 - 20 */
		60, 59, 57, 56, 55, 54, 53, 51, 48, 47,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 38, 37, 36, 34,	        /* 31 - 40 */
                33, 32, 31, 30, 29, 28, 27, 26, 25, 23,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */
  },

  { /* Gladiators */
           {90, 80, 79, 78, 77, 76, 75, 73, 72, 71, 70,         /*  0 - 10 */
/* PARA */	68, 67, 66, 65, 64, 63, 62, 61, 60, 59,		/* 11 - 20 */
		58, 56, 55, 54, 53, 51, 50, 49, 48, 46,		/* 21 - 30 */
                45, 44, 42, 41, 39, 38, 36, 35, 34, 32,	        /* 31 - 40 */
                30, 28, 27, 26, 25, 24, 23, 21, 19, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 85, 84, 83, 81, 79, 78, 76, 74, 73, 71,         /*  0 - 10 */
/* ROD */	69, 67, 66, 64, 63, 62, 60, 58, 58, 57,		/* 11 - 20 */
		56, 54, 52, 51, 49, 47, 48, 47, 45, 44,		/* 21 - 30 */
                43, 42, 41, 40, 39, 37, 36, 35, 33, 31,	        /* 31 - 40 */
                30, 30, 29, 28, 27, 26, 25, 24, 23, 22,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 80, 79, 77, 75, 74, 73, 72, 71, 70, 68,         /*  0 - 10 */
/* PETRI */	67, 66, 64, 63, 62, 61, 60, 59, 57, 56,		/* 11 - 20 */
		55, 53, 52, 50, 49, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 38, 37, 36, 35, 34, 33, 32, 32,	        /* 31 - 40 */
                33, 32, 31, 30, 28, 27, 26, 24, 20, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 80, 79, 77, 75, 74, 73, 72, 71, 70, 68,         /*  0 - 10 */
/* BREATH */	67, 66, 64, 63, 62, 61, 60, 59, 57, 56,		/* 11 - 20 */
		55, 53, 52, 50, 49, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 38, 37, 36, 35, 34, 33, 32, 32,	        /* 31 - 40 */
                33, 32, 31, 30, 28, 27, 26, 24, 20, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 85, 84, 83, 81, 80, 79, 78, 76, 75, 74,         /*  0 - 10 */
/* SPELL */	73, 73, 72, 71, 70, 68, 67, 66, 63, 62,		/* 11 - 20 */
		60, 59, 57, 56, 55, 54, 53, 51, 48, 47,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 38, 37, 36, 34,	        /* 31 - 40 */
                33, 32, 31, 30, 29, 28, 27, 26, 25, 23,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */
  },

  { /* Druid */
           {90, 75, 75, 74, 74, 73, 73, 72, 71, 70, 69,         /*  0 - 10 */
/* PARA */	68, 67, 66, 65, 64, 63, 62, 61, 60, 59,		/* 11 - 20 */
		58, 57, 56, 55, 54, 53, 52, 51, 50, 49,		/* 21 - 30 */
                48, 47, 46, 45, 44, 43, 42, 42, 41, 41,	        /* 31 - 40 */
                40, 39, 38, 37, 37, 38, 38, 37, 37, 36,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 80, 79, 78, 76, 74, 73, 72, 71, 70, 69,         /*  0 - 10 */
/* ROD */	68, 66, 64, 63, 61, 59, 57, 55, 53, 52,		/* 11 - 20 */
		51, 50, 48, 46, 44, 43, 41, 40, 38, 37,		/* 21 - 30 */
                35, 33, 31, 29, 28, 27, 26, 25, 24, 24,	        /* 31 - 40 */
                23, 22, 21, 20, 19, 18, 17, 16, 15, 13,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 70, 69, 68, 68, 67, 66, 65, 64, 63, 62,         /*  0 - 10 */
/* PETRI */	63, 62, 61, 60, 59, 59, 58, 57, 57, 56,		/* 11 - 20 */
		55, 55, 54, 54, 53, 52, 52, 51, 51, 50,		/* 21 - 30 */
                49, 49, 48, 47, 46, 44, 43, 42, 41, 40,	        /* 31 - 40 */
                41, 40, 39, 38, 37, 35, 34, 33, 32, 31,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 85, 84, 84, 83, 82, 82, 81, 81, 80,         /*  0 - 10 */
/* BREATH */	80, 79, 79, 79, 78, 78, 77, 76, 76, 75,		/* 11 - 20 */
		74, 73, 72, 71, 70, 69, 68, 68, 67, 66,		/* 21 - 30 */
                65, 64, 63, 62, 61, 60, 59, 58, 57, 56,	        /* 31 - 40 */
                55, 55, 54, 54, 54, 53, 53, 52, 52, 51,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 81, 80, 79, 78, 76, 75, 74,         /*  0 - 10 */
/* SPELL */	73, 73, 72, 71, 70, 68, 67, 66, 63, 62,		/* 11 - 20 */
		60, 59, 57, 56, 55, 54, 53, 51, 48, 47,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 38, 37, 36, 34,	        /* 31 - 40 */
                33, 32, 31, 30, 29, 28, 27, 26, 25, 23,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */
  },

  { /* Death Knight */
           {90, 80, 79, 77, 75, 72, 69, 67, 64, 62, 60,	        /*  0 - 10 */
/* PARA */	58, 56, 54, 53, 53, 52, 52, 51, 51, 50,		/* 11 - 20 */
		49, 48, 47, 46, 45, 44, 43, 42, 40, 39,		/* 21 - 30 */
                39, 38, 38, 37, 37, 36, 36, 35, 34, 33,	        /* 31 - 40 */
                33, 32, 32, 31, 31, 30, 30, 29, 29, 28,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 65, 63, 61, 59, 57, 55, 53, 51, 50, 49,         /*  0 - 10 */
/* ROD */	48, 47, 47, 46, 45, 44, 43, 42, 41, 40,		/* 11 - 20 */
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30,		/* 21 - 30 */
                29, 28, 27, 26, 25, 24, 23, 22, 21, 20,	        /* 31 - 40 */
                19, 18, 17, 16, 15, 14, 13, 12, 11,  9,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 69, 68, 67, 65, 64,	        /*  0 - 10 */
/* PETRI */	63, 62, 61, 60, 59, 58, 57, 56, 55, 54,		/* 11 - 20 */
		53, 52, 51, 50, 48, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 37, 36, 35, 34, 32, 30, 28, 27,	        /* 31 - 40 */
                26, 25, 23, 22, 20, 18, 17, 16, 15, 13,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 81, 79, 78, 76, 75, 74, 73,	        /*  0 - 10 */
/* BREATH */	57, 56, 55, 54, 53, 52, 51, 50, 49, 48,		/* 11 - 20 */
		47, 46, 45, 43, 42, 41, 40, 39, 38, 36,		/* 21 - 30 */
                35, 34, 33, 32, 31, 31, 30, 30, 29, 29,	        /* 31 - 40 */
                29, 28, 27, 26, 25, 25, 24, 24, 24, 22,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,         /*  0 - 10 */
/* SPELL */	60, 59, 58, 57, 56, 55, 52, 50, 49, 47,		/* 11 - 20 */
		46, 44, 42, 41, 40, 38, 36, 35, 33, 31,		/* 21 - 30 */
                29, 28, 27, 26, 26, 25, 24, 23, 22, 21,	        /* 31 - 40 */
                20, 19, 18, 16, 14, 12, 11, 10,  9,  8,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */
  },

  { /* Monk */
           {90, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61,	        /*  0 - 10 */
/* PARA */	60, 59, 58, 57, 56, 55, 54, 54, 52, 51,		/* 11 - 20 */
		50, 49, 48, 46, 45, 44, 43, 41, 40, 39,		/* 21 - 30 */
                38, 37, 35, 34, 32, 31, 30, 28, 27, 26,	        /* 31 - 40 */
                25, 23, 21, 19, 17, 15, 13, 11,  9,  7,	        /* 41 - 50 */
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,         /*  0 - 10 */
/* ROD */	65, 64, 50, 49, 47, 46, 45, 44, 43, 42,		/* 11 - 20 */
		55, 54, 53, 52, 51, 50, 49, 48, 47, 46,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 39, 38, 37, 36,	        /* 31 - 40 */
                35, 34, 34, 33, 32, 31, 30, 29, 28, 27,	        /* 41 - 50 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66,         /*  0 - 10 */
/* PETRI */	64, 63, 62, 61, 60, 59, 58, 57, 56, 55,		/* 11 - 20 */
		54, 53, 52, 51, 49, 47, 45, 44, 43, 42,		/* 21 - 30 */
                40, 39, 37, 36, 35, 34, 33, 32, 31, 30,	        /* 31 - 40 */
                29, 28, 27, 27, 26, 25, 24, 24, 23, 22,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81,         /*  0 - 10 */
/* BREATH */	80, 79, 78, 77, 76, 75, 74, 73, 72, 70,		/* 11 - 20 */
		69, 68, 67, 65, 64, 64, 63, 63, 62, 60,		/* 21 - 30 */
                59, 58, 58, 57, 56, 55, 54, 53, 52, 51,	        /* 31 - 40 */
                50, 49, 47, 46, 44, 42, 41, 39, 38, 37,	        /* 41 - 50 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0},			/* 51 - 60 */

           {90, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76,         /*  0 - 10 */
/* SPELL */	75, 74, 73, 72, 71, 70, 69, 68, 68, 67,		/* 11 - 20 */
		66, 65, 63, 62, 60, 59, 58, 57, 56, 55,		/* 21 - 30 */
                54, 53, 52, 51, 50, 49, 48, 47, 45, 44,	        /* 31 - 40 */
                43, 42, 41, 40, 39, 38, 36, 35, 34, 32,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */
  },

  { /* Paladin */
           {90, 80, 79, 78, 77, 76, 75, 73, 72, 71, 70,         /*  0 - 10 */
/* PARA */	68, 67, 66, 65, 64, 63, 62, 61, 60, 59,		/* 11 - 20 */
		58, 56, 55, 54, 53, 51, 50, 49, 48, 46,		/* 21 - 30 */
                45, 44, 42, 41, 39, 38, 36, 35, 34, 32,	        /* 31 - 40 */
                30, 28, 27, 26, 25, 24, 23, 21, 19, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 85, 84, 83, 81, 79, 78, 76, 74, 73, 71,         /*  0 - 10 */
/* ROD */	69, 67, 66, 64, 63, 62, 60, 58, 58, 57,		/* 11 - 20 */
		56, 54, 52, 51, 49, 47, 48, 47, 45, 44,		/* 21 - 30 */
                43, 42, 41, 40, 39, 37, 36, 35, 33, 31,	        /* 31 - 40 */
                30, 30, 29, 28, 27, 26, 25, 24, 23, 22,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 80, 79, 77, 75, 74, 73, 72, 71, 70, 68,         /*  0 - 10 */
/* PETRI */	67, 66, 64, 63, 62, 61, 60, 59, 57, 56,		/* 11 - 20 */
		55, 53, 52, 50, 49, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 38, 37, 36, 35, 34, 33, 32, 32,	        /* 31 - 40 */
                33, 32, 31, 30, 28, 27, 26, 24, 20, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 80, 79, 77, 75, 74, 73, 72, 71, 70, 68,         /*  0 - 10 */
/* BREATH */	67, 66, 64, 63, 62, 61, 60, 59, 57, 56,		/* 11 - 20 */
		55, 53, 52, 50, 49, 47, 45, 44, 42, 41,		/* 21 - 30 */
                40, 39, 38, 37, 36, 35, 34, 33, 32, 32,	        /* 31 - 40 */
                33, 32, 31, 30, 28, 27, 26, 24, 20, 17,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */

           {90, 85, 84, 83, 81, 80, 79, 78, 76, 75, 74,         /*  0 - 10 */
/* SPELL */	73, 73, 72, 71, 70, 68, 67, 66, 63, 62,		/* 11 - 20 */
		60, 59, 57, 56, 55, 54, 53, 51, 48, 47,		/* 21 - 30 */
                45, 44, 43, 42, 41, 40, 38, 37, 36, 34,	        /* 31 - 40 */
                33, 32, 31, 30, 29, 28, 27, 26, 25, 23,	        /* 41 - 50 */
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},                 /* 51 - 60 */
  }

};


int mag_savingthrow(struct char_data * ch, int type)
{
  int save;

  /* negative apply_saving_throw values make saving throws better! */

  if (IS_NPC(ch)) /* NPCs use warrior tables according to some book */
    save = saving_throws[CLASS_GLADIATOR][type][(int)MIN(GET_LEVEL(ch), 50)];
  else
    save = saving_throws[(int)GET_CLASS(ch)][type][(int)MIN(GET_LEVEL(ch), 50)];

  save += GET_SAVE(ch, type);

  /* throwing a 0 is always a failure */
  if (MAX(1, save) < number(0, 99))
    return TRUE;
  else
    return FALSE;
}


/* affect_update: called from comm.c (causes spells to wear off) */
void affect_update(void)
{
  static struct affected_type *af, *next;
  static struct char_data *i;
  extern char *spell_wear_off_msg[];
  extern char *chant_wear_off_msg[];
  extern char *prayer_wear_off_msg[];
  extern char *song_wear_off_msg[];

  for (i = character_list; i; i = i->next)
    for (af = i->affected; af; af = next) {
      next = af->next;
      if (af->duration >= 1)
	af->duration--;
      else if (af->duration == -1)	/* No action */
	af->duration = -1;	/* GODs only! unlimited */
      else {
	if ((af->type > 0) && (af->type <= MAX_ABILITIES))
	  if (!af->next || (af->next->type != af->type) ||
	      (af->next->duration > 0))
	    if (*spell_wear_off_msg[af->type]) {
              switch(af->src) {
              case ABT_SPELL:
	      send_to_char(spell_wear_off_msg[af->type], i);
                break;
              case ABT_CHANT:
                send_to_char(chant_wear_off_msg[af->type], i);
                break;
              case ABT_PRAYER:
                send_to_char(prayer_wear_off_msg[af->type], i);
                break;
              case ABT_SONG:
                send_to_char(song_wear_off_msg[af->type], i);
                break;
              }
	      send_to_char("\r\n", i);
	    }
	affect_remove(i, af);
      }
    }
}


/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */
int mag_materials(struct char_data * ch, int item0, int item1, int item2,
		      int extract, int verbose)
{
  struct obj_data *tobj;
  struct obj_data *obj0 = NULL, *obj1 = NULL, *obj2 = NULL;

  for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
    if ((item0 > 0) && (GET_OBJ_VNUM(tobj) == item0)) {
      obj0 = tobj;
      item0 = -1;
    } else if ((item1 > 0) && (GET_OBJ_VNUM(tobj) == item1)) {
      obj1 = tobj;
      item1 = -1;
    } else if ((item2 > 0) && (GET_OBJ_VNUM(tobj) == item2)) {
      obj2 = tobj;
      item2 = -1;
    }
  }
  if ((item0 > 0) || (item1 > 0) || (item2 > 0)) {
    if (verbose) {
      switch (number(0, 2)) {
      case 0:
	send_to_char("A wart sprouts on your nose.\r\n", ch);
	break;
      case 1:
	send_to_char("Your hair falls out in clumps.\r\n", ch);
	break;
      case 2:
	send_to_char("A huge corn develops on your big toe.\r\n", ch);
	break;
      }
    }
    return (FALSE);
  }
  if (extract) {
    if (item0 < 0) {
      obj_from_char(obj0);
      extract_obj(obj0);
    }
    if (item1 < 0) {
      obj_from_char(obj1);
      extract_obj(obj1);
    }
    if (item2 < 0) {
      obj_from_char(obj2);
      extract_obj(obj2);
    }
  }
  if (verbose) {
    send_to_char("A puff of smoke rises from your pack.\r\n", ch);
    act("A puff of smoke rises from $n's pack.", TRUE, ch, NULL, NULL, TO_ROOM);
  }
  return (TRUE);
}




/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 */
extern int warn_newbie(struct char_data *, struct char_data *);
void mag_damage(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, byte type, int savetype)
{
  int is_mage = 0, is_cleric = 0;
  int dam = 0;

  if (victim == NULL || ch == NULL) 
    return;

  is_mage = (GET_CLASS(ch) == CLASS_SORCERER);
  is_cleric = (GET_CLASS(ch) == CLASS_CLERIC);

  if (!pkill_ok(ch, victim)) {
    send_to_char("Player killing is forbidden here.\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
    send_to_char("A flash of white light dispels your violent magic!\r\n", ch);
    return;
  }

  if (ROOM_FLAGGED(victim->in_room, ROOM_NOMAGIC)) {
    send_to_char("A flash of white light dispels your violent magic!\r\n", ch);
    return;
  }

  if (!warn_newbie(ch, victim))
    return; /* Live to fight another day. */

  switch (type) {
  case ABT_SPELL:
    switch (spellnum) {
    /* Mostly mages */
    case SPELL_MAGIC_MISSILE:
      dam = number (1, 2);
    case SPELL_CHILL_TOUCH:	/* chill touch also has an affect */
      if (is_mage)
        dam = dice(1, 4) + 2;
      else
        dam = dice(1, 3) + 2;
      break;
    case SPELL_BURNING_HANDS:
      if (is_mage)
        dam = number(4, 6) + 2;
      else
        dam = dice(2, 3) + 2;
      break;
    case SPELL_SHOCKING_GRASP:
      if (is_mage)
        dam = number(10, 21) + 5;
      else
        dam = dice(2, 4) + 5;
      break;
    case SPELL_LIGHTNING_BOLT:
      if (is_mage)
        dam = number(12, 23) + 5;
      else
        dam = number(3, 4) + 5;
      break;
    case SPELL_FIRE_BREATH:
      dam = number(20, 30) + 4;
      break;
    case SPELL_GAS_BREATH:
      dam = number(25, 35) + 4;
      break;
    case SPELL_FROST_BREATH:
      dam = number(30, 40) + 4;
      break;
    case SPELL_ACID_BREATH:
      dam = number(35, 45) + 4;
      break;
    case SPELL_LIGHTNING_BREATH:
      dam = number(40, 50) + 4;
      break;
    case SPELL_COLOR_SPRAY:
      if (is_mage)
        dam = number(20, 30) + 4;
      else
        dam = dice(3, 6) + 4;
      break;
    case SPELL_FIREBALL:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your fireball has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's fireball gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(8,6) +7;
      } else
      if (is_mage)
        dam = number(45, 57) + 7;
      else
        dam = number(25, 33) + 7;
      break;
    case SPELL_DEMONFIRE:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your demonfire has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's demonfire gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(20, 20);
      } else
      if (is_mage)
        dam = dice(4, 12) + 10;
      else
        dam = number(43, 55);
      break;
    case SPELL_WRATHOFGOD:
      dam = dice(4, 10) + 10;
      break;
    case SPELL_BLACKDART:
      dam = number(8, 12);
      break;
    case SPELL_BLACKBREATH: /* Also has affects */
      dam = number(15, 25);
      break;

    case SPELL_DISPEL_EVIL:
      if (IS_GOOD(victim)) {
        act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
        dam = 0;
        return;
      } else
      dam = dice(6, 8) + 6;
      break;

    case SPELL_DISPEL_GOOD:
      if (IS_EVIL(victim)) {
        act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
        dam = 0;
        return;
      } else
      dam = dice(6, 8) + 6;
      break;

    case SPELL_CALL_LIGHTNING:
      dam = dice(6, 11) + 12;
      break;

    case SPELL_HARM:
      if (is_mage)
        dam = dice(10, 5) + 6;
      else
        dam = dice(4, 7) + 6;
      break;

    case SPELL_ENERGY_DRAIN:
      dam = dice(4, 8) + 6;
      break;

    /* Area spells */
    case SPELL_EARTHQUAKE:
      dam = number(35, 45);
      break;

    case SPELL_FIRESTORM: /* Area */
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your firestorm has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's firestorm gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(20, 20);
      } else
      if (GET_CLASS(ch) == CLASS_DARK_KNIGHT)
        dam = number(25, 61);
      else
        dam = number(70, 102);
      break;

    case SPELL_ICESTORM: /* Area */
      dam = dice(4, 8) + 4;
      break;

    case SPELL_RAIN_OF_FIRE: /* Demon Area Spell */
      dam = dice(12, 20) + number(175, 300);
      break;

    case SPELL_CAUSE_LIGHT:
      dam = number(1, 3);
      break;

    case SPELL_CAUSE_CRITIC:
      dam = number(10, 16);
      break;

    case SPELL_LAY_HANDS:
      dam = number(9, 12); 
      break;

    case SPELL_HOLY_MACE:
      dam = number(88, 102);
      break;

    case SPELL_SONIC_WALL: /* Also affects */
      dam = dice(10, 7) + number(50, 150);
      break;

    case SPELL_RAY_OF_LIGHT:
      dam = number(108, 122);
      break;

    case SPELL_STONE_HAIL:
      dam = number(160, 195);
      break;

    case SPELL_FLYING_FIST:
      dam = number(170, 227);
      break;

    case SPELL_SHOCK_SPHERE:
      dam = number(225, 291);
      break;

    case SPELL_CAUSTIC_RAIN:
      if (is_mage)
        dam = number(255, 327);
      else
        dam = number(212, 267);
      break;

    case SPELL_WITHER: /* Plus affect */
      dam = dice(15, 13) + number(65, 150);
      break;

    case SPELL_METEOR_SWARM:
      if (is_mage)
        dam = number(365, 440);
      else
        dam = number(315, 374);
      break;

    case SPELL_BALEFIRE:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your balefire has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's balefire gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(20, 20);
      } else
      dam = number(390, 480);
      break;

    case SPELL_SCORCH:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("You unable to scorch $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's attempt at scorching you gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(25, 20);
      } else
      dam = number(485, 572);
      break;

    case SPELL_IMMOLATE:
      dam = number(535, 616);
      break;

    case SPELL_HELLFIRE:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your hellfire has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's hellfire gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(9, 10);
      } else
      dam = number(95, 120);
      break;

    case SPELL_FROST_BLADE:
      dam = number(110, 140);
      break;

    case SPELL_FLAME_BLADE:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your flameblade has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's flameblade gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(10, 15);
      } else
      dam = number(135, 180);
      break;
 
    case SPELL_ACID_STREAM:
      dam = number(175, 202);
      break;
 
    case SPELL_FLAME_STRIKE:
      if (GET_CLASS(victim) == CLASS_DEMON &&
          GET_SKILL(victim, SKILL_ABSORB_FIRE)) {
       act("Your flame strike has no affect on $N.",FALSE,ch, 0, victim, TO_CHAR);
       act("$n's flame strike gives you added strength!",FALSE,ch,0,victim,TO_VICT);
       GET_HIT(victim) += dice(15, 15);
      } else
      dam = number(200, 225);
      break;
 
    case SPELL_FINGER_OF_DEATH:
      if (IS_NPC(victim) && victim->player.race == RACE_NPC_UNDEAD) {
        send_to_char(NOEFFECT, ch);
        return;
      }
      dam = number(230, 275);
      break;
 
    case SPELL_SCOURGE:
      dam = number(235, 300);
      break;
 
    case SPELL_SOUL_RIP:
      dam = number(295, 357);
      break;
 
    case SPELL_FELLBLADE:
      dam = number(312, 385);
      break;
 
    case SPELL_EXTERMINATE:
      dam = number(370, 412);
      break;

    case SPELL_BLADE_OF_LIGHT:
      dam = number(185, 225);
      break;
 
    case SPELL_FLAMES_PURE:
      dam = number(235, 275);
      break;
 
    case SPELL_PRISMATIC_BEAM: /* Also Blinds */
      dam = number(220, 250);
      break;
 
    case SPELL_HOLOCAUST:
      dam = number(315, 385);
      break;
 
    case SPELL_CRY_FOR_JUSTICE:
      dam = number(340, 412);
      break;
 
    }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    case CHANT_YIN_XU:
      dam = number(40, 53);
      break;

    case CHANT_YANG_XU:
      dam = number(178, 269);
      break;

    case CHANT_PSIONIC_DRAIN:
      dam = number(65, 82);
      break;

    case CHANT_PSYCHIC_FURY:
      dam = number(300, 488);
      break;
 
    }

    break;
  case ABT_PRAYER:
    switch (spellnum) {
    case PRAY_GODS_FURY:
      dam = number(220, 240);
      break;

    case PRAY_DISPEL_EVIL:
      dam = dice(6, 8) + 6;
      if (IS_EVIL(ch)) {
        victim = ch;
        dam = GET_HIT(ch) - 1;
      } else if (IS_GOOD(victim)) {
        act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
        dam = 0;
        return;
      }
      break;

    case PRAY_DISPEL_GOOD:
      dam = dice(6, 8) + 6;
      if (IS_GOOD(ch)) {
        victim = ch;
        dam = GET_HIT(ch) - 1;
      } else if (IS_EVIL(victim)) {
        act("The gods protect $N.", FALSE, ch, 0, victim, TO_CHAR);
        dam = 0;
        return;
      }
      break;

    case PRAY_HARM:
      dam = number(18, 21);
      break;

    case PRAY_EARTHQUAKE:
      dam = number(10, 15);
      break;

    case PRAY_CALL_LIGHTNING:
      dam = number(35, 42);
      break;

    case PRAY_HAND_BALANCE:
      dam = number(125, 138);
      break;

    case PRAY_ELEMENTAL_BURST:
      dam = number(50, 56);
      break;

    case PRAY_WINDS_PAIN:
      dam = number(90, 96);
      break;

    case PRAY_SPIRIT_STRIKE:
      dam = number(150, 162);
      break;
 
    case PRAY_ABOMINATION:
      dam = number(160, 180);
      break;

    case PRAY_WINDS_RECKONING:
      dam = number(200, 220);
      break;

    case PRAY_ANGEL_BREATH:
      dam = number(260, 286);
      break;

    case PRAY_SOUL_SCOURGE:
      dam = number(310, 330);
      break;

    case PRAY_PLAGUE:
      dam = number(290, 308);
      break;

    case PRAY_EXORCISM:
      if (GET_ALIGNMENT(ch) <= 0) {
        send_to_char("Your evil prevents you from doing any significant damage.\r\n", ch);
        dam = 0;
        return;
      } else if (GET_ALIGNMENT(victim) >= GET_ALIGNMENT(ch)) {
        send_to_char("Your victim is too good-aligned, you do no damage.\r\n", ch);
        dam = 0;
        return;
      } else
        dam = GET_SPELLPOWER(ch) + 
            ((GET_ALIGNMENT(ch) - GET_ALIGNMENT(victim)) / 2.5);
      break;

    case PRAY_HEAVENSBEAM:
      if (GET_ALIGNMENT(ch) <= 0) {
        send_to_char("Your evil ways prevent you from calling down the lights of heaven.\r\n", ch);
        dam = 0;
        GET_MANA(ch) -= 175;
        return;
      } else
        dam = GET_SPELLPOWER(ch) + number(50, 150) + (GET_TIER(ch) * 100);
      break;

    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;

  }

  /*Apply SPELLPOWER*/
  dam += GET_SPELLPOWER(ch) * 2;
    
  /* If it is not an NPC cut dam by quarter (just like regular dmg) */
  if (!IS_NPC(victim))
    dam >>= 2;

  /* divide damage by two if victim makes his saving throw */
  if (mag_savingthrow(victim, savetype))
    dam >>= 1;

  /* PALADIN align penalties */
  if (GET_CLASS(ch) == CLASS_PALADIN) {
    if (GET_ALIGNMENT(ch) >= 201 && GET_ALIGNMENT(ch) <= 600)
     dam = dam * .75;
    if (GET_ALIGNMENT(ch) >= -200 && GET_ALIGNMENT(ch) <= 200)
     dam = dam * .5;
    if (GET_ALIGNMENT(ch) >= -600 && GET_ALIGNMENT(ch) <= -201)
     dam = dam * .25;
    if (GET_ALIGNMENT(ch) <= -601) {
     send_to_char("Your spell has no potency with such bad alignment.\r\n", ch);
     dam = 0;
    }
  }

  /* DARK KNIGHT align penalties */
  if (GET_CLASS(ch) == CLASS_DARK_KNIGHT) {
    if (GET_ALIGNMENT(ch) <= -201 && GET_ALIGNMENT(ch) >= -600)
     dam = dam * .75;
    if (GET_ALIGNMENT(ch) <= 200 && GET_ALIGNMENT(ch) >= -200)
     dam = dam * .5;
    if (GET_ALIGNMENT(ch) <= 600 && GET_ALIGNMENT(ch) >= 201)
     dam = dam * .25;
    if (GET_ALIGNMENT(ch) >= 601) {
     send_to_char("Your spell has no potency with such good alignment.\r\n",ch);
     dam = 0;
    }
  }

  /* and finally, inflict the damage */
  if (number(1, 10) <= 6 && IS_AFFECTED(victim, AFF_DEFLECT)) {
    act("Your magic is deflected by $n's shield and slams back into you!", 
         FALSE, victim, 0, 0, TO_VICT);
    act("$N's magic is deflected by your shield and slams back into $M!",
         TRUE, victim, 0, ch, TO_CHAR);
    act("$N's magic is deflected by $n's shield and slams back into $M!", 
         TRUE, victim, 0, ch, TO_ROOM); 
    damage(ch, ch, dam * 5, spellnum, type);
  } else  
  damage(ch, victim, dam * 5, spellnum, type);
}


/*
 * Every spell that does an affect comes through here.  This determines
 * the effect, whether it is added or replacement, whether it is legal or
 * not, etc.
 *
 * affect_join(vict, aff, add_dur, avg_dur, add_mod, avg_mod)
*/

#define MAX_SPELL_AFFECTS 5	/* change if more needed */

void mag_affects(int level, struct char_data * ch, struct char_data * victim,
		      int spellnum, byte type, int savetype)
{
  struct affected_type af[MAX_SPELL_AFFECTS];
  int is_mage = FALSE, is_cleric = FALSE;
  bool accum_affect = FALSE, accum_duration = FALSE;
  char *to_vict = NULL, *to_room = NULL;
  int i;

  if (victim == NULL || ch == NULL)
    return;

  is_mage = (GET_CLASS(ch) == CLASS_SORCERER);
  is_cleric = (GET_CLASS(ch) == CLASS_CLERIC);

  for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
    af[i].type = spellnum;
    af[i].bitvector = 0;
    af[i].modifier = 0;
    af[i].location = APPLY_NONE;
  }

  switch (type) {
  case ABT_SPELL:
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      af[i].src = ABT_SPELL;
    /* Handle spells here */
  switch (spellnum) {
  case SPELL_CHILL_TOUCH:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_STR;
    if (mag_savingthrow(victim, savetype))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    af[0].modifier = -1;
    accum_duration = FALSE;
    to_vict = "You feel your strength wither!";
    break;

  case SPELL_AGITATION:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -5;
    af[0].duration = 4 + (number(10, 100) / 10);

    af[1].location = APPLY_DAMROLL;
    af[1].modifier = -5;
    af[1].duration = 4 + (number(10, 100) / 10);

    af[2].location = APPLY_AC;
    af[2].modifier = 10;
    af[2].duration = 4 + (number(10, 100) / 10);

    accum_duration = FALSE;
    to_vict = "A horrible itching hits you suddenly";
    to_room = "$n begins to scratch himself vigorously";
    break;

  case SPELL_ARMOR:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].modifier = -10;
    af[0].duration = 24;
    af[0].bitvector = AFF_ARMOR;
    accum_duration = FALSE;
    to_vict = "You feel someone protecting you.";
    break;

  case SPELL_BLUR:
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].modifier = -15;
    af[0].duration = number(10, 25)/5;
    accum_duration = FALSE;
    to_vict = "Your location becomes harder to pinpoint.";
    break;

  case SPELL_BLESS:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Bless a vampire... interesting concept, anyway.\r\n", ch);
      return;
    }
    af[0].location = APPLY_HITROLL;
    af[0].modifier = 3;
    af[0].duration = 6;

    af[1].location = APPLY_SAVING_SPELL;
    af[1].modifier = -1;
    af[1].duration = 6;

    accum_duration = FALSE;
    to_vict = "You feel righteous.";
    break;

  case SPELL_BLINDNESS:
    if (!pkill_ok(ch, victim) || MOB_FLAGGED(victim, MOB_NOBLIND) ||
        AFF_FLAGGED(victim, AFF_NOBLIND) || mag_savingthrow(victim, savetype)) {
      send_to_char("You fail.\r\n", ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].modifier = -4;
    af[0].duration = 2;
    af[0].bitvector = AFF_BLIND;

    af[1].location = APPLY_AC;
    af[1].modifier = 40;
    af[1].duration = 2;
    accum_duration = FALSE;
    af[1].bitvector = AFF_BLIND;

    to_room = "$n seems to be blinded!";
    to_vict = "You have been blinded!";
    break;

  case SPELL_CONFUSE:
    if (IS_AFFECTED(victim, AFF_CONFUSE)) {
      sprintf(buf, "%s is wise to your magic, it has no affect.\r\n",
               GET_NAME(victim));
      send_to_char(buf, ch);
      return;
    }
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -20;
    af[0].duration = 4;
    accum_duration = FALSE;
    af[0].bitvector = AFF_CONFUSE;

    to_room = "$n gets a confused look in his eye.";
    to_vict = "The world just doesn't seem right anymore.";

    WAIT_STATE(victim, PULSE_VIOLENCE * 4);

    break;

  case SPELL_CURSE:
    if (!pk_allowed || !pkill_ok(ch, victim) ||
        mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }

    af[0].location = APPLY_HITROLL;
    af[0].duration = 1 + (number(14, 40) >> 1);
    af[0].modifier = -1;
    af[0].bitvector = AFF_CURSE;

    af[1].location = APPLY_DAMROLL;
    af[1].duration = 1 + (number(14, 40) >> 1);
    af[1].modifier = -1;
    af[1].bitvector = AFF_CURSE;

    accum_duration = FALSE;
    accum_affect = FALSE;
    to_room = "$n briefly glows red!";
    to_vict = "You feel very uncomfortable.";
    break;

  case SPELL_DETECT_ALIGN:
    af[0].duration = 12 + number(1, 10);
    af[0].bitvector = AFF_DETECT_ALIGN;
    accum_duration = FALSE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_INVIS:
    af[0].duration = 12 + number(1, 10);
    af[0].bitvector = AFF_DETECT_INVIS;
    accum_duration = FALSE;
    to_vict = "Your eyes tingle.";
    break;

  case SPELL_DETECT_MAGIC:
    af[0].duration = 12 + number(1, 10);
    af[0].bitvector = AFF_DETECT_MAGIC;
    accum_duration = FALSE;
    to_vict = "Your eyes tingle.";
    to_room = "$n's eyes glow briefly.";
    break;

  case SPELL_DIVINE_NIMBUS:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires have no need for such weak magics.\r\n", ch);
      return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }

    af[0].duration = 4;
    af[0].bitvector = AFF_DIVINE_NIMBUS;
    accum_duration = FALSE;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_FLY:
    af[0].duration = number(5, 40);
    af[0].bitvector = AFF_FLY;
    accum_duration = FALSE;
    to_vict = "You fly off the ground!";
    to_room = "$n flies off the ground.";
    break;

  case SPELL_INFRAVISION:
    af[0].duration = 12 + number(1, 10);
    af[0].bitvector = AFF_INFRAVISION;
    accum_duration = FALSE;
    to_vict = "Your eyes glow red.";
    to_room = "$n's eyes glow red.";
    break;

  case SPELL_INVISIBLE:
    if (!victim)
      victim = ch;

    af[0].duration = 12 + (number(5, 20) >> 2);
    af[0].modifier = -10;
    af[0].location = APPLY_AC;
    af[0].bitvector = AFF_INVISIBLE;
    accum_duration = FALSE;
    to_vict = "You vanish.";
    to_room = "$n slowly fades out of existence.";
    break;

  case SPELL_HASTE:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use this on a mob.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_TITAN) {
      send_to_char("Try as you might, you cannot speed a titan up.\r\n", ch);
      return;
    }
    af[0].bitvector = AFF_HASTE;
    af[0].duration = 3 + (number(25, 50) / 2);
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -2;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel faster";
    to_room = "$n visibly speeds up.";
    break;

  case SPELL_MIST_FORM:
    af[0].bitvector = AFF_MIST_FORM;
    af[0].duration = 1;
    af[0].location = APPLY_AC;
    af[0].modifier = -10;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You take on a vaporish form.";
    to_room = "$n takes on a vaporish form.";
    break;

  case SPELL_POISON:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    af[0].location = APPLY_STR;
    af[0].duration = 2;
    af[0].modifier = -2;
    af[0].bitvector = AFF_POISON;
    accum_duration = FALSE;
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_PROT_FROM_EVIL:
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTECT_EVIL;
    accum_duration = FALSE;
    to_vict = "You feel invulnerable!";
    break;

  case SPELL_SANCTUARY:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("This protective magic does not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires have no need for such weak magics.\r\n", ch);
      return;
    }
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    if ((GET_CLASS(ch) == CLASS_CLERIC) ||
        (GET_CLASS(ch) == CLASS_ARCANIC) ||
        (GET_CLASS(ch) == CLASS_TEMPLAR) ||
        (GET_CLASS(ch) == CLASS_STORR) ||
        (GET_CLASS(ch) == CLASS_SAGE))
      af[0].duration = 1 + (GET_LEVEL(ch) * 4 / 50);
    else
      af[0].duration = 4;

    af[0].bitvector = AFF_SANCTUARY;

    accum_duration = FALSE;
    to_vict = "A white aura momentarily surrounds you.";
    to_room = "$n is surrounded by a white aura.";
    break;

  case SPELL_SLEEP:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    if (MOB_FLAGGED(victim, MOB_NOSLEEP))
      return;
    if (mag_savingthrow(victim, savetype))
      return;

    af[0].duration = 4 + (number(10, 20) >> 2);
    af[0].bitvector = AFF_SLEEP;
    accum_duration = FALSE;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_SNARE:
    if (!pk_allowed && !pkill_ok(ch, victim)) {
      send_to_char("You cannot cast this spell on them at this time.\r\n", ch);
      return;
    }
    if (mag_savingthrow(victim, savetype)) {
      send_to_char("Your spell does not take hold.\r\n", ch);
      return;
    }

    if (GET_LEVEL(ch) <= 120)
      af[0].duration = 1;
    if (GET_LEVEL(ch) <= 135 && GET_LEVEL(ch) >= 121)
      af[0].duration = 2;
    if (GET_LEVEL(ch) >= 136)
      af[0].duration = 3;
    af[0].bitvector = AFF_SNARE;
    accum_duration = FALSE;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("Your feet adhere to the ground, you cannot flee!", 
           FALSE, victim, 0, 0, TO_CHAR);
      act("$n's feet have adhered to the ground!", TRUE, victim, 0, 0, TO_ROOM);
    }
    break;

  case SPELL_POTION_SLEEP:
    if (mag_savingthrow(victim, savetype))
      return;
    af[0].duration = 4 + (number(10, 20) >> 2);
    af[0].bitvector = AFF_SLEEP;
    accum_duration = FALSE;

    if (GET_POS(victim) > POS_SLEEPING) {
      act("You feel very sleepy...  Zzzz......", FALSE, victim, 0, 0, TO_CHAR);
      act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
      GET_POS(victim) = POS_SLEEPING;
    }
    break;

  case SPELL_SOFTEN_FLESH:		/* Soften Flesh - Crysist */
    af[0].duration = 4;
    accum_duration = FALSE;
    af[0].bitvector = AFF_SOFTEN_FLESH;
    
    to_room = "$n's skin turns filmy and pale.";
    to_vict = "Your skin turns flimsy and pale.";
    break;

  case SPELL_MINOR_STRENGTH:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_STR)) {
      send_to_char("You already have a strength buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_STR;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_STR;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel stronger!";
    break; 

  case SPELL_STRENGTH:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (IS_AFFECTED(ch, AFF_STR)) {
      send_to_char("You already have a strength buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_STR;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_STR;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel stronger!";
    break;

  case SPELL_MAJOR_STRENGTH:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_STR)) {
      send_to_char("You already have a strength buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_STR;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 5;
    af[0].bitvector = AFF_STR;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel stronger!";
    break;

  case SPELL_MINOR_DEXTERITY:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_DEX)) {
      send_to_char("You already have a dexterity buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_DEX;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_DEX;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more dextrous!";
    break;

  case SPELL_DEXTERITY:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_DEX)) {
      send_to_char("You already have a dexterity buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_DEX;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_DEX;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more dextrous!";
    break;

  case SPELL_MINOR_INTELLIGENCE:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_INT)) {
      send_to_char("You already have an intelligence buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_INT;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_INT;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more intelligent!";
    break;

  case SPELL_INTELLIGENCE:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_INT)) {
      send_to_char("You already have an intelligence buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_INT;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_INT;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more intelligent!";
    break;

  case SPELL_MINOR_WISDOM:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_WIS)) {
      send_to_char("You already have a wisdom buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_WIS;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_WIS;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel wise!";
    break;

  case SPELL_WISDOM:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_WIS)) {
      send_to_char("You already have a wisdom buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_WIS;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_WIS;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel wise!";
    break;

  case SPELL_MINOR_CONSTITUTION:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_CON)) {
      send_to_char("You already have a constitution buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_CON;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_CON;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "Your constitution increases!";
    break;

  case SPELL_CONSTITUTION:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_CON)) {
      send_to_char("You already have a constitution buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_CON;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_CON;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "Your constitution increases!";
    break;

  case SPELL_MINOR_CHARISMA:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_CHA)) {
      send_to_char("You already have a charisma buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_CHA;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_CHA;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more charismatic!";
    break;

  case SPELL_CHARISMA:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_CHA)) {
      send_to_char("You already have a charisma buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_CHA;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_CHA;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more charismatic!";
    break;

  case SPELL_MINOR_LUCK:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_LUCK)) {
      send_to_char("You already have a luck buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_LUCK;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 2;
    af[0].bitvector = AFF_LUCK;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more lucky!";
    break;

  case SPELL_LUCK:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (IS_AFFECTED(ch, AFF_LUCK)) {
      send_to_char("You already have a luck buff.\r\n", ch);
      return;
    }
    af[0].location = APPLY_LUCK;
    af[0].duration = (number(30, 40) >> 1) + 4;
    af[0].modifier = 3;
    af[0].bitvector = AFF_LUCK;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel more lucky!";
    break;

  case SPELL_SENSE_LIFE:
    to_vict = "Your feel your awareness improve.";
    af[0].duration = number(4, 10);
    af[0].bitvector = AFF_SENSE_LIFE;
    accum_duration = FALSE;
    break;

  case SPELL_WATERWALK:
    af[0].duration = 24;
    af[0].bitvector = AFF_WATERWALK;
    accum_duration = FALSE;
    to_vict = "You feel webbing between your toes.";
    break;

  case SPELL_DARKWARD:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires have no need for such weak magics.\r\n", ch);
      return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].duration = 6;
    af[0].bitvector = AFF_DARKWARD;

    accum_duration = FALSE;
    to_vict = "A dark ward surrounds you.";
    to_room = "$n is surrounded by a dark ward.";
    break;

  case SPELL_PROTFROMGOOD:
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].duration = 24;
    af[0].bitvector = AFF_PROTFROMGOOD;
    accum_duration = FALSE;
    to_vict = "You feel invulnerable!";
    to_room = "$n looks invulnerable to good things.";
    break;

  case SPELL_BLACKDART:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_STR;
    af[0].duration = 2;
    af[0].modifier = -2;
    af[0].bitvector = AFF_POISON;
    accum_duration = FALSE;
    to_vict = "You feel very sick.";
    to_room = "$n gets violently ill!";
    break;

  case SPELL_BLACKBREATH:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_STR;
    af[0].duration = number(5, 10);
    af[0].modifier = -1;
    af[1].location = APPLY_DEX;
    af[1].duration = number(5, 10);
    af[1].modifier = -1;
    af[2].location = APPLY_HITROLL;
    af[2].duration = number(5, 10);
    af[2].modifier = -5;    
    af[0].bitvector = AFF_BLACKBREATH;
    accum_duration = FALSE;
    to_vict = "You feel very sluggish.";
    to_room = "$n gets very sluggish!";
    break;

  case SPELL_CHAOSARMOR:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires have no need for such weak magics.\r\n", ch);
      return;
    }
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].duration = 8;
    af[0].modifier = -20;
    af[1].location = APPLY_DAMROLL;
    af[1].duration = 8;
    af[1].modifier = +4;
    af[0].bitvector = AFF_CHAOSARMOR;
    accum_duration = FALSE;
    to_vict = "You feel ready to fight a war!";
    to_room = "$n is surrounded by $s gods armor.";
    break;

  case SPELL_AEGIS:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].duration = 10;
    af[0].modifier = -15;
    af[0].bitvector = AFF_AEGIS;
    accum_duration = FALSE;
    to_vict = "A protective shield surrounds you.";
    to_room = "$n is surrounded by a protective shield.";
    break;

  case SPELL_EYESOFTHEDEAD:
    af[0].location = APPLY_HITROLL;
    af[0].duration = 4;
    af[0].modifier = 1;
    af[1].duration = number(5, 10);
    af[1].bitvector = AFF_DETECT_INVIS;
    af[1].duration = number(5, 10);
    af[2].bitvector = AFF_INFRAVISION;
    af[2].duration = number(5, 10);
    accum_duration = FALSE;
    to_vict = "Your vision takes on a new dimension of sight.";
    to_room = "$n's eyes roll back into $s head.";
    break;

  case SPELL_PARALYZE:
    af[0].duration = 2;
    af[0].bitvector = AFF_SLOW;
    accum_duration = FALSE;

  case SPELL_FIRESHIELD:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].duration = 6;
    af[0].modifier = -15;
    af[1].location = APPLY_DAMROLL;
    af[1].duration = 6;
    af[1].modifier = +5;
    af[0].bitvector = AFF_FIRESHIELD;
    accum_duration = FALSE;
    to_vict = "You are surrounded by a shield of fire.";
    to_room = "A shield of fire surrounds $n.";
    break;
  case SPELL_REGENERATE:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    af[0].duration = (number(10, 30) / 2);
    af[0].bitvector = AFF_REGENERATE;
    to_vict = "Your body starts regenerating.";
    to_room = "$n's body starts regenerating faster.";
    accum_duration = FALSE;
    break;

  case SPELL_BREATHE:
    af[0].duration = (number(10, 30) / 2);
    af[0].bitvector = AFF_BREATHE;
    to_vict = "You develop gills.";
    to_room = "$n develops gills.";
    accum_duration = FALSE;
    break;
    
   case SPELL_SLOW:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
     af[0].duration = number(10, 30)/10+5;
     af[0].bitvector = AFF_SLOW;
     accum_duration = FALSE;
     to_room = "$n slows down.";
     to_vict = "You are now moving in slow motion.";
     break;      

  case SPELL_DUMBNESS:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_INT;
    if (mag_savingthrow(victim, savetype))
      af[0].duration = 1;
    else
      af[0].duration = 4;
    af[0].modifier = -3;
    accum_duration = FALSE;
    to_room = "$n slowly tunrs $s his head toward you and says, 'duh.... wut happund?'";
    to_vict = "You feel dumber somehow.";
    break;

  case SPELL_PHANTOM_ARMOR:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].duration = 2;
    af[0].modifier = -10;
    af[0].bitvector = AFF_PHANTOM_ARMOR;
    accum_duration = FALSE;
    to_vict = "The spirits on the undead come forth to protect you!";
    to_room = "$n is surrounded by a hoard of phantoms.";
    break;

  case SPELL_SPECTRAL_WINGS:
    af[0].duration = 24;
    af[0].bitvector = AFF_FLY;
    accum_duration = FALSE;
    to_vict = "You sprout a pair of spectral wings and rise from the ground!";
    to_room = "$n sprouts a pair of spectral wings and rises from the ground.";
    break;

  case SPELL_HOLY_FURY:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_TITAN) {
      send_to_char("Titans cannot use this magic.\r\n", ch);
      return;
    }
    af[0].bitvector = AFF_HASTE;
    af[0].duration = 3 + (number(20, 40) / 2);
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel like you can take on an army!";
    to_room = "$n gets a strange furious scowl on $m face and speeds up.";
    break;

  case SPELL_CHAMPION_STRENGTH:
    af[0].location = APPLY_STR;
    af[0].duration = 5;
    af[0].modifier = 3;
      af[1].location = APPLY_DAMROLL;
      af[1].duration = 5;
      af[1].modifier = 3;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You feel like you have the strength of a champion!";
    break;

  case SPELL_PLAGUE:
    if (mag_savingthrow(victim, savetype)) {
      send_to_char(NOEFFECT, ch);
      return;
    }
    af[0].location = APPLY_HITROLL;
    af[0].duration = 48;
    af[0].modifier = -5;
    af[0].bitvector = AFF_PLAGUE;
    accum_duration = FALSE;
    to_vict = "You sneeze.";
    to_room = "$n sneezes.";
    break;

   case SPELL_SONIC_WALL:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].duration = number(5, 50)/10+2;
    af[0].bitvector = AFF_DEAF;
    accum_duration = FALSE;
    to_room = "$n becomes deaf.";
    to_vict = "You hear only ringing.";
    break;  

/* Thanks Sean! */
/* Who's Sean? */
   case SPELL_GROWTH:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use this on a mob.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
     af[0].location = APPLY_CHAR_WEIGHT;
     af[0].duration = 8;
     af[0].modifier = number(5, 10) * 2;
     
     af[1].location = APPLY_CHAR_HEIGHT;
     af[1].duration = 8;     
     af[1].modifier = number(5, 10) * .5;

     af[2].location = APPLY_STR;
     af[2].duration = 8;
     af[2].modifier += number(5, 10) * .06;

     af[3].location = APPLY_DAMROLL;
     af[3].duration = 8;
     af[3].modifier = 4;

     af[4].location = APPLY_HITROLL;
     af[4].duration = 8;
     af[4].modifier = 4; 
     to_vict = "You become larger.";
     to_room = "$n becomes larger."; 
     accum_duration = FALSE;
     break;

    case SPELL_MANA_ECONOMY:
    /* check added by SM so Sorc can brew this, without paladin or dk using it */
    if (GET_CLASS(ch) == CLASS_PALADIN || GET_CLASS(ch) == CLASS_DARK_KNIGHT) {
    	send_to_char("Your are not experanced enough in the magical arts!!", ch);
    	return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
      af[0].duration = 4;
      af[0].bitvector = AFF_MANA_ECONOMY;
      accum_duration = FALSE;
      to_vict = "You conserve your mana.";
      break;

    case SPELL_WITHER:
      af[0].location = APPLY_STR;
      if (mag_savingthrow(victim, savetype))
        af[0].duration = 1;
      else
        af[0].duration = 4;
      af[0].modifier = -1;
      accum_duration = FALSE;
      break;

    case SPELL_PRISMATIC_BEAM:

    if (!pkill_ok(ch, victim) || MOB_FLAGGED(victim, MOB_NOBLIND) ||
        AFF_FLAGGED(victim, AFF_NOBLIND) || mag_savingthrow(victim, savetype)) {
        send_to_char("You fail.\r\n", ch);
        return;
      }

      af[0].location = APPLY_HITROLL;
      af[0].modifier = -4;
      af[0].duration = 1;
      af[0].bitvector = AFF_BLIND;
      af[1].location = APPLY_AC;
      af[1].modifier = 40;
      af[1].duration = 1;
      af[1].bitvector = AFF_BLIND;
      accum_duration = FALSE;
      to_room = "$n seems to be blinded!";
      to_vict = "You have been blinded!";
      break; 

  case SPELL_CONE_COLD:
      if (!pk_allowed && !pkill_ok(ch, victim))
        return;
      if (IS_NPC(victim))
        return;
      af[0].location = APPLY_AC;
      af[0].modifier = -50;
      af[0].duration = 1;
      af[0].bitvector = AFF_COLD;
      accum_duration = FALSE;
      to_room = "\r\n/cW$n is blasted with ice, frozen solid!/c0";
      to_vict = "\r\n/cWThe last thing you see is a blast of white "
                "ice slamming full force into your body!/c0";

    break;

  case SPELL_INSOMNIA:
    if (!pk_allowed && !pkill_ok(ch, victim)) {
      send_to_char("Both parties must approve pkill for you to successfully cast this spell.\r\n", ch);
      return;
    }
    if (IS_NPC(victim)) {
      send_to_char("Not on mobs, sorry.\r\n", ch);
      return;
    }
    af[0].bitvector = AFF_INSOMNIA;
    af[0].duration = number(10, 50);
    accum_duration = FALSE;
    to_room = "$n blinks blearily, wishing $e could sleep.";
    to_vict = "You become very tired.";
    break;

  case SPELL_SEAL:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].bitvector = AFF_SEALED;
    af[0].duration = number(50, 150) / 25;
    accum_duration = FALSE;
    to_room = "$N makes a grand gesture, sprinkling $n with a magical dust.";
    to_vict = "$N makes a grand gesture, sprinkling you with a magical dust.";
    break;

  case SPELL_COLOR_SPRAY:
    if (!number(0, 4)) { 
       GET_POS(victim) = POS_SITTING;
       to_room = "$n is knocked off of $s feet by a spray of color!";
       to_vict = "You are knocked off of your feet by $N's color spray!";
       WAIT_STATE(victim, PULSE_VIOLENCE * 2);
       if(!IS_NPC(ch)) {
       WAIT_STATE(ch, PULSE_VIOLENCE * 3);
       } else {
       GET_SKILL_WAIT(ch) = 2;
       }
    }
    break;

  case SPELL_BLAZEWARD:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("This protective magic does not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    af[0].duration = 8;
    af[0].bitvector = AFF_BLAZEWARD;

    accum_duration = FALSE;
    to_vict = "A brilliant golden blaze momentarily surrounds you.";
    to_room = "$n is surrounded by a brilliant golden blaze.";
    break; 

  case SPELL_CLEAR_SKY:
    if(OUTSIDE(ch)) {
	weather_info.sky = SKY_CLOUDLESS;
	send_to_char("The clouds part.\r\n", ch);
    }
    else {
	send_to_char("You must be outside!\r\n", ch);
    }
    break;

  case SPELL_INDESTR_AURA:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("This protective magic does not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    af[0].duration = 10;
    af[0].bitvector = AFF_INDESTR_AURA;

    accum_duration = FALSE;
    to_vict = "An indestructable aura surrounds you.";
    to_room = "$n is surrounded by an indestructable aura.";
    break;

  case SPELL_NETHERCLAW:
    if (!GET_EQ(ch, WEAR_WIELD)) {
      send_to_char("You need to be wielding a claw weapon.\r\n", ch);
      return;
    }
    if (GET_OBJ_VAL(GET_EQ(ch, WEAR_WIELD), 3) != TYPE_CLAW - TYPE_HIT) {
      send_to_char("Only claw weapons can be melded into the Netherclaw.\r\n", ch);
      return;
    }  
    if (GET_EQ(ch, WEAR_HANDS)) {
      send_to_char("You must have bare hands to meld into the Netherclaw.\r\n", ch);
      return;
    }
    af[0].duration = 10 + GET_TIER(ch);
    af[0].bitvector = AFF_NETHERCLAW;

    accum_duration = FALSE;
    to_vict = "You meld your weapon with your hand to create the Netherclaw!";
    to_room = "$n melds $s weapon with $s hand to create the Netherclaw!";
    break;
    } // End of spells 
    break;  

  case ABT_CHANT:
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      af[i].src = ABT_CHANT;
    /* Handle chants here */
    switch(spellnum) {
    case CHANT_ANCIENT_PROT:
      if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
        send_to_char("You cannot use this protection in the Grid.", ch);
        return;
      } 
      if (!MAJOR_PROTECT_OK(ch)) {
        send_to_char("You are already protected by other magic.\r\n", ch);
        return;
      }
      af[0].duration = 4;
      af[0].bitvector = AFF_ANCIENT_PROT;

      accum_duration = FALSE;
      to_vict = "The ancients begin protecting you.";
      to_room = "$n is surrounded by an ancient aura.";
      break;

    case CHANT_FIND_CENTER:
      af[0].location = APPLY_DEX;
      af[0].duration = 4;
      af[0].modifier = 2;
      af[1].location = APPLY_STR;
      af[1].duration = 4;
      af[1].modifier = 2;

      accum_duration = FALSE;
      to_vict = "Your energy is channeled.";
      to_room = "$n seems more centered.";
      break;

    case CHANT_OMM:
      af[0].location = APPLY_HITROLL;
      af[0].duration = dice(2, (number(40, 100) >> 3));
      if (GET_WIS(ch) > 18)
        af[0].modifier = +3;
      else
        af[0].modifier = +2;
      accum_duration = FALSE;
      to_vict = "You feel more controlled.";
      break;

    case CHANT_MENTAL_BARRIER:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
      af[0].bitvector = AFF_MENTAL_BARRIER;
      af[0].duration = dice(4, 2) + 1;
      accum_duration = FALSE;
      to_vict = "You are engulfed in a shroud of your mental energy.";
      to_room = "$n is engulfed in a shroud of mental energy.";
      break;

  case CHANT_PSIONIC_DRAIN:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].duration = 2;
    af[0].bitvector = AFF_SLOW;
    af[1].duration = 2;
    af[1].location = APPLY_HITROLL;
    af[1].modifier = -3;
    accum_duration = FALSE;
    to_vict = "You become very weak.";
    to_room = "$n becomes notably weaker.";
    break;

  case CHANT_BALANCE:
    af[0].duration = 1;
    af[0].bitvector = AFF_BALANCE;
    accum_duration = FALSE;
    to_vict = "You find the perfect balance.";
    to_room = "$n poises on the balls of $s feet.";
    break;

    } /* End of chants */
    break; 

  case ABT_PRAYER:
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      af[i].src = ABT_PRAYER;
    /* Handle prayers here */
    switch(spellnum) {

  case PRAY_HOLY_ARMOR:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (!MINOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].location = APPLY_AC;
    af[0].duration = 8;
    af[0].modifier = -30;
    af[1].location = APPLY_HITROLL;
    af[1].duration = 8;
    af[1].modifier = +4;
    af[0].bitvector = AFF_HOLYARMOR;
    accum_duration = FALSE;
    to_vict = "You feel ready to defend your god!";
    to_room = "$n is armored by $s god.";
    break;

  case PRAY_AGITATION:
    if (!pk_allowed && !pkill_ok(ch, victim))
      return;
    af[0].location = APPLY_HITROLL;
    af[0].modifier = -5;
    af[0].duration = 4 + (number(10, 50) / 10);

    af[1].location = APPLY_DAMROLL;
    af[1].modifier = -5;
    af[1].duration = 4 + (number(10, 50) / 10);

    af[2].location = APPLY_AC;
    af[2].modifier = 10;
    af[2].duration = 4 + (number(10, 50) / 10);

    accum_duration = FALSE;
    to_vict = "A horrible itching hits you suddenly";
    to_room = "$n begins to scratch himself vigorously";
    break;

    case PRAY_BLESSING:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
      af[0].location = APPLY_HITROLL;
      af[0].modifier = 3;
      af[0].duration = 6;
      af[1].location = APPLY_SAVING_SPELL;
      af[1].modifier = -1;
      af[1].duration = 6;
      accum_duration = FALSE;
      to_vict = "You feel righteous.";
      break;

    case PRAY_HEAVY_SKIN:
      if (IS_NPC(victim)) {
        send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
        return;
      }
      if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
        send_to_char("You cannot use this protection in the Grid.", ch);
        return;
      } 
      if (!MINOR_PROTECT_OK(ch)) {
        send_to_char("You are already protected by other magic.\r\n", ch);
        return;
      }
      af[0].location = APPLY_AC;
      af[0].modifier = -10;
      af[0].duration = MIN(number(5, 50)+3, 24);
      af[0].bitvector = AFF_HEAVY_SKIN;
      accum_duration = FALSE;
      to_vict = "The gods protect you.";
      break;

    case PRAY_DEFLECT:
      af[0].duration = 6;
      af[0].bitvector = AFF_DEFLECT;
      accum_duration = FALSE;
      to_vict = "A holy shield of deflection surrounds you.";
      to_room = "$n is surrounded by a holy shield of deflection.";
      break;

    case PRAY_SANCTUARY:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID) &&
        GET_CLASS(ch) != CLASS_CLERIC) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("This protective magic does not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires have no need for such weak magics.\r\n", ch);
      return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    if ((GET_CLASS(ch) == CLASS_CLERIC) ||
        (GET_CLASS(ch) == CLASS_ARCANIC) ||
        (GET_CLASS(ch) == CLASS_TEMPLAR) ||
        (GET_CLASS(ch) == CLASS_STORR) ||
        (GET_CLASS(ch) == CLASS_CRUSADER) ||
        (GET_CLASS(ch) == CLASS_SAGE))
      af[0].duration = 1 + (GET_LEVEL(ch) * 4 / 50);
    else
      af[0].duration = 6;

      af[0].bitvector = AFF_SANCTUARY;
      accum_duration = FALSE;
      to_vict = "A shimmering, white aura surrounds you.";
      to_room = "$n is surrounded by a shimmering white aura.";
      break;

    case PRAY_SECOND_SIGHT:
      af[0].duration = 4 + (number(5, 10) >> 2);
      af[0].bitvector = AFF_DETECT_INVIS;
      accum_duration = FALSE;
      to_vict = "Your eyes take on a new dimension of sight.";
      break;

    case PRAY_INFRAVISION:
      af[0].duration = 6 + (number(10, 50) >> 2);
      af[0].bitvector = AFF_INFRAVISION;
      accum_duration = FALSE;
      to_vict = "Your eyes become ultra-sensitive to light.";
      to_room = "$n's eyes glow red.";
      break;

    case PRAY_WEAKENED_FLESH:
      if(!pkill_ok(ch, victim)) {
        send_to_char("You fail.\r\n", ch);
        return;
      }
      af[0].duration = 4;
      af[0].bitvector = AFF_WEAKENEDFLESH;
      accum_duration = FALSE;
      to_vict = "Your skin whithers up and becomes weak!.";
      to_room = "$n's skin shrivels up and becomes weak!.";
      break;

    case PRAY_BLINDNESS:
      if (!pkill_ok(ch, victim) || MOB_FLAGGED(victim, MOB_NOBLIND) ||
          AFF_FLAGGED(victim, AFF_NOBLIND)||mag_savingthrow(victim, savetype)) {
        send_to_char("You fail.\r\n", ch);
        return;
      }

      af[0].location = APPLY_HITROLL;
      af[0].modifier = -4;
      af[0].duration = 2;
      af[0].bitvector = AFF_BLIND;

      af[1].location = APPLY_AC;
      af[1].modifier = 40;
      af[1].duration = 2;
      accum_duration = FALSE;
      af[1].bitvector = AFF_BLIND;

      to_room = "$n seems to be blinded!";
      to_vict = "You have been blinded!";
      break;

    case PRAY_HASTE:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use this on a mob.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_TITAN) {
      send_to_char("Try as you might, you can't speed a titan up.\r\n", ch);
      return;
    } 
      af[0].bitvector = AFF_HASTE;
      af[0].duration = 3 + (number(50, 75) / 2);
      af[0].location = APPLY_HITROLL;
      af[0].modifier = -2;
      accum_duration = FALSE;
      accum_affect = FALSE;
      to_vict = "You feel faster";
      to_room = "$n visibly speeds up.";
      break;

    case PRAY_SACRED_SHIELD:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
       send_to_char("Sacred shield. On a vampire. Riiigght.\r\n", ch);
       return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
      af[0].duration = 4;
      af[0].bitvector = AFF_SACRED_SHIELD;
      accum_duration = FALSE;
      to_vict = "A shield of sacred energy surrounds you.";
      to_room = "$n is surrounded by a shield of sacred energy.";
      break;

   case PRAY_DROWSE:
      if(!pkill_ok(ch, victim)) {
        send_to_char("You fail.\r\n", ch);
        return;
      }
      if(IS_NPC(victim)) {
        send_to_char(NOEFFECT, ch);
        return;
      }
      af[0].location = APPLY_AC;
      af[0].modifier = 10;
      af[0].duration = 1 + (number(20, 140) / 27);
      af[0].bitvector = AFF_DROWSE;
      af[1].location = APPLY_HITROLL;
      af[1].modifier = -5;
      af[1].duration = 1 + (number(20, 140) / 27);
      af[1].bitvector = AFF_DROWSE;
      accum_duration = FALSE;
      to_room = "$n's eyes begin to droop, $s movements become sluggish.";
      to_vict = "You become very tired, your ability to fight decreases.";

    case PRAY_AFTERLIFE:
      if (GET_ALIGNMENT(ch) <= 0) {
        send_to_char("Your alignment prevents any communals with your gods.\r\n"
                     "You cannot use this prayer right now.\r\n", ch);
        return;
      } else
      af[0].duration = 5 + GET_TIER(ch);
      af[0].bitvector = AFF_AFTERLIFE;
      accum_duration = FALSE;
      to_vict = "Your prayer is answered, your soul is protected.";
      to_room = "$n closes $s eyes, drawing upon $s saintly powers.";
      break; 

    case PRAY_ENLIGHTENMENT:
      if (GET_ALIGNMENT(ch) <= 0) {
        send_to_char("Your alignment prevents any communals with your gods.\r\n"
                     "You cannot use this prayer right now.\r\n", ch);
        return;
      } else
      af[0].duration = 5 + GET_TIER(ch);
      af[0].bitvector = AFF_ENLIGHT;
      accum_duration = FALSE;
      to_vict = "Your prayer is answered, your are enlightened.";
      to_room = "$n closes $s eyes, enlightening $mself.";
      break;

  case PRAY_HEAVENSBEAM:
    if (!number(0, 4) && GET_ALIGNMENT(victim) <= 0) {
      GET_POS(victim) = POS_SITTING;
      to_room = "$n is blasted off $s feet by the light of heaven!";
      to_vict = "You are knocked off of your feet by the light of heaven!";
      GET_SKILL_WAIT(ch) = 2;
    }
    if (!number(0, 4) && GET_ALIGNMENT(victim) <= 0) {
      af[0].duration = 2;
      af[0].bitvector = AFF_BLIND;
      accum_duration = FALSE;
      to_vict = "The light of heaven blinds you!";
      to_room = "$n is blinded by the light of heaven!";
    }
    break;

  case PRAY_DIVINESHIELD:
    if (IS_NPC(victim)) {
      send_to_char("Players only, you cannot use protectives on mobs.\r\n", ch);
      return;
    }
    if (!MAJOR_PROTECT_OK(ch)) {
      send_to_char("You are already protected by other magic.\r\n", ch);
      return;
    }
    af[0].duration = GET_TIER(ch) * 4;
    af[0].bitvector = AFF_DIVINE_SHIELD;
    accum_duration = FALSE;
    to_vict = "You are surrounded by a divine shield.";
    to_room = "$n is surrounded by a divine shield.";
    break;

  case PRAY_ETHEREAL:
    af[0].bitvector = AFF_ETHEREAL;
    af[0].duration = 3;
    af[0].location = APPLY_AC;
    af[0].modifier = -10;
    accum_duration = FALSE;
    accum_affect = FALSE;
    to_vict = "You body becomes ethereal, one with the angels.";
    to_room = "$n's body becomes almost angelic, taking on an ethereal form.";
    break;

    } /* End of prayers */
    break; 
  case ABT_SONG:
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      af[i].src = ABT_SONG;
    /* Handle songs here */
    switch(spellnum) {
    case SONG_LULLABY:
      if ((!pk_allowed && !pkill_ok(ch, victim)) ||
          MOB_FLAGGED(victim, MOB_NOSLEEP) ||
          mag_savingthrow(victim, savetype))
        return;
 
      af[0].duration = 4 + (GET_LEVEL(ch) >> 2);
      af[0].bitvector = AFF_SLEEP;
      accum_duration = FALSE;

      if (GET_POS(victim) > POS_SLEEPING) {
        act("You feel very sleepy.  Zzzz...", FALSE, victim, 0, 0, TO_CHAR);
        act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
        GET_POS(victim) = POS_SLEEPING;
      }
      break;

    } /* End of songs */
    break; 
  }

  /*
   * If this is a mob that has this affect set in its mob file, do not
   * perform the affect.  This prevents people from un-sancting mobs
   * by sancting them and waiting for it to fade, for example.
   */
  if (IS_NPC(victim) && !affected_by_spell(victim, spellnum, type))
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
      if (IS_AFFECTED(victim, af[i].bitvector)) {
	send_to_char(NOEFFECT, ch);
	return;
      }

  /*
   * If the victim is already affected by this spell, and the spell does
   * not have an accumulative effect, then fail the spell.
   */
  if (affected_by_spell(victim, spellnum, type) &&
      !(accum_duration||accum_affect)) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  for (i = 0; i < MAX_SPELL_AFFECTS; i++)
    if (af[i].bitvector || (af[i].location != APPLY_NONE))
      affect_join(victim, af+i, accum_duration, FALSE, accum_affect, FALSE);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);
}


/*
 * This function is used to provide services to mag_groups.  This function
 * is the one you should change to add new group spells.
 */

void perform_mag_groups(int level, struct char_data * ch,
			struct char_data * tch, int spellnum,
                        byte type, int savetype)
{

    if (ch->in_room != tch->in_room)
      return;  

  switch (type) {
  case ABT_SPELL:
  switch (spellnum) {
    case SPELL_GROUP_HEAL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this magic in the Grid.", ch);
      return;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (ROOM_FLAGGED(tch->in_room, ROOM_GRID)) {
      send_to_char("You cannot receive this magic in the Grid.", ch);
      return;
    }
    if (GET_CLASS(tch) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
      mag_points(level, ch, tch, SPELL_HEAL, type, savetype);
      break;
    case SPELL_GROUP_ARMOR:
      mag_affects(level, ch, tch, SPELL_ARMOR, type, savetype);
      break;
    case SPELL_GROUP_RECALL:
      spell_recall(level, ch, tch, NULL);
      break;
    }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {
    case PRAY_GROUP_ARMOR:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
      mag_affects(level, ch, tch, PRAY_HEAVY_SKIN, type, savetype);
      break;

    case PRAY_GROUP_HEAL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(tch) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
      mag_points(level, ch, tch, PRAY_CURE_LIGHT, type, savetype);
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;
  }
}


/*
 * Every spell that affects the group should run through here
 * perform_mag_groups contains the switch statement to send us to the right
 * magic.
 *
 * group spells affect everyone grouped with the caster who is in the room,
 * caster last.
 *
 * To add new group spells, you shouldn't have to change anything in
 * mag_groups -- just add a new case to perform_mag_groups.
 */

void mag_groups(int level, struct char_data * ch, int spellnum, byte type, int savetype)
{
  struct char_data *tch, *k;
  struct follow_type *f, *f_next;

  if (ch == NULL)
    return;

  if (!IS_AFFECTED(ch, AFF_GROUP))
    return;
  if (ch->master != NULL)
    k = ch->master;
  else
    k = ch;
  for (f = k->followers; f; f = f_next) {
    f_next = f->next;
    tch = f->follower;
    if (tch->in_room != ch->in_room)
      continue;
    if (!IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (ch == tch)
      continue;
    perform_mag_groups(level, ch, tch, spellnum, type, savetype);
  }

  if ((k != ch) && IS_AFFECTED(k, AFF_GROUP))
    perform_mag_groups(level, ch, k, spellnum, type, savetype);
  perform_mag_groups(level, ch, ch, spellnum, type, savetype);
}


/*
 * mass spells affect every creature in the room except the caster.
 *
 * No spells of this class currently implemented as of Circle 3.0.
 */

void mag_masses(int level, struct char_data * ch, int spellnum, byte type, int savetype)
{
  struct char_data *tch, *tch_next;

  for (tch = world[ch->in_room].people; tch; tch = tch_next) {
    tch_next = tch->next_in_room;
    if (tch == ch)
      continue;

    switch (spellnum) {
    }
  }
}


/*
 * Every spell that affects an area (room) runs through here.  These are
 * generally offensive spells.  This calls mag_damage to do the actual
 * damage -- all spells listed here must also have a case in mag_damage()
 * in order for them to work.
 *
 *  area spells have limited targets within the room.
*/

void mag_areas(int level, struct char_data * ch, int spellnum, byte type, int savetype)
{
  struct char_data *tch, *next_tch;
  char *to_char = NULL;
  char *to_room = NULL;

  if (ch == NULL)
    return;

  /*
   * to add spells to this fn, just add the message here plus an entry
   * in mag_damage for the damaging part of the spell.
   */
  switch (type) {
  case ABT_SPELL:
    switch (spellnum) {
    case SPELL_FIRE_BREATH:
      to_char = "You snort and fire shoots out of your nostrils!";
      to_room = "A gout of fire shoots out of $n's nostrils at you!";
    break;
    case SPELL_GAS_BREATH:
      to_char = "You burp and a noxious gas rolls from your nostrils!";
      to_room = "$n rumbles and a noxious gas rolls out of $s nostrils!";
    break;
    case SPELL_FROST_BREATH:
      to_char = "You shiver as a shaft of frost leaps from your mouth!";
      to_room = "$n shivers as a shaft of frost leaps from $s mouth!";
    break;
    case SPELL_ACID_BREATH:
      to_char = "A wash of acid leaps from your mouth!";
      to_room = "$n looks pained as a wash of acid leaps from $s mouth!";
    break;
    case SPELL_LIGHTNING_BREATH:
      to_char = "You open your mouth and bolts of lightning shoot out!";
      to_room = "$n opens $s mouth and bolts of lightning shoot out!";
    break;
    case SPELL_EARTHQUAKE:
      to_char = "You gesture and the earth begins to shake all around you!";
      to_room="$n gracefully gestures and the earth begins to shake violently!";
    break;
    case SPELL_CONFUSE:
      to_char = "You move your hands in rapid motions, confusing everyone in the room.";
      to_room = "$n moves $s hands in rapid motions, confusing everyone in the room.";
    break;
    case SPELL_FIRESTORM:
      to_char = "You gesture and suddenly, fire is raining from the sky!";
      to_room="As $n gestures, fire starts pooring like rain from the heavens!";
      break;
    case SPELL_ICESTORM:
      to_char = "You gesture and suddenly, large shards of ice jet down from the sky!";
      to_room ="As $n gestures, large shards of ice come jetting down from the heavens!";
      break;
    case SPELL_RAIN_OF_FIRE:
      to_char = "You raise your scaly arms high overhead, calling down a rain of fire!";
      to_room = "$n raises $s scaly arms above $s head, calling down a rain of fire!";
      break;
    }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {
    case PRAY_EARTHQUAKE:
      to_char = "You gesture and the earth begins to shake all around you!";
      to_room="$n gracefully gestures and the earth begins to shake violently!";
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;
  }
  if (to_char != NULL)
    act(to_char, FALSE, ch, 0, 0, TO_CHAR);
  if (to_room != NULL)
    act(to_room, FALSE, ch, 0, 0, TO_ROOM);
  

  for (tch = world[ch->in_room].people; tch; tch = next_tch) {
    next_tch = tch->next_in_room;

    /*
     * The skips: 1: the caster
     *            2: immortals
     *            3: if no pk on this mud, skips over all players
     *            4: pets (charmed NPCs)
     *            5: mounts ridden by players
     *            6: people grouped with the caster
     * players can only hit players in CRIMEOK rooms 4) players can only hit
     * charmed mobs in CRIMEOK rooms
     */

    if (tch == ch)
      continue;
    if (!IS_NPC(tch) && GET_LEVEL(tch) >= LVL_IMMORT)
      continue;
    if (tch == ch->master)
      continue;
    if (IS_AFFECTED(tch, AFF_GROUP))
      continue;
    if (!pk_allowed && !pkill_ok(ch, tch) && !IS_NPC(ch) && !IS_NPC(tch))
      continue;
    if (!IS_NPC(ch) && IS_NPC(tch) && IS_AFFECTED(tch, AFF_CHARM))
      continue;
    if (GET_RIDER(tch) && (!(ROOM_FLAGGED(tch->in_room, ROOM_ARENA))))
      continue;
    mag_damage(GET_LEVEL(ch), ch, tch, spellnum, type, 1);
  }
}


/*
 *  Every spell which summons/gates/conjours a mob comes through here.
 *
 *  None of these spells are currently implemented in Circle 3.0; these
 *  were taken as examples from the JediMUD code.  Summons can be used
 *  for spells like clone, ariel servant, etc.
 */

static char *mag_summon_msgs[] = {
  "\r\n",
  "$n makes a strange magical gesture; you feel a strong breeze!\r\n",
  "$n animates a corpse!\r\n",
  "$N appears from a cloud of thick blue smoke!\r\n",
  "$N appears from a cloud of thick green smoke!\r\n",
  "$N appears from a cloud of thick red smoke!\r\n",
  "$N disappears in a thick black cloud!\r\n"
  "As $n makes a strange magical gesture, you feel a strong breeze.\r\n",
  "As $n makes a strange magical gesture, you feel a searing heat.\r\n",
  "As $n makes a strange magical gesture, you feel a sudden chill.\r\n",
  "As $n makes a strange magical gesture, you feel the dust swirl.\r\n",
  "$n magically divides!\r\n",
  "$n animates a corpse!\r\n",
  "$N descends from the heavens and trots up to $n.\r\n",
  "$N rises from a heap of ashes and stands beside $n!\r\n"
};

static char *mag_summon_fail_msgs[] = {
  "\r\n",
  "There are no such creatures.\r\n",
  "Uh oh...\r\n",
  "Oh dear.\r\n",
  "Oh shit!\r\n",
  "The elements resist!\r\n",
  "You failed.\r\n",
  "There is no corpse!\r\n"
};


// stock circle mobile defines
#define MOB_CLONE		69
#define MOB_ZOMBIE		101
#define MOB_AERIALSERVANT	109
// Dibrova mob summon defines
#define MOB_UNDEAD_MOUNT         32
#define MOB_FAMILIAR_CAT         50
#define MOB_FAMILIAR_OWL         51
#define MOB_FAMILIAR_SNAKE       52
#define MOB_FAMILIAR_FERRET      53
#define MOB_INVIS_STALKER        54
#define MOB_HOLY_STEED           56
#define MOB_DEMON_SUMMON_I       57
#define MOB_DEMON_SUMMON_II      58
#define MOB_DEMON_SUMMON_III     59
#define MOB_DEMON_SUMMON_IV      60
#define MOB_DHALIN               61
#define MOB_FEHZOU               62
#define MOB_NECROLIEU            63
#define MOB_ANIMATED_I           64
#define MOB_ANIMATED_II          65
#define MOB_ANIMATED_III         66
#define MOB_DRAKE                67

void mag_summons(int level, struct char_data * ch, struct obj_data * obj,
		      int spellnum, byte type, int savetype)
{
  struct char_data *mob = NULL;
  struct obj_data *tobj, *next_obj;
  struct follow_type *f;
//  struct affected_type af;

  int pfail = 0;
  int msg = 0, fmsg = 0;
  int num = 1;
  int i;
  int mob_num = 0;
  int handle_corpse = 0;
  int num_followers = 0;
  bool failed = FALSE;
  bool summon_follow = TRUE;
  bool handle_jar = FALSE;
  bool is_mount = FALSE;
  if (ch == NULL)
    return;

  if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
    send_to_char("You cannot use this magic in the Grid.", ch);
    return;
  }

  switch (spellnum) {
  case SPELL_ANIMATE_CORPSE:
    if ((obj == NULL) || (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) ||
	(!GET_OBJ_VAL(obj, 3))) {
      act(mag_summon_fail_msgs[7], FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
    if (GET_OBJ_VAL(obj, 3) == 1) {
      if (obj->obj_flags.pid && GET_PFILEPOS(ch) != obj->obj_flags.pid) {
        send_to_char("You can't touch that corpse.\r\n", ch);
        return;
      }
    }
    handle_corpse = 1;
    msg = 2;

    if (GET_LEVEL(ch) < 100)
      mob_num = MOB_ANIMATED_I;
    else if (GET_LEVEL(ch) > 99 && GET_LEVEL(ch) < 125)
      mob_num = MOB_ANIMATED_II;
    else if (GET_LEVEL(ch) > 124)
      mob_num = MOB_ANIMATED_III;
    else
      mob_num = MOB_ANIMATED_I;

    pfail = 8;
    break;

  case SPELL_FIND_FAMILIAR:
    msg = 12;
    mob_num = number(MOB_FAMILIAR_CAT, MOB_FAMILIAR_FERRET);
    summon_follow = FALSE;
    handle_jar = TRUE;
    pfail = 8;
    break;

  case SPELL_INVIS_STALKER:
    msg = 12;
    mob_num = MOB_INVIS_STALKER;
    summon_follow = FALSE;
    handle_jar = TRUE;
    pfail = 8;
    break;

  case SPELL_UNDEAD_MOUNT:
    mob_num = MOB_UNDEAD_MOUNT;
    summon_follow = FALSE;
    is_mount = TRUE;
    msg = 13;
    pfail = 5;
    break;

  case SPELL_HOLY_STEED:
    mob_num = MOB_HOLY_STEED;
    summon_follow = FALSE;
    is_mount = TRUE;
    pfail = 5;
    msg = 12;
    break;

  case SPELL_SUMMON_DHALIN:
    msg = 3;
    mob_num = MOB_DHALIN;
    pfail = 7;
    break;

  case SPELL_SUMMON_FEHZOU:
    msg = 4;
    mob_num = MOB_FEHZOU;
    pfail = 7;
    break;

  case SPELL_SUMMON_NECROLIEU:
    msg = 5;
    mob_num = MOB_NECROLIEU;
    pfail = 8;
    break;

  case SPELL_CONJURE_FIREDRAKE:
    msg = 5;
    mob_num = MOB_DRAKE;
    pfail = 8;
    break;

  default:
    return;
  }

  if (IS_AFFECTED(ch, AFF_CHARM)) {
    send_to_char("You are too giddy to have any followers!\r\n", ch);
    return;
  }

  for (f = ch->followers; f; f = f->next) {
    if (IS_NPC(f->follower))
      num_followers++;
  }
 


  if (num_followers >= cha_max_followers[GET_CHA(ch)]) {
    send_to_char("Your charisma won't support any more followers.\r\n", ch);
    failed = TRUE;
  }

  if (number(0, 51) < pfail) {
    send_to_char(mag_summon_fail_msgs[fmsg], ch);
    failed = TRUE;
  }
  /* If spell is animate dead, just bail with a humorous message. */
  if (failed && handle_corpse) {
//  send_to_char(funny message);
//  act(message);
    return;
  }

  /* Don't bother with clone failures either */
  if (failed && (spellnum == SPELL_CLONE)) {
//  send_to_char("You can't create that many copies of yourself, it's );
//  act(message);
    return;
  }

  if (handle_jar) {
    if (num_followers > 0) {
      send_to_char("It is too afraid of the others you've summoned!\r\n", ch);
      return;
    }
  }

  for (i = 0; i < num; i++) {
    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
      send_to_char("You don't quite remember how to make that creature.\r\n", ch);
      return;
    }
    char_to_room(mob, ch->in_room);
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;
    load_mtrigger(mob);

    /* Caster failed. Summoned creature arrives, but enraged rather than compliant! */
    if (failed) {
      act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
      act("$n arrives enraged and attacks you!", FALSE, mob, 0, 0, TO_CHAR);
      act("$n arrives enraged at being summoned and attacks $N!", FALSE, mob, 0, ch, TO_ROOM);
      if (handle_jar)
        handle_jar = FALSE;
      hit(mob, ch, TYPE_UNDEFINED);
    } else {
      if (is_mount) {
        sprintf(buf, "Your mount %s and stands by your side.\r\n",
         (spellnum == SPELL_UNDEAD_MOUNT ? "rises from a heap of ashes" : "descends from the heavens"));
        send_to_char(buf, ch);
      }
      if (!handle_jar) 
        act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);
      if (summon_follow) {
        SET_BIT_AR(AFF_FLAGS(mob), AFF_CHARM);
        add_follower(mob, ch);
      }
//      act(mag_summon_msgs[msg], FALSE, ch, 0, mob, TO_ROOM);

      if (spellnum == SPELL_CLONE) {
        strcpy(GET_NAME(mob), GET_NAME(ch));
        strcpy(mob->player.short_descr, GET_NAME(ch));
      }
    }
  }

  if (handle_jar) {
    act("$n hops out of the bushes and runs up to $N.", FALSE, mob, 0, ch, TO_NOTVICT);
    act("You become one with $N.", FALSE, ch, 0, mob, TO_CHAR);
    send_to_char("Type return to go back to your body.\r\n", ch);
    ch->desc->character = mob;
    ch->desc->original = ch;
    mob->desc = ch->desc;
    ch->desc = NULL;
  }
  if (handle_corpse) {
    for (tobj = obj->contains; tobj; tobj = next_obj) {
      next_obj = tobj->next_content;
      obj_from_obj(tobj);
      obj_to_char(tobj, mob);
    }
    extract_obj(obj);
  }
}


void mag_points(int level, struct char_data * ch, struct char_data * victim,
		     int spellnum, byte type, int savetype)
{
  int hit = 0;
  int move = 0;
  int qi = 0;
  int vim = 0;
  int aria = 0;

  if (victim == NULL)
    return;

  switch (type) {
  case ABT_SPELL:
  switch (spellnum) {
  case SPELL_CURE_LIGHT:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    hit = dice(1, 8) + 1 + (level >> 2);
    send_to_char("You feel better.\r\n", victim);
    break;

  case SPELL_CURE_CRITIC:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (dice(3, 8) + 3 + (level >> 2) / 2);
    else
      hit = dice(3, 8) + 3 + (level >> 2);
    send_to_char("You feel a lot better!\r\n", victim);
    break;

  case SPELL_HEAL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (100 + dice(3, 8) / 2);
    else
      hit = 100 + dice(3, 8);
    send_to_char("A warm feeling floods your body.\r\n", victim);
    break;

  case SPELL_CUREALL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.\r\n", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN) {
      hit = (3000 + dice(1, 1) / 2);
      move = (3000 + dice (1, 1) / 2);
    } else
      hit = (3000 + dice(1, 1) / 2);
      move = (3000 + dice (1, 1) / 2);
    send_to_char("Your body surges with power as your fatigue fades away.\r\n", victim);
    break;
  case SPELL_REFRESH:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
      move = dice(2, 3) + 25 + (level >> 3) + GET_SPELLPOWER(ch);
      send_to_char("You feel the spring return to your step!\r\n", victim);
      break;  
  case SPELL_ENTANGLE:
    GET_MOVE(victim) = 0;
    send_to_room("Roots erupt out of the ground, thrashing about you!\r\n", ch->in_room);
    send_to_char("You have been entangled!\r\n", victim);
    break;

  }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    case CHANT_HEALING_HAND:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (dice(5, 6) + (level >> 2) / 2);
    else
      hit = dice(5, 6) + (level >> 2);
      send_to_char("You feel much better.\r\n", victim);
      break;

    case CHANT_LEND_HEALTH:
      /* Pretty manual here... */
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
      hit = dice(6, 3);
      if ((hit*1.2) > GET_HIT(ch)) {
        send_to_char("You don't have the health to spare.\r\n", ch);
        return;
      }
      hit = MIN(hit, GET_MAX_HIT(victim)-GET_HIT(victim));
      if (!hit) {
        send_to_char("They don't need your health just now.\r\n", ch);
        return;
      }
      act("You feel $n's health rush into your body.", TRUE, ch, 0, 
          victim, TO_VICT);
      act("You feel your health rush into $N's body.", TRUE, ch, 0, 
          victim, TO_VICT);
      GET_HIT(ch) -= (int)(hit*1.2);
      break;
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {

    case PRAY_CURE_LIGHT:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (dice(1, 8) + 1 + (level >> 2) + GET_SPELLPOWER(ch) / 2);
    else
      hit = dice(1, 8) + 1 + (level >> 2) + GET_SPELLPOWER(ch);
      send_to_char("You feel better.\r\n", victim);
      break;

    case PRAY_CURE_SERIOUS:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (20 + dice(2, 6) + 2 + (level >> 2) + GET_SPELLPOWER(ch) / 2);
    else
      hit = 20 + dice(2, 6) + 2 + (level >> 2) + GET_SPELLPOWER(ch);
      send_to_char("You feel much better.\r\n", victim);
      break;

    case PRAY_CURE_CRITICAL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (50 + dice(3, 8) + 3 + (level >> 2) + GET_SPELLPOWER(ch) / 2);
    else
      hit = 50 + dice(3, 8) + 3 + (level >> 2) + GET_SPELLPOWER(ch);
      send_to_char("You feel a lot better!\r\n", victim);
      break;

    case PRAY_REFRESH:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
      move = dice(2, 3) + 25 + (level >> 3) + GET_SPELLPOWER(ch);
      send_to_char("You feel the spring return to your step!\r\n", victim);
      break;
    case PRAY_HEAL:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (100 + dice(3, 8) + GET_SPELLPOWER(ch) / 2);
    else
      hit = 100 + dice(3, 8) + GET_SPELLPOWER(ch);
      send_to_char("A warm feeling floods your body.\r\n", victim);
      break;
    case PRAY_VITALITY:
    if (ROOM_FLAGGED(ch->in_room, ROOM_GRID)) {
      send_to_char("You cannot use this protection in the Grid.", ch);
      return;
    } 
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Healing magics do not work in the Pit.\r\n"
                   "You must fight a fair fight - to the death.\r\n", ch);
      return;
    }
    if (GET_CLASS(victim) == CLASS_VAMPIRE) {
      send_to_char("Vampires are resistant to healing magics.\r\n", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_TITAN)
      hit = (50 + level + dice(10, 15) + GET_SPELLPOWER(ch) / 2);
    else
      hit = 50 + level + dice(10, 15) + GET_SPELLPOWER(ch);
      send_to_char("A rush of vitality infuses your being.\r\n", victim);
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;
  }
  GET_HIT(victim) = MIN(GET_MAX_HIT(victim), GET_HIT(victim) + hit);
  GET_MOVE(victim) = MIN(GET_MAX_MOVE(victim), GET_MOVE(victim) + move);
  GET_QI(victim) = MIN(GET_MAX_QI(victim), GET_QI(victim) + qi);
  GET_VIM(victim) = MIN(GET_MAX_VIM(victim), GET_VIM(victim) + vim);
  GET_ARIA(victim) = MIN(GET_MAX_ARIA(victim), GET_ARIA(victim) + aria);
  update_pos(victim);
}


void mag_unaffects(int level, struct char_data * ch, struct char_data * victim,
		        int spellnum, byte type, int savetype)
{
  int rem_num = 0;
  int rem_type = 0;
  int rem_num2 = 0;
  int rem_type2 = 0;
  int hit = 0;
  char *to_vict = NULL, *to_room = NULL;

  if (victim == NULL)
    return;

  switch (type) {
  case ABT_SPELL:
  switch (spellnum) {
  case SPELL_CURE_BLIND:
  case SPELL_HEAL:
  case SPELL_CUREALL:
    rem_type = ABT_SPELL;
    rem_num = SPELL_BLINDNESS;
    rem_type2 = ABT_PRAYER;
    rem_num2 = PRAY_BLINDNESS;
    to_vict = "Your vision returns!";
    to_room = "There's a momentary gleam in $n's eyes.";
    break;
  case SPELL_REMOVE_POISON:
    rem_type = ABT_SPELL;
    rem_num = AFF_POISON;
    rem_num = SPELL_POISON;
    to_vict = "A warm feeling runs through your body!";
    to_room = "$n looks better.";
    break;
  case SPELL_REMOVE_CURSE:
    rem_type = ABT_SPELL;
    rem_num = SPELL_CURSE;
    to_vict = "You don't feel so unlucky.";
    break;
  }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {
    case PRAY_REMOVE_POISON:
      rem_type = ABT_SPELL;
      rem_num = AFF_POISON;
      rem_num = SPELL_POISON;
      to_vict = "A warm feeling runs through your body!";
      to_room = "$n looks better.";
      break;
    case PRAY_LIFT_CURSE:
      rem_type = ABT_SPELL;
      rem_num = SPELL_CURSE;
      to_vict = "You don't feel so unlucky.";
      break;
    case PRAY_CURE_BLIND:
    case PRAY_HEAL:
    case PRAY_VITALITY:
      rem_type = ABT_PRAYER;
      rem_num = PRAY_BLINDNESS;
      rem_type2 = ABT_SPELL;
      rem_num2 = SPELL_BLINDNESS;
      to_vict = "Your vision returns!";
      to_room = "There's a momentary gleam in $n's eyes.";
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;

  }

  if (affected_by_spell(victim, rem_num, rem_type))
    hit |=1;
  if (affected_by_spell(victim, rem_num2, rem_type2))
    hit |=2;

  if (!hit) {
    send_to_char(NOEFFECT, ch);
    return;
  }

  if (hit | 1)
    affect_from_char(victim, rem_num, rem_type);

  if (hit | 2)
    affect_from_char(victim, rem_num2, rem_type2);

  if (to_vict != NULL)
    act(to_vict, FALSE, victim, 0, ch, TO_CHAR);
  if (to_room != NULL)
    act(to_room, TRUE, victim, 0, ch, TO_ROOM);

}


void mag_alter_objs(int level, struct char_data * ch, struct obj_data * obj,
		         int spellnum, byte type, int savetype)
{
  char *to_char = NULL;
  char *to_room = NULL;

  if (obj == NULL)
    return;

  switch (type) {
  case ABT_SPELL:
    switch (spellnum) {
    case SPELL_BLESS:
      if (!IS_OBJ_STAT(obj, ITEM_BLESS) &&
          (GET_OBJ_WEIGHT(obj) <= 5 * GET_LEVEL(ch))) {
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_BLESS);
        to_char = "$p glows briefly.";
      }
      break;
    case SPELL_CURSE:
      if (!IS_OBJ_STAT(obj, ITEM_NODROP)) {
        SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)--;
        to_char = "$p briefly glows red.";
      }
      break;
    case SPELL_INVISIBLE:
      if (!IS_OBJ_STAT(obj, ITEM_NOINVIS | ITEM_INVISIBLE)) {
        SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        to_char = "$p vanishes.";
      }
      break;
    case SPELL_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
          (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
          (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && !GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 1;
        to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT_AR(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "$p briefly glows blue.";
      }
      break;
    case SPELL_REMOVE_POISON:
      if (((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN) ||
         (GET_OBJ_TYPE(obj) == ITEM_FOOD)) && GET_OBJ_VAL(obj, 3)) {
        GET_OBJ_VAL(obj, 3) = 0;
        to_char = "$p steams briefly.";
      }
      break;
    case SPELL_REMOVE_INVIS:
      REMOVE_BIT_AR(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
      to_char = "$p fades into focus.";
      break;
    }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {
     case PRAY_LIFT_CURSE:
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        REMOVE_BIT_AR(obj->obj_flags.extra_flags, ITEM_NODROP);
        if (GET_OBJ_TYPE(obj) == ITEM_WEAPON)
          GET_OBJ_VAL(obj, 2)++;
        to_char = "$p briefly glows blue.";
      }
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;
  }

  if (to_char == NULL)
    send_to_char(NOEFFECT, ch);
  else
    act(to_char, TRUE, ch, obj, 0, TO_CHAR);

  if (to_room != NULL)
    act(to_room, TRUE, ch, obj, 0, TO_ROOM);
  else if (to_char != NULL)
    act(to_char, TRUE, ch, obj, 0, TO_ROOM);

}



void mag_creations(int level, struct char_data * ch, int spellnum, byte type)
{
  struct obj_data *tobj;
  int z=0;

  if (ch == NULL)
    return;
  level = MAX(MIN(level, LVL_IMPL), 1);

  switch (type) {
  case ABT_SPELL:
    switch (spellnum) {
    case SPELL_CREATE_FOOD:
      z = 10;
      break;
    case SPELL_CREATESPRING:
      z = 10051;
      break;
    }
    break;
  case ABT_CHANT:
    switch (spellnum) {
    }
    break;
  case ABT_PRAYER:
    switch (spellnum) {
    case PRAY_FOOD:
      z = 10;
      break;
    }
    break;
  case ABT_SONG:
    switch (spellnum) {
    }
    break;
  }

  if (!z) {
    send_to_char("Spell unimplemented, it would seem.\r\n", ch);
    return;
  }

  if (!(tobj = read_object(z, VIRTUAL))) {
    send_to_char("I seem to have goofed.\r\n", ch);
    sprintf(buf, "SYSERR: spell_creations, spell %d, obj %d: obj not found",
	    spellnum, z);
    log(buf);
    return;
  }
  switch(spellnum){
  case SPELL_CREATESPRING:
    obj_to_room(tobj, IN_ROOM(ch));
    break;
  default:
    obj_to_char(tobj, ch);
    break;
  }
  act("$n creates $p.", FALSE, ch, tobj, 0, TO_ROOM);
  act("You create $p.", FALSE, ch, tobj, 0, TO_CHAR);
  load_otrigger(tobj);
}

