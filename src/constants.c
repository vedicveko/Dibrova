/* ************************************************************************
*   File: constants.c                                   Part of CircleMUD *
*  Usage: Numeric and string contants used by the MUD                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "conf.h"
#include "sysdep.h"

#include "structs.h"

const char circlemud_version[] = {
    "CircleMUD, version 3.00 beta patchlevel 12\r\nDibrova II\r\n"
};


/* strings corresponding to ordinals/bitvectors in structs.h ***********/


/* (Note: strings for class definitions in class.c instead of here) */


/* cardinal directions */
const char *dirs[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "northeast",
    "northwest",
    "southeast",
    "southwest",
    "\n"
};

const char *AEdirs[] = {
    "n",
    "e",
    "s",
    "w",
    "u",
    "d",
    "ne",
    "nw",
    "se",
    "sw",
    "\n"
};

const char *sdirs[] = {
    "n",
    "e",
    "s",
    "w",
    "u",
    "d",
    "ne",
    "nw",
    "se",
    "sw",
    " "
};


/* ROOM_x */
const char *room_bits[] = {
    "DARK",
    "DEATH",
    "!MOB",
    "INDOORS",
    "PEACEFUL",
    "SOUNDPROOF",
    "!TRACK",
    "!MAGIC",
    "TUNNEL",
    "PRIVATE",
    "GODROOM",
    "HOUSE",
    "HCRSH",
    "ATRIUM",
    "OLC",
    "*",			/* BFS MARK */
    "!PORTAL",
    "ARENA",
    "HEALING",
    "IMPEN",
    "GRID",
    "SINGLESPACE",
    "VAULT",
    "HAVEN",
    "SCORCHED",
    "COURTROOM",
    "!PLINK",
    "SALTWATER_FISHING",
    "FRESHWATER_FISHING",
    "\n"
};


/* EX_x */
const char *exit_bits[] = {
    "DOOR",
    "CLOSED",
    "LOCKED",
    "PICKPROOF",
    "HIDDEN",
    "STEEP",
    "\n"
};


/* SECT_ */
const char *sector_types[] = {
    "Inside",
    "City",
    "Field",
    "Forest",
    "Hills",
    "Mountains",
    "Water (Swim)",
    "Water (No Swim)",
    "Underwater",
    "In Flight",
    "Road",
    "Sand",
    "Desert",
    "Snow",
    "Swamp",
    "Ocean",
    "Notravel",
	"Cave",
    "\n"
};


/* SEX_x */
const char *genders[] = {
    "Neutral",
    "Male",
    "Female"
};

const char *rank[][3] = {
    {"Serf", "Serf", "Serf"},
    {"Freeman", "Freeman", "Freewoman"},
    {"Citizen", "Citizen", "Citizen"},
    {"Lord", "Lord", "Lady"},
    {"Baron", "Baron", "Baroness"},
    {"VisCount", "VisCount", "VisCountess"},
    {"Count", "Count", "Countess"},
    {"Earl", "Earl", "Earless"},
    {"Marquee", "Marquee", "Maquessa"},
    {"Duke", "Duke", "Duchess"},
    {"Prince", "Prince", "Princess"},
    {"King", "King", "Queen"},
    {"Emperor", "Emperor", "Empress"},
    {"/cBPoohbah/c0", "/cBPoohbah/c0", "/cBPoohbah/c0"},
    {"Dweeb", "Dweeb", "Dweebess"},
    {"Geek", "Geek", "Geekess"},
    {"Loser", "Loser", "Loser"}

};

/*
const char *kingdoms[] =
{

} 
*/
/* POS_x */
const char *position_types[] = {
    "Dead",
    "Mortally wounded",
    "Incapacitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "\n"
};


/* PLR_x */
const char *player_bits[] = {
    "KILLER",
    "THIEF",
    "FROZEN",
    "DONTSET",
    "WRITING",
    "MAILING",
    "CSH",
    "SITEOK",
    "NOSHOUT",
    "NOTITLE",
    "DELETED",
    "LOADRM",
    "!WIZL",
    "!DEL",
    "INVST",
    "CRYO",
    "MULTIOK",
    "DRUID",
    "TOAD",
    "EXILE",
    "BOUNTIED",
    "QUESTOR",
    "INSURED",
    "KEEPRANK",
    "PKILL_OK",
    "SELLER",
    "FISHING",
    "FISH_ON",
    "OUTFITTED",
    "PIT_QP",
    "PIT_GOLD",
    "\n"
};


/* MOB_x */
const char *action_bits[] = {
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "AWARE",
    "AGGR",
    "STAY-ZONE",
    "WIMPY",
    "AGGR_EVIL",
    "AGGR_GOOD",
    "AGGR_NEUTRAL",
    "MEMORY",
    "HELPER",
    "!CHARM",
    "!SUMMN",
    "!SLEEP",
    "!BASH",
    "!BLIND",
    "MOUNT",
    "HUNTER",
    "USE_DOORS",
    "QUESTMASTER",
    "QUEST",
    "PC_ASSIST",
    "VENOMOUS",
    "!PLINK",
    "INSTAGGRO",
    "\n"
};


/* PRF_x */
const char *preference_bits[] = {
    "BRIEF",
    "COMPACT",
    "DEAF",
    "!TELL",
    "D_HP",
    "D_MANA",
    "D_MOVE",
    "AUTOEX",
    "!HASS",
    "QUEST",
    "SUMN",
    "!REP",
    "LIGHT",
    "C1",
    "C2",
    "!WIZ",
    "L1",
    "L2",
    "!AUC",
    "!GOS",
    "!GTZ",
    "RMFLG",
    "AUTOASSIST",
    "AUTOSPLIT",
    "D_GOLD",
    "D_MOBPCT",
    "D_PCT",
    "D_XTOLVL",
    "TIPS",
    "AFK",
    "ARENA",
    "AUTOLOOT",
    "NOINFO",
    "AUTODIAG",
    "AUTOSAC",
    "D_TANK",
    "D_SHORT",
    "D_QI",
    "D_VIM",
    "D_ARIA",
    "A-TITLE",
    "MULTI",
    "A-MAP",
    "IHIDE",
    "NORANK",
    "NOGRID",
    "AUTOGOLD",
    "MOBFLAGS",
    "NOFOLLOW",
    "\n"
};

/* AFF_x */
const char *affected_bits[] = {
    "\0",			/* REQUIRED BY 128 BIT */
    "BLIND",			/* 1  */
    "INVIS",
    "DET-ALIGN",
    "DET-INVIS",
    "DET-MAGIC",
    "SENSE-LIFE",		/* 6  */
    "WATERWALK",
    "SANCT",
    "GROUP",
    "CURSE",
    "INFRA",			/* 11 */
    "POISON",
    "PROT-EVIL",
    "PROT-GOOD",
    "SLEEP",
    "!TRACK",			/* 16 */
    "SILENCED",
    "FIRE SHIELD",
    "SNEAK",
    "HIDE",
    "PARALYZE",			/* 21 */
    "CHARM",
    "FLY",
    "BREATHE",
    "HASTE",
    "CHAOS ARMOR",		/* 26 */
    "HOLY ARMOR",
    "AEGIS",
    "REGENERATE",
    "BLACKDART",
    "\0",			
    "DARKWARD",
    "SEE_HIDDEN",
    "SLOW",
    "INCOGNITO",	
    "BRACK BREATH",
    "PLAGUE",
    "RUN",
    "DEAF",
    "ANC PROT",	
    "SACRED SHIELD",
    "MENTAL BARRIER",
    "WAR CRY",
    "SHADOW",
    "AWE",	
    "MANA_ECONOMY",
    "!BLIND",
    "WEAK FLESH",
    "BALANCE",
    "COLD",		
    "DROWSE",
    "ALIGN_GOOD",
    "ALIGN_EVIL",
    "NOMAGIC",
    "INSOMNIA",		
    "BLAZEWARD",
    "INDESTRUCTABLE AURA",
    "BERSERK",
    "MIST FORM",
    "SOFTEN FLESH",	
    "CONFUSE",
    "QUEST TIME",
    "AWARE",
    "IMMBLESSED",
    "SEALED",			
    "VAMPIRIC_AURA",
    "HOVER",
    "HISS",
    "MESMERIZED",
    "SHROUD",		
    "BURROWED",
    "DEATH_CALL",
    "EARTH_SHROUD",
    "ABSORB",
    "CLOUDED",			
    "UNDER_ATTACK",
    "SPEAK",
    "AFTERLIFE",
    "DIVINE SHIELD",
    "IMPEDE",			
    "ETHEREAL",
    "ENLIGHTENED",
    "GAUGE",
    "NETHERBLAZE",
    "NETHERCLAW",		
    "DECEIVE",
    "UNFURLED",
    "ARMOR",
    "HEAVY SKIN",
    "PHANTOM ARMOR",
    "DIVINE NIMBUS",
    "STRENGTH",
    "DEFLECTION",
    "DEXTERITY",
    "INTELLIGENCE",
    "WISDOM",
    "CONSTITUTION",
    "CHARISMA",
    "LUCK",
	"MIST AURA",
    "\n"
};

/* CON_x */
const char *connected_types[] = {
    "Playing",
    "Disconnecting",
    "Get name",
    "Confirm name",
    "Get password",
    "Get new PW",
    "Confirm new PW",
    "Select sex",
    "Select class",
    "Reading MOTD",
    "Main Menu",
    "Get descript.",
    "Changing PW 1",
    "Changing PW 2",
    "Changing PW 3",
    "Self-Delete 1",
    "Self-Delete 2",
    "Object edit",
    "Room edit",
    "Zone edit",
    "Mobile edit",
    "Shop edit",
    "Get Color Pref",
    "Rolling stats",
    "Selecting hometown",
    "Selecting Race",
    "Trigger edit",
    "Quest edit",
    "\n"
};


/* WEAR_x - for eq list */
const char *where[] = {
    "<used as light>      ",
    "<worn on finger>     ",
    "<worn on finger>     ",
    "<worn around neck>   ",
    "<worn around neck>   ",
    "<worn on body>       ",
    "<worn on head>       ",
    "<worn on legs>       ",
    "<worn on feet>       ",
    "<worn on hands>      ",
    "<worn on arms>       ",
    "<worn as shield>     ",
    "<worn about body>    ",
    "<worn about waist>   ",
    "<worn around wrist>  ",
    "<worn around wrist>  ",
    "<wielded>            ",
    "<held>               ",
    "<worn on ear>        ",
    "<worn on ear>        ",
    "<worn on thumb>      ",
    "<worn on thumb>      ",
    "<worn over eyes>     ",
    "<worn on face>       ",
    "<floating>           ",
    "<floating>           ",
    "<worn on ankles>     ",
    "<worn on back>       "
};


/* WEAR_x - for stat */
const char *equipment_types[] = {
    "Used as light",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waist",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "Worn on right ear",
    "Worn on left ear",
    "Worn on right thumb",
    "Worn on left thumb",
    "Worn over eyes",
    "Worn on face",
    "Floating about",
    "Floating about",
    "Worn on ankles",
    "Worn on back",
    "\n"
};


/* ITEM_x (ordinal object types) */
const char *item_types[] = {
    "UNDEFINED",
    "LIGHT",
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",
    "FIRE WEAPON",
    "MISSILE",
    "TREASURE",
    "ARMOR",
    "POTION",
    "WORN",
    "OTHER",
    "TRASH",
    "TRAP",
    "CONTAINER",
    "NOTE",
    "LIQ CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",
    "PEN",
    "BOAT",
    "FOUNTAIN",
    "PORTAL",
    "DOLL",
    "SMOKE",
    "STONE",
    "DEED",
    "RUNE",
    "HERB",
    "JAR",
    "SHIP",
    "TILLER",
    "FISHING POLE",
    "PIECE",
    "\n"
};


/* ITEM_WEAR_ (wear bitvector) */
const char *wear_bits[] = {
    "TAKE",
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAIST",
    "WRIST",
    "WIELD",
    "HOLD",
    "EAR",
    "THUMB",
    "EYES",
    "FACE",
    "FLOATING",
    "ANKLES",
    "BACK",
    "\n"
};


/* ITEM_x (extra bits) */
const char *extra_bits[] = {
    "GLOW",
    "HUM",
    "!RENT",
    "!DONATE",
    "!INVIS",
    "INVISIBLE",
    "MAGIC",
    "!DROP",
    "BLESS",
    "!GOOD",
    "!EVIL",
    "!NEUTRAL",
    "!SORCERER",
    "!CLERIC",
    "!THIEF",
    "!GLADIATOR",
    "!SELL",
    "!DRUID",
    "!DEATHKNIGHT",
    "!MONK",
    "!PALADIN",
    "QUEST_VIS",
    "!BARD",
    "BURIED",
    "NOT SEEN",
    "NO_TRANSFER",
    "!LOCATE",
    "VAMP_CAN_SEE",
    "TITAN_CAN_SEE",
    "SAINT_CAN_SEE",
    "DEMON_CAN_SEE",
    "SILVER",
    "SMALL",
    "MEDIUM",
    "LARGE",
    "TINY",
    "PC_CORPSE",
    "NPC_CORPSE",
    "UNIQUE",
    "SPECIAL",
	"NOSTEAL",
    "\n"
};


/* APPLY_x */
const char *apply_types[] = {
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "CHA",
    "CLASS",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MAXMANA",
    "MAXHIT",
    "MAXMOVE",
    "GOLD",
    "EXP",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "LUCK",
    "SPELLPOWER",
    "MAXQI",
    "MAXVIM",
    "MAXARIA",
    "\n"
};


/* CONT_x */
const char *container_bits[] = {
    "CLOSEABLE",
    "PICKPROOF",
    "CLOSED",
    "LOCKED",
    "\n",
};

/* HERBS */
const char *herb_bits[] = {
    "ACONITE",
    "CALEDONIA",
    "GOLDENSEAL",
    "WILDROOT",
    "NIRINBARK",
    "COLTSFOOT",
    "BURDOCK",
    "ANGELICA",
    "SALSTINE",
    "BAYBERRY",
    "\n",
};


/* LIQ_x */
const char *drinks[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "clear water",
    "liquid health",
    "liquid mana",
    "liquid qi",
    "liquid move",
    "liquid vim",
    "immortality",
    "good alignment",
    "\n"
};


/* other constants for liquids ******************************************/


/* one-word alias for each drink */
const char *drinknames[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "water",
    "health",
    "mana",
    "qi",
    "move",
    "vim",
    "immortality",
    "alignment",
    "\n"
};


/* effect of drinks on hunger, thirst, and drunkenness -- see values.doc */
const int drink_aff[][3] = {
    {0, 1, 10},
    {3, 2, 5},
    {5, 2, 5},
    {2, 2, 5},
    {1, 2, 5},
    {6, 1, 4},
    {0, 1, 8},
    {10, 0, 0},
    {3, 3, 3},
    {0, 4, -8},
    {0, 3, 6},
    {0, 1, 6},
    {0, 1, 6},
    {0, 2, -1},
    {0, 1, -2},
    {0, 0, 13}
};


/* color of the various drinks */
const char *color_liquid[] = {
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "crystal clear",
    "brightly shining",
    "glowing red",
    "glowing white",
    "bright green",
    "forest green",
    "glowing",
    "pure white"
};


/* level of fullness for drink containers */
const char *fullness[] = {
    "less than half ",
    "about half ",
    "more than half ",
    ""
};


/* str, int, wis, dex, con applies **************************************/


/* [ch] strength apply (all) */
const struct str_app_type str_app[] = {
    {-5, -4, 0, 0},		/* str = 0 */
    {-5, -4, 3, 1},		/* str = 1 */
    {-3, -2, 3, 2},
    {-3, -1, 10, 3},
    {-2, -1, 25, 4},
    {-2, -1, 55, 5},		/* str = 5 */
    {-1, 0, 80, 6},
    {-1, 0, 88, 7},
    {1, 0, 95, 8},
    {1, 0, 100, 9},
    {1, 0, 110, 10},		/* str = 10 */
    {1, 0, 115, 11},
    {1, 0, 120, 12},
    {1, 0, 125, 13},
    {1, 0, 140, 14},
    {1, 0, 160, 15},		/* str = 15 */
    {1, 1, 180, 16},
    {2, 1, 200, 18},
    {2, 2, 255, 20},		/* dex = 18 */
    {4, 7, 300, 40},
    {5, 8, 350, 40},		/* str = 20 */
    {5, 9, 400, 40},
    {5, 10, 450, 40},
    {6, 11, 500, 40},
    {7, 12, 550, 40},
    {8, 14, 650, 40},		/* str = 25 */
    {1, 3, 280, 22},		/* str = 18/0 - 18-50 */
    {2, 3, 305, 24},		/* str = 18/51 - 18-75 */
    {2, 4, 330, 26},		/* str = 18/76 - 18-90 */
    {2, 5, 380, 28},		/* str = 18/91 - 18-99 */
    {3, 6, 480, 30}		/* str = 18/100 */
};



/* [dex] skill apply (thieves only) */
const struct dex_skill_type dex_app_skill[] = {
    {-99, -99, -90, -99, -60},	/* dex = 0 */
    {-90, -90, -60, -90, -50},	/* dex = 1 */
    {-80, -80, -40, -80, -45},
    {-70, -70, -30, -70, -40},
    {-60, -60, -30, -60, -35},
    {-50, -50, -20, -50, -30},	/* dex = 5 */
    {-40, -40, -20, -40, -25},
    {-30, -30, -15, -30, -20},
    {-20, -20, -15, -20, -15},
    {-15, -10, -10, -20, -10},
    {-10, -5, -10, -15, -5},	/* dex = 10 */
    {-5, 0, -5, -10, 0},
    {0, 0, 0, -5, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},		/* dex = 15 */
    {0, 5, 0, 0, 0},
    {5, 10, 0, 5, 5},
    {10, 15, 5, 10, 10},	/* dex = 18 */
    {15, 20, 10, 15, 15},
    {15, 20, 10, 15, 15},	/* dex = 20 */
    {20, 25, 10, 15, 20},
    {20, 25, 15, 20, 20},
    {25, 25, 15, 20, 20},
    {25, 30, 15, 25, 25},
    {25, 30, 15, 25, 25}	/* dex = 25 */
};



/* [dex] apply (all) */
struct dex_app_type dex_app[] = {
    {-7, -7, 6},		/* dex = 0 */
    {-6, -6, 5},		/* dex = 1 */
    {-4, -4, 5},
    {-3, -3, 4},
    {-2, -2, 3},
    {-1, -1, 2},		/* dex = 5 */
    {0, 0, 1},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},			/* dex = 10 */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, -1},			/* dex = 15 */
    {1, 1, -2},
    {2, 2, -3},
    {2, 2, -4},			/* dex = 18 */
    {3, 3, -4},
    {3, 3, -4},			/* dex = 20 */
    {4, 4, -5},
    {4, 4, -5},
    {4, 4, -5},
    {5, 5, -6},
    {5, 5, -6}			/* dex = 25 */
};



/* [con] apply (all) */
struct con_app_type con_app[] = {
    {-4, 20},			/* con = 0 */
    {-3, 25},			/* con = 1 */
    {-2, 30},
    {-2, 35},
    {-1, 40},
    {-1, 45},			/* con = 5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},			/* con = 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},			/* con = 15 */
    {2, 95},
    {2, 97},
    {3, 99},			/* con = 18 */
    {3, 99},
    {4, 99},			/* con = 20 */
    {5, 99},
    {5, 99},
    {5, 99},
    {6, 99},
    {6, 99}			/* con = 25 */
};



/* [int] apply (all) */
struct int_app_type int_app[] = {
    {3},			/* int = 0 */
    {5},			/* int = 1 */
    {7},
    {8},
    {9},
    {10},			/* int = 5 */
    {11},
    {12},
    {13},
    {14},
    {16},			/* int = 10 */
    {18},
    {20},
    {23},
    {27},
    {30},			/* int = 15 */
    {34},
    {38},
    {41},			/* int = 18 */
    {43},
    {46},			/* int = 20 */
    {49},
    {52},
    {54},
    {56},
    {58}			/* int = 25 */
};


/* [wis] apply (all) */
struct wis_app_type wis_app[] = {
    {0},			/* wis = 0 */
    {0},			/* wis = 1 */
    {0},
    {0},
    {0},
    {0},			/* wis = 5 */
    {1},
    {1},
    {1},
    {1},
    {1},			/* wis = 10 */
    {2},
    {2},
    {2},
    {3},
    {3},			/* wis = 15 */
    {3},
    {3},
    {4},			/* wis = 18 */
    {4},
    {4},			/* wis = 20 */
    {4},
    {4},
    {5},
    {5},
    {6}				/* wis = 25 */
};



const char *spell_wear_off_msg[] = {
    "RESERVED DB.C",		/* 0 */
    "You feel less protected.",	/* 1 */
    "!Teleport!",
    "You feel less righteous.",
    "You feel a cloak of blindness disolve.",
    "!Burning Hands!",		/* 5 */
    "!Call Lightning",
    "You feel more self-confident.",
    "You feel your strength return.",
    "!Clone!",
    "!Color Spray!",		/* 10 */
    "!Control Weather!",
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",		/* 15 */
    "!Cure Light!",
    "You feel more optimistic.",
    "You feel less aware.",
    "Your eyes stop tingling.",
    "The detect magic wears off.",	/* 20 */
    "The detect poison wears off.",
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",		/* 25 */
    "!Fireball!",
    "!Harm!",
    "!Heal!",
    "You feel yourself exposed.",
    "!Lightning Bolt!",		/* 30 */
    "!Locate object!",
    "!Magic Missile!",
    "The poison leaves you.",
    "You feel less protected.",
    "!Remove Curse!",		/* 35 */
    "The white aura around your body fades.",
    "!Shocking Grasp!",
    "You feel less tired.",
    "Your magical strength fades away.",
    "!summon!",			/* 40 */
    "!Ventriloquate!",
    "!Word of Recall!",
    "!Remove Poison!",
    "You feel less aware of your surroundings.",
    "!Animate Dead!",		/* 45 */
    "!Dispel Good!",
    "!Group Armor!",
    "!Group Cure Light!",
    "!Group Recall!",
    "Your night vision seems to fade.",	/* 50 */
    "Your feet become less buoyant.",
    "You gently float to the ground.",
    "!Refresh!",
    "!Dimensional Door!",
    "!Portal!",			/* 55 */
    "Your speed returns to normal.",
    "!Cure ALL!",
    "!Fire Storm!",
    "Your body is no longer regenerating.",
    "!Create Spring!",		/* 60 */
    "Your fire shield carries away into the wind.",
    "!Banish!",
    "!Holy Word!",
    "!Demon Fire!",
    "You are no longer silenced.",
    "You feel more vulnerable.",
    "!Black Breath!",
    "Your chaos armor dissipates into a thin mist.",
    "You are no longer paralyzed.",
    "Your protective shield wears off.", /* 70 */
    "Your protective ward wears away.",
    "!Wrath of God!",
    "!Ice Storm!",
    "!Black Dart!",
    "Your eyes come back into focus.",  /* 75 */
    "Your gills vanish.",
    "You feel less slow.",
    "!cause light!",
    "!cause critical!",
    "!lay hands!", /* 80 */
    "!restore mana!",
    "!restore mega-mana!",
    "!remove invis!",
    "You feel smarter.",
    "The phantoms leave you.",
    "Your spectral wings vanish and you fall to the ground.",
    "!undead mount!",
    "You no longer feel like you can take on an army.",
    "Your magical strength fades away.", /* 89 */
    "!holy mace!", /* 90 */
    "You feel better.",
    "You can hear again.",
    "!ray of light!",
    "You feel your body shrink back to its normal size.",
    "!identify!", /* 95 */
    "!fire breath!",
    "!gas breath!",
    "!frost breath!",
    "!acid breath!",
    "!lightning breath!",	/* 100 */
    "You feel less in control of your mana.",
    "!stone hail!",
    "!flying fist!",
    "!shock sphere!",
    "!caustic rain",
    "!wither!",
    "!meteor swarm!",
    "!balefire!",
    "!scorch!",
    "!immolate!",
    "!hellfire!",
    "!frost blade!",
    "!flame blade!",
    "!acid stream!",
    "!flame strike!",
    "!finger of death!",
    "!scourge!",
    "!soul rip!",
    "!fellblade!",	
    "!exterminate!",            /* 120 */
    "!knock!",
    "!blade light!",
    "!flames pure!",
    "!priz beam!",
    "!!",
    "!holocaust!",
    "!cry justice!",
    "!vim restore!",
    "!qi restore!",		
    "!aria restore!",
    "You slowly phase into focus.",
    "Your body warms back up. You can move again.",
    "You lose your opacity.",
    "Your insomnia is gone.",
    "The blaze subsides.",      /* 135 */
    "You are no longer indestructable.",
    "!entangle!",
    "You form becomes more substantial.",
    "Your flesh becomes less soft.",
    "You feel less agitated.",
    "!confuse!",
    "!clear sky!",
    "!animate corpse!",
    "You can move freely once again.",
    "!potion-only sleep!",
    "!find familiar!",
    "!invisible stalker!",
    "!holy steed!",
    "Your nimbus fades away.",
    "!clairvoyance!",
    "!summon dahlin!",
    "!summon fehzu!",
    "!summon necrolieu!",
    "Your Netherclaw fades and then disappears altogether.",
    "!conjure drake!",
    "!rain of fire!",
    "!treewalk dearthwood!",
    "!treewalk grasslands!",
    "!treewalk tanglewood!",
    "!treewalk elven valley!",
    "!treewalk darkwood!",
    "!treewalk sinister forest!",
    "!treewalk hermit!",
    "!treewalk crystalmir!",
    "!treewalk thewster!",
    "!treewalk kailaani!",
    "!treewalk little sister!",
    "!sense spawn!",
    "You feel a little less strong",
    "You feel a lot less strong",
    "You feel a bit less dextrous.",
    "You feel less dextrous.",
    "You feel slightly less intelligent.",
    "You feel less intelligent.",
    "You feel slightly less wise.",
    "You feel less wise.",
    "Your constitution lowers slightly",
    "Your constitution drops.",
    "You feel slightly less charismatic",
    "You feel less charismatic",
    "You feel slightly less lucky.",
    "You feel less lucky.",
    "/c0"
};

const char *chant_wear_off_msg[] = {
    "RESERVED DB.C",		/* 0 */
    "!healing hand!",
    "The ancients turn their attention to the more needy.",
    "You become less centered.",
    "You feel less controlled.",
    "!yin xu!",			/* 5 */
    "!yang xu!",
    "Your mental barrier fades.",
    "Your strength returns.",
    "!lend health!",
    "!psychic fury!",		/* 10 */
    "You feel your balance slip back to normal."
};

const char *prayer_wear_off_msg[] = {
    "RESERVED DB.C",		/* 0 */
    "!cure light!",
    "!cure critical!",
    "!remove poison!",
    "!lift curse!",
    "!gods fury!",		/* 5 */
    "You feel less blessed.",
    "!dispel good!",
    "!dispel evil!",
    "!harm!",
    "!food!",			/* 10 */
    "!water!",
    "!detect poison!",
    "Your skin softens.",
    "!guiding light!" "Your shimmering aura fades.",	/* 15 */
    "Your mortal sight returns.",
    "Things seem less bright.",
    "Your vision returns!",
    "!cure blindness!",
    "!group armor!",		/* 20 */
    "!group heal!",
    "!earthquake!",
    "!call lightning!",
    "The world around you seems to speed up as you slow down.",
    "!summon!",			/* 25 */
    "!refresh!",
    "Your sacred shield dissipates.",
    "!heal!",
    "!pacify!",
    "!hand of balance!",
    "!elemental burst!",
    "!wings of pain!",
    "!spirit strike!",
    "!abomination!",
    "!winds of reckoning!",	/* 35 */
    "!angel breath!",
    "!soul scourge!",
    "!plague!",
    "Your flesh does not seem so weak anymore.",
    "You begin to feel less drowsy, your movements more sure.",	/* 40 */
    "!vitality!",
    "!reverse align!",
    "!retrieve corpse!",
    "You feel less agitated.",
    "!holy water!",
    "!exorcism!",
    "You are no longer protected by afterlife.",
    "!heavens beam",
    "Your divine shield wears off.",
    "You are no longer ethereal.",
    "!choir!",
    "You are no longer enlightened.",
    "!cure serious!",
    "The holy armor wears off.",
    "Your shield of deflection has worn off."
};

const char *song_wear_off_msg[] = {
    "RESERVED DB.C",
    "!bravery!",
    "!honor!",
    "!hero!",
    "You are less tired."
};

char *npc_class_types[] = {
    "Other",
    "Mage",
    "Cleric",
    "Thief",
    "Warrior",
    "Druid",
    "Paladin",
    "Doll Mage",
    "Monk",
    "Dark Knight",
    "City Guard",
    "Receptionist",
    "Citizen",
    "Clan Guard",
    "Vampire",
    "\n"
};

char *npc_race_types[] = {
    "Other",
    "Mammal",
    "Reptile",
    "Avian",
    "Fish",
    "Insect",
    "Undead",
    "Humanoid",
    "Beast",
    "Dragon",
    "Giant",
    "Shadow",
    "Aqua Elf",
    "Dwarf",
    "Human",
    "Elf",
    "Drow",
    "Halfling",
    "Kender",
    "Ogre",
    "Eldar",
    "Wolfen",
    "Triton",
    "Valkyrie",
    "Sprite",
    "Treant",
    "Plant",
    "Hydra",
    "Broken One",
    "Doom Guard",
    "Elemental",
    "Goblin",
    "Orc",
    "Golem",
    "Spirit",
    "Reaver",
    "Skeleton",
    "Illithid",
    "Gorgon",
    "Werebeast",
    "Titan",
    "Demon",
    "Lizardman",
    "Scorpion",
    "Lich",
    "Celestial",
    "Spider",
    "Naga",
    "Mummy",
    "Wraith",
    "Minotaur",
    "Mephit",
    "Equine",
    "\n",
};


const int rev_dir[] = {
    2,
    3,
    0,
    1,
    5,
    4,
    7,
    6,
    9,
    8
};

#if defined(OASIS_MPROG)
/*
 * Definitions necessary for MobProg support in OasisOLC
 */
const char *mobprog_types[] = {
    "INFILE",
    "ACT",
    "SPEECH",
    "RAND",
    "FIGHT",
    "DEATH",
    "HITPRCNT",
    "ENTRY",
    "GREET",
    "ALL_GREET",
    "GIVE",
    "BRIBE",
    "\n"
};
#endif


const int movement_loss[] = {
    1,				/* Inside     */
    1,				/* City       */
    2,				/* Field      */
    3,				/* Forest     */
    4,				/* Hills      */
    6,				/* Mountains  */
    4,				/* Swimming   */
    1,				/* Unswimable */
    5,				/* Underwater */
    1,				/* Flying     */
    1,				/* Road       */
    6,				/* Sand       */
    7,				/* Desert     */
    4,				/* Snow       */
    4,				/* Swamp      */
    4,				/* Ocean      */
	1,				/* Notravel   */
	1				/* Cave       */
};


const char *weekdays[] = {
    "the Day of the Moon",
    "the Day of the Bull",
    "the Day of the Deception",
    "the Day of Thunder",
    "the Day of Freedom",
    "the day of the Great Gods",
    "the Day of the Sun"
};


const char *month_name[] = {
    "Month of Winter",		/* 0 */
    "Month of the Winter Wolf",
    "Month of the Frost Giant",
    "Month of the Old Forces",
    "Month of the Grand Struggle",
    "Month of the Spring",
    "Month of Nature",
    "Month of Futility",
    "Month of the Dragon",
    "Month of the Sun",
    "Month of the Heat",
    "Month of the Battle",
    "Month of the Dark Shades",
    "Month of the Shadows",
    "Month of the Long Shadows",
    "Month of the Ancient Darkness",
    "Month of the Great Evil"
};

const char *god_types[] = {
    "/cMClan Master/c0",
    "/cbDg Scripts/c0",
    "/cgCoder/c0",
    "/cmBuilder/c0",
    "/ccAdministration/c0",
    "/cyQuests/c0"
};

const char *god_lvls[] = {
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};

char *prac_types[] = {
    "skill",
    "spell",
    "chant",
    "prayer",
    "song"
};

const int cha_max_followers[26] = {
    1,				/* 0 */
    1,				/* 1 */
    1,
    1,
    1,
    1,				/* 5 */
    1,
    1,
    1,
    1,
    1,				/* 10 */
    1,
    1,
    2,
    2,
    2,				/* 15 */
    2,
    3,
    3,
    3,
    3,				/* 20 */
    3,
    3,
    4,
    4,
    4				/* 25 */
};

char *item_materials[] = {
   "UNDEFINED",
   "cloth",
   "leather",
   "copper",
   "iron",
   "steel",
   "silver",
   "magical",
   "\n"
};
