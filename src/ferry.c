/**************************************************************************
 * File: ferry.c                                                          *
 * Usage: This is code for ferries.                                       *
 * Created: 7/27/99                                                       *
 **************************************************************************/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

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

/* external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[];

extern struct room_data *world;
int transpath = 0;
char curstat1[100], curstat2[100], curstat3[100];

void enter_stat(int station, char wherefrom[100], int which);
void leave_stat(int station, char whereto[100], int which);

void train_upd(void)
{
  int wdoor, edoor, station[5];
  station[1] = real_room(31300); // Wyndham 
  station[0] = real_room(5349); // McGintey 
  station[2] = real_room(19132); // Vernige
  station[3] = real_room(19656); //BR
 
  wdoor = 3; 
  edoor = 1; 

  switch (transpath)
    {
    case 0:
// World
      leave_stat(station[0], "Wyndham", 1);
   
      break;
    case 1:
	enter_stat(station[1], "Wyndham", 1);
      break;
    case 2:
        leave_stat(station[1], "Vernige", 1);

      break;
    case 3:
        enter_stat(station[2], "Vernige", 1);

      break;
    case 4:
      leave_stat(station[2], "Black River Station", 1);

      break;
    case 5:
      enter_stat(station[3], "Black River Station", 1);

      break;
    case 6:
      leave_stat(station[3], "Wyndham", 1);
      break;

    case 7:
      enter_stat(station[1], "Wyndham", 1);
      break;

    case 8:
      leave_stat(station[1], "McGintey Cove", 1); 

      break;
    case 9:
      enter_stat(station[0], "McGintey Cove", 1);

      break;
    default:
      log("SYSERR: Big problem with the ferry!");
      break;
    }
  transpath++;
  if (transpath == 10) transpath=0;
  return;
} 

void enter_stat(int station, char whereat[100],
                int which)
{
  int transroom = 0, wdoor, edoor;
  char buf[MAX_STRING_LENGTH];
  transroom = real_room(31371); 

  wdoor = 3;
  edoor = 1; 

  world[transroom].dir_option[wdoor]->to_room = station;
  world[station].dir_option[edoor]->to_room = transroom;
  send_to_room("The ferry pulls into the docks, and the doors to your west open..\n\r", transroom);
  sprintf(buf, "The first mate announces, 'Welcome to %s'\n\r", whereat);
  send_to_room(buf, transroom);
  sprintf(buf, "With a slight bump, the ferry pulls into\n\r");
  send_to_room(buf, station);
  send_to_room("the station, and the doors to your east open.\n\r", station);
  return;
}

void leave_stat(int station, char whereto[100], int which)
{
  int transroom = 0, wdoor, edoor;
  char buf[MAX_STRING_LENGTH];
  transroom = real_room(31371);
  wdoor = 3; 
  edoor = 1; 

  world[transroom].dir_option[wdoor]->to_room = NOWHERE;
  world[station].dir_option[edoor]->to_room = NOWHERE;
  sprintf(buf, "The first mate announces, 'Next stop: %s'\n\r", whereto);
  send_to_room(buf, transroom);
  send_to_room("The ferry pulls away from the docks, and starts it's journey\n\r", transroom);
  send_to_room("The ferry pulls away from the docks, and it sails away.\n\r", station);
  return;
}



