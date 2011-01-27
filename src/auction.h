/* ************************************************************************
*   File: auction.h                                     Part of CircleMUD *
*   Author: Shawn Stoffer (Zaylon)                     Cities of Glory    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
extern struct char_data * auction_bidder;
extern int auction_current_bid;
extern int auction_now;
extern struct obj_data * auction_obj;

ACMD(do_auction);
void auction_check (struct char_data * ch, int mode);
void auction_activity();
int can_auction(struct char_data *ch);
int auction_remove(int);
int auction_insert(struct obj_data*, int, int);
int auction_valid_item(int auction_num);

#define MAX_AUCTION   10

#define AUCTION_LEAVE 0
#define AUCTION_GONE  1
#define AUCTION_DIE   2

/* alright, first idea was a linked list, but that proved
 * to be a problem, so lets make this a hash table instead.
 * the hash table approach allows us to save the memory
 * space of 8 bytes (int num, for auction number, and 
 *  struct auction_type *, to the next auctioned item.)
 */
struct auction_type {
  time_t time_left;
  int bid;
  struct obj_data * obj;
  struct char_data * bidder;
};
