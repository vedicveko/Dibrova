#define MAX_CLANS        20
#define LVL_CLAN_GOD     LVL_ADMIN
#define DEFAULT_APP_LVL  8
#define CLAN_PLAN_LENGTH 800

#define GET_CLAN(ch)            ((ch)->player_specials->saved.clan)
#define GET_CLAN_RANK(ch)       ((ch)->player_specials->saved.clan_rank)

#define CP_SET_PLAN   0
#define CP_ENROLL     1
#define CP_EXPEL      2
#define CP_PROMOTE    3
#define CP_DEMOTE     4
#define CP_SET_FEES   5
#define CP_WITHDRAW   6
#define CP_SET_APPLEV 7
#define CP_WAR        8
#define CP_SET_TAX    9
#define NUM_CP        10        /* Number of clan privileges */

#define CM_DUES   1
#define CM_APPFEE 2
#define CM_WAR    3
#define CM_TAX    4

#define CB_DEPOSIT  1
#define CB_WITHDRAW 2

void save_clans(void);
void init_clans(void);
sh_int find_clan_by_id(int clan_id);
sh_int find_clan(char *name);

extern struct clan_rec clan[MAX_CLANS];
extern int num_of_clans;

struct clan_rec {
  int   id;
  char  name[32];
  ubyte ranks;
  char  rank_name[20][20];
  long  treasure;
  int   members;
  int   power;
  int   app_fee;
  int   dues;
  int   spells[5];
  int   app_level;
  ubyte privilege[20];
  int   at_war;
  char  description[CLAN_PLAN_LENGTH];
  int   alliance;
  int   pkwin;
  int   pklose;
  int   hall;
  int   war_timer;
  ubyte raided;
  ubyte seen;
  int   tax;
  int   spare2;
  int   spare3;
  ubyte spare4;
  ubyte spare5;
  char spare6[32];
};

