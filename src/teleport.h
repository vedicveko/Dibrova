/*
**    File: teleport.h
**      By: Cj Stremick
**     For: Flexible implementation of teleporter shops.
** History: -CMS- 10/5/98 Created
**          -CMS- 11/10/99 Ownership abondoned.  If this code is going to 
**          be subject to fucking shit-assed style, I will take no more 
**          credit or blame for what happens to it.
*/

#define FILE_TELEPORT "misc/teleport"

/* How does the operator act? */
#define OPER_ACT_FRIENDLY   0
#define OPER_ACT_GROUCHY    1
#define OPER_ACT_DISTRACTED 2

/* teleport.c func prototypes */
void load_teleporters();
struct teleport_shop *get_teleport_shop(struct char_data *oper);
struct teleport_dest *get_teleport_dest(struct teleport_shop *tshop,
                                        int dest_num);

/* Info about destinations. */
struct teleport_dest {
  room_vnum            dest;               /* Destination room      */
  int                  gold_cost;          /* Cost for transporting */
  struct teleport_dest *next;              /* Next destination      */
};

/* Info about teleport shops. */
struct teleport_shop {
  mob_vnum             operator;           /* Teleport operator     */
  int                  action;             /* Operator action       */
  struct teleport_dest *destinations;      /* List of destinations  */
  struct teleport_shop *next;              /* The next shop         */
};


