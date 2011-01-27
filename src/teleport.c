/*
**    File: teleport.c
**      By: Cj Stremick
**     For: Flexible implementation of teleporter shops.
** History: -CMS- 10/5/98 Created
**          -CMS- 11/10/99 Ownership abondoned.  If this code is going to 
**          be subject to fucking shit-assed style, I will take no more 
**          credit or blame for what happens to it.
*/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "utils.h"
#include "teleport.h"

struct teleport_shop *teleport_shops=NULL; /* Global list of teleport shops. */
extern struct index_data *mob_index;
extern struct room_data *world;

char *tactions[] = {
 "Friendly",
 "Grouchy",
 "Distracted"
};

/* Load teleporter file */
void load_teleporters() {

  FILE *fp;      /* teleporter file. */
  char buf[256]; /* General purpose buffer */
  struct teleport_shop *temp_shop;
  struct teleport_dest *temp_dest;

  if (!(fp = fopen(FILE_TELEPORT, "r"))) {
    sprintf(buf, "SYSERR: opening teleporter file '%s'", FILE_TELEPORT);
    perror(buf);
    exit(1);
  }

  get_line(fp, buf);
  while (strcmp(buf, "$")) {

    CREATE(temp_shop, struct teleport_shop, 1);
    temp_shop->destinations=NULL;
    temp_shop->next=NULL;

    /* read a teleport operator. */
    if (sscanf(buf, "%d %d", &temp_shop->operator,
                              &temp_shop->action) != 2) {
      printf("SYSERR: Format error in teleport file (oper).\n");
      exit(1);
    }

    /* Get the list of destinations. */
    get_line(fp, buf);
    while (strcmp(buf, "~")) {
      CREATE(temp_dest, struct teleport_dest, 1);
      if (sscanf(buf, "%d %d", &temp_dest->dest,
                                &temp_dest->gold_cost) != 2){
        printf("SYSERR: Format error in teleport file (dest).\n");
        exit(1);
      }
      temp_dest->next = temp_shop->destinations;
      temp_shop->destinations = temp_dest;
      get_line(fp, buf); /* Get the next destination. */
    }
    temp_shop->next = teleport_shops;
    teleport_shops = temp_shop;
    get_line(fp, buf); /* Get the next teleport operator. */
  }

  fclose(fp);
  return;
}

SPECIAL(teleporter){

  char tmpbuf[2048];
  struct char_data *oper;
  struct teleport_shop *tshop;
  struct teleport_dest *tdest;
  int ct=0;

  oper = (struct char_data *)me;
  if (!(tshop = get_teleport_shop(oper)))
    return FALSE;

  if (CMD_IS("list")) {
    strcpy(tmpbuf, "Destinations:\r\n------------\r\n");
    for (tdest = tshop->destinations; tdest; tdest = tdest->next) {
      sprintf(tmpbuf, "%s#%d %s for %d gold.\r\n", tmpbuf, ++ct,
              world[real_room(tdest->dest)].name, tdest->gold_cost);
    }
    send_to_char(tmpbuf, ch);
    return TRUE;
  } else if (CMD_IS("buy")) {
    if ((tdest = get_teleport_dest(tshop, atoi(argument)))) {
      if (GET_GOLD(ch) >= tdest->gold_cost) {
        GET_GOLD(ch)-=tdest->gold_cost;
        char_from_room(ch);
        char_to_room(ch, real_room(tdest->dest));
        look_at_room(ch, FALSE);
      } else {
        send_to_char("No pay, no play.\r\n", ch);
      }
    } else {
      send_to_char("I don't know where that is.\r\n", ch);
    }
    return TRUE;
  }
  return FALSE;
}

struct teleport_shop *get_teleport_shop(struct char_data *oper) {

  struct teleport_shop *tshop;

  for (tshop = teleport_shops; tshop; tshop = tshop->next)
    if (IS_MOB(oper) && GET_MOB_VNUM(oper) == tshop->operator)
      return tshop;

  return NULL;
}

struct teleport_dest *get_teleport_dest(struct teleport_shop *tshop,
                                        int dest_num) {

  struct teleport_dest *dest;

  dest_num--;
  for (dest = tshop->destinations; dest; dest_num--, dest = dest->next)
    if (!dest_num)
      return dest;

  return NULL;
}

