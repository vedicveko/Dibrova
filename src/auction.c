/* ************************************************************************
*   File: auction.h                                     Part of CircleMUD *
*   Author: Shawn Stoffer (Zaylon)                     Cities of Glory    *
*                                                                         *
*  Description:  This code implements a pseudo hash table approach to an  *
*                auction system to auction multiple items.                *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#include "conf.h"
#include "sysdep.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "auction.h"

struct auction_type * auction_nums[MAX_AUCTION] = { 
  /*0*/ /*1*/ /*2*/ /*3*/ /*4*/ /*5*/ /*6*/ /*7*/ /*8*/ /*9*/
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL 
};
/* a side effect of using a hash table instead of a linked list.
 * If we would have used a linked list we could simply have used
 * the head pointer, if the list was empty (no auctions going on)
 * then we would simply not worry about auctions, but if the head
 * were some value, then an auction would have to be going on. If
 * we wanted to use the hash table the same way then we have to 
 * check each and every value of the hash table for that, very 
 * inefficient.
 */
byte current_auction = 0;

/*
ACMD(do_auction) {
  int i = 0, value = 0;
  char arg[SMALL_BUFSIZE];
  char buf[MAX_STRING_LENGTH];
  char * USAGE = "auction sell <itemname> <value> [time]\r\n"
                 "auction <bid> <number> <amount>\r\n"
                 "auction cancel\r\nauction end\r\nauction information\r\n";
  struct auction_type * temp_auc;

  if (IS_NPC(ch)) {
    send_to_char("Mobiles cannot participate in auctions.\r\n", ch);
    return;
  }

  skip_spaces(&argument);
  if (!*argument) {
    send_to_char(USAGE, ch);
    return;
  }

  argument = one_argument(argument, arg);
  skip_spaces(&argument);

  if (is_abbrev(arg, "sell")) {
    int value2 = 0;
    struct obj_data * temp_obj;

    if (!*argument) { 
      send_to_char(USAGE, ch); 
      return;
    }

    argument = one_argument(argument, arg);

    if (!(temp_obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
      send_to_char("You don't have that item.\r\n", ch);
      return;
    }


    if((GET_OBJ_TYPE(temp_obj) == ITEM_SCROLL) || (GET_OBJ_TYPE(temp_obj) == ITEM_CONTAINER)) {
      send_to_char("You are not allowed to auction that.\r\n", ch);
      return;
    }

    if (current_auction) {
      i = can_auction(ch);
      if (i > MAX_AUCTION) {  
        strcpy(buf, "You must wait until one of the auctions are over.\r\n");
        send_to_char(buf, ch);
        return;
      } else if (i == -1) {
        strcpy(buf, "You must wait until your auction is over.\r\n");
        send_to_char(buf, ch);
        return;
      }
    }


    argument = one_argument(argument, arg);

    if ((value = atoi(arg)) <= 0) 
      value = GET_OBJ_COST(temp_obj);
    
    if ((value2 = atoi(argument)) <= 0) value2 = 5;
    else if (value2 > 8) {
      send_to_char("Auctions can only last 8 hours maximum, your auction will now last 8 hours.\r\n", ch);
      value2 = 8;
    }

    if ((i = auction_insert(temp_obj, value, value2)) < 0) {
      send_to_char("/cmYour auction was not accepted!!  There are too many \r\n"
                   "/cmitems being auctioned right now.  Wait for one of \r\n"
                   "/cmthe auctions to end./c0\r\n", ch);
      return;
    }

    sprintf(buf, "You auction your %s for %d coins.\r\n", 
                 temp_obj->short_description, value);
    send_to_char(buf, ch);

    sprintf(buf, "/cm%s /cmauctions, \"/cm%d./cw %s/cm, for %d /cmcoins.\"/c0 \r\n", 
                 GET_NAME(ch), i+1, temp_obj->short_description, value);
    send_to_all(buf);

  } else  if (is_abbrev(arg, "bid")) {
    if (current_auction)  {

      argument = one_argument(argument, arg);

      if ((i = atoi(arg)) > 0) {
        if (!(i > 0 && i < MAX_AUCTION) || !(temp_auc = auction_nums[i-1])) {
          send_to_char("There is no item in that auction slot.\r\n", ch);
          return;
        }
      } else {  
        send_to_char("What did you want to bid on?\r\n",ch);
        return;
      }

      if (!(temp_auc->obj && temp_auc->obj->carried_by)) {
        auction_remove(i-1);
        send_to_char("You may no longer bid on that.\r\n", ch);
        return;
      }

    } else { 
     send_to_char("There is nothing to bid on.\r\n", ch);
     return;
    }

    argument = one_argument(argument, arg);
    value = atoi(arg);
    if (!temp_auc->bidder) {
      if (value < temp_auc->bid) {
        sprintf(buf, "The current bid is %d, you must at least that for your bid to be accepted.\r\n", temp_auc->bid);
        send_to_char(buf, ch);
        return;
      }
    } else {
      if (value < temp_auc->bid+500) {
        sprintf(buf, "The current bid is %d, you must bid at least 500 coins more for your bid to be accepted.\r\n", temp_auc->bid);
        send_to_char(buf, ch);
        return;
      }
    }
    if (value > GET_GOLD(ch)) {
      send_to_char("You do not have that much money.\r\n", ch);
      return;
    }
    
    temp_auc->bid = value;
    temp_auc->bidder = ch;

    sprintf(buf, "/cm%s /cmauctions, \"%d. %d /cmcoins for %s/cm's %s/cm\"/c0\r\n",
            GET_NAME(ch), i, temp_auc->bid, GET_NAME(temp_auc->obj->carried_by),
            temp_auc->obj->short_description);
    send_to_all(buf);
  } else if (is_abbrev(arg, "identify")) {
    value = atoi(argument);

    if (!current_auction) {
      send_to_char("There is no auction going on.\r\n", ch);
      return;
    } 

    if (!value) {
      send_to_char("Which of the items auctioned do you want to identify?\r\n",
                   ch);
      return;
    }

    if (value <= 0 || value > MAX_AUCTION) return; 
    if (!(temp_auc = auction_nums[value-1])) {
      send_to_char("That item is not being auctioned.\r\n", ch);
      return;
    }

    if (!temp_auc->obj) return;
    if (!temp_auc->obj->carried_by) return;

    if (GET_GOLD(ch) < 100) {
     send_to_char("/crThe auctionier tells you, \"I don't work for free...identification costs 100 gold coins.\"\r\n", ch); return;
    }

    GET_GOLD(ch) -= 100;

    spell_identify(50, ch, 0, temp_auc->obj, 0, 0);
  } else if (is_abbrev(arg, "cancel"))  {

    if (!current_auction) {
      send_to_char("There is no auction going on right now.\r\n",ch);
      return;
    }

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      if (!*argument) {
        send_to_char("Which auction do you want to cancel?\r\n", ch);
        return;
      }
      i = atoi(argument)-1;
      if (i < 0 || !auction_nums[i]) {
        send_to_char("There is no auction running with that number.\r\n", ch);
        return;
      }
    } else {
      for (i = 0; i < MAX_AUCTION; i++) 
        if (auction_nums[i] && auction_nums[i]->obj && auction_nums[i]->obj &&
            auction_nums[i]->obj->carried_by &&
            GET_IDNUM(auction_nums[i]->obj->carried_by) == GET_IDNUM(ch)) break;
      if (i >= MAX_AUCTION) {
        send_to_char("You do not have any items being auctioned.\r\n", ch);
        return;
      }
    }

    temp_auc = auction_nums[i];

    if (temp_auc && temp_auc->obj && temp_auc->obj->carried_by
        && (GET_IDNUM(ch) == GET_IDNUM(temp_auc->obj->carried_by) 
        || GET_LEVEL(ch) >= LVL_CODER)) {

     send_to_char("The auction has been cancelled.\r\n", 
                  temp_auc->obj->carried_by);
     if (temp_auc->bidder) 
       send_to_char("The auction has been cancelled.\r\n", temp_auc->bidder);

     auction_remove(i);

     sprintf(buf, "%s /cmauctions, \"%d. The auction has been cancelled.\"/c0\r\n", 
                  GET_NAME(ch), i+1);
     send_to_all(buf);
    }
  } else if (is_abbrev(arg, "end")) {

    if (!current_auction) {
      send_to_char("There is no auction going on right now.\r\n",ch);
      return;
    }

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
      if (!*argument)  {
        send_to_char("Which auction do you want to end?\r\n", ch);
        return;
      }
      i = atoi(argument)-1;
      if (i < 0 || !auction_nums[i]) {
        send_to_char("There is no auction running with that number.\r\n", ch);
        return;
      }
    } else {
      for (i = 0; i < MAX_AUCTION; i++) 
        if (auction_nums[i] && auction_nums[i]->obj && auction_nums[i]->obj &&
            auction_nums[i]->obj->carried_by &&
            GET_IDNUM(auction_nums[i]->obj->carried_by) == GET_IDNUM(ch)) break;
      if (i >= MAX_AUCTION) {
        send_to_char("You do not have any items being auctioned.\r\n", ch);
        return;
      }
    }

    temp_auc = auction_nums[i];
    if (temp_auc && temp_auc->obj && temp_auc->obj->carried_by &&
        (GET_IDNUM(ch) == GET_IDNUM(temp_auc->obj->carried_by))) {
     temp_auc->time_left = 1;
     auction_activity();
    }
  } else if (is_abbrev(arg, "information")) {
    for (i = 0; i < MAX_AUCTION; i++)
    {
      if (!(temp_auc = auction_nums[i])) continue;
      if (!(temp_auc->obj && temp_auc->obj->carried_by && 
            temp_auc->obj->carried_by->desc &&
            !temp_auc->obj->carried_by->desc->connected)) {
        auction_remove(i);
        continue;
      }

      if (temp_auc->bidder) {
        sprintf(buf,"/cmThe auctionier auctions, \"%d. With %d /cmhours left of"
                     " auction time, %s /cmhas the highest bid at %d /cmcoins, "
                     "for %s/cm.\"/c0\r\n",
                     i+1, (int)temp_auc->time_left, GET_NAME(temp_auc->bidder),
                     temp_auc->bid, temp_auc->obj->short_description);
        send_to_char(buf, ch);
      } else {
        sprintf(buf, "/cmThe auctionier auctions, \"%d. %s /cmremains on the block "
                     "for another %d /cmhours, awaiting a bid.\"/c0\r\n",
                     i+1,temp_auc->obj->short_description, (int)temp_auc->time_left);
        send_to_char(buf, ch);
      }
    }
  } else {
    send_to_char(USAGE, ch);
    return;
  }
}
*/

void auction_activity(void) {
  char buf[MAX_STRING_LENGTH];
  struct auction_type * temp_auc;
  int i;
//  strcpy(where_at, "auction_activity");

  /* This check makes sure that nothing is screwy with the auction...
   * it is done like this so that this can be taken advantage of:
   *  check if obj is carried_by anyone (i.e. make sure owner has not dropped 
   * item or put it away), then check if the owner still exists, lastly check
   * if the owner has disconnected, or any other awesome state of non-playing.
   * The order is important because it will only try to check the state
   * of the descriptor if there is a descriptor, further, it will only try to 
   * check the descriptor if there is a player there.  (I explained it in the 
   * reverse order of use, f.i.: ->carried_by means that there is a char there,
   *  ->desc makes sure there is a descriptor.  This is important, because if
   * any of the checks fail then it won't execute the statements later, 
   * so that should a descriptor not exist it will not crash the mud by
   * trying to check the non-existant descriptors state.
   */
  for (i = 0; i < MAX_AUCTION; i++) 
  {
    if (!(temp_auc = auction_nums[i])) continue;
    if (!temp_auc->obj) { auction_remove(i); continue; }
    if (temp_auc->obj->carried_by && temp_auc->obj->carried_by->desc && 
        !temp_auc->obj->carried_by->desc->connected) {
  
    if (--temp_auc->time_left) {
      if (temp_auc->bidder) {
        sprintf(buf,"/cmThe auctionier auctions, \"%d. With %d /cmhours left of"
                     " auction time, %s /cmhas the highest bid at %d /cmcoins, "
                     "for %s/cm.\"/c0\r\n", 
                     i+1, (int)temp_auc->time_left, GET_NAME(temp_auc->bidder), 
                     temp_auc->bid, temp_auc->obj->short_description);
        send_to_all(buf);
      } else {
        sprintf(buf, "/cmThe auctionier auctions, \"%d. %s /cmremains on the block "
                     "for another %d /cmhours, awaiting a bid.\"/c0\r\n",
                     i+1,temp_auc->obj->short_description, (int)temp_auc->time_left);
        send_to_all(buf);
      }
    } else {
      if (temp_auc->bidder) {
        sprintf(buf, "/cmThe auctionier auctions, \"%d. %s /cmhas been sold to %s for "
                     "%d coins.\"/c0\r\n", i+1, temp_auc->obj->short_description,
                     GET_NAME(temp_auc->bidder), temp_auc->bid);
        send_to_all(buf);
        if (temp_auc->bid > GET_GOLD(temp_auc->bidder)) {
          sprintf(buf, "/cmThe auctionier auctions, \"%d. %s /cmdid not have "
                       "enough money.  Auction will be extended for 3 hours.\"/c0\r\n", 
                  i+1,GET_NAME(temp_auc->bidder));
          send_to_all(buf);
          temp_auc->time_left+=3;
          return;
        }
        sprintf(buf, "The auctionier takes your %s and pays you %d coins, minus"
                     " the house cut of ten percent of course.\r\n", 
                     temp_auc->obj->short_description, temp_auc->bid);
        send_to_char(buf, temp_auc->obj->carried_by);
        GET_GOLD(temp_auc->bidder) -= temp_auc->bid;
        GET_GOLD(temp_auc->obj->carried_by) += temp_auc->bid - temp_auc->bid/10;
        obj_from_char(temp_auc->obj);
        obj_to_char(temp_auc->obj, temp_auc->bidder);   
      
        sprintf(buf, "The auctionier gives you %s in exchange for %d coins.\r\n",
                     temp_auc->obj->short_description, temp_auc->bid);
        send_to_char(buf, temp_auc->bidder);
        auction_remove(i);
      } else {
        sprintf(buf, "/cmThe auctionier auctions, \"%d. Auction cancelled due to lack of interest./c0\"\r\n", i+1);
        send_to_all(buf);
        auction_remove(i);
      }
    }
  } 
 }
}
  
void auction_check(struct char_data *ch, int mode) {
  int i;
  struct auction_type * temp_auc;
  char buf3[MAX_STRING_LENGTH];

// strcpy(where_at, "auction_check");

  if (!current_auction) return;

 for (i = 0; i < MAX_AUCTION; i++) {
  if (!(temp_auc = auction_nums[i])) continue;
  if (ch == temp_auc->bidder) {
    if (mode == AUCTION_DIE)
      sprintf(buf3, "/cmThe auctionier auctions, \"%d. %s unfortunately is no longer with us, R.I.P.  Bid remains at %d coins.\"/c0\r\n", i+1, GET_NAME(ch), temp_auc->bid);
    else 
      sprintf(buf3, "/cmThe auctionier auctions, \"%d. %s has backed out of the auction.  Bid remains at %d coins though.\"/c0\r\n", i+1, GET_NAME(ch), temp_auc->bid);
    send_to_all(buf3);
    temp_auc->bidder = 0;
    return;
  }

  if (temp_auc->obj->carried_by && ch == temp_auc->obj->carried_by) {
    if (mode == AUCTION_DIE)
      sprintf(buf3, "/cmThe auctionier auctions, \"%d. %s unfortunately is no longer with us, R.I.P.  The auction is cancelled for obvious reasons.\"/c0\r\n", i+1, GET_NAME(ch));
    else 
      sprintf(buf3, "/cmThe auctionier auctions, \"%d. %s has left the game, auction cancelled.\"/c0\r\n", i+1, GET_NAME(temp_auc->obj->carried_by));
    send_to_all(buf3);
    auction_remove(i);
    return;
  }
 }
}

int auction_remove(int value) {
  struct auction_type * t;
  t = auction_nums[value];
  auction_nums[value] = NULL;
  free(t);
  current_auction--;
  return 1;
}

/* Insert the auction at the first open slot.  If there is an error, 
 * return -1, otherwise return the location of the inserted element */
int auction_insert(struct obj_data * obj, int bid, int time_left) {
  int i = 0;
  struct auction_type * temp_auc;
  /* This for loop finds the first place where there is no auction 
   * entry, in other words the first place that is !auction_nums[i]
   */
  for (i = 0; i < MAX_AUCTION && auction_nums[i];i++);

  if (i >= MAX_AUCTION) return -1;

  CREATE(temp_auc, struct auction_type, 1);
  temp_auc->bidder = 0;
  temp_auc->bid = bid;
  temp_auc->obj = obj;
  temp_auc->time_left = time_left;
  auction_nums[i] = temp_auc;
  current_auction++;
  return i;
} 

int can_auction(struct char_data * ch) {
  int i = 0;
  for (i = 0; i < MAX_AUCTION && auction_nums[i];i++) 
    if (!auction_nums[i] || ( auction_nums[i]->obj->carried_by && 
        GET_IDNUM(auction_nums[i]->obj->carried_by) == GET_IDNUM(ch)))
      break;
  if (i >= MAX_AUCTION) return MAX_AUCTION+1;
  if (auction_nums[i]) return -1;
  return i;
}
