/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#include "conf.h"
#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"

extern struct room_data *world;
extern int top_of_world;
extern int mini_mud;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern int top_of_mobt;
extern struct char_data *mob_proto;


/* functions to perform assignments */

void ASSIGNMOB(int mob, SPECIAL(fname))
{
  int rnum;

  if ((rnum = real_mobile(mob)) >= 0)
    mob_index[rnum].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant mob #%d",
	    mob);
    log(buf);
  }
}

void ASSIGNOBJ(int obj, SPECIAL(fname))
{
  if (real_object(obj) >= 0)
    obj_index[real_object(obj)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant obj #%d",
	    obj);
    log(buf);
  }
}

void ASSIGNROOM(int room, SPECIAL(fname))
{
  if (real_room(room) >= 0)
    world[real_room(room)].func = fname;
  else if (!mini_mud) {
    sprintf(buf, "SYSERR: Attempt to assign spec to non-existant rm. #%d",
	    room);
    log(buf);
  }
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  int i;

  SPECIAL(postmaster);
  SPECIAL(clan_mail);
  SPECIAL(cityguard);
  SPECIAL(receptionist);
  SPECIAL(cryogenicist);
  SPECIAL(guild_guard);
  SPECIAL(guild);
  SPECIAL(puff);
  SPECIAL(fido);
  SPECIAL(janitor);
  SPECIAL(mayor);
  SPECIAL(snake);
  SPECIAL(thief);
  SPECIAL(magic_user);
  SPECIAL(street_tough);
  SPECIAL(blood_bank);
  SPECIAL(casino);
  SPECIAL(lava_beast);
  SPECIAL(ankou_follower);
  SPECIAL(flaming_efreet);
  SPECIAL(newbie_guard);
  SPECIAL(newbie_helper);
  SPECIAL(play_war);
  SPECIAL(simple_heal);
  SPECIAL(teleporter);
  SPECIAL(sergeant_follower);
  SPECIAL(newbie_teleport);
  SPECIAL(sund_earl);
  SPECIAL(hangman);  
  SPECIAL(blinder);
  SPECIAL(silktrader);
  SPECIAL(butcher);   
  SPECIAL(idiot);  
  SPECIAL(athos);
  SPECIAL(stu);
  SPECIAL(toll_guard);
  SPECIAL(rattlesnake);
  SPECIAL(robedthief);
  SPECIAL(believer);
  SPECIAL(hidden_believer);
  SPECIAL(gibbering_man);
  SPECIAL(gunter);
  SPECIAL(quiltclub);
  SPECIAL(styxferry);
  SPECIAL(gym);
  SPECIAL(library);
  SPECIAL(lord_keep_quest);
  SPECIAL(elven_tapestry_quest);
  SPECIAL(monk_guild);
  SPECIAL(paladin_guild);
  SPECIAL(druid_guild);
  SPECIAL(sorcerer_guild);
  SPECIAL(cleric_guild);
  SPECIAL(gladiator_guild);
  SPECIAL(thief_guild);
  SPECIAL(dkn_guild);
  SPECIAL(pon_neroth);
  SPECIAL(omel);
  SPECIAL(booger_beast);
  SPECIAL(duke_stalatharata);
  SPECIAL(dworn);
  SPECIAL(old_sailor);
  SPECIAL(pinky_quest);
  SPECIAL(riding_trainer);
  SPECIAL(rat);
  SPECIAL(class_mage);
  SPECIAL(class_warrior);
  SPECIAL(class_cleric);
  SPECIAL(clan_guard);
  SPECIAL(highwayman);
  SPECIAL(arrest);
  SPECIAL(elf_maiden);
  SPECIAL(ferry_tickets);
  SPECIAL(ogdlen);
  SPECIAL(ogdlen2);
  SPECIAL(mean_goblin);
  SPECIAL(yragg);
  SPECIAL(leprechaun);
  SPECIAL(netherman);
  SPECIAL(make_vamp);
  SPECIAL(fairy_follower);
  SPECIAL(fairy_catch);
  SPECIAL(fairy_hermit);
  SPECIAL(library_quest);
  SPECIAL(arena_quest_master);
  SPECIAL(arena_quest_exiter);
  SPECIAL(toll_clan);
  SPECIAL(token_exchange);
  SPECIAL(quest_shop);
  SPECIAL(bounty);
  SPECIAL(quest_mage);
  SPECIAL(bridgeguard);
  SPECIAL(immort_teacher);
  SPECIAL(pkill_allow);
  SPECIAL(grid_dude);
  SPECIAL(vendor_shop);
  SPECIAL(undertaker);
  SPECIAL(broker);
  SPECIAL(seer_rub);
  SPECIAL(greenber);
  SPECIAL(harried_maid);
  SPECIAL(Burly_gruff_guard);
  SPECIAL(Brutal_guard);
  SPECIAL(Maniac_rapist_with_money);
    
  void assign_kings_castle(void);

/* assign_kings_castle(); */
 
  // Vendors

  ASSIGNMOB(30203, vendor_shop);
  ASSIGNMOB(30506, vendor_shop); /* Clan DarkLords (Silvis) */
  ASSIGNMOB(30507, vendor_shop); /* Clan DarkLords (Jade) */
  ASSIGNMOB(30508, vendor_shop); /* Clan DarkLords (Katrina) */

  ASSIGNMOB(30503, postmaster); /* Clan DarkLords */

  ASSIGNMOB(12499, bounty);

  ASSIGNMOB(1265, broker);

  ASSIGNMOB(1099, undertaker);

  ASSIGNMOB(3015, pkill_allow);

   ASSIGNMOB(30402, simple_heal); /* Clan Wickeds Healer */
   ASSIGNMOB(30502, simple_heal); /* Clan DarkLords Healer */
   ASSIGNMOB(30204, simple_heal); /* Clan Eternals Healer */

   ASSIGNMOB(1258,  toll_clan); // Smelly Cow Toll

  /* The Dream */
  ASSIGNMOB(27122, Maniac_rapist_with_money);
  ASSIGNMOB(27128, Brutal_guard);
  ASSIGNMOB(27132, Burly_gruff_guard);
  
  ASSIGNMOB(27104, greenber); 
  ASSIGNMOB(27137, harried_maid); 

  /* Spec Quests */
  ASSIGNMOB(3145, quest_mage);
  ASSIGNMOB(25026, arena_quest_master);
  ASSIGNMOB(25027, arena_quest_exiter);
  ASSIGNMOB(25009, fairy_follower);
  ASSIGNMOB(25008, fairy_catch);
  ASSIGNMOB(25007, fairy_hermit);
  ASSIGNMOB(25070, grid_dude);
  ASSIGNMOB(3307, lord_keep_quest);
  ASSIGNMOB(19023, elven_tapestry_quest);
  ASSIGNMOB(25000, pon_neroth);
  ASSIGNMOB(25001, omel);
  ASSIGNMOB(25002, booger_beast);
  ASSIGNMOB(4400,  duke_stalatharata);
  ASSIGNMOB(25004, dworn);
  ASSIGNMOB(1505, old_sailor);
  ASSIGNMOB(1067, pinky_quest);
  ASSIGNMOB(1098, library_quest);

  ASSIGNMOB(161, ogdlen);
  ASSIGNMOB(165, ogdlen2);
  ASSIGNMOB(169, mean_goblin);

  ASSIGNMOB(25050, seer_rub);

  /* Immortal Zone */
  ASSIGNMOB(1201, postmaster);
  ASSIGNMOB(1230, clan_mail);
  ASSIGNMOB(1202, janitor);
  ASSIGNMOB(1079, postmaster);

  /* Vamps */
  ASSIGNMOB(2906, make_vamp);
  ASSIGNMOB(26700, immort_teacher);
  ASSIGNMOB(26701, immort_teacher);
  ASSIGNMOB(26702, immort_teacher);
  ASSIGNMOB(26703, immort_teacher);
  ASSIGNMOB(26704, immort_teacher);
  ASSIGNMOB(26705, immort_teacher);
  ASSIGNMOB(26706, immort_teacher);
  ASSIGNMOB(26707, immort_teacher);
  ASSIGNMOB(26708, immort_teacher);
  ASSIGNMOB(26709, immort_teacher);

  /* Titans */
  ASSIGNMOB(26710, immort_teacher);
  ASSIGNMOB(26711, immort_teacher);
  ASSIGNMOB(26712, immort_teacher);
  ASSIGNMOB(26713, immort_teacher);
  ASSIGNMOB(26714, immort_teacher);
  ASSIGNMOB(26715, immort_teacher);

  /* Saints */
  ASSIGNMOB(26715, immort_teacher);
  ASSIGNMOB(26716, immort_teacher);
  ASSIGNMOB(26717, immort_teacher);
  ASSIGNMOB(26718, immort_teacher);
  ASSIGNMOB(26719, immort_teacher);
  ASSIGNMOB(26720, immort_teacher);
  ASSIGNMOB(26721, immort_teacher);
  ASSIGNMOB(26722, immort_teacher);
  ASSIGNMOB(26723, immort_teacher);

  /* Demons */
  ASSIGNMOB(26724, immort_teacher);
  ASSIGNMOB(26725, immort_teacher);
  ASSIGNMOB(26726, immort_teacher);
  ASSIGNMOB(26727, immort_teacher);
  ASSIGNMOB(26728, immort_teacher);
  ASSIGNMOB(26729, immort_teacher);
  ASSIGNMOB(26730, immort_teacher);
  ASSIGNMOB(26731, immort_teacher);

  /* Midgaard */
  ASSIGNMOB(3010, postmaster);
  ASSIGNMOB(3020, guild);
  ASSIGNMOB(3021, guild);
  ASSIGNMOB(3022, guild);
  ASSIGNMOB(3023, guild);
  ASSIGNMOB(3096, teleporter);
  ASSIGNMOB(3073, guild);
  ASSIGNMOB(3072, guild);
  ASSIGNMOB(3075, guild);
  ASSIGNMOB(3077, guild);
  ASSIGNMOB(3024, guild_guard);
  ASSIGNMOB(3025, guild_guard);
  ASSIGNMOB(3026, guild_guard);
  ASSIGNMOB(3027, guild_guard);
  ASSIGNMOB(3072, guild_guard);
  ASSIGNMOB(3074, guild_guard);
  ASSIGNMOB(3076, guild_guard);
  ASSIGNMOB(3061, janitor);
  ASSIGNMOB(3062, fido);
  ASSIGNMOB(3066, fido);
  ASSIGNMOB(3068, janitor);
  ASSIGNMOB(3095, cryogenicist);
  ASSIGNMOB(3105, mayor);
  ASSIGNMOB(3058, newbie_guard);
  ASSIGNMOB(3069, newbie_helper);
  ASSIGNMOB(3070, guild);
  ASSIGNMOB(3071, guild_guard);
  ASSIGNMOB(3080, newbie_teleport);
  ASSIGNMOB(3097, yragg);
  ASSIGNMOB(1240, token_exchange);
  ASSIGNMOB(1239, quest_shop);

  /* New Thalos */
  ASSIGNMOB(17500, guild);
  ASSIGNMOB(17501, guild);
  ASSIGNMOB(17502, guild);
  ASSIGNMOB(17503, guild);
  ASSIGNMOB(17556, guild_guard);
  ASSIGNMOB(17557, guild_guard);
  ASSIGNMOB(17558, guild_guard);
  ASSIGNMOB(17559, guild_guard);
  ASSIGNMOB(17546, library);

  /* Mordilnia */
  ASSIGNMOB(14906, guild);
  ASSIGNMOB(14907, guild);
  ASSIGNMOB(14908, guild);
  ASSIGNMOB(14909, guild);

  /* The Light Forest */
  ASSIGNMOB(3599, netherman);

  /* The Swamp */
  ASSIGNMOB(2205, magic_user);


/* Jareth */
  ASSIGNMOB(1097, leprechaun);
  ASSIGNMOB(1010, postmaster);
  ASSIGNMOB(1014, casino);
  ASSIGNMOB(1020, guild);
  ASSIGNMOB(1021, guild);
  ASSIGNMOB(1022, guild);
  ASSIGNMOB(1023, guild);
  ASSIGNMOB(1032, guild);
  ASSIGNMOB(1035, guild);
  ASSIGNMOB(1037, guild);
  ASSIGNMOB(1038, guild);
  ASSIGNMOB(1027, guild_guard);
  ASSIGNMOB(1030, blood_bank);
  ASSIGNMOB(1033, guild_guard);
  ASSIGNMOB(1049, riding_trainer);
  ASSIGNMOB(1061, janitor);
// He's a pinky boy now  ASSIGNMOB(1067, cityguard);
  ASSIGNMOB(1031, street_tough);
  ASSIGNMOB(1073, guild_guard); /* Mage */
  ASSIGNMOB(1074, guild_guard);
  ASSIGNMOB(1075, guild_guard);
  ASSIGNMOB(1076, guild_guard);
  ASSIGNMOB(1077, guild_guard);
  ASSIGNMOB(1039, guild_guard);
  ASSIGNMOB(1034, guild_guard);
  ASSIGNMOB(1036, guild_guard);
  ASSIGNMOB(1095, cryogenicist);
  ASSIGNMOB(1080, newbie_teleport);
  ASSIGNMOB(1096, teleporter);
  ASSIGNMOB(1017, gym);
  ASSIGNMOB(1018, library);


  /* Realm of the Undead */
  ASSIGNMOB(3219, styxferry);

  /* Sundhaven */
  ASSIGNMOB(6600, sund_earl);        /* Earl of Sundhaven */
  ASSIGNMOB(6601, cityguard);
  ASSIGNMOB(6602, hangman);
  ASSIGNMOB(6664, postmaster);
  ASSIGNMOB(6656, guild_guard);
  ASSIGNMOB(6655, guild_guard); 
  ASSIGNMOB(6658, guild_guard);
  ASSIGNMOB(6657, guild_guard);
  ASSIGNMOB(6666, stu);
  ASSIGNMOB(6606, fido);             /* Smoke rat */
  ASSIGNMOB(6616, guild);
  ASSIGNMOB(6619, guild);
  ASSIGNMOB(6617, guild);  
  ASSIGNMOB(6618, guild);
  ASSIGNMOB(6607, thief);
  ASSIGNMOB(6648, butcher);
  ASSIGNMOB(6661, blinder);
  ASSIGNMOB(6637, silktrader);
  ASSIGNMOB(6615, idiot);
  ASSIGNMOB(6653, athos);

  /*Forest Village*/

  /* McGintey Cove */
  ASSIGNMOB(6710, postmaster);
  ASSIGNMOB(6727, simple_heal);
  ASSIGNMOB(6759, newbie_teleport);
  ASSIGNMOB(6717, fido);
  ASSIGNMOB(6758, teleporter);
  ASSIGNMOB(6806, guild_guard);
  ASSIGNMOB(6808, guild_guard);
  ASSIGNMOB(6802, guild_guard);
  ASSIGNMOB(6800, guild_guard);
  ASSIGNMOB(6804, guild_guard);
  ASSIGNMOB(6810, guild_guard);
  ASSIGNMOB(6840, guild_guard);
  ASSIGNMOB(6838, guild_guard);
  ASSIGNMOB(6807, guild);
  ASSIGNMOB(6809, guild);
  ASSIGNMOB(6803, guild);
  ASSIGNMOB(6801, guild);
  ASSIGNMOB(6805, guild);
  ASSIGNMOB(6811, guild);
  ASSIGNMOB(6841, guild);
  ASSIGNMOB(6839, guild);
  ASSIGNMOB(6907, play_war);
  ASSIGNMOB(6705, toll_guard);
  ASSIGNMOB(5311, ferry_tickets);
  /* Dun Pith */
  ASSIGNMOB(5854, postmaster);
  ASSIGNMOB(5861, casino);
  ASSIGNMOB(5862, play_war);

  /* Silverthorne */
  ASSIGNMOB(5705, janitor);
  ASSIGNMOB(5706, janitor);
  ASSIGNMOB(5717, postmaster);
  ASSIGNMOB(5719, guild_guard);
  ASSIGNMOB(5720, guild_guard);
  ASSIGNMOB(5721, guild_guard);
  ASSIGNMOB(5722, guild_guard);
  ASSIGNMOB(5723, guild_guard);
  ASSIGNMOB(5724, guild_guard);
  ASSIGNMOB(5725, guild_guard);
  ASSIGNMOB(5726, guild_guard);
  ASSIGNMOB(5727, guild);
  ASSIGNMOB(5728, guild);
  ASSIGNMOB(5729, guild);
  ASSIGNMOB(5730, guild);
  ASSIGNMOB(5731, guild);
  ASSIGNMOB(5732, guild);
  ASSIGNMOB(5733, guild);
  ASSIGNMOB(5734, guild);
  ASSIGNMOB(5735, newbie_teleport);
  ASSIGNMOB(5713, teleporter);

  /* Mordecai's */
  ASSIGNMOB(3613, thief);

  /* X'Raantra's */
  ASSIGNMOB(5901, believer);
  ASSIGNMOB(5913, hidden_believer);
  ASSIGNMOB(5902, gibbering_man);
  ASSIGNMOB(5909, gunter);

  /* Halfling Village */
  ASSIGNMOB(614, quiltclub);

  /* Lava Tubes */
  ASSIGNMOB(800, lava_beast);

  /* The Dark Realm of Lord Ankou */
  ASSIGNMOB(1404, ankou_follower);

  /*  */
  ASSIGNMOB(3902, flaming_efreet);
  ASSIGNMOB(3910, flaming_efreet);
  ASSIGNMOB(3908, flaming_efreet);

  /* SHIRE */
  ASSIGNMOB(6000, magic_user);

  /* Malathar */
  ASSIGNMOB(4405, janitor);
  ASSIGNMOB(4408, janitor);
  ASSIGNMOB(4409, janitor);
  ASSIGNMOB(4459, newbie_teleport);
  ASSIGNMOB(4430, guild);
  ASSIGNMOB(4431, guild);
  ASSIGNMOB(4432, guild);
  ASSIGNMOB(4433, guild);
  ASSIGNMOB(4434, guild);
  ASSIGNMOB(4435, guild);
  ASSIGNMOB(4436, guild);
  ASSIGNMOB(4460, guild_guard);
  ASSIGNMOB(4461, guild_guard);
  ASSIGNMOB(4300, postmaster);
  ASSIGNMOB(4458, teleporter);

  /* Ofingia */
  ASSIGNMOB(8112, janitor);
  ASSIGNMOB(8123, janitor);
  ASSIGNMOB(8178, newbie_teleport);
  ASSIGNMOB(8008, postmaster);
  ASSIGNMOB(8177, teleporter);
  ASSIGNMOB(8009, guild);
  ASSIGNMOB(8011, guild);
  ASSIGNMOB(8013, guild);
  ASSIGNMOB(8015, guild);
  ASSIGNMOB(8017, guild);
  ASSIGNMOB(8019, guild);
  ASSIGNMOB(8021, guild);
  ASSIGNMOB(8023, guild);
  ASSIGNMOB(8010, guild_guard);
  ASSIGNMOB(8012, guild_guard);
  ASSIGNMOB(8014, guild_guard);
  ASSIGNMOB(8016, guild_guard);
  ASSIGNMOB(8018, guild_guard);
  ASSIGNMOB(8020, guild_guard);
  ASSIGNMOB(8022, guild_guard);
  ASSIGNMOB(8024, guild_guard);

  /* Elven City */
  ASSIGNMOB(19067, janitor);
  ASSIGNMOB(19027, newbie_teleport);
  ASSIGNMOB(19050, guild_guard);
  ASSIGNMOB(19051, guild_guard);
  ASSIGNMOB(19052, guild_guard);
  ASSIGNMOB(19053, guild_guard);
  ASSIGNMOB(19054, guild_guard);
  ASSIGNMOB(19055, guild_guard);
  ASSIGNMOB(19056, guild_guard);
  ASSIGNMOB(19057, guild_guard);
  ASSIGNMOB(19058, guild);
  ASSIGNMOB(19059, guild);
  ASSIGNMOB(19060, guild);
  ASSIGNMOB(19061, guild);
  ASSIGNMOB(19062, guild);
  ASSIGNMOB(19063, guild);
  ASSIGNMOB(19064, guild);
  ASSIGNMOB(19065, guild);
  ASSIGNMOB(19070, postmaster);

  /* ROME
  ASSIGNMOB(12009, magic_user);
  ASSIGNMOB(12018, cityguard);
  ASSIGNMOB(12020, magic_user);
  ASSIGNMOB(12021, cityguard);
  ASSIGNMOB(12025, magic_user);
  ASSIGNMOB(12030, magic_user);
  ASSIGNMOB(12031, magic_user);
  ASSIGNMOB(12032, magic_user);
  */

  /* DWARVEN KINGDOM */
  ASSIGNMOB(6500, cityguard);
  ASSIGNMOB(6502, magic_user);
  ASSIGNMOB(6509, magic_user);
  ASSIGNMOB(6516, magic_user);
  ASSIGNMOB(6512, teleporter);

  /* Elven Woods */
  ASSIGNMOB(19010, cityguard);

  ASSIGNMOB(1800, sergeant_follower);
  ASSIGNMOB(1801, sergeant_follower);
  ASSIGNMOB(1802, sergeant_follower);


  /* Woods */
  ASSIGNMOB(601, rattlesnake);

  /* Thieven Temple */
  ASSIGNMOB(5406, robedthief);

  /* Vernige */
  ASSIGNMOB(19236, rat);
  ASSIGNMOB(19102, janitor);
  

   // Ferries

  ASSIGNMOB(31314, ferry_tickets);

 
  /* Dun Pith */
  ASSIGNMOB(5802, highwayman);
  ASSIGNMOB(5803, toll_guard);
  ASSIGNMOB(8400, arrest);

  /* King's Road and Castle */
  ASSIGNMOB(8400, arrest);
  ASSIGNMOB(8405, elf_maiden);

  /* Clan mobs */

  /* Villspah */
  ASSIGNMOB(9328, bridgeguard);


  for(i = 0; i < top_of_mobt; i++) {
    switch (mob_proto[i].player.class) {
    case CLASS_NPC_RECEPTION:
      ASSIGNMOB(mob_index[i].virtual, receptionist);
      break;
    case CLASS_NPC_CITY_GUARD:
      ASSIGNMOB(mob_index[i].virtual, cityguard);
      break;
    case CLASS_CLANGUARD:
      ASSIGNMOB(mob_index[i].virtual, clan_guard);
      break;
    default:
      break;
    }
  }
}



/* assign special procedures to objects */
void assign_objects(void)
{
  SPECIAL(bank);
  SPECIAL(gen_board);
  SPECIAL(marbles);
  SPECIAL(slot_machine);
  SPECIAL(docket);
  SPECIAL(bell);

  SPECIAL(scaredy_cat);
  
  ASSIGNOBJ(3096, gen_board);	/* social board */
  ASSIGNOBJ(3097, gen_board);	/* freeze board */
  ASSIGNOBJ(3098, gen_board);	/* immortal board */
  ASSIGNOBJ(3099, gen_board);	/* mortal board */
  ASSIGNOBJ(1205, gen_board);	/* builder board */
  ASSIGNOBJ(1226, gen_board);	/* coders board */
  ASSIGNOBJ(3082, gen_board);	/* quest board */
  ASSIGNOBJ(3087, gen_board);	/* adder's board */
  ASSIGNOBJ(3088, gen_board);	/* Paladin board */
  ASSIGNOBJ(3089, gen_board);	/* Druid board */
  ASSIGNOBJ(3090, gen_board);	/* Dark Knight board */
  ASSIGNOBJ(3091, gen_board);	/* Monk board */
  ASSIGNOBJ(3092, gen_board);	/* Cleric board */
  ASSIGNOBJ(3093, gen_board);	/* Sorcerer board */
  ASSIGNOBJ(3094, gen_board);	/* Galdiator board */
  ASSIGNOBJ(3095, gen_board);	/* Thief board */
  ASSIGNOBJ(3026, gen_board);   /* Suggestion Board */
  ASSIGNOBJ(3084, gen_board);   /* Changes Board */
  ASSIGNOBJ(1291, gen_board);   /* Delilmanduils Board */
  ASSIGNOBJ(1292, gen_board);   /* Raikers Board */

  /* Clan boards */
  ASSIGNOBJ(30100, gen_board);   /* Genocide Board */
  ASSIGNOBJ(30201, gen_board);   /* Shadow Alliance */
  ASSIGNOBJ(1271, gen_board);    // Midgaard Board
  ASSIGNOBJ(30400, gen_board);   /* Pahlawan Board */
  ASSIGNOBJ(30300, gen_board);   /* ::seVen:: Board */
  ASSIGNOBJ(30500, gen_board);   /* HED Board */
  ASSIGNOBJ(30600, gen_board);   /* Lost Souls Board */
  ASSIGNOBJ(30700, gen_board);   /* Black Hands Board */

  ASSIGNOBJ(3034, bank);	/* atm */
  ASSIGNOBJ(3036, bank);	/* cashcard */
  ASSIGNOBJ(1058, bank);	/* deposit slips */
  ASSIGNOBJ(1059, bank);	/* withdrawl slips */

  ASSIGNOBJ(6612, bank);
  ASSIGNOBJ(6647, marbles);

  ASSIGNOBJ(7240, bell);

  ASSIGNOBJ(1204, docket);

  ASSIGNOBJ(98,   scaredy_cat);


}



/* assign special procedures to rooms */
void assign_rooms(void)
{
  extern int dts_are_dumps;
  int i;

  SPECIAL(dump);
  SPECIAL(pet_shops);
  SPECIAL(pray_for_items);
  SPECIAL(kaan_altar);
  SPECIAL(bank);
  SPECIAL(hospital);
  SPECIAL(pool);
  SPECIAL(thieven_maze);
  SPECIAL(chute);
  SPECIAL(serf);
  SPECIAL(horse_trader);
  SPECIAL(giant_eats);
  SPECIAL(giant_farts);
  SPECIAL(poshuls_tower);
  SPECIAL(seven_wards);

  ASSIGNROOM(30508, bank); /* Clan DarkLords */
  ASSIGNROOM(30214, bank); /* Clan Eternals */


  /* Spec Quests */
  ASSIGNROOM(162, giant_eats);
  ASSIGNROOM(173, giant_farts);

  ASSIGNROOM(6723, pet_shops);
  ASSIGNROOM(6719, bank);

  ASSIGNROOM(1022, pet_shops);
  ASSIGNROOM(1084, hospital);
  ASSIGNROOM(1011, bank);
  ASSIGNROOM(3090, hospital);
  ASSIGNROOM(3030, dump);
  ASSIGNROOM(1, dump);
  ASSIGNROOM(3031, pet_shops);
  ASSIGNROOM(3096, seven_wards);
  ASSIGNROOM(17621, bank);
  ASSIGNROOM(17647, dump);
  ASSIGNROOM(4445, pet_shops);
  ASSIGNROOM(5478, kaan_altar);
  ASSIGNROOM(3099, bank);
  ASSIGNROOM(30413, bank);
  ASSIGNROOM(6421, pool);
  ASSIGNROOM(6762, hospital);
  ASSIGNROOM(5773, hospital);
  ASSIGNROOM(5766, bank);
  ASSIGNROOM(5450, thieven_maze);
  ASSIGNROOM(5427, thieven_maze);
  ASSIGNROOM(5435, thieven_maze);
  ASSIGNROOM(5441, thieven_maze);
  ASSIGNROOM(5472, thieven_maze);
  ASSIGNROOM(5465, thieven_maze);
  ASSIGNROOM(5459, thieven_maze);
  ASSIGNROOM(5453, thieven_maze);
  ASSIGNROOM(8025, bank);
  ASSIGNROOM(8022, hospital);
  ASSIGNROOM(4407, hospital);
  ASSIGNROOM(4305, bank);
  ASSIGNROOM(4307, hospital);
  ASSIGNROOM(19046, bank);
  ASSIGNROOM(19078, hospital);
  ASSIGNROOM(18083, chute);
  ASSIGNROOM(5847, bank);
  ASSIGNROOM(5848, hospital);
  ASSIGNROOM(14917, hospital);
  ASSIGNROOM(20280, hospital);
  ASSIGNROOM(50047, poshuls_tower);

  ASSIGNROOM(8437, serf);
  ASSIGNROOM(8439, serf);
  ASSIGNROOM(8443, serf);

  ASSIGNROOM(1065, horse_trader);
  ASSIGNROOM(3586, horse_trader);
  ASSIGNROOM(9327, horse_trader);
  ASSIGNROOM(20281, horse_trader);

  if (dts_are_dumps)
    for (i = 0; i < top_of_world; i++)
      if (IS_SET_AR(ROOM_FLAGS(i), ROOM_DEATH))
	world[i].func = dump;
}
