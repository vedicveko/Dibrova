/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define __CONFIG_C__

#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "hometowns.h"

#define TRUE	1
#define YES		1
#define YEP		1
#define FALSE	0
#define NO		0
#define NOPE	0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/


/* GAME PLAY OPTIONS */

/*
 * pk_allowed sets the tone of the entire game.  If pk_allowed is set to
 * NO, then players will not be allowed to kill, summon, charm, or sleep
 * other players, as well as a variety of other "asshole player" protections.
 * However, if you decide you want to have an all-out knock-down drag-out
 * PK Mud, just set pk_allowed to YES - and anything goes.
 */
int pk_allowed = NO;

/* is playerthieving allowed? */
int pt_allowed = YES;

/* minimum level a player must be to shout/holler/gossip/auction */
int level_can_shout = 1;

/* number of movement points it costs to holler */
int holler_move_cost = 20;

/* max level considered to be a newbie. */
int newbie_level = 10;

/* exp change limits */
int max_exp_gain = 1000000;	/* max gainable per kill */
int max_exp_loss = 10000000;	/* max losable per death */

/* number of tics (usually 75 seconds) before PC/NPC corpses decompose */
int max_npc_corpse_time = 5;
int max_pc_corpse_time = 100;

/* should items in death traps automatically be junked? */
int dts_are_dumps = YES;

/* "okay" etc. */
char *OK = "Okay.\r\n";
char *NOPERSON = "No-one by that name here.\r\n";
char *NOEFFECT = "Nothing seems to happen.\r\n";

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.)
 */
int free_rent = NO;

/* maximum number of items players are allowed to rent */
int max_obj_save = 80;

/* receptionist's surcharge on top of item costs */
int min_rent_cost = 100;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.
 */
int auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int autosave_time = 5;

/* Lifetime of crashfiles and forced-rent (idlesave) files in days */
int crash_file_timeout = 10;

/* Lifetime of normal rent files in days */
int rent_file_timeout = 30;


/****************************************************************************/
/****************************************************************************/


/* ROOM NUMBERS */

/* virtual number of room that immorts should enter at by default */
int immort_start_room = 1204;

/* virtual number of room that frozen players should enter at */
int frozen_start_room = 1202;

/*
 * virtual numbers of donation rooms.  note: you must change code in
 * do_drop of act.item.c if you change the number of non-NOWHERE
 * donation rooms.
 */
int donation_room_1 = 3063;
int donation_room_2 = NOWHERE;	/* unused - room for expansion */
int donation_room_3 = NOWHERE;	/* unused - room for expansion */


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/*
 * This is the default port the game should run on if no port is given on
 * the command-line.  NOTE WELL: If you're using the 'autorun' script, the
 * port number there will override this setting.  Change the PORT= line in
 * instead of (or in addition to) changing this.
*/
int DFLT_PORT = 4000;

/* default directory to use as data directory */
char *DFLT_DIR = "lib";

/* maximum number of players allowed before game starts to turn people away */
int MAX_PLAYERS = 50;

/* maximum size of bug, typo and idea files in bytes (to prevent bombing) */
int max_filesize = 50000;

/* maximum number of password attempts before disconnection */
int max_bad_pws = 3;

/*
 * Some nameservers are very slow and cause the game to lag terribly every 
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

int nameserver_is_slow = YES;


char *MENU =
"\r\n\r\n\r\n"
"                                /cy ______\r\n"
"/cw              /'''''/''''/'''/''/'////\\\\'\\''\\'''\\''''\\'''''\\\r\n"
"              '                                          '\r\n"
"              '        /cW0) /cLExit Dibrova./cw                  '\r\n"
"              '        /cW1) /cLStart playing./cw                 '\r\n"
"              '        /cW2) /cLRead the background story./cw     '\r\n"
"              '        /cW3) /cLChange password./cw               '\r\n"
"              '        /cW4) /cLDelete this character./cw         '\r\n"
"              '        /cW5) /cLList playing characters./cw       '\r\n"
"              '                                          '\r\n"
"              \\,,,,,\\,,,,\\,,,\\,,\\,\\\\////,/,,/,,,/,,,,/,,,,,/\r\n"
"/cy                                 |    |\r\n"
"                                 |    |    /cWWhat say you?/cy\r\n"
"                                 |    |\r\n"
"                        /cg ________/cy|/cg____/cy|/cg________/c0"
"\r\n";
/*
char *GREETINGS = 

"\r\n"
"\aINTERNAL SERVER ERROR\r\n"
"Malfunction in register 0xf45\r\n"
"Please reboot and try again\r\n"
"\r\n\r\n\r\nOr Not\r\n\r\n\r\n"
"By what name do wish to be known? ";

 APRIL FOOLS HAHAH */

char *GREETINGSTWO =

"\r\n/c0"
"                  ,/                          `,\r\n"
"        _________{(           :D 2:          })_________\r\n"
"       /.-------./``         DIBROVA        //`.-------.``\r\n"
"      //@@@@@@@//@@``  )                    (  //@@``@@@@@@@``\r\n"
"     //@@@@@@@//@@@@>>/       A Fantasy     `<<@@@@``@@@@@@@``\r\n"
"    //O@O@O@O//@O@O//      Roleplaying Game    ``O@O@``O@O@O@O``\r\n"
"  //OOOOOOOO//OOOO||            `  /          ||OOOO``OOOOOOOO``\r\n"
" //O%O%O%O%//O%O%O%``           ))((          //%O%O%O``%O%O%O%O``\r\n"
"||%%%%%%%%//'  `%%%%``         //  \\        //%%%%'   ```%%%%%%%||\r\n"
"((%%%%%%%((      %%%%%``    ((    ))    //%%%%%       ))%%%%%%))\r\n"
" `:::' `::``      `:::::``   `)~~(/    //:::::'      //::' `:::/\r\n"
"  )'     `;)'      (`  ` `` `<@  @>' / / '  ')      `(;'     `(\r\n"
"          (               '`` )^^( /  /               )\r\n"
"      _                    ) ``oo/   (\r\n"
"     (@)                    '  `'   /                       _\r\n"
"     |-|'________oOo_________'__^__/_________oOo__________ (@)\r\n"
"     | |         VVV                         VVV          `|-|\r\n"
"     | |              Based on CircleMUD 3.0               | |\r\n"
"     |-|             Created by  Jeremy Elson              |-|\r\n"
"     | |         A derivative of DikuMUD (GAMMA 0.0)       | |\r\n"
"     | |                    Created by                     |-|\r\n"
"     | |         Hans Henrik Staerfeldt, Katja Nyboe,      | |\r\n"
"     | | Tom Madsen, Michael Seifert, and Sebastian Hammer |-|\r\n"
"     |_|`________________________________________________  | |\r\n"
"     (@)                   ` ,/ `_____/ `` ~`/~          ``|-|\r\n"
"      ~               ___//^~      `____/``                (@)\r\n"
"                     <<<  `     __  <____/||                ~\r\n"
"                               <-- ` <___/||\r\n"
"                                  || <___//\r\n"
"                                  ` `/__//\r\n"
"                                   ~----~\r\n\r\n"
"By what name do you wish to be known? ";

char *GREETINGS =

"\r\n/c0"
"                                     ,-.\r\n"
"                ___,---.__          //'|`\\          __,---,___\r\n"
"             ,-'    \\`    `-.____,-'  |  `-.____,-'    //    `-.\r\n"
"           ,'        |           -'\\     //`-           |        `.\r\n"
"          //      ___//               `. ,'          ,  , \\___      \\\r\n"
"         |    ,-'   `-.__   _         |        ,    __,-'   `-.    |\r\n"
"         |   //          //\\_  `   .    |    ,      _//\\          \\   |\r\n"
"         \\  |           \\ \\`-.___ \\   |   // ___,-'// //           |  //\r\n"
"          \\  \\           | `._   `\\\\  |  ////'   _,' |           //  //\r\n"
"           `-.\\         //'  _ `---'' , . ``---' _  `\\         //,-'\r\n"
"              ``       //     \\    ,='// \\`=.    //     \\       ''\r\n"
"                      |__   //|\\_,--.,-.--,--._//|\\   __|\r\n"
"                      //  `.//  \\`\\ |  |  | //,//'  \\,'   \\\r\n"
"                     //   //     ||--+--|--+-/-|     \\   \\\r\n"
"                    |   |     //'\\_\\_\\ | //_//_//`\\     |   |\r\n"
"                     \\   \\__, \\_     `-'     _// .__//   //\r\n"
"                      `-._,-'   `-._______,-'   `-._,-'\r\n"
"                                   Dibrova\r\n"
"                           Based on CircleMUD 3.0,\r\n"
"                           Created by Jeremy Elson\r\n"
"                     A derivative of DikuMUD (GAMMA 0.0)\r\n"
"                                Created by\r\n"
"                    Hans Henrik Staerfeldt, Katja Nyboe,\r\n"
"              Tom Madsen, Michael Seifert, and Sebastian Hammer\r\n"
"\r\n\r\n"
"By what name do you wish to be known? ";

char *WELC_MESSG =
"\r\n"
"Welcome to Dibrova!\r\n"
"   KIDS AND PARENTS - This game contains violence and language that may\r\n"
"                      not be suitable for kids.  If this sort of stuff\r\n"
"                      offends you, you should pick a different game.\r\n"
"      EVERYONE ELSE - Have fun!\r\n\r\n";


char *START_MESSG =
"Welcome and thanks for playing Dibrova.  If you are new to MUDding, I\r\n"
"suggest starting by checking out the help by typing HELP at the game\r\n"
"prompt.  Enjoy!\r\n";

/****************************************************************************/
/****************************************************************************/


/* AUTOWIZ OPTIONS */

/*
 * Should the game automatically create a new wizlist/immlist every time
 * someone immorts, or is promoted to a higher (or lower) god level?
 * NOTE: this only works under UNIX systems.
 */
int use_autowiz =NO; 

/* If yes, what is the lowest level which should be on the wizlist?  (All
   immort levels below the level you specify will go on the immlist instead.) */
int min_wizlist_lev = LVL_ADMIN;

