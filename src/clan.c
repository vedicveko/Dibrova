/** ************************************************************************
 * File: clan.c                       Intended to be used with CircleMUD  *
 * Usage: This is the code for clans                                      *
 * By Mehdi Keddache (Heritsun on Eclipse of Fate eclipse.argy.com 7777)  *
 * CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 * CircleMUD (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 **************************************************************************/

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "clan.h"

int num_of_clans;
struct clan_rec clan[MAX_CLANS];

struct obj_data *Obj_from_store(struct obj_file_elem object);
int Obj_to_store(struct obj_data * obj, FILE * fl);
extern int save_char_file_u(struct char_file_u st);
extern struct descriptor_data *descriptor_list;
extern struct char_data *is_playing(char *vict_name);
extern void proc_color(char*, int);
extern char *rank[][3];
extern char *class_abbrevs[];
extern char *race_abbrevs[];
extern struct room_data *world;
void do_clan_list(struct char_data *ch);
void do_clan_where(struct char_data *ch);
void do_clan_war(struct char_data *ch, char *arg);
void do_clan_alliance(struct char_data *ch, char *arg);
extern struct index_data *mob_index;
extern struct zone_data *zone_table;
extern int top_of_zone_table;
void mobsay(struct char_data *ch, const char *msg);
ACMD(do_move);
extern int top_of_world;
// void write_event(char *event);
bool in_allied_lands(struct char_data *ch);
bool in_war_zone(struct char_data *ch);
bool in_clan_lands(struct char_data *ch);
void do_generic_knock_down(struct char_data *ch, struct char_data *vict, int type);
void colorless_str(char *str);
void zedit_save_to_disk(int zone_num);

char clan_privileges[NUM_CP+1][20] ={
"setplan","enroll","expel","promote","demote","setfees","withdraw","setapplev"};

void send_clan_format(struct char_data *ch)
{
int c,r;

send_to_char("Clan commands available to you:\n\r"
             "   clan who\r\n"
             "   clan status\r\n"
             "   clan info <clan>\r\n",ch);
if (GET_LEVEL(ch) <=LVL_CLAN_GOD)
send_to_char("   clan expel (self only)\r\n", ch);
if(GET_LEVEL(ch)>=LVL_CLAN_GOD)
  send_to_char("   clan create     <leader> <clan name>\r\n"
               "   clan destroy    <clan>\r\n"
               "   clan enroll     <player> <clan>\r\n"
               "   clan expel      <player> <clan>\r\n"
               "   clan promote    <player> <clan>\r\n"
               "   clan demote     <player> <clan>\r\n"
               "   clan withdraw   <amount> <clan>\r\n"
               "   clan deposit    <amount> <clan>\r\n"
               "   clan set ranks  <rank>   <clan>\r\n"
               "   clan set appfee <amount> <clan>\r\n"
               "   clan set dues   <amount> <clan>\r\n"
               "   clan set applev <level>  <clan>\r\n"
               "   clan set tax    <percentage> <clan>\r\n"
               "   clan set plan   <clan>\r\n"
               "   clan privilege  <privilege>   <rank> <clan>\r\n"
               "   clan set title  <clan number> <rank> <title>\r\n"
               "   clan war        <clan ID>\r\n"
               "   clan alliance   <clan ID>\r\n"
               ,ch);

else {
  c=find_clan_by_id(GET_CLAN(ch));
  r=GET_CLAN_RANK(ch);
  if(r<1)
    send_to_char("   clan apply      <clan>\r\n",ch);
  if(c>=0) {
    send_to_char("   clan deposit    <amount>\r\n",ch);
    send_to_char("   clan list\r\n", ch);
    send_to_char("   clan where\r\n", ch);
    if(r>=clan[c].privilege[CP_WITHDRAW])
      send_to_char("   clan withdraw   <amount>\r\n" ,ch);
    if(r>=clan[c].privilege[CP_ENROLL] -1)
      send_to_char("   clan enroll     <player>\r\n" ,ch);
    if(r>=clan[c].privilege[CP_EXPEL]) {
      send_to_char("   clan expel      <player>\r\n" ,ch);
      send_to_char("   clan war        <clan ID>\r\n", ch);
      send_to_char("   clan alliance   <clan ID>\r\n", ch);
    }
    if(r>=clan[c].privilege[CP_PROMOTE])
      send_to_char("   clan promote    <player>\r\n",ch);
    if(r>=clan[c].privilege[CP_DEMOTE])
      send_to_char("   clan demote     <player>\r\n",ch);
    if(r>=clan[c].privilege[CP_SET_APPLEV])
      send_to_char("   clan set applev <level>\r\n",ch);
    if(r>=clan[c].privilege[CP_SET_TAX])
      send_to_char("   clan set tax    <percentage>\r\n", ch);
    if(r>=clan[c].privilege[CP_SET_FEES])
      send_to_char("   clan set appfee <amount>\r\n"
                   "   clan set dues   <amount>\r\n",ch);
    if(r>=clan[c].privilege[CP_SET_PLAN])
      send_to_char("   clan set plan\r\n",ch);
    if(r==clan[c].ranks)
      send_to_char("   clan set ranks  <rank>\r\n"
                   "   clan set title  <rank> <title>\r\n"
                   "   clan privilege  <privilege> <rank>\r\n",ch);
    }
  }
}

void do_clan_create (struct char_data *ch, char *arg)
{
struct char_data *leader = NULL;
char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
int i;

if (!*arg) {
  send_clan_format(ch);
  return; }

if (GET_LEVEL(ch) < LVL_CLAN_GOD) {
  send_to_char("You are not mighty enough to create new clans!\r\n", ch);
  return; }

if(num_of_clans == MAX_CLANS) {
  send_to_char("Max clans reached. WOW!\r\n",ch);
  return; }

half_chop(arg, arg1, arg2);

if(!(leader=get_char_vis(ch,arg1))) {
  send_to_char("The leader of the new clan must be present.\r\n",ch);
  return; }

if(strlen(arg2)>=32) {
  send_to_char("Clan name too long! (32 characters max)\r\n",ch);
  return; }

if(GET_LEVEL(leader)>=LVL_IMMORT) {
  send_to_char("You cannot set an immortal as the leader of a clan.\r\n",ch);
  return; }

if(GET_CLAN(leader)!=0 && GET_CLAN_RANK(leader)!=0) {
  send_to_char("The leader already belongs to a clan!\r\n",ch);
  return; }

if(find_clan(arg2)!=-1) {
  send_to_char("That clan name already exists!\r\n",ch);
  return; }

strncpy(clan[num_of_clans].name, CAP((char *)arg2), 32);
//for(i=0;i<num_of_clans;i++)
//  if(new_id<clan[i].id)
//    new_id=clan[i].id;
clan[num_of_clans].id=num_of_clans;
clan[num_of_clans].ranks =  6;
strcpy(clan[num_of_clans].rank_name[0],"/cwMember");
strcpy(clan[num_of_clans].rank_name[1],"/cbDeveloper/cw");
strcpy(clan[num_of_clans].rank_name[2],"/cRDiplomat/cw");
strcpy(clan[num_of_clans].rank_name[3],"/cYTreasurer/cw");
strcpy(clan[num_of_clans].rank_name[4],"/cGRecruiter/cw");
strcpy(clan[num_of_clans].rank_name[5],"/cCLeader/cw");
clan[num_of_clans].treasure = 0 ;
clan[num_of_clans].members = 1 ;
clan[num_of_clans].power = GET_LEVEL(leader) ;
clan[num_of_clans].app_fee = 0 ;
clan[num_of_clans].tax = 0 ;
clan[num_of_clans].dues = 0 ;
clan[num_of_clans].app_level = DEFAULT_APP_LVL ;
for(i=0;i<5;i++)
  clan[num_of_clans].spells[i]=0;
for(i=0;i<20;i++)
  clan[num_of_clans].privilege[i]=clan[num_of_clans].ranks;

  clan[num_of_clans].at_war=0;
  clan[num_of_clans].alliance=0;
  clan[num_of_clans].hall = 300 + clan[num_of_clans].id;
num_of_clans++;
save_clans();

sprintf(buf, "New ID is %d, Clan Hall zone num is %d, have a nice day.\r\n", clan[num_of_clans-1].id, clan[num_of_clans-1].hall); 

send_to_char(buf, ch);

send_to_char("Clan created\r\n", ch);

GET_CLAN(leader)=clan[num_of_clans-1].id;
GET_CLAN_RANK(leader)=clan[num_of_clans-1].ranks;
save_char(leader, leader->in_room);

return;
}


void do_clan_destroy (struct char_data *ch, char *arg)
{

int i,j;
extern int top_of_p_table;
extern struct player_index_element *player_table;
struct char_file_u chdata;
struct char_data *victim=NULL;

if (!*arg) {
  send_clan_format(ch);
  return; }

if(isdigit(*arg)){
  i = atoi(arg);
  if(i < 0 || i >=num_of_clans){
    send_to_char("No clan by that number.\r\n", ch);
    return;
  }
}
else
  if ((i = find_clan(arg)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; }

if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
  send_to_char("Your not mighty enough to destroy clans!\r\n", ch);
  return; }

for (j = 0; j <= top_of_p_table; j++){
  if((victim=is_playing((player_table +j)->name))) {
    if(GET_CLAN(victim)==clan[i].id) {
      GET_CLAN(victim)=0;
      GET_CLAN_RANK(victim)=0;
      save_char(victim, victim->in_room); } }
  else {
    load_char((player_table + j)->name, &chdata);
    if(chdata.player_specials_saved.clan==clan[i].id) {
      chdata.player_specials_saved.clan=0;
      chdata.player_specials_saved.clan_rank=0;
      save_char_file_u(chdata); } } }

memset(&clan[i], sizeof(struct clan_rec), 0);

for (j = i; j < num_of_clans - 1; j++)
  clan[j] = clan[j + 1];

num_of_clans--;

send_to_char("Clan deleted.\r\n", ch);
save_clans();
return;
}

void do_clan_enroll (struct char_data *ch, char *arg)
{
struct char_data *vict=NULL;
int clan_num,immcom=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if((GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_ENROLL] -1) && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(vict=get_char_vis(ch,arg))) {
  send_to_char("Er, Who ??\r\n",ch);
  return;
}
else {
  if(GET_CLAN(vict)!=clan[clan_num].id) {
    if(GET_CLAN_RANK(vict)>0) {
      send_to_char("They're already in a clan.\r\n",ch);
      return;
    }
    else {
      send_to_char("They didn't request to join your clan.\r\n",ch);
      return;
    }
  }
  else
    if(GET_CLAN_RANK(vict)>0) {
      send_to_char("They're already in your clan.\r\n",ch);
      return;
    }
  if(GET_LEVEL(vict)>=LVL_IMMORT) {
    send_to_char("You cannot enroll immortals in clans.\r\n",ch);
    return; }
}

if (PLR_FLAGGED(vict, PLR_EXILE)) {
  REMOVE_BIT_AR(PLR_FLAGS(vict), PLR_EXILE);
}
GET_CLAN_RANK(vict)++;
save_char(vict, vict->in_room);
clan[clan_num].power += GET_LEVEL(vict);
clan[clan_num].members++;
send_to_char("You've been enrolled in the clan you chose!\r\n",vict);
send_to_char("Done.\r\n",ch);

return;
}

void do_clan_expel (struct char_data *ch, char *arg)
{
struct char_data *vict=NULL;
int clan_num,immcom=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return; } }
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; } }

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_EXPEL] && !immcom) {

  if(!(vict=get_char_vis(ch,arg))) {
    send_to_char("Er, Who ??\r\n",ch);
    return;
  } if (vict!=ch){
    send_to_char("You can only expel yourself!\r\n", ch);
    return;
  }

    GET_CLAN(ch)=0;
    GET_CLAN_RANK(ch)=0;
    save_char(ch, ch->in_room);
    clan[clan_num].members--;
    clan[clan_num].power-=GET_LEVEL(ch);
    SET_BIT_AR(PLR_FLAGS(ch), PLR_EXILE);
    GET_EXP(ch) = MAX(1, (GET_EXP(ch) - 50000000));
    sprintf(buf, "CLAN: %s has exiled (him/her)self from %s/c0",
            GET_NAME(ch), clan[clan_num].name);
    log(buf);
    send_to_char("You've removed yourself from your clan!\r\n",vict); 


//  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return; }

if(!(vict=get_char_vis(ch,arg))) {
  send_to_char("Er, Who ??\r\n",ch);
  return; }
else {
  if(GET_CLAN(vict)!=clan[clan_num].id) {
    send_to_char("They're not in your clan.\r\n",ch);
    return; }
  else {
    if(GET_CLAN_RANK(vict)>=GET_CLAN_RANK(ch) && !immcom) {
      send_to_char("You cannot kick out that person.\r\n",ch);
      return; } } }

GET_CLAN(vict)=0;
GET_CLAN_RANK(vict)=0;
save_char(vict, vict->in_room);
clan[clan_num].members--;
clan[clan_num].power-=GET_LEVEL(vict);
SET_BIT_AR(PLR_FLAGS(vict), PLR_EXILE);

if(GET_EXP(vict) > 50000000) {
  GET_EXP(vict) -= 50000000;
}
else {
  GET_EXP(vict) = 100;
}

sprintf(buf, "CLAN: %s has been exiled from %s/c0", GET_NAME(ch), clan[clan_num].name);
log(buf);
send_to_char("You've been kicked out of your clan!\r\n",vict);
send_to_char("Done.\r\n",ch);

return;
}

void do_clan_demote (struct char_data *ch, char *arg)
{
struct char_data *vict=NULL;
int clan_num,immcom=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return; } }
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; } }

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_DEMOTE] && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return; }

if(!(vict=get_char_vis(ch,arg))) {
  send_to_char("Er, Who ??\r\n",ch);
  return; }
else {
  if(GET_CLAN(vict)!=clan[clan_num].id) {
    send_to_char("They're not in your clan.\r\n",ch);
    return; }
  else {
    if(GET_CLAN_RANK(vict)==1) {
      send_to_char("They can't be demoted any further, use expel now.\r\n",ch);
      return; }
    if(GET_CLAN_RANK(vict)>=GET_CLAN_RANK(ch) && !immcom) {
      send_to_char("You cannot demote a person of this rank!\r\n",ch);
      return; } } }

GET_CLAN_RANK(vict)--;
save_char(vict, vict->in_room);
send_to_char("You've demoted within your clan!\r\n",vict);
send_to_char("Done.\r\n",ch);
return;
}

void do_clan_promote (struct char_data *ch, char *arg)
{
struct char_data *vict=NULL;
int clan_num,immcom=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return; } }
else {
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; } }

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_PROMOTE] && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return; }

if(!(vict=get_char_vis(ch,arg))) {
  send_to_char("Er, Who ??\r\n",ch);
  return; }
else {
  if(GET_CLAN(vict)!=clan[clan_num].id) {
    send_to_char("They're not in your clan.\r\n",ch);
    return; }
  else {
    if(GET_CLAN_RANK(vict)==0) {
      send_to_char("They're not enrolled yet.\r\n",ch);
      return; }
    if((GET_CLAN_RANK(vict)+1)>GET_CLAN_RANK(ch) && !immcom) {
      send_to_char("You cannot promote that person over your rank!\r\n",ch);
      return; }
    if(GET_CLAN_RANK(vict)==clan[clan_num].ranks) {
      send_to_char("You cannot promote someone over the top rank!\r\n",ch);
      return; } } }

GET_CLAN_RANK(vict)++;
save_char(vict, vict->in_room);
send_to_char("You've been promoted within your clan!\r\n",vict);
send_to_char("Done.\r\n",ch);
return;
}

void do_clan_who (struct char_data *ch)
{
struct descriptor_data *d;
struct char_data *tch;
char line_disp[90];

if(GET_CLAN_RANK(ch)==0) {
  send_to_char("You do not belong to a clan!\r\n",ch);
  return; }

send_to_char("\r\nList of your clan members\r\n",ch);
send_to_char("-------------------------\r\n",ch);
  for (d=descriptor_list; d; d=d->next){
    if((tch=d->character))
      if(GET_CLAN(tch)==GET_CLAN(ch) && GET_CLAN_RANK(tch)>0) {
        sprintf(line_disp,"%s\r\n",GET_NAME(tch));
        send_to_char(line_disp,ch); }
  }
return;
}

void do_clan_status (struct char_data *ch)
{
char line_disp[90];
int clan_num;

if(GET_LEVEL(ch)>=LVL_IMMORT) {
  send_to_char("You are immortal and cannot join any clan!\r\n",ch);
  return; }

clan_num=find_clan_by_id(GET_CLAN(ch));

if(GET_CLAN_RANK(ch)==0) {
  if(clan_num>=0) {
    sprintf(line_disp,"You applied to %s\r\n",clan[clan_num].name);
    send_to_char(line_disp,ch);
    return; }
  else {
    send_to_char("You do not belong to a clan!\r\n",ch);
    return; }
}

 sprintf(line_disp,"You are %s (Rank %d) of %s (ID %d)\r\n",
    clan[clan_num].rank_name[GET_CLAN_RANK(ch)-1],GET_CLAN_RANK(ch),
    clan[clan_num].name,clan[clan_num].id);
 send_to_char(line_disp,ch);

return;
}

void do_clan_apply (struct char_data *ch, char *arg)
{
int clan_num;

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)>=LVL_IMMORT) {
  send_to_char("Gods cannot apply for any clan.\r\n",ch);
  return; }

if(GET_CLAN_RANK(ch)>0) {
  send_to_char("You already belong to a clan!\r\n",ch);
  return; }
else {
  if ((clan_num = find_clan(arg)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; } }

if(GET_LEVEL(ch) < clan[clan_num].app_level) {
  send_to_char("You are not mighty enough to apply to this clan.\r\n",ch);
  return; }

if(GET_GOLD(ch) < clan[clan_num].app_fee) {
  send_to_char("You cannot afford the application fee!\r\n", ch);
  return; }

GET_GOLD(ch) -= clan[clan_num].app_fee;
clan[clan_num].treasure += clan[clan_num].app_fee;
save_clans();
GET_CLAN(ch)=clan[clan_num].id;
save_char(ch, ch->in_room);
send_to_char("You've applied to the clan!\r\n",ch);

return;
}

void do_clan_info (struct char_data *ch, char *arg)
{
int i=0,j,d;
char cname[100];

if(num_of_clans <= 1) {
  send_to_char("No clans have formed yet.\r\n",ch);
  return; }

if(!(arg)) {
  sprintf(buf, "\r");
  for(i=1; i < num_of_clans; i++) {
	  sprintf(cname, "%-32s",clan[i].name); 
	  colorless_str(cname);
      sprintf(buf, "%s(%-2d) %-32s Members: %-3d  Power: %-6d\r\n",buf, i, cname,
      clan[i].members,clan[i].power);
      colorless_str(buf);
      page_string(ch->desc,buf, 1);
  }
  return;
}
else{
  if((isdigit(*arg)) && (arg != NULL || !arg)){
    i = atoi(arg);
    if(!i || i >=num_of_clans){
  sprintf(buf, "Unknown Clan Number.\r\n\r");
  for(i=1; i < num_of_clans; i++) {
	  sprintf(cname, "%-32s",clan[i].name); 
	  colorless_str(cname);
      sprintf(buf, "%s(%-2d) %-32s Members: %-3d  Power: %-6d\r\n",buf, i, cname,
      clan[i].members,clan[i].power);
      colorless_str(buf);
      page_string(ch->desc,buf, 1);
  }
  return;

     }
  }
  else   
    if ((i = find_clan(arg)) < 0) {
  sprintf(buf, "\r");
  for(i=1; i < num_of_clans; i++) {
	  sprintf(cname, "%-32s",clan[i].name); 
	  colorless_str(cname);
      sprintf(buf, "%s(%-2d) %-32s Members: %-3d  Power: %-6d\r\n",buf, i, cname,
      clan[i].members,clan[i].power);
      colorless_str(buf);
      page_string(ch->desc,buf, 1);
  }
  return;

    }
}

sprintf(buf, "Info for clan <<%s>>:\r\n",clan[i].name);
send_to_char(buf, ch);
sprintf(buf, "Ranks      : %d\r\nTitles     : ",clan[i].ranks);
for(j=0;j<clan[i].ranks;j++)
  sprintf(buf, "%s%s /c0",buf,clan[i].rank_name[j]);
sprintf(buf, "%s\r\nMembers: [%-2d] Power: [%-5d] Treasure: [%ld]\r\n",buf, clan[i].members, clan[i].power, clan[i].treasure);

/*for(j=0; j<5;j++)
  if(clan[i].spells[j])
    sprintf(buf, "%s%d ",buf,clan[i].spells[j]);
*/
sprintf(buf, "%s\r\n",buf);

send_to_char(buf, ch);
sprintf(buf, "Application fee  : %d gold\r\nMonthly Dues     : %d gold\r\n", clan[i].app_fee, clan[i].dues);
send_to_char(buf, ch);
sprintf(buf, "Per-kill Tax     : %d percent\r\n", clan[i].tax);
send_to_char(buf, ch);
sprintf(buf, "Application level: %d\r\n", clan[i].app_level);
send_to_char(buf, ch);

sprintf(buf,"\r\nClan privileges:\r\n");

for(j=0; j<5;j++)
  sprintf(buf, "%s%s: [%d] ",buf,clan_privileges[j],clan[i].privilege[j]);

sprintf(buf, "%s\r\n", buf);

for(j=5; j<NUM_CP;j++)
  sprintf(buf, "%s%s: [%d] ",buf,clan_privileges[j],clan[i].privilege[j]);
sprintf(buf, "%s\r\n",buf);
send_to_char(buf, ch);

/*
sprintf(buf, "Description:\r\n%s\r\n\n", clan[i].description);
send_to_char(buf, ch);
*/
if((!clan[i].at_war)) {
  send_to_char("\r\nThis clan is at peace with all others.\r\n", ch);
}
else {
  sprintf(buf, "\r\nThis clan has declared war on %s\r\n", clan[find_clan_by_id(clan[i].at_war)].name);
  send_to_char(buf, ch);
}

if(clan[i].alliance)
{
  sprintf(buf, "This clan is allied with %s.\r\n", clan[find_clan_by_id(clan[i].alliance)].name);
  send_to_char(buf, ch);
}

sprintf(buf, "PK       Wins: %-5d  Losses: %-5d\r\n", clan[i].pkwin, clan[i].pklose);
send_to_char(buf, ch);
sprintf(buf, "\r\nThis clan owns the following zones:\r\n");
send_to_char(buf, ch);
  for (d=0;d<=top_of_zone_table;d++) {
   if (zone_table[d].owner != -1) {
    if (zone_table[d].owner == clan[i].id) {
      if(zone_table[d].number == clan[i].hall) {
       sprintf(buf, "/cc %s (CLAN HALL) /c0\r\n", zone_table[d].name);
      } else {
       sprintf(buf, "/cc %s /c0\r\n", zone_table[d].name);
      }
      send_to_char(buf, ch);      
    } 
   }
  }
return;
}

sh_int find_clan_by_id(int idnum)
{
  int i;

  for( i=0; i < num_of_clans; i++)
    if(idnum==clan[i].id)
      return i;
  return -1;

}

sh_int find_clan(char *name)
{

  int i;
  char tmp[128];

  for( i=0; i < num_of_clans; i++) {
    strcpy(tmp, clan[i].name);
    proc_color(tmp, 0);
    if(strn_cmp(CAP(name), CAP(tmp), 32)==0)
      return i;
  }
  return -1;

}

void save_clans()
{
FILE *fl;

if (!(fl = fopen(CLAN_FILE, "wb"))) {
  log("SYSERR: Unable to open clan file");
  return; }

fwrite(&num_of_clans, sizeof(int), 1, fl);
fwrite(clan, sizeof(struct clan_rec), num_of_clans, fl);
fclose(fl);
return;
}


void init_clans()
{
FILE *fl;
int i,j,d;
extern int top_of_p_table;
extern struct player_index_element *player_table;
struct char_file_u chdata;

memset(clan,0,sizeof(struct clan_rec)*MAX_CLANS);
num_of_clans=0;
i=0;

if (!(fl = fopen(CLAN_FILE, "rb"))) {
  log("   Clan file does not exist. Will create a new one");
  save_clans();
  return; }

fread(&num_of_clans, sizeof(int), 1, fl);
fread(clan, sizeof(struct clan_rec), num_of_clans, fl);
fclose(fl);

log("   Calculating powers and members");
for(i=0;i<num_of_clans;i++) {
  clan[i].power=0;
  clan[i].members=0;
for (d=0;d<=top_of_zone_table;d++) {
   if (zone_table[d].owner == clan[i].id) {
     clan[i].power += 50;
  }
 }

}
for (j = 0; j <= top_of_p_table; j++){
  load_char((player_table + j)->name, &chdata);
  if((i=find_clan_by_id(chdata.player_specials_saved.clan))>=0) {
         clan[i].power+=chdata.level;
    if((chdata.class) >= (CLASS_ASSASSIN))
	 clan[i].power+=150;
    if((chdata.class) >= (CLASS_VAMPIRE)) 
	 clan[i].power+=150;	
   clan[i].members++;
  }
}

return;
}

void do_clan_bank(struct char_data *ch, char *arg, int action)
{
int clan_num,immcom=0;
long amount=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT && !IS_NPC(ch)) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD && !IS_NPC(ch)) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; 
  }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_WITHDRAW] && !immcom && action
==CB_WITHDRAW) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(*arg)) {
  send_to_char("Deposit how much?\r\n",ch);
  return;
  }

if(!is_number(arg)) {
  send_to_char("Deposit what?\r\n",ch);
  return;
  }

amount=atoi(arg);

if(!immcom && action==CB_DEPOSIT && GET_GOLD(ch)<amount) {
  send_to_char("You do not have that kind of money!\r\n",ch);
  return;
  }

if(action==CB_WITHDRAW && clan[clan_num].treasure<amount) {
  send_to_char("The clan is not wealthy enough for your needs!\r\n",ch);
  return;
  }

switch(action) {
  case CB_WITHDRAW:
    GET_GOLD(ch)+=amount;
    clan[clan_num].treasure-=amount;
    sprintf(buf, "CLAN: %s withdrew %ld from %s/c0", GET_NAME(ch),
            amount, clan[clan_num].name);
    log(buf);
    send_to_char("You withdraw from the clan's treasure.\r\n",ch);
    break;
  case CB_DEPOSIT:
    if(!immcom) GET_GOLD(ch)-=amount;
    clan[clan_num].treasure+=amount;
    sprintf(buf, "CLAN: %s deposited %ld to %s/c0", GET_NAME(ch), 
            amount, clan[clan_num].name);
    log(buf);
    send_to_char("You add to the clan's treasure.\r\n",ch);
    break;
  default:
    send_to_char("Problem in command, please report.\r\n",ch);
    break;
  }
save_char(ch, ch->in_room);
save_clans();
return;
}

void do_clan_money(struct char_data *ch, char *arg, int action)
{
int clan_num,immcom=0;
long amount=0;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_FEES] && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(*arg)) {
  send_to_char("Set it to how much?\r\n",ch);
  return;
  }

if(!is_number(arg)) {
  send_to_char("Set it to what?\r\n",ch);
  return;
  }

amount=atoi(arg);

switch(action) {
  case CM_APPFEE:
    clan[clan_num].app_fee=amount;
    send_to_char("You change the application fee.\r\n",ch);
    break;
  case CM_DUES:
    clan[clan_num].dues=amount;
    send_to_char("You change the monthly dues.\r\n",ch);
    break;
  case CM_TAX:
    if(amount > 99) { amount = 99; } 
    clan[clan_num].tax=amount;
    send_to_char("You change the clan per-kill tax.\r\n", ch);
    break;
  default:
    send_to_char("Problem in command, please report.\r\n",ch);
    break;
  }

save_clans();
return;
}

void do_clan_ranks(struct char_data *ch, char *arg)
{
int i,j;
int clan_num,immcom=0;
int new_ranks;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];
extern int top_of_p_table;
extern struct player_index_element *player_table;
struct char_file_u chdata;
struct char_data *victim=NULL;

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(*arg)) {
  send_to_char("Set how many ranks?\r\n",ch);
  return;
  }

if(!is_number(arg)) {
  send_to_char("Set the ranks to what?\r\n",ch);
  return;
  }

new_ranks=atoi(arg);

if(new_ranks==clan[clan_num].ranks) {
  send_to_char("The clan already has this number of ranks.\r\n",ch);
  return;
  }

if(new_ranks<2 || new_ranks>20) {
  send_to_char("Clans must have from 2 to 20 ranks.\r\n",ch);
  return;
  }

if(GET_GOLD(ch)<750000 && !immcom) {
  send_to_char("Changing the clan hierarchy requires 750,000 coins!\r\n",ch);
  return;
  }

if(!immcom)
  GET_GOLD(ch)-=750000;

for (j = 0; j <= top_of_p_table; j++) {
  if((victim=is_playing((player_table +j)->name))) {
    if(GET_CLAN(victim)==clan[clan_num].id) {
      if(GET_CLAN_RANK(victim)<clan[clan_num].ranks && GET_CLAN_RANK(victim)>0)
        GET_CLAN_RANK(victim)=1;
      if(GET_CLAN_RANK(victim)==clan[clan_num].ranks)
        GET_CLAN_RANK(victim)=new_ranks;
      save_char(victim, victim->in_room);
    }
  }
  else {
    load_char((player_table + j)->name, &chdata);
    if(chdata.player_specials_saved.clan==clan[clan_num].id) {
      if(chdata.player_specials_saved.clan_rank<clan[clan_num].ranks && chdata.
player_specials_saved.clan_rank>0)
        chdata.player_specials_saved.clan_rank=1;
      if(chdata.player_specials_saved.clan_rank==clan[clan_num].ranks)
        chdata.player_specials_saved.clan_rank=new_ranks;
      save_char_file_u(chdata);
    }
  }
}

clan[clan_num].ranks=new_ranks;
for(i=0;i<clan[clan_num].ranks-1;i++)
  strcpy(clan[clan_num].rank_name[i],"Member");
strcpy(clan[clan_num].rank_name[clan[clan_num].ranks -1],"Leader");
for(i=0;i<NUM_CP;i++)
  clan[clan_num].privilege[i]=new_ranks;

save_clans();
return;
}

void do_clan_titles( struct char_data *ch, char *arg)
{
char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
int clan_num=0,rank;

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
  if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg2);
  if(!is_number(arg1)) {
    send_to_char("You need to specify a clan number.\r\n",ch);
    return;
  }
  if((clan_num=atoi(arg1))<0 || clan_num>=num_of_clans) {
    send_to_char("There is no clan with that number.\r\n",ch);
    return;
  }
}

half_chop(arg,arg1,arg2);

if(!is_number(arg1)) {
  send_to_char("You need to specify a rank number.\r\n",ch);
  return; }

rank=atoi(arg1);

if(rank<1 || rank>clan[clan_num].ranks) {
  send_to_char("This clan has no such rank number.\r\n",ch);
  return; }

if(strlen(arg2)<1 || strlen(arg2)>19) {
  send_to_char("You need a clan title of under 20 characters.\r\n",ch);
  return; }

strcpy(clan[clan_num].rank_name[rank-1],arg2);
save_clans();
send_to_char("Done.\r\n",ch);
return;
}

void do_clan_application( struct char_data *ch, char *arg)
{
int clan_num,immcom=0;
int applevel;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg2)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_APPLEV] && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(*arg)) {
  send_to_char("Set to which level?\r\n",ch);
  return;
  }

if(!is_number(arg)) {
  send_to_char("Set the application level to what?\r\n",ch);
  return;
  }

applevel=atoi(arg);

if(applevel<1 || applevel>999) {
  send_to_char("The application level can go from 1 to 999.\r\n",ch);
  return;
  }

clan[clan_num].app_level=applevel;
save_clans();

return;
}

void do_clan_sp(struct char_data *ch, char *arg, int priv)
{
int clan_num,immcom=0;
int rank;
char arg1[MAX_INPUT_LENGTH];
char arg2[MAX_INPUT_LENGTH];

if (!(*arg)) {
  send_clan_format(ch);
  return; }


if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return;
  }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  immcom=1;
  half_chop(arg,arg1,arg2);
  strcpy(arg,arg1);
  if ((clan_num = find_clan(arg1)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }
}

if(GET_CLAN_RANK(ch)!=clan[clan_num].ranks && !immcom) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return;
  }

if(!(*arg)) {
  send_to_char("Set the privilege to which rank?\r\n",ch);
  return;
  }

if(!is_number(arg)) {
  send_to_char("Set the privilege to what?\r\n",ch);
  return;
  }

rank=atoi(arg);

if(rank<1 || rank>clan[clan_num].ranks) {
  send_to_char("There is no such rank in the clan.\r\n",ch);
  return;
  }

clan[clan_num].privilege[priv]=rank;
save_clans();

return;
}

void do_clan_plan(struct char_data *ch, char *arg)
{
int clan_num;

send_to_char("Command not ready yet\r\n",ch);
return;

if(GET_LEVEL(ch)<LVL_IMMORT) {
  if((clan_num=find_clan_by_id(GET_CLAN(ch)))<0) {
    send_to_char("You don't belong to any clan!\r\n",ch);
    return; }
  if(GET_CLAN_RANK(ch)<clan[clan_num].privilege[CP_SET_PLAN]) {
    send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
    return; }
}
else {
  if(GET_LEVEL(ch)<LVL_CLAN_GOD) {
    send_to_char("You do not have clan privileges.\r\n", ch);
    return; }
  if (!(*arg)) {
    send_clan_format(ch);
    return; }
  if ((clan_num = find_clan(arg)) < 0) {
    send_to_char("Unknown clan.\r\n", ch);
    return; }
}

if(strlen(clan[clan_num].description)==0) {
   sprintf(buf, "Enter the description, or plan for clan <<%s>>.\r\n",clan[clan_num].name);
   send_to_char(buf, ch);
}
else {
   sprintf(buf, "Old plan for clan <<%s>>:\r\n", clan[clan_num].name);
   send_to_char(buf, ch);
   send_to_char(clan[clan_num].description, ch);
   send_to_char("Enter new plan:\r\n", ch);
}
send_to_char("End with @ on a line by itself.\r\n", ch);
/*ch->desc->str   = clan[clan_num].description;*/
ch->desc->max_str = CLAN_PLAN_LENGTH;
save_clans();
return;
}

void do_clan_privilege( struct char_data *ch, char *arg)
{
char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
int i;

half_chop(arg,arg1,arg2);

if (is_abbrev(arg1,"setplan"  )) { do_clan_sp(ch,arg2,CP_SET_PLAN);   return ;}
if (is_abbrev(arg1,"enroll"   )) { do_clan_sp(ch,arg2,CP_ENROLL);     return ;}
if (is_abbrev(arg1,"expel"    )) { do_clan_sp(ch,arg2,CP_EXPEL);      return ;}
if (is_abbrev(arg1,"promote"  )) { do_clan_sp(ch,arg2,CP_PROMOTE);    return ;}
if (is_abbrev(arg1,"demote"   )) { do_clan_sp(ch,arg2,CP_DEMOTE);     return ;}
if (is_abbrev(arg1,"withdraw" )) { do_clan_sp(ch,arg2,CP_WITHDRAW);   return ;}
if (is_abbrev(arg1,"setfees"  )) { do_clan_sp(ch,arg2,CP_SET_FEES);   return ;}
if (is_abbrev(arg1,"setapplev")) { do_clan_sp(ch,arg2,CP_SET_APPLEV); return ;}
if (is_abbrev(arg1,"settax"))    { do_clan_sp(ch,arg2,CP_SET_TAX);    return ;}
send_to_char("\r\nClan privileges:\r\n", ch);
for(i=0;i<NUM_CP;i++) {
  sprintf(arg1,"\t%s\r\n",clan_privileges[i]);
  send_to_char(arg1,ch); }
}

void do_clan_set(struct char_data *ch, char *arg)
{
char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];

half_chop(arg,arg1,arg2);

if (is_abbrev(arg1, "plan"      )) { do_clan_plan(ch,arg2);            return ;
}
if (is_abbrev(arg1, "ranks"     )) { do_clan_ranks(ch,arg2);           return ;
}
if (is_abbrev(arg1, "title"     )) { do_clan_titles(ch,arg2);          return ;
}
if (is_abbrev(arg1, "privilege" )) { do_clan_privilege(ch,arg2);       return ;
}
if (is_abbrev(arg1, "dues"      )) { do_clan_money(ch,arg2,CM_DUES);   return ;
}
if (is_abbrev(arg1, "appfee"    )) { do_clan_money(ch,arg2,CM_APPFEE); return ;
}
if (is_abbrev(arg1, "applev"    )) { do_clan_application(ch,arg2);     return ;
}
if (is_abbrev(arg1, "tax"       )) { do_clan_money(ch,arg2,CM_TAX);    return ;
}
send_clan_format(ch);
}

ACMD(do_clan)
{
char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

half_chop(argument, arg1, arg2);

if (is_abbrev(arg1, "create"  )) { do_clan_create(ch,arg2);   return ;}
if (is_abbrev(arg1, "destroy" )) { do_clan_destroy(ch,arg2);  return ;}
if (is_abbrev(arg1, "enroll"  )) { do_clan_enroll(ch,arg2);   return ;}
if (is_abbrev(arg1, "expel"   )) { do_clan_expel(ch,arg2);    return ;}
if (is_abbrev(arg1, "who"     )) { do_clan_who(ch);           return ;}
if (is_abbrev(arg1, "status"  )) { do_clan_status(ch);        return ;}
if (is_abbrev(arg1, "info"    )) { do_clan_info(ch,arg2);     return ;}
if (is_abbrev(arg1, "apply"   )) { do_clan_apply(ch,arg2);    return ;}
if (is_abbrev(arg1, "demote"  )) { do_clan_demote(ch,arg2);   return ;}
if (is_abbrev(arg1, "promote" )) { do_clan_promote(ch,arg2);  return ;}
if (is_abbrev(arg1, "set"     )) { do_clan_set(ch,arg2);      return ;}
if (is_abbrev(arg1, "withdraw")) { do_clan_bank(ch,arg2,CB_WITHDRAW); return ;}
if (is_abbrev(arg1, "deposit" )) { do_clan_bank(ch,arg2,CB_DEPOSIT);  return ;}
if (is_abbrev(arg1, "list"    )) { do_clan_list(ch);          return; }
if (is_abbrev(arg1, "where"     )) { do_clan_where(ch);          return; }
if (is_abbrev(arg1, "war"     )) { do_clan_war(ch, arg2);          return; }
if (is_abbrev(arg1, "alliance"     )) { do_clan_alliance(ch, arg2);          return; }
send_clan_format(ch);
}


void do_clan_list(struct char_data *ch)
{

  FILE *pfile;
  int hits=0, i = 1;
  struct char_file_u vbuf;
  char buf[MAX_STRING_LENGTH];

  if(!GET_CLAN(ch)) {
    send_to_char("But you're not in a clan!\r\n", ch);
    return;
  }
  if (!(pfile = fopen(PLAYER_FILE, "r+b"))) {
    return;
  }

  sprintf(buf, "/cbClan Members:\r\n------------------------------------/c0\r\n"
);
  fseek(pfile, (long) (i * sizeof(struct char_file_u)), SEEK_SET);
  fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  for(i=2; !feof(pfile); i++) {
 if(find_clan_by_id(GET_CLAN(ch)) == find_clan_by_id(vbuf.player_specials_saved.clan)) {

      hits++;
  sprintf(buf, "%s/cy[%d %s %s/cy]/cw %s %s/cw %s (%s)/c0\r\n", buf,
      vbuf.level, race_abbrevs[(int) vbuf.race],
      class_abbrevs[(int) vbuf.class],
      rank[(int)vbuf.player_specials_saved.rank][(int)vbuf.sex],
      vbuf.name, vbuf.title,
      clan[find_clan_by_id(vbuf.player_specials_saved.clan)].rank_name[vbuf.player_specials_saved.clan_rank-1]);

    }
    fseek(pfile, (long) (i * sizeof(struct char_file_u)), SEEK_SET);
    fread(&vbuf, sizeof(struct char_file_u), 1, pfile);
  }
  fclose(pfile);

sprintf(buf, "%s%d players in the clan.\r\n", buf, hits);
  page_string(ch->desc, buf, 1);
}

void do_clan_war (struct char_data *ch, char *arg)
{

   int clan_num;

//send_to_char("This option is not yet availible.\r\n", ch);

  if (!(*arg)) {
  send_clan_format(ch);
  return; }

if(GET_CLAN_RANK(ch)<clan[find_clan_by_id(GET_CLAN(ch))].privilege[CP_EXPEL]) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return; }

if(clan[find_clan_by_id(GET_CLAN(ch))].war_timer) {
  sprintf(buf, "Sorry you can't pussy out for another %d minutes\r\n", clan[find_clan_by_id(GET_CLAN(ch))].war_timer);
  return;
}

  clan_num = atoi(arg);
  if (clan_num > num_of_clans) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }

if(clan_num == clan[find_clan_by_id(GET_CLAN(ch))].alliance) {
  send_to_char("You are already allied with that clan.\r\nIn order to declare war you must first break the alliance.\r\n", ch);
  return;
}

if(!clan_num) {
 if(!clan[find_clan_by_id(GET_CLAN(ch))].at_war) {
    send_to_char("Clan ID 0 does not exist.\r\n", ch);
    return;
 }
 send_to_char("You are no longer at war\r\n", ch);
 clan[find_clan_by_id(GET_CLAN(ch))].at_war = 0;
 return;
}

if(clan[find_clan_by_id(GET_CLAN(ch))].at_war) {
    if(clan[find_clan_by_id(GET_CLAN(ch))].at_war == clan_num) { 
      send_to_char("You are no longer at war.\r\n", ch);
      clan[find_clan_by_id(GET_CLAN(ch))].at_war = 0;

      return;
    }
   }

clan[find_clan_by_id(GET_CLAN(ch))].at_war = clan_num;
sprintf(buf, "/cRCLAN INFO: %s has declared war on %s!/c0\r\n", clan[find_clan_by_id(GET_CLAN(ch))].name, clan[find_clan_by_id(clan_num)].name);
send_to_all(buf);
send_to_char("So be it.\r\n", ch);
save_clans();
sprintf(buf, "%d", clan[find_clan_by_id(GET_CLAN(ch))].at_war);
send_to_char(buf, ch);
return;
}

void do_clan_alliance(struct char_data *ch, char *arg)
{

 int clan_num;


  if (!(*arg)) {
  send_clan_format(ch);
  return; }


if(GET_CLAN_RANK(ch)<clan[find_clan_by_id(GET_CLAN(ch))].privilege[CP_EXPEL]) {
  send_to_char("You're not influent enough in the clan to do that!\r\n",ch);
  return; }

  if (((clan_num = atoi(arg)) < 0) || (clan_num > num_of_clans)) {
    send_to_char("Unknown clan.\r\n", ch);
    return;
  }


if(clan_num == clan[find_clan_by_id(GET_CLAN(ch))].at_war) {
  send_to_char("You are already at war with that clan.\r\nIn order to ally you must first resind the declaration of war.\r\n", ch);
  return;
}

if(!clan_num) {
 if(!clan[find_clan_by_id(GET_CLAN(ch))].alliance) {
    send_to_char("Clan ID 0 does not exist.\r\n", ch);
    return;
 }
 send_to_char("You are no longer at war\r\n", ch);
 clan[find_clan_by_id(GET_CLAN(ch))].alliance = 0;
 return;
}


if(clan[find_clan_by_id(GET_CLAN(ch))].alliance) {
    if(clan[find_clan_by_id(GET_CLAN(ch))].alliance == clan_num) { 
      send_to_char("You break the alliance.\r\n", ch);
      clan[find_clan_by_id(GET_CLAN(ch))].alliance = 0;
      return;
    }
   }

clan[find_clan_by_id(GET_CLAN(ch))].alliance = clan_num;
sprintf(buf, "/cRCLAN INFO: %s has allied with %s!/c0\r\n", clan[find_clan_by_id(GET_CLAN(ch))].name, clan[find_clan_by_id(clan_num)].name);
send_to_all(buf);
send_to_char("So be it.\r\n", ch);
save_clans();
return;
}


void do_clan_where(struct char_data *ch)
{

//  register struct char_data *i;
//  struct descriptor_data *d;
//  int found = 0;

 /*   send_to_char("Clan Members Playing\r\n-------\r\n", ch);
    for (d = descriptor_list; d; d = d->next)
      if (!d->connected) {
	i = (d->original ? d->original : d->character);
	if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
        if(clan[GET_CLAN(ch)].id == clan[GET_CLAN(i)].id) {
           if(find_clan_by_id(GET_CLAN(ch)) == find_clan_by_id(GET_CLAN(i))) {
	    sprintf(buf, "%-20s - %s\r\n", GET_NAME(i),
	  	    world[i->in_room].name);
            found = TRUE;
           }
          }
	  send_to_char(buf, ch);
      }
    if (!found) */
      send_to_char("Not a soul around, you think you might hear a cricket chirping...\r\n", ch);
}


SPECIAL(toll_clan)
{
  /* Toll guards will force a player to give them money in  ** 
  ** exchange for passage.  This was originally designed    **
  ** for McGintey Cove but could be used anywhere.  Just    ** 
  ** update the below structure.                            */

  int i, found = FALSE, clan_num = zone_table[world[ch->in_room].zone].owner;
  struct char_data *vict = (struct char_data *) me;


  /* mob vnum, dir to block, toll (in gold) */
  const int toll_clan[][3]={
    {1258, SCMD_EAST, 50},
    {1258, SCMD_NORTH, 50},
    {  -1,   -1,  -1}
  };


  if(!IS_MOVE(cmd) && !CMD_IS("pay")) {
    if(in_war_zone(ch) && !FIGHTING(vict)) {
      do_generic_knock_down(vict, ch, SKILL_BASH);
      mobsay(ch, "How dare you trespass here!");
    }
    return FALSE;
  }

  for (i = 0; toll_clan[i][0] > 0 && !found; i++)
    if (GET_MOB_VNUM((struct char_data *)me) == toll_clan[i][0] &&
        IN_ROOM(ch) == IN_ROOM((struct char_data *)me))
      found=TRUE;

  if (!found)
    return FALSE;
  i--;

  if (cmd == toll_clan[i][1]) {
    switch(toll_clan[i][0]) {
    default:
      act("$N steps in front of $n and holds $S hand out for payment.",
          FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
      sprintf(buf, "This land is owned by %s and they require a toll of %d gold to enter.", clan[clan_num].name, toll_clan[i][2]);
      mobsay((struct char_data *) me, buf);
      break;
    return TRUE;
   }
   return TRUE;
  }
  else if(CMD_IS("pay")) {
    if (GET_GOLD(ch) >= toll_clan[i][2]) {
      GET_GOLD(ch) -= toll_clan[i][2];
    if(zone_table[world[ch->in_room].zone].owner >= 0) {
      // Put the gold in the clan bank (Vedic)
      clan[clan_num].treasure += toll_clan[i][2]; 
    }


      switch(toll_clan[i][0]) {
      case 1258:
        act("$n hands $N the payoff and leaves unmolested.",
            FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
        act("You bow your head in acquiescance and pay the ridiculous "
            "amount of 50 coins.", FALSE, ch, 0, 0, TO_CHAR);
        break;
      }
      do_move(ch, "", toll_clan[i][1], 0);
    } else
      switch(toll_clan[i][0]) {
      default:
        send_to_char("You haven't got the money.  Get lost!\r\n", ch);
        break;
      }
    return TRUE;
  }
  return FALSE; 
}

SPECIAL(clan_guard)
{

  struct char_data *vict = (struct char_data *) me;
   if(!cmd) {
     if(FIGHTING(vict) && !number(0,3)) { 
     sprintf(buf, "/cR%s shouts, 'Help! we are being raided by %s!'/c0\r\n", GET_NAME(vict), GET_NAME(FIGHTING(vict)));
     send_to_all(buf);
     if(GET_CLAN(FIGHTING(vict))) {
       clan[zone_table[world[vict->in_room].zone].owner -1].raided = GET_CLAN(FIGHTING(vict));
       clan[zone_table[world[vict->in_room].zone].owner -1].seen = 10;
       save_clans();
     }
    }
    if(in_war_zone(ch) && !FIGHTING(vict)) {
      do_generic_knock_down(vict, ch, SKILL_BASH);
      mobsay(ch, "How dare you trespass here!");
    }
    return FALSE;
  }

  if ((cmd == SCMD_NORTH || cmd == SCMD_EAST || cmd == SCMD_WEST || 
       cmd == SCMD_UP || cmd == SCMD_DOWN || cmd == SCMD_NORTHEAST || 
       cmd == SCMD_NORTHWEST || cmd == SCMD_SOUTHEAST || 
       cmd == SCMD_SOUTHWEST) && GET_LEVEL(ch) < LVL_IMMORT) {
  if(IS_NPC(ch) || !GET_CLAN(ch)) {
     send_to_char("Your kind isn't allowed here.\r\n", ch);
     return TRUE;
  }

    if (GET_CLAN(ch) != zone_table[world[IN_ROOM((struct char_data *)me)].zone].owner || GET_CLAN_RANK(ch) == 0) {

      act("$N blocks you and says 'Members only!'",
          FALSE, ch, 0, (struct char_data *)me, TO_CHAR);
      act("$N blocks $n and says 'Members only!'",
          FALSE, ch, 0, (struct char_data *)me, TO_ROOM);
      return TRUE;
    }
  }
  return FALSE; 
}

/* First, the basics: finding the filename; loading/saving objects */
/* Return a filename given a house vnum */
int vault_get_filename(int vnum, char *filename)
{
  if (vnum < 0)
    return 0;

  sprintf(filename, "vaults/%d.vault", vnum);
  return 1;
}
/* Load all objects for a vault */
int vault_load(room_vnum vnum)
{
  FILE *fl;
  char fname[MAX_STRING_LENGTH];
  struct obj_file_elem object;
  room_rnum rnum;

  if ((rnum = real_room(vnum)) == -1)
    return 0;
  if (!vault_get_filename(vnum, fname))
    return 0;
  if (!(fl = fopen(fname, "r+b"))) {
    /* no file found */
    return 0;
  }
  while (!feof(fl)) {
    fread(&object, sizeof(struct obj_file_elem), 1, fl);
    if (ferror(fl)) {
      perror("Reading vault file: vault_load.");
      fclose(fl);
      return 0;
    }
    if (!feof(fl))
      obj_to_room(Obj_from_store(object), rnum);
  }

  fclose(fl);

  return 1;
}

/* Save all objects for a vault (recursive; initial call must be followed
   by a call to vault_restore_weight)  Assumes file is open already. */
int vault_save(struct obj_data * obj, FILE * fp)
{
  struct obj_data *tmp;
  int result;

  if (obj) {
    if (!IS_OBJ_STAT(obj, ITEM_NORENT)) {
      vault_save(obj->contains, fp);
      vault_save(obj->next_content, fp);
      result = Obj_to_store(obj, fp);
      if (!result)
        return 0;

    for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
      GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    }
  }
  return 1;
}

/* restore weight of containers after vault_save has changed them for saving */
void vault_restore_weight(struct obj_data * obj)
{
  if (obj) {
    vault_restore_weight(obj->contains);
    vault_restore_weight(obj->next_content);
    if (obj->in_obj)
      GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
  }
}

/* Save all objects in a vault */
void vault_crashsave(room_vnum vnum)
{
  int rnum;
  char buf[MAX_STRING_LENGTH];
  FILE *fp;

  if ((rnum = real_room(vnum)) == -1)
    return;
  if (!vault_get_filename(vnum, buf))
    return;
  if (!(fp = fopen(buf, "wb"))) {
    perror("SYSERR: Error saving vault file");
    return;
  }
  if (!vault_save(world[rnum].contents, fp)) {
    fclose(fp);
    return;
  }
  fclose(fp);
  vault_restore_weight(world[rnum].contents);
}

/* call from boot_db */
void vault_boot(void)
{
  int curr_room, room_num;
  
  for (curr_room = 0; curr_room <= top_of_world; curr_room++){
    room_num = world[curr_room].number;
    if(ROOM_FLAGGED(real_room(room_num), ROOM_VAULT)) {
      vault_load(world[curr_room].number);
    }
  }
}

/* crash-save all the vault */
void vault_save_all(void)
{
  int curr_room, room_num;
  
  for (curr_room = 0; curr_room <= top_of_world; curr_room++) {
    room_num = world[curr_room].number;
    if (ROOM_FLAGGED(real_room(room_num), ROOM_VAULT)) {
      vault_crashsave(world[curr_room].number);
    }
  }
}

// New Functions 20011212 (DV)

bool in_war_zone(struct char_data *ch) {
  if((GET_CLAN(ch)) && (clan[find_clan_by_id(GET_CLAN(ch))].at_war == zone_table[world[ch->in_room].zone].owner) &&  (clan[find_clan_by_id(zone_table[world[ch->in_room].zone].owner)].at_war == find_clan_by_id(GET_CLAN(ch)))) {
      return TRUE;
  }
  return FALSE;
}

bool in_allied_lands(struct char_data *ch) {
  if((GET_CLAN(ch)) && (clan[find_clan_by_id(GET_CLAN(ch))].alliance == zone_table[world[ch->in_room].zone].owner) &&
  (clan[find_clan_by_id(zone_table[world[ch->in_room].zone].owner)].alliance == find_clan_by_id(GET_CLAN(ch)))) {
      return TRUE;
  }
  return FALSE;
}

bool in_my_clan_lands(struct char_data *ch) {
  if((GET_CLAN(ch)) && (find_clan_by_id(GET_CLAN(ch)) == zone_table[world[ch->in_room].zone].owner)) {
      return TRUE;
  }
  return FALSE;
}

bool in_other_clans_lands(struct char_data *ch) {

  if(!zone_table[world[ch->in_room].zone].owner) { return FALSE; }

  if(!GET_CLAN(ch)) { return FALSE; }

  if(find_clan_by_id(GET_CLAN(ch)) != zone_table[world[ch->in_room].zone].owner) {
      return TRUE;
  }
  return FALSE;
}

void claim_zone(struct char_data *ch, struct obj_data *obj)
{
	int zone_nr = 0;
	if((GET_OBJ_TYPE(obj) == ITEM_DEED) && (ROOM_FLAGGED(IN_ROOM(ch), ROOM_VAULT)))
	{
		if(GET_CLAN(ch))
		{
			zone_nr = (int)GET_OBJ_VAL(obj, 0);
			if(zone_table[zone_nr].owner != GET_CLAN(ch))
			{
				zone_table[zone_nr].owner = find_clan_by_id(GET_CLAN(ch));
				send_to_char("You claim the zone for your clan!\r\n", ch);
				zedit_save_to_disk(zone_nr);
				clan[find_clan_by_id(GET_CLAN(ch))].power += 1000;
				sprintf(buf, "%s has claimed %s for %s!\r\n", GET_NAME(ch), zone_table
					[zone_nr].name, clan[find_clan_by_id(GET_CLAN(ch))].name);
				send_to_all(buf);
				if(GET_RANK(ch) < RANK_EMPEROR) {GET_RANK(ch)++;}
			}
		}
	}
}


void warn_about_owned_zone(struct char_data *ch) {

  bool found = 0;
  int j = 0;

  if(!GET_CLAN(ch)) { return; }

  if(!zone_table[world[ch->in_room].zone].owner) { return; }

  for (j = 0; j < NUM_OF_DIRS; j++) {
      if (world[ch->in_room].zone != world[world[ch->in_room].dir_option[j]->to_room].zone) {
         found = 1;
      }   
  }
  if(found) {
    sprintf(buf, "A sign has been posted here. It is labeled, 'This land is property of %s.'\r\n", clan[find_clan_by_id(zone_table[world[ch->in_room].zone].owner)].name);
    send_to_char(buf, ch);
  }
 
}
