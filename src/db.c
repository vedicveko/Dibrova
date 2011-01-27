/**************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __DB_C__


#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "hometowns.h"
#include "dg_scripts.h"
#include "diskio.h"
#include "mounts.h"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
**************************************************************************/

struct room_data *world = NULL;	/* array of rooms		 */
int top_of_world = 0;		/* ref to top element of world	 */

struct char_data *character_list = NULL; /* global linked list of chars */

struct aq_data *aquest_table;
int top_of_aquestt = 0;

struct index_data **trig_index; /* index table for triggers      */
int top_of_trigt = 0;           /* top of trigger index table    */
long max_id = MOBOBJ_ID_BASE;   /* for unique mob/obj id's       */
	
struct index_data *mob_index;	/* index table for mobile file	 */
struct char_data *mob_proto;	/* prototypes for mobs		 */
int top_of_mobt = 0;		/* top of mobile index table	 */

struct obj_data *object_list = NULL;	/* global linked list of objs	 */
struct index_data *obj_index;	/* index table for object file	 */
struct obj_data *obj_proto;	/* prototypes for objs		 */
int top_of_objt = 0;		/* top of object index table	 */

struct zone_data *zone_table;	/* zone table			 */
int top_of_zone_table = 0;	/* top element of zone tab	 */
struct message_list weapon_messages[MAX_MESSAGES];/* standard messages    */
struct message_list skill_messages[MAX_MESSAGES]; /* skill messages    */
struct message_list spell_messages[MAX_MESSAGES]; /* spell messages    */
struct message_list chant_messages[MAX_MESSAGES]; /* chant messages    */
struct message_list prayer_messages[MAX_MESSAGES];/* prayer messages   */
struct message_list song_messages[MAX_MESSAGES];  /* song messages     */

struct player_index_element *player_table = NULL;	/* index to plr file	 */
FILE *player_fl = NULL;		/* file desc of player file	 */
int top_of_p_table = 0;		/* ref to top of table		 */
int top_of_p_file = 0;		/* ref of size of p file	 */
long top_idnum = 0;		/* highest idnum in use		 */

/* For TIPS support... */
int tip_count;
long *tip_list;

int no_mail = 0;		/* mail disabled?		 */
int mini_mud = 0;		/* mini-mud mode?		 */
int no_rent_check = 0;		/* skip rent check on boot?	 */
time_t boot_time = 0;		/* time of mud boot		 */
int restrict = 0;		/* level of game restriction	 */
int r_immort_start_room;	/* rnum of immort start room	 */
int r_frozen_start_room;	/* rnum of frozen start room	 */

char *credits = NULL;		/* game credits			 */
char *news = NULL;		/* mud news			 */
char *motd = NULL;		/* message of the day - mortals */
char *imotd = NULL;		/* message of the day - immorts */
char *help = NULL;		/* help screen			 */
char *info = NULL;		/* info page			 */
char *wizlist = NULL;		/* list of higher gods		 */
char *immlist = NULL;		/* list of peon gods		 */
char *background = NULL;	/* background story		 */
char *handbook = NULL;		/* handbook for new immortals	 */
char *policies = NULL;		/* policies page		 */

struct help_index_element *help_table = 0;	/* the help table	 */
int top_of_helpt = 0;		/* top of help index table	 */

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;	/* the infomation about the weather */
struct player_special_data dummy_mob;	/* dummy spec area for mobs	 */
struct reset_q_type reset_q;	/* queue of zones to be reset	 */

/* local functions */
void setup_dir(FILE * fl, int room, int dir);
void index_boot(int mode);
void discrete_load(FILE * fl, int mode);
void parse_trigger(FILE *fl, int virtual_nr);
void parse_room(FILE * fl, int virtual_nr);
void parse_mobile(FILE * mob_f, int nr);
char *parse_object(FILE * obj_f, int nr);
void parse_quest(FILE * quest_f, int nr);
void load_zones(FILE * fl, char *zonename);
void load_help(FILE *fl);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
void assign_the_shopkeepers(void);
void build_player_index(void);
void char_to_store(struct char_data * ch, struct char_file_u * st);
void store_to_char(struct char_file_u * st, struct char_data * ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
int file_to_string_alloc(char *name, char **buf);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(int zone, int cmd_no, char *message);
void reset_time(void);
void clear_char(struct char_data * ch);
bool illegal_affection(int affection);
void vnum_illegal(struct char_data * ch);
int special_loading(byte type);
int mountt_clear_deleted_types(int id);

/* external functions */
extern struct descriptor_data *descriptor_list;
void load_messages(struct message_list *, char *);
void weather_and_time(int mode);
void mag_assign_abils(void);
void boot_social_messages(void);
void update_obj_file(void);	/* In objsave.c */
void load_corpses(void);

extern char *affected_bits[];
void colorless_str(char *str);

void sort_commands(void);
void sort_spells(void);
void load_banned(void);
void Read_Invalid_List(void);
void boot_the_shops(FILE * shop_f, char *filename, int rec_count);
int hsort(const void *a, const void *b);
void init_clans(void);
void load_teleporters();
extern void load_exp_table();
extern void load_thaco_table();
void save_char_vars(struct char_data *ch);  
/* ships are loaded at boot time, not at zreset because
 * zreset looks only at the object's load room where the
 * command is invoked and ships are mobile.  Each ship MUST
 * be unique to the game, otherwise they won't work.
 */
void load_ship(int number, int room);
void Ship_boot(void);

/* external vars */
extern int no_specials;
extern float race_height_idx[];
extern float race_weight_idx[];

#define READ_SIZE 256

/*************************************************************************
*  routines for booting the system                                       *
*************************************************************************/

/* this is necessary for the autowiz system */
void reboot_wizlists(void)
{
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
}


ACMD(do_reboot)
{
  int i;

  one_argument(argument, arg);

  if (!str_cmp(arg, "all") || *arg == '*') {
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
    file_to_string_alloc(IMMLIST_FILE, &immlist);
    file_to_string_alloc(NEWS_FILE, &news);
    file_to_string_alloc(CREDITS_FILE, &credits);
    file_to_string_alloc(MOTD_FILE, &motd);
    file_to_string_alloc(IMOTD_FILE, &imotd);
    file_to_string_alloc(HELP_PAGE_FILE, &help);
    file_to_string_alloc(INFO_FILE, &info);
    file_to_string_alloc(POLICIES_FILE, &policies);
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
    file_to_string_alloc(BACKGROUND_FILE, &background);
  } else if (!str_cmp(arg, "wizlist"))
    file_to_string_alloc(WIZLIST_FILE, &wizlist);
  else if (!str_cmp(arg, "immlist"))
    file_to_string_alloc(IMMLIST_FILE, &immlist);
  else if (!str_cmp(arg, "news"))
    file_to_string_alloc(NEWS_FILE, &news);
  else if (!str_cmp(arg, "credits"))
    file_to_string_alloc(CREDITS_FILE, &credits);
  else if (!str_cmp(arg, "motd"))
    file_to_string_alloc(MOTD_FILE, &motd);
  else if (!str_cmp(arg, "imotd"))
    file_to_string_alloc(IMOTD_FILE, &imotd);
  else if (!str_cmp(arg, "help"))
    file_to_string_alloc(HELP_PAGE_FILE, &help);
  else if (!str_cmp(arg, "info"))
    file_to_string_alloc(INFO_FILE, &info);
  else if (!str_cmp(arg, "policy"))
    file_to_string_alloc(POLICIES_FILE, &policies);
  else if (!str_cmp(arg, "handbook"))
    file_to_string_alloc(HANDBOOK_FILE, &handbook);
  else if (!str_cmp(arg, "background"))
    file_to_string_alloc(BACKGROUND_FILE, &background);
  else if (!str_cmp(arg, "xhelp")) {
    if (help_table) {
      for (i = 0; i <= top_of_helpt; i++) {
        if (help_table[i].keyword)
	  free(help_table[i].keyword);
        if (help_table[i].entry && !help_table[i].duplicate)
	  free(help_table[i].entry);
      }
      free(help_table);
    }
    top_of_helpt = 0;
    index_boot(DB_BOOT_HLP);
  } else if (!str_cmp(arg, "tips")) {
    free(tip_list);
    load_tips();
  } else {
    send_to_char("Unknown reload option.\r\n", ch);
    return;
  }

  send_to_char(OK, ch);
}


void boot_world(void)
{
  log("Indexing Tips.");
  load_tips();

  log("Loading zone table.");
  index_boot(DB_BOOT_ZON);

  log("Loading triggers and generating index.");
  index_boot(DB_BOOT_TRG);

  log("Loading rooms.");
  index_boot(DB_BOOT_WLD);

  log("Renumbering rooms.");
  renum_world();

  log("Checking start rooms.");
  check_start_rooms();

  log("Loading mobs and generating index.");
  index_boot(DB_BOOT_MOB);

  log("Loading objs and generating index.");
  index_boot(DB_BOOT_OBJ);

  log("Renumbering zone table.");
  renum_zone_table();

  if (!no_specials) {
    log("Loading shops.");
    index_boot(DB_BOOT_SHP);
  }
  
//  log("Loading quests.");
//  index_boot(DB_BOOT_QST);
  
  log("Loading Corpses");
  load_corpses();

  log("Loading teleporters.");
  load_teleporters();

}

  

/* body of the booting system */
void boot_db(void)
{
  int i;

  log("Boot db -- BEGIN.");

  log("Resetting the game time:");
  reset_time();

  log("Computing experience table.");
  load_exp_table();

  log("Computing thac0 table.");
  load_thaco_table();

  log("Reading news, credits, help, bground, info & motds.");
  file_to_string_alloc(NEWS_FILE, &news);
  file_to_string_alloc(CREDITS_FILE, &credits);
  file_to_string_alloc(MOTD_FILE, &motd);
  file_to_string_alloc(IMOTD_FILE, &imotd);
  file_to_string_alloc(HELP_PAGE_FILE, &help);
  file_to_string_alloc(INFO_FILE, &info);
  file_to_string_alloc(WIZLIST_FILE, &wizlist);
  file_to_string_alloc(IMMLIST_FILE, &immlist);
  file_to_string_alloc(POLICIES_FILE, &policies);
  file_to_string_alloc(HANDBOOK_FILE, &handbook);
  file_to_string_alloc(BACKGROUND_FILE, &background);


  boot_world();

  log("Loading help entries.");
  index_boot(DB_BOOT_HLP);

  log("Generating player index.");
  build_player_index();

  log("Loading weapon messages.");
  load_messages(weapon_messages, MSG_WEAPONS);

  log("Loading spell messages.");
  load_messages(spell_messages, MSG_SPELLS);

  log("Loading skill messages.");
  load_messages(skill_messages, MSG_SKILLS);

  log("Loading chant messages.");
  load_messages(chant_messages, MSG_CHANTS);

  log("Loading prayer messages.");
  load_messages(prayer_messages, MSG_PRAYERS);

  log("Loading song messages.");
  load_messages(song_messages, MSG_SONGS);

  log("Loading social messages.");
  boot_social_messages();

  log("Assigning function pointers:");

  if (!no_specials) {
    log("   Mobiles.");
    assign_mobiles();
    log("   Shopkeepers.");
    assign_the_shopkeepers();
    log("   Objects.");
    assign_objects();
    log("   Rooms.");
    assign_rooms();
  }
  log("   Abilities.");
  mag_assign_abils();

  log("Assigning abilities levels.");
  init_abil_levels();

  log("Sorting command list and spells.");
  sort_commands();
  sort_spells();

  log("Booting Clans.");
  init_clans();

  log("Initializing Mount-Types.");
  mountt_load();

  log("Booting mail system.");
  if (!scan_file()) {
    log("    Mail boot failed -- Mail system disabled");
    no_mail = 1;
  }
  log("Reading banned site and invalid-name list.");
  load_banned();
  Read_Invalid_List();

  if (!no_rent_check) {
    log("Deleting timed-out crash and rent files:");
    update_obj_file();
    log("Done.");
  }
  for (i = 0; i <= top_of_zone_table; i++) {
    sprintf(buf2, "Resetting %s (rooms %d-%d).",
	    zone_table[i].name, (i ? (zone_table[i - 1].top + 1) : 0),
	    zone_table[i].top);
    log(buf2);
    reset_zone(i);
  }

  reset_q.head = reset_q.tail = NULL;

  if (!mini_mud) {
    log("Booting houses.");
    House_boot();
    log("Booting ships.");
    Ship_boot();
  }
  boot_time = time(0);

  log("Booting vaults.");
  vault_boot();
  log("Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time(void)
{
  long beginning_of_time = 650336715;
  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);

  if (time_info.hours <= 4)
    weather_info.sunlight = SUN_DARK;
  else if (time_info.hours == 5)
    weather_info.sunlight = SUN_RISE;
  else if (time_info.hours <= 20)
    weather_info.sunlight = SUN_LIGHT;
  else if (time_info.hours == 21)
    weather_info.sunlight = SUN_SET;
  else
    weather_info.sunlight = SUN_DARK;

  sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
	  time_info.day, time_info.month, time_info.year);
  log(buf);

  weather_info.pressure = 960;
  if ((time_info.month >= 7) && (time_info.month <= 12))
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}



/* generate index table for the player file */
void build_player_index(void)
{
  int nr = -1, i;
  long size, recs;
  struct char_file_u dummy;

  if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
    if (errno != ENOENT) {
      perror("SYSERR: fatal error opening playerfile");
      exit(1);
    } else {
      log("No playerfile.  Creating a new one.");
      touch(PLAYER_FILE);
      if (!(player_fl = fopen(PLAYER_FILE, "r+b"))) {
	perror("SYSERR: fatal error opening playerfile");
	exit(1);
      }
    }
  }

  fseek(player_fl, 0L, SEEK_END);
  size = ftell(player_fl);
  rewind(player_fl);
  if (size % sizeof(struct char_file_u))
    fprintf(stderr, "\aWARNING:  PLAYERFILE IS PROBABLY CORRUPT!\n");
  recs = size / sizeof(struct char_file_u);
  if (recs) {
    sprintf(buf, "   %ld players in database.", recs);
    log(buf);
    CREATE(player_table, struct player_index_element, recs);
  } else {
    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;
    return;
  }

  for (; !feof(player_fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, player_fl);
    if (!feof(player_fl)) {	/* new record */
      nr++;
      CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
      for (i = 0;
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); i++);
      player_table[nr].id = dummy.char_specials_saved.idnum;
      top_idnum = MAX(top_idnum, dummy.char_specials_saved.idnum);
    }
  }

  top_of_p_file = top_of_p_table = nr;
}



/* function to count how many hash-mark delimited records exist in a file */
int count_hash_records(FILE * fl)
{
  char buf[128];
  int count = 0;

  while (fgets(buf, 128, fl))
    if (*buf == '#')
      count++;

  return count;
}



void index_boot(int mode)
{
  char *index_filename, *prefix;
  FILE *index, *db_file;
  int rec_count = 0;

  switch (mode) {
  case DB_BOOT_TRG:
    prefix = TRG_PREFIX;
    break;
  case DB_BOOT_WLD:
    prefix = WLD_PREFIX;
    break;
  case DB_BOOT_MOB:
    prefix = MOB_PREFIX;
    break;
  case DB_BOOT_OBJ:
    prefix = OBJ_PREFIX;
    break;
  case DB_BOOT_ZON:
    prefix = ZON_PREFIX;
    break;
  case DB_BOOT_SHP:
    prefix = SHP_PREFIX;
    break;
  case DB_BOOT_HLP:
    prefix = HLP_PREFIX;
    break;
  case DB_BOOT_QST:
    prefix = QST_PREFIX;
    break;
 default:
    log("SYSERR: Unknown subcommand to index_boot!");
    exit(1);
    break;
  }

  if (mini_mud)
    index_filename = MINDEX_FILE;
  else
    index_filename = INDEX_FILE;

  sprintf(buf2, "%s/%s", prefix, index_filename);

  if (!(index = fopen(buf2, "r"))) {
    sprintf(buf1, "SYSERR: opening index file '%s'", buf2);
    perror(buf1);
    exit(1);
  }

  /* first, count the number of records in the file so we can malloc */
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      log("SYSERR: file listed in index not found");
      fscanf(index, "%s\n", buf1);
      continue;
    } else {
      if (mode == DB_BOOT_ZON)
	rec_count++;
      else
	rec_count += count_hash_records(db_file);
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* Exit if 0 records, unless this is shops */
  if (!rec_count) {
    if (mode == DB_BOOT_SHP)
      return;
    log("SYSERR: boot error - 0 records counted");
    exit(1);
  }

  rec_count++;

  switch (mode) {
  case DB_BOOT_TRG:
    CREATE(trig_index, struct index_data *, rec_count);
    break;
  case DB_BOOT_WLD:
    CREATE(world, struct room_data, rec_count);
    break;
  case DB_BOOT_MOB:
    CREATE(mob_proto, struct char_data, rec_count);
    CREATE(mob_index, struct index_data, rec_count);
    break;
  case DB_BOOT_OBJ:
    CREATE(obj_proto, struct obj_data, rec_count);
    CREATE(obj_index, struct index_data, rec_count);
    break;
  case DB_BOOT_ZON:
    CREATE(zone_table, struct zone_data, rec_count);
    break;
  case DB_BOOT_HLP:
    CREATE(help_table, struct help_index_element, rec_count * 2);
    break;
  }

  rewind(index);
  fscanf(index, "%s\n", buf1);
  while (*buf1 != '$') {
    sprintf(buf2, "%s/%s", prefix, buf1);
    if (!(db_file = fopen(buf2, "r"))) {
      perror(buf2);
      exit(1);
    }
    switch (mode) {
    case DB_BOOT_TRG:
    case DB_BOOT_WLD:
    case DB_BOOT_OBJ:
    case DB_BOOT_MOB:
    case DB_BOOT_QST:
      discrete_load(db_file, mode);
      break;
    case DB_BOOT_ZON:
      load_zones(db_file, buf2);
      break;
    case DB_BOOT_HLP:
      load_help(db_file);
      break;
    case DB_BOOT_SHP:
      boot_the_shops(db_file, buf2, rec_count);
      break;
    }

    fclose(db_file);
    fscanf(index, "%s\n", buf1);
  }

  /* sort the help index */
  if (mode == DB_BOOT_HLP) {
    qsort(help_table, top_of_helpt, sizeof(struct help_index_element), hsort);
    top_of_helpt--;
  }

}


void discrete_load(FILE * fl, int mode)
{
  int nr = -1, last = 0;
  char line[256];

   const char *modes[] = {"world", "mob", "obj", "ZON", "SHP", "HLP", "trg", "qst"};
   /* modes positions correspond to DB_BOOT_xxx in db.h */

  for (;;) {
    /*
     * we have to do special processing with the obj files because they have
     * no end-of-record marker :(
     */
    if (mode != DB_BOOT_OBJ || nr < 0)
      if (!get_line(fl, line)) {
	fprintf(stderr, "SYSERR: Format error after %s #%d (file not terminated with '$'?)\n", modes[mode], nr);
	exit(1);
      }
    if (*line == '$')
      return;

    if (*line == '#') {
      last = nr;
      if (sscanf(line, "#%d", &nr) != 1) {
	fprintf(stderr, "SYSERR: Format error after %s #%d\n", modes[mode], last);
	exit(1);
      }
      if (nr >= 99999)
	return;
      else
	switch (mode) {
        case DB_BOOT_TRG:
          parse_trigger(fl, nr);
          break;
	case DB_BOOT_WLD:
	  parse_room(fl, nr);
	  break;
	case DB_BOOT_MOB:
	  parse_mobile(fl, nr);
	  break;
	case DB_BOOT_OBJ:
	  strcpy(line, parse_object(fl, nr));
	  break;

	}
    } else {
      fprintf(stderr, "SYSERR: Format error in %s file near %s #%d\n",
	      modes[mode], modes[mode], nr);
      fprintf(stderr, "Offending line: '%s'\n", line);
      exit(1);
    }
  }
}


long asciiflag_conv(char *flag)
{
  long flags = 0;
  int is_number = 1;
  register char *p;

  for (p = flag; *p; p++) {
    if (islower(*p))
      flags |= 1 << (*p - 'a');
    else if (isupper(*p))
      flags |= 1 << (26 + (*p - 'A'));

    if (!isdigit(*p))
      is_number = 0;
  }

  if (is_number)
    flags = atol(flag);

  return flags;
}

char fread_letter(FILE *fp)
{
  char c;
  do {
    c = getc(fp);  
  } while (isspace(c));
  return c;
}

/* load the rooms */
void parse_room(FILE * fl, int virtual_nr)
{
  static int room_nr = 0, zone = 0;
  int t[10], i;
  char line[256], flags[128], flags2[128], flags3[128], flags4[128];
  struct extra_descr_data *new_descr;
  char letter;

  sprintf(buf2, "room #%d", virtual_nr);

  if (virtual_nr <= (zone ? zone_table[zone - 1].top : -1)) {
    fprintf(stderr, "SYSERR: Room #%d is below zone %d.\n", virtual_nr, zone);
    exit(1);
  }
  while (virtual_nr > zone_table[zone].top)
    if (++zone > top_of_zone_table) {
      fprintf(stderr, "SYSERR: Room %d is outside of any zone.\n", virtual_nr);
      exit(1);
    }
  world[room_nr].zone = zone;
  world[room_nr].number = virtual_nr;
  world[room_nr].name = fread_string(fl, buf2);
  world[room_nr].description = fread_string(fl, buf2);

  if (!get_line(fl, line) ||
      sscanf(line, " %d %s %s %s %s %d ", t, flags, flags2, flags3,
             flags4, t + 2) != 6) {
    fprintf(stderr, "SYSERR: Format error in room #%d\n", virtual_nr);
    exit(1);
  }
  /* t[0] is the zone number; ignored with the zone-file system */
  world[room_nr].room_flags[0] = asciiflag_conv(flags);
  world[room_nr].room_flags[1] = asciiflag_conv(flags2);
  world[room_nr].room_flags[2] = asciiflag_conv(flags3);
  world[room_nr].room_flags[3] = asciiflag_conv(flags4);
  world[room_nr].sector_type = t[2];
  world[room_nr].func = NULL;
  world[room_nr].contents = NULL;
  world[room_nr].people = NULL;
  world[room_nr].light = 0;	/* Zero light sources */

  for (i = 0; i < NUM_OF_DIRS; i++)
    world[room_nr].dir_option[i] = NULL;

  world[room_nr].ex_description = NULL;

  sprintf(buf, "SYSERR: Format error in room #%d (expecting D/E/S)", virtual_nr);

  for (;;) {
    if (!get_line(fl, line)) {
      fprintf(stderr, "%s\n", buf);
      exit(1);
    }
    switch (*line) {
    case 'D':
      setup_dir(fl, room_nr, atoi(line + 1));
      break;
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(fl, buf2);
      new_descr->description = fread_string(fl, buf2);
      new_descr->next = world[room_nr].ex_description;
      world[room_nr].ex_description = new_descr;
      break;
    case 'S':			/* end of room */
      /* DG triggers -- script is defined after the end of the room */
      letter = fread_letter(fl);
      ungetc(letter, fl);
      while (letter=='T') {
        dg_read_trigger(fl, &world[room_nr], WLD_TRIGGER);
        letter = fread_letter(fl);
        ungetc(letter, fl);
      }
      top_of_world = room_nr++;
      return;
      break;
    default:
      fprintf(stderr, "%s\n", buf);
      exit(1);
      break;
    }
  }
}



/* read direction data */
void setup_dir(FILE * fl, int room, int dir)
{
  int t[5];
  char line[256];

  sprintf(buf2, "room #%d, direction D%d", world[room].number, dir);

  CREATE(world[room].dir_option[dir], struct room_direction_data, 1);
  world[room].dir_option[dir]->general_description = fread_string(fl, buf2);
  world[room].dir_option[dir]->keyword = fread_string(fl, buf2);

  if (!get_line(fl, line)) {
    fprintf(stderr, "SYSERR: Format error, %s\n", buf2);
    exit(1);
  }
  t[3]=0;
  if (sscanf(line, " %d %d %d %d", t, t + 1, t + 2, t + 3) < 3) {
    fprintf(stderr, "SYSERR: Format error, %s\n", buf2);
    exit(1);
  }
  if (t[0] == 1)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR;
  else if (t[0] == 2)
    world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
  else
    world[room].dir_option[dir]->exit_info = 0;

  world[room].dir_option[dir]->key = t[1];
  world[room].dir_option[dir]->to_room = t[2];

  /* Set extra exit info */
  if (t[3])
    world[room].dir_option[dir]->exit_info |= t[3];

}


/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
  extern int immort_start_room;
  extern int frozen_start_room;

  if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Immort start room does not exist.  Change in config.c.");
    r_immort_start_room = real_room(3001);
  }
  if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
    if (!mini_mud)
      log("SYSERR:  Warning: Frozen start room does not exist.  Change in config.c.");
    r_frozen_start_room = real_room(3001);
  }

}

/* resolve all vnums into rnums in the world */
void renum_world(void)
{
  register int room, door;

  for (room = 0; room <= top_of_world; room++)
    for (door = 0; door < NUM_OF_DIRS; door++)
      if (world[room].dir_option[door])
	if (world[room].dir_option[door]->to_room != NOWHERE)
	  world[room].dir_option[door]->to_room =
	    real_room(world[room].dir_option[door]->to_room);
}


#define ZCMD zone_table[zone].cmd[cmd_no]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
  int zone, cmd_no, a, b, c, olda, oldb, oldc;
  char buf[128];

  for (zone = 0; zone <= top_of_zone_table; zone++)
    for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
      a = b = c = 0;
      olda = ZCMD.arg1;
      oldb = ZCMD.arg2;
      oldc = ZCMD.arg3;
      switch (ZCMD.command) {
      case 'M':
	a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
	c = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'O':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	if (ZCMD.arg3 != NOWHERE)
	  c = ZCMD.arg3 = real_room(ZCMD.arg3);
	break;
      case 'G':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'E':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	break;
      case 'P':
	a = ZCMD.arg1 = real_object(ZCMD.arg1);
	c = ZCMD.arg3 = real_object(ZCMD.arg3);
	break;
      case 'D':
	a = ZCMD.arg1 = real_room(ZCMD.arg1);
	break;
      case 'R': /* rem obj from room */
        a = ZCMD.arg1 = real_room(ZCMD.arg1);
	b = ZCMD.arg2 = real_object(ZCMD.arg2);
        break;
      case 'T': /* a trigger */
        /* designer's choice: convert this later */
        /* b = ZCMD.arg2 = real_trigger(ZCMD.arg2); */
        b = real_trigger(ZCMD.arg2); /* leave this in for validation */
        break;
      case 'V': /* trigger variable assignment */
        if (ZCMD.arg1 == WLD_TRIGGER)
          b = ZCMD.arg2 = real_room(ZCMD.arg2);
        break;
      }
      if (a < 0 || b < 0 || c < 0) {
	if (!mini_mud) {
	  sprintf(buf,  "Invalid vnum %d, cmd disabled",
			 (a < 0) ? olda : ((b < 0) ? oldb : oldc));
	  log_zone_error(zone, cmd_no, buf);
	}
	ZCMD.command = '*';
      }
    }
}



void parse_simple_mob(FILE *mob_f, int i, int nr)
{
  int j, t[10];
  char line[256];

    mob_proto[i].real_stats.str = number(5, 25); 
    mob_proto[i].real_stats.intel = number(5, 25); 
    mob_proto[i].real_stats.wis = number(5, 25); 
    mob_proto[i].real_stats.dex = number(5, 25); 
    mob_proto[i].real_stats.con = number(5, 25); 
    mob_proto[i].real_stats.cha = number(5, 25); 
    mob_proto[i].real_stats.luck = number(5, 25);
    mob_proto[i].mob_specials.maxfactor = 25;

    if (!get_line(mob_f, line) || (sscanf(line, " %d %d %d %dd%d+%d %dd%d+%d ",
	  t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8) != 9)) {
      fprintf(stderr, "SYSERR: Format error in mob #%d, first line after S flag\n"
	      "...expecting line of form '# # # #d#+# #d#+#'\n", nr);
      exit(1);
    }
    GET_LEVEL(mob_proto + i) = t[0];
    mob_proto[i].points.hitroll = t[1];
    mob_proto[i].points.armor = t[2];

    /* max hit = 0 is a flag that H, M, V is xdy+z */
    mob_proto[i].points.max_hit = 0;
    mob_proto[i].points.hit = t[3];
    mob_proto[i].points.mana = t[4];
    mob_proto[i].points.move = t[5];

    mob_proto[i].points.max_mana = 10;
    mob_proto[i].points.max_move = 50;

    mob_proto[i].mob_specials.damnodice = t[6];
    mob_proto[i].mob_specials.damsizedice = t[7];
    mob_proto[i].points.damroll = t[8];

    if (!get_line(mob_f, line) || (sscanf(line, " %d %d ", t, t + 1) != 2)) {
      fprintf(stderr, "SYSERR: Format error in mob #%d, second line after S flag\n"
		      "...expecting line of form '# #'\n", nr);
      exit(1);
    }

    GET_GOLD(mob_proto + i) = t[0];
    GET_EXP(mob_proto + i) = t[1];

    if (!get_line(mob_f, line) || (sscanf(line, " %d %d %d %d ", t, t + 1, t + 2, t + 3) != 3)) {
      fprintf(stderr, "SYSERR: Format error in last line of mob #%d\n"
	      "...expecting line of form '# # #'\n", nr);
      exit(1);
    }

    mob_proto[i].char_specials.position = t[0];
    mob_proto[i].mob_specials.default_pos = t[1];
    mob_proto[i].player.sex = t[2];

    mob_proto[i].player.class = CLASS_NPC_OTHER;
    mob_proto[i].player.weight = 200;
    mob_proto[i].player.height = 198;

    for (j = 0; j < 3; j++)
      GET_COND(mob_proto + i, j) = -1;

    /*
     * these are now save applies; base save numbers for MOBs are now from
     * the warrior save table.
     */
    for (j = 0; j < 5; j++)
      GET_SAVE(mob_proto + i, j) = 0;
}


/*
 * interpret_espec is the function that takes espec keywords and values
 * and assigns the correct value to the mob as appropriate.  Adding new
 * e-specs is absurdly easy -- just add a new CASE statement to this
 * function!  No other changes need to be made anywhere in the code.
 */

#define CASE(test) if (!matched && !str_cmp(keyword, test) && (matched = 1))
#define RANGE(low, high) (num_arg = MAX((low), MIN((high), (num_arg))))

void interpret_espec(char *keyword, char *value, int i, int nr)
{
  int num_arg, matched = 0;

  num_arg = atoi(value);

  CASE("BareHandAttack") {
    RANGE(0, 99);
    mob_proto[i].mob_specials.attack_type = num_arg;
  }

  CASE("Str") {
    RANGE(3, 25);
    mob_proto[i].real_stats.str = num_arg;
  }

  CASE("StrAdd") {
    RANGE(0, 100);
    mob_proto[i].real_stats.str_add = num_arg;    
  }

  CASE("Int") {
    RANGE(3, 25);
    mob_proto[i].real_stats.intel = num_arg;
  }

  CASE("Wis") {
    RANGE(3, 25);
    mob_proto[i].real_stats.wis = num_arg;
  }

  CASE("Dex") {
    RANGE(3, 25);
    mob_proto[i].real_stats.dex = num_arg;
  }

  CASE("Con") {
    RANGE(3, 25);
    mob_proto[i].real_stats.con = num_arg;
  }

  CASE("Cha") {
    RANGE(3, 25);
    mob_proto[i].real_stats.cha = num_arg;
  }

  CASE("Class") {
    RANGE(0, NUM_NPC_CLASS);
    mob_proto[i].player.class = num_arg;
  }
  CASE("Race") {
    RANGE(0, NUM_NPC_RACE);
    mob_proto[i].player.race = num_arg;
  }
  CASE("Luck") {
    RANGE(3, 25);
    mob_proto[i].real_stats.luck = num_arg;
  }
  CASE("MaxFactor") {
    RANGE(1, 255);
    mob_proto[i].mob_specials.maxfactor = num_arg;
  }
  CASE("Skill1") {
	  mob_proto[i].mob_specials.skills[MOB_SKILL_ONE] = num_arg;
  }
  CASE("Skill2") {
	  mob_proto[i].mob_specials.skills[MOB_SKILL_TWO] = num_arg;
  }
  CASE("Skill3") {
	  mob_proto[i].mob_specials.skills[MOB_SKILL_THREE] = num_arg;
  }
  CASE("Skill4") {
	  mob_proto[i].mob_specials.skills[MOB_SKILL_FOUR] = num_arg;
  }
  CASE("Skill5") {
	  mob_proto[i].mob_specials.skills[MOB_SKILL_FIVE] = num_arg;
  }
  CASE("Spell1") {
	  mob_proto[i].mob_specials.spells[MOB_SKILL_ONE] = num_arg;
  }
  CASE("Spell2") {
	  mob_proto[i].mob_specials.spells[MOB_SKILL_TWO] = num_arg;
  }
  CASE("Spell3") {
	  mob_proto[i].mob_specials.spells[MOB_SKILL_THREE] = num_arg;
  }
  CASE("Spell4") {
	  mob_proto[i].mob_specials.spells[MOB_SKILL_FOUR] = num_arg;
  }
  CASE("Spell5") {
	  mob_proto[i].mob_specials.spells[MOB_SKILL_FIVE] = num_arg;
  }
  CASE("Chant1") {
	  mob_proto[i].mob_specials.chants[MOB_SKILL_ONE] = num_arg;
  }
  CASE("Chant2") {
	  mob_proto[i].mob_specials.chants[MOB_SKILL_TWO] = num_arg;
  }
  CASE("Chant3") {
	  mob_proto[i].mob_specials.chants[MOB_SKILL_THREE] = num_arg;
  }
  CASE("Chant4") {
	  mob_proto[i].mob_specials.chants[MOB_SKILL_FOUR] = num_arg;
  }
  CASE("Chant5") {
	  mob_proto[i].mob_specials.chants[MOB_SKILL_FIVE] = num_arg;
  }
  CASE("Prayer1") {
	  mob_proto[i].mob_specials.prayers[MOB_SKILL_ONE] = num_arg;
  }
  CASE("Prayer2") {
	  mob_proto[i].mob_specials.prayers[MOB_SKILL_TWO] = num_arg;
  }
  CASE("Prayer3") {
	  mob_proto[i].mob_specials.prayers[MOB_SKILL_THREE] = num_arg;
  }
  CASE("Prayer4") {
	  mob_proto[i].mob_specials.prayers[MOB_SKILL_FOUR] = num_arg;
  }
  CASE("Prayer5") {
	  mob_proto[i].mob_specials.prayers[MOB_SKILL_FIVE] = num_arg;
  }
  CASE("MountType") {
	  mob_proto[i].mob_specials.mount_type = num_arg;
  }
  if (!matched) {
    fprintf(stderr, "SYSERR: Warning: unrecognized espec keyword %s in mob #%d\n",
	    keyword, nr);
  }    
}

#undef CASE
#undef RANGE

void parse_espec(char *buf, int i, int nr)
{
  char *ptr;

  if ((ptr = strchr(buf, ':')) != NULL) {
    *(ptr++) = '\0';
    while (isspace(*ptr))
      ptr++;
  } else
    ptr = "";

  interpret_espec(buf, ptr, i, nr);
}


void parse_enhanced_mob(FILE *mob_f, int i, int nr)
{
  char line[256];

  parse_simple_mob(mob_f, i, nr);

  while (get_line(mob_f, line)) {
    if (!strcmp(line, "E"))	/* end of the ehanced section */
      return;
    else if (*line == '#') {	/* we've hit the next mob, maybe? */
      fprintf(stderr, "SYSERR: Unterminated E section in mob #%d\n", nr);
      exit(1);
    } else
      parse_espec(line, i, nr);
  }

  fprintf(stderr, "SYESRR: Unexpected end of file reached after mob #%d\n", nr);
  exit(1);
}


void parse_mobile(FILE *mob_f, int nr)
{
  static int i = 0;
  int j, t[10];
  char line[256], *tmpptr, letter;
  char f1[128], f2[128], f3[128], f4[128], f5[128], f6[128], f7[128], f8[128];

  mob_index[i].virtual = nr;
  mob_index[i].number = 0;
  mob_index[i].func = NULL;

  clear_char(mob_proto + i);

  mob_proto[i].player_specials = &dummy_mob;
  sprintf(buf2, "mob vnum %d", nr);

  /***** String data *****/
  mob_proto[i].player.name = fread_string(mob_f, buf2);
  tmpptr = mob_proto[i].player.short_descr = fread_string(mob_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);
  mob_proto[i].player.long_descr = fread_string(mob_f, buf2);
  mob_proto[i].player.description = fread_string(mob_f, buf2);
  mob_proto[i].player.title = NULL;

  /* *** Numeric data *** */
  if ((!get_line(mob_f, line)) || (sscanf(line, "%s %s %s %s %s %s %s %s %d %c", f1, f2, f3, f4, f5, f6, f7, f8, t + 2, &letter) != 10)) {
    fprintf(stderr, "SYSERR: Format error after string section of mob #%d\n"
		    "...expecting line of form '# # # # # # # # # {S | E}'\n", nr);
    exit(1);
  }
  MOB_FLAGS(mob_proto + i)[0] = asciiflag_conv(f1);
  MOB_FLAGS(mob_proto + i)[1] = asciiflag_conv(f2);
  MOB_FLAGS(mob_proto + i)[2] = asciiflag_conv(f3);
  MOB_FLAGS(mob_proto + i)[3] = asciiflag_conv(f4);
  SET_BIT_AR(MOB_FLAGS(mob_proto + i), MOB_ISNPC);
  AFF_FLAGS(mob_proto + i)[0] = asciiflag_conv(f5);
  AFF_FLAGS(mob_proto + i)[1] = asciiflag_conv(f6);
  AFF_FLAGS(mob_proto + i)[2] = asciiflag_conv(f7);
  AFF_FLAGS(mob_proto + i)[3] = asciiflag_conv(f8);
  GET_ALIGNMENT(mob_proto + i) = t[2];

  switch (UPPER(letter)) {
  case 'S':	/* Simple monsters */
    parse_simple_mob(mob_f, i, nr);
    break;
  case 'E':	/* Circle3 Enhanced monsters */
    parse_enhanced_mob(mob_f, i, nr);
    break;
	/*
  case 'F':
	  if ((!get_line(mob_f, line)) || (sscanf(line, "Skills: %s %s %s %s %s %s %s %s %s %s", f1, f2, f3, f4, f5, f6, f7, f8, f9, f10) != 10)) {
		  mob_proto[i].mob_specials.skills[MOB_SKILL_ONE] = asciiflag_conv(f1);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_TWO] = asciiflag_conv(f2);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_THREE] = asciiflag_conv(f3);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_FOUR] = asciiflag_conv(f4);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_FIVE] = asciiflag_conv(f5);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_SIX] = asciiflag_conv(f6);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_SEVEN] = asciiflag_conv(f7);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_EIGHT] = asciiflag_conv(f8);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_NINE] = asciiflag_conv(f9);
		  mob_proto[i].mob_specials.skills[MOB_SKILL_TEN] = asciiflag_conv(f10);
	  }
	break;*/
/*
  case 'F':
	  while(get_line(mob_f, line)) {
       tag_argument(line, tag);
       num = atoi(line);
	  }
	  if(!strcmp(tag, "F")) {
		  do {
			  get_line(mob_f, line);
			  sscanf(line, "%d %d", &num, &num2);
			  if(num != 0)
				  SET_MOB_SKILL(mob_proto + i, num, num2);
		  } while (num != 0);
	  }
  break;
  case 'G':
	  while(get_line(mob_f, line)) {
       tag_argument(line, tag);
       num = atoi(line);
	  }
	  if(!strcmp(tag, "G")) {
		  do {
			  get_line(mob_f, line);
			  sscanf(line, "%d %d", &num, &num2);
			  if(num != 0)
				  SET_MOB_SPELL(mob_proto + i, num, num2);
		  } while (num != 0);
	  }
  break;
  case 'H':
	  while(get_line(mob_f, line)) {
       tag_argument(line, tag);
       num = atoi(line);
	  }
	  if(!strcmp(tag, "H")) {
		  do {
			  get_line(mob_f, line);
			  sscanf(line, "%d %d", &num, &num2);
			  if(num != 0)
				  SET_MOB_CHANT(mob_proto + i, num, num2);
		  } while (num != 0);
	  }
  break;
  case 'I':
	  while(get_line(mob_f, line)) {
       tag_argument(line, tag);
       num = atoi(line);
	  }
	  if(!strcmp(tag, "I")) {
		  do {
			  get_line(mob_f, line);
			  sscanf(line, "%d %d", &num, &num2);
			  if(num != 0)
				  SET_MOB_PRAYER(mob_proto + i, num, num2);
		  } while (num != 0);
	  }
  break;
*/
  /* add new mob types here.. */
  default:
 //   parse_simple_mob(mob_f, i, nr);
/* lets see if it works */
    fprintf(stderr, "SYSERR: Unsupported mob type '%c' in mob #%d\n", letter, nr);
    exit(1); 
//    break;
  }
  /* DG triggers -- script info follows mob S/E section */
  letter = fread_letter(mob_f);
  ungetc(letter, mob_f);
  while (letter=='T') {
    dg_read_trigger(mob_f, &mob_proto[i], MOB_TRIGGER);
    letter = fread_letter(mob_f);
    ungetc(letter, mob_f);
  }

  mob_proto[i].aff_stats = mob_proto[i].real_stats;

  for (j = 0; j < NUM_WEARS; j++)
    mob_proto[i].equipment[j] = NULL;

  mob_proto[i].nr = i;
  mob_proto[i].desc = NULL;

  top_of_mobt = i++;
}

void tag_argument(char *argument, char *tag)
{
  char *tmp = argument, *ttag = tag, *wrt = argument;
  int i;

  for(i = 0; i < 4; i++)
    *(ttag++) = *(tmp++);
  *ttag = '\0';
  
  while(*tmp == ':' || *tmp == ' ')
    tmp++;

  while(*tmp)
    *(wrt++) = *(tmp++);
  *wrt = '\0';
}

/* read all objects from obj file; generate index and prototypes */
char *parse_object(FILE * obj_f, int nr)
{
  static int i = 0, retval;
  static char line[256];
  int t[15], j;
  char *tmpptr;
  char f1[256], f2[256], f3[256], f4[256], f5[256], f6[256], f7[256], f8[256];
  struct extra_descr_data *new_descr;

  obj_index[i].virtual = nr;
  obj_index[i].number = 0;
  obj_index[i].func = NULL;

  clear_object(obj_proto + i);
  obj_proto[i].in_room = NOWHERE;
  obj_proto[i].item_number = i;

  sprintf(buf2, "object #%d", nr);

  /* *** string data *** */
  if ((obj_proto[i].name = fread_string(obj_f, buf2)) == NULL) {
    fprintf(stderr, "SYSERR: Null obj name or format error at or near %s\n", buf2);
    exit(1);
  }
  tmpptr = obj_proto[i].short_description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    if (!str_cmp(fname(tmpptr), "a") || !str_cmp(fname(tmpptr), "an") ||
	!str_cmp(fname(tmpptr), "the"))
      *tmpptr = LOWER(*tmpptr);

  tmpptr = obj_proto[i].description = fread_string(obj_f, buf2);
  if (tmpptr && *tmpptr)
    *tmpptr = UPPER(*tmpptr);
  obj_proto[i].action_description = fread_string(obj_f, buf2);

  /* *** numeric data *** */
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, " %d %s %s %s %s %s %s %s %s",
                  t, f1, f2, f3, f4, f5, f6, f7 , f8)) != 9) {
    fprintf(stderr, "Format error in first numeric line (expecting 9 args, got %d), %s\n", retval, buf2);
    exit(1);
  }

  obj_proto[i].obj_flags.type_flag = t[0];
  obj_proto[i].obj_flags.extra_flags[0] = asciiflag_conv(f1);
  obj_proto[i].obj_flags.extra_flags[1] = asciiflag_conv(f2);
  obj_proto[i].obj_flags.extra_flags[2] = asciiflag_conv(f3);
  obj_proto[i].obj_flags.extra_flags[3] = asciiflag_conv(f4);
  obj_proto[i].obj_flags.wear_flags[0] = asciiflag_conv(f5);
  obj_proto[i].obj_flags.wear_flags[1] = asciiflag_conv(f6);
  obj_proto[i].obj_flags.wear_flags[2] = asciiflag_conv(f7);
  obj_proto[i].obj_flags.wear_flags[3] = asciiflag_conv(f8);

  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d %d %d %d %d %d",
                       t, t + 1, t + 2, t + 3, t+4, t+5, t+6, t+7)) != 8) {
    fprintf(stderr, "SYSERR: Format error in second numeric line (expecting 8 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.value[0] = t[0];
  obj_proto[i].obj_flags.value[1] = t[1];
  obj_proto[i].obj_flags.value[2] = t[2];
  obj_proto[i].obj_flags.value[3] = t[3];
  obj_proto[i].obj_flags.bitvector[0] = t[4];
  obj_proto[i].obj_flags.bitvector[1] = t[5];
  obj_proto[i].obj_flags.bitvector[2] = t[6];
  obj_proto[i].obj_flags.bitvector[3] = t[7];

  t[3]=0; /* This will be minlevel - default it to zero. */
  t[4]=-1; /* clan default is -1 */
  t[5]=-1; /* player num default is -1 */
  t[6]=-1; /* vamp_wear default */
  t[7]=-1; /* titan_wear default */
  t[8]=-1; /* saint_wear default */
  t[9]=-1; /* demon_wear default */
  t[10]=-1; /* obj bid num default to 0 */
  t[11]=-1; /* obj bid amount */
  t[12]=0; /* auction timer */
  t[13]=-1; /* Player who is bidding on obj */
  t[14]=-1; /* Seller of obj */
  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", t, t + 1, t + 2, t + 3, t + 4, t + 5, t + 6, t + 7, t + 8, t + 9, t + 10, t + 11, t + 12, t + 13, t + 14)) < 4) {
    fprintf(stderr, "SYSERR: Format error in third numeric
            line (expecting 3 or 4 args, got %d), %s\n", retval, buf2);
    exit(1);
  }
  obj_proto[i].obj_flags.weight = t[0];
  obj_proto[i].obj_flags.cost = t[1];
  obj_proto[i].obj_flags.cost_per_day = t[2];
  obj_proto[i].obj_flags.minlevel = t[3];
  obj_proto[i].obj_flags.clan = t[4];
  obj_proto[i].obj_flags.player = t[5];
  obj_proto[i].obj_flags.vamp_wear = t[6];
  obj_proto[i].obj_flags.titan_wear = t[7];
  obj_proto[i].obj_flags.saint_wear = t[8];
  obj_proto[i].obj_flags.demon_wear = t[9];
  obj_proto[i].obj_flags.obj_bid_num = t[10];
  obj_proto[i].obj_flags.bid_amnt = t[11];
  obj_proto[i].obj_flags.auc_timer = t[12];
  obj_proto[i].obj_flags.auc_player = t[13];
  obj_proto[i].obj_flags.auc_seller = t[14];


  if (!get_line(obj_f, line) ||
      (retval = sscanf(line, "%d %d %d 0 0 0 0 0 0", t, t + 1, t + 2)) < 1) {
    fprintf(stderr, "SYSERR: Format error in fourth numeric
            line (expecting 3 arg, got %d), %s\n", retval, buf2);
    exit(1);
  } 

  obj_proto[i].obj_flags.timer = t[0];
  obj_proto[i].obj_flags.material = t[1];  
  obj_proto[i].obj_flags.racial_bonus = t[2];

  /* check to make sure that weight of containers exceeds curr. quantity */
  if (obj_proto[i].obj_flags.type_flag == ITEM_DRINKCON ||
      obj_proto[i].obj_flags.type_flag == ITEM_FOUNTAIN) {
    if (obj_proto[i].obj_flags.weight < obj_proto[i].obj_flags.value[1])
      obj_proto[i].obj_flags.weight = obj_proto[i].obj_flags.value[1] + 5;
  }

  /* *** extra descriptions and affect fields *** */

  for (j = 0; j < MAX_OBJ_AFFECT; j++) {
    obj_proto[i].affected[j].location = APPLY_NONE;
    obj_proto[i].affected[j].modifier = 0;
  }

  strcat(buf2, ", after numeric constants\n..."
               "expecting 'E', 'A', 'C', '$', or next object number");
  j = 0;

  for (;;) {
    if (!get_line(obj_f, line)) {
      fprintf(stderr, "SYSERR: Format error in %s\n", buf2);
      exit(1);
    }
    switch (*line) {
    case 'E':
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->keyword = fread_string(obj_f, buf2);
      new_descr->description = fread_string(obj_f, buf2);
      new_descr->next = obj_proto[i].ex_description;
      obj_proto[i].ex_description = new_descr;
      break;
    case 'A':
      if (j >= MAX_OBJ_AFFECT) {
	fprintf(stderr, "SYSERR: Too many A fields (%d max), %s\n", MAX_OBJ_AFFECT, buf2);
	exit(1);
      }
      if (!get_line(obj_f, line) || (sscanf(line, " %d %d ", t, t + 1) != 2)) {
	fprintf(stderr, "SYSERR: Format error in 'A' field, %s\n", buf2);
	fprintf(stderr, "...offending line: '%s'\n", line);
      }
      obj_proto[i].affected[j].location = t[0];
      obj_proto[i].affected[j].modifier = t[1];
      j++;
      break;
    case 'C': /* Weapon Spells */
      get_line(obj_f, line);
      sscanf(line, "%d %d", t, t+1);
      obj_proto[i].obj_flags.spell=t[0];
      obj_proto[i].obj_flags.spell_extra=t[1];
      break;
    case 'T':  /* DG triggers */
      dg_obj_trigger(line, &obj_proto[i]);
      break;
    case '$':
    case '#':
      top_of_objt = i++;
      return line;
      break;
    default:
      fprintf(stderr, "SYSERR: Format error in %s\n", buf2);
      exit(1);
      break;
    }
  }
}


#define Z	zone_table[zone]

/* load the zone table and command tables */
void load_zones(FILE * fl, char *zonename)
{
  static int zone = 0;
  int cmd_no = 0, num_of_cmds = 0, line_num = 0, tmp, error, arg_num;
  char *ptr, buf[256], zname[256];
  char t1[80], t2[80];

  strcpy(zname, zonename);

  while (get_line(fl, buf))
    num_of_cmds++;		/* this should be correct within 3 or so */
  rewind(fl);

  if (num_of_cmds == 0) {
    fprintf(stderr, "SYSERR: %s is empty!\n", zname);
    exit(0);
  } else
    CREATE(Z.cmd, struct reset_com, num_of_cmds);

  line_num += get_line(fl, buf);

  if (sscanf(buf, "#%d", &Z.number) != 1) {
    fprintf(stderr, "SYSERR: Format error in %s, line %d\n", zname, line_num);
    exit(0);
  }
  sprintf(buf2, "beginning of zone #%d", Z.number);

  line_num += get_line(fl, buf);
  if ((ptr = strchr(buf, '~')) != NULL)	/* take off the '~' if it's there */
    *ptr = '\0';
  Z.name = str_dup(buf);

  line_num += get_line(fl, buf);
  Z.development=0;
  Z.owner=-1;
  if (sscanf(buf, " %d %d %d %d %d",
             &Z.top, &Z.lifespan, &Z.reset_mode, &Z.development, &Z.owner) < 3) {
    fprintf(stderr, "SYSERR: Format error in 3-constant line of %s", zname);
    exit(1);
  }
  cmd_no = 0;

  for (;;) {
    if ((tmp = get_line(fl, buf)) == 0) {
      fprintf(stderr, "Format error in %s - premature end of file\n", zname);
      exit(1);
    }
    line_num += tmp;
    ptr = buf;
    skip_spaces(&ptr);

    if ((ZCMD.command = *ptr) == '*')
      continue;

    ptr++;

    if (ZCMD.command == 'S' || ZCMD.command == '$') {
      ZCMD.command = 'S';
      break;
    }
    error = 0;
    if (strchr("D", ZCMD.command) != NULL) {	/* ### */
      if ((arg_num = sscanf(ptr, " %d %d %d %d", &tmp, &ZCMD.arg1,
                 &ZCMD.arg2, &ZCMD.arg3)) != 4)
	error = 1;
    } else if (strchr("RT", ZCMD.command) != NULL) { 
      if (sscanf(ptr, " %d %d %d ", &tmp, &ZCMD.arg1,
                 &ZCMD.arg2) != 3)
        error = 1;
    } else if (strchr("G", ZCMD.command) != NULL) { 
      if ((arg_num = sscanf(ptr, " %d %d %d %d ", &tmp, &ZCMD.arg1,
           &ZCMD.arg2, &ZCMD.arg3)) != 4) {
        if (arg_num != 3)
          error = 1;
        else
          ZCMD.arg3 = 0;
      }
      else if (ZCMD.arg3 < 0)
        ZCMD.arg3 = 0;
    } else if (ZCMD.command=='V') { /* a string-arg command */ 
      if (sscanf(ptr, " %d %d %d %d %s %s", &tmp, &ZCMD.arg1, &ZCMD.arg2,
		 &ZCMD.arg3, t1, t2) != 6)
	error = 1;
      else {
        ZCMD.sarg1 = str_dup(t1);
        ZCMD.sarg2 = str_dup(t2);
      }
    } else {
      if ((arg_num = sscanf(ptr, " %d %d %d %d %d ", &tmp, &ZCMD.arg1,
           &ZCMD.arg2, &ZCMD.arg3, &ZCMD.arg4)) != 5) {
        error = 1;
        if (arg_num != 4)
          error = 1;
        else
          ZCMD.arg4 = 0;
      }
      else if (ZCMD.arg4 < 0)
        ZCMD.arg4 = 0;
    } 

    ZCMD.if_flag = tmp;

    if (error) {
      fprintf(stderr, "SYSERR: Format error in %s, line %d: '%s'\n", zname, line_num, buf);
      exit(1);
    }
    ZCMD.line = line_num;
    cmd_no++;
  }

  top_of_zone_table = zone++;
}

#undef Z


void get_one_line(FILE *fl, char *buf)
{
  if (fgets(buf, READ_SIZE, fl) == NULL) {
    log("SYSERR: error reading help file: not terminated with $?");
    exit(1);
  }

  buf[strlen(buf) - 1] = '\0'; /* take off the trailing \n */
}


void load_help(FILE *fl)
{
  char key[READ_SIZE+1], next_key[READ_SIZE+1], entry[32384];
  char line[READ_SIZE+1], *scan;
  struct help_index_element el;

  /* Get the min level for the help in this file */
  fscanf(fl, "%d\n", &el.level);

  /* get the first keyword line */
  get_one_line(fl, key);
  while (*key != '$') {
    /* read in the corresponding help entry */
    strcpy(entry, strcat(key, "\r\n"));
    get_one_line(fl, line);
    while (*line != '#') {
      strcat(entry, strcat(line, "\r\n"));
      get_one_line(fl, line);
    }

    /* now, add the entry to the index with each keyword on the keyword line */
    el.duplicate = 0;
    el.entry = str_dup(entry);
    scan = one_word(key, next_key);
    while (*next_key) {
      el.keyword = str_dup(next_key);
      help_table[top_of_helpt++] = el;
      el.duplicate++;
      scan = one_word(scan, next_key);
    }

    /* get next keyword line (or $) */
    get_one_line(fl, key);
  }
}


int hsort(const void *a, const void *b)
{
  struct help_index_element *a1, *b1;

  a1 = (struct help_index_element *) a;
  b1 = (struct help_index_element *) b;

  return (str_cmp(a1->keyword, b1->keyword));
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time	 	 *
*************************************************************************/

int mountt_clear_deleted_types(int id)
{
	int i = 0;
	for (i = 0; i <= top_of_mobt; i++)
		if (mob_proto[i].mob_specials.mount_type == id)
			mob_proto[i].mob_specials.mount_type = 0;
	return TRUE;
}

int vnum_mobile(char *searchname, struct char_data * ch)
{
  int nr, found = 0;

  for (nr = 0; nr <= top_of_mobt; nr++) {
    if (isname(searchname, mob_proto[nr].player.name)) {
      sprintf(buf, "%s%3d. [%5d] %s\r\n", buf, ++found,
	      mob_index[nr].virtual,
	      mob_proto[nr].player.short_descr);
    }
  }
  page_string(ch->desc, buf, 1);
  return (found);
}
#define MAX_NR_ITEMS_LIST		20   /* value x 100 = max items that
                                      * will be listed by vwear and
									  * vnum.
									  */
void vwear_itemtype(int type, struct char_data * ch)
{
  int nr, found = 0;
  char affs[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char output[LARGE_BUFSIZE][MAX_NR_ITEMS_LIST];
  int i = 0, bufs_used = 1, objs_in_buf = 0;
  bool output_written = FALSE;
  bool found_objs = FALSE;
  *affs = '\0';

  for(i = 0; i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++)
  {
	  if (GET_OBJ_TYPE(&obj_proto[nr]) == type)
	  {
		  found_objs = TRUE;
		  output_written = FALSE;
		  for(i = 0;i < bufs_used;i++)
		  {
			  if (output_written == TRUE)
				  continue;
			  else if (objs_in_buf >= 100)
			  {
				  objs_in_buf = 0;
				  bufs_used++;
				  continue;
			  }
			  else {
				  objs_in_buf++;
				  output_written = TRUE;
				  sprintf(name, "%s", obj_proto[nr].short_description);
				  colorless_str(name);
				  sprintf(output[i], "%s%3d. [%5d] (%3d) %-40.40s", output[i], ++found,
					  obj_index[nr].virtual,
					  obj_proto[nr].obj_flags.minlevel, name);
				  sprintbitarray(obj_proto[nr].obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, affs);
				  sprintf(output[i], "%s     /c0%s\r\n", output[i], affs);
			  }
		  }
	  }
  }
  if (found_objs == FALSE)
  {
	  send_to_char("No items of that type.\r\n", ch);
	  return;
  }
  page_string(ch->desc, output[0], 0);
  for(i = 0;i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';
  return;
}

void vwear_wearpos(int wearpos, struct char_data * ch)
{
  int nr, found = 0;
  char affs[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char output[LARGE_BUFSIZE][MAX_NR_ITEMS_LIST];
  int i = 0, bufs_used = 1, objs_in_buf = 0;
  bool output_written = FALSE;
  bool found_objs = FALSE;
  *affs = '\0';

  for(i = 0; i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++)
  {
	  if (CAN_WEAR(&obj_proto[nr], wearpos))
	  {
		  found_objs = TRUE;
		  output_written = FALSE;
		  for(i = 0;i < bufs_used;i++)
		  {
			  if (output_written == TRUE)
				  continue;
			  else if (objs_in_buf >= 100)
			  {
				  objs_in_buf = 0;
				  bufs_used++;
				  continue;
			  }
			  else {
				  objs_in_buf++;
				  output_written = TRUE;
				  sprintf(name, "%s", obj_proto[nr].short_description);
				  colorless_str(name);
				  sprintf(output[i], "%s%3d. [%5d] (%3d) %-40.40s", output[i], ++found,
					  obj_index[nr].virtual,
					  obj_proto[nr].obj_flags.minlevel, name);
				  sprintbitarray(obj_proto[nr].obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, affs);
				  sprintf(output[i], "%s     /c0%s\r\n", output[i], affs);
			  }
		  }
	  }
  }
  if (found_objs == FALSE)
  {
	  send_to_char("No items on that wearpos.\r\n", ch);
	  return;
  }
  page_string(ch->desc, output[0], 0);
  for(i = 0;i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';
  return;
}

bool illegal_affection(int affection)
{
	if (!affection)
		return FALSE;
	else {
		switch (affection) {
		case AFF_BALANCE:
		case AFF_INDESTR_AURA:
		case AFF_SANCTUARY:
		case AFF_HASTE:
		case AFF_INCOGNITO:
		case AFF_SNEAK:
		case AFF_DARKWARD:
		case AFF_BLAZEWARD:
		case AFF_ANCIENT_PROT:
		case AFF_BERSERK:
		case AFF_DIVINE_NIMBUS:
		case AFF_FIRESHIELD:
		case AFF_CHAOSARMOR:
		case AFF_PHANTOM_ARMOR:
		case AFF_HIDE:
		case AFF_RUN:
		case AFF_MENTAL_BARRIER:
		case AFF_MIST_FORM:
		case AFF_DEFLECT:
		case AFF_HEAVY_SKIN:
		case AFF_SEALED:
		case AFF_VAMP_AURA:
		case AFF_HOVER:
		case AFF_HISS:
		case AFF_MESMERIZED:
		case AFF_SHROUD:
		case AFF_BURROW:
		case AFF_DEATHCALL:
		case AFF_EARTH_SHROUD:
		case AFF_ABSORB:
		case AFF_CLOUD:
		case AFF_UNDER_ATTACK:
		case AFF_SPEAK:
		case AFF_AFTERLIFE:
		case AFF_DIVINE_SHIELD:
		case AFF_IMPEDE:
		case AFF_ETHEREAL:
		case AFF_ENLIGHT:
		case AFF_GAUGE:
		case AFF_NETHERBLAZE:
		case AFF_NETHERCLAW:
		case AFF_DECEIVE:
		case AFF_UNFURLED:
		case AFF_HOLYARMOR:
		case AFF_SOFTEN_FLESH:
		case AFF_CONFUSE:
		case AFF_WEAKENEDFLESH:
		case AFF_WARCRY:
		case AFF_AWE:
		case AFF_SHADOW:
		case AFF_SACRED_SHIELD:
		case AFF_AEGIS:
		case AFF_PROTFROMGOOD:
		case AFF_PROTECT_EVIL:
			return TRUE;
		break;
		default:
			return FALSE;
		}
		return FALSE;
	}
}

void vnum_illegal(struct char_data * ch)
{
  int nr, found = 0;
  char affs[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char output[LARGE_BUFSIZE][MAX_NR_ITEMS_LIST];
  int i = 0, bufs_used = 1, objs_in_buf = 0, j = 0;
  bool output_written = FALSE;
  bool found_objs = FALSE, ill_aff = FALSE;
  *affs = '\0';

  for(i = 0; i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++)
  {
	  ill_aff = FALSE;
	  for (j = 0; j < 150; j++)
	  {
		  if (!IS_SET_AR(obj_proto[nr].obj_flags.bitvector, j))
			  continue;
		  else {
			  if (ill_aff == FALSE)
			  {
				  if (illegal_affection(j) == TRUE)
					  ill_aff = TRUE;
			  } else
				  continue;
		  }
	  }
	  if (ill_aff == TRUE)
	  {
		  found_objs = TRUE;
		  output_written = FALSE;
		  for(i = 0;i < bufs_used;i++)
		  {
			  if (output_written == TRUE)
				  continue;
			  else if (objs_in_buf >= 100)
			  {
				  objs_in_buf = 0;
				  bufs_used++;
				  continue;
			  }
			  else {
				  objs_in_buf++;
				  output_written = TRUE;
				  sprintf(name, "%s", obj_proto[nr].short_description);
				  colorless_str(name);
				  sprintf(output[i], "%s%3d. [%5d] (%3d) %-40.40s", output[i], ++found,
					  obj_index[nr].virtual,
					  obj_proto[nr].obj_flags.minlevel, name);
				  sprintbitarray(obj_proto[nr].obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, affs);
				  sprintf(output[i], "%s     /c0%s\r\n", output[i], affs);
			  }
		  }
	  }
  }
  if (found_objs == FALSE)
  {
	  send_to_char("No items with illegal affections.\r\n", ch);
	  return;
  }
  page_string(ch->desc, output[0], 0);
  for(i = 0;i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';
  return;
}

#define LOAD_CARDS		1
#define LOAD_LETTERS	2
#define LOAD_IMMORTEQ	3

int special_loading(byte type)
{
  int nr = 0, found = 0, low = 0, hi = 0;
  struct obj_data *sobj = NULL;
  if (!type)
	  return 0;
  else if (type == LOAD_CARDS)
  {
	  low = 26040;
	  hi  = 26091;
  }
  else if (type == LOAD_LETTERS)
  {
	  low = 26193;
	  hi  = 26199;
  }
  else if (type == LOAD_IMMORTEQ)
  {
	  low = 26200;
	  hi  = 26599;
  }
  for (nr = 0, sobj = object_list; sobj; sobj = sobj->next)
	  if (GET_OBJ_VNUM(sobj) >= low && GET_OBJ_VNUM(sobj) <= hi)
	  {
		  if (sobj->in_room > NOWHERE && !sobj->carried_by)
			  found++;
	  }
  sobj = NULL;
  return (found);
}

int vnum_object(char *searchname, struct char_data * ch)
{
  int nr, found = 0;
  char affs[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char output[LARGE_BUFSIZE][MAX_NR_ITEMS_LIST];
  int i = 0, bufs_used = 1, objs_in_buf = 0;
  bool output_written = FALSE;
  bool found_objs = FALSE;
  *affs = '\0';

  for(i = 0; i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++)
  {
	  if (isname(searchname, obj_proto[nr].name))
	  {
		  found_objs = TRUE;
		  output_written = FALSE;
		  for(i = 0;i < bufs_used;i++)
		  {
			  if (output_written == TRUE)
				  continue;
			  else if (objs_in_buf >= 100)
			  {
				  objs_in_buf = 0;
				  bufs_used++;
				  continue;
			  }
			  else {
				  objs_in_buf++;
				  output_written = TRUE;
				  sprintf(name, "%s", obj_proto[nr].short_description);
				  colorless_str(name);
				  sprintf(output[i], "%s%3d. [%5d] (%3d) %-40.40s", output[i], ++found,
					  obj_index[nr].virtual,
					  obj_proto[nr].obj_flags.minlevel, name);
				  sprintbitarray(obj_proto[nr].obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, affs);
				  sprintf(output[i], "%s     /c0%s\r\n", output[i], affs);
			  }
		  }
	  }
  }
  if (found_objs == FALSE)
  {
	  send_to_char("No items with that name.\r\n", ch);
	  return (TRUE);
  }
  page_string(ch->desc, output[0], 0);
  for(i = 0;i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';
  return (TRUE);
}


/* create a character, and add it to the char list */
struct char_data *create_char(void)
{
  struct char_data *ch;

  CREATE(ch, struct char_data, 1);
  clear_char(ch);
  ch->next = character_list;
  character_list = ch;
  GET_ID(ch) = max_id++;

  return ch;
}


/* create a new mobile from a prototype */
struct char_data *read_mobile(int nr, int type)
{
  int i;
  struct char_data *mob;

  if (type == VIRTUAL) {
    if ((i = real_mobile(nr)) < 0) {
      sprintf(buf, "Mobile (V) %d does not exist in database.", nr);
      log(buf);
      return NULL;
    }
  } else
    i = nr;

  CREATE(mob, struct char_data, 1);
  clear_char(mob);
  *mob = mob_proto[i];
  mob->next = character_list;
  character_list = mob;

  if (GET_LEVEL(mob) < 10)
    mob->points.max_hit = number(GET_LEVEL(mob) * 8, GET_LEVEL(mob) * 10)+1;
  else if (GET_LEVEL(mob) < 25)
    mob->points.max_hit = number(GET_LEVEL(mob) * 10, GET_LEVEL(mob) * 22);
  else if (GET_LEVEL(mob) < 50)
    mob->points.max_hit = number(GET_LEVEL(mob) * 22, GET_LEVEL(mob) * 30);
  else if (GET_LEVEL(mob) < 75)
    mob->points.max_hit = number(GET_LEVEL(mob) * 30, GET_LEVEL(mob) * 40);
  else if (GET_LEVEL(mob) < 100)
    mob->points.max_hit = number(GET_LEVEL(mob) * 40, GET_LEVEL(mob) * 45);
  else if (GET_LEVEL(mob) < 125)
    mob->points.max_hit = number(GET_LEVEL(mob) * 45, GET_LEVEL(mob) * 50);
  else if (GET_LEVEL(mob) < 150)
    mob->points.max_hit = number(GET_LEVEL(mob) * 50, GET_LEVEL(mob) * 55);
  else if (GET_LEVEL(mob) < 175)
    mob->points.max_hit = number(GET_LEVEL(mob) * 58, GET_LEVEL(mob) * 70);
  else
    mob->points.max_hit = number(GET_LEVEL(mob) * 68, GET_LEVEL(mob) * 74);

  mob->points.max_hit *= 1.2;
  mob->points.hit  = mob->points.max_hit;
  mob->points.mana = mob->points.max_mana;
  mob->points.move = mob->points.max_move;
  mob->points.qi   = mob->points.max_qi  = 100;
  mob->points.vim  = mob->points.max_vim = 100;
  mob->points.aria = mob->points.max_aria = 100;

  mob->player.time.birth = time(0);
  mob->player.time.played = 0;
  mob->player.time.logon = time(0);

  mob_index[i].number++;
  GET_ID(mob) = max_id++;
  assign_triggers(mob, MOB_TRIGGER);

  return mob;
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
  struct obj_data *obj;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  obj->next = object_list;
  object_list = obj;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);
  GET_OBJ_VROOM(obj) = NOWHERE;
  GET_OBJ_TIMER(obj) = 0;
  return obj;
}


/* create a new object from a prototype */
struct obj_data *read_object(int nr, int type)
{
  struct obj_data *obj;
  int i;

  if (nr < 0) {
    log("SYSERR: trying to create obj with negative num!");
    return NULL;
  }
  if (type == VIRTUAL) {
    if ((i = real_object(nr)) < 0) {
      sprintf(buf, "Object (V) %d does not exist in database.", nr);
      log(buf);
      return NULL;
    }
  } else
    i = nr;

  CREATE(obj, struct obj_data, 1);
  clear_object(obj);
  *obj = obj_proto[i];
  obj->next = object_list;
  object_list = obj;

  obj_index[i].number++;
  GET_ID(obj) = max_id++;
  assign_triggers(obj, OBJ_TRIGGER);

  return obj;
}



#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
  int i;
  struct reset_q_element *update_u, *temp;
  static int timer = 0;
  char buf[128];

  /* jelson 10/22/92 */
  if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
    /* one minute has passed */
    /*
     * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
     * factor of 60
     */

    timer = 0;

    /* since one minute has passed, increment zone ages */
    for (i = 0; i <= top_of_zone_table; i++) {
      if (zone_table[i].age < zone_table[i].lifespan &&
	  zone_table[i].reset_mode)
	(zone_table[i].age)++;

      if (zone_table[i].age >= zone_table[i].lifespan &&
	  zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
	/* enqueue zone */

	CREATE(update_u, struct reset_q_element, 1);

	update_u->zone_to_reset = i;
	update_u->next = 0;

	if (!reset_q.head)
	  reset_q.head = reset_q.tail = update_u;
	else {
	  reset_q.tail->next = update_u;
	  reset_q.tail = update_u;
	}

	zone_table[i].age = ZO_DEAD;
      }
    }
  }	/* end - one minute has passed */


  /* dequeue zones (if possible) and reset */
  /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
  for (update_u = reset_q.head; update_u; update_u = update_u->next)
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	is_empty(update_u->zone_to_reset)) {
      reset_zone(update_u->zone_to_reset);
      sprintf(buf, "Auto zone reset: %s",
	      zone_table[update_u->zone_to_reset].name);
      mudlog(buf, CMP, LVL_IMPL + 1, FALSE);
      /* dequeue */
      if (update_u == reset_q.head)
	reset_q.head = reset_q.head->next;
      else {
	for (temp = reset_q.head; temp->next != update_u;
	     temp = temp->next);

	if (!update_u->next)
	  reset_q.tail = temp;

	temp->next = update_u->next;
      }

      free(update_u);
      break;
    }
}

void log_zone_error(int zone, int cmd_no, char *message)
{
  char buf[256];

  sprintf(buf, "SYSERR: zone file: %s", message);
  mudlog(buf, NRM, LVL_ADMIN, TRUE);

  sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
	  ZCMD.command, zone_table[zone].number, ZCMD.line);
  mudlog(buf, NRM, LVL_ADMIN, TRUE);
}

#define ZONE_ERROR(message) \
	{ log_zone_error(zone, cmd_no, message); last_cmd = 0; }

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
  int cmd_no, last_cmd = 0;
  int mob_load = FALSE; /* ### */
  int obj_load = FALSE; /* ### */
  struct char_data *mob = NULL;
  struct obj_data *obj, *obj_to;
  int room_vnum, room_rnum;
  struct char_data *tmob=NULL; /* for trigger assignment */
  struct obj_data *tobj=NULL;  /* for trigger assignment */

  for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {

    if (ZCMD.if_flag && !last_cmd && !mob_load && !obj_load)
      continue;

    if (!ZCMD.if_flag) { /* ### */
      mob_load = FALSE;
      obj_load = FALSE;
    }

    switch (ZCMD.command) {
    case '*':			/* ignore command */
      last_cmd = 0;
      break;

    case 'M':			/* read a mobile */
      if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) &&
          (number(1, 100) >= ZCMD.arg4)) {
	mob = read_mobile(ZCMD.arg1, REAL);
	char_to_room(mob, ZCMD.arg3);
        load_mtrigger(mob);
        tmob = mob;
	last_cmd = 1;
        mob_load = TRUE;
      } else
	last_cmd = 0;
      tobj = NULL;
      break;

    case 'O':			/* read an object */
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          (number(1, 100) >= load_percent(ZCMD.arg4))) {
	if (ZCMD.arg3 >= 0) {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj_to_room(obj, ZCMD.arg3);
          load_otrigger(obj);
          tobj = obj;
	  last_cmd = 1;
          obj_load = TRUE;
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj->in_room = NOWHERE;
          tobj = obj;
	  last_cmd = 1;
	}
      } else
	last_cmd = 0;
      tmob = NULL;
      break;

    case 'P':			/* object to object */
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          obj_load && (number(1, 100) >= load_percent(ZCMD.arg4))) {
	obj = read_object(ZCMD.arg1, REAL);
	if (!(obj_to = get_obj_num(ZCMD.arg3))) {
	  ZONE_ERROR("target obj not found");
	  break;
	}
	obj_to_obj(obj, obj_to);
        load_otrigger(obj);
        tobj = obj;
	last_cmd = 1;
      } else
	last_cmd = 0;
      tmob = NULL;
      break;

    case 'G':			/* obj_to_char */
      if (!mob) {
	ZONE_ERROR("attempt to give obj to non-existant mob");
	break;
      }
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          mob_load && (number(1, 100) >= load_percent(ZCMD.arg3))) {
	obj = read_object(ZCMD.arg1, REAL);
	obj_to_char(obj, mob);
        tobj = obj;
        load_otrigger(obj);
	last_cmd = 1;
      } else
	last_cmd = 0;
      tmob = NULL;
      break;

    case 'E':			/* object to equipment list */
      if (!mob) {
	ZONE_ERROR("trying to equip non-existant mob");
	break;
      }
      if ((obj_index[ZCMD.arg1].number < ZCMD.arg2) &&
          mob_load && (number(1, 100) >= load_percent(ZCMD.arg4))) {
	if (ZCMD.arg3 < 0 || ZCMD.arg3 >= NUM_WEARS) {
	  ZONE_ERROR("invalid equipment pos number");
	} else {
	  obj = read_object(ZCMD.arg1, REAL);
          IN_ROOM(obj) = IN_ROOM(mob);
          load_otrigger(obj);
          if (wear_otrigger(obj, mob, ZCMD.arg3) ) {
	    IN_ROOM(obj) = NOWHERE;
	    equip_char(mob, obj, ZCMD.arg3);
          } else
            obj_to_char(obj, mob);
          tobj = obj;
	  last_cmd = 1;
	}
      } else
	last_cmd = 0;
      tmob = NULL;
      break;

    case 'R': /* rem obj from room */
      if ((obj = get_obj_in_list_num(ZCMD.arg2, world[ZCMD.arg1].contents)) != NULL) {
        obj_from_room(obj);
        extract_obj(obj);
      }
      last_cmd = 1;
      tmob = NULL;
      tobj = NULL;
      break;


    case 'D':			/* set state of door */
      if (ZCMD.arg2 < 0 || ZCMD.arg2 >= NUM_OF_DIRS ||
	  (world[ZCMD.arg1].dir_option[ZCMD.arg2] == NULL)) {
	ZONE_ERROR("door does not exist");
      } else
	switch (ZCMD.arg3) {
	case 0:
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_CLOSED);
	  break;
	case 1:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		     EX_LOCKED);
	  break;
	case 2:
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_LOCKED);
	  SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
		  EX_CLOSED);
	  break;
	}
      last_cmd = 1;
      tmob = NULL;
      tobj = NULL;
      break;

    case 'T': /* trigger command; details to be filled in later */
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob))
          CREATE(SCRIPT(tmob), struct script_data, 1);
        add_trigger(SCRIPT(tmob), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj))
          CREATE(SCRIPT(tobj), struct script_data, 1);
        add_trigger(SCRIPT(tobj), read_trigger(real_trigger(ZCMD.arg2)), -1);
        last_cmd = 1;
      }
      break;

    case 'V':
      if (ZCMD.arg1==MOB_TRIGGER && tmob) {
        if (!SCRIPT(tmob)) {
          ZONE_ERROR("Attempt to give variable to scriptless mobile");
        } else
          add_var(&(SCRIPT(tmob)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==OBJ_TRIGGER && tobj) {
        if (!SCRIPT(tobj)) {
          ZONE_ERROR("Attempt to give variable to scriptless object");
        } else
          add_var(&(SCRIPT(tobj)->global_vars), ZCMD.sarg1, ZCMD.sarg2,
                  ZCMD.arg3);
        last_cmd = 1;
      } else if (ZCMD.arg1==WLD_TRIGGER) {
        if (ZCMD.arg2<0 || ZCMD.arg2>top_of_world) {
          ZONE_ERROR("Invalid room number in variable assignment");
        } else {
          if (!(world[ZCMD.arg2].script)) {
            ZONE_ERROR("Attempt to give variable to scriptless object");
          } else
            add_var(&(world[ZCMD.arg2].script->global_vars),
                    ZCMD.sarg1, ZCMD.sarg2, ZCMD.arg3);
          last_cmd = 1;
        }
      }
      break;
    default:
      ZONE_ERROR("unknown cmd in reset table; cmd disabled");
      ZCMD.command = '*';
      break;
    }
  }

  zone_table[zone].age = 0;

  /* handle reset_wtrigger's */
  room_vnum = zone_table[zone].number * 100;
  while (room_vnum <= zone_table[zone].top) {
    room_rnum = real_room(room_vnum);
    if (room_rnum != NOWHERE) reset_wtrigger(&world[room_rnum]);
    room_vnum++;
  }
}



/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (world[i->character->in_room].zone == zone_nr)
	return 0;

  return 1;
}





/*************************************************************************
*  stuff related to the save/load player system				 *
*************************************************************************/


long get_id_by_name(char *name)
{
  int i;

  one_argument(name, arg);
  for (i = 0; i <= top_of_p_table; i++)
    if (!strcmp((player_table + i)->name, arg))
      return ((player_table + i)->id);

  return -1;
}


char *get_name_by_id(long id)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++)
    if ((player_table + i)->id == id)
      return ((player_table + i)->name);

  return NULL;
}


/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u * char_element)
{
  int player_i;

  int find_name(char *name);

  if ((player_i = find_name(name)) >= 0) {
    fseek(player_fl, (long) (player_i * sizeof(struct char_file_u)), SEEK_SET);
    fread(char_element, sizeof(struct char_file_u), 1, player_fl);
    return (player_i);
  } else
    return (-1);
}




/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */
void save_char(struct char_data * ch, int load_room)
{
  struct char_file_u st;

  if (IS_NPC(ch) || !ch->desc || GET_PFILEPOS(ch) < 0)
    return;

  char_to_store(ch, &st);

  strncpy(st.host, ch->desc->host, HOST_LENGTH);
  st.host[HOST_LENGTH] = '\0';

  if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
    if (load_room == NOWHERE) {
      st.player_specials_saved.load_room = NOWHERE;
    } else
      st.player_specials_saved.load_room = world[load_room].number;
  }

  fseek(player_fl, GET_PFILEPOS(ch) * sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
  save_char_vars(ch);
}



/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u * st, struct char_data * ch)
{
  int i;

  /* to save memory, only PC's -- not MOB's -- have player_specials */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_RACE(ch) = st->race;
  GET_LEVEL(ch) = st->level;

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.title = str_dup(st->title);
  ch->player.description = str_dup(st->description);

  ch->player.hometown = st->hometown;
  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->real_stats = st->stats;
  ch->aff_stats = st->stats;
  ch->points = st->points;
  ch->char_specials.saved = st->char_specials_saved;
  ch->player_specials->saved = st->player_specials_saved;
  POOFIN(ch) = NULL;
  POOFOUT(ch) = NULL;
  for (i=0; i < MAX_ABILITIES; i++)
    ch->player_specials->abilities[i] = st->abilities[i];

  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;

  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;
  ch->points.armor = 100;
  ch->points.hitroll = 0;
  ch->points.damroll = 0;
  ch->points.spellpower = 0;

  if (ch->player.name == NULL)
    CREATE(ch->player.name, char, strlen(st->name) + 1);
  strcpy(ch->player.name, st->name);
  strcpy(ch->player.passwd, st->pwd);

  /* Add all spell effects */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }

  for (i = 0; i < MAX_ASSIGNED; i++) {
    ch->player_specials->assigned[i] = NOBODY;
  }

  /*
   * If you're not poisioned and you've been away for more than an hour of
   * real time, we'll set your HMV back to full
   */

  if (!IS_AFFECTED(ch, AFF_POISON) &&
      (((long) (time(0) - st->last_logon)) >= SECS_PER_REAL_HOUR)) {
    GET_HIT(ch)  = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_QI(ch)   = GET_MAX_QI(ch);
    GET_VIM(ch)  = GET_MAX_VIM(ch);
    GET_ARIA(ch) = GET_MAX_ARIA(ch);
  }
}				/* store_to_char */




/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data * ch, struct char_file_u * st)
{
  int i;
  struct affected_type *af;
  struct obj_data *char_eq[NUM_WEARS];

  /* Unaffect everything a character can be affected by */

  for (i = 0; i < NUM_WEARS; i++) {
    if (GET_EQ(ch, i)) {
      char_eq[i] = unequip_char(ch, i);
#ifndef NO_EXTRANEOUS_TRIGGERS
      remove_otrigger(char_eq[i], ch);
#endif
    } else
      char_eq[i] = NULL;
  }

  for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      af = af->next;
    } else {
      st->affected[i].type = 0;	/* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }


  /*
   * remove the affections so that the raw values are stored; otherwise the
   * effects are doubled when the char logs back in.
   */

  while (ch->affected)
    affect_remove(ch, ch->affected);

  if ((i >= MAX_AFFECT) && af && af->next)
    log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");

  ch->aff_stats = ch->real_stats;

  st->birth = ch->player.time.birth;
  st->played = ch->player.time.played;
  st->played += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight = GET_WEIGHT(ch);
  st->height = GET_HEIGHT(ch);
  st->sex = GET_SEX(ch);
  st->class = GET_CLASS(ch);
  st->race = GET_RACE(ch);
  st->level = GET_LEVEL(ch);
  st->stats = ch->real_stats;
  st->points = ch->points;
  st->char_specials_saved = ch->char_specials.saved;
  st->player_specials_saved = ch->player_specials->saved;
  for (i=0; i < MAX_ABILITIES; i++)
    st->abilities[i] = ch->player_specials->abilities[i];

  st->points.armor = 100;
  st->points.hitroll = 0;
  st->points.damroll = 0;
  
  st->current_quest = ch->current_quest;
  
/*
  for (i=0; i < MAX_COMPLETED_QUESTS;i++)
    st->completed_quests[i] = -1;
  for (i=0; i <= ch->num_completed_quests; i++)
    st->completed_quests[i] = ch->completed_quests[i];
*/
    

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';

  strcpy(st->name, GET_NAME(ch));
  strcpy(st->pwd, GET_PASSWD(ch));

  /* add spell and eq affections back in now */
  for (i = 0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }

  for (i = 0; i < NUM_WEARS; i++) {
    if (char_eq[i]) {
#ifndef NO_EXTRANEOUS_TRIGGERS
      if (wear_otrigger(char_eq[i], ch, i))
#endif
        equip_char(ch, char_eq[i], i);
#ifndef NO_EXTRANEOUS_TRIGGERS
      else
        obj_to_char(char_eq[i], ch);
#endif
    }
  }
}				/* Char to store */



void save_etext(struct char_data * ch)
{
/* this will be really cool soon */

}


/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		       (++top_of_p_table + 1)))) {
    perror("SYSERR: create entry");
    exit(1);
  }
  CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);

  return (top_of_p_table);
}



/************************************************************************
*  funcs of a (more or less) general utility nature			*
************************************************************************/


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
  char buf[MAX_STRING_LENGTH], tmp[512], *rslt;
  register char *point;
  int done = 0, length = 0, templength = 0;

  *buf = '\0';

  do {
    if (!fgets(tmp, 1024, fl)) {
      fprintf(stderr, "SYSERR: fread_string: format error at or near %s\n",
	      error);
      exit(1);
    }
    /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
    if ((point = strchr(tmp, '~')) != NULL) {
      *point = '\0';
      done = 1;
    } else {
      point = tmp + strlen(tmp) - 1;
      *(point++) = '\r';
      *(point++) = '\n';
      *point = '\0';
    }

    templength = strlen(tmp);

    if (length + templength >= MAX_STRING_LENGTH) {
      log("SYSERR: fread_string: string too large (db.c)");
      log(error);
      exit(1);
    } else {
      strcat(buf + length, tmp);
      length += templength;
    }
  } while (!done);

  /* allocate space for the new string and copy it */
  if (strlen(buf) > 0) {
    CREATE(rslt, char, length + 1);
    strcpy(rslt, buf);
  } else
    rslt = NULL;

  return rslt;
}


/* release memory allocated for a char struct */
void free_char(struct char_data * ch)
{
  int i;
  struct alias *a;
  void free_alias(struct alias * a);

  if (ch->player_specials != NULL && ch->player_specials != &dummy_mob) {
    while ((a = GET_ALIASES(ch)) != NULL) {
      GET_ALIASES(ch) = (GET_ALIASES(ch))->next;
      free_alias(a);
    }
    if (ch->player_specials->poofin)
      free(ch->player_specials->poofin);
    if (ch->player_specials->poofout)
      free(ch->player_specials->poofout);
    free(ch->player_specials);
    if (IS_NPC(ch))
      log("SYSERR: Mob had player_specials allocated!");
  }
  if (!IS_NPC(ch) || (IS_NPC(ch) && GET_MOB_RNUM(ch) == -1)) {
    /* if this is a player, or a non-prototyped non-player, free all */
    if (GET_NAME(ch))
      free(GET_NAME(ch));
    if (ch->player.title)
      free(ch->player.title);
    if (ch->player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description)
      free(ch->player.description);
    if (ch->completed_quests)
      free(ch->completed_quests);   
  } else if ((i = GET_MOB_RNUM(ch)) > -1) {
    /* otherwise, free strings only if the string is not pointing at proto */
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
      free(ch->player.name);
    if (ch->player.title && ch->player.title != mob_proto[i].player.title)
      free(ch->player.title);
    if (ch->player.short_descr && ch->player.short_descr != mob_proto[i].player.short_descr)
      free(ch->player.short_descr);
    if (ch->player.long_descr && ch->player.long_descr != mob_proto[i].player.long_descr)
      free(ch->player.long_descr);
    if (ch->player.description && ch->player.description != mob_proto[i].player.description)
      free(ch->player.description);
  }
  while (ch->affected)
    affect_remove(ch, ch->affected);

  free(ch);
}




/* release memory allocated for an obj struct */
void free_obj(struct obj_data * obj)
{
  int nr;
  struct extra_descr_data *this, *next_one;

  if ((nr = GET_OBJ_RNUM(obj)) == -1) {
    if (obj->name)
      free(obj->name);
    if (obj->description)
      free(obj->description);
    if (obj->short_description)
      free(obj->short_description);
    if (obj->action_description)
      free(obj->action_description);
    if (obj->ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  } else {
    if (obj->name && obj->name != obj_proto[nr].name)
      free(obj->name);
    if (obj->description && obj->description != obj_proto[nr].description)
      free(obj->description);
    if (obj->short_description && obj->short_description != obj_proto[nr].short_description)
      free(obj->short_description);
    if (obj->action_description && obj->action_description != obj_proto[nr].action_description)
      free(obj->action_description);
    if (obj->ex_description && obj->ex_description != obj_proto[nr].ex_description)
      for (this = obj->ex_description; this; this = next_one) {
	next_one = this->next;
	if (this->keyword)
	  free(this->keyword);
	if (this->description)
	  free(this->description);
	free(this);
      }
  }

  free(obj);
}



/* read contets of a text file, alloc space, point buf to it */
int file_to_string_alloc(char *name, char **buf)
{
  char temp[MAX_STRING_LENGTH];

  if (*buf)
    free(*buf);

  if (file_to_string(name, temp) < 0) {
    *buf = "";
    return -1;
  } else {
    *buf = str_dup(temp);
    return 0;
  }
}


/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[READ_SIZE+3];

  *buf = '\0';

  if (!(fl = fopen(name, "r"))) {
    sprintf(tmp, "SYSERR: reading %s", name);
    perror(tmp);
    return (-1);
  }
  do {
    fgets(tmp, READ_SIZE, fl);
    tmp[strlen(tmp) - 1] = '\0'; /* take off the trailing \n */
    strcat(tmp, "\r\n");

    if (!feof(fl)) {
      if (strlen(buf) + strlen(tmp) + 1 > MAX_STRING_LENGTH) {
        sprintf(buf, "SYSERR: %s: string too big (%d max)", name,
		MAX_STRING_LENGTH);
	log(buf);
	*buf = '\0';
	return -1;
      }
      strcat(buf, tmp);
    }
  } while (!feof(fl));

  fclose(fl);

  return (0);
}



/* clear some of the the working variables of a char */
void reset_char(struct char_data * ch)
{
  int i;
  
  if(PRF_FLAGGED(ch, PRF_MULTIPLAY))
    TOGGLE_BIT_AR(PRF_FLAGS(ch), PRF_MULTIPLAY);

  for (i = 0; i < NUM_WEARS; i++)
    GET_EQ(ch, i) = NULL;

  ch->followers = NULL;
  ch->master = NULL;
  ch->in_room = NOWHERE;
  ch->carrying = NULL;
  ch->next = NULL;
  ch->next_fighting = NULL;
  ch->next_in_room = NULL;
  FIGHTING(ch) = NULL;
  ch->char_specials.position = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;
  ch->char_specials.carry_weight = 0;
  ch->char_specials.carry_items = 0;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;

  GET_LAST_TELL(ch) = NOBODY;
}



/* clear ALL the working variables of a char; do NOT free any space alloc'ed */
void clear_char(struct char_data * ch)
{
  memset((char *) ch, 0, sizeof(struct char_data));

  ch->in_room = NOWHERE;
  GET_PFILEPOS(ch) = -1;
  GET_WAS_IN(ch) = NOWHERE;
  GET_POS(ch) = POS_STANDING;
  ch->mob_specials.default_pos = POS_STANDING;

  GET_AC(ch) = 100;		/* Basic Armor */
  if (ch->points.max_mana < 100)
    ch->points.max_mana = 100;
}


void clear_object(struct obj_data * obj)
{
  memset((char *) obj, 0, sizeof(struct obj_data));

  obj->item_number = NOTHING;
  obj->in_room = NOWHERE;
}

/* initialize a new character only if class is set */
void init_char(struct char_data * ch)
{
  int i;

  /* create a player_special structure */
  if (ch->player_specials == NULL)
    CREATE(ch->player_specials, struct player_special_data, 1);

  /* *** if this is our first player --- he be God *** */
  if (top_of_p_table == 0) {
    GET_EXP(ch) = 2000000000;
    GET_LEVEL(ch) = LVL_IMPL;

    ch->points.max_hit  = 2000;
    ch->points.max_mana = 2000;
    ch->points.max_move = 2000;
    ch->points.max_qi   = 2000;
    ch->points.max_vim  = 2000;
    ch->points.max_aria = 2000;
  } else {
    ch->points.max_hit  = 22;
    ch->player_specials->saved.wimp_level = 11;
    ch->points.max_mana = 100;
    ch->points.max_move = 82;
    ch->points.max_qi   = 80;
    ch->points.max_vim  = 80;
    ch->points.max_aria = 80;
  }
  ch->points.armor = 100;
  set_title(ch, NULL);

  GET_PRACTICES(ch) = 3;

  ch->points.hit  = GET_MAX_HIT(ch);
  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.move = GET_MAX_MOVE(ch);
  ch->points.qi   = GET_MAX_QI(ch);
  ch->points.vim  = GET_MAX_VIM(ch);
  ch->points.aria = GET_MAX_ARIA(ch);

  ch->player.short_descr = NULL;
  ch->player.long_descr = NULL;
  ch->player.description = NULL;
  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

 
  ch->num_completed_quests = -1;
  ch->completed_quests = NULL;
  ch->current_quest = -1;

  for (i = 0; i < MAX_TONGUE; i++)
    GET_TALK(ch, i) = 0;

  /* Adjust height and weight for race and sex */
  GET_HEIGHT(ch) = (int)(number(66, 74) * race_height_idx[(int)GET_RACE(ch)]);
  GET_WEIGHT(ch) = (int)(GET_HEIGHT(ch) * race_weight_idx[(int)GET_RACE(ch)]);
  if (ch->player.sex == SEX_FEMALE) {
    ch->player.height *= .85;
    ch->player.weight *= .85;
  }


  player_table[top_of_p_table].id = GET_IDNUM(ch) = ++top_idnum;

  for (i = 0; i < MAX_ABILITIES; i++) {
      ch->player_specials->abilities[i].type = -1;
      ch->player_specials->abilities[i].num = 0;
      ch->player_specials->abilities[i].pct = 0;
  }

 for(i=0; i < AF_ARRAY_MAX; i++)
   ch->char_specials.saved.affected_by[i] = 0;

  for (i = 0; i < 5; i++)
    GET_SAVE(ch, i) = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GET_LEVEL(ch) >= LVL_IMPL ? -1 : 24);

  GET_LOADROOM(ch) = NOWHERE;

}



/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_world;

  /* perform binary search on world-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((world + mid)->number == virtual)
      return mid;
    if (bot >= top)
      return NOWHERE;
    if ((world + mid)->number > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

void save_char_file_u(struct char_file_u st)
{
int player_i;
int find_name(char *name);

if((player_i = find_name(st.name)) >=0 )
  {
  fseek(player_fl, player_i * sizeof(struct char_file_u), SEEK_SET);
  fwrite(&st, sizeof(struct char_file_u), 1, player_fl);
  }
}


/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_mobt;

  /* perform binary search on mob-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((mob_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}



/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;) {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->virtual == virtual)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->virtual > virtual)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}


void load_tips() {

  FILE *fp;
  char tip_line[READ_SIZE];
  int i=0;

  if (!(fp=fopen(TIP_FILE, "r"))){
    printf("Can't open tip file '%s' for reading.\n", TIP_FILE);
    exit(1);
  }
 
  /* Seed the rand.  This may be done elsewhere but it won't hurt... */
  srand( (unsigned)time( NULL ) );

  /* The first line will have the number of lines in the tip file. */ 
  fgets(tip_line, READ_SIZE, fp);
  tip_count = atoi(tip_line);
 
  tip_list = (long *)malloc(sizeof(long)*(tip_count+1));
 
  if (!tip_list) {
    printf("Failed to allocate memory for tip list!\n");
    exit(1);
  }

  tip_list[i++] = ftell(fp);
 
  while(fgets(tip_line, READ_SIZE, fp))
    tip_list[i++] = ftell(fp);
 
  fclose(fp);
/*
  for (i=0; i<tip_count; i++)
    printf(" Tip %d file offset: %d\n", i, tip_list[i]);
*/

}

char *get_random_tip() {

  FILE *fp;
  char tip_line[1024];
  char *tip_ptr;
  int tip_no;
 
  tip_no = 1 + (int)((float)(tip_count)*rand()/(RAND_MAX+1.0));

  /* Open the tip file */
  if (!(fp=fopen(TIP_FILE, "r"))){
    printf("Can't open tip file '%s' for reading.\n", TIP_FILE);
    exit(1);
  }
 
  fseek(fp, tip_list[tip_no-1], SEEK_SET);
  fgets(tip_line, READ_SIZE, fp);
  tip_ptr=tip_line;
  fclose(fp);
  return tip_ptr;

}

int vnum_weapon(int attacktype, struct char_data *ch)
{
  int nr, found = 0;
  char affs[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char output[LARGE_BUFSIZE][MAX_NR_ITEMS_LIST];
  int i = 0, bufs_used = 1, objs_in_buf = 0;
  bool output_written = FALSE;
  bool found_objs = FALSE;
  *affs = '\0';

  for(i = 0; i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';

  for (nr = 0; nr <= top_of_objt; nr++)
  {
	  if (obj_proto[nr].obj_flags.type_flag == ITEM_WEAPON && obj_proto[nr].obj_flags.value[3] == attacktype)
	  {
		  found_objs = TRUE;
		  output_written = FALSE;
		  for(i = 0;i < bufs_used;i++)
		  {
			  if (output_written == TRUE)
				  continue;
			  else if (objs_in_buf >= 100)
			  {
				  objs_in_buf = 0;
				  bufs_used++;
				  continue;
			  }
			  else {
				  objs_in_buf++;
				  output_written = TRUE;
				  sprintf(name, "%s", obj_proto[nr].short_description);
				  colorless_str(name);
				  sprintf(output[i], "%s%3d. [%5d] (%3d) %-40.40s", output[i], ++found,
					  obj_index[nr].virtual,
					  obj_proto[nr].obj_flags.minlevel, name);
				  sprintbitarray(obj_proto[nr].obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, affs);
				  sprintf(output[i], "%s     /c0%s\r\n", output[i], affs);
			  }
		  }
	  }
  }
  if (found_objs == FALSE)
  {
	  send_to_char("No weapons with that weapontype.\r\n", ch);
	  return (TRUE);
  }
  page_string(ch->desc, output[0], 0);
  for(i = 0;i < MAX_NR_ITEMS_LIST;i++)
	  *output[i] = '\0';
  return (TRUE);
}
