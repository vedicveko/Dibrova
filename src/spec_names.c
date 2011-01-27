/* ************************************************************************
*   File: spec_names.c                                  Part of CircleMUD *
*  Usage: Keep the names of the spec procs handy for stats and stuff      *
*         like that                                                       *
*                                                                         *
*  By Gekke Eekhoorn of BUG, march 1995.                                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include <string.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"

#include "spec_names.h"  /* definition of proctype */
  SPECIAL(postmaster);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(guild);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  SPECIAL(gym);
  SPECIAL(library);
  SPECIAL(street_tough);
  SPECIAL(blood_bank);
  SPECIAL(casino);
  SPECIAL(lava_beast);
  SPECIAL(questmaster); 
  SPECIAL(ankou_follower);
  SPECIAL(flaming_efreet);
  SPECIAL(newbie_guard);
  SPECIAL(newbie_helper);
  SPECIAL(play_war);
  SPECIAL(simple_heal); 
  SPECIAL(bank);
  SPECIAL(scaredy_cat);
  SPECIAL(gen_board);   /*end of objs */
  SPECIAL(dump);
  SPECIAL(pet_shops);  
  /*SPECIAL(pray_for_items); */
  SPECIAL(kaan_altar);
  SPECIAL(bank);
  SPECIAL(hospital); /*end of rooms */
  SPECIAL(seal_exits);
  SPECIAL(teleporter);
  

struct spec_list {
  int number;
  char name[25];
  SPECIAL(*func);
  sbyte type; /* 0, 1, 2 for mob, obj, room */
};

struct spec_list spec_names[] = {
/* BEGIN HERE */
  {0, "postmaster", postmaster, 0},
  {1, "cityguard", cityguard, 0},
  {2, "receptionist", receptionist, 0},
  {3, "cryogenicist", cryogenicist, 0},
  {4, "guild_guard", guild_guard, 0},
  {5, "guild", guild, 0},
  {6, "puff", puff, 0},
  {7, "fido", fido, 0},
  {8, "janitor", janitor, 0},
  {9, "mayor", mayor, 0},
  {10, "snake", snake, 0},
  {11, "thief", thief, 0},
  {12, "magic_user", magic_user, 0},
  {13, "gym", gym, 0},
  {14, "library", library, 0},
  {15, "street_tough", street_tough, 0},
  {16, "blood_bank", blood_bank, 0},
  {17, "casino", casino, 0},
  {18, "lava_beast", lava_beast, 0},
  {19, "ankou_follower", ankou_follower, 0},
  {20, "flaming_efreet", flaming_efreet, 0},
  {21, "newbie_guard", newbie_guard, 0},
  {22, "newbie_helper", newbie_helper, 0},
  {23, "play_war", play_war, 0},
  {24, "simple_heal", simple_heal, 0},
  {25, "bank", bank, 1},
  {26, "gen_board", gen_board, 1},
  {27, "dump", dump, 2},
  {28, "pet_shops", pet_shops, 2},
  {29, "dump", dump, 2},        /*  {39, "pray_for_items", pray_for_items, 2}, */
  {30, "kaan_altar", kaan_altar, 2},
  {31, "bank", bank, 2},
  {32, "hospital", hospital, 2},
  {33, "teleporter", teleporter, 0},
/* STOP HERE */
  { -1, "Unknown; exists", NULL, -1} /* Terminator */
};


/* Get the name of a special proc. */
char *get_spec_name(SPECIAL(func))
{
  int i=0;

//  char buf[25];

  if (!func)
    return "None";

  for (i = 0; spec_names[i].func; i++){
//    sprintf(buf, "number is %d, spec_func is %s", i, spec_names[i].name);
//    log (buf);
    if (spec_names[i].func == func) return spec_names[i].name;
  }
  return spec_names[i].name;
}

/* Get a pointer to the function when you have a name */
proctype get_spec_proc(char *name, int type)
{
  int i=0;

  skip_spaces(&name);
  for (i = 0; spec_names[i].func; i++) 
    if (!strcmp(name, spec_names[i].name))
      if (spec_names[i].type == type)
        return spec_names[i].func;

  return spec_names[i].func;
}

/* Show all specprocs */
/* Don't ask me; I haven't got the foggiest idea why I put this in.
   Debugging maybe... :] */
void list_spec_procs(struct char_data *ch)
{
  int i=0;
  char buf[MAX_STRING_LENGTH];
  char temp[MAX_STRING_LENGTH];
  char types[3][6]; 

  strcpy(types[0], "mob");
  strcpy(types[1], "obj");
  strcpy(types[2], "room");

  for(; spec_names[i].func; i++) {
    sprintf(temp, "%s(%s)", spec_names[i].name, types[spec_names[i].type]);
    sprintf(buf, "%19s", temp);
    send_to_char(buf, ch);
    if (i%4==3)
      send_to_char("\r\n", ch);
  }
  send_to_char("\r\n", ch);
}
