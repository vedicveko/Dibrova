
#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "spells.h"
#include "interpreter.h"
#include "hometowns.h"

extern int DFLT_PORT;

/* The menu for choosing a hometown in interpreter.c: */
const char *hometowns_menu =
"\r\n"
/*
" /cWSelect a hometown:/c0\r\n\r\n"
" /crMidgaard      /cw- Center of most everything.  Very newbie friendly.\r\n"
" /crJareth        /cw- Close proximity to Midgaard. Good starter zone for\r\n"
"                 newbies, blood bank, casino.  Labeled the 'City of\r\n" 
"                 Freedom'.\r\n"
" /crMcGintey Cove /cw- Largest seaport in all Dibrova  Extensive bazaar in\r\n"
"                 town's center attracts travelers near and far.\r\n"
" /crSilverthorne  /cw- Small forest town, known for its noble citizens\r\n"
"                 and learned scholars of magic.\r\n"
" /crElven City    /cw- Tree City Capital of the Elven Nation.\r\n"
" /crMalathar      /cw- Center for religious studies, known for its many\r\n"
"                 academies and centers of art.\r\n\r\n"
" /cWYour choice?\r\n";
*/
" /cWAt this time the option of choosing hometowns has been removed.\r\n"
"\r\n"
" Midgaard will serve as the primary hometown until further notice.\r\n"
"\r\n\r\n Please type /crMidgaard/cW at the prompt.\r\n\r\n";


int parse_hometown(char *arg)
{
  if (isname(arg, "midgaard")) {
    return HOME_MIDGAARD;
  } else if (isname(arg, "jareth")) {
    return HOME_JARETH;
  } else if (isname(arg, "mcgintey cove")) {
    return HOME_MCGINTEY;
  } else if (isname(arg, "silverthorne")) {
    return HOME_SILVERTHORNE;
  } else if (isname(arg, "elven city")) {
    return HOME_ELVENCITY;
  } else if (isname(arg, "malathar")) {
    return HOME_MALATHAR;
  } else 
    return HOME_UNDEFINED;
}

const int load_rooms[] = {
  3001,
  1005,
  6775,
  5747,
  19008,
  8112,
  4476,
  30002
};

const int don_rooms[] = {
  3063,
  1174,
  6775,
  5770,
  19075,
  8000,
  4308,
  30003
};

const int clan_don_rooms[] = {
  -1, 
  30104, /*Genocide*/
  30204, /*Shadow Alliance*/
  30304,  /*::seVen::*/
  30404,  /*Pahalawan Knights*/
  30504,  /*Holy Evil Doers*/
  30604   /*Lost Souls*/
};

const sh_int primary_ht_zones[] = {
  30,
  10,
  67,
  57,
  190,
  81,
  44,
  300
};

const char *hometowns[] = {
  "Midgaard",
  "Jareth",
  "McGintey Cove",
  "Silverthorne",
  "Elven City",
  "Ofingia",
  "Malathar",
  "Genocide Stronghold"
};

int get_default_room(struct char_data *ch) {

  extern int port;

  if (port!=5150)
    return 3001;

  if (real_room(load_rooms[GET_HOME(ch)])==NOWHERE)
    return 3001; /* minimud, broken room, ...? */
  else
    return load_rooms[GET_HOME(ch)];

}

sh_int get_ht_zone(int room) {

  sh_int zn, i;

  zn = (sh_int)room/100;
  for (i = 0; i < NUM_STARTROOMS; i++)
    if (zn == primary_ht_zones[i])
      return i;
  return -1;
}

