/* Race support for Dibrova.  */


#include "sysdep.h"
#include "conf.h"

#include "structs.h"
#include "handler.h"
#include "interpreter.h"
#include "utils.h"

const char *race_abbrevs[] = {
    "Dwa",
    "Eld",
    "Elf",
    "Gnt",
    "Hlf",
    "Hum",
    "Ken",
    "Ogr",
    "Tri",
    "Val",
    "Wol",
    "Gob",
    "Drd",
    "Trl",
    "Pix",
    "Drw",
    "Gnm",
    "Nym",
    "Nga",
    "Cen",
    "Gar",
    "Hrp",
    "Drc",
    "Scp",
    "Spr",
    "Avi",
    "\n"
};

const char *pc_race_types[] = {
    "Dwarf",
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
    "Goblin",
    "Drider",
    "Troll",
    "Pixie",
    "Drow",
    "Gnome",
    "Nymph",
    "Naga",
    "Centaur",
    "Gargoyle",
    "Harpy",
    "Draconian",
    "Scorpius",
    "Sprite",
    "Aviades",
    "\n"
};

int min_stat_table[NUM_RACES][NUM_STATS] = {
/* INT WIS DEX STR CON CHA LUCK */
  { 6,  8, 15,  9, 13,  5,  8}, // Dwarf
  {15,  9,  6,  6,  5,  8, 13}, // Eldar
  { 6,  6,  9,  7, 15, 13,  8}, // Elf
  { 5,  6,  9, 13, 15,  7,  3}, // Giant
  { 8,  6, 10,  8,  8,  6, 10}, // Halfling (NOL)
  { 6, 13,  9,  7,  7,  7, 15}, // Human
  { 7,  7, 13,  9,  8,  7, 15}, // Kender
  { 6,  6,  6, 14, 12,  6, 12}, // Ogre (NOL)
  { 7,  9, 15,  8,  9,  3, 13}, // Triton
  { 8,  6,  8,  9, 13, 15,  5}, // Valkyrie
  { 5,  5, 15,  9, 13,  9,  8}, // Wolfen
  { 7,  7, 15,  9,  7,  6, 13}, // Goblin
  { 7,  6,  9, 15,  9,  5, 13}, // Drider
  { 9,  8, 13, 15,  9,  4,  6}, // Troll
  { 9, 13,  9,  5,  6,  7, 15}, // Pixie
  {13, 15,  7,  9,  7,  4,  9}, // Drow
  { 9, 15,  6, 13,  6,  8,  7}, // Gnome
  {15, 13,  8,  9,  5,  6,  8}, // Nymph
  { 7,  7,  9, 15,  9, 13,  4}, // Naga
  {13,  9,  6, 15,  8,  5,  8}, // Centaur
  { 5,  8,  5, 13, 15,  9,  9}, // Gargoyle
  { 7,  7,  9,  9, 13,  4, 15}, // Harpy
  { 6,  6,  7, 13, 15,  8,  9}, // Draconian
  { 8,  6,  8, 15,  9,  5, 13}, // Scorpius
  { 8,  8, 13,  5,  9, 15,  6}, // Sprite
  { 8, 15, 13,  6,  8,  5,  9}  // Aviades
};

int max_stat_table[NUM_RACES][NUM_STATS] = {
/* INT WIS DEX STR CON CHA LUCK */
  {16, 18, 25, 19, 23, 15, 18}, // Dwarf
  {25, 19, 16, 16, 15, 18, 23}, // Eldar
  {16, 16, 19, 17, 25, 23, 18}, // Elf
  {15, 16, 19, 23, 25, 17, 13}, // Giant
  {18, 16, 20, 18, 18, 16, 20}, // Halfling (NOL)
  {16, 23, 19, 17, 17, 17, 25}, // Human
  {17, 17, 23, 19, 18, 17, 25}, // Kender
  {16, 16, 16, 24, 22, 16, 22}, // Ogre (NOL)
  {17, 19, 25, 18, 19, 13, 23}, // Triton
  {18, 16, 18, 19, 23, 25, 15}, // Valkyrie
  {15, 15, 25, 19, 23, 19, 18}, // Wolfen
  {17, 17, 25, 19, 17, 16, 23}, // Goblin
  {17, 16, 19, 25, 19, 15, 23}, // Drider
  {19, 18, 23, 25, 19, 14, 16}, // Troll
  {19, 23, 19, 15, 16, 17, 25}, // Pixie
  {23, 25, 17, 19, 17, 14, 19}, // Drow
  {19, 25, 16, 23, 16, 18, 17}, // Gnome
  {25, 23, 18, 19, 15, 16, 18}, // Nymph
  {17, 17, 19, 25, 19, 23, 14}, // Naga
  {23, 19, 16, 25, 18, 15, 18}, // Centaur
  {15, 18, 15, 23, 25, 19, 19}, // Gargoyle
  {17, 17, 19, 19, 23, 14, 25}, // Harpy
  {16, 16, 17, 23, 25, 18, 19}, // Draconian
  {18, 16, 18, 25, 19, 15, 23}, // Scorpius
  {18, 18, 23, 15, 19, 25, 16}, // Sprite
  {18, 25, 23, 16, 18, 15, 19}  // Aviades
};

/* These indices are all related to human (1.0) */
const float race_height_idx[] = {
    .67,
    1.00,
    1.10,
    1.80,
    .55,
    1.00,
    .70,
    1.50,
    1.00,
    1.00,
    1.00,
    .85,
    1.00,
    1.20,
    .45,
    1.00,
    .70,
    .65,
    1.15,
    1.10,
    1.00,
    1.00,
    1.00,
    1.00,
    .75,
    1.00,
};

/* Pounds per inch */
const float race_weight_idx[] = {
    3.25,
    1.75,
    2.25,
    2.50,
    2.75,
    2.50,
    2.50,
    3.00,
    2.50,
    2.50,
    2.75,
    2.75,
    2.50,
    2.75,
    1.75,
    2.50,
    2.50,
    1.50,
    2.65,
    2.75,
    2.50,
    2.50,
    2.65,
    2.50,
    1.90,
    2.50
};

/* Race menu has been divided by class, many to choose now */
const char *race_menu_sorc =
"\r\n"
" /cWSORCERER RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary SORCERER races include:\r\n"
" /crDrow, Eldar, Pixie\r\n"
" /cc*choosing any of these races will award bonus spellpower\r\n\r\n"
" /cwSecondary SORCERER races include:\r\n"
" /crAviades, Gargoyle, Gnome, Goblin, Harpy, Sprite, Troll\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwSORCERERs may choose, but encouraged not to, the following races:\r\n"
" /crCentaur, Draconian, Drider, Dwarf, Elf, Giant, Kender, Naga, Nymph,\r\n"
" /crScorpius, Triton, Valkyrie, Wolfen, Human\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_cleric =
"\r\n"
" /cWCLERIC RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary cleric races include:\r\n"
" /crCentaur, Gnome, Nymph\r\n"
" /cc*choosing any of these races will award bonus spellpower\r\n\r\n"
" /cwSecondary SORCERER races include:\r\n"
" /crAviades, Draconian, Eldar, Giant, Kender, Sprite, Valkrie\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwSORCERERs may choose, but encouraged not to, the following races:\r\n"
" /crDrider, Drow, Dwarf, Elf, Gargoyle, Goblin, Harpy, Naga, Pixie,\r\n"
" /crScorpius, Triton, Troll, Wolfen, Human\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_glad =
"\r\n"
" /cWGLADIATOR RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary gladiator races include:\r\n"
" /crDwarf, Giant, Troll\r\n"
" /cc*choosing any of these races will award bonus damroll\r\n\r\n"
" /cwSecondary galdiator races include:\r\n"
" /crCentaur, Drider, Drow, Gargoyle, Scorpius, Triton, Wolfen\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwGladiators may choose, but encouraged not to, the following races:\r\n"
" /crAviades, Draconian, Eldar, Elf, Gnome, Goblin, Harpy, Kender, Naga,\r\n"
" /crNymph, Pixie, Sprite, Valkyrie, Human\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_thief =
"\r\n"
" /cWTHIEF RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary thief races include:\r\n"
" /crDrider, Goblin, Kender\r\n"
" /cc*choosing any of these races will award bonus hitroll and damroll\r\n\r\n"
" /cwSecondary thief races include:\r\n"
" /crDwarf, Elf, Gnome, Harpy, Naga, Pixie, Triton\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n" 
" /cwThieves may choose, but encouraged not to, the following races:\r\n"
" /crAviades, Centaur, Draconian, Drow, Eldar, Gargoyle, Giant, Nymph,\r\n"
" /crScorpius, Sprite, Troll, Human, Valkyrie, Wolfen\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_pal =
"\r\n"
" /cWPALADIN RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary paladin races include:\r\n"
" /crGargoyle, Naga, Valkyrie\r\n"
" /cc*choosing any of these races will award bonus hitroll and spellpower\r\n\r\n"
" /cwSecondary paladin races include:\r\n"
" /crCentaur, Dwarf, Eldar, Elf, Human\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwPaladins may choose, but encouraged not to, the following races:\r\n"
" /crAviades, Draconian, Drider, Drow, Elf, Giant, Gnome, Harpy, Kender,\r\n"
" /crNymph, Pixie, Scorpius, Sprite, Triton, Troll, Wolfen\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_dk =
"\r\n"
" /cWDARK KNIGHT RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary dark knight races include:\r\n"
" /crDraconian, Harpy, Scorpius\r\n"
" /cc*choosing any of these races will award bonus hitroll and spellpower\r\n\r\n"
" /cwSecondary dark knight races include:\r\n"
" /crDrider, Drow, Troll, Gargoyle, Human, Wolfen\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwDark knights may choose, but encouraged not to, the following races:\r\n"
" /crAviades, Centaur, Dwarf, Eldar, Elf, Giant, Gnome, Goblin, Kender,\r\n"
" /crNaga, Nymph, Pixie, Sprite, Triton, Valkyrie\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_monk =
"\r\n"
" /cWMONK RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary monk races include:\r\n"
" /crAviades, Triton, Human\r\n"
" /cc*choosing any of these races will award bonus damroll and armor class\r\n\r\n"
" /cwSecondary monk races include:\r\n"
" /crDraconian, Dwarf, Eldar, Elf, Goblin, Naga, Nymph\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwMonks may choose, but encouraged not to, the following races:\r\n"
" /crCentaur, Drider, Drow, Gargoyle, Giant, Gnome, Goblin, Harpy, Kender,\r\n"
" /crPixie, Scorpius, Sprite, Triton, Troll, Valkyrie, Wolfen\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";

const char *race_menu_druid =
"\r\n"
" /cWDRUID RACE SELECTION\r\n"
" /c0-----------------------\r\n"
" /cwPrimary druid races include:\r\n"
" /crElf, Sprite, Wolfen\r\n"
" /cc*choosing any of these races will award bonus damroll, hitroll, and spellpower\r\n\r\n"
" /cwSecondary druid races include:\r\n"
" /crGiant, Kender, Human, Nymph, Pixie, Scorpius, Valkyrie\r\n"
" /cc*none of these races award any bonus or penalty stats\r\n\r\n"
" /cwDruids may choose, but encouraged not to, the following races:\r\n"
" /crAviades, Centaur, Draconian, Drider, Drow, Dwarf, Eldar, Gargoyle,\r\n"
" /crGnome, Goblin, Harpy, Naga, Triton, Troll\r\n"
" /cc*these races incur a penalty to hitroll, damroll, and spellpower\r\n\r\n"
" /cwYour choice?/c0\r\n\r\n\r\n"
" /ccFOR FURTHER INFORMATION ON RACES, SPECIFICALLY SIZE RESTRICTIONS, \r\n"
" /ccVISIT OUR WEBSITE AT WWW.DIBROVA.ORG\r\n";


/* The menu for choosing a race in interpreter.c: */
const char *race_menu =
"\r\n"
" /cWSelect a race:/c0\r\n\r\n"
" /cyRACE       SIZE    PRIMARY CLASS   SECONDARY CLASS\r\n" 
" /cw-----------------------------------------------------------\r\n"
" /crAviades    SMALL   MONK            CLERIC, SORCERER\r\n"
" /crCentaur    LARGE   CLERIC          GLADIATOR, PALADIN\r\n"
" /crDraconian  MEDIUM  DARK KNIGHT     CLERIC, MONK\r\n"
" /crDrider     MEDIUM  THIEF           GLADIATOR, DARK KNIGHT\r\n"
" /crDrow       SMALL   SORCERER        GLADIATOR, DARK KNIGHT\r\n"
" /crDwarf      SMALL   GLADIATOR       THIEF, PALADIN, MONK\r\n"
" /crEldar      MEDIUM  SORCERER        CLERIC, PALADIN, MONK\r\n"
" /crElf        SMALL   DRUID           THIEF, PALADIN, MONK\r\n"
" /crGargoyle   LARGE   PALADIN         GLADIATOR, SORCERER\r\n"
" /crGiant      LARGE   GLADIATOR       CLERIC, DRUID\r\n"
" /crGnome      TINY    CLERIC          THIEF, SORCERER\r\n"
" /crGoblin     SMALL   THIEF           SORCERER, MONK\r\n"
//" /crHalfling   SMALL   THIEF           DRUID, CLERIC\r\n" -removed race (-Adrian)
" /crHarpy      SMALL   DARK KNIGHT     THIEF, SORCERER\r\n"
" /crHuman      MEDIUM  NONE            ALL\r\n"
" /crKender     TINY    THIEF           CLERIC, DRUID\r\n"
" /crNaga       MEDIUM  PALADIN         THIEF, MONK\r\n"
" /crNymph      SMALL   CLERIC          MONK, DRUID\r\n"
//" /crOgre       LARGE   GLADIATOR       DARK KNIGHT, CLERIC\r\n" -removed race (-Adrian)
" /crPixie      TINY    SORCERER        THIEF, DRUID\r\n"
" /crScorpius   LARGE   DARK KNIGHT     GLADIATOR, DRUID\r\n"
" /crSprite     TINY    DRUID           CLERIC, SORCERER\r\n"
" /crTriton     MEDIUM  MONK            GLADIATOR, THIEF\r\n"
" /crTroll      LARGE   GLADIATOR       SORCERER, DARK KNIGHT\r\n"
" /crValkyrie   MEDIUM  PALADIN         CLERIC, DRUID\r\n"
" /crWolfen     LARGE   DRUID           GLADIATOR, CLERIC\r\n"
" /cWYour choice?\r\n"
" /cy* Size will make a bearing on which equipment you will be able\r\n"
"   to wear. Primary classes get stat bonuses, secondary get no penalties.\r\n";


int parse_race(char *arg) {

  if (isname(arg, (char *)pc_race_types[RACE_DWARF])) {
    return RACE_DWARF;
  } else if (isname(arg, (char *)pc_race_types[RACE_ELDAR])) {
    return RACE_ELDAR;
  } else if (isname(arg, (char *)pc_race_types[RACE_ELF])) {
    return RACE_ELF;
  } else if (isname(arg, (char *)pc_race_types[RACE_GIANT])) {
    return RACE_GIANT;
/* -removed race (-Adrian)
  } else if (isname(arg, (char *)pc_race_types[RACE_HALFLING])) {
    return RACE_HALFLING;
*/
  } else if (isname(arg, (char *)pc_race_types[RACE_HUMAN])) {
    return RACE_HUMAN;
  } else if (isname(arg, (char *)pc_race_types[RACE_KENDER])) {
    return RACE_KENDER;
/* -removed race (-Adrian)
  } else if (isname(arg, (char *)pc_race_types[RACE_OGRE])) {
    return RACE_OGRE;
*/
  } else if (isname(arg, (char *)pc_race_types[RACE_TRITON])) {
    return RACE_TRITON;
  } else if (isname(arg, (char *)pc_race_types[RACE_VALKYRIE])) {
    return RACE_VALKYRIE;
  } else if (isname(arg, (char *)pc_race_types[RACE_WOLFEN])) {
    return RACE_WOLFEN;
  } else if (isname(arg, (char *)pc_race_types[RACE_GOBLIN])) {
    return RACE_GOBLIN;
  } else if (isname(arg, (char *)pc_race_types[RACE_DRIDER])) {
    return RACE_DRIDER;
  } else if (isname(arg, (char *)pc_race_types[RACE_TROLL])) {
    return RACE_TROLL;
  } else if (isname(arg, (char *)pc_race_types[RACE_PIXIE])) {
    return RACE_PIXIE;
  } else if (isname(arg, (char *)pc_race_types[RACE_DROW])) {
    return RACE_DROW;
  } else if (isname(arg, (char *)pc_race_types[RACE_GNOME])) {
    return RACE_GNOME;
  } else if (isname(arg, (char *)pc_race_types[RACE_NYMPH])) {
    return RACE_NYMPH;
  } else if (isname(arg, (char *)pc_race_types[RACE_NAGA])) {
    return RACE_NAGA;
  } else if (isname(arg, (char *)pc_race_types[RACE_CENTAUR])) {
    return RACE_CENTAUR;
  } else if (isname(arg, (char *)pc_race_types[RACE_GARGOYLE])) {
    return RACE_GARGOYLE;
  } else if (isname(arg, (char *)pc_race_types[RACE_HARPY])) {
    return RACE_HARPY;
  } else if (isname(arg, (char *)pc_race_types[RACE_DRACONIAN])) {
    return RACE_DRACONIAN;
  } else if (isname(arg, (char *)pc_race_types[RACE_SCORPIUS])) {
    return RACE_SCORPIUS;
  } else if (isname(arg, (char *)pc_race_types[RACE_SPRITE])) {
    return RACE_SPRITE;
  } else if (isname(arg, (char *)pc_race_types[RACE_AVIADES])) {
    return RACE_AVIADES;
  } else return RACE_UNDEFINED;

}
/* To add any new bonuses or penalties to this table you *
 * need to make the number (1092) higher by the number of *
 * of additional lines you add                           */
const int race_class_bonus[1092][6] = {

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_DWARF,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_DWARF,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_DWARF,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_DWARF,	CLASS_GLADIATOR,		 0,  10,   0,   0 },
  {RACE_DWARF,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_DWARF,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_DWARF,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_DWARF,	CLASS_PALADIN,			 0,   0,   0,   0 },
  {RACE_DWARF,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_DWARF,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_ACROBAT,			  0,    0,    0,   0 },
  {RACE_DWARF,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_CHAMPION,			 15,   15,   15,   0 },
  {RACE_DWARF,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_COMBAT_MASTER,	 15,   15,   15,   0 },
  {RACE_DWARF,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_SAMURAI,			  0,    0,    0,   0 },
  {RACE_DWARF,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_MERCENARY,		 15,   15,   15,   0 },
  {RACE_DWARF,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_DWARF,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_DWARF,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_DWARF,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_DWARF,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_TROLL,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_TROLL,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_GLADIATOR,		 0,  10,   0,   0 },
  {RACE_TROLL,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_DARK_KNIGHT,		 0,   0,   0,   0 },
  {RACE_TROLL,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_TROLL,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_WARLOCK,			 15,   15,   15,   0 },
  {RACE_TROLL,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_REAPER,			 15,   15,   15,   0 },
  {RACE_TROLL,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_DARKMAGE,			  0,    0,    0,   0 },
  {RACE_TROLL,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_TROLL,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_TROLL,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_TROLL,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_TROLL,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_GIANT,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_GIANT,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_GLADIATOR,		 0,  10,   0,   0 },
  {RACE_GIANT,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_GIANT,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_GIANT,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_CRUSADER,			 15,   15,   15,   0 },
  {RACE_GIANT,	CLASS_BEASTMASTER,		 15,   15,   15,   0 },
  {RACE_GIANT,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_SAGE,				  0,    0,    0,   0 },
  {RACE_GIANT,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_GIANT,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_GIANT,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_GIANT,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_GIANT,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_KENDER,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_KENDER,	CLASS_THIEF,			 5,   5,   0,   0 },
  {RACE_KENDER,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_KENDER,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_KENDER,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_RANGER,			 15,   15,   15,   0 },
  {RACE_KENDER,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_STORR,			  0,    0,    0,   0 },
  {RACE_KENDER,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_ROGUE,			 15,   15,   15,   0 },
  {RACE_KENDER,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_KENDER,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_KENDER,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_KENDER,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_KENDER,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_GOBLIN,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_GOBLIN,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_THIEF,			 5,   5,   0,   0 },
  {RACE_GOBLIN,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_GOBLIN,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_GOBLIN,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_SHADOWMAGE,		 15,   15,   15,   0 },
  {RACE_GOBLIN,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_ACROBAT,			 15,   15,   15,   0 },
  {RACE_GOBLIN,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_PSIONIST,			  0,    0,    0,   0 },
  {RACE_GOBLIN,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_GOBLIN,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_GOBLIN,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_GOBLIN,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_GOBLIN,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_DRIDER,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_THIEF,			 5,   5,   0,   0 },
  {RACE_DRIDER,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_DRIDER,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_DARK_KNIGHT,		 0,   0,   0,   0 },
  {RACE_DRIDER,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_DRIDER,	CLASS_ASSASSIN,			 15,   15,   15,   0 },
  {RACE_DRIDER,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_REAPER,			  0,    0,    0,   0 },
  {RACE_DRIDER,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_MERCENARY,		 15,   15,   15,   0 },
  {RACE_DRIDER,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_DRIDER,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_DRIDER,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_DRIDER,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_DRIDER,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_AVIADES,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_AVIADES,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_AVIADES,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_MONK,				 3,   7,   0,   0 },
  {RACE_AVIADES,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_AVIADES,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_ARCANIC,			  0,    0,    0,   0 },
  {RACE_AVIADES,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_PSIONIST,			 15,   15,   15,   0 },
  {RACE_AVIADES,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_STORR,			 15,   15,   15,   0 },
  {RACE_AVIADES,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_AVIADES,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_AVIADES,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_AVIADES,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_AVIADES,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */ 
  {RACE_HUMAN,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_HUMAN,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_HUMAN,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_HUMAN,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_HUMAN,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_HUMAN,	CLASS_DARK_KNIGHT,		 0,   0,   0,   0 },
  {RACE_HUMAN,	CLASS_MONK,				 3,   7,   0,   0 },
  {RACE_HUMAN,	CLASS_PALADIN,			 0,   0,   0,   0 },
  {RACE_HUMAN,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_HUMAN,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_INQUISITOR,		 15,   15,   15,   0 },
  {RACE_HUMAN,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_FIANNA,			  0,    0,    0,   0 },
  {RACE_HUMAN,	CLASS_SHAMAN,			 15,   15,   15,   0 },
  {RACE_HUMAN,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_DESTROYER,		 15,   15,   15,   0 },
  {RACE_HUMAN,	CLASS_STYXIAN,			  0,    0,    0,   0 },
  {RACE_HUMAN,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_HUMAN,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_HUMAN,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_HUMAN,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_HUMAN,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */  
  {RACE_TRITON,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_TRITON,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_TRITON,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_MONK,				 3,   7,   0,   0 },
  {RACE_TRITON,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_TRITON,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_TRITON,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_TRITON,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_TRITON,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_TRITON,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_SPRITE,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_SPRITE,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_SPRITE,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_DRUID,			 5,   5,   0,   0 },
  {RACE_SPRITE,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_SPRITE,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_FORESTAL,			 15,   15,   15,   0 },
  {RACE_SPRITE,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_SAGE,				 15,   15,   15,   0 },
  {RACE_SPRITE,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_ROGUE,			  0,    0,    0,   0 },
  {RACE_SPRITE,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_SPRITE,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_SPRITE,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_SPRITE,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_SPRITE,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_ELF,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_ELF,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_ELF,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_ELF,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_ELF,	CLASS_DRUID,			 5,   5,   0,   0 },
  {RACE_ELF,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_ELF,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_ELF,	CLASS_PALADIN,			 0,   0,   0,   0 },
  {RACE_ELF,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_ELF,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_RANGER,			 15,   15,   15,   0 },
  {RACE_ELF,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_FORESTAL,			  0,    0,    0,   0 },
  {RACE_ELF,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_FIANNA,			 15,   15,   15,   0 },
  {RACE_ELF,	CLASS_SHAMAN,			 15,   15,   15,   0 },
  {RACE_ELF,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_ELF,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_ELF,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_ELF,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_ELF,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_WOLFEN,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_WOLFEN,	CLASS_DRUID,			 5,   5,   0,   0 },
  {RACE_WOLFEN,	CLASS_DARK_KNIGHT,		 0,   0,   0,   0 },
  {RACE_WOLFEN,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_WOLFEN,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_BEASTMASTER,		 15,   15,   15,   0 },
  {RACE_WOLFEN,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_DESTROYER,		  0,    0,    0,   0 },
  {RACE_WOLFEN,	CLASS_STYXIAN,			 15,   15,   15,   0 },
  {RACE_WOLFEN,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_WOLFEN,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_WOLFEN,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_WOLFEN,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_WOLFEN,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_GNOME,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_GNOME,	CLASS_CLERIC,			 3,   0,   7,   0 },
  {RACE_GNOME,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_GNOME,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_GNOME,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_SHADOWMAGE,		  0,    0,    0,   0 },
  {RACE_GNOME,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_ARCANIC,			 15,   15,   15,   0 },
  {RACE_GNOME,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_ROGUE,			 15,   15,   15,   0 },
  {RACE_GNOME,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_GNOME,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_GNOME,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_GNOME,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_GNOME,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_NYMPH,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_CLERIC,			 3,   0,   7,   0 },
  {RACE_NYMPH,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_NYMPH,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_NYMPH,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_NYMPH,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_STORR,			 15,   15,   15,   0 },
  {RACE_NYMPH,	CLASS_SAGE,				 15,   15,   15,   0 },
  {RACE_NYMPH,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_SHAMAN,			  0,    0,    0,   0 },
  {RACE_NYMPH,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_NYMPH,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_NYMPH,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_NYMPH,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_NYMPH,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_CENTAUR,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_CLERIC,			 3,   0,   7,   0 },
  {RACE_CENTAUR,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_CENTAUR,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_PALADIN,			 0,   0,   0,   0 },
  {RACE_CENTAUR,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_CENTAUR,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_CHAMPION,			  0,    0,    0,   0 },
  {RACE_CENTAUR,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_CRUSADER,			 15,   15,   15,   0 },
  {RACE_CENTAUR,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_TEMPLAR,			 15,   15,   15,   0 },
  {RACE_CENTAUR,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_CENTAUR,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_CENTAUR,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_CENTAUR,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_CENTAUR,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_PIXIE,	CLASS_SORCERER,			 0,   0,  10,   0 },
  {RACE_PIXIE,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_PIXIE,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_PIXIE,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_PIXIE,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_SHADOWMAGE,		 15,   15,   15,   0 },
  {RACE_PIXIE,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_RANGER,			  0,    0,    0,   0 },
  {RACE_PIXIE,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_FORESTAL,			 15,   15,   15,   0 },
  {RACE_PIXIE,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_PIXIE,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_PIXIE,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_PIXIE,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_PIXIE,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_ELDAR,	CLASS_SORCERER,			 0,   0,  10,   0 },
  {RACE_ELDAR,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_ELDAR,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_ELDAR,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_ELDAR,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_ELDAR,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_ELDAR,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_ELDAR,	CLASS_PALADIN,			 0,   0,   0,   0 },
  {RACE_ELDAR,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_ELDAR,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_ARCANIC,			 15,   15,   15,   0 },
  {RACE_ELDAR,	CLASS_MAGI,				 15,   15,   15,   0 },
  {RACE_ELDAR,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_PSIONIST,			 15,   15,   15,   0 },
  {RACE_ELDAR,	CLASS_TEMPLAR,			  0,    0,    0,   0 },
  {RACE_ELDAR,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_ELDAR,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_ELDAR,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_ELDAR,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_ELDAR,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_DROW,	CLASS_SORCERER,			 0,   0,  10,   0 },
  {RACE_DROW,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_DROW,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_DARK_KNIGHT,		 0,   0,   0,   0 },
  {RACE_DROW,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_DROW,	CLASS_ASSASSIN,			  0,    0,    0,   0 },
  {RACE_DROW,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_WARLOCK,			 15,   15,   15,   0 },
  {RACE_DROW,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_DARKMAGE,			 15,   15,   15,   0 },
  {RACE_DROW,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_DROW,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_DROW,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_DROW,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_DROW,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_VALKYRIE,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_VALKYRIE,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_VALKYRIE,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_PALADIN,			 5,   0,   5,   0 },
  {RACE_VALKYRIE,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_VALKYRIE,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_CRUSADER,			  0,    0,    0,   0 },
  {RACE_VALKYRIE,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_SAMURAI,			 15,   15,   15,   0 },
  {RACE_VALKYRIE,	CLASS_FIANNA,			 15,   15,   15,   0 },
  {RACE_VALKYRIE,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_VALKYRIE,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_VALKYRIE,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_VALKYRIE,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_VALKYRIE,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_NAGA,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_NAGA,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_NAGA,	CLASS_PALADIN,			 5,   0,   5,   0 },
  {RACE_NAGA,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_NAGA,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_CRUSADER,			  0,    0,    0,   0 },
  {RACE_NAGA,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_TEMPLAR,			 15,   15,   15,   0 },
  {RACE_NAGA,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_SAMURAI,			 15,   15,   15,   0 },
  {RACE_NAGA,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_NAGA,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_NAGA,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_NAGA,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_NAGA,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_GARGOYLE,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_GARGOYLE,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_GARGOYLE,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_DARK_KNIGHT,		-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_PALADIN,			 5,   0,   5,   0 },
  {RACE_GARGOYLE,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_GARGOYLE,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_WARLOCK,			  0,    0,    0,   0 },
  {RACE_GARGOYLE,	CLASS_CHAMPION,			 15,   15,   15,   0 },
  {RACE_GARGOYLE,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_MAGI,				 15,   15,   15,   0 },
  {RACE_GARGOYLE,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_GARGOYLE,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_GARGOYLE,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_GARGOYLE,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_GARGOYLE,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_HARPY,	CLASS_SORCERER,			 0,   0,   0,   0 },
  {RACE_HARPY,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_THIEF,			 0,   0,   0,   0 },
  {RACE_HARPY,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_DARK_KNIGHT,		 5,   0,   5,   0 },
  {RACE_HARPY,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_HARPY,	CLASS_ASSASSIN,			 15,   15,   15,   0 },
  {RACE_HARPY,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_INQUISITOR,		  0,    0,    0,   0 },
  {RACE_HARPY,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_DARKMAGE,			 15,   15,   15,   0 },
  {RACE_HARPY,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_HARPY,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_HARPY,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_HARPY,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_HARPY,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */            
  {RACE_DRACONIAN,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_CLERIC,			 0,   0,   0,   0 },
  {RACE_DRACONIAN,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_GLADIATOR,		-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_DRUID,			-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_DARK_KNIGHT,		 5,   0,   5,   0 },
  {RACE_DRACONIAN,	CLASS_MONK,				 0,   0,   0,   0 },
  {RACE_DRACONIAN,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_DRACONIAN,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_INQUISITOR,		 15,   15,   15,   0 },
  {RACE_DRACONIAN,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_REAPER,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_COMBAT_MASTER,	  0,    0,    0,   0 },
  {RACE_DRACONIAN,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_BEASTMASTER,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_DESTROYER,		 15,   15,   15,   0 },
  {RACE_DRACONIAN,	CLASS_STYXIAN,			-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_DRACONIAN,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_DRACONIAN,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_DRACONIAN,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_DRACONIAN,	CLASS_DEMON,			25,  25,  25,   0 },

/* RACE           CLASS              +HR  +DR  +SP  +AC */
  {RACE_SCORPIUS,	CLASS_SORCERER,			-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_CLERIC,			-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_THIEF,			-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_GLADIATOR,		 0,   0,   0,   0 },
  {RACE_SCORPIUS,	CLASS_DRUID,			 0,   0,   0,   0 },
  {RACE_SCORPIUS,	CLASS_DARK_KNIGHT,		 5,   0,   5,   0 },
  {RACE_SCORPIUS,	CLASS_MONK,				-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_PALADIN,			-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_BARD,				-5,  -5,  -5,   0 },
  {RACE_SCORPIUS,	CLASS_ASSASSIN,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_SHADOWMAGE,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_INQUISITOR,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_ACROBAT,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_RANGER,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_WARLOCK,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_CHAMPION,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_REAPER,			 15,   15,   15,   0 },
  {RACE_SCORPIUS,	CLASS_COMBAT_MASTER,	-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_CRUSADER,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_BEASTMASTER,		  0,    0,    0,   0 },
  {RACE_SCORPIUS,	CLASS_ARCANIC,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_MAGI,				-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_DARKMAGE,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_FORESTAL,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_PSIONIST,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_TEMPLAR,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_STORR,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_SAGE,				-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_SAMURAI,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_FIANNA,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_SHAMAN,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_MERCENARY,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_ROGUE,			-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_DESTROYER,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_STYXIAN,			 15,   15,   15,   0 },
  {RACE_SCORPIUS,	CLASS_WHOKNOWS5,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_WHOKNOWS6,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_WHOKNOWS7,		-15,  -15,  -15,   0 },
  {RACE_SCORPIUS,	CLASS_VAMPIRE,			25,  25,  25,   0 },
  {RACE_SCORPIUS,	CLASS_TITAN,			25,  25,  25,   0 },
  {RACE_SCORPIUS,	CLASS_SAINT,			25,  25,  25,   0 },
  {RACE_SCORPIUS,	CLASS_DEMON,			25,  25,  25,   0 }
};
