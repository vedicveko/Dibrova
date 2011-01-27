/************************************************************************
 * OasisOLC - oedit.c						v1.5	*
 * Copyright 1996 Harvey Gilpin.					*
 * Original author: Levork						*
 ************************************************************************/

#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "spells.h"
#include "utils.h"
#include "db.h"
#include "boards.h"
#include "shop.h"
#include "olc.h"
#include "clan.h"
#include "dg_olc.h"

/*------------------------------------------------------------------------*/

/*
 * External variable declarations.
 */

extern struct obj_data *obj_proto;
extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern int top_of_objt;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
extern struct shop_data *shop_index;
extern int top_shop;
extern struct attack_hit_type attack_hit_text[];
extern char *item_types[];
extern char *wear_bits[];
extern char *extra_bits[];
extern char *drinks[];
extern char *apply_types[];
extern char *container_bits[];
extern char *herb_bits[];
extern char *spells[];
extern struct board_info_type board_info[];
extern struct descriptor_data *descriptor_list;
extern char *affected_bits[];

/*------------------------------------------------------------------------*/

/*
 * Handy macros.
 */
#define S_PRODUCT(s, i) ((s)->producing[(i)])

/*------------------------------------------------------------------------*/

void oedit_disp_container_flags_menu(struct descriptor_data *d);
void oedit_disp_herb_menu(struct descriptor_data *d);
void oedit_disp_extradesc_menu(struct descriptor_data *d);
void oedit_disp_weapon_menu(struct descriptor_data *d);
void oedit_disp_val1_menu(struct descriptor_data *d);
void oedit_disp_val2_menu(struct descriptor_data *d);
void oedit_disp_val3_menu(struct descriptor_data *d);
void oedit_disp_val4_menu(struct descriptor_data *d);
void oedit_disp_type_menu(struct descriptor_data *d);
void oedit_disp_extra_menu(struct descriptor_data *d);
void oedit_disp_wear_menu(struct descriptor_data *d);
void oedit_disp_menu(struct descriptor_data *d);
void oedit_parse(struct descriptor_data *d, char *arg);
void oedit_disp_spells_menu(struct descriptor_data *d);
void oedit_liquid_type(struct descriptor_data *d);
void oedit_setup_new(struct descriptor_data *d);
void oedit_setup_existing(struct descriptor_data *d, int real_num);
void oedit_save_to_disk(int zone);
void oedit_save_internally(struct descriptor_data *d);
void oedit_disp_affection_list(struct descriptor_data *d);
void oedit_calculate_cost(struct descriptor_data *d);
void oedit_calculate_cost_applies(struct descriptor_data *d);
int oedit_is_manual_cost(struct obj_data *obj);

/*------------------------------------------------------------------------*\
  Utility and exported functions
\*------------------------------------------------------------------------*/

void oedit_setup_new(struct descriptor_data *d)
{
  CREATE(OLC_OBJ(d), struct obj_data, 1);

  clear_object(OLC_OBJ(d));
  OLC_OBJ(d)->name = str_dup("unfinished object");
  OLC_OBJ(d)->description = str_dup("An unfinished object is lying here.");
  OLC_OBJ(d)->short_description = str_dup("an unfinished object");
  SET_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), ITEM_WEAR_TAKE);
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  oedit_disp_menu(d);
  OLC_OBJ(d)->obj_flags.minlevel = 1;
  OLC_OBJ(d)->obj_flags.clan = -1;
  OLC_OBJ(d)->obj_flags.player = -1;
  OLC_OBJ(d)->obj_flags.vamp_wear = -1;
  OLC_OBJ(d)->obj_flags.titan_wear = -1;
  OLC_OBJ(d)->obj_flags.saint_wear = -1;
  OLC_OBJ(d)->obj_flags.demon_wear = -1;
}

/*------------------------------------------------------------------------*/

void oedit_setup_existing(struct descriptor_data *d, int real_num)
{
  struct extra_descr_data *this, *temp, *temp2;
  struct obj_data *obj;

  /*
   * Allocate object in memory.
   */
  CREATE(obj, struct obj_data, 1);

  clear_object(obj);
  *obj = obj_proto[real_num];

  /*
   * Copy all strings over.
   */
  obj->name = str_dup(obj_proto[real_num].name ? obj_proto[real_num].name : "undefined");
  obj->short_description = str_dup(obj_proto[real_num].short_description ?
		obj_proto[real_num].short_description : "undefined");
  obj->description = str_dup(obj_proto[real_num].description ?
		obj_proto[real_num].description : "undefined");
  obj->action_description = (obj_proto[real_num].action_description ?
		str_dup(obj_proto[real_num].action_description) : NULL);

  /*
   * Extra descriptions if necessary.
   */
  if (obj_proto[real_num].ex_description) {
    CREATE(temp, struct extra_descr_data, 1);

    obj->ex_description = temp;
    for (this = obj_proto[real_num].ex_description; this; this = this->next) {
      temp->keyword = (this->keyword && *this->keyword) ? str_dup(this->keyword) : NULL;
      temp->description = (this->description && *this->description) ?
		str_dup(this->description) : NULL;
      if (this->next) {
	CREATE(temp2, struct extra_descr_data, 1);
	temp->next = temp2;
	temp = temp2;
      } else
	temp->next = NULL;
    }
  }
  if (SCRIPT(obj))
    script_copy(obj, &obj_proto[real_num], OBJ_TRIGGER);
  /*
   * Attach new object to player's descriptor.
   */
  OLC_OBJ(d) = obj;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = OBJ_TRIGGER;
  dg_olc_script_copy(d);
  oedit_disp_menu(d);
}

/*------------------------------------------------------------------------*/

#define ZCMD zone_table[zone].cmd[cmd_no]

void oedit_save_internally(struct descriptor_data *d)
{
  int i, shop, robj_num, found = FALSE, zone, cmd_no;
  struct extra_descr_data *this, *next_one;
  struct obj_data *obj, *swap, *new_obj_proto;
  struct index_data *new_obj_index;
  struct descriptor_data *dsc;

  /*
   * Write object to internal tables.
   */
  if ((robj_num = real_object(OLC_NUM(d))) > 0) {
    /*
     * We need to run through each and every object currently in the
     * game to see which ones are pointing to this prototype.
     * if object is pointing to this prototype, then we need to replace it
     * with the new one.
     */
    CREATE(swap, struct obj_data, 1);

    for (obj = object_list; obj; obj = obj->next) {
      if (obj->item_number == robj_num) {
	*swap = *obj;
	*obj = *OLC_OBJ(d);
	/*
	 * Copy game-time dependent variables over.
	 */
	obj->in_room = swap->in_room;
	obj->item_number = robj_num;
	obj->carried_by = swap->carried_by;
	obj->worn_by = swap->worn_by;
	obj->worn_on = swap->worn_on;
	obj->in_obj = swap->in_obj;
	obj->contains = swap->contains;
	obj->next_content = swap->next_content;
	obj->next = swap->next;
      }
    }
    free_obj(swap);
    /*
     * It is now safe to free the old prototype and write over it.
     */
    if (obj_proto[robj_num].name)
      free(obj_proto[robj_num].name);
    if (obj_proto[robj_num].description)
      free(obj_proto[robj_num].description);
    if (obj_proto[robj_num].short_description)
      free(obj_proto[robj_num].short_description);
    if (obj_proto[robj_num].action_description)
      free(obj_proto[robj_num].action_description);
    if (obj_proto[robj_num].ex_description)
      for (this = obj_proto[robj_num].ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
    obj_proto[robj_num] = *OLC_OBJ(d);
    obj_proto[robj_num].item_number = robj_num;
  } else {
    /*
     * It's a new object, we must build new tables to contain it.
     */
    CREATE(new_obj_index, struct index_data, top_of_objt + 2);
    CREATE(new_obj_proto, struct obj_data, top_of_objt + 2);

    /*
     * Start counting through both tables.
     */
    for (i = 0; i <= top_of_objt; i++) {
      /*
       * If we haven't found it.
       */
      if (!found) {
	/*
	 * Check if current virtual is bigger than our virtual number.
	 */
	if (obj_index[i].virtual > OLC_NUM(d)) {
	  found = TRUE;
	  robj_num = i;
	  OLC_OBJ(d)->item_number = robj_num;
	  new_obj_index[robj_num].virtual = OLC_NUM(d);
	  new_obj_index[robj_num].number = 0;
	  new_obj_index[robj_num].func = NULL;
	  new_obj_proto[robj_num] = *(OLC_OBJ(d));
          new_obj_proto[robj_num].proto_script = OLC_SCRIPT(d);
	  new_obj_proto[robj_num].in_room = NOWHERE;

	  /*
	   * Copy over the mob that should be here.
	   */
	  new_obj_index[robj_num + 1] = obj_index[robj_num];
	  new_obj_proto[robj_num + 1] = obj_proto[robj_num];
	  new_obj_proto[robj_num + 1].item_number = robj_num + 1;
	} else {
	  /*
	   * Just copy from old to new, no number change.
	   */
	  new_obj_proto[i] = obj_proto[i];
	  new_obj_index[i] = obj_index[i];
	}
      } else {
	/*
	 * We HAVE already found it, therefore copy to object + 1 
	 */
	new_obj_index[i + 1] = obj_index[i];
	new_obj_proto[i + 1] = obj_proto[i];
	new_obj_proto[i + 1].item_number = i + 1;
      }
    }
    if (!found) {
      robj_num = i;
      OLC_OBJ(d)->item_number = robj_num;
      new_obj_index[robj_num].virtual = OLC_NUM(d);
      new_obj_index[robj_num].number = 0;
      new_obj_index[robj_num].func = NULL;
      new_obj_proto[robj_num] = *(OLC_OBJ(d));
      new_obj_proto[robj_num].in_room = NOWHERE;
    }
    /*
     * Free and replace old tables.
     */
    free(obj_proto);
    free(obj_index);
    obj_proto = new_obj_proto;
    obj_index = new_obj_index;
    top_of_objt++;

    /*
     * Renumber live objects.
     */
    for (obj = object_list; obj; obj = obj->next)
      if (GET_OBJ_RNUM(obj) >= robj_num)
	GET_OBJ_RNUM(obj)++;

    /*
     * Renumber zone table.
     */
    for (zone = 0; zone <= top_of_zone_table; zone++)
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
	switch (ZCMD.command) {
	case 'P':
	  if (ZCMD.arg3 >= robj_num)
	    ZCMD.arg3++;
	  /*
	   * No break here - drop into next case.
	   */
	case 'O':
	case 'G':
	case 'E':
	  if (ZCMD.arg1 >= robj_num)
	    ZCMD.arg1++;
	  break;
	case 'R':
	  if (ZCMD.arg2 >= robj_num)
	    ZCMD.arg2++;
	  break;
	}

    /*
     * Renumber notice boards.
     */
    for (i = 0; i < NUM_OF_BOARDS; i++)
      if (BOARD_RNUM(i) >= robj_num)
	BOARD_RNUM(i) = BOARD_RNUM(i) + 1;

    /*
     * Renumber shop produce.
     */
    for (shop = 0; shop < top_shop; shop++)
      for (i = 0; SHOP_PRODUCT(shop, i) != -1; i++)
	if (SHOP_PRODUCT(shop, i) >= robj_num)
	  SHOP_PRODUCT(shop, i)++;

    /*
     * Renumber produce in shops being edited.
     */
    for (dsc = descriptor_list; dsc; dsc = dsc->next)
      if (dsc->connected == CON_SEDIT)
	for (i = 0; S_PRODUCT(OLC_SHOP(dsc), i) != -1; i++)
	  if (S_PRODUCT(OLC_SHOP(dsc), i) >= robj_num)
	    S_PRODUCT(OLC_SHOP(dsc), i)++;

  }
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_OBJ);
}

/*------------------------------------------------------------------------*/

void oedit_save_to_disk(int zone_num)
{
  int counter, counter2, realcounter;
  FILE *fp;
  struct obj_data *obj;
  struct extra_descr_data *ex_desc;

  sprintf(buf, "%s/%d.new", OBJ_PREFIX, zone_table[zone_num].number);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog("SYSERR: OLC: Cannot open objects file!", BRF, LVL_BUILDER, TRUE);
    return;
  }
  /*
   * Start running through all objects in this zone.
   */
  for (counter = zone_table[zone_num].number * 100;
       counter <= zone_table[zone_num].top; counter++) {
    if ((realcounter = real_object(counter)) >= 0) {
      if ((obj = (obj_proto + realcounter))->action_description) {
	strcpy(buf1, obj->action_description);
	strip_string(buf1);
      } else
	*buf1 = '\0';

      fprintf(fp,
	      "#%d\n"
	      "%s~\n"
	      "%s~\n"
	      "%s~\n"
	      "%s~\n"
	      "%d %d %d %d %d %d %d %d %d\n"
	      "%d %d %d %d %d %d %d %d\n"
	      "%d %d %d %d %d %d %d %d %d %d\n"
              "%d %d %d 0 0 0 0 0 0\n",
	      GET_OBJ_VNUM(obj),
	      (obj->name && *obj->name) ? obj->name : "undefined",
	      (obj->short_description && *obj->short_description) ?
			obj->short_description : "undefined",
	      (obj->description && *obj->description) ?
			obj->description : "undefined",
	      buf1, GET_OBJ_TYPE(obj), GET_OBJ_EXTRA(obj)[0],
              GET_OBJ_EXTRA(obj)[1], GET_OBJ_EXTRA(obj)[2],
              GET_OBJ_EXTRA(obj)[3], GET_OBJ_WEAR(obj)[0],
              GET_OBJ_WEAR(obj)[1], GET_OBJ_WEAR(obj)[2],
              GET_OBJ_WEAR(obj)[3],
	      GET_OBJ_VAL(obj, 0), GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2),
	      GET_OBJ_VAL(obj, 3), obj->obj_flags.bitvector[0],
              obj->obj_flags.bitvector[1], obj->obj_flags.bitvector[2],
              obj->obj_flags.bitvector[3],
              GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj),
	      GET_OBJ_RENT(obj), obj->obj_flags.minlevel,
              GET_OBJ_CLAN(obj), GET_OBJ_PLAYER(obj),
              GET_OBJ_VAMP(obj), GET_OBJ_TITAN(obj),
              GET_OBJ_SAINT(obj), GET_OBJ_DEMON(obj),
              GET_OBJ_TIMER(obj), GET_OBJ_MATERIAL(obj),
              obj->obj_flags.racial_bonus
	      );

      script_save_to_disk(fp, obj, OBJ_TRIGGER);

      /*
       * Do we have extra descriptions? 
       */
      if (obj->ex_description) {	/* Yes, save them too. */
	for (ex_desc = obj->ex_description; ex_desc; ex_desc = ex_desc->next) {
	  /*
	   * Sanity check to prevent nasty protection faults.
	   */
	  if (!*ex_desc->keyword || !*ex_desc->description) {
	    mudlog("SYSERR: OLC: oedit_save_to_disk: Corrupt ex_desc!", BRF, LVL_BUILDER, TRUE);
	    continue;
	  }
	  strcpy(buf1, ex_desc->description);
	  strip_string(buf1);
	  fprintf(fp, "E\n"
		  "%s~\n"
		  "%s~\n", ex_desc->keyword, buf1);
	}
      }
      /*
       * Do we have affects? 
       */
      for (counter2 = 0; counter2 < MAX_OBJ_AFFECT; counter2++)
	if (obj->affected[counter2].modifier)
	  fprintf(fp, "A\n"
		  "%d %d\n", obj->affected[counter2].location,
		  obj->affected[counter2].modifier);
      /*
       * Persistent spell? 
       */
      if (GET_OBJ_SPELL(obj))
	  fprintf(fp, "C\n"
		  "%d %d\n", GET_OBJ_SPELL(obj), GET_OBJ_SPELL_EXTRA(obj));
    }
  }

  /*
   * Write the final line, close the file.
   */
  fprintf(fp, "$~\n");
  fclose(fp);
  sprintf(buf2, "%s/%d.obj", OBJ_PREFIX, zone_table[zone_num].number);
  /*
   * We're fubar'd if we crash between the two lines below.
   */
  remove(buf2);
  rename(buf, buf2);

  olc_remove_from_save_list(zone_table[zone_num].number, OLC_SAVE_OBJ);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/*
 * For container flags.
 */
void oedit_disp_container_flags_menu(struct descriptor_data *d)
{
  get_char_cols(d->character);
  sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 1), container_bits, buf1);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  sprintf(buf,
	  "%s1%s) CLOSEABLE\r\n"
	  "%s2%s) PICKPROOF\r\n"
	  "%s3%s) CLOSED\r\n"
	  "%s4%s) LOCKED\r\n"
	  "Container flags: %s%s%s\r\n"
	  "Enter flag, 0 to quit : ",
	  grn, nrm, grn, nrm, grn, nrm, grn, nrm, cyn, buf1, nrm);
  send_to_char(buf, d->character);
}
/* For herbs */
void oedit_disp_herb_menu(struct descriptor_data *d)
{
  get_char_cols(d->character);
  sprintbit(GET_OBJ_VAL(OLC_OBJ(d), 0), herb_bits, buf1);
  
  sprintf(buf,
          "%s1%s) ACONITE\r\n"
	  "%s2%s) CALEDONIA\r\n"
	  "%s3%s) GOLDENSEAL\r\n"
	  "%s4%s) WILDROOT\r\n"
	  "%s5%s) NIRINBARK\r\n"
	  "%s6%s) COLTSFOOT\r\n"
	  "%s7%s) BURDOCK\r\n"
	  "%s8%s) ANGELICA\r\n"
	  "%s9%s) SALSTINE\r\n"
	  "%s10%s) BAYBERRY\r\n"
          "Choose only one: %s%s%s\r\n"
          "Enter herb #, 0 to quit : ",
 	  grn, nrm, grn, nrm, grn, nrm, grn, nrm, grn, nrm,
 	  grn, nrm, grn, nrm, grn, nrm, grn, nrm, grn, nrm,
	  cyn, buf1, nrm);
   send_to_char(buf, d->character); 
}

/*
 * For extra descriptions.
 */
void oedit_disp_extradesc_menu(struct descriptor_data *d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);

  strcpy(buf1, !extra_desc->next ? "<Not set>\r\n" : "Set.");

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  sprintf(buf,
	  "Extra desc menu\r\n"
	  "%s1%s) Keyword: %s%s\r\n"
	  "%s2%s) Description:\r\n%s%s\r\n"
	  "%s3%s) Goto next description: %s\r\n"
	  "%s0%s) Quit\r\n"
	  "Enter choice : ",

     	  grn, nrm, yel, (extra_desc->keyword && *extra_desc->keyword) ? extra_desc->keyword : "<NONE>",
	  grn, nrm, yel, (extra_desc->description && *extra_desc->description) ? extra_desc->description : "<NONE>",
	  grn, nrm, buf1, grn, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_EXTRADESC_MENU;
}

/*
 * Ask for *which* apply to edit.
 */
void oedit_disp_prompt_apply_menu(struct descriptor_data *d)
{
  int counter;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < MAX_OBJ_AFFECT; counter++) {
    if (OLC_OBJ(d)->affected[counter].modifier) {
      sprinttype(OLC_OBJ(d)->affected[counter].location, apply_types, buf2);
      sprintf(buf, " %s%d%s) %+d to %s\r\n", grn, counter + 1, nrm,
	      OLC_OBJ(d)->affected[counter].modifier, buf2);
      send_to_char(buf, d->character);
    } else {
      sprintf(buf, " %s%d%s) None.\r\n", grn, counter + 1, nrm);
      send_to_char(buf, d->character);
    }
  }
  send_to_char("\r\nEnter affection to modify (0 to quit) : ", d->character);
  OLC_MODE(d) = OEDIT_PROMPT_APPLY;
}

/*
 * Ask for liquid type.
 */
void oedit_liquid_type(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < NUM_LIQ_TYPES; counter++) {
    sprintf(buf, " %s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
	    drinks[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter drink type : ", nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_VALUE_3;
}

/*
 * The actual apply to set.
 */
void oedit_disp_apply_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < NUM_APPLIES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		apply_types[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter apply type (0 is no apply) : ", d->character);
  OLC_MODE(d) = OEDIT_APPLY;
}

/*
 * Weapon type.
 */
void oedit_disp_weapon_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < NUM_ATTACK_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		attack_hit_text[counter].singular,
		!(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter weapon type : ", d->character);
}

/*
 * Spell type.
 */
void oedit_disp_spells_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 1; counter < NUM_SPELLS; counter++) {
    sprintf(buf, "%s%2d%s) %s%-20.20s %s", grn, counter, nrm, yel,
		spells[counter], !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintf(buf, "\r\n%sEnter spell choice (0 for none) : ", nrm);
  send_to_char(buf, d->character);
}

void oedit_disp_spells_wpn_menu(struct descriptor_data *d)
{
   extern struct abil_info_type abil_info[5][MAX_ABILITIES+1];
  //struct abil_info_type ab;
  int counter, columns = 0;
  //ab = abil_info[(int)ABT_SPELL][counter];

  get_char_cols(d->character);
  for (counter = 1; counter < MAX_ABILITIES; counter++) {
	  if (IS_SET(abil_info[ABT_SPELL][counter].routines, MAG_DAMAGE) || IS_SET(abil_info[ABT_SPELL][counter].routines, MAG_AREAS)) {
		  sprintf(buf, "%s%2d%s) %s%-20.20s %s", grn, counter,
			  nrm, yel, spells[counter], !(++columns % 3) ? "\r\n" : "");
		  send_to_char(buf, d->character);
	  }
  }
  sprintf(buf, "\r\n%sEnter weapon spell choice (0 for none) : ", nrm);
  send_to_char(buf, d->character);
}

/*
 * Object value #1
 */
void oedit_disp_val1_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_1;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    /*
     * values 0 and 1 are unused.. jump to 2 
     */
    oedit_disp_val3_menu(d);
    break;
  case ITEM_SCROLL:
  case ITEM_WAND:
  case ITEM_STAFF:
  case ITEM_POTION:
    send_to_char("Spell level : ", d->character);
    break;
  case ITEM_WEAPON:
    /*
     * This doesn't seem to be used if I remembe right.
     */
    oedit_disp_val2_menu(d);
    break;
  case ITEM_ARMOR:
    send_to_char("Apply to AC : ", d->character);
    break;
  case ITEM_PIECE:
    send_to_char("Vnum of connector obj : ", d->character);
    break;
  case ITEM_CONTAINER:
    send_to_char("Max weight to contain : ", d->character);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    send_to_char("Max drink units : ", d->character);
    break;
  case ITEM_FOOD:
    send_to_char("Hours to fill stomach : ", d->character);
    break;
  case ITEM_MONEY:
    send_to_char("Number of gold coins : ", d->character);
    break;
  case ITEM_DEED:
    send_to_char("Zone Number : ", d->character);
    break;
  case ITEM_NOTE:
    /*
     * This is supposed to be language, but it's unused.
     */
    break;
  case ITEM_STONE:
    send_to_char("How many times can it be used to hone?: ", d->character);
    break;
  case ITEM_PORTAL:
    send_to_char("Portal to which room?: ", d->character);
    break;
  case ITEM_HERB:
    oedit_disp_herb_menu(d);
   break;
  case ITEM_SHIP:
    send_to_char("Enter type of ship. 0 for water, 1 for air : ", d->character);
    break;
  case ITEM_TILLER:
    send_to_char("Enter Vnum of your ship object : ", d->character);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #2
 */
void oedit_disp_val2_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_2;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char("Max number of charges : ", d->character);
    break;
  case ITEM_WEAPON:
    oedit_disp_val4_menu(d);
    break;
  case ITEM_PIECE:
    send_to_char("Vnum of Final Object : ", d->character);
    break;
  case ITEM_FOOD:
    /*
     * Values 2 and 3 are unused, jump to 4...Odd.
     */
    oedit_disp_val4_menu(d);
    break;
  case ITEM_CONTAINER:
    /*
     * These are flags, needs a bit of special handling.
     */
    oedit_disp_container_flags_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    send_to_char("Initial drink units : ", d->character);
    break;
  case ITEM_SHIP:
    send_to_char("Enter vnum of room players board : ", d->character);
    break;
  case ITEM_TILLER:
    send_to_char("Vnum of first ship room : ", d->character);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #3
 */
void oedit_disp_val3_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_3;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_LIGHT:
    send_to_char("Number of hours (0 = burnt, -1 is infinite) : ", d->character);
    break;
  case ITEM_SCROLL:
  case ITEM_POTION:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WAND:
  case ITEM_STAFF:
    send_to_char("Number of charges remaining : ", d->character);
    break;
  case ITEM_CONTAINER:
    send_to_char("Vnum of key to open container (-1 for no key) : ", d->character);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
    oedit_liquid_type(d);
    break;
  case ITEM_TILLER:
    send_to_char("Vnum of last ship room : ", d->character);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object value #4
 */
void oedit_disp_val4_menu(struct descriptor_data *d)
{
  OLC_MODE(d) = OEDIT_VALUE_4;
  switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_WAND:
  case ITEM_STAFF:
    oedit_disp_spells_menu(d);
    break;
  case ITEM_WEAPON:
    oedit_disp_weapon_menu(d);
    break;
  case ITEM_DRINKCON:
  case ITEM_FOUNTAIN:
  case ITEM_FOOD:
    send_to_char("Poisoned (0 = not poison) : ", d->character);
    break;
  default:
    oedit_disp_menu(d);
  }
}

/*
 * Object type.
 */
void oedit_disp_type_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 1; counter <= NUM_ITEM_TYPES; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		item_types[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter object type : ", d->character);
}

/*
 * Object extra flags.
 */
void oedit_disp_extra_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < NUM_ITEM_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		extra_bits[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_EXTRA(OLC_OBJ(d)), extra_bits, EF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nObject flags: %s%s%s\r\n"
	  "Enter object extra flag (0 to quit) : ",
	  cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

/*
 * Object wear flags.
 */
void oedit_disp_wear_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_cols(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (counter = 0; counter < NUM_ITEM_WEARS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
		wear_bits[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(GET_OBJ_WEAR(OLC_OBJ(d)), wear_bits, TW_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nWear flags: %s%s%s\r\n"
	  "Enter wear flag, 0 to quit : ", cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

void oedit_disp_affection_list(struct descriptor_data *d) {
  int counter, columns=0;

  for (counter = 1; counter < NUM_AFF_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter, nrm,
		affected_bits[counter], !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
}


/*
 * Display main menu.
 */
void oedit_disp_menu(struct descriptor_data *d)
{
  struct obj_data *obj;

  obj = OLC_OBJ(d);
  get_char_cols(d->character);

  /*
   * Build buffers for first part of menu.
   */
  sprinttype(GET_OBJ_TYPE(obj), item_types, buf1);
  sprintbitarray(GET_OBJ_EXTRA(obj), extra_bits, EF_ARRAY_MAX, buf2);

  /*
   * Build first half of menu.
   */
  sprintf(buf,
#if defined(CLEAR_SCREEN)
	  "[H[J"
#endif
	  "-- Item number : [%s%d%s]\r\n"
	  "%s1%s) Namelist : %s%s\r\n"
	  "%s2%s) S-Desc   : /cw%s%s\r\n"
	  "%s3%s) L-Desc   :-/cw\r\n%s%s\r\n"
	  "%s4%s) A-Desc   :-\r\n%s%s"
	  "%s5%s) Type        : %s%s\r\n"
	  "%s6%s) Extra flags : %s%s\r\n",

	  cyn, OLC_NUM(d), nrm,
	  grn, nrm, yel, (obj->name && *obj->name) ? obj->name : "undefined",
	  grn, nrm, yel, (obj->short_description && *obj->short_description) ? obj->short_description : "undefined",
	  grn, nrm, yel, (obj->description && *obj->description) ? obj->description : "undefined",
	  grn, nrm, yel, (obj->action_description && *obj->action_description) ? obj->action_description : "<only used for potions, scrolls, staves and wands>\r\n",
	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, buf2
	  );
  /*
   * Send first half.
   */
  send_to_char(buf, d->character);

  /*
   * Build second half of menu.
   */
  *buf2 = '\0';
  sprintbitarray(OLC_OBJ(d)->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX,  buf2);
  sprintbitarray(GET_OBJ_WEAR(obj), wear_bits, TW_ARRAY_MAX, buf1);
  sprintf(buf,
	  "%s7%s) Wear flags  : %s%s\r\n"
	  "%s8%s) Weight      : %s%d\r\n"
	  "%s9%s) Cost        : %s%d\r\n"
	  "%sA%s) Cost/Day    : %s%d\r\n"
	  "%sF%s) Min Level   : %s%d\r\n"
	  "%sG%s) Spell (mod) : %s%d (%d)\r\n"
	  "%sH%s) Affections  : %s%s\r\n"
	  "%sB%s) Timer       : %s%d\r\n"
          "%sI%s) Clan        : %s%d\r\n"
          "%sP%s) Player      : %s%d\r\n"
          "%sV%s) Vamp_wear   : %s%d\r\n"
          "%sT%s) Titan_wear  : %s%d\r\n"
          "%sN%s) Saint_wear  : %s%d\r\n"
          "%sM%s) Demon _wear : %s%d\r\n"
	  "%sC%s) Values      : %s%d %d %d %d\r\n"
	  "%sD%s) Applies menu\r\n"
	  "%sE%s) Extra descriptions menu\r\n"
          "%sS%s) Script      : %s%s\r\n"
	  "%sQ%s) Quit\r\n"
	  "Enter choice : ",

	  grn, nrm, cyn, buf1,
	  grn, nrm, cyn, GET_OBJ_WEIGHT(obj),
	  grn, nrm, cyn, GET_OBJ_COST(obj),
	  grn, nrm, cyn, GET_OBJ_RENT(obj),
	  grn, nrm, cyn, obj->obj_flags.minlevel,
	  grn, nrm, cyn, GET_OBJ_SPELL(obj), GET_OBJ_SPELL_EXTRA(obj),
	  grn, nrm, cyn, buf2,
	  grn, nrm, cyn, GET_OBJ_TIMER(obj),
	  grn, nrm, cyn, GET_OBJ_CLAN(obj),
	  grn, nrm, cyn, GET_OBJ_PLAYER(obj),
	  grn, nrm, cyn, GET_OBJ_VAMP(obj),
	  grn, nrm, cyn, GET_OBJ_TITAN(obj),
	  grn, nrm, cyn, GET_OBJ_SAINT(obj),
	  grn, nrm, cyn, GET_OBJ_DEMON(obj),
	  grn, nrm, cyn, GET_OBJ_VAL(obj, 0),
	  GET_OBJ_VAL(obj, 1),
	  GET_OBJ_VAL(obj, 2),
	  GET_OBJ_VAL(obj, 3),
	  grn, nrm, grn, nrm,
          grn, nrm, cyn, obj->proto_script?"Set.":"Not Set.",
	  grn, nrm
	  );
  send_to_char(buf, d->character);
  OLC_MODE(d) = OEDIT_MAIN_MENU;
}

/***************************************************************************
 main loop (of sorts).. basically interpreter throws all input to here
 ***************************************************************************/

void oedit_parse(struct descriptor_data *d, char *arg)
{
  int number, max_val, min_val, i=0;
  switch (OLC_MODE(d)) {

  case OEDIT_CONFIRM_SAVESTRING:
    switch (*arg) {
    case 'y':
    case 'Y':
      send_to_char("Saving object to memory.\r\n", d->character);
      oedit_save_internally(d);
      sprintf(buf, "OLC: %s edits obj %d", GET_NAME(d->character), OLC_NUM(d));
      mudlog(buf, CMP, MAX(LVL_BUILDER, GET_INVIS_LEV(d->character)), TRUE);
      cleanup_olc(d, CLEANUP_STRUCTS);
      return;
    case 'n':
    case 'N':
      /*
       * Cleanup all.
       */
      cleanup_olc(d, CLEANUP_ALL);
      return;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save this object internally?\r\n", d->character);
      return;
    }

  case OEDIT_MAIN_MENU:
    /*
     * Throw us out to whichever edit mode based on user input.
     */
    switch (*arg) {
    case 'q':
    case 'Q':
		if (!oedit_is_manual_cost(OLC_OBJ(d)))
		{
			GET_OBJ_COST(OLC_OBJ(d)) = 0;
			oedit_calculate_cost(d);
		}
      if (OLC_VAL(d)) {	/* Something has been modified. */
	send_to_char("Do you wish to save this object internally? : ", d->character);
	OLC_MODE(d) = OEDIT_CONFIRM_SAVESTRING;
      } else
	cleanup_olc(d, CLEANUP_ALL);
      return;
    case '1':
      send_to_char("Enter namelist : ", d->character);
      OLC_MODE(d) = OEDIT_EDIT_NAMELIST;
      break;
    case '2':
      send_to_char("Enter short desc : ", d->character);
      OLC_MODE(d) = OEDIT_SHORTDESC;
      break;
    case '3':
      send_to_char("Enter long desc :-\r\n| ", d->character);
      OLC_MODE(d) = OEDIT_LONGDESC;
      break;
    case '4':
      OLC_MODE(d) = OEDIT_ACTDESC;
      SEND_TO_Q("Enter action description: (/s saves /h for help)\r\n\r\n", d);
      d->backstr = NULL;
      if (OLC_OBJ(d)->action_description) {
	SEND_TO_Q(OLC_OBJ(d)->action_description, d);
	d->backstr = str_dup(OLC_OBJ(d)->action_description);
      }
      d->str = &OLC_OBJ(d)->action_description;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      break;
    case '5':
      oedit_disp_type_menu(d);
      OLC_MODE(d) = OEDIT_TYPE;
      break;
    case '6':
      oedit_disp_extra_menu(d);
      OLC_MODE(d) = OEDIT_EXTRAS;
      break;
    case '7':
      oedit_disp_wear_menu(d);
      OLC_MODE(d) = OEDIT_WEAR;
      break;
    case '8':
      send_to_char("Enter weight : ", d->character);
      OLC_MODE(d) = OEDIT_WEIGHT;
      break;
	case '9':
		GET_OBJ_COST(OLC_OBJ(d)) = 0;
		if (oedit_is_manual_cost(OLC_OBJ(d)))
		{
			OLC_VAL(d) = 1;
			send_to_char("Enter cost : ", d->character);
			OLC_MODE(d) = OEDIT_COST;
		} else {
			oedit_calculate_cost(d);
			OLC_VAL(d) = 1;
			oedit_disp_menu(d);
		}
      break;
	  /*
    case '9':
      send_to_char("Enter cost : ", d->character);
      OLC_MODE(d) = OEDIT_COST;
      break;
	  */
    case 'a':
    case 'A':
      send_to_char("Enter cost per day : ", d->character);
      OLC_MODE(d) = OEDIT_COSTPERDAY;
      break;
    case 'b':
    case 'B':
      send_to_char("Enter timer : ", d->character);
      OLC_MODE(d) = OEDIT_TIMER;
      break;
    case 'i':
    case 'I':

    if(num_of_clans == 0) {
      send_to_char("No clans have formed yet.\r\n", d->character);
      return; }

    sprintf(buf, "\r");
    for(i=0; i < num_of_clans; i++)
      sprintf(buf, "%s[%-3d]  %-20s\r\n",buf, i, clan[i].name);
      page_string(d->character->desc,buf, 1);
      send_to_char("\r\nChoose a clan : ", d->character);
      OLC_MODE(d) = OEDIT_CLAN;
      break;
    case 'p':
    case 'P':

      send_to_char("\r\nEnter player idnum : ", d->character);
      OLC_MODE(d) = OEDIT_PLAYER;
      break;

    case 'v':
    case 'V':
      send_to_char("Enter vampire wear # (0 is none) : ", d->character);
      OLC_MODE(d) = OEDIT_VAMP_WEAR;
      break;

    case 't':
    case 'T':
      send_to_char("Enter titan wear # (0 is none) : ", d->character);
      OLC_MODE(d) = OEDIT_TITAN_WEAR;
      break;

    case 'n':
    case 'N':
      send_to_char("Enter saint wear # (0 is none) : ", d->character);
      OLC_MODE(d) = OEDIT_SAINT_WEAR;
      break;

    case 'm':
    case 'M':
      send_to_char("Enter demon wear # (0 is none) : ", d->character);
      OLC_MODE(d) = OEDIT_DEMON_WEAR;
      break;

    case 'c':
    case 'C':
      /*
       * Clear any old values  
       */
      GET_OBJ_VAL(OLC_OBJ(d), 0) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 1) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 2) = 0;
      GET_OBJ_VAL(OLC_OBJ(d), 3) = 0;
      oedit_disp_val1_menu(d);
      break;
    case 'd':
    case 'D':
      oedit_disp_prompt_apply_menu(d);
      break;
    case 'e':
    case 'E':
      /*
       * If extra descriptions don't exist.
       */
      if (!OLC_OBJ(d)->ex_description) {
	CREATE(OLC_OBJ(d)->ex_description, struct extra_descr_data, 1);
	OLC_OBJ(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_OBJ(d)->ex_description;
      oedit_disp_extradesc_menu(d);
      break;
    case 'f':
    case 'F':
      send_to_char("Enter minimum level : ", d->character);
      OLC_MODE(d) = OEDIT_MINLEVEL;
      break;
    case 'g':
    case 'G':
	  oedit_disp_spells_wpn_menu(d);
      send_to_char("Enter weapon spell number : ", d->character);
      OLC_MODE(d) = OEDIT_PERSISTENT_SPELL;
      break;
    case 'h':
    case 'H':
      oedit_disp_affection_list(d);
      sprintbitarray(OLC_OBJ(d)->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
      send_to_char("\r\n", d->character);
      send_to_char(buf, d->character);
      send_to_char("\r\nEnter affection (0 to quit): ", d->character);

      OLC_MODE(d) = OEDIT_AFFECTION;
      break;
    case 's':
    case 'S':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      return;

    default:
      oedit_disp_menu(d);
      break;
    }
    return;			/*
				 * end of OEDIT_MAIN_MENU 
				 */
  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;

  case OEDIT_EDIT_NAMELIST:
    if (OLC_OBJ(d)->name)
      free(OLC_OBJ(d)->name);
    OLC_OBJ(d)->name = str_dup((arg && *arg) ? arg : "undefined");
    break;

  case OEDIT_SHORTDESC:
    if (OLC_OBJ(d)->short_description)
      free(OLC_OBJ(d)->short_description);
    OLC_OBJ(d)->short_description = str_dup((arg && *arg) ? arg : "undefined");
    break;

  case OEDIT_LONGDESC:
    if (OLC_OBJ(d)->description)
      free(OLC_OBJ(d)->description);
    OLC_OBJ(d)->description = str_dup((arg && *arg) ? arg : "undefined");
    break;

  case OEDIT_TYPE:
    number = atoi(arg);
    if ((number < 1) || (number > NUM_ITEM_TYPES)) {
      send_to_char("Invalid choice, try again : ", d->character);
      return;
    } else {
      GET_OBJ_TYPE(OLC_OBJ(d)) = number;
	  if (!oedit_is_manual_cost(OLC_OBJ(d)))
		  oedit_calculate_cost(d);
	}
    break;

  case OEDIT_EXTRAS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_FLAGS)) {
      oedit_disp_extra_menu(d);
      return;
    } else if (number == 0)
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_EXTRA(OLC_OBJ(d)), number-1);
      oedit_disp_extra_menu(d);
      return;
    }

  case OEDIT_WEAR:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ITEM_WEARS)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      oedit_disp_wear_menu(d);
      return;
    } else if (number == 0)	/* Quit. */
      break;
    else {
      TOGGLE_BIT_AR(GET_OBJ_WEAR(OLC_OBJ(d)), number-1);
      oedit_disp_wear_menu(d);
      return;
    }

  case OEDIT_WEIGHT:
    GET_OBJ_WEIGHT(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_COST:
	 GET_OBJ_COST(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_COSTPERDAY:
    GET_OBJ_RENT(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_TIMER:
    GET_OBJ_TIMER(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_CLAN:
    GET_OBJ_CLAN(OLC_OBJ(d)) = MIN(atoi(arg), num_of_clans);
    break;

  case OEDIT_PLAYER:
    GET_OBJ_PLAYER(OLC_OBJ(d)) = atoi(arg);
    break;

  case OEDIT_VAMP_WEAR:
    GET_OBJ_VAMP(OLC_OBJ(d)) = MIN(atoi(arg), 7);
    break;

  case OEDIT_TITAN_WEAR:
    GET_OBJ_TITAN(OLC_OBJ(d)) = MIN(atoi(arg), 7);
    break;

  case OEDIT_SAINT_WEAR:
    GET_OBJ_SAINT(OLC_OBJ(d)) = MIN(atoi(arg), 7);
    break;

  case OEDIT_DEMON_WEAR:
    GET_OBJ_DEMON(OLC_OBJ(d)) = MIN(atoi(arg), 7);
    break;


  case OEDIT_MINLEVEL:
    OLC_OBJ(d)->obj_flags.minlevel = MIN(atoi(arg), 160);
    break;

  case OEDIT_PERSISTENT_SPELL:
    OLC_OBJ(d)->obj_flags.spell = atoi(arg);
    send_to_char("Enter the chance per round (0-8): ", d->character);
    OLC_MODE(d) = OEDIT_PERSISTENT_SPELL_EXTRA;
    return;

  case OEDIT_AFFECTION:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_AFF_FLAGS-1)) {
      send_to_char("That is not a valid choice!\r\n", d->character);
    } else if (number == 0)
        break;
    else {
      /*
       * Toggle the bit.
       */
      TOGGLE_BIT_AR(OLC_OBJ(d)->obj_flags.bitvector, number);
      oedit_disp_affection_list(d);
      sprintbitarray(OLC_OBJ(d)->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf);
      send_to_char("\r\n", d->character);
      send_to_char(buf, d->character);
      send_to_char("\r\nEnter affection (0 to quit): ", d->character);
    }
    return;

  case OEDIT_PERSISTENT_SPELL_EXTRA:
    OLC_OBJ(d)->obj_flags.spell_extra = MIN(atoi(arg), 8);
    break;

  case OEDIT_VALUE_1:
    /*
     * Lucky, I don't need to check any of these for out of range values.
     * Hmm, I'm not so sure - Rv  
     */
    GET_OBJ_VAL(OLC_OBJ(d), 0) = atoi(arg);
    /*
     * proceed to menu 2 
     */
    oedit_disp_val2_menu(d);
    return;
  case OEDIT_VALUE_2:
    /*
     * Here, I do need to check for out of range values.
     */
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      if (number < 0 || number >= NUM_SPELLS)
	oedit_disp_val2_menu(d);
      else {
	GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
	oedit_disp_val3_menu(d);
      }
      break;
    case ITEM_CONTAINER:
      /*
       * Needs some special handling since we are dealing with flag values
       * here.
       */
      if (number < 0 || number > 4)
	oedit_disp_container_flags_menu(d);
      else if (number != 0) {
        TOGGLE_BIT(GET_OBJ_VAL(OLC_OBJ(d), 1), 1 << (number-1));
        OLC_VAL(d) = 1;
	oedit_disp_val2_menu(d);
      } else
	oedit_disp_val3_menu(d);
      break;
    case ITEM_TILLER:
      min_val = 0;
      max_val = 57499;
      break;
    default:
      GET_OBJ_VAL(OLC_OBJ(d), 1) = number;
      oedit_disp_val3_menu(d);
    }
    return;

  case OEDIT_VALUE_3:
    number = atoi(arg);
    /*
     * Quick'n'easy error checking.
     */
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      min_val = 0;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 1;
      max_val = 50;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 0;
      max_val = 20;
      break;
    case ITEM_DRINKCON:
    case ITEM_FOUNTAIN:
      min_val = 0;
      max_val = NUM_LIQ_TYPES - 1;
      break;
    case ITEM_TILLER:
      min_val = 0;
      max_val = 57499;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 2) = MAX(min_val, MIN(number, max_val));
    oedit_disp_val4_menu(d);
    return;

  case OEDIT_VALUE_4:
    number = atoi(arg);
    switch (GET_OBJ_TYPE(OLC_OBJ(d))) {
    case ITEM_SCROLL:
    case ITEM_POTION:
      min_val = 0;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      min_val = 1;
      max_val = NUM_SPELLS - 1;
      break;
    case ITEM_WEAPON:
      min_val = 0;
      max_val = NUM_ATTACK_TYPES - 1;
      break;
    default:
      min_val = -32000;
      max_val = 32000;
      break;
    }
    GET_OBJ_VAL(OLC_OBJ(d), 3) = MAX(min_val, MIN(number, max_val));
    break;

  case OEDIT_PROMPT_APPLY:
    if ((number = atoi(arg)) == 0)
      break;
    else if (number < 0 || number > MAX_OBJ_AFFECT) {
      oedit_disp_prompt_apply_menu(d);
      return;
    }
    OLC_VAL(d) = number - 1;
    OLC_MODE(d) = OEDIT_APPLY;
    oedit_disp_apply_menu(d);
    return;

  case OEDIT_APPLY:
    if ((number = atoi(arg)) == 0) {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = 0;
      OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = 0;
      oedit_disp_prompt_apply_menu(d);
    } else if (number < 0 || number >= NUM_APPLIES)
      oedit_disp_apply_menu(d);
    else {
      OLC_OBJ(d)->affected[OLC_VAL(d)].location = number;
      send_to_char("Modifier : ", d->character);
      OLC_MODE(d) = OEDIT_APPLYMOD;
    }
    return;

  case OEDIT_APPLYMOD:
    OLC_OBJ(d)->affected[OLC_VAL(d)].modifier = atoi(arg);
    oedit_disp_prompt_apply_menu(d);
    return;

  case OEDIT_EXTRADESC_KEY:
    if (OLC_DESC(d)->keyword)
      free(OLC_DESC(d)->keyword);
    OLC_DESC(d)->keyword = str_dup((arg && *arg) ? arg : "undefined");
    oedit_disp_extradesc_menu(d);
    return;

  case OEDIT_EXTRADESC_MENU:
    switch ((number = atoi(arg))) {
    case 0:
      if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	struct extra_descr_data **tmp_desc;

	if (OLC_DESC(d)->keyword)
	  free(OLC_DESC(d)->keyword);
	if (OLC_DESC(d)->description)
	  free(OLC_DESC(d)->description);

	/*
	 * Clean up pointers  
	 */
	for (tmp_desc = &(OLC_OBJ(d)->ex_description); *tmp_desc;
	     tmp_desc = &((*tmp_desc)->next)) {
	  if (*tmp_desc == OLC_DESC(d)) {
	    *tmp_desc = NULL;
	    break;
	  }
	}
	free(OLC_DESC(d));
      }
    break;

    case 1:
      OLC_MODE(d) = OEDIT_EXTRADESC_KEY;
      send_to_char("Enter keywords, separated by spaces :-\r\n| ", d->character);
      return;

    case 2:
      OLC_MODE(d) = OEDIT_EXTRADESC_DESCRIPTION;
      SEND_TO_Q("Enter the extra description: (/s saves /h for help)\r\n\r\n", d);
      d->backstr = NULL;
      if (OLC_DESC(d)->description) {
	SEND_TO_Q(OLC_DESC(d)->description, d);
	d->backstr = str_dup(OLC_DESC(d)->description);
      }
      d->str = &OLC_DESC(d)->description;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      return;

    case 3:
      /*
       * Only go to the next description if this one is finished.
       */
      if (OLC_DESC(d)->keyword && OLC_DESC(d)->description) {
	struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else {	/* Make new extra description and attach at end. */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = OLC_DESC(d)->next;
	}
      }
      /*
       * No break - drop into default case.
       */
    default:
      oedit_disp_extradesc_menu(d);
      return;
    }
    break;
  default:
    mudlog("SYSERR: OLC: Reached default case in oedit_parse()!", BRF, LVL_BUILDER, TRUE);
    send_to_char("Oops...\r\n", d->character);
    break;
  }

  /*
   * If we get here, we have changed something.  
   */
  OLC_VAL(d) = 1;
  oedit_disp_menu(d);
}


/*----Defines for cost calculating----*/
#define COST_DR_HR_SPR			2000
#define COST_HIT_MANA_MOVE		10
#define COST_SPELL_LEVEL		30 // for wands, staves, scrolls and potions.
#define COST_CONT_MAXWEIGHT		100
#define COST_FOOD_HOURS			4
#define COST_PER_SPELL			5000 // for wands, staves, scrolls and potions.
#define COST_PER_CHARGE			1000 // for wands and staves.
#define COST_PER_AC				1000
#define COST_STAT				3000
#define COST_SAVING_THROWS		100
#define COST_CHAR_X				10
#define COST_WEAPON_SPELL		30000
#define COST_AFFECTION			10000
#define COST_BASE_WEAPON		50000
#define COST_BASE_ARMOR			30000
/* For lights */
#define COST_LIGHT_INFINITIVE	10000
#define COST_LIGHT_PERHOUR		10
/* For drink containers */
#define COST_DRINKUNITS			4
#define COST_WATER				2
#define COST_BEER				33
#define COST_WINE				347
#define COST_ALE				48
#define COST_DARKALE			49
#define COST_WHISKY				87
#define COST_LEMONADE			10
#define COST_FIREBRT			65
#define COST_LOCALSPC			55
#define COST_SLIME				15
#define COST_MILK				9
#define COST_TEA				14
#define COST_COFFE				14
#define COST_BLOOD				150
#define COST_SALTWATER			4
#define COST_CLEARWATER			6
/*
 * I know, above defines maybe should be put in a header file,
 * but I cant really see the harm in them being here.
 * + I am to lazy to put them in a header file
 * + handy to have em in this file.
 */


int oedit_is_manual_cost(struct obj_data *obj)
{
	switch (GET_OBJ_TYPE(obj))
	{
	case ITEM_SCROLL:
	case ITEM_WAND:
	case ITEM_POTION:
	case ITEM_STAFF:
	case ITEM_PIECE:
	case ITEM_OTHER:
	case ITEM_DEED:
		return TRUE;
		break;
	default:
		return FALSE;
	}
	return FALSE;
}
void oedit_calculate_cost(struct descriptor_data *d)
{
	int cost = 0, final_cost = 0, i=0;
	float multi = 0;

	switch (GET_OBJ_TYPE(OLC_OBJ(d)))
	{
	case 0:
		number(75, 200);
		break;
	case ITEM_RUNE:
		cost = 30000;
		break;
	case ITEM_TRASH:
		cost = 10;
		break;
	case ITEM_TREASURE:
		cost = number(100, 10000);
		break;
	case ITEM_PORTAL:
	case ITEM_FOUNTAIN:
	case ITEM_KEY:
		cost = 0;
		break;
	case ITEM_POLE:
		cost = number(75, 200);
		break;
	case ITEM_JAR:
		cost = number(100, 350);
		break;
	case ITEM_STONE:
		cost = number(25, 125);
		break;
	case ITEM_NOTE:
		cost = number(2, 20);
		break;
	case ITEM_MONEY:
		cost = number(100, 5000);
		break;
	case ITEM_BOAT:
		cost = number(50, 300);
		break;
	case ITEM_FIREWEAPON:
	case ITEM_MISSILE:
	case ITEM_PEN:
	case ITEM_TRAP:
	case ITEM_DOLL:
	case ITEM_SMOKE:
	case ITEM_HERB:
	case ITEM_SHIP:
	case ITEM_TILLER:
		cost = 0;
		break;
	case ITEM_ARMOR:
		multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
		cost += COST_BASE_ARMOR;
		oedit_calculate_cost_applies(d);
		cost += (GET_OBJ_VAL(OLC_OBJ(d), 0)*COST_PER_AC);
		for (i=0; i < 100; i++) {
			if (IS_SET_AR(OLC_OBJ(d)->obj_flags.bitvector, i)) {
				cost += COST_AFFECTION;
			}
		}
		break;
	case ITEM_WORN:
		multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
		cost += COST_BASE_ARMOR;
		oedit_calculate_cost_applies(d);
		cost += (GET_OBJ_VAL(OLC_OBJ(d), 0)*COST_PER_AC);
		for (i=0; i < 100; i++) {
			if (IS_SET_AR(OLC_OBJ(d)->obj_flags.bitvector, i)) {
				cost += COST_AFFECTION;
			}
		}
		break;
    case ITEM_WEAPON:
		multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
		cost += COST_BASE_WEAPON;
		oedit_calculate_cost_applies(d);
		if (GET_OBJ_SPELL(OLC_OBJ(d)) && GET_OBJ_SPELL_EXTRA(OLC_OBJ(d))) {
			cost += COST_WEAPON_SPELL;
		}
		for (i=0; i < 100; i++) {
			if (IS_SET_AR(OLC_OBJ(d)->obj_flags.bitvector, i)) {
				cost += COST_AFFECTION;
			}
		}
      break;
	case ITEM_LIGHT:
		multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
		oedit_calculate_cost_applies(d);
		if (GET_OBJ_VAL(OLC_OBJ(d), 2) == -1)
			cost += COST_LIGHT_INFINITIVE;
		else if (GET_OBJ_VAL(OLC_OBJ(d), 2) > 0)
			cost += COST_LIGHT_PERHOUR;
		for (i=0; i < 100; i++) {
			if (IS_SET_AR(OLC_OBJ(d)->obj_flags.bitvector, i)) {
				cost += COST_AFFECTION;
			}
		}
      break;
	case ITEM_CONTAINER:
		multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
		oedit_calculate_cost_applies(d);
		cost += (GET_OBJ_VAL(OLC_OBJ(d), 0)*COST_CONT_MAXWEIGHT);
		for (i=0; i < 100; i++) {
			if (IS_SET_AR(OLC_OBJ(d)->obj_flags.bitvector, i)) {
				cost += COST_AFFECTION;
			}
		}
      break;
	case ITEM_DRINKCON:
		cost = (GET_OBJ_VAL(OLC_OBJ(d), 0)*COST_DRINKUNITS);
      break;
	case ITEM_FOOD:
		cost = (GET_OBJ_VAL(OLC_OBJ(d), 0)*COST_FOOD_HOURS);
      break;
    default:
		GET_OBJ_COST(OLC_OBJ(d)) += number(10, 200);
      break;
    }
	if (multi > 0)
		final_cost = cost*multi;
	else
		final_cost = cost;
	GET_OBJ_COST(OLC_OBJ(d)) += final_cost;
}

#define MAX_APPLIES	30
void oedit_calculate_cost_applies(struct descriptor_data *d)
{
  int counter;
  int cost = 0, final_cost = 0;
  float multi;

  for (counter = 0; counter < MAX_APPLIES; counter++) {
     if (OLC_OBJ(d)->affected[counter].location == APPLY_DAMROLL) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_DR_HR_SPR;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_HITROLL) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_DR_HR_SPR;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_SPELLPOWER) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_DR_HR_SPR;
	 }/*
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_HIT) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_HITPOINTS;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_MOVE) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_MOVEMENT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_MANA) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_MANA;
	 }*/
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_STR) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_DEX) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_INT) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_WIS) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_CON) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_CHA) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_STAT;
	 }
	 else if (OLC_OBJ(d)->affected[counter].location == APPLY_AC) {
		 cost += OLC_OBJ(d)->affected[counter].modifier*COST_PER_AC;
	 }
  }
  multi = ((float)GET_OBJ_LEVEL(OLC_OBJ(d))/150);
  final_cost = cost*multi;
  GET_OBJ_COST(OLC_OBJ(d)) = final_cost;
}