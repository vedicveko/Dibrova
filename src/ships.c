/* ************************************************************************
*   File: ship.c                                                          *
*                                                                         *
*   Usage: ship booting code.                                             *
*                                                                         *
*   Added to Dibrova by Rapideye/Sezzno                                   *
*   Coded by Tom Youderian (Aule of Heroes of Kore)                       *
************************************************************************ */
 
 
#include "conf.h"
#include "sysdep.h"
 
 
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "house.h"
 
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct obj_data *object_list;

void load_ship(int number, int room)
{
  struct obj_data *obj;
  int r_num, r_room;

  if ((r_num = real_object(number)) < 0) {
    sprintf(buf, "SYSERR:    Ship Warning: There is no ship vnum %d.\r\n",
        number);
    log(buf);
    return;
  }
  if ((r_room = real_room(room)) < 0) {
    sprintf(buf, "SYSERR:    Ship Warning: There is no room vnum %d.\r\n",
        room);
    log(buf);
    return;
  }
  obj = read_object(r_num, REAL);
  obj_to_room(obj, r_room);
}
 
 
 
void Ship_boot(void)
{
 
  /* ship vnum, room vnum where it loads */
     load_ship(15098, 15016);
}

