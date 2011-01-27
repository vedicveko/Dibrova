/*************************************************************************
*   File: structs.h                                     Part of CircleMUD *
*  Usage: header file for central structures and contstants               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database	*/
#define NOTHING	   -1    /* nil reference for objects		*/
#define NOBODY	   -1    /* nil reference for mobiles		*/

#define SPECIAL(name) \
   int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/* misc editor defines **************************************************/

/* format modes for format_text */
#define FORMAT_INDENT		(1 << 0)


/* room-related defines *************************************************/


/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NORTHEAST      6
#define NORTHWEST      7
#define SOUTHEAST      8



#define SOUTHWEST      9


/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK				0   /* Dark							*/
#define ROOM_DEATH				1   /* Death trap					*/
#define ROOM_NOMOB				2   /* MOBs not allowed				*/
#define ROOM_INDOORS			3   /* Indoors						*/
#define ROOM_PEACEFUL			4   /* Violence not allowed			*/
#define ROOM_SOUNDPROOF			5   /* Shouts, gossip blocked		*/
#define ROOM_NOTRACK			6   /* Track won't go through		*/
#define ROOM_NOMAGIC			7   /* Magic not allowed			*/
#define ROOM_TUNNEL				8   /* room for only 2 persons		*/
#define ROOM_PRIVATE			9   /* Can't teleport in			*/
#define ROOM_GODROOM			10  /* > LVL_ADMIN only allowed		*/
#define ROOM_HOUSE				11  /* (R) Room is a house			*/
#define ROOM_HOUSE_CRASH		12  /* (R) House needs saving		*/
#define ROOM_ATRIUM				13  /* (R) The door to a house		*/
#define ROOM_OLC				14  /* (R) Modifyable/!compress		*/
#define ROOM_BFS_MARK			15  /* (R) breath-first srch mrk	*/
#define ROOM_NOPORTAL			16  /* Can't create portal to		*/
#define ROOM_ARENA				17  /* Part of an Arena				*/
#define ROOM_HEALING            18  /* Healing Room					*/
#define ROOM_IMPEN              19  /* Impenetrable darkness		*/
#define ROOM_GRID               20  /* Level Grid					*/
#define ROOM_SINGLE_SPACE       21  /* room for only 1 person		*/
#define ROOM_VAULT              22 
#define ROOM_HAVEN              23  /* Vampire safe rooms			*/
#define ROOM_SCORCHED           24  /* Scorched Earth from Titans	*/
#define ROOM_COURT              25  /* Player trial room flag		*/
#define ROOM_NOPLINK            26
#define ROOM_SALTWATER_FISH     27  /* Player can fish here			*/
#define ROOM_FRESHWATER_FISH    28  /* Player can fish here too		*/

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR	(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED	(1 << 1)   /* The door is closed	*/
#define EX_LOCKED	(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF	(1 << 3)   /* Lock can't be picked	*/
#define EX_HIDDEN	(1 << 4)   /* Hidden exit               */
#define EX_STEEP        (1 << 5)   /* Steep or sheer            */


/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater		*/
#define SECT_FLYING	         9		   /* Wheee!			*/
#define SECT_ROAD	         10		   /* On a road.		*/
#define SECT_SAND            11            /* Walking in sand           */ 
#define SECT_DESERT          12            /* Walking in desert         */ 
#define SECT_SNOW            13            /* Walking in snow           */ 
#define SECT_SWAMP           14            /* Walking in swamp          */ 
#define SECT_OCEAN           15            /* Ocean (surface)           */ 
#define SECT_NOTRAVEL        16            // Morts can only enter aboard ships
#define SECT_CAVE			 17            // Morts can only enter aboard ships


/* Stat positions for scanning stat tables in races.c */
#define STAT_INT  0
#define STAT_WIS  1
#define STAT_DEX  2
#define STAT_STR  3
#define STAT_CON  4
#define STAT_CHA  5
#define STAT_LUCK 6
#define NUM_STATS 7

/* char and mob-related defines *****************************************/


/* PC classes */
#define CLASS_UNDEFINED	     -1
#define CLASS_SORCERER        0
#define CLASS_CLERIC          1
#define CLASS_THIEF           2
#define CLASS_GLADIATOR       3
#define CLASS_DRUID           4
#define CLASS_DARK_KNIGHT     5
#define CLASS_MONK            6
#define CLASS_PALADIN         7
#define CLASS_BARD            8
#define CLASS_ASSASSIN        9
#define CLASS_SHADOWMAGE     10
#define CLASS_INQUISITOR     11
#define CLASS_ACROBAT        12
#define CLASS_RANGER         13
#define CLASS_WARLOCK        14
#define CLASS_CHAMPION       15
#define CLASS_REAPER         16
#define CLASS_COMBAT_MASTER  17
#define CLASS_CRUSADER       18
#define CLASS_BEASTMASTER    19
#define CLASS_ARCANIC        20
#define CLASS_MAGI           21
#define CLASS_DARKMAGE       22
#define CLASS_FORESTAL       23
#define CLASS_PSIONIST       24
#define CLASS_TEMPLAR        25
#define CLASS_STORR          26
#define CLASS_SAGE           27
#define CLASS_SAMURAI        28
#define CLASS_FIANNA         29
#define CLASS_SHAMAN         30
#define CLASS_MERCENARY      31
#define CLASS_ROGUE          32
#define CLASS_DESTROYER      33
#define CLASS_STYXIAN        34
#define CLASS_WHOKNOWS5      35
#define CLASS_WHOKNOWS6      36
#define CLASS_WHOKNOWS7      37
#define CLASS_VAMPIRE        38
#define CLASS_TITAN          39
#define CLASS_SAINT          40
#define CLASS_DEMON          41

#define NUM_CLASSES          42  /* This must be the number of classes!! */

/* PC races */
#define RACE_UNDEFINED    -1
#define RACE_DWARF         0
#define RACE_ELDAR         1
#define RACE_ELF           2
#define RACE_GIANT         3
#define RACE_HALFLING      4
#define RACE_HUMAN         5
#define RACE_KENDER        6
#define RACE_OGRE          7
#define RACE_TRITON        8
#define RACE_VALKYRIE      9
#define RACE_WOLFEN        10 
#define RACE_GOBLIN        11 
#define RACE_DRIDER        12 
#define RACE_TROLL         13 
#define RACE_PIXIE         14 
#define RACE_DROW          15 
#define RACE_GNOME         16 
#define RACE_NYMPH         17 
#define RACE_NAGA          18 
#define RACE_CENTAUR       19 
#define RACE_GARGOYLE      20 
#define RACE_HARPY         21 
#define RACE_DRACONIAN     22 
#define RACE_SCORPIUS      23 
#define RACE_SPRITE        24 
#define RACE_AVIADES       25 
#define NUM_RACES          26

/* NPC classes (currently used - I felt free to implement!) */
#define CLASS_NPC_OTHER       0
#define CLASS_NPC_MAGE        1   
#define CLASS_NPC_CLERIC      2 	 
#define CLASS_NPC_THIEF       3 	 
#define CLASS_NPC_WARRIOR     4	 
#define CLASS_NPC_DRUID       5
#define CLASS_NPC_PALADIN     6
#define CLASS_NPC_DOLL_MAGE   7
#define CLASS_NPC_MONK        8
#define CLASS_NPC_DARK_KNIGHT 9
#define CLASS_NPC_CITY_GUARD  10
#define CLASS_NPC_RECEPTION   11
#define CLASS_NPC_CITIZEN     12
#define CLASS_CLANGUARD       13
#define CLASS_NPC_VAMPIRE     14
#define NUM_NPC_CLASS 	      15 

#define RACE_NPC_OTHER        0
#define RACE_NPC_MAMAL       1
#define RACE_NPC_REPTILE     2
#define RACE_NPC_AVIAN       3
#define RACE_NPC_FISH        4
#define RACE_NPC_INSECT      5
#define RACE_NPC_UNDEAD      6
#define RACE_NPC_HUMANOID    7
#define RACE_NPC_ANIMAL      8
#define RACE_NPC_DRAGON      9
#define RACE_NPC_GIANT       10
#define RACE_NPC_SHADOW      11
#define RACE_NPC_AQUA_ELF    12
#define RACE_NPC_DWARF       13
#define RACE_NPC_HUMAN       14
#define RACE_NPC_ELF         15
#define RACE_NPC_DROW        16
#define RACE_NPC_HALFLING    17
#define RACE_NPC_KENDER      18
#define RACE_NPC_OGRE        19
#define RACE_NPC_ELDAR       20
#define RACE_NPC_WOLFEN      21
#define RACE_NPC_TRITON      22
#define RACE_NPC_VALKRYIE    23
#define RACE_NPC_SPRITE      24
#define RACE_NPC_TREANT      25
#define RACE_NPC_PLANT       26
#define RACE_NPC_HYDRA       27
#define RACE_NPC_BROKEN_ONE  28
#define RACE_NPC_DOOM_GUARD  29
#define RACE_NPC_ELEMENTAL   30
#define RACE_NPC_GOBLIN      31
#define RACE_NPC_ORC         32
#define RACE_NPC_GOLEM       33
#define RACE_NPC_SPIRIT      34
#define RACE_NPC_REAVER      35
#define RACE_NPC_SKELETON    36
#define RACE_NPC_ILLITHID    37
#define RACE_NPC_GORGON      38
#define RACE_NPC_WEREBEAST   39
#define RACE_NPC_TITON       40
#define RACE_NPC_DEMON       41
#define RACE_NPC_LIZARDMAN   42
#define RACE_NPC_SCORPION    43
#define RACE_NPC_LICH        44
#define RACE_NPC_CELESTIAL   45
#define RACE_NPC_SPIDER      46
#define RACE_NPC_NAGA        47
#define RACE_NPC_MUMMY       48
#define RACE_NPC_WRAITH      49 
#define RACE_NPC_MINOTAUR    50
#define RACE_NPC_MEPHIT      51
#define RACE_NPC_EQUINE      52
#define NUM_NPC_RACE         53 

/* Ranks */
#define RANK_NOTFREEMAN   0
#define RANK_FREEMAN   1  
#define RANK_CITIZEN   2
#define RANK_LORD      3
#define RANK_BARON     4
#define RANK_VISCOUNT  5
#define RANK_COUNT     6
#define RANK_EARL      7
#define RANK_MARQUEE   8
#define RANK_DUKE      9
#define RANK_PRINCE    10
#define RANK_KING      11
#define RANK_EMPEROR   12
#define MAX_RANK       13 

// Item Material defines....
#define MATERIAL_OTHER   0
#define MATERIAL_CLOTH   1
#define MATERIAL_LEATHER 2
#define MATERIAL_COPPER  3
#define MATERIAL_BRONZE  4
#define MATERIAL_IRON    5 
#define MATERIAL_STEEL   6 
#define MATERIAL_SILVER  7 
#define MATERIAL_MAGICAL 8
#define MAX_MATERIALS    9 

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_RESTING    5	/* resting		*/
#define POS_SITTING    6	/* sitting		*/
#define POS_FIGHTING   7	/* fighting		*/
#define POS_STANDING   8	/* standing		*/

/*Hit Type*/
#define HIT_DUAL   0     /*Character is dual wielded*/

/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER	0   /* Player is a player-killer	*/
#define PLR_THIEF	1   /* Player is a player-thief		*/
#define PLR_FROZEN	2   /* Player is frozen			*/
#define PLR_DONTSET     3   /* Don't EVER set (ISNPC bit)	*/
#define PLR_WRITING	4   /* Player writing (board/mail/olc)	*/
#define PLR_MAILING	5  /* Player is writing mail		*/
#define PLR_CRASH	6   /* Player needs to be crash-saved	*/
#define PLR_SITEOK	7   /* Player has been site-cleared	*/
#define PLR_NOSHOUT	8   /* Player not allowed to shout/goss	*/
#define PLR_NOTITLE	9   /* Player not allowed to set title	*/
#define PLR_DELETED	10  /* Player deleted - space reusable	*/
#define PLR_LOADROOM	11  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	12  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	13  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	14  /* Player should enter game wizinvis*/
#define PLR_CRYO	15  /* Player is cryo-saved (purge prog)*/
#define PLR_MULTI_OK   16
#define PLR_DRUID       17
#define PLR_TOAD        18
#define PLR_EXILE       19  /* Player is a clan exile           */
#define PLR_BOUNTY      20  /* Player has a bounty on life      */
#define PLR_QUESTOR     21 
#define PLR_INSURED     22  /* Player has life insurance        */
#define PLR_KEEPRANK    23
#define PLR_PKILL_OK    24  /* Player can engage in pkill       */
#define PLR_SELLER      25  /* Player is auctioning             */
#define PLR_FISHING     26  /* Player has a line in the water   */
#define PLR_FISH_ON     27  /* Player has a fish on their line  */
#define PLR_OUTFITTED   28
#define PLR_GLAD_BET_QP 29  /* Player is fighting in pit for qps */
#define PLR_GLAD_BET_GOLD 30 /* Player is fighting in pit for gold */

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         0  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     1  /* Mob should not move		*/
#define MOB_SCAVENGER    2  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        3  /* (R) Automatically set on all Mobs*/
#define MOB_AWARE	 4  /* Mob can't be backstabbed		*/
#define MOB_AGGRESSIVE   5  /* Mob hits players in the room	*/
#define MOB_STAY_ZONE    6  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        7  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 8  /* auto attack evil PC's		*/
#define MOB_AGGR_GOOD	 9  /* auto attack good PC's		*/
#define MOB_AGGR_NEUTRAL 10 /* auto attack neutral PC's		*/
#define MOB_MEMORY	 11 /* remember attackers if attacked	*/
#define MOB_HELPER	 12 /* attack PCs fighting other NPCs	*/
#define MOB_NOCHARM	 13 /* Mob can't be charmed		*/
#define MOB_NOSUMMON	 14 /* Mob can't be summoned		*/
#define MOB_NOSLEEP	 15 /* Mob can't be slept		*/
#define MOB_NOBASH	 16 /* Mob can't be bashed (e.g. trees)	*/
#define MOB_NOBLIND	 17 /* Mob can't be blinded		*/
#define MOB_MOUNT        18 /* Mob can be mounted               */
#define MOB_HUNTER       19 /* Mob will hunt attackers!         */ 
#define MOB_USE_DOORS    20 /* Mob will use doors               */ 
#define MOB_QUESTMASTER  21
#define MOB_QUEST        22 
#define MOB_PC_ASSIST    23 /* Mob helps players in fight       */
#define MOB_VENOMOUS     24 /* Player gets bit and poisoned     */
#define MOB_NOPLINK      25
#define MOB_INSTAGGRO    26

/* Preference flags: used by char_data.player_specials.pref     */
#define PRF_BRIEF       0  /* Room descs won't normally be shown*/
#define PRF_COMPACT     1  /* No extra CRLF pair before prompts	*/
#define PRF_DEAF	2  /* Can't hear shouts			*/
#define PRF_NOTELL	3  /* Can't receive tells		*/
#define PRF_DISPHP	4  /* Display hit points in prompt	*/
#define PRF_DISPMANA	5  /* Display mana points in prompt	*/
#define PRF_DISPMOVE	6  /* Display move points in prompt	*/
#define PRF_AUTOEXIT	7  /* Display exits in a room		*/
#define PRF_NOHASSLE	8  /* Aggr mobs won't attack		*/
#define PRF_QUEST	9  /* On quest				*/
#define PRF_SUMMONABLE	10 /* Can be summoned			*/
#define PRF_NOREPEAT	11 /* No repetition of comm commands	*/
#define PRF_HOLYLIGHT	12 /* Can see in dark			*/
#define PRF_COLOR_1	13 /* Color (low bit)			*/
#define PRF_COLOR_2	14 /* Color (high bit)			*/
#define PRF_NOWIZ	15 /* Can't hear wizline		*/
#define PRF_LOG1	16 /* On-line System Log (low bit)	*/
#define PRF_LOG2	17 /* On-line System Log (high bit)	*/
#define PRF_NOAUCT	18 /* Can't hear auction channel	*/
#define PRF_NOGOSS	19 /* Can't hear gossip channel		*/
#define PRF_NOGRATZ	20 /* Can't hear grats channel		*/
#define PRF_ROOMFLAGS	21 /* Can see room flags (ROOM_x)	*/
#define PRF_AUTOASSIST	22 /* Autoassist your group             */ 
#define PRF_AUTOSPLIT	23 /* Autosplit gold                    */ 
#define PRF_DISPGOLD    24 /* Show amt of carried gold in prompt*/
#define PRF_DISPMOBP    25 /* hp % of the guy you're fighting   */
#define PRF_DISPPCT     26 /* Your hp %                         */
#define PRF_DISPXTOL    27 /* Exp you need for your next level  */
#define PRF_SHOWTIPS    28 /* Newbie Tips                       */
#define PRF_AFK         29 /* Away from keyboard                */
#define PRF_ARENA	30 /* Can see arena messages		*/
#define PRF_AUTOLOOT    31 /* Automatically loot corpses        */
#define PRF_NOINFO      32 /* Subscribed to the INFO channel    */
#define PRF_AUTODIAG    33 /* Will see the mob's gen cond in fights */
#define PRF_AUTOSAC     34 /* Auto-sac corpses                  */
#define PRF_DISPTANK    35 /* Show Tank HP % in prompt          */
#define PRF_DISPSHORT   36 /* Set wrapping in prompt            */
#define PRF_DISPQI      37 /* Show Qi in prompt                 */
#define PRF_DISPVIM     38 /* Show vim in prompt                */
#define PRF_DISPARIA    39 /* Show Aria in prompt               */
#define PRF_AUTOTITLE   40 /* Player keeps title on leveling    */
#define PRF_MULTIPLAY   41 /* Multiplaying?                     */
#define PRF_AUTOMAP     42 /* Auto display map on movement?     */
#define PRF_IHIDE       43
#define PRF_NORANK      44 /* No rank shown on who list         */
#define PRF_NOGRID      45 /* Player will not go to Grid at lvl */
#define PRF_AUTOGOLD    46 /* Automatically get gold from corpses */
#define PRF_MOBFLAGS    47 /* See a mob's vnum after their decsrip */
#define PRF_NOFOLLOW    48

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_DONTUSE           0   /* DON'T USE                  */
#define AFF_BLIND             1   /* (R) Char is blind		*/
#define AFF_INVISIBLE         2   /* Char is invisible		*/
#define AFF_DETECT_ALIGN      3   /* Char is sensitive to align */
#define AFF_DETECT_INVIS      4   /* Char can see invis chars   */
#define AFF_DETECT_MAGIC      5   /* Char is sensitive to magic */
#define AFF_SENSE_LIFE        6   /* Char can sense hidden life */
#define AFF_WATERWALK	      7   /* Char can walk on water	*/
#define AFF_SANCTUARY         8   /* Char protected by sanct.	*/
#define AFF_GROUP             9   /* (R) Char is grouped	*/
#define AFF_CURSE             10   /* Char is cursed		*/
#define AFF_INFRAVISION       11  /* Char can see in dark	*/
#define AFF_POISON            12  /* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL      13  /* Char protected from evil   */
#define AFF_PROTFROMGOOD      14  /* Char protected from good   */
#define AFF_SLEEP             15  /* (R) Char magically asleep	*/
#define AFF_NOTRACK	      16  /* Char can't be tracked	*/
#define AFF_SILENCE	      17  /* Char is Silenced           */
#define AFF_FIRESHIELD	      18  /* Room for future expansion	*/
#define AFF_SNEAK             19  /* Char can move quietly	*/
#define AFF_HIDE              20  /* Char is hidden		*/
#define AFF_PARALYZE	      21  /* Room for future expansion	*/
#define AFF_CHARM             22  /* Char is charmed		*/
#define AFF_FLY		      23  /* Char is flying...          */
#define AFF_BREATHE	      24  /* Breathe underwater         */
#define AFF_HASTE	      25  /* Char is fast, Storm 7/24/98*/
#define AFF_CHAOSARMOR        26
#define AFF_HOLYARMOR         27
#define AFF_AEGIS             28
#define AFF_REGENERATE        29
#define AFF_BLACKDART         30
#define AFF_DONTUSE2          31
#define AFF_DARKWARD          32
#define AFF_SEE_HIDDEN        33  /* Can see hidden exits       */
#define AFF_SLOW	      34  /* Slow                                */
#define AFF_INCOGNITO         35  /* Incognito                           */
#define AFF_BLACKBREATH       36
#define AFF_PLAGUE            37
#define AFF_RUN               38
#define AFF_DEAF              39  /* Player is deaf!                     */
#define AFF_ANCIENT_PROT      40  /* Player is protected by the ancients */
#define AFF_SACRED_SHIELD     41  /* Protected by a shied of energy      */
#define AFF_MENTAL_BARRIER    42  /* A mental barrier that absorbs energy*/
#define AFF_WARCRY            43  /* Hitroll bonus                       */
#define AFF_SHADOW            44  /* Move in the shadows                 */
#define AFF_AWE               45  /* In awe                              */
#define AFF_MANA_ECONOMY      46  /* Conserving Mana                     */
#define AFF_NOBLIND           47  /* Can't be blinded with skills or spells */
#define AFF_WEAKENEDFLESH     48  /* Takes 1.25x damage in battle        */
#define AFF_BALANCE           49  /* !BASH, !TRIP, !SWEEP                */
#define AFF_COLD              50  /* Frozen                              */
#define AFF_DROWSE            51  /* Drowse prayer                       */
#define AFF_ALIGN_GOOD        52  /* Eq aff to change align              */
#define AFF_ALIGN_EVIL        53  /* Eq aff to change align              */
#define AFF_NOMAGIC           54  /* Multi-use flag for eq only          */
#define AFF_INSOMNIA          55  /* !SLEEP                              */
#define AFF_BLAZEWARD         56  /* Quest Potion                        */
#define AFF_INDESTR_AURA      57  /* Quest Potion                        */
#define AFF_BERSERK           58  
#define AFF_MIST_FORM         59  /* Pass under doors when closed        */
#define AFF_SOFTEN_FLESH      60  /* victim takes double damage - Crysist */
#define AFF_CONFUSE	      61
#define AFF_QUEST_TIME        62  /* Time left in quest or until quest   */
#define AFF_AWARE             63  /* PC flag matching NPC flag           */
#define AFF_IMMBLESSED        64  /* Player has been blessed by the gods */
#define AFF_SEALED            65  /* Cant leave the room                 */
#define AFF_VAMP_AURA         66  /* Vampiric Aura (vamps only)          */
#define AFF_HOVER             67  /* Vampire fly */
#define AFF_HISS              68  /* Vampire skill */
#define AFF_MESMERIZED        69  /* Vampire holding skill */
#define AFF_SHROUD            70  /* Vamp protection from sunlight       */
#define AFF_BURROW            71  /* Vamp healing flag      */
#define AFF_DEATHCALL         72  /* Vamp dmg affect        */
#define AFF_EARTH_SHROUD      73  /* Titan perma affect     */
#define AFF_ABSORB            74  /* Titan heal affect      */
#define AFF_CLOUD             75  /* Titan locator skill    */
#define AFF_UNDER_ATTACK      76  /* Titan attack locator   */
#define AFF_SPEAK	      77  /* Court affect for allowing speech */
#define AFF_AFTERLIFE         78  /* Saint affection to keep eq */
#define AFF_DIVINE_SHIELD     79  /* Saint affect (no aggro) */
#define AFF_IMPEDE            80  /* Saint affect block flee */
#define AFF_ETHEREAL          81  /* Saints pass door       */
#define AFF_ENLIGHT           82  /* Saints bonus damage    */
#define AFF_GAUGE             83  /* +dr thief skill        */
#define AFF_NETHERBLAZE       84  /* Demon Aura             */
#define AFF_NETHERCLAW        85  /* Demon weapon dam boost */
#define AFF_DECEIVE           86  /* Demon disguising skill */
#define AFF_UNFURLED          87  /* Demon wing affect      */
#define AFF_ARMOR             88
#define AFF_HEAVY_SKIN        89
#define AFF_PHANTOM_ARMOR     90
#define AFF_DIVINE_NIMBUS     91
#define AFF_STR               92  /* Used for all three str bufs */
#define AFF_DEFLECT           93
#define AFF_DEX               94
#define AFF_INT               95
#define AFF_WIS               96
#define AFF_CON               97
#define AFF_CHA               98
#define AFF_LUCK              99
#define AFF_FOG_MIST_AURA	  100
#define AFF_SNARE			  101

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING	 0	/* Playing - Nominal state	*/
#define CON_CLOSE	 1	/* Disconnecting		*/
#define CON_GET_NAME	 2	/* By what name ..?		*/
#define CON_NAME_CNFRM	 3	/* Did I get that right, x?	*/
#define CON_PASSWORD	 4	/* Password:			*/
#define CON_NEWPASSWD	 5	/* Give me a password for x	*/
#define CON_CNFPASSWD	 6	/* Please retype password:	*/
#define CON_QSEX	 7	/* Sex?				*/
#define CON_QCLASS	 8	/* Class?			*/
#define CON_RMOTD	 9	/* PRESS RETURN after MOTD	*/
#define CON_MENU	 10	/* Your choice: (main menu)	*/
#define CON_EXDESC	 11	/* Enter a new description:	*/
#define CON_CHPWD_GETOLD 12	/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW 13	/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY   14	/* Verify new password		*/
#define CON_DELCNF1	 15	/* Delete confirmation 1	*/
#define CON_DELCNF2	 16	/* Delete confirmation 2	*/
#define CON_OEDIT	 17	/* OLC mode - object edit       */
#define CON_REDIT	 18	/* OLC mode - room edit         */
#define CON_ZEDIT	 19	/* OLC mode - zone info edit    */
#define CON_MEDIT	 20	/* OLC mode - mobile edit       */
#define CON_SEDIT	 21	/* OLC mode - shop edit         */
#define CON_QCOLOR	 22	/* Want ANSI Color enabled?     */
#define CON_ROLL_STATS   23	/* Char is rolling stats        */
#define CON_QHOME        24     /* Char is selecting his/her hometown */
#define CON_QRACE        25     /* Select Race                  */
#define CON_TRIGEDIT     26		/*. OLC mode - trigger edit    .*/
#define CON_QEDIT	 27

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAIST     13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WEAR_WIELD     16
#define WEAR_HOLD      17
#define WEAR_EAR_R     18
#define WEAR_EAR_L     19
#define WEAR_THUMB_R   20
#define WEAR_THUMB_L   21
#define WEAR_EYES      22
#define WEAR_FACE      23
#define WEAR_FLOAT_1   24
#define WEAR_FLOAT_2   25
#define WEAR_ANKLE     26
#define WEAR_BACK      27
#define NUM_WEARS      28	/* This must be the # of eq positions!! */


/* object-related defines ********************************************/


/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1		/* Item is a light source	*/
#define ITEM_SCROLL     2		/* Item is a scroll		*/
#define ITEM_WAND       3		/* Item is a wand		*/
#define ITEM_STAFF      4		/* Item is a staff		*/
#define ITEM_WEAPON     5		/* Item is a weapon		*/
#define ITEM_FIREWEAPON 6		/* Unimplemented		*/
#define ITEM_MISSILE    7		/* Unimplemented		*/
#define ITEM_TREASURE   8		/* Item is a treasure, not gold	*/
#define ITEM_ARMOR      9		/* Item is armor		*/
#define ITEM_POTION    10 		/* Item is a potion		*/
#define ITEM_WORN      11		/* Unimplemented		*/
#define ITEM_OTHER     12		/* Misc object			*/
#define ITEM_TRASH     13		/* Trash - shopkeeps won't buy	*/
#define ITEM_TRAP      14		/* Unimplemented		*/
#define ITEM_CONTAINER 15		/* Item is a container		*/
#define ITEM_NOTE      16		/* Item is note 		*/
#define ITEM_DRINKCON  17		/* Item is a drink container	*/
#define ITEM_KEY       18		/* Item is a key		*/
#define ITEM_FOOD      19		/* Item is food			*/
#define ITEM_MONEY     20		/* Item is money (gold)		*/
#define ITEM_PEN       21		/* Item is a pen		*/
#define ITEM_BOAT      22		/* Item is a boat		*/
#define ITEM_FOUNTAIN  23		/* Item is a fountain		*/
#define ITEM_PORTAL    24		/* Item is a portal.		*/
#define ITEM_DOLL      25
#define ITEM_SMOKE     26               /* Item can be smoked!          */
#define ITEM_STONE     27               /* Sharpening Stone             */
#define ITEM_DEED      28              // ITem is a deed to a zone.
#define ITEM_RUNE      29              // Item is a recall rune       
#define ITEM_HERB      30               /* Herbs used with MIX cmd      */
#define ITEM_JAR       31               /* Used to mix herbs in water   */
#define ITEM_SHIP      32               // Ship objects
#define ITEM_TILLER    33               // Steers ship object
#define ITEM_POLE      34               /* Fishing Pole                 */
#define ITEM_PIECE     35               /* Item can be combined with other */


/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE		0  /* Item can be taken		*/
#define ITEM_WEAR_FINGER	1  /* Can be worn on finger	*/
#define ITEM_WEAR_NECK		2  /* Can be worn around neck 	*/
#define ITEM_WEAR_BODY		3  /* Can be worn on body 	*/
#define ITEM_WEAR_HEAD		4  /* Can be worn on head 	*/
#define ITEM_WEAR_LEGS		5  /* Can be worn on legs	*/
#define ITEM_WEAR_FEET		6  /* Can be worn on feet	*/
#define ITEM_WEAR_HANDS		7  /* Can be worn on hands	*/
#define ITEM_WEAR_ARMS		8  /* Can be worn on arms	*/
#define ITEM_WEAR_SHIELD	9  /* Can be used as a shield	*/
#define ITEM_WEAR_ABOUT		10 /* Can be worn about body 	*/
#define ITEM_WEAR_WAIST 	11 /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		12 /* Can be worn on wrist 	*/
#define ITEM_WEAR_WIELD		13 /* Can be wielded		*/
#define ITEM_WEAR_HOLD		14 /* Can be held		*/
#define ITEM_WEAR_EAR		15 /* Can be worn on ear         */
#define ITEM_WEAR_THUMB 	16 /* Can be worn on thumb       */
#define ITEM_WEAR_EYES		17 /* Can be worn on eyes        */
#define ITEM_WEAR_FACE		18 /* Can be worn on face       */
#define ITEM_WEAR_FLOAT_1       19
#define ITEM_WEAR_ANKLE         20
#define ITEM_WEAR_BACK          21


/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW          0	/* Item is glowing		*/
#define ITEM_HUM           1	/* Item is humming		*/
#define ITEM_NORENT        2	/* Item cannot be rented	*/
#define ITEM_NODONATE      3	/* Item cannot be donated	*/
#define ITEM_NOINVIS	   4	/* Item cannot be made invis	*/
#define ITEM_INVISIBLE     5	/* Item is invisible		*/
#define ITEM_MAGIC         6	/* Item is magical		*/
#define ITEM_NODROP        7	/* Item is cursed: can't drop	*/
#define ITEM_BLESS         8	/* Item is blessed		*/
#define ITEM_ANTI_GOOD     9	/* Not usable by good people	*/
#define ITEM_ANTI_EVIL     10	/* Not usable by evil people	*/
#define ITEM_ANTI_NEUTRAL  11	/* Not usable by neutral people	*/
#define ITEM_ANTI_SORCERER 12	/* Not usable by mages		*/
#define ITEM_ANTI_CLERIC   13	/* Not usable by clerics	*/
#define ITEM_ANTI_THIEF	   14	/* Not usable by thieves	*/
#define ITEM_ANTI_GLADIATOR 15	/* Not usable by warriors	*/
#define ITEM_NOSELL	   16	/* Shopkeepers won't touch it	*/
#define ITEM_ANTI_DRUID    17	/* Not usable by rangers	*/
#define ITEM_ANTI_DARK_KNIGHT 18/* Not usable by Dark Knights   */
#define ITEM_ANTI_MONK     19   /* Not usable by Monks          */
#define ITEM_ANTI_PALADIN  20   /* Not usable by Paladins       */
#define ITEM_QUEST         21   /* Vis only to quest            */
#define ITEM_ANTI_BARD     22   /* Not usable by Bard           */
#define ITEM_BURIED        23
#define ITEM_NOT_SEEN      24   /* Makes the L-descrip unseen   */
#define ITEM_NOTRANSFER    25   /* Item can not leave inv       */
#define ITEM_NOLOCATE      26   /* Cannot be found with magic   */
#define ITEM_VAMP_CAN_SEE  27   /* Only seen by vampires        */
#define ITEM_TITAN_CAN_SEE 28   /* Only seen by titans          */
#define ITEM_SAINT_CAN_SEE 29   /* Only seen by saints          */
#define ITEM_DEMON_CAN_SEE 30   /* Only seen by demons          */
#define ITEM_SILVER        31   
#define ITEM_SMALL_SIZE    32
#define ITEM_MEDIUM_SIZE   33
#define ITEM_LARGE_SIZE    34
#define ITEM_TINY_SIZE     35
#define ITEM_PC_CORPSE     36    /* Players Corpse               */
#define ITEM_NPC_CORPSE    37    /* Mobs Corpse                  */
#define ITEM_UNIQUE_SAVE   38    /* Ascii rent files/Corpse saving */
#define ITEM_SPECIAL       39    /* Allows for higher than max stats */
#define ITEM_NOSTEAL       40   /* Cant be stolen */



/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0	/* No effect			*/
#define APPLY_STR               1	/* Apply to strength		*/
#define APPLY_DEX               2	/* Apply to dexterity		*/
#define APPLY_INT               3	/* Apply to constitution	*/
#define APPLY_WIS               4	/* Apply to wisdom		*/
#define APPLY_CON               5	/* Apply to constitution	*/
#define APPLY_CHA		6	/* Apply to charisma		*/
#define APPLY_CLASS             7	/* Reserved			*/
#define APPLY_LEVEL             8	/* Reserved			*/
#define APPLY_AGE               9	/* Apply to age			*/
#define APPLY_CHAR_WEIGHT      10	/* Apply to weight		*/
#define APPLY_CHAR_HEIGHT      11	/* Apply to height		*/
#define APPLY_MANA             12	/* Apply to max mana		*/
#define APPLY_HIT              13	/* Apply to max hit points	*/
#define APPLY_MOVE             14	/* Apply to max move points	*/
#define APPLY_GOLD             15	/* Reserved			*/
#define APPLY_EXP              16	/* Reserved			*/
#define APPLY_AC               17	/* Apply to Armor Class		*/
#define APPLY_HITROLL          18	/* Apply to hitroll		*/
#define APPLY_DAMROLL          19	/* Apply to damage roll		*/
#define APPLY_SAVING_PARA      20	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       21	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     22	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    23	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     24	/* Apply to save throw: spells	*/
#define APPLY_LUCK             25	/* Apply to luck                */
#define APPLY_SPELLPOWER       26	/* Apply to spellpower          */
#define APPLY_QI               27       /* Apply to Chi                 */
#define APPLY_VIM              28       /* Apply to Vim                 */
#define APPLY_ARIA             29       /* Apply to Aria                */


/* Container flags - value[1] */
#define CONT_CLOSEABLE  (1 << 0)          /* Container can be closed	*/
#define CONT_PICKPROOF  (1 << 1)          /* Container is pickproof	*/
#define CONT_CLOSED     (1 << 2)          /* Container is closed	*/
#define CONT_LOCKED     (1 << 3)          /* Container is locked	*/


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15
#define LIQ_HEALTH     16
#define LIQ_MANA       17
#define LIQ_QI         18
#define LIQ_MOVE       19
#define LIQ_VIM        20
#define LIQ_IMMORTALITY 21
#define LIQ_GOODALIGN  22



/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2
#define UNUSED1      3
#define UNUSED2      4


/* Sun state for weather_data */
#define SUN_DARK	0
#define SUN_RISE	1
#define SUN_LIGHT	2
#define SUN_SET		3


/* Sky conditions for weather_data */
#define SKY_CLOUDLESS	0
#define SKY_CLOUDY	1
#define SKY_RAINING	2
#define SKY_LIGHTNING	3
#define SKY_SNOWING     4

/* Array stuff */
#define RF_ARRAY_MAX    4
#define PM_ARRAY_MAX    4
#define PR_ARRAY_MAX    4
#define AF_ARRAY_MAX    4
#define TW_ARRAY_MAX    4
#define EF_ARRAY_MAX    4

/* Rent codes */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


/* other #defined constants **********************************************/

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 *
 * LVL_IMPL should always be the HIGHEST possible immortal level, and
 * LVL_IMMORT should always be the LOWEST immortal level.  The number of
 * mortal levels will always be LVL_IMMORT - 1.
 */
#define LVL_IMPL       160
#define LVL_UNUSED_2   159
#define LVL_UNUSED_1   158
#define LVL_CODER_H    157
#define LVL_CODER      156
#define LVL_BUILDER_H  155
#define LVL_BUILDER    154
#define LVL_QUESTOR    153
#define LVL_ADMIN      152
#define LVL_IMMORT     151

/* Level of the 'freeze' command */
#define LVL_FREEZE	LVL_UNUSED_2
#define LVL_TOAD        LVL_UNUSED_2

/* Storm -- 9/14/98 Gods should not be a specific god at a specific level. */
#define GOD_ALL		(1 << 0)
#define GOD_IMPL	(1 << 1)
#define GOD_CODER	(1 << 2)
#define GOD_BUILDER	(1 << 3)
#define GOD_ADMIN	(1 << 4)
#define GOD_QUESTOR	(1 << 5)

#define NUM_OF_DIRS	10	/* number of directions in a room (nsewud) */

#define OPT_USEC	100000	/* 10 passes per second */
#define PASSES_PER_SEC	(1000000 / OPT_USEC)
#define RL_SEC		* PASSES_PER_SEC

#define PULSE_ZONE      (10 RL_SEC)
#define PULSE_MOBILE    (10 RL_SEC)
#define PULSE_VIOLENCE  (2 RL_SEC)
#define PULSE_AUCTION   (9 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       96          /* Max length of prompt        */
#define GARBAGE_SPACE		32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE		1024        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE	   (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define MAX_STRING_LENGTH	8192
#define MAX_INPUT_LENGTH	256	/* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH	512	/* Max size of *raw* input */
#define MAX_MESSAGES		200
#define MAX_NAME_LENGTH		20  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_PWD_LENGTH		10  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH	80  /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH		30  /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH		240 /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE		3   /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT		32  /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT		6 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_ABILITIES           200 /* Max number of stored abilities      */
#define MAX_ASSIGNED            3   /*for corpse retrieval*/
#define MAX_COMPLETED_QUESTS	1024
#define MAX_MOB_SKILLS			6 // do _*NOT*_ change this value!

#define MOB_SKILL_UNUSED	0
#define MOB_SKILL_ONE		1
#define MOB_SKILL_TWO		2
#define MOB_SKILL_THREE		3
#define MOB_SKILL_FOUR		4
#define MOB_SKILL_FIVE		5

/**********************************************************************
* Structures                                                          *
**********************************************************************/


typedef signed char		sbyte;
typedef unsigned char		ubyte;
typedef signed short int	sh_int;
typedef unsigned short int	ush_int;
typedef char			bool;

#ifndef CIRCLE_WINDOWS
typedef char			byte;
#endif

typedef int	room_vnum;	/* A room's vnum type */
typedef int	obj_vnum;	/* An object's vnum type */
typedef int	mob_vnum;	/* A mob's vnum type */

typedef int	room_rnum;	/* A room's real (internel) number type */
typedef int	obj_rnum;	/* An object's real (internal) num type */
typedef int	mob_rnum;	/* A mobile's real (internal) num type */


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
   char	*keyword;                 /* Keyword in look/examine          */
   char	*description;             /* What to see                      */
   struct extra_descr_data *next; /* Next in list                     */
};


/* object-related structures ******************************************/

#define NUM_OBJ_VAL_POSITIONS 4
/* object flags; used in obj_data */
struct obj_flag_data {
   int	value[NUM_OBJ_VAL_POSITIONS];	/* Values of the item (see list)    */
   byte type_flag;	/* Type of item			    */
   int  wear_flags[TW_ARRAY_MAX];/* Where you can wear it	    */
   int 	extra_flags[EF_ARRAY_MAX];/* If it hums, glows, etc.	    */
   int	weight;         /* Weight what else                 */
   int	cost;           /* Value when sold (gp.)            */
   int	cost_per_day;   /* Cost to keep pr. real day        */
   int	timer;          /* Timer for object                 */
   int  clan;           /* Clan setting n object            */
   int  player;         /* Owned eq                         */
   int  vamp_wear;      /* vamp eq                          */
   int  titan_wear;     /* titan eq                         */
   int  saint_wear;     /* saint eq                         */
   int  demon_wear;     /* demon eq                         */
   int  obj_bid_num;    /* For use in auction system        */
   int  bid_amnt;       /* For use in auction system        */
   int  auc_timer;      /* Timer left in aution system      */
   long auc_player;     /* Which player will the obj go to? */
   long auc_seller;     /* Which player is selling the obj? */
   int	exp;            /* For SAC - (CORPSES ONLY)         */
   int	pid;            /* Player ID - (PLAYER CORPSES ONLY)         */
   int  minlevel;       /* Level restrict                   */
   int  spell;          /* Persistent spell number          */
   int  spell_extra;    /* Persistent spell modifier        */
   int	special;        /* Unsaved - For hone, etc.         */
   int 	bitvector[AF_ARRAY_MAX];/* To set chars bits        */
   byte material;
   int  racial_bonus;
};


/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
   byte location;      /* Which ability to change (APPLY_XXX) */
   sbyte modifier;     /* How much it changes by              */
};


/* ================== Memory Structure for Objects ================== */
struct obj_data {
   obj_vnum item_number;	/* Where in database			*/
   room_rnum in_room;		/* In what room -1 when conta/carr	*/

   struct obj_flag_data obj_flags;/* Object information               */
   struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* affects */

   char	*name;                    /* Title of object :get etc.        */
   char	*description;		  /* When in room                     */
   char	*short_description;       /* when worn/carry/in cont.         */
   char	*action_description;      /* What to write when used          */
   struct extra_descr_data *ex_description; /* extra descriptions     */
   struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
   struct char_data *worn_by;	  /* Worn by?			      */
   sh_int worn_on;		  /* Worn where?		      */

   struct obj_data *in_obj;       /* In what object NULL when none    */
   struct obj_data *contains;     /* Contains objects                 */

   long id;                       /* used by DG triggers              */
   bool timer_onoff;              // Kill timer when a baseball, letter or imm eq is picked up
   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;    /* script info for the object       */

   struct obj_data *next_content; /* For 'contains' lists             */
   struct obj_data *next;         /* For the object list              */
   struct char_data *bidder;    /* current bidder for item on the block */ 
   struct char_data *pkiller;   /* points from pc corpse to pc killer */ 
   int vroom;
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files		   */
struct obj_file_elem {
   obj_vnum item_number;

   sh_int locate;
   int	value[NUM_OBJ_VAL_POSITIONS];
   int	extra_flags[EF_ARRAY_MAX];
   int	weight;
   int	timer;
   int	bitvector[AF_ARRAY_MAX];
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct rent_info {
   int	time;
   int	rentcode;
   int	net_cost_per_diem;
   int	gold;
   int	account;
   int	nitems;
   int	spare0;
   int	spare1;
   int	spare2;
   int	spare3;
   int	spare4;
   int	spare5;
   int	spare6;
   int	spare7;
};
/* ======================================================================= */


/* room-related structures ************************************************/


struct room_direction_data {
   char	*general_description;       /* When look DIR.			*/

   char	*keyword;		/* for open/close			*/

   sh_int exit_info;		/* Exit info				*/
   obj_vnum key;		/* Key's number (-1 for no key)		*/
   room_rnum to_room;		/* Where direction leads (NOWHERE)	*/
};


/* ================== Memory Structure for room ======================= */
struct room_data {
   room_vnum number;		/* Rooms number	(vnum)		      */
   sh_int zone;                 /* Room zone (for resetting)          */
   int	sector_type;            /* sector type (move/hide)            */
   char	*name;                  /* Rooms name 'You are ...'           */
   char	*description;           /* Shown when entered                 */
   struct extra_descr_data *ex_description; /* for examine/look       */
   struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
   int room_flags[RF_ARRAY_MAX];/* DEATH,DARK ... etc                 */

   byte light;                  /* Number of lightsources in room     */
   SPECIAL(*func);

   struct trig_proto_list *proto_script; /* list of default triggers  */
   struct script_data *script;  /* script info for the object         */

   struct obj_data *contents;   /* List of items in room              */
   struct char_data *people;    /* List of NPC / PC in room           */

   byte blood;
};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
   long	id;
   struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
   int hours, day, month;
   sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
   time_t birth;    /* This represents the characters age                */
   time_t logon;    /* Time of the last logon (used to calculate played) */
   int	played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
   char	passwd[MAX_PWD_LENGTH+1]; /* character's password      */
   char	*name;	       /* PC / NPC s name (kill ...  )         */
   char	*short_descr;  /* for NPC 'actions'                    */
   char	*long_descr;   /* for 'look'			       */
   char	*description;  /* Extra descriptions                   */
   char	*title;        /* PC / NPC's title                     */
   byte sex;           /* PC / NPC's sex                       */
   byte class;         /* PC / NPC's class		       */
   byte race;          /* PC / NPC's race 		       */
   ubyte level;        /* PC / NPC's level                     */
   int	hometown;      /* PC Hometown (zone)                   */
   struct time_data time;  /* PC's AGE in days                 */
   int weight;         /* PC / NPC's weight - Pounds           */
   ubyte height;       /* PC / NPC's height - Inches           */
};


/* Char's stats.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_stat_data {
   sbyte str;
   sbyte str_add;      /* 000 - 100 if strength 18             */
   sbyte intel;
   sbyte wis;
   sbyte dex;
   sbyte con;
   sbyte cha;
   sbyte luck;
   sbyte unused1;
   sbyte unused2;
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
   int mana;
   int max_mana;     /* Max move for PC/NPC		        */
   int hit;
   int max_hit;      /* Max hit for PC/NPC                      */
   int move;
   int max_move;     /* Max move for PC/NPC                     */
   int qi;
   int max_qi;       /* Max Qi for PC/NPC                       */
   int vim;
   int max_vim;      /* Max Vim for PC/NPC                      */
   int aria;
   int max_aria;     /* Max Aria for PC/NPC                     */

   sh_int armor;        /* Internal -100..100, external -10..10 AC */
   int	gold;           /* Money carried                           */
   int	bank_gold;	/* Gold the char has in a bank account	   */
   uint	exp;            /* The experience of the player            */

   int   hitroll;       /* Any bonus or penalty to the hit roll    */
   int   damroll;       /* Any bonus or penalty to the damage roll */
   int   spellpower;    /* Any bonus or penalty to the spell power */
};

/* Structures for the Dibrova player abilities system */
struct char_abil_type {
  byte   type;
  uint   num;
  byte   pct;
};
/* Structures for the Dibrova mob abilities system    */
struct mob_abil_type {
  byte   type;
  uint   num;
  byte   pct;
};

/* 
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
   int	alignment;		/* +-1000 for alignments                */
   long	idnum;			/* player's idnum; -1 for mobiles	*/
   int 	act[PM_ARRAY_MAX];/* act flag for NPC's; player flag for PC's */
   int	affected_by[AF_ARRAY_MAX];/* Bitvector for spells/skills affected by */
   sh_int apply_saving_throw[5]; /* Saving throw (Bonuses)		*/
};


/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
   struct char_data *fighting;	/* Opponent				*/
   struct char_data *hunting;	/* Char hunted by this char		*/
   struct char_data *killvictim; /* NPC who killed PC                    */

   byte position;		/* Standing, fighting, sleeping, etc.	*/

   int	carry_weight;		/* Carried weight			*/
   byte carry_items;		/* Number of items carried		*/
   int	timer;			/* Timer for update			*/
   int safe;                    /* PC safe from pkills for 5 ticks      */
   int  killvictimer;           /* Timer for mobs who just killed a PC  */

   struct char_data *rider;     /* The character's rider                */
   struct char_data *mount;     /* The character's mount                */
   struct char_data *warned;    /* Player's been warned about atacking  */
                                /* this mob.                            */
   struct char_special_data_saved saved; /* constants saved in plrfile	*/
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
struct player_special_data_saved {
   byte PADDING0;		/* used to be spells_to_learn		*/
   bool talks[MAX_TONGUE];	/* PC s Tongues 0 for NPC		*/
   int	wimp_level;		/* Below this # of hit points, flee!	*/
   ubyte freeze_level;		/* Level of god who froze char, if any	*/
   sh_int invis_level;		/* level of invisibility		*/
   sh_int spare1;		/*                            		*/
   int	pref[PR_ARRAY_MAX];	/* preference flags for PC's.		*/
   ubyte bad_pws;		/* number of bad password attemps	*/
   sbyte conditions[5];         /* Drunk, full, thirsty			*/

   /* spares below for future expansion.  You can change the names from
      'sparen' to something meaningful, but don't change the order.  */

   ubyte clan;
   ubyte clan_rank;
   ubyte knight;                /* Knighted? */
   ubyte rank;
   ubyte countdown;
   ubyte pksafetimer;
   int spells_to_learn;		/* How many can you learn yet this level*/
   int tier;
   int quest_points;
   int load_room;
   int questmob;
   int arena_room;
   int bet_amt;
   int betted_on;
   int nextquest;
   int questobj;  
   int questgiver;
   long	num_kills;  /* Times this char has killed    */
   long	num_deaths; /* Times this char has died      */
   long	spare01; 
   long	spare02;
   long	god_type;  /*Storm 9/14/98*/
   long spare03;
   long spare04;
   long spare05;
   int iextra[20];
   byte bextra[10];
   ubyte uextra[10];
   /* Killing same mobs XP penalties arrays */
   int  kills_vnum[100];    /* Virtual numbers of the mobs killed */
   byte kills_amount[100];  /* Number of mobs of that type killed */
   byte kills_curpos;       /* Current position in array          */
   
};

/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data {
   struct player_special_data_saved saved;

   char	*poofin;		/* Description on arrival of a god.     */
   char	*poofout;		/* Description upon a god's exit.       */
   struct alias *aliases;	/* Character's aliases			*/
   long last_tell;		/* idnum of last tell from		*/
   void *last_olc_targ;		/* olc control				*/
   int last_olc_mode;		/* olc control				*/
   int priv;                    /* Private channel                      */
   int assigned[MAX_ASSIGNED];   /* Assigned to get corpse of?           */
   struct char_abil_type abilities[MAX_ABILITIES];/* PC Abilities         */
   char *ihide;
};


/* Specials used by NPCs, not PCs */
struct mob_special_data {
   byte last_direction;     /* The last direction the monster went     */
   int	attack_type;        /* The Attack Type Bitvector for NPC's     */
   byte default_pos;        /* Default position for NPC                */
   memory_rec *memory;	    /* List of attackers to remember	       */
   byte damnodice;          /* The number of damage dice's	       */
   byte damsizedice;        /* The size of the damage dice's           */
   int wait_state;	    /* Wait state for bashed mobs	       */
   int skill_wait;
   byte maxfactor;          /* Max number of kills of mob allowed      */
   int skills[MAX_MOB_SKILLS];  /* NPC Skills         */
   int spells[MAX_MOB_SKILLS];  /* NPC Spells         */
   int chants[MAX_MOB_SKILLS];  /* NPC Chants         */
   int prayers[MAX_MOB_SKILLS]; /* NPC Prayers        */
   byte mount_type;				/* If a mount, what type is it? */
};



/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
   byte src;             /* The type of abil that caused this       */
   sh_int type;          /* The abil num that caused this           */
   sh_int duration;      /* For how long its effects will last      */
   sbyte modifier;       /* This is added to apropriate ability     */
   byte location;        /* Tells which ability to change(APPLY_XXX)*/
   long	bitvector;       /* Tells which bits to set (AFF_XXX)       */

   struct affected_type *next;
};


/* Structure used for chars following other chars */
struct follow_type {
   struct char_data *follower;
   struct follow_type *next;
};


/* ================== Structure for player/non-player ===================== */
struct char_data {
   int pfilepos;			 /* playerfile pos		  */
   sh_int nr;                            /* Mob's rnum			  */
   room_rnum in_room;                    /* Location (real room number)	  */
   room_rnum was_in_room;		 /* location for linkdead people  */

   struct char_player_data player;       /* Normal data                   */
   struct char_stat_data real_stats;	 /* Stats without modifiers       */
   struct char_stat_data aff_stats;	 /* Stats with spells/stones/etc  */
   struct char_point_data points;        /* Points                        */
   struct char_special_data char_specials;	/* PC/NPC specials	  */
   struct player_special_data *player_specials; /* PC specials		  */
   struct mob_special_data mob_specials;	/* NPC specials		  */

   struct affected_type *affected;       /* affected by what spells       */
   struct obj_data *equipment[NUM_WEARS];/* Equipment array               */

   struct obj_data *carrying;            /* Head of list                  */
   struct descriptor_data *desc;         /* NULL for mobiles              */

   long id;                            /* used by DG triggers             */
   struct trig_proto_list *proto_script; /* list of default triggers      */
   struct script_data *script;         /* script info for the object      */
   struct script_memory *memory;       /* for mob memory triggers         */
   struct char_data *next_in_room;     /* For room->people - list         */

   struct char_data *next;             /* For either monster or ppl-list  */
   struct char_data *next_fighting;    /* For fighting list               */

   
   sh_int *completed_quests;                /* Quests completed           */
   int    num_completed_quests;             /* Number completed           */
   int    current_quest;                    /* vnum of current quest      */
   struct follow_type *followers;        /* List of chars followers       */
   struct char_data *master;             /* Who is char following?        */
   int stats;/*how many stat points does the char have left? Storm 7/15/98*/
};
/* ====================================================================== */


/* ==================== File Structure for Player ======================= */
/*             BEWARE: Changing it will ruin the playerfile		  */
struct char_file_u {
   /* char_player_data */
   char	name[MAX_NAME_LENGTH+1];
   char	description[EXDSCR_LENGTH];
   char	title[MAX_TITLE_LENGTH+1];
   byte sex;
   byte class;
   byte race;
   ubyte level;
   byte unused1;
   byte unused2;
   byte unused3;
   byte unused4;
   int  current_quest;
   long unused6;
   long unused7;
   sh_int completed_quests[MAX_COMPLETED_QUESTS];
   char unusedZ[1024];
   sh_int hometown;
   time_t birth;   /* Time of birth of character     */
   int	played;    /* Number of secs played in total */
   ubyte weight;
   ubyte height;

   char	pwd[MAX_PWD_LENGTH+1];    /* character's password */

   struct char_special_data_saved char_specials_saved;
   struct player_special_data_saved player_specials_saved;
   struct char_stat_data stats;
   struct char_point_data points;
   struct char_abil_type abilities[MAX_ABILITIES];
   struct affected_type affected[MAX_AFFECT];

   time_t last_logon;		/* Time (in secs) of last logon */
   char host[HOST_LENGTH+1];	/* host of last logon */
};
/* ====================================================================== */


/* descriptor-related structures ******************************************/


struct txt_block {
   char	*text;
   int aliased;
   struct txt_block *next;
};


struct txt_q {
   struct txt_block *head;
   struct txt_block *tail;
};


struct descriptor_data {
   socket_t	descriptor;	/* file descriptor for socket		*/
   char	host[HOST_LENGTH+1];	/* hostname				*/
   byte close_me;               /* flag: this desc. should be closed    */
   byte	bad_pws;		/* number of bad pw attemps this login	*/
   byte idle_tics;		/* tics idle at password prompt		*/
   int	connected;		/* mode of 'connectedness'		*/
   int	wait;			/* wait for how many loops		*/
   int	desc_num;		/* unique num assigned to desc		*/
   time_t login_time;		/* when the person connected		*/
   char *showstr_head;		/* for keeping track of an internal str	*/
   char **showstr_vector;	/* for paging through texts		*/
   int  showstr_count;		/* number of pages to page through	*/
   int  showstr_page;		/* which page are we currently showing?	*/
   char	**str;			/* for the modify-str system		*/
   size_t max_str;	        /*		-			*/
   char *backstr;		/* added for handling abort buffers	*/
   long	mail_to;		/* name for mail system			*/
   int	has_prompt;		/* is the user at a prompt?             */
   char	inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input		*/
   char	last_input[MAX_INPUT_LENGTH]; /* the last input			*/
   char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer		*/
   char *output;		/* ptr to the current output buffer	*/
   int  bufptr;			/* ptr to end of current output		*/
   int	bufspace;		/* space left in the output buffer	*/
   struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
   struct txt_q input;		/* q of unprocessed input		*/
   struct char_data *character;	/* linked to char			*/
   struct char_data *original;	/* original char if switched		*/
   struct descriptor_data *snooping; /* Who is this char snooping	*/
   struct descriptor_data *snoop_by; /* And who is snooping this char	*/
   struct descriptor_data *next; /* link to next descriptor		*/
   struct olc_data *olc;	     /*. OLC info - defined in olc.h   .*/
};


/* other miscellaneous structures ***************************************/


struct msg_type {
   char	*attacker_msg;  /* message to attacker */
   char	*victim_msg;    /* message to victim   */
   char	*room_msg;      /* message to room     */
};


struct message_type {
   struct msg_type die_msg;	/* messages when death			*/
   struct msg_type miss_msg;	/* messages when miss			*/
   struct msg_type hit_msg;	/* messages when hit			*/
   struct msg_type god_msg;	/* messages when hit on god		*/
   struct message_type *next;	/* to next messages of this kind.	*/
};


struct message_list {
   int	a_type;			/* Attack type				*/
   int	number_of_attacks;	/* How many attack messages to chose from. */
   struct message_type *msg;	/* List of messages.			*/
};


struct dex_skill_type {
   sh_int p_pocket;
   sh_int p_locks;
   sh_int traps;
   sh_int sneak;
   sh_int hide;
};


struct dex_app_type {
   sh_int reaction;
   sh_int miss_att;
   sh_int defensive;
};


struct str_app_type {
   sh_int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
   sh_int todam;    /* Damage Bonus/Penalty                */
   sh_int carry_w;  /* Maximum weight that can be carrried */
   sh_int wield_w;  /* Maximum weight that can be wielded  */
};


struct wis_app_type {
   byte bonus;       /* how many practices player gains per lev */
};


struct int_app_type {
   byte learn;       /* how many % a player learns a spell/skill */
};


struct con_app_type {
   sh_int hitp;
   sh_int shock;
};


struct weather_data {
   int	pressure;	/* How is the pressure ( Mb ) */
   int	change;	/* How fast and what way does it change. */
   int	sky;	/* How is the sky. */
   int	sunlight;	/* And how much sun. */
   int ground_snow;   /* Snow on the ground? */
};


struct title_type {
   char	*title_m;
   char	*title_f;
   int	exp;
};


/* element in monster and object index-tables   */
struct index_data {
   int	virtual;    /* virtual number of this mob/obj           */
   int	number;     /* number of existing units of this mob/obj	*/
   SPECIAL(*func);

   char *farg;         /* string argument for special function     */
   struct trig_data *proto;     /* for triggers... the trigger     */
};

/* linked list for mob/object prototype trigger lists */
struct trig_proto_list {
  int vnum;                             /* vnum of the trigger   */
  struct trig_proto_list *next;         /* next trigger          */
};


/* Auctioning states */

#define AUC_NULL_STATE		0   /* not doing anything */
#define AUC_OFFERING		1   /* object has been offfered */
#define AUC_GOING_ONCE		2	/* object is going once! */
#define AUC_GOING_TWICE		3	/* object is going twice! */
#define AUC_LAST_CALL		4	/* last call for the object! */
#define AUC_SOLD			5
/* Auction cancle states */
#define AUC_NORMAL_CANCEL	6	/* normal cancellation of auction */
#define AUC_QUIT_CANCEL		7	/* auction canclled because player quit */
#define AUC_WIZ_CANCEL		8	/* auction cancelled by a god */
/* Other auctioneer functions */
#define AUC_STAT			9
#define AUC_BID				10

