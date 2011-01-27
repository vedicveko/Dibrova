/* ************************************************************************
*   File: shop.c                                        Part of CircleMUD *
*  Usage: shopkeepers: loading config files, spec procs.                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
 

/***
 * The entire shop rewrite for Circle 3.0 was done by Jeff Fink.  Thanks Jeff!
 ***/

#define __SHOP_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "shop.h"
#include "clan.h"
#include "spells.h"

/* External variables */
extern struct str_app_type str_app[];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *mob_proto;
extern struct obj_data *obj_proto;
extern struct room_data *world;
extern struct time_info_data time_info;
extern char *item_types[];
extern char *extra_bits[];
extern struct zone_data *zone_table;
extern struct char_data *is_playing(char *vict_name);
extern int save_char_file_u(struct char_file_u st);

/* Forward/External function declarations */
ACMD(do_tell);
ACMD(do_action);
ACMD(do_echo);
ACMD(do_say);
char *fname(char *namelist);
void sort_keeper_objs(struct char_data * keeper, int shop_nr);
extern int save_char_file_u(struct char_file_u st);
extern struct descriptor_data *descriptor_list;
bool in_allied_lands(struct char_data *ch);
bool in_war_zone(struct char_data *ch);
bool in_my_clan_lands(struct char_data *ch);
void vendor_objects(struct char_data *ch, int obj_room, int type, char *argument, int j);
int PRICE(struct obj_data *obj);

/* Local variables */
struct shop_data *shop_index;
int top_shop = 0;
int cmd_say, cmd_tell, cmd_emote, cmd_slap, cmd_puke;

/* config arrays */
const char *operator_str[] = {
        "[({",
        "])}",
        "|+",
        "&*",
        "^'"
} ;

/* Constant list for printing out who we sell to */
const char *trade_letters[] = {
        "Good",                 /* First, the alignment based ones */
        "Evil",
        "Neutral",
        "Sorcerer",           /* Then the class based ones */
        "Cleric",
        "Thief",
        "Gladiator",
        "Druid",
        "Death Knight",
        "Monk",
        "Paladin",
        "Dwarf",              /* Then the race based ones */
        "Eldar",
        "Elf",
        "Giant",
        "Halfling",
        "Human",
        "Kender",
        "Ogre",
        "Triton",
        "Valkyrie",
        "Wolfen",
        "\n"
} ;


char *shop_bits[] = {
        "WILL_FIGHT",
        "USES_BANK",
        "\n"
} ;

int is_ok_char(struct char_data * keeper, struct char_data * ch, int shop_nr)
{
  char buf[200];

  if (!(CAN_SEE(keeper, ch))) {
    do_say(keeper, MSG_NO_SEE_CHAR, cmd_say, 0);
    return (FALSE);
  }
  if (IS_GOD(ch))
    return (TRUE);

  if ((IS_GOOD(ch) && NOTRADE_GOOD(shop_nr)) ||
      (IS_EVIL(ch) && NOTRADE_EVIL(shop_nr)) ||
      (IS_NEUTRAL(ch) && NOTRADE_NEUTRAL(shop_nr))) {
    sprintf(buf, "%s %s", GET_NAME(ch), MSG_NO_SELL_ALIGN);
    do_tell(keeper, buf, cmd_tell, 0);
    return (FALSE);
  }
  if (IS_NPC(ch))
    return (TRUE);

  if ((IS_SORCERER(ch) && NOTRADE_SORCERER(shop_nr)) ||
      (IS_CLERIC(ch) && NOTRADE_CLERIC(shop_nr)) ||
      (IS_THIEF(ch) && NOTRADE_THIEF(shop_nr)) ||
      (IS_GLADIATOR(ch) && NOTRADE_GLADIATOR(shop_nr)) ||
      (IS_DARK_KNIGHT(ch) && NOTRADE_DARK_KNIGHT(shop_nr)) ||
      (IS_MONK(ch) && NOTRADE_MONK(shop_nr)) ||
      (IS_PALADIN(ch) && NOTRADE_PALADIN(shop_nr)) ||
      (IS_BARD(ch) && NOTRADE_BARD(shop_nr))) {
    sprintf(buf, "%s %s", GET_NAME(ch), MSG_NO_SELL_CLASS);
    do_tell(keeper, buf, cmd_tell, 0);
    return (FALSE);
  }

  if ((IS_DWARF(ch) && NOTRADE_DWARF(shop_nr)) ||
      (IS_ELDAR(ch) && NOTRADE_ELDAR(shop_nr)) ||
      (IS_ELF(ch) && NOTRADE_ELF(shop_nr)) ||
      (IS_GIANT(ch) && NOTRADE_GIANT(shop_nr)) ||
      (IS_HALFLING(ch) && NOTRADE_HALFLING(shop_nr)) ||
      (IS_HUMAN(ch) && NOTRADE_HUMAN(shop_nr)) ||
      (IS_KENDER(ch) && NOTRADE_KENDER(shop_nr)) ||
      (IS_OGRE(ch) && NOTRADE_OGRE(shop_nr)) ||
      (IS_TRITON(ch) && NOTRADE_TRITON(shop_nr)) ||
      (IS_VALKYRIE(ch) && NOTRADE_VALKYRIE(shop_nr)) ||
      (IS_WOLFEN(ch) && NOTRADE_WOLFEN(shop_nr))) {
    sprintf(buf, "%s %s", GET_NAME(ch), MSG_NO_SELL_RACE);
    do_tell(keeper, buf, cmd_tell, 0);
    return (FALSE);
  }
 
  return (TRUE);
}


int is_open(struct char_data * keeper, int shop_nr, int msg)
{
  char buf[200];

  *buf = 0;
  if (SHOP_OPEN1(shop_nr) > time_info.hours)
    strcpy(buf, MSG_NOT_OPEN_YET);
  else if (SHOP_CLOSE1(shop_nr) < time_info.hours) {
    if (SHOP_OPEN2(shop_nr) > time_info.hours)
      strcpy(buf, MSG_NOT_REOPEN_YET);
    else if (SHOP_CLOSE2(shop_nr) < time_info.hours)
      strcpy(buf, MSG_CLOSED_FOR_DAY);
  }

  if (!(*buf))
    return (TRUE);
  if (msg)
    do_say(keeper, buf, cmd_tell, 0);
  return (FALSE);
}


int is_ok(struct char_data * keeper, struct char_data * ch, int shop_nr)
{
  if (is_open(keeper, shop_nr, TRUE))
    return (is_ok_char(keeper, ch, shop_nr));
  else
    return (FALSE);
}


void push(struct stack_data * stack, int pushval)
{
  S_DATA(stack, S_LEN(stack)++) = pushval;
}


int top(struct stack_data * stack)
{
  if (S_LEN(stack) > 0)
    return (S_DATA(stack, S_LEN(stack) - 1));
  else
    return (NOTHING);
}


int pop(struct stack_data * stack)
{
  if (S_LEN(stack) > 0)
    return (S_DATA(stack, --S_LEN(stack)));
  else {
    log("Illegal expression in shop keyword list");
    return (0);
  }
}


void evaluate_operation(struct stack_data * ops, struct stack_data * vals)
{
  int oper;

  if ((oper = pop(ops)) == OPER_NOT)
    push(vals, !pop(vals));
  else if (oper == OPER_AND)
    push(vals, pop(vals) && pop(vals));
  else if (oper == OPER_OR)
    push(vals, pop(vals) || pop(vals));
}


int find_oper_num(char token)
{
  int index;

  for (index = 0; index <= MAX_OPER; index++)
    if (strchr(operator_str[index], token))
      return (index);
  return (NOTHING);
}


int evaluate_expression(struct obj_data * obj, char *expr)
{
  struct stack_data ops, vals;
  char *ptr, *end, name[200];
  int temp, index;

  if (!expr)
    return TRUE;

  if (!isalpha(*expr))
	return TRUE;

  ops.len = vals.len = 0;
  ptr = expr;
  while (*ptr) {
    if (isspace(*ptr))
      ptr++;
    else {
      if ((temp = find_oper_num(*ptr)) == NOTHING) {
	end = ptr;
	while (*ptr && !isspace(*ptr) && (find_oper_num(*ptr) == NOTHING))
	  ptr++;
	strncpy(name, end, ptr - end);
	name[ptr - end] = 0;
	for (index = 0; *extra_bits[index] != '\n'; index++)
	  if (!str_cmp(name, extra_bits[index])) {
	    push(&vals, IS_SET_AR(GET_OBJ_EXTRA(obj), index));
	    break;
	  }
	if (*extra_bits[index] == '\n')
	  push(&vals, isname(name, obj->name));
      } else {
	if (temp != OPER_OPEN_PAREN)
	  while (top(&ops) > temp)
	    evaluate_operation(&ops, &vals);

	if (temp == OPER_CLOSE_PAREN) {
	  if ((temp = pop(&ops)) != OPER_OPEN_PAREN) {
	    log("Illegal parenthesis in shop keyword expression");
	    return (FALSE);
	  }
	} else
	  push(&ops, temp);
	ptr++;
      }
    }
  }
  while (top(&ops) != NOTHING)
    evaluate_operation(&ops, &vals);
  temp = pop(&vals);
  if (top(&vals) != NOTHING) {
    log("Extra operands left on shop keyword expression stack");
    return (FALSE);
  }
  return (temp);
}

/* Shopkeeper will tell a player about an item */
void inform(char *argument, struct char_data *ch, struct char_data *keeper,
            int shop_nr) {

  struct obj_data *obj;

  if (!(obj = get_obj_in_list_vis(ch, argument, keeper->carrying))) {

  }

}

int trade_with(struct obj_data * item, int shop_nr)
{
  int counter;

  if (GET_OBJ_COST(item) < 1)
    return (OBJECT_NOVAL);

  if (IS_OBJ_STAT(item, ITEM_NOSELL))
    return (OBJECT_NOTOK);

  for (counter = 0; SHOP_BUYTYPE(shop_nr, counter) != NOTHING; counter++)
    if (SHOP_BUYTYPE(shop_nr, counter) == GET_OBJ_TYPE(item)) {
      if ((GET_OBJ_VAL(item, 2) == 0) &&
	  ((GET_OBJ_TYPE(item) == ITEM_WAND) ||
	   (GET_OBJ_TYPE(item) == ITEM_STAFF)))
	return (OBJECT_DEAD);
      else if (evaluate_expression(item, SHOP_BUYWORD(shop_nr, counter)))
	return (OBJECT_OK);
    }

  return (OBJECT_NOTOK);
}


int same_obj(struct obj_data * obj1, struct obj_data * obj2)
{
  int index;

  if (!obj1 || !obj2)
    return (obj1 == obj2);

  if (GET_OBJ_RNUM(obj1) != GET_OBJ_RNUM(obj2))
    return (FALSE);

  if (GET_OBJ_COST(obj1) != GET_OBJ_COST(obj2))
    return (FALSE);

  if (GET_OBJ_EXTRA_AR(obj1, 0) != GET_OBJ_EXTRA_AR(obj2, 0) ||
      GET_OBJ_EXTRA_AR(obj1, 1) != GET_OBJ_EXTRA_AR(obj2, 1) ||
      GET_OBJ_EXTRA_AR(obj1, 2) != GET_OBJ_EXTRA_AR(obj2, 2) ||
      GET_OBJ_EXTRA_AR(obj1, 3) != GET_OBJ_EXTRA_AR(obj2, 3))
    return (FALSE);

  for (index = 0; index < MAX_OBJ_AFFECT; index++)
    if ((obj1->affected[index].location != obj2->affected[index].location) ||
	(obj1->affected[index].modifier != obj2->affected[index].modifier))
      return (FALSE);

  return (TRUE);
}


int shop_producing(struct obj_data * item, int shop_nr)
{
  int counter;

  if (GET_OBJ_RNUM(item) < 0)
    return (FALSE);

  for (counter = 0; SHOP_PRODUCT(shop_nr, counter) != NOTHING; counter++)
    if (same_obj(item, &obj_proto[SHOP_PRODUCT(shop_nr, counter)]))
      return (TRUE);
  return (FALSE);
}


int transaction_amt(char *arg)
{
  int num;

  one_argument(arg, buf);
  if (*buf)
    if ((is_number(buf))) {
      num = atoi(buf);
      strcpy(arg, arg + strlen(buf) + 1);
      return (num);
    }
  return (1);
}


char *times_message(struct obj_data * obj, char *name, int num)
{
  static char buf[256];
  char *ptr;

  if (obj)
    strcpy(buf, obj->short_description);
  else {
    if ((ptr = strchr(name, '.')) == NULL)
      ptr = name;
    else
      ptr++;
    sprintf(buf, "%s %s", AN(ptr), ptr);
  }

  if (num > 1)
    sprintf(END_OF(buf), " (x %d)", num);
  return (buf);
}


struct obj_data *get_slide_obj_vis(struct char_data * ch, char *name,
				            struct obj_data * list)
{
  struct obj_data *i, *last_match = 0;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return (0);

  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (CAN_SEE_OBJ(ch, i) && !same_obj(last_match, i)) {
	if (j == number)
	  return (i);
	last_match = i;
	j++;
      }
  return (0);
}


struct obj_data *get_hash_obj_vis(struct char_data * ch, char *name,
				           struct obj_data * list)
{
  struct obj_data *loop, *last_obj = 0;
  int index;

  if ((is_number(name + 1)))
    index = atoi(name + 1);
  else
    return (0);

  for (loop = list; loop; loop = loop->next_content)
    if (CAN_SEE_OBJ(ch, loop) && (loop->obj_flags.cost > 0))
      if (!same_obj(last_obj, loop)) {
	if (--index == 0)
	  return (loop);
	last_obj = loop;
      }
  return (0);
}


struct obj_data *get_purchase_obj(struct char_data * ch, char *arg,
		            struct char_data * keeper, int shop_nr, int msg)
{
  char buf[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
  struct obj_data *obj;

  one_argument(arg, name);
  do {
    if (*name == '#')
      obj = get_hash_obj_vis(ch, name, keeper->carrying);
    else
      obj = get_slide_obj_vis(ch, name, keeper->carrying);
    if (!obj) {
      if (msg) {
	sprintf(buf, shop_index[shop_nr].no_such_item1, GET_NAME(ch));
	do_tell(keeper, buf, cmd_tell, 0);
      }
      return (0);
    }
    if (GET_OBJ_COST(obj) <= 0) {
      extract_obj(obj);
      obj = 0;
    }
  } while (!obj);
  return (obj);
}


int buy_price(struct obj_data * obj, int shop_nr)
{
  return ((int) (GET_OBJ_COST(obj) * SHOP_BUYPROFIT(shop_nr)));
}


void shopping_buy(char *arg, struct char_data * ch,
		       struct char_data * keeper, int shop_nr)
{
  char tempstr[200], buf[MAX_STRING_LENGTH], cbuf[MAX_STRING_LENGTH];
  struct obj_data *obj, *last_obj = NULL;
  int goldamt = 0, buynum, bought = 0, clan_num = 0, clanamt = 0, percent, learned, amount = 0;
      clan_num = zone_table[world[keeper->in_room].zone].owner; 

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
    sort_keeper_objs(keeper, shop_nr);

  if ((buynum = transaction_amt(arg)) < 0) {
    sprintf(buf, "%s A negative amount?  Try selling me something.",
	    GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  if (!(*arg) || !(buynum)) {
    sprintf(buf, "%s What do you want to buy??", GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  if (!(obj = get_purchase_obj(ch, arg, keeper, shop_nr, TRUE)))
    return;

  if ((buy_price(obj, shop_nr) > GET_GOLD(ch)) && !IS_GOD(ch)) {
    sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);

    switch (SHOP_BROKE_TEMPER(shop_nr)) {
    case 0:
      do_action(keeper, GET_NAME(ch), cmd_puke, 0);
      return;
    case 1:
      do_echo(keeper, "smokes on his joint.", cmd_emote, SCMD_EMOTE);
      return;
    default:
      return;
    }
  }
  if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
    sprintf(buf, "%s: You can't carry any more items.\n\r",
	    fname(obj->name));
    send_to_char(buf, ch);
    return;
  }
  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    sprintf(buf, "%s: You can't carry that much weight.\n\r",
	    fname(obj->name));
    send_to_char(buf, ch);
    return;
  }
  while ((obj) && ((GET_GOLD(ch) >= buy_price(obj, shop_nr)) || IS_GOD(ch))
	 && (IS_CARRYING_N(ch) < CAN_CARRY_N(ch)) && (bought < buynum)
	 && (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) <= CAN_CARRY_W(ch))) {
    bought++;
    /* Test if producing shop ! */
    if (shop_producing(obj, shop_nr))
      obj = read_object(GET_OBJ_RNUM(obj), REAL);
    else {
      obj_from_char(obj);
      SHOP_SORT(shop_nr)--;
    }
    obj_to_char(obj, ch);

    percent = number(1, 30);
    learned = number(1, 130);

    goldamt += buy_price(obj, shop_nr);
    if (!IS_GOD(ch)) {
      if (GET_SKILL(ch, SKILL_HAGGLE) >= 1) {
        if (GET_SKILL(ch, SKILL_HAGGLE) > learned) {
          sprintf(buf, "You haggle for an additional %d percent off!\r\n",
                        percent);
          send_to_char(buf, ch);
          GET_GOLD(ch) -= (buy_price(obj, shop_nr) * ((percent) * .01));
          amount = (buy_price(obj, shop_nr) * ((percent) * .01));
        } 
      }   
      GET_GOLD(ch) -= buy_price(obj, shop_nr);

      amount = buy_price(obj, shop_nr);


    GET_GOLD(keeper) += amount;
    clan_num = zone_table[world[keeper->in_room].zone].owner;

    if(clan_num) {
     // Put the gold in the clan bank (Vedic)
     clanamt = amount * .3;
     clan[find_clan_by_id(clan_num)].treasure += clanamt;
   }

  
 }

    last_obj = obj;
    obj = get_purchase_obj(ch, arg, keeper, shop_nr, FALSE);
    if (!same_obj(obj, last_obj))
      break;
  }

  if (bought < buynum) {
    if (!obj || !same_obj(last_obj, obj))
      sprintf(buf, "%s I only have %d to sell you.", GET_NAME(ch), bought);
    else if (GET_GOLD(ch) < buy_price(obj, shop_nr))
      sprintf(buf, "%s You can only afford %d.", GET_NAME(ch), bought);
    else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      sprintf(buf, "%s You can only hold %d.", GET_NAME(ch), bought);
    else if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) > CAN_CARRY_W(ch))
      sprintf(buf, "%s You can only carry %d.", GET_NAME(ch), bought);
    else
      sprintf(buf, "%s Something screwy only gave you %d.", GET_NAME(ch),
	      bought);
    do_tell(keeper, buf, cmd_tell, 0);
  }

  sprintf(tempstr, times_message(ch->carrying, 0, bought));
  sprintf(buf, "$n buys %s.", tempstr);
  act(buf, FALSE, ch, obj, 0, TO_ROOM);

  // sprintf(buf, shop_index[shop_nr].message_buy, GET_NAME(ch), amount);
  // do_tell(keeper, buf, cmd_tell, 0);
  sprintf(buf, "You now have %s.\n\r", tempstr);
  send_to_char(buf, ch);

  if(clan_num > 0) {
    sprintf(cbuf, "%s %s thanks you for your purchase.", 
            GET_NAME(ch), clan[find_clan_by_id(clan_num)].name);
    do_tell(keeper, cbuf, cmd_tell, 0);
    save_clans();
  }

  if (SHOP_USES_BANK(shop_nr))
    if (GET_GOLD(keeper) > MAX_OUTSIDE_BANK) {
      SHOP_BANK(shop_nr) += (GET_GOLD(keeper) - MAX_OUTSIDE_BANK);
      GET_GOLD(keeper) = MAX_OUTSIDE_BANK;
    }
}


struct obj_data *get_selling_obj(struct char_data * ch, char *name,
		            struct char_data * keeper, int shop_nr, int msg)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int result;

  if (!(obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
    if (msg) {
      sprintf(buf, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
      do_tell(keeper, buf, cmd_tell, 0);
    }
    return (0);
  }
  if ((result = trade_with(obj, shop_nr)) == OBJECT_OK)
    return (obj);

  switch (result) {
  case OBJECT_NOVAL:
    sprintf(buf, "%s You've got to be kidding, that thing is worthless!",
	    GET_NAME(ch));
    break;
  case OBJECT_NOTOK:
    sprintf(buf, shop_index[shop_nr].do_not_buy, GET_NAME(ch));
    break;
  case OBJECT_DEAD:
    sprintf(buf, "%s %s", GET_NAME(ch), MSG_NO_USED_WANDSTAFF);
    break;
  default:
    sprintf(buf, "SYSERR: Illegal return value of %d from trade_with() (shop.c)",
	    result);
    log(buf);
    sprintf(buf, "%s An error has occurred.", GET_NAME(ch));
    break;
  }
  if (msg)
    do_tell(keeper, buf, cmd_tell, 0);
  return (0);
}


int sell_price(struct char_data * ch, struct obj_data * obj, int shop_nr)
{
  return ((int) (GET_OBJ_COST(obj) * SHOP_SELLPROFIT(shop_nr)));
}


struct obj_data *slide_obj(struct obj_data * obj, struct char_data * keeper,
			            int shop_nr)
/*
   This function is a slight hack!  To make sure that duplicate items are
   only listed once on the "list", this function groups "identical"
   objects together on the shopkeeper's inventory list.  The hack involves
   knowing how the list is put together, and manipulating the order of
   the objects on the list.  (But since most of DIKU is not encapsulated,
   and information hiding is almost never used, it isn't that big a deal) -JF
*/
{
  struct obj_data *loop;
  int temp;

  if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
    sort_keeper_objs(keeper, shop_nr);

  /* Extract the object if it is identical to one produced */
  if (shop_producing(obj, shop_nr)) {
    temp = GET_OBJ_RNUM(obj);
    extract_obj(obj);
    return (&obj_proto[temp]);
  }
  SHOP_SORT(shop_nr)++;
  loop = keeper->carrying;
  obj_to_char(obj, keeper);
  keeper->carrying = loop;
  while (loop) {
    if (same_obj(obj, loop)) {
      obj->next_content = loop->next_content;
      loop->next_content = obj;
      return (obj);
    }
    loop = loop->next_content;
  }
  keeper->carrying = obj;
  return (obj);
}


void sort_keeper_objs(struct char_data * keeper, int shop_nr)
{
  struct obj_data *list = 0, *temp;

  while (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper)) {
    temp = keeper->carrying;
    obj_from_char(temp);
    temp->next_content = list;
    list = temp;
  }

  while (list) {
    temp = list;
    list = list->next_content;
    if ((shop_producing(temp, shop_nr)) &&
	!(get_obj_in_list_num(GET_OBJ_RNUM(temp), keeper->carrying))) {
      obj_to_char(temp, keeper);
      SHOP_SORT(shop_nr)++;
    } else
      (void) slide_obj(temp, keeper, shop_nr);
  }
}


void shopping_sell(char *arg, struct char_data * ch,
		        struct char_data * keeper, int shop_nr)
{
  char tempstr[200], buf[MAX_STRING_LENGTH], name[200];
  struct obj_data *obj, *tag = 0;
  int sellnum, sold = 0, goldamt = 0;

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  if ((sellnum = transaction_amt(arg)) < 0) {
    sprintf(buf, "%s A negative amount?  Try buying something.",
	    GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  if (!(*arg) || !(sellnum)) {
    sprintf(buf, "%s What do you want to sell??", GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  one_argument(arg, name);
  if (!(obj = get_selling_obj(ch, name, keeper, shop_nr, TRUE)))
    return;

  if (GET_GOLD(keeper) + SHOP_BANK(shop_nr) < sell_price(ch, obj, shop_nr)) {
    sprintf(buf, shop_index[shop_nr].missing_cash1, GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  while ((obj) && (GET_GOLD(keeper) + SHOP_BANK(shop_nr) >=
		   sell_price(ch, obj, shop_nr)) && (sold < sellnum)) {
    sold++;

    goldamt += sell_price(ch, obj, shop_nr);
    GET_GOLD(keeper) -= sell_price(ch, obj, shop_nr);
	
    obj_from_char(obj);
    tag = slide_obj(obj, keeper, shop_nr);
    obj = get_selling_obj(ch, name, keeper, shop_nr, FALSE);
  }

  if (sold < sellnum) {
    if (!obj)
      sprintf(buf, "%s You only have %d of those.", GET_NAME(ch), sold);
    else if (GET_GOLD(keeper) + SHOP_BANK(shop_nr) <
	     sell_price(ch, obj, shop_nr))
      sprintf(buf, "%s I can only afford to buy %d of those.",
	      GET_NAME(ch), sold);
    else
      sprintf(buf, "%s Something really screwy made me buy %d.",
	      GET_NAME(ch), sold);

    do_tell(keeper, buf, cmd_tell, 0);
  }
  GET_GOLD(ch) += goldamt;
  strcpy(tempstr, times_message(0, name, sold));
  sprintf(buf, "$n sells %s.", tempstr);
  act(buf, FALSE, ch, obj, 0, TO_ROOM);

  sprintf(buf, shop_index[shop_nr].message_sell, GET_NAME(ch), goldamt);
  do_tell(keeper, buf, cmd_tell, 0);
  sprintf(buf, "The shopkeeper now has %s.\n\r", tempstr);
  send_to_char(buf, ch);

  if (GET_GOLD(keeper) < MIN_OUTSIDE_BANK) {
    goldamt = MIN(MAX_OUTSIDE_BANK - GET_GOLD(keeper), SHOP_BANK(shop_nr));
    SHOP_BANK(shop_nr) -= goldamt;
    GET_GOLD(keeper) += goldamt;
  }
}


void shopping_value(char *arg, struct char_data * ch,
		         struct char_data * keeper, int shop_nr)
{
  char buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  char name[MAX_INPUT_LENGTH];

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  if (!(*arg)) {
    sprintf(buf, "%s What do you want me to valuate??", GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  one_argument(arg, name);
  if (!(obj = get_selling_obj(ch, name, keeper, shop_nr, TRUE)))
    return;

  sprintf(buf, "%s I'll give you %d gold coins for that!", GET_NAME(ch),
	  sell_price(ch, obj, shop_nr));
  do_tell(keeper, buf, cmd_tell, 0);

  return;
}


/* Will add spaces to a string to compensate for color codes -Rapax*/
void append_space_to_str(char *str, char *modi)
{

  char arg;
  int i, k;
  int to_add = 0;
  
  for(i = 2; i < strlen(str); i++)
  {
    if((str[i-2] == '/') && (str[i-1] == 'c')){
      arg = str[i];
      switch(arg){
       case 'l': case 'r':
       case 'L': case 'R':
       case 'g': case 'y':
       case 'G': case 'Y':
       case 'b': case 'm':
       case 'B': case 'M':
       case 'c': case 'w':
       case 'C': case 'W':
       case 'v': case 'u':
       case 'V': case '0':
       case 'p': case 'P':
         to_add += 3;
         break;
       default:
         break;
      }
    }
  }

  strcpy(modi, str);  

  for(k = 0; k < to_add; k++)
    strcat(modi, " ");  
}

void colorless_str(char *str)
{
  char tmpb[2*MAX_STRING_LENGTH];
  int len = strlen(str), i, j;
  char tmp2[2*MAX_STRING_LENGTH];
  char ch;

  strcpy(tmpb, str);
  if (len <= 2) return;
  for (i = 0, j = 0; i < len; j++, i++)
  {
    if(tmpb[i] == '/')
      if(tmpb[i+1] == 'c') {
        ch = tmpb[i+2];
        switch (ch){
          case 'l': case 'r':
          case 'L': case 'R':
          case 'g': case 'y':
          case 'G': case 'Y':
          case 'b': case 'm':
          case 'B': case 'M':
          case 'c': case 'w':
          case 'C': case 'W': 
          case 'v': case 'u':
          case 'V': case '0':
          case 'p': case 'P':
            i+=3;
            break;
          default:
            break;
         }
       }
   tmp2[j] = tmpb[i];
   }
   tmp2[j] = '\0';
   sprintf(str, "%s", tmp2);
}
 

char *list_object(struct obj_data * obj, int cnt, int index, int shop_nr)
{
  static char buf[256];
  char buf2[300], buf3[200], buf4[200];
 /* extern char *drinks[];*/


/*Working on new list format -Rapax*/
/*if (shop_producing(obj, shop_nr))
    strcpy(buf2, "Unl  ";
  else
    sprintf(buf2, "%2d   ", cnt);
*/ 
  if(obj->obj_flags.minlevel <= 1)
    strcpy(buf3, " - ");
  else
    sprintf(buf3, "%3d", obj->obj_flags.minlevel);

  sprintf(buf, "%2d)  %s   ",index , buf3);
  
  strcpy(buf4, obj->short_description);
  colorless_str(buf4);

  sprintf(buf2, "%-58s %6d\r\n", buf4, buy_price(obj, shop_nr));
  strcat(buf, buf2);
  return (buf);
}


 
/* Old list code */
/*  if (shop_producing(obj, shop_nr))
    strcpy(buf2, "Unlimited   ");
  else
    sprintf(buf2, "%5d       ", cnt);
  sprintf(buf, " %2d)  %s", index, buf2);

  
  strcpy(buf3, obj->short_description);

  if ((GET_OBJ_TYPE(obj) == ITEM_DRINKCON) && (GET_OBJ_VAL(obj, 1)))
    sprintf(END_OF(buf3), " of %s", drinks[GET_OBJ_VAL(obj, 2)]);

  if(obj->obj_flags.minlevel > 0)
    sprintf(buf3, "%s(%d)",buf3, obj->obj_flags.minlevel);

  if ((GET_OBJ_TYPE(obj) == ITEM_WAND) || (GET_OBJ_TYPE(obj) == ITEM_STAFF))
    if (GET_OBJ_VAL(obj, 2) < GET_OBJ_VAL(obj, 1))
      strcat(buf3, " (partially used)");

  append_space_to_str(buf3, buf4);

  sprintf(buf2, "%-48s %6d\n\r", buf4, buy_price(obj, shop_nr));
  strcat(buf, CAP(buf2));
  return (buf);
}*/


void shopping_list(char *arg, struct char_data * ch,
		        struct char_data * keeper, int shop_nr)
{
  char buf[MAX_STRING_LENGTH], name[200];
  struct obj_data *obj, *last_obj = 0;
  int cnt = 0, index = 0;

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  if (SHOP_SORT(shop_nr) < IS_CARRYING_N(keeper))
    sort_keeper_objs(keeper, shop_nr);

  one_argument(arg, name);
  strcpy(buf, " ##  LVL     ITEM                                                    Cost\n\r");
  strcat(buf, "-------------------------------------------------------------------------\n\r");
  if (keeper->carrying)
    for (obj = keeper->carrying; obj; obj = obj->next_content)
      if (CAN_SEE_OBJ(ch, obj) && (obj->obj_flags.cost > 0)) {
	if (!last_obj) {
	  last_obj = obj;
	  cnt = 1;
	} else if (same_obj(last_obj, obj))
	  cnt++;
	else {
	  index++;
	  if (!(*name) || isname(name, last_obj->name))
	    strcat(buf, list_object(last_obj, cnt, index, shop_nr));
	  cnt = 1;
	  last_obj = obj;
	}
      }
  index++;
  if (!last_obj)
    if (*name)
      strcpy(buf, "Presently, none of those are for sale.\n\r");
    else
      strcpy(buf, "Currently, there is nothing for sale.\n\r");
  else if (!(*name) || isname(name, last_obj->name))
    strcat(buf, list_object(last_obj, cnt, index, shop_nr));

  page_string(ch->desc, buf, 1);
}


int ok_shop_room(int shop_nr, int room)
{
  int index;

  for (index = 0; SHOP_ROOM(shop_nr, index) != NOWHERE; index++)
    if (SHOP_ROOM(shop_nr, index) == room)
      return (TRUE);
  return (FALSE);
}


SPECIAL(shop_keeper)
{
  char argm[MAX_INPUT_LENGTH];
  struct char_data *keeper = (struct char_data *) me;
  int shop_nr;

  for (shop_nr = 0; shop_nr < top_shop; shop_nr++)
    if (SHOP_KEEPER(shop_nr) == keeper->nr)
      break;

  if (shop_nr >= top_shop)
    return (FALSE);

  if (SHOP_FUNC(shop_nr))	/* Check secondary function */
    if ((SHOP_FUNC(shop_nr)) (ch, me, cmd, arg))
      return (TRUE);

  if (keeper == ch) {
    if (cmd)
      SHOP_SORT(shop_nr) = 0;	/* Safety in case "drop all" */
    return (FALSE);
  }
  if (!ok_shop_room(shop_nr, world[ch->in_room].number))
    return (0);

  if (!AWAKE(keeper))
    return (FALSE);

  if (CMD_IS("steal")) {
    sprintf(argm, "$N shouts '%s'", MSG_NO_STEAL_HERE);
    do_action(keeper, GET_NAME(ch), cmd_slap, 0);
    act(argm, FALSE, ch, 0, keeper, TO_CHAR);
    return (TRUE);
  }

  if (CMD_IS("buy")) {
    shopping_buy(argument, ch, keeper, shop_nr);
    return (TRUE);
  } else if (CMD_IS("sell")) {
    shopping_sell(argument, ch, keeper, shop_nr);
    return (TRUE);
  } else if (CMD_IS("value")) {
    shopping_value(argument, ch, keeper, shop_nr);
    return (TRUE);
  } else if (CMD_IS("list")) {
    shopping_list(argument, ch, keeper, shop_nr);
    return (TRUE);
  } else if (CMD_IS("inform")) {
    inform(argument, ch, keeper, shop_nr);
    return (TRUE);
  }
  return (FALSE);
}


int ok_damage_shopkeeper(struct char_data * ch, struct char_data * victim)
{
  char buf[200];
  int index;

  if (IS_NPC(victim) && (mob_index[GET_MOB_RNUM(victim)].func == shop_keeper))
    for (index = 0; index < top_shop; index++)
      if ((GET_MOB_RNUM(victim) == SHOP_KEEPER(index)) && !SHOP_KILL_CHARS(index)) {
	do_action(victim, GET_NAME(ch), cmd_slap, 0);
	sprintf(buf, "%s %s", GET_NAME(ch), MSG_CANT_KILL_KEEPER);
	do_tell(victim, buf, cmd_tell, 0);
	return (FALSE);
      }
  return (TRUE);
}


int add_to_list(struct shop_buy_data * list, int type, int *len, int *val)
{
  if (*val >= 0) {
    if (*len < MAX_SHOP_OBJ) {
      if (type == LIST_PRODUCE)
	*val = real_object(*val);
      if (*val >= 0) {
	BUY_TYPE(list[*len]) = *val;
	BUY_WORD(list[(*len)++]) = 0;
      } else
	*val = 0;
      return (FALSE);
    } else
      return (TRUE);
  }
  return (FALSE);
}


int end_read_list(struct shop_buy_data * list, int len, int error)
{
  if (error) {
    sprintf(buf, "Raise MAX_SHOP_OBJ constant in shop.h to %d", len + error);
    log(buf);
  }
  BUY_WORD(list[len]) = 0;
  BUY_TYPE(list[len++]) = NOTHING;
  return (len);
}


void read_line(FILE * shop_f, char *string, void *data)
{
  if (!get_line(shop_f, buf) || !sscanf(buf, string, data)) {
    fprintf(stderr, "SYSERR: Error in shop #%d\n", SHOP_NUM(top_shop));
    exit(1);
  }
}


int read_list(FILE * shop_f, struct shop_buy_data * list, int new_format,
	          int max, int type)
{
  int count, temp, len = 0, error = 0;

  if (new_format) {
    do {
      read_line(shop_f, "%d", &temp);
      error += add_to_list(list, type, &len, &temp);
    } while (temp >= 0);
  } else
    for (count = 0; count < max; count++) {
      read_line(shop_f, "%d", &temp);
      error += add_to_list(list, type, &len, &temp);
    }
  return (end_read_list(list, len, error));
}


int read_type_list(FILE * shop_f, struct shop_buy_data * list,
		       int new_format, int max)
{
  int index, num, len = 0, error = 0;
  char *ptr;

  if (!new_format)
    return (read_list(shop_f, list, 0, max, LIST_TRADE));
  do {
    fgets(buf, MAX_STRING_LENGTH - 1, shop_f);
    if ((ptr = strchr(buf, ';')) != NULL)
      *ptr = 0;
    else
      *(END_OF(buf) - 1) = 0;
    for (index = 0, num = NOTHING; *item_types[index] != '\n'; index++)
      if (!strn_cmp(item_types[index], buf, strlen(item_types[index]))) {
	num = index;
	strcpy(buf, buf + strlen(item_types[index]));
	break;
      }
    ptr = buf;
    if (num == NOTHING) {
      sscanf(buf, "%d", &num);
      while (!isdigit(*ptr))
	ptr++;
      while (isdigit(*ptr))
	ptr++;
    }
    while (isspace(*ptr))
      ptr++;
    while (isspace(*(END_OF(ptr) - 1)))
      *(END_OF(ptr) - 1) = 0;
    error += add_to_list(list, LIST_TRADE, &len, &num);
    if (*ptr)
      BUY_WORD(list[len - 1]) = str_dup(ptr);
  } while (num >= 0);
  return (end_read_list(list, len, error));
}


void boot_the_shops(FILE * shop_f, char *filename, int rec_count)
{
  char *buf, buf2[150];
  int temp, count, new_format = 0;
  struct shop_buy_data list[MAX_SHOP_OBJ + 1];
  int done = 0;

  sprintf(buf2, "beginning of shop file %s", filename);

  while (!done) {
    buf = fread_string(shop_f, buf2);
    if (*buf == '#') {		/* New shop */
      sscanf(buf, "#%d\n", &temp);
      sprintf(buf2, "shop #%d in shop file %s", temp, filename);
      free(buf);		/* Plug memory leak! */
      if (!top_shop)
	CREATE(shop_index, struct shop_data, rec_count);

      SHOP_NUM(top_shop) = temp;
      temp = read_list(shop_f, list, new_format, MAX_PROD, LIST_PRODUCE);
      CREATE(shop_index[top_shop].producing, int, temp);
      for (count = 0; count < temp; count++)
	SHOP_PRODUCT(top_shop, count) = BUY_TYPE(list[count]);

      read_line(shop_f, "%f", &SHOP_BUYPROFIT(top_shop));
      read_line(shop_f, "%f", &SHOP_SELLPROFIT(top_shop));

      temp = read_type_list(shop_f, list, new_format, MAX_TRADE);
      CREATE(shop_index[top_shop].type, struct shop_buy_data, temp);
      for (count = 0; count < temp; count++) {
	SHOP_BUYTYPE(top_shop, count) = BUY_TYPE(list[count]);
	SHOP_BUYWORD(top_shop, count) = BUY_WORD(list[count]);
      }

      shop_index[top_shop].no_such_item1 = fread_string(shop_f, buf2);
      shop_index[top_shop].no_such_item2 = fread_string(shop_f, buf2);
      shop_index[top_shop].do_not_buy = fread_string(shop_f, buf2);
      shop_index[top_shop].missing_cash1 = fread_string(shop_f, buf2);
      shop_index[top_shop].missing_cash2 = fread_string(shop_f, buf2);
      shop_index[top_shop].message_buy = fread_string(shop_f, buf2);
      shop_index[top_shop].message_sell = fread_string(shop_f, buf2);
      read_line(shop_f, "%d", &SHOP_BROKE_TEMPER(top_shop));
      read_line(shop_f, "%d", &SHOP_BITVECTOR(top_shop));
      read_line(shop_f, "%d", &SHOP_KEEPER(top_shop));

      SHOP_KEEPER(top_shop) = real_mobile(SHOP_KEEPER(top_shop));
      read_line(shop_f, "%d", &SHOP_TRADE_WITH(top_shop));

      temp = read_list(shop_f, list, new_format, 1, LIST_ROOM);
      CREATE(shop_index[top_shop].in_room, int, temp);
      for (count = 0; count < temp; count++)
	SHOP_ROOM(top_shop, count) = BUY_TYPE(list[count]);

      read_line(shop_f, "%d", &SHOP_OPEN1(top_shop));
      read_line(shop_f, "%d", &SHOP_CLOSE1(top_shop));
      read_line(shop_f, "%d", &SHOP_OPEN2(top_shop));
      read_line(shop_f, "%d", &SHOP_CLOSE2(top_shop));

      SHOP_BANK(top_shop) = 0;
      SHOP_SORT(top_shop) = 0;
      SHOP_FUNC(top_shop) = 0;
      top_shop++;
    } else {
      if (*buf == '$')		/* EOF */
	done = TRUE;
      else if (strstr(buf, VERSION3_TAG))	/* New format marker */
	new_format = 1;
      free(buf);		/* Plug memory leak! */
    }
  }
}


void assign_the_shopkeepers(void)
{
  int index;

  cmd_say = find_command("say");
  cmd_tell = find_command("tell");
  cmd_emote = find_command("emote");
  cmd_slap = find_command("slap");
  cmd_puke = find_command("puke");
  for (index = 0; index < top_shop; index++) {
    if (SHOP_KEEPER(index) == NOBODY)
      continue;
    if (mob_index[SHOP_KEEPER(index)].func)
      SHOP_FUNC(index) = mob_index[SHOP_KEEPER(index)].func;
    mob_index[SHOP_KEEPER(index)].func = shop_keeper;
  }
}


char *customer_string(int shop_nr, int detailed)
{
  int index, cnt = 1;
  static char buf[256];

  *buf = 0;
  for (index = 0; *trade_letters[index] != '\n'; index++, cnt *= 2)
    if (!(SHOP_TRADE_WITH(shop_nr) & cnt))
      if (detailed) {
	if (*buf)
	  strcat(buf, ", ");
	strcat(buf, trade_letters[index]);
      } else
	sprintf(END_OF(buf), "%c", *trade_letters[index]);
    else if (!detailed)
      strcat(buf, "_");

  return (buf);
}


void list_all_shops(struct char_data * ch)
{
  int shop_nr;

  strcpy(buf, "\n\r");
  for (shop_nr = 0; shop_nr < top_shop; shop_nr++) {
    if (!(shop_nr % 19)) {
      strcat(buf, " ##   Virtual   Where    Keeper    Buy   Sell   Customers\n\r");
      strcat(buf, "---------------------------------------------------------\n\r");
    }
    sprintf(buf2, "%3d   %6d   %6d    ", shop_nr + 1, SHOP_NUM(shop_nr),
	    SHOP_ROOM(shop_nr, 0));
    if (SHOP_KEEPER(shop_nr) < 0)
      strcpy(buf1, "<NONE>");
    else
      sprintf(buf1, "%6d", mob_index[SHOP_KEEPER(shop_nr)].virtual);
    sprintf(END_OF(buf2), "%s   %3.2f   %3.2f    ", buf1,
	    SHOP_SELLPROFIT(shop_nr), SHOP_BUYPROFIT(shop_nr));
    strcat(buf2, customer_string(shop_nr, FALSE));
    sprintf(END_OF(buf), "%s\n\r", buf2);
  }

  page_string(ch->desc, buf, 1);
}


void handle_detailed_list(char *buf, char *buf1, struct char_data * ch)
{
  if ((strlen(buf1) + strlen(buf) < 78) || (strlen(buf) < 20))
    strcat(buf, buf1);
  else {
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    sprintf(buf, "            %s", buf1);
  }
}


void list_detailed_shop(struct char_data * ch, int shop_nr)
{
  struct obj_data *obj;
  struct char_data *k;
  int index, temp;

  sprintf(buf, "Vnum:       [%5d], Rnum: [%5d]\n\r", SHOP_NUM(shop_nr),
	  shop_nr + 1);
  send_to_char(buf, ch);

  strcpy(buf, "Rooms:      ");
  for (index = 0; SHOP_ROOM(shop_nr, index) != NOWHERE; index++) {
    if (index)
      strcat(buf, ", ");
    if ((temp = real_room(SHOP_ROOM(shop_nr, index))) != NOWHERE)
      sprintf(buf1, "%s (#%d)", world[temp].name, world[temp].number);
    else
      sprintf(buf1, "<UNKNOWN> (#%d)", SHOP_ROOM(shop_nr, index));
    handle_detailed_list(buf, buf1, ch);
  }
  if (!index)
    send_to_char("Rooms:      None!\n\r", ch);
  else {
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
  }

  strcpy(buf, "Shopkeeper: ");
  if (SHOP_KEEPER(shop_nr) >= 0) {
    sprintf(END_OF(buf), "%s (#%d), Special Function: %s\n\r",
	    GET_NAME(&mob_proto[SHOP_KEEPER(shop_nr)]),
	mob_index[SHOP_KEEPER(shop_nr)].virtual, YESNO(SHOP_FUNC(shop_nr)));
    if ((k = get_char_num(SHOP_KEEPER(shop_nr)))) {
      send_to_char(buf, ch);
      sprintf(buf, "Coins:      [%9d], Bank: [%9d] (Total: %d)\n\r",
	 GET_GOLD(k), SHOP_BANK(shop_nr), GET_GOLD(k) + SHOP_BANK(shop_nr));
    }
  } else
    strcat(buf, "<NONE>\n\r");
  send_to_char(buf, ch);

  strcpy(buf1, customer_string(shop_nr, TRUE));
  sprintf(buf, "Customers:  %s\n\r", (*buf1) ? buf1 : "None");
  send_to_char(buf, ch);

  strcpy(buf, "Produces:   ");
  for (index = 0; SHOP_PRODUCT(shop_nr, index) != NOTHING; index++) {
    obj = &obj_proto[SHOP_PRODUCT(shop_nr, index)];
    if (index)
      strcat(buf, ", ");
    sprintf(buf1, "%s (#%d)", obj->short_description,
	    obj_index[SHOP_PRODUCT(shop_nr, index)].virtual);
    handle_detailed_list(buf, buf1, ch);
  }
  if (!index)
    send_to_char("Produces:   Nothing!\n\r", ch);
  else {
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
  }

  strcpy(buf, "Buys:       ");
  for (index = 0; SHOP_BUYTYPE(shop_nr, index) != NOTHING; index++) {
    if (index)
      strcat(buf, ", ");
    sprintf(buf1, "%s (#%d) ", item_types[SHOP_BUYTYPE(shop_nr, index)],
	    SHOP_BUYTYPE(shop_nr, index));
    if (SHOP_BUYWORD(shop_nr, index))
      sprintf(END_OF(buf1), "[%s]", SHOP_BUYWORD(shop_nr, index));
    else
      strcat(buf1, "[all]");
    handle_detailed_list(buf, buf1, ch);
  }
  if (!index)
    send_to_char("Buys:       Nothing!\n\r", ch);
  else {
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
  }

  sprintf(buf, "Buy at:     [%4.2f], Sell at: [%4.2f], Open: [%d-%d, %d-%d]%s",
     SHOP_SELLPROFIT(shop_nr), SHOP_BUYPROFIT(shop_nr), SHOP_OPEN1(shop_nr),
   SHOP_CLOSE1(shop_nr), SHOP_OPEN2(shop_nr), SHOP_CLOSE2(shop_nr), "\n\r");

  send_to_char(buf, ch);

  sprintbit((long) SHOP_BITVECTOR(shop_nr), shop_bits, buf1);
  sprintf(buf, "Bits:       %s\n\r", buf1);
  send_to_char(buf, ch);
}


void show_shops(struct char_data * ch, char *arg)
{
  send_to_char("This option has been disabled.\r\n", ch);
  return;
}
/*
  int shop_nr;

  if (!*arg)
    list_all_shops(ch);
  else {
    if (!str_cmp(arg, ".")) {
      for (shop_nr = 0; shop_nr < top_shop; shop_nr++)
	if (ok_shop_room(shop_nr, world[ch->in_room].number))
	  break;

      if (shop_nr == top_shop) {
	send_to_char("This isn't a shop!\n\r", ch);
	return;
      }
    } else if (is_number(arg))
      shop_nr = atoi(arg) - 1;
    else
      shop_nr = -1;

    if ((shop_nr < 0) || (shop_nr >= top_shop)) {
      send_to_char("Illegal shop number.\n\r", ch);
      return;
    }
    list_detailed_shop(ch, shop_nr);
  }
}
*/

/*---------------------- VENDORS -------------------------*/
int PRICE(struct obj_data *obj)
{
	int multiplier = 0, cost = 0;
	if (obj == NULL)
		return (0);
	else {
		multiplier = 2;
		// Add other changes to multipier here.
		if (GET_OBJ_TYPE(obj) == ITEM_RUNE)
			multiplier = 5;

		cost = (GET_OBJ_COST(obj) * multiplier);
		if (cost == 0)
			cost = (GET_OBJ_LEVEL(obj) * 500);
		return ((int)cost);
	}
}

/* Mob          :  The shopkeeper.
 * Room         :  The dropoff room, where objects to be sold in shop are dropped.
 * Type         :  0 = Individual   1 = Clan.
 * Pid          :  If Individual, PlayerID of player owning the shop.
 * ClanId       :  If Clan, IdentNR of the clan.
 * HasIdentify  :  If Clan, does it have 0, no identify option, or 1, identify option.
 */
const int clan_stores[][6]={
//    Mob  Room   Type  Pid  ClanId   HasIdentify
    {30506, 30509, 1,    0,     5,        1},	// Darklords (Silvis)
    {30507, 30510, 1,    0,     5,        1},   // Darklords (Jade)
    {30508, 30511, 1,    0,     5,        1},   // Darklords (Katrina)
    {30203, 30206, 1,    0,     2,        1}, // Eternals
    {  -1,   -1,  -1,    0,    -1,       -1}
  };

  char *clan_stores_owners[] = {
    "Silvis",
    "Jade",
    "Katrina",
    "Eternals",
    "Starlyx",
    "\n"
  };

struct obj_data *vendor_get_slide_obj_vis(struct char_data * ch, char *name,
				            struct obj_data * list)
{
  struct obj_data *i, *last_match = 0;
  int j, number;
  char tmpname[MAX_INPUT_LENGTH];
  char *tmp;

  strcpy(tmpname, name);
  tmp = tmpname;
  if (!(number = get_number(&tmp)))
    return (0);

  for (i = list, j = 1; i && (j <= number); i = i->next_content)
    if (isname(tmp, i->name))
      if (GET_OBJ_TYPE(i) != ITEM_MONEY && CAN_SEE_OBJ(ch, i) && !same_obj(last_match, i) && i->obj_flags.player == -1) {
	if (j == number)
	  return (i);
	last_match = i;
	j++;
      }
  return (0);
}


struct obj_data *vendor_get_hash_obj_vis(struct char_data * ch, char *name,
				           struct obj_data * list)
{
  struct obj_data *loop, *last_obj = 0;
  int index;

  if ((is_number(name + 1)))
    index = atoi(name + 1);
  else
    return (0);

  for (loop = list; loop; loop = loop->next_content)
    if (GET_OBJ_TYPE(loop) != ITEM_MONEY && CAN_SEE_OBJ(ch, loop) && (loop->obj_flags.cost > 0))
      if (!same_obj(last_obj, loop)) {
	if (--index == 0)
	  return (loop);
	last_obj = loop;
      }
  return (0);
}


struct obj_data *vendor_get_purchase_obj(struct char_data * ch, char *arg,
		            int room, int msg)
{
  char name[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  int rnum = real_room(room);

  one_argument(arg, name);
  do {
    if (*name == '#')
      obj = vendor_get_hash_obj_vis(ch, name, world[rnum].contents);
    else
      obj = vendor_get_slide_obj_vis(ch, name, world[rnum].contents);
    if (!obj) {
      if (msg) {
	send_to_char("I'm afraid I don't have that in stock.\r\n", ch);
      }
      return (0);
    }
    if (GET_OBJ_COST(obj) <= 0) {
      extract_obj(obj);
      obj = 0;
    }
  } while (!obj);
  return (obj);
}

char *vendor_list_object(struct obj_data * obj, int cnt, int index)
{
  static char buf[256];
  char buf2[300], buf3[200], buf4[200], buf5[256];
 /* extern char *drinks[];*/

  if(obj->obj_flags.minlevel <= 1)
    strcpy(buf3, " - ");
  else
    sprintf(buf3, "%3d", obj->obj_flags.minlevel);

  sprintf(buf, "%2d)  %s   ",index , buf3);
  
  if(GET_OBJ_TYPE(obj) == ITEM_RUNE && real_room(GET_OBJ_VAL(obj, 1))) {
    sprintf(buf5, "A rune to %s", world[real_room(GET_OBJ_VAL(obj, 1))].name);
    strcpy(buf4, buf5);
  }
/*  else if (isname(obj->name, "mount_token") && real_mobile(GET_OBJ_VAL(obj, 1))) {
    sprintf(buf5, "A stable token for %s", GET_NAME(real_mobile(GET_OBJ_VAL(obj, 1))));
    strcpy(buf4, buf5);
  } */
  else {
     strcpy(buf4, obj->short_description);
  }
  colorless_str(buf4);

  sprintf(buf2, "%-58s %6d\r\n", buf4, PRICE(obj));
  strcat(buf, buf2);
  return (buf);
}

void vendor_buy(char *arg, struct char_data * ch,
		       struct char_data *keeper, int room, int i)
{
  char tempstr[200], buf[MAX_STRING_LENGTH]; // cbuf[MAX_STRING_LENGTH];
  struct obj_data *obj, *last_obj = NULL;
  int price = 0, j, goldamt = 0, buynum, bought = 0, clan_num = 0, found = 0;
  int rnum = real_room(room), pamt;
  struct descriptor_data *d;
  struct char_file_u chdata;
  extern struct player_index_element *player_table; 
  extern int top_of_p_table;
 
  if ((buynum = transaction_amt(arg)) < 0) {
    sprintf(buf, "%s A negative amount?  Try selling me something.",
	    GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  if (!(*arg) || !(buynum)) {
    sprintf(buf, "%s What do you want to buy??", GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
    return;
  }
  if (!(obj = vendor_get_purchase_obj(ch, arg, room, TRUE)))
    return;

  if(GET_OBJ_TYPE(obj) == ITEM_MONEY) {
   send_to_char("You can't buy that!\r\n", ch);
   return;
  }
  if(obj->obj_flags.player != -1) {
   send_to_char("That eq is owned, you can't buy it.\r\n", ch);
   return;
  }
  if ((PRICE(obj) > GET_GOLD(ch)) && !IS_GOD(ch)) {
    sprintf(buf, "%s You don't have enough money!", GET_NAME(ch));
    do_tell(keeper, buf, cmd_tell, 0);
  }
  if ((IS_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
    sprintf(buf, "%s: You can't carry any more items.\n\r",
	    fname(obj->name));
    send_to_char(buf, ch);
    return;
  }
  if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
    sprintf(buf, "%s: You can't carry that much weight.\n\r",
	    fname(obj->name));
    send_to_char(buf, ch);
    return;
  }
  while ((obj) && ((GET_GOLD(ch) >= PRICE(obj)) || IS_GOD(ch))
	 && (IS_CARRYING_N(ch) < CAN_CARRY_N(ch)) && (bought < buynum)
	 && (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) <= CAN_CARRY_W(ch))) {
    bought++;
    /* Test if producing shop ! */
    obj_from_room(obj);
    obj_to_char(obj, ch);

    goldamt += PRICE(obj);

    if(GET_GOLD(ch) < PRICE(obj)) {
      send_to_char("You do not have enough gold.\r\n", ch);
      return;
    }
   
    price = PRICE(obj);


    // Give war zone and clan discounts 
    if(in_war_zone(ch)) {
      price *= 1.75;

    }
    else if(in_allied_lands(ch)) {
      price *= .9;
    }
    else if(in_my_clan_lands(ch)) {
      price *= .75;
    }

    if (!IS_GOD(ch)) {
      GET_GOLD(ch) -= price;

    pamt = price * .75;  

	// I removed Vedics version of this.
	// Vedics code would only work if the vendor was
	// placed in a zone belonging to the clan.
	// And, hey, clans can buy vendors and place
	// them in other zones than their own.
    if(clan_stores[i][2] == 1)
	{
		clan_num = find_clan_by_id(clan_stores[i][4]);
        clan[clan_num].treasure += (int)pamt;
		save_char(ch, ch->in_room);
		save_clans();
    }
    if(clan_stores[i][2] == 2) {
   // Otherwise it is owned by an idividual and we deposit to their bank account

      for (d = descriptor_list; d && !found; d = d->next) {
        if (d->connected) continue;
 
        if(GET_IDNUM(d->character) == clan_stores[i][3]) { 
          GET_BANK_GOLD(d->character) += pamt;
          save_char(d->character, d->character->in_room);
          found = 1;
        }
      }
      if(!found) {
        for (j = 0; j <= top_of_p_table; j++){
          load_char((player_table + j)->name, &chdata);
          if(isname(chdata.name, clan_stores_owners[i])) {
            chdata.points.bank_gold += pamt;;
            save_char_file_u(chdata); 
          }
        } 
      }
    }
  }
   
    last_obj = obj;
    obj = vendor_get_purchase_obj(ch, arg, room, FALSE);
    if (!same_obj(obj, last_obj))
      break;
  }
  if (bought < buynum) {
    if (!obj || !same_obj(last_obj, obj))
      sprintf(buf, "%s I only have %d to sell you.", GET_NAME(ch), bought);
    else if (GET_GOLD(ch) < price)
      sprintf(buf, "%s You can only afford %d.", GET_NAME(ch), bought);
    else if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
      sprintf(buf, "%s You can only hold %d.", GET_NAME(ch), bought);
    else if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) > CAN_CARRY_W(ch))
      sprintf(buf, "%s You can only carry %d.", GET_NAME(ch), bought);
    else
      sprintf(buf, "%s Something screwy only gave you %d.", GET_NAME(ch),
	      bought);
    do_tell(keeper, buf, cmd_tell, 0);
  }

  sprintf(tempstr, times_message(ch->carrying, 0, bought));
  sprintf(buf, "$n buys %s.", tempstr);
  act(buf, FALSE, ch, obj, 0, TO_ROOM);

  sprintf(buf, "%s That will be %d gold.", GET_NAME(ch), goldamt);
  do_tell(keeper, buf, cmd_tell, 0);
  sprintf(buf, "You now have %s.\n\r", tempstr);
  send_to_char(buf, ch);

/*
  if((zone_table[world[rnum].zone].owner) && (clan_stores[i][2] == 1)) {
    sprintf(cbuf, "%s %s thanks you for your purchase.", 
            GET_NAME(ch), clan[clan_num].name +1);
    do_tell(keeper, cbuf, cmd_tell, 0);
  }
*/
}

#define VENDTYPE_LIST		0
#define VENDTYPE_IDENT		1
#define VENDTYPE_BUY		2

SPECIAL(vendor_shop)
{
  int obj_room, i, found = FALSE;
  //int index = 0;
  //int cnt = 1;
  //struct obj_data *obj, *last_obj = NULL;

  for (i = 0; clan_stores[i][0] > 0 && !found; i++)
    if (GET_MOB_VNUM((struct char_data *)me) == clan_stores[i][0] &&
        IN_ROOM(ch) == IN_ROOM((struct char_data *)me))
      found=TRUE;

  if (!found)
    return FALSE;
  i--;

  obj_room = real_room(clan_stores[i][1]);


  if (CMD_IS("list")) {
    sprintf(buf, "Available items are:\r\n");
  strcat(buf, " ##  ITEMS  LVL   ITEM                                               Cost\n\r");
  strcat(buf, "-------------------------------------------------------------------------\n\r");
  send_to_char(buf, ch);
  vendor_objects(ch, obj_room, VENDTYPE_LIST, argument, i);
  return 1;
  } else if (CMD_IS("buy")) {

    //vendor_buy(argument, ch, (struct char_data *)me, clan_stores[i][1], i);  
	  vendor_objects(ch, obj_room, VENDTYPE_BUY, argument, i);
    return 1;
  } else if (CMD_IS("identify")) {
	  vendor_objects(ch, obj_room, VENDTYPE_IDENT, argument, i);
    return 1;
  }
  /* All commands except list and buy */
  return 0;
}

void vendor_objects(struct char_data *ch, int obj_room, int type, char *argument, int j)
{
	struct obj_data *obj = NULL;
	struct obj_data *oboj = NULL;
	struct obj_data *obj_b = NULL;
	char output[MAX_STRING_LENGTH];
	char tmp_name[MAX_INPUT_LENGTH];
	int i = 0, nr_items = 0, ident_nr = 0, arg_nr = 0;
	int clan_num = 0, buy_nr = 0, price = 0;
	char objects[150][150];
	int objects_nr[150];
	int objects_vn[150];
	bool duplicate = FALSE, item_found = FALSE;

	if (ch == NULL)
		return;
	else if (obj_room == 0)
		return;
	else {
		for(i = 0; i <= 150;i++)
		{
			*objects[i] = '\0';
			objects_vn[i] = 0;
			objects_nr[i] = 0;
		}
		nr_items = 0;
		*output = '\0';
		for (obj = world[obj_room].contents; obj; obj = obj->next_content)
		{
			duplicate = FALSE;
			if (nr_items >= 100)
				continue;
			else if (obj == NULL)
				continue;
			else if (nr_items == 150)
				continue;
			else if (GET_OBJ_TYPE(obj) == ITEM_MONEY)
				continue;
			else {
				for(i = 0; i <= 150;i++)
					if (GET_OBJ_VNUM(obj) == objects_vn[i] && !(GET_OBJ_TYPE(obj) == ITEM_RUNE && real_room(GET_OBJ_VAL(obj, 1))))
					{
						objects_nr[i]++;
						duplicate = TRUE;
					}
				if (duplicate == FALSE)
				{
					nr_items++;
					if (objects_vn[nr_items-1] == 0)
					{
						if (GET_OBJ_TYPE(obj) == ITEM_RUNE && real_room(GET_OBJ_VAL(obj, 1)))
						{
							sprintf(tmp_name, "%-39s", world[real_room(GET_OBJ_VAL(obj, 1))].name);
							colorless_str(tmp_name);
							sprintf(objects[nr_items-1], "[%3d]   A rune to %-39s  %6d", GET_OBJ_LEVEL(obj), tmp_name, PRICE(obj));
						} else {
							sprintf(tmp_name, "%-49s", obj->short_description); 
							colorless_str(tmp_name);
							sprintf(objects[nr_items-1], "[%3d]   %-49s  %6d", GET_OBJ_LEVEL(obj), tmp_name, PRICE(obj));
						}
					}
					objects_vn[nr_items-1] = GET_OBJ_VNUM(obj);
					objects_nr[nr_items-1] = 1;
				}
			}
		} // end of first loop
		if (type == VENDTYPE_LIST)
		{
			for(i = 0; i <= nr_items-1;i++)
				sprintf(output, "%s%2d) (%3d) %s\r\n", output, (i+1), objects_nr[i], objects[i]);
			sprintf(output, "%s\r\n %d items listed.\r\n", output, nr_items);
			colorless_str(output);
			page_string(ch->desc, output, 1);
			return;
		}
		else if (type == VENDTYPE_IDENT)
		{
			arg_nr = atoi(argument);
			if (arg_nr == 0)
				send_to_char("You need to enter a number of object to identify.\r\n", ch);
			else
			{
				if (clan_stores[j][5] != 0)
				{
					ident_nr = objects_vn[(arg_nr-1)];
					if (ident_nr)
					{
						if ((oboj = read_object((ident_nr), VIRTUAL)) == NULL)
							send_to_char("There is no object with that number.\r\n", ch);
						else {
							if (GET_GOLD(ch) < 50)
								send_to_char("You are to poor!\r\n", ch);
							else {
								spell_identify(150, ch, 0, oboj);
								GET_GOLD(ch) -= 50;
								clan_num = find_clan_by_id(clan_stores[j][4]);
								clan[clan_num].treasure += 50;
								save_char(ch, ch->in_room);
								save_clans();
							}
						}
					} else {
						send_to_char("No item with that number.\r\n", ch);
						return;
					}
				} else {
					send_to_char("This shop do not support identifying.\r\n", ch);
					return;
				}
			}
			return;
		}
		else if (type == VENDTYPE_BUY)
		{
			buy_nr = atoi(argument);
			if (buy_nr >= 1 && buy_nr <= nr_items)
			{
				for (obj_b = world[obj_room].contents; obj_b; obj_b = obj_b->next_content)
				{
					if (GET_OBJ_VNUM(obj_b) == objects_vn[(buy_nr-1)])
					{
						item_found = TRUE;
						price = PRICE(obj_b);
						if (GET_GOLD(ch) >= price)
						{
							obj_from_room(obj_b);
							obj_to_char(obj_b, ch);
							GET_GOLD(ch) -= price;
							sprintf(output, "You now got %s./c0\r\n", obj_b->short_description);
							send_to_char(output, ch);
							price = (PRICE(obj_b) * .75);
							clan_num = find_clan_by_id(clan_stores[j][4]);
							clan[clan_num].treasure += price;
							save_char(ch, ch->in_room);
							save_clans();
							return;
						} else {
							send_to_char("You can not afford it.\r\n", ch);
							return;
						}
					}
				}
				if (item_found == TRUE)
					return;
				else {
					send_to_char("No item found.\r\n", ch);
					return;
				}
			} else {
				send_to_char("No item with that number.\r\n", ch);
				return;
			}
		}
	}
}

