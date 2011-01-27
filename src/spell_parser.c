/* ************************************************************************
*   File: spell_parser.c                                Part of CircleMUD *
*  Usage: top-level magic routines; outside points of entry to magic sys. *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "dg_scripts.h"

/* 5 = skill, spell, chant, prayer, song */
struct abil_info_type abil_info[5][MAX_ABILITIES+1];

#define SINFO abil_info[ABT_SPELL][spellnum]
#define CINFO abil_info[ABT_CHANT][chant]
#define PINFO abil_info[ABT_PRAYER][prayer]

extern struct room_data *world;
extern struct message_list spell_messages[];
extern struct zone_data *zone_table;
int react_first_step(struct char_data *ch, struct char_data *victim);

/*
 * This arrangement is pretty stupid, but the number of skills is limited by
 * the playerfile.  We can arbitrarily increase the number of skills by
 * increasing the space in the playerfile. Meanwhile, this should provide
 * ample slots for skills.
 */

/* Spells */
char *spells[] = {
  "!RESERVED!",			/* 0 - reserved */
  "armor",			/* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "color spray",		/* 10 */
  "control weather",
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",		/* 20 */
  "detect poison",
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",		/* 30 */
  "locate object",
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",			/* 40 */
  "ventriloquate",
  "word of recall",
  "remove poison",
  "sense life",
  "animate dead",
  "dispel good",
  "group armor",
  "group cure light",
  "group recall",
  "infravision",		/* 50 */
  "waterwalk",
  "fly",
  "refresh",
  "dimensional door",
  "portal",
  "haste",
  "cure all",
  "firestorm",
  "regenerate",
  "create spring", /* 60 */
  "fire shield",
  "banish",
  "holy word",
  "demonfire",
  "silence",	/* 65 */
  "protection from good",
  "black breath",
  "chaos armor",
  "paralyze",
  "aegis",     /* 70 */
  "darkward",
  "wrath of god",
  "icestorm",
  "black dart",	
  "eyes of the dead", /* 75 */
  "breathe",
  "slow",
  "cause light",
  "cause critical",
  "lay hands",   /* 80 */
  "restore mana",
  "restore mega-mana",
  "remove invisibility",
  "dumbness",	
  "phantom armor", /* 85 */
  "spectral wings",
  "undead mount",
  "holy fury",
  "champion strength",	
  "holy mace",  /* 90 */
  "plague",
  "sonic wall",
  "ray of light", 
  "growth",	
  "identify",  /* 95 */
  "fire breath", 
  "gas breath", 
  "frost breath", 
  "acid breath",     
  "lightning breath", /* 100 */ 
  "mana economy",
  "stone hail",
  "flying fist",
  "shock sphere",
  "caustic rain", /* 105 */
  "wither",
  "meteor swarm",
  "balefire",
  "scorch",
  "immolate", /* 110 */
  "hellfire",
  "frost blade",
  "flame blade",
  "acid stream",
  "flame strike",  /* 115 */
  "finger of death",
  "scourge",
  "soul rip",
  "fellblade",
  "exterminate",  /* 120 */
  "knock",       
  "blades of light",
  "flames of purification",
  "prismatic beam",
  "** free **",   /* 125 */
  "holocaust",
  "cry for justice",
  "vim restore",
  "qi restore",
  "aria restore", /* 130 */
  "blur",
  "cone of cold",
  "opaque",
  "insomnia",
  "blazeward",    /* 135 */
  "indestructable aura",
  "entangle",
  "mist form",
  "soften flesh",	/* Crysist */
  "agitation",    /* 140 */
  "confuse",
  "clear sky",
  "animate corpse",
  "seal person",
  "potion-only sleep", /* 145 */
  "find familiar",
  "invisible stalker",
  "holy steed",
  "divine nimbus",
  "clairvoyance",  /* 150 */
  "summon dahlin",
  "summon fehzou",
  "summon necrolieu",
  "netherclaw",
  "conjure drake", /* 155 */
  "rain of fire",
  "treewalk dearthwood",
  "treewalk grasslands",
  "treewalk tanglewood",
  "treewalk elvenvalley", /* 160 */
  "treewalk darkwood",
  "treewalk sinister",
  "treewalk hermit",
  "treewalk crystalmir",
  "treewalk thewster", /* 165 */
  "treewalk kailaani",
  "treewalk littlesister",
  "sense spawn",
  "minor strength",
  "major strength",
  "minor dexterity",
  "dexterity",
  "minor intelligence",
  "intelligence",
  "minor wisdom",
  "wisdom",
  "minor constitution",
  "constitution",
  "minor charisma",
  "charisma",
  "minor luck",
  "luck",
  "snare",
/* End of spells */
  "\n"
};

/* Skills */
char *skills[] = {
  "!RESERVED!",			/* 0 - reserved */
  "backstab",			/* 1 */
  "bash",
  "hide",
  "kick",
  "pick lock",
  "punch",
  "rescue",
  "sneak",
  "steal",
  "track",			/* 10 */
  "second attack",
  "third attack",
  "fourth attack",
  "deathblow",
  "circle",	                /* 15 */
  "block",
  "dodge",
  "spy",
  "stun",
  "forage",                     /* 20 */
  "eyegouge",
  "dirtthrow",
  "bandage",
  "call of the wild",
  "trip",	                /* 25 */
  "retreat",
  "incognito",
  "sweep",
  "groinkick",
  "brain",	                /* 30 */
  "fifth attack",
  "doorbash",
  "hold breath",
  "switch opponent",
  "mother earth",	        /* 35 */
  "hone",
  "warcry",
  "disarm", 
  "gut",
  "knockout",	                /* 40 */
  "throw",
  "sixth attack",
  "seventh attack",
  "riposte",
  "ride",	                /* 45 */
  "tangle weed", 
  "run",
  "roundhouse",
  "kickflip",
  "chop",                       /* 50 */
  "shadow", 
  "shield punch", 
  "awe", 
  "backslash", 
  "uppercut",                   /* 55 */ 
  "charge", 
  "sleight of hand",
  "flail",
  "drown",
  "fury",                       /* 60 */
  "powerslash",
  "swarm",
  "elemental",
  "monsoon",
  "tackle",                     /* 65 */
  "pressure",
  "kiya",
  "headcut",
  "kamikaze",
  "headbut",                    /* 70 */
  "bearhug",
  "bodyslam",
  "befriend",
  "inject",
  "swat",                       /* 75 */
  "stomp",
  "trickpunch",
  "lowblow",
  "knee",
  "dig",                        /* 80 */
  "bury",
  "push",
  "brew",
  "scribe",
  "dual wield",                 /* 85 */
  "climb",
  "fairyfollower",
  "psychotic stabbing",
  "swordthrust",
  "berserk",                    /* 90 */
  "aura of nature",
  "fast healing",
  "haggle",
  "trample",
  "second backstab",            /* 95 */
  "third backstab",
  "fourth backstab",
  "mix",
  "beg",
  "taint fluid",                /* 100 */
  "vanish",
  "mesmerize",
  "hiss",
  "hover",
  "swoop",                      /* 105 */
  "tear",
  "rend",
  "find flesh",
  "sacred earth",
  "death call",                 /* 110 */
  "spit blood",
  "harness wind",
  "harness water",
  "harness fire",
  "harness earth",              /* 115 */
  "scorched earth",
  "absorb",
  "locator cloud",
  "summon elemental",
  "intimidate",                 /* 120 */
  "bellow",
  "gauge",
  "critical hit",
  "turn",
  "wheel",
  "quickdraw",
  "deceive",
  "dark allegiance",
  "absorb fire",
  "soulsuck",
  "bestow wings",
  "rub",
  "fog",
  "fog mind",
  "fog mist aura",
/* End of skills */
  "\n"
};

/* Chants */
char *chants[] = {
  "!RESERVED!",			/* 0 - reserved */
  "healing hand",
  "ancient protection",
  "find center",
  "omm",
  "yin xu",
  "yang xu",
  "mental barrier",
  "psionic drain",
  "lend health",
  "psychic fury",
  "balance",
/* End of chants */
  "\n"
};

/* Prayers */
char *prayers[] = {
  "!RESERVED!",			/* 0 - reserved */
  "cure light",
  "cure critical",
  "remove poison",
  "lift curse",
  "gods fury",                  /* 5 */
  "blessing",
  "dispel good",
  "dispel evil",
  "harm",
  "food",                       /* 10 */
  "water",
  "detect poison",
  "heavy skin",
  "guiding light",
  "sanctuary",
  "second sight",
  "infravision",
  "blindness",
  "cure blindness",
  "group armor",
  "group cure light",
  "earthquake",
  "call lightning",
  "haste",
  "summon",
  "refresh",
  "sacred shield",
  "heal",
  "pacify",
  "hand of balance",
  "elemental burst",
  "winds of pain",
  "spirit strike",
  "abomination",
  "winds of reckoning",
  "angel breath",
  "soul scourge",
  "plague",
  "weakened flesh",
  "drowse",
  "vitality",
  "reverse align",
  "retrieve corpse",
  "agitation",
  "holy water",
  "exorcism",
  "afterlife",
  "heavens beam",
  "divine shield",
  "ethereal",
  "choir",
  "enlightenment",
  "cure serious",
  "holy armor",
  "deflection",

/* End of prayers */
  "\n"
};

/* Songs */
char *songs[] = {
  "!RESERVED!",			/* 0 - reserved */
  "song of bravery",
  "honor",
  "heros march",
  "lullaby",
/* End of songs */
  "\n"
};

/* Fighting styles */
char *styles[] = {
  "!RESERVED!",			/* 0 - reserved */
  "snake style",
  "tiger style",
  "style crane",
  "style monkey",
  "rabid squirrel",
  "!UNUSED!",
  "\n"
};

/* OBJECT SPELLS AND NPC SPELLS/SKILLS */
char *specials[] = {
  "!RESERVED!",			/* 0 - reserved */
  "identify",
  "fire breath",
  "gas breath",
  "frost breath",
  "acid breath",
  "lightning breath",
  "\n"
};


struct syllable {
  char *org;
  char *new;
};


struct syllable syls[] = {
  {" ", " "},
  {"ar", "abra"},
  {"ate", "i"},
  {"cau", "kada"},
  {"blind", "nose"},
  {"bur", "mosa"},
  {"cu", "judi"},
  {"de", "oculo"},
  {"dis", "mar"},
  {"ect", "kamina"},
  {"en", "uns"},
  {"gro", "cra"},
  {"light", "dies"},
  {"lo", "hi"},
  {"magi", "kari"},
  {"mon", "bar"},
  {"mor", "zak"},
  {"move", "sido"},
  {"ness", "lacri"},
  {"ning", "illa"},
  {"per", "duda"},
  {"ra", "gru"},
  {"re", "candus"},
  {"son", "sabru"},
  {"tect", "infra"},
  {"tri", "cula"},
  {"ven", "nofo"},
  {"word of", "inject"},
  {"a", "i"}, {"b", "v"}, {"c", "q"}, {"d", "m"}, {"e", "o"}, {"f", "y"}, {"g", "t"},
  {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"}, {"m", "w"}, {"n", "b"},
  {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"}, {"u", "e"},
  {"v", "z"}, {"w", "x"}, {"x", "n"}, {"y", "l"}, {"z", "k"}, {"", ""}
};


int mag_rescost(struct char_data * ch, int num, byte type)
{
  if (GET_CLASS(ch) < CLASS_VAMPIRE)
    return MAX(abil_info[(int)type][num].res_max - (abil_info[(int)type][num].res_change *
    (GET_LEVEL(ch) - abil_info[(int)type][num].min_level[(int) GET_CLASS(ch)])),
     abil_info[(int)type][num].res_min);
  else
    return abil_info[(int)type][num].res_min;
}


/* say_spell erodes buf, buf1, buf2 */
void say_spell(struct char_data * ch, int spellnum, struct char_data * tch,
	            struct obj_data * tobj, int ranged)
{
  char lbuf[256];

  struct char_data *i;
  int j, ofs = 0;

  *buf = '\0';
  strcpy(lbuf, spells[spellnum]);

  while (*(lbuf + ofs)) {
    for (j = 0; *(syls[j].org); j++) {
      if (!strncmp(syls[j].org, lbuf + ofs, strlen(syls[j].org))) {
	strcat(buf, syls[j].new);
	ofs += strlen(syls[j].org);
      }
    }
  }
// world[ch->in_room].name
  if (tch != NULL && tch->in_room == ch->in_room) {
    if (tch == ch)
      sprintf(lbuf, "$n closes $s eyes and utters the words, '%%s'.");
    else if (ranged)
      sprintf(lbuf, "Out to %s: $n stares at $N and utters the words, '%%s'.", world[tch->in_room].name);
    else
      sprintf(lbuf, "$n stares at $N and utters the words, '%%s'.");
  } else if (tobj != NULL &&
	     ((tobj->in_room == ch->in_room) || (tobj->carried_by == ch)))
    sprintf(lbuf, "$n stares at $p and utters the words, '%%s'.");
  else
    sprintf(lbuf, "$n utters the words, '%%s'.");

  sprintf(buf1, lbuf, spells[spellnum]);
  sprintf(buf2, lbuf, buf);

  for (i = world[ch->in_room].people; i; i = i->next_in_room) {
    if (i == ch || i == tch || !i->desc || !AWAKE(i))
      continue;
    if (GET_CLASS(ch) == GET_CLASS(i))
      perform_act(buf1, ch, tobj, tch, i);
    else
      perform_act(buf2, ch, tobj, tch, i);
  }

  if (tch != NULL && tch != ch && tch->in_room == ch->in_room) {
    sprintf(buf1, "$n stares at you and utters the words, '%s'.",
	    GET_CLASS(ch) == GET_CLASS(tch) ? spells[spellnum] : buf);
    act(buf1, FALSE, ch, NULL, tch, TO_VICT);
  }
}

/* Don't forget to finish this!! */
// and add ranged messages at that time too!
void say_chant(struct char_data * ch, int chantnum, struct char_data * tch,
	            struct obj_data * tobj)
{
  act("$n chants.", FALSE, ch, tobj, tch, TO_ROOM);
}

/* Don't forget to finish this!! */
void say_prayer(struct char_data * ch, int prayernum, struct char_data * tch,
	            struct obj_data * tobj)
{
  act("You look to the heavens and pray.", FALSE, ch, tobj, tch, TO_CHAR);
  act("$n looks to the heavens and prays.", FALSE, ch, tobj, tch, TO_ROOM);
}

/* Don't forget to finish this!! */
void sing_song(struct char_data * ch, int songnum, struct char_data * tch,
	            struct obj_data * tobj)
{
/* Maybe have a list of phrases for each song that goes along with the
   song numbers?  Yeah.  I think so. */
  act("$n sings.", FALSE, ch, tobj, tch, TO_ROOM);
}

char *skill_name(int num)
{
  int i = 0;

  if (num <= 0) {
    if (num == -1)
      return "UNUSED";
    else
      return "UNDEFINED";
  }

  while (num && *spells[i] != '\n') {
    num--;
    i++;
  }

  if (*spells[i] != '\n')
    return spells[i];
  else
    return "UNDEFINED";
}

	 
int find_abil_num(char *name, char **list)
{
  int index = 0, ok;
  char *temp, *temp2;
  char first[256], first2[256];

  while (*list[++index] != '\n') {
    if (is_abbrev(name, list[index]))
      return index;

    ok = 1;
    temp = any_one_arg(list[index], first);
    temp2 = any_one_arg(name, first2);
    while (*first && *first2 && ok) {
      if (!is_abbrev(first2, first))
	ok = 0;
      temp = any_one_arg(temp, first);
      temp2 = any_one_arg(temp2, first2);
    }

    if (ok && !*first2)
      return index;
  }

  return -1;
}



/*
 * This function is the very heart of the entire magic system.  All
 * invocations of all types of magic -- objects, spoken and unspoken PC
 * and NPC spells, the works -- all come through this function eventually.
 * This is also the entry point for non-spoken or unrestricted spells.
 * Spellnum 0 is legal but silently ignored here, to make callers simpler.
 */
int call_magic(struct char_data * caster, struct char_data * cvict,
	       struct obj_data * ovict, int spellnum, byte type, 
               int level, int casttype, int ranged)
{
  int savetype;
  struct abil_info_type ab;

  ab = abil_info[(int)type][spellnum];

  if (spellnum < 1 || spellnum > TOP_SPELL_DEFINE)
    return 0;

  if (caster->nr != real_mobile(DG_CASTER_PROXY)) {
    if (ROOM_FLAGGED(caster->in_room, ROOM_NOMAGIC)) {
      send_to_char("Your magic fizzles out and dies.\r\n", caster);
      act("$n's magic fizzles out and dies.", FALSE, caster, 0, 0, TO_ROOM);
      return 0;
    }

    if (IS_SET_AR(ROOM_FLAGS(caster->in_room), ROOM_PEACEFUL) &&
        (ab.violent || IS_SET(ab.routines, MAG_DAMAGE))) {
      send_to_char("A flash of white light fills the room, dispelling your "
		 "violent magic!\r\n", caster);
      act("White light from no particular source suddenly fills the room, "
  	"then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
      return 0;
    }

  if (ranged && ROOM_FLAGGED(cvict->in_room, ROOM_NOMAGIC)) {
    send_to_char("Your magic fizzles out and dies.\r\n", caster);
    return 0;
  }

    if (ranged && IS_SET_AR(ROOM_FLAGS(cvict->in_room), ROOM_PEACEFUL) &&
        (ab.violent || IS_SET(ab.routines, MAG_DAMAGE))) {
      send_to_char("A flash of white light fills the room, dispelling your "
                 "violent magic!\r\n", caster);
      act("White light from no particular source suddenly fills the room, "
        "then vanishes.", FALSE, caster, 0, 0, TO_ROOM);
      return 0;
    }

    if (ranged) {
      if (ROOM_FLAGGED(caster->in_room, ROOM_NOPLINK) ||
         ROOM_FLAGGED(cvict->in_room, ROOM_NOPLINK) ||
         MOB_FLAGGED(cvict, MOB_NOPLINK)) {
        send_to_char("You can't get a good shot!\r\n", caster);
        return 0;
      }
    }  
  }
   
  /* determine the type of saving throw */
  switch (casttype) {
  case CAST_STAFF:
  case CAST_SCROLL:
  case CAST_POTION:
  case CAST_WAND:
    savetype = SAVING_ROD;
    break;
  case CAST_SPELL:
    savetype = SAVING_SPELL;
    break;
  default:
    savetype = SAVING_BREATH;
    break;
  }

  if (IS_SET(ab.routines, MAG_DAMAGE))
    mag_damage(level, caster, cvict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_AFFECTS))
    mag_affects(level, caster, cvict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_UNAFFECTS))
    mag_unaffects(level, caster, cvict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_POINTS))
    mag_points(level, caster, cvict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_ALTER_OBJS))
    mag_alter_objs(level, caster, ovict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_GROUPS))
    mag_groups(level, caster, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_MASSES))
    mag_masses(level, caster, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_AREAS))
    mag_areas(level, caster, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_SUMMONS))
    mag_summons(level, caster, ovict, spellnum, type, savetype);

  if (IS_SET(ab.routines, MAG_CREATIONS))
    mag_creations(level, caster, spellnum, type);

  if (IS_SET(ab.routines, MAG_MANUAL)) {
    switch (type) {
    case ABT_SPELL:
      switch (spellnum) {
      case SPELL_CHARM:		MANUAL_SPELL(spell_charm); break;
      case SPELL_CREATE_WATER:	MANUAL_SPELL(spell_create_water); break;
      case SPELL_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
      case SPELL_ENCHANT_WEAPON:  MANUAL_SPELL(spell_enchant_weapon); break;
      case SPELL_IDENTIFY:	MANUAL_SPELL(spell_identify); break;
      case SPELL_LOCATE_OBJECT:   MANUAL_SPELL(spell_locate_object); break;
      case SPELL_SUMMON:		MANUAL_SPELL(spell_summon); break;
      case SPELL_WORD_OF_RECALL:  MANUAL_SPELL(spell_recall); break;
      case SPELL_SENSE_SPAWN:      MANUAL_SPELL(spell_sense_spawn); break;
      case SPELL_DIMENSION_DOOR:  MANUAL_SPELL(spell_dimension_door); break;
      case SPELL_PORTAL:       	MANUAL_SPELL(spell_portal); break;
      case SPELL_TREEWALK_DEARTH:MANUAL_SPELL(spell_treewalk_dearthwood); break;
      case SPELL_TREEWALK_GLANDS:MANUAL_SPELL(spell_treewalk_grasslands); break;
      case SPELL_TREEWALK_TWOOD: MANUAL_SPELL(spell_treewalk_tanglewood); break;
      case SPELL_TREEWALK_VALLEY: MANUAL_SPELL(spell_treewalk_valley); break;
      case SPELL_TREEWALK_DWOOD:  MANUAL_SPELL(spell_treewalk_darkwood); break;
      case SPELL_TREEWALK_SINISTER:MANUAL_SPELL(spell_treewalk_sinister); break;
      case SPELL_TREEWALK_HERMIT: MANUAL_SPELL(spell_treewalk_hermit); break;
      case SPELL_TREEWALK_CMIR:   MANUAL_SPELL(spell_treewalk_crystalmir);break;
      case SPELL_TREEWALK_THEW:   MANUAL_SPELL(spell_treewalk_thewster); break;
      case SPELL_TREEWALK_KAI:    MANUAL_SPELL(spell_treewalk_kailaani); break;
      case SPELL_TREEWALK_SISTER: MANUAL_SPELL(spell_treewalk_little_sister); break;
      case SPELL_CREATESPRING:    MANUAL_SPELL(spell_createspring); break;
      case SPELL_RESTORE_MANA:    MANUAL_SPELL(spell_restore_mana); break;
      case SPELL_RESTORE_MEGAMANA:MANUAL_SPELL(spell_restore_megamana); break;
      case SPELL_VIM_RESTORE:     MANUAL_SPELL(spell_vim_restore); break;
      case SPELL_QI_RESTORE:      MANUAL_SPELL(spell_qi_restore); break;
      case SPELL_ARIA_RESTORE:    MANUAL_SPELL(spell_aria_restore); break;
      case SPELL_OPAQUE:          MANUAL_SPELL(spell_opaque); break;
      case SPELL_KNOCK:           MANUAL_SPELL(spell_knock); break; 

      }
      break;
    case ABT_CHANT:
      switch (spellnum) {
      }
      break; 
    case ABT_PRAYER:
      switch (spellnum) {
      case PRAY_WATER:         	MANUAL_SPELL(spell_create_water); break;
      case PRAY_DETECT_POISON:	MANUAL_SPELL(spell_detect_poison); break;
      case PRAY_GUIDING_LIGHT:  MANUAL_SPELL(spell_recall); break;
      case PRAY_SUMMON:         MANUAL_SPELL(spell_summon); break;
      case PRAY_PACIFY:         MANUAL_SPELL(pray_pacify); break;
      case PRAY_REVERSE_ALIGN:  MANUAL_SPELL(pray_reverse_align); break;
      case PRAY_RETRIEVE_CORPSE: MANUAL_SPELL(pray_retrieve_corpse); break;
      case PRAY_HOLY_WATER:     MANUAL_SPELL(pray_holy_water); break;
      case PRAY_CHOIR:          MANUAL_SPELL(pray_choir); break;
      }
      break; 
    case ABT_SONG:
      switch (spellnum) {
      }
      break; 
    }
  }
  return 1;
}

/*
 * mag_objectmagic: This is the entry-point for all magic items.  This should
 * only be called by the 'quaff', 'use', 'recite', etc. routines.
 *
 * For reference, object values 0-3:
 * staff  - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * wand   - [0]	level	[1] max charges	[2] num charges	[3] spell num
 * scroll - [0]	level	[1] spell num	[2] spell num	[3] spell num
 * potion - [0] level	[1] spell num	[2] spell num	[3] spell num
 *
 * Staves and wands will default to level 14 if the level is not specified;
 * the DikuMUD format did not specify staff and wand levels in the world
 * files (this is a CircleMUD enhancement).
 */

void mag_objectmagic(struct char_data * ch, struct obj_data * obj,
		          char *argument, byte type)
{
  int i, k;
  struct char_data *tch = NULL, *next_tch;
  struct obj_data *tobj = NULL;

  one_argument(argument, arg);

  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM,
                   ch, &tch, &tobj);
/*
  k = generic_find(arg, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
		   FIND_OBJ_EQUIP, ch, &tch, &tobj);
*/
  switch (GET_OBJ_TYPE(obj)) {
  case ITEM_STAFF:
    act("You tap $p three times on the ground.", FALSE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, 0, TO_ROOM);
    else
      act("$n taps $p three times on the ground.", FALSE, ch, obj, 0, TO_ROOM);

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
    } else {
      GET_OBJ_VAL(obj, 2)--;
      WAIT_STATE(ch, PULSE_VIOLENCE);
      for (tch = world[ch->in_room].people; tch; tch = next_tch) {
	next_tch = tch->next_in_room;
	if (ch == tch)
	  continue;
	if (GET_OBJ_VAL(obj, 0))
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), type,
		     GET_OBJ_VAL(obj, 0), CAST_STAFF, FALSE);
	else
	  call_magic(ch, tch, NULL, GET_OBJ_VAL(obj, 3), type,
		     DEFAULT_STAFF_LVL, CAST_STAFF, FALSE);
      }
    }
    break;
  case ITEM_WAND:
    if (k == FIND_CHAR_ROOM) {
      if (tch == ch) {
	act("You point $p at yourself.", FALSE, ch, obj, 0, TO_CHAR);
	act("$n points $p at $mself.", FALSE, ch, obj, 0, TO_ROOM);
      } else {
	act("You point $p at $N.", FALSE, ch, obj, tch, TO_CHAR);
	if (obj->action_description != NULL)
	  act(obj->action_description, FALSE, ch, obj, tch, TO_ROOM);
	else
	  act("$n points $p at $N.", TRUE, ch, obj, tch, TO_ROOM);
      }
    } else if (tobj != NULL) {
      act("You point $p at $P.", FALSE, ch, obj, tobj, TO_CHAR);
      if (obj->action_description != NULL)
	act(obj->action_description, FALSE, ch, obj, tobj, TO_ROOM);
      else
	act("$n points $p at $P.", TRUE, ch, obj, tobj, TO_ROOM);
    } else {
      act("At what should $p be pointed?", FALSE, ch, obj, NULL, TO_CHAR);
      return;
    }

    if (GET_OBJ_VAL(obj, 2) <= 0) {
      act("It seems powerless.", FALSE, ch, obj, 0, TO_CHAR);
      act("Nothing seems to happen.", FALSE, ch, obj, 0, TO_ROOM);
      return;
    }
    GET_OBJ_VAL(obj, 2)--;
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (GET_OBJ_VAL(obj, 0))
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), type,
		 GET_OBJ_VAL(obj, 0), CAST_WAND, FALSE);
    else
      call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, 3), type,
		 DEFAULT_WAND_LVL, CAST_WAND, FALSE);
    break;
  case ITEM_SCROLL:
    if (*arg) {
      if (!k) {
	act("There is nothing to here to affect with $p.", FALSE,
	    ch, obj, NULL, TO_CHAR);
	return;
      }
    } else
      tch = ch;

    act("You recite $p which dissolves.", TRUE, ch, obj, 0, TO_CHAR);
    if (obj->action_description)
      act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
    else
      act("$n recites $p.", FALSE, ch, obj, NULL, TO_ROOM);

    WAIT_STATE(ch, PULSE_VIOLENCE);
    for (i = 1; i < 4; i++)
      if (!(call_magic(ch, tch, tobj, GET_OBJ_VAL(obj, i), type,
		       GET_OBJ_VAL(obj, 0), CAST_SCROLL, FALSE)))
	break;

    if (obj != NULL)
      extract_obj(obj);
    break;
  case ITEM_RUNE:
    tch = ch;
    if(FIGHTING(ch)) { 
	send_to_char("You're kinda busy now....\r\n", ch);
        break;
    }
    if (zone_table[world[real_room(20100)].zone].number ==
        zone_table[world[IN_ROOM(ch)].zone].number) {
      send_to_char("Oops... looks like you are in the Grid.\r\n"
                   "You must fight a fair fight - to the death.\r\n\r\n"
                   "Thanks for the bug report, Dan.\r\n", ch);
      return;
    } 
    if (PLR_FLAGGED(ch, PLR_FISHING)) {
      send_to_char("You'll have to pack up your fishing gear first. Reelin.\r\n", ch);
      return;
    } else {
      act("$n recites the ancient text of $p and disappears.", 
          FALSE, ch, obj, NULL, TO_ROOM);
      act("You begin reciting the ancient text of $p and disappear.", 
          FALSE, ch, obj, NULL, TO_CHAR);
      char_from_room(ch);
      if (real_room(GET_OBJ_VAL(obj, 1))) {
        char_to_room(ch, real_room(GET_OBJ_VAL(obj, 1)));
      } else { 
        char_to_room(ch, real_room(3001)); 
      }
      act("$n appears without warning.", FALSE, ch, NULL, NULL, TO_ROOM);
      look_at_room(ch, 0);        
    }
       
  break;
  case ITEM_POTION:
    tch = ch;
    if (FIGHTING(ch) && !number(0,(int)(GET_LUCK(ch)/6))) {
      act("$n tries to quaff $p but it shatters!", FALSE, ch, obj,
          NULL, TO_ROOM);
      act("You try to quaff $p but it shatters!", FALSE, ch, obj,
          NULL, TO_CHAR);
    } else {
      act("You quaff $p.", FALSE, ch, obj, NULL, TO_CHAR);
      if (obj->action_description)
        act(obj->action_description, FALSE, ch, obj, NULL, TO_ROOM);
      else
        act("$n quaffs $p.", TRUE, ch, obj, NULL, TO_ROOM);

      for (i = 1; i < 4; i++)
        if (!(call_magic(ch, ch, NULL, GET_OBJ_VAL(obj, i), type,
  		       GET_OBJ_VAL(obj, 0), CAST_POTION, FALSE)))
        break;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (obj != NULL)
      extract_obj(obj);
    break;
  default:
    log("SYSERR: Unknown object_type in mag_objectmagic");
    break;
  }
}


/*
 * cast_spell is used generically to cast any spoken spell, assuming we
 * already have the target char/obj and spell number.  It checks all
 * restrictions, etc., prints the words, etc.
 *
 * Entry point for NPC casts.  Recommended entry point for spells cast
 * by NPCs via specprocs.
 */

int cast_spell(struct char_data * ch, struct char_data * tch,
	           struct obj_data * tobj, int spellnum, byte type, int ranged)
{
  char buf[128];
  struct abil_info_type *abi;
  char *msg[][5] = {
  /* Sleeping */
    {"\0",
     "You dream about great magical powers.\r\n",
     "You dream of ancient ceremonies.\r\n",
     "You dream about the gods.\r\n",
     "You dream about entertaining the king one day.\r\n"},
  /* Self-only */
    {"\0",
     "You can only cast this spell upon yourself!\r\n",
     "That chant can only be directed toward yourself.\r\n",
     "You may only use that prayer for yourself.\r\n",
     "That song may only be sung for yourself!\r\n"},
  /* Not self */
    {"\0",
     "You cannot cast this spell upon yourself!\r\n",
     "You cannot direct that chant toward yourself!\r\n",
     "You cannot say that prayer for yourself!\r\n",
     "You cannot sing that song for yourself!\r\n"},
  /* Group only* */
    {"\0",
     "You can't cast this spell if you're not in a group!\r\n",
     "You can't use that chant without a group.\r\n",
     "You can't use that prayer if you're not in a group.\r\n",
     "You can't sing that song if you're not in a group.\r\n"}
  };

  if (spellnum < 0 || spellnum > TOP_SPELL_DEFINE) {
    sprintf(buf, "SYSERR: cast_spell trying to call spellnum %d\n", spellnum);
    log(buf);
    return 0;
  }

  abi = &abil_info[(int)type][spellnum];

  if (GET_POS(ch) < abi->min_position) {
    switch (GET_POS(ch)) {
      case POS_SLEEPING:
      send_to_char(msg[0][(int)type], ch);
      break;
    case POS_RESTING:
      send_to_char("You cannot concentrate while resting.\r\n", ch);
      break;
    case POS_SITTING:
      send_to_char("You can't do this sitting!\r\n", ch);
      break;
    case POS_FIGHTING:
      send_to_char("Impossible!  You can't concentrate enough!\r\n", ch);
      break;
    default:
      send_to_char("You can't do much of anything like this!\r\n", ch);
      break;
    }
    return 0;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tch)) {
    send_to_char("You are afraid you might hurt your master!\r\n", ch);
    return 0;
  }
  if ((tch != ch) && IS_SET(abi->targets, TAR_SELF_ONLY)) {
    send_to_char(msg[1][(int)type], ch);
    return 0;
  }
  if ((tch == ch) && IS_SET(abi->targets, TAR_NOT_SELF)) {
    send_to_char(msg[2][(int)type], ch);
    return 0;
  }
  if (IS_SET(abi->routines, MAG_GROUPS) && !IS_AFFECTED(ch, AFF_GROUP)) {
    send_to_char(msg[3][(int)type], ch);
    return 0;
  }
  send_to_char(OK, ch);

  switch (type) {
  case ABT_SPELL:
    say_spell(ch, spellnum, tch, tobj, ranged ? TRUE : FALSE);
    break;
  case ABT_CHANT:
    say_chant(ch, spellnum, tch, tobj); //, ranged ? TRUE : FALSE);
    break;
  case ABT_PRAYER:
    say_prayer(ch, spellnum, tch, tobj); //, ranged ? TRUE : FALSE);
    break;
  case ABT_SONG:
    sing_song(ch, spellnum, tch, tobj); //, ranged ? TRUE : FALSE);
    break;
  }
  if (GET_CLASS(ch) >= CLASS_VAMPIRE)
	  return (call_magic(ch, tch, tobj, spellnum, type, 150, CAST_SPELL, ranged ? TRUE : FALSE));
  else
	  return (call_magic(ch, tch, tobj, spellnum, type, GET_LEVEL(ch), CAST_SPELL, ranged ? TRUE : FALSE));
}


/*
 * do_cast is the entry point for PC-casted spells.  It parses the arguments,
 * determines the spell number and finds a target, throws the die to see if
 * the spell can be cast, checks for sufficient mana and subtracts it, and
 * passes control to cast_spell().
 */

ACMD(do_cast)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, spellnum, i, target = 0, ranged = 0, prob;
  int dir = 0, is_in = ch->in_room, dir_check = 0;

  if (IS_NPC(ch))
    return;

  /* get: blank, spell name, target name */
  s = strtok(argument, "'");

  if (GET_CLASS(ch) == CLASS_CLERIC) {
    send_to_char("You are a cleric. Clerics 'pray' to their gods.\r\n", ch);
    return;
  }
  if (GET_CLASS(ch) == CLASS_MONK) {
    send_to_char("You are a monk. Monks 'chant' to harness their qi.\r\n", ch);
    return;
  }
  if (SECT(ch->in_room) == SECT_UNDERWATER && GET_RACE(ch) != RACE_TRITON)
  {
	  send_to_char("You can not cast spells underwater!\r\n", ch);
	  return;
  }
  if (s == NULL) {
    send_to_char("Cast what where?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Spell names must be enclosed in the Holy Magic Symbols: '\r\n", ch);
    return;
  }
  t = strtok(NULL, "\0");

  /* spellnum = search_block(s, spells, 0); */
  spellnum = find_abil_num(s, spells);

  if ((spellnum < 1) || (spellnum > MAX_ABILITIES)) {
    send_to_char("Cast what?!?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < SINFO.min_level[(int) GET_CLASS(ch)] && 
      GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You do not know that spell!\r\n", ch);
    return;
  }
  if (GET_SPELL(ch, spellnum) == 0) {
    send_to_char("You are unfamiliar with that spell.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(SINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    } // target not in caster's room? spell ranged? check surrounding rooms for vict.
    if (!target && (IS_SET(SINFO.targets, TAR_CHAR_ROOM)) &&
                       (IS_SET(SINFO.targets, TAR_RANGED))) {
      for (dir = 0;dir < NUM_OF_DIRS;dir++,ch->in_room = is_in) {
        if(!CAN_GO(ch,dir)) {
           continue;
        } else {
          ch->in_room = world[ch->in_room].dir_option[dir]->to_room;
           if ((tch = get_char_room_vis(ch, t)) != NULL) {
            target = TRUE;  ranged = TRUE;
            break;
          }
        }
      }
      ch->in_room = is_in;
    }


    if (!target && IS_SET(SINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(SINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(SINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(SINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the spell isn't violent, default to self */
    if (!target && IS_SET(SINFO.targets, TAR_CHAR_ROOM) &&
	!SINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "Upon %s should the spell be cast?\r\n",
	 IS_SET(SINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "who");
      send_to_char(buf, ch);
      return;
    }
  }


   if ((target && ranged && ROOM_FLAGGED(ch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && (MOB_FLAGGED(tch, MOB_NOPLINK)))) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }

   if (target && ranged && (dir_check = react_first_step(ch, tch)) < 0) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }
   
   if (target && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
       send_to_char("Your spell sputters and dies.\r\n", ch);
       return;
   }

   if (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOMAGIC)) {
       send_to_char("Your spell arcs forward, then fizzles.\r\n", ch);
       return;
   }

   if (target && SINFO.violent && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A brilliant flash of light dispels your violent magic!\r\n", ch);
       return;
   }
 
   if (target && SINFO.violent && ranged && ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A blinding sheet of light deflects your violent magic!\r\n", ch);
       return;
   }

  if (target && (tch == ch) && SINFO.violent) {
    send_to_char("You shouldn't cast that on yourself -- could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your spell!\r\n", ch);
    return;
  }

  mana = mag_rescost(ch, spellnum, ABT_SPELL);
  if (IS_AFFECTED(ch, AFF_MANA_ECONOMY)) {
    mana >>= 1;
  }
  if (IS_SORCERER(ch)) {
    mana = mana / 1.3;
  }

  if (GET_CLASS(ch) == CLASS_DRUID)
  {
	  if ((mana > 0) && (GET_VIM(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT))
	  {
		  send_to_char("You haven't the energy to cast that spell!\r\n", ch);
		  return;
	  }
  } else {
	  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT))
	  {
		  send_to_char("You haven't the energy to cast that spell!\r\n", ch);
		  return;
	  }
  }
  /* some classes have align restricts.  If they are too far
           away from their optimal align, their casting ability 
           becomes hampered. */
  if (GET_CLASS(ch) == CLASS_DARK_KNIGHT)
    prob = (int)((((int)(GET_ALIGNMENT(ch)/10) - 100) *
           GET_SPELL(ch, spellnum)) / -200); /* Evil is better! */
  else if (GET_CLASS(ch) == CLASS_PALADIN)
    prob = (int)((((int)(GET_ALIGNMENT(ch)/10) + 100) *
           GET_SPELL(ch, spellnum)) / 200); /* Good is better! */
  else
    prob = GET_SPELL(ch, spellnum);

  if (number(1, 200) > GET_SPELLPOWER(ch))
    prob += (int)GET_SPELLPOWER(ch)/5;

  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101) > prob && !GET_LEVEL(ch) >= LVL_IMMORT) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, spellnum, ABT_SPELL))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
	{
		if (GET_CLASS(ch) == CLASS_DRUID)
			GET_VIM(ch) = MAX(0, MIN(GET_MAX_VIM(ch), GET_VIM(ch) - (mana >> 1)));
		else
			GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
	}
    if (SINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
    improve_abil(ch, spellnum, ABT_SPELL);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, spellnum, ABT_SPELL, ranged ? TRUE : FALSE)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
	  if (mana > 0)
	  {
		  if (GET_CLASS(ch) == CLASS_DRUID)
			  GET_VIM(ch) = MAX(0, MIN(GET_MAX_VIM(ch), GET_VIM(ch) - (mana >> 1)));
		  else
			  GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
	  }
        if (GET_LEVEL(ch) < LVL_IMMORT)
          GET_EXP(ch) += (mana >> 2); /* Give a little exp for success */
          if (ranged)  // was the spell ranged? bring on the the vict!
            mob_reaction(ch, tch);
	}
  }
}


ACMD(do_chant)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int qi, chant, i, target = 0, ranged = 0, prob;
  int dir = 0, is_in = ch->in_room, dir_check = 0;

  if (IS_NPC(ch))
    return;

  /* get: blank, chant name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Chant what?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Chant names must be enclosed in the Holy Symbols: '\r\n",ch);
    return;
  }
  if (SECT(ch->in_room) == SECT_UNDERWATER && GET_RACE(ch) != RACE_TRITON)
  {
	  send_to_char("You can not chant underwater!\r\n", ch);
	  return;
  }
	
  t = strtok(NULL, "\0");

  chant = find_abil_num(s, chants);

  if ((chant < 1) || (chant > MAX_ABILITIES)) {
    send_to_char("Chant what?!?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < CINFO.min_level[(int) GET_CLASS(ch)] &&
      GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You do not know that chant!\r\n", ch);
    return;
  }
  if (!GET_CHANT(ch, chant)) {
    send_to_char("You are unfamiliar with that chant.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(CINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(CINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }
    if (!target && (IS_SET(CINFO.targets, TAR_CHAR_ROOM)) &&
                       (IS_SET(CINFO.targets, TAR_RANGED))) {
      for (dir = 0;dir < NUM_OF_DIRS;dir++,ch->in_room = is_in) {
        if(!CAN_GO(ch,dir)) {
           continue;
        } else {
          ch->in_room=world[ch->in_room].dir_option[dir]->to_room;
           if ((tch = get_char_room_vis(ch, t)) != NULL) {
            target = TRUE;  ranged = TRUE;
            break;
          }
        }
      }
      ch->in_room = is_in;
    }
    if (!target && IS_SET(CINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(CINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(CINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(CINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(CINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(CINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(CINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the prayer isn't violent, default to self */
    if (!target && IS_SET(CINFO.targets, TAR_CHAR_ROOM) &&
	!CINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "For %s shall you chant?\r\n",
	 IS_SET(CINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "whom");
      send_to_char(buf, ch);
      return;
    }
  }

   if ((target && ranged && ROOM_FLAGGED(ch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && (MOB_FLAGGED(tch, MOB_NOPLINK)))) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }
 
   if (target && ranged && (dir_check = react_first_step(ch, tch)) < 0) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }
 
   if (target && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
       send_to_char("A fine white ray of hot light seals your lips.\r\n", ch);
       return;
   }
 
   if (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOMAGIC)) {
       send_to_char("Powerful chaotic forces scatter your chant to the winds.\r\n", ch);
       return;
   }

   if (target && CINFO.violent && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A brilliant flash of light dispels your violent magic!\r\n", ch);
       return;
   }
 
   if (target && ranged && CINFO.violent && ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A blinding sheet of light deflects your violent magic!\r\n", ch);
       return;
   }


  if (target && (tch == ch) && CINFO.violent) {
    send_to_char("You shouldn't chant for that.  Could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your chant!\r\n", ch);
    return;
  }
  qi = mag_rescost(ch, chant, ABT_CHANT);
  if ((qi > 0) && (GET_QI(ch) < qi) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to perform that chant!\r\n", ch);
    return;
  }

  prob = GET_CHANT(ch, chant);
  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101) > prob && !GET_LEVEL(ch) >= LVL_IMMORT) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, chant, ABT_CHANT))
      send_to_char("You lost your concentration!\r\n", ch);
    if (qi > 0)
      GET_QI(ch) = MAX(0, MIN(GET_MAX_QI(ch), GET_QI(ch) - (qi >> 1)));
    if (CINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
    improve_abil(ch, chant, ABT_CHANT);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, chant, ABT_CHANT, ranged ? TRUE : FALSE)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (qi > 0) {
	GET_QI(ch) = MAX(0, MIN(GET_MAX_QI(ch), GET_QI(ch) - qi));
        if (GET_LEVEL(ch) < LVL_IMMORT)
          GET_EXP(ch) += (qi >> 2); /* Give a little exp for success */
          if (ranged)  // was the spell ranged? bring on the the vict! 
            mob_reaction(ch, tch);
      }
    }
  }
}

ACMD(do_pray)
{
  struct char_data *tch = NULL;
  struct obj_data *tobj = NULL;
  char *s, *t;
  int mana, prayer, i, target = 0, ranged = 0, prob;
  int dir = 0, is_in = ch->in_room, dir_check = 0;

  if (IS_NPC(ch))
    return;

  /* get: blank, prayer name, target name */
  s = strtok(argument, "'");

  if (s == NULL) {
    send_to_char("Pray for what?\r\n", ch);
    return;
  }
  s = strtok(NULL, "'");
  if (s == NULL) {
    send_to_char("Prayer names must be enclosed in the Holy Symbols: '\r\n",ch);
    return;
  }
  if (SECT(ch->in_room) == SECT_UNDERWATER && GET_RACE(ch) != RACE_TRITON)
  {
	  send_to_char("You can not pray underwater!\r\n", ch);
	  return;
  }
	
  t = strtok(NULL, "\0");

  prayer = find_abil_num(s, prayers);

  if ((prayer < 1) || (prayer > MAX_ABILITIES)) {
    send_to_char("Pray for what?!?\r\n", ch);
    return;
  }
  if (GET_LEVEL(ch) < PINFO.min_level[(int) GET_CLASS(ch)] &&
       GET_CLASS(ch) < CLASS_VAMPIRE) {
    send_to_char("You do not know that prayer!\r\n", ch);
    return;
  }
  if (!GET_PRAYER(ch, prayer)) {
    send_to_char("You are unfamiliar with that prayer.\r\n", ch);
    return;
  }
  /* Find the target */
  if (t != NULL) {
    one_argument(strcpy(arg, t), t);
    skip_spaces(&t);
  }
  if (IS_SET(PINFO.targets, TAR_IGNORE)) {
    target = TRUE;
  } else if (t != NULL && *t) {
    if (!target && (IS_SET(PINFO.targets, TAR_CHAR_ROOM))) {
      if ((tch = get_char_room_vis(ch, t)) != NULL)
	target = TRUE;
    }//ranged check for prayers.
    if (!target && (IS_SET(PINFO.targets, TAR_CHAR_ROOM)) &&
                       (IS_SET(PINFO.targets, TAR_RANGED))) {
      for (dir = 0;dir < NUM_OF_DIRS;dir++,ch->in_room = is_in) {
        if(!CAN_GO(ch,dir)) {
           continue;
        } else {
          ch->in_room=world[ch->in_room].dir_option[dir]->to_room;
           if ((tch = get_char_room_vis(ch, t)) != NULL) {
            target = TRUE;  ranged = TRUE;
            break;
          }
        }
      }
      ch->in_room = is_in;
    }
    if (!target && IS_SET(PINFO.targets, TAR_CHAR_WORLD))
      if ((tch = get_char_vis(ch, t)))
	target = TRUE;

    if (!target && IS_SET(PINFO.targets, TAR_OBJ_INV))
      if ((tobj = get_obj_in_list_vis(ch, t, ch->carrying)))
	target = TRUE;

    if (!target && IS_SET(PINFO.targets, TAR_OBJ_EQUIP)) {
      for (i = 0; !target && i < NUM_WEARS; i++)
	if (GET_EQ(ch, i) && isname(t, GET_EQ(ch, i)->name)) {
	  tobj = GET_EQ(ch, i);
	  target = TRUE;
	}
    }
    if (!target && IS_SET(PINFO.targets, TAR_OBJ_ROOM))
      if ((tobj = get_obj_in_list_vis(ch, t, world[ch->in_room].contents)))
	target = TRUE;

    if (!target && IS_SET(PINFO.targets, TAR_OBJ_WORLD))
      if ((tobj = get_obj_vis(ch, t)))
	target = TRUE;

  } else {			/* if target string is empty */
    if (!target && IS_SET(PINFO.targets, TAR_FIGHT_SELF))
      if (FIGHTING(ch) != NULL) {
	tch = ch;
	target = TRUE;
      }
    if (!target && IS_SET(PINFO.targets, TAR_FIGHT_VICT))
      if (FIGHTING(ch) != NULL) {
	tch = FIGHTING(ch);
	target = TRUE;
      }
    /* if no target specified, and the prayer isn't violent, default to self */
    if (!target && IS_SET(PINFO.targets, TAR_CHAR_ROOM) &&
	!PINFO.violent) {
      tch = ch;
      target = TRUE;
    }
    if (!target) {
      sprintf(buf, "For %s will you pray?\r\n",
	 IS_SET(PINFO.targets, TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_WORLD) ?
	      "what" : "whom");
      send_to_char(buf, ch);
      return;
    }
  }

   if ((target && ranged && ROOM_FLAGGED(ch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOPLINK)) ||
       (target && ranged && (MOB_FLAGGED(tch, MOB_NOPLINK)))) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }
 
   if (target && ranged && (dir_check = react_first_step(ch, tch)) < 0) {
       send_to_char("You can't get a good shot!\r\n", ch);
       return;
   }

   if (target && ROOM_FLAGGED(ch->in_room, ROOM_NOMAGIC)) {
       send_to_char("Your prayer cannot be heard from here.\r\n", ch);
       return;
   }
 
   if (target && ranged && ROOM_FLAGGED(tch->in_room, ROOM_NOMAGIC)) {
       send_to_char("Divine chaotic forces intercept your prayer.\r\n", ch);
       return;
   }
 
   if (target && PINFO.violent && ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A brilliant flash of light dispels your violent magic!\r\n", ch);
       return;
   }
 
   if (target && ranged && PINFO.violent && ROOM_FLAGGED(tch->in_room, ROOM_PEACEFUL)) {
       send_to_char("A blinding sheet of light deflects your violent magic!\r\n", ch);
       return;
   }


  if (target && (tch == ch) && PINFO.violent) {
    send_to_char("You shouldn't pray for that.  Could be bad for your health!\r\n", ch);
    return;
  }
  if (!target) {
    send_to_char("Cannot find the target of your prayer!\r\n", ch);
    return;
  }
  mana = mag_rescost(ch, prayer, ABT_PRAYER);
  if (IS_AFFECTED(ch, AFF_MANA_ECONOMY)) {
    mana >>= 1;
  } 
  if ((mana > 0) && (GET_MANA(ch) < mana) && (GET_LEVEL(ch) < LVL_IMMORT)) {
    send_to_char("You haven't the energy to pray for that!\r\n", ch);
    return;
  }

  prob = GET_PRAYER(ch, prayer);
  /* You throws the dice and you takes your chances.. 101% is total failure */
  if (number(0, 101) > prob && !GET_LEVEL(ch) >= LVL_IMMORT) {
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (!tch || !skill_message(0, ch, tch, prayer, ABT_PRAYER))
      send_to_char("You lost your concentration!\r\n", ch);
    if (mana > 0)
      GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - (mana >> 1)));
    if (PINFO.violent && tch && IS_NPC(tch))
      hit(tch, ch, TYPE_UNDEFINED);
    improve_abil(ch, prayer, ABT_PRAYER);
  } else { /* cast spell returns 1 on success; subtract mana & set waitstate */
    if (cast_spell(ch, tch, tobj, prayer, ABT_PRAYER, ranged ? TRUE : FALSE)) {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      if (mana > 0) {
	GET_MANA(ch) = MAX(0, MIN(GET_MAX_MANA(ch), GET_MANA(ch) - mana));
        if (GET_LEVEL(ch) < LVL_IMMORT)
          GET_EXP(ch) += (mana >> 2); /* Give a little exp for success */
          if (ranged)  // was the spell ranged? bring on the the vict! 
            mob_reaction(ch, tch);
      }
    }
  }
}


void abil_level(int type, int num, int class, int level)
{
  char buf[256];
  int bad = 0;

  if (num < 0 || num > MAX_ABILITIES) {
    sprintf(buf, "SYSERR: attempting assign to illegal num %d", num);
    log(buf);
    return;
  }

  if (class < 0 || class >= NUM_CLASSES) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal class %d",
	    skill_name(num), class);
    log(buf);
    bad = 1;
  }

  if (level < 1 || level > LVL_IMPL) {
    sprintf(buf, "SYSERR: assigning '%s' to illegal level %d",
	    skill_name(num), level);
    log(buf);
    bad = 1;
  }

  if (!bad)    
    abil_info[type][num].min_level[class] = level;
}



void rape_skills(struct char_data *ch, int level)
{  
  int i;

  for(i=1;i<MAX_ABILITIES;i++) {
    if(abil_info[ABT_SKILL][i].min_level[(int)GET_CLASS(ch)] > level) {
       set_ability(ch, ABT_SKILL, i, 0);
    }
    if(level < abil_info[ABT_PRAYER][i].min_level[(int)GET_CLASS(ch)]) {
       set_ability(ch, ABT_PRAYER, i, 0);
    }
    if(level < abil_info[ABT_CHANT][i].min_level[(int)GET_CLASS(ch)]) {
       set_ability(ch, ABT_CHANT, i, 0);
    }
    if(level < abil_info[ABT_SPELL][i].min_level[(int)GET_CLASS(ch)]) {
       set_ability(ch, ABT_SPELL, i, 0);
    }

  }
  sprintf(buf, "%s has been raped (%d)\r\n", GET_NAME(ch), GET_LEVEL(ch));
  log(buf);
}
   


/* Assign the abilities on boot up */
void abilo(int num, int max_res, int min_res, int res_change, int minpos,
	         int targets, int violent, int routines, byte type)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    abil_info[(int)type][num].min_level[i] = LVL_IMMORT;

  abil_info[(int)type][num].res_max = max_res;
  abil_info[(int)type][num].res_min = min_res;
  abil_info[(int)type][num].res_change = res_change;
  abil_info[(int)type][num].min_position = minpos;
  abil_info[(int)type][num].targets = targets;
  abil_info[(int)type][num].violent = violent;
  abil_info[(int)type][num].routines = routines;

}


void unused_abil(int spl, int type)
{
  int i;

  for (i = 0; i < NUM_CLASSES; i++)
    abil_info[type][spl].min_level[i] = LVL_IMPL + 1;

  abil_info[type][spl].res_max = 0;
  abil_info[type][spl].res_min = 0;
  abil_info[type][spl].res_change = 0;
  abil_info[type][spl].min_position = 0;
  abil_info[type][spl].targets = 0;
  abil_info[type][spl].violent = 0;
  abil_info[type][spl].routines = 0;

}

#define spello(spell, max, min, chg, pos, targ, v, r) \
         abilo(spell, max, min, chg, pos, targ, v, r, ABT_SPELL);
#define skillo(skill) abilo(skill, 0, 0, 0, 0, 0, 0, 0, ABT_SKILL);
#define chanto(chant, max, min, chg, pos, targ, v, r) \
         abilo(chant, max, min, chg, pos, targ, v, r, ABT_CHANT);
#define prayo(prayer, max, min, chg, pos, targ, v, r) \
         abilo(prayer, max, min, chg, pos, targ, v, r, ABT_PRAYER);
#define songo(song) abilo(song, 0, 0, 0, 0, 0, 0, 0, ABT_SONG);


/*
 * Arguments for abilo calls:
 *
 * num, maxres, minres, reschng, minpos, targets, violent?, routines, type.
 *
 * num:  Number of the ability.  Usually the symbolic name as defined in
 * spells.h (such as SPELL_HEAL).  The list depends on the type parm.
 *
 * maxres :  The maximum res this abil will take (i.e., the res it
 * will take when the player first gets the spell).
 *
 * minres :  The minimum res this abil will take, no matter how high
 * level the player is.
 *
 * reschng:  The change in res for the spell from level to level.  This
 * number should be positive, but represents the reduction in res cost as
 * the caster's level increases.
 *
 * minpos  :  Minimum position the caster must be in for the spell to work
 * (usually fighting or standing). targets :  A "list" of the valid targets
 * for the spell, joined with bitwise OR ('|').
 *
 * violent :  TRUE or FALSE, depending on if this is considered a violent
 * spell and should not be cast in PEACEFUL rooms or on yourself.  Should be
 * set on any spell that inflicts damage, is considered aggressive (i.e.
 * charm, curse), or is otherwise nasty.
 *
 * routines:  A list of magic routines which are associated with this spell
 * if the spell uses spell templates.  Also joined with bitwise OR ('|').
 *
 * type:  The type of ability.  skill, chant, spell, prayer, song, etc.
 *
 * See the CircleMUD documentation for a more detailed description of these
 * fields.
 */

void mag_assign_abils(void)
{
  int i, j;

  /* Do not change the loop below */
  for (j = 0; j < 5; j++)
    for (i = 1; i <= MAX_ABILITIES; i++)
      unused_abil(i, j);
  /* Do not change the loop above */

  spello(SPELL_AGITATION, 120, 80, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_ARMOR, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLUR, 45, 15, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);


  spello(SPELL_BLESS, 35, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_BLINDNESS, 35, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_BURNING_HANDS, 30, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CALL_LIGHTNING, 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CHARM, 75, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_MANUAL);

  spello(SPELL_CHILL_TOUCH, 30, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CLONE, 80, 65, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  spello(SPELL_CONFUSE, 190, 150, 3, POS_STANDING,
        TAR_IGNORE, TRUE, MAG_AREAS); 

  spello(SPELL_COLOR_SPRAY, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_CLEAR_SKY, 175, 105, 15, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_AFFECTS);

  spello(SPELL_CREATE_FOOD, 30, 5, 4, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS);

  spello(SPELL_CREATE_WATER, 30, 5, 4, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_CURE_BLIND, 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  spello(SPELL_CURE_CRITIC, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_CURE_LIGHT, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_SEAL, 150, 125, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_AFFECTS);

  spello(SPELL_CURSE, 80, 50, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_DETECT_ALIGN, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_INVIS, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_MAGIC, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DETECT_POISON, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DIMENSION_DOOR, 400, 350, 3, POS_STANDING,
	TAR_CHAR_WORLD , FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_DEARTH, 90, 60, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_GLANDS, 100, 80, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_TWOOD, 125, 100, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_VALLEY, 150, 100, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_DWOOD, 150, 100, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_SINISTER, 175, 150, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_HERMIT, 200, 150, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_CMIR, 250, 200, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_THEW, 350, 300, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_KAI, 400, 375, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_TREEWALK_SISTER, 450, 400, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_DIVINE_NIMBUS, 100, 76, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_DISPEL_EVIL, 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_DISPEL_GOOD, 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EARTHQUAKE, 40, 25, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ENCHANT_WEAPON, 240, 190, 10, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  spello(SPELL_ENERGY_DRAIN, 40, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_MANUAL);

  spello(SPELL_GROUP_ARMOR, 50, 30, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_FIREBALL, 40, 30, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLY, 30, 10, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_GROUP_HEAL, 80, 60, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  spello(SPELL_HARM, 75, 45, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HASTE, 60, 25, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MIST_FORM, 250, 225, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HEAL, 60, 40, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);

  spello(SPELL_INFRAVISION, 25, 10, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_INVISIBLE, 35, 25, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_KNOCK, 75, 50, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_NETHERCLAW, 500, 425, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
 
  spello(SPELL_LIGHTNING_BOLT, 30, 15, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LOCATE_OBJECT, 25, 20, 1, POS_STANDING,
	TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_MAGIC_MISSILE, 25, 5, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_POISON, 50, 20, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_OBJ_INV, TRUE, MAG_AFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_PORTAL, 100, 50, 4, POS_STANDING,
	TAR_CHAR_WORLD, FALSE, MAG_MANUAL);

  spello(SPELL_PROT_FROM_EVIL, 40, 10, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_REFRESH, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  spello(SPELL_REMOVE_CURSE, 45, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SANCTUARY, 110, 85, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SHOCKING_GRASP, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SLEEP, 40, 25, 5, POS_FIGHTING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_SLOW, 50, 15, 3, POS_STANDING,
	TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_MINOR_STRENGTH, 25, 20, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_STRENGTH, 35, 30, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MAJOR_STRENGTH, 55, 50, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_DEXTERITY, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_DEXTERITY, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_INTELLIGENCE, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_INTELLIGENCE, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_WISDOM, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_WISDOM, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_CONSTITUTION, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CONSTITUTION, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_CHARISMA, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CHARISMA, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_MINOR_LUCK, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_LUCK, 25, 20, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_SOFTEN_FLESH, 100, 50, 5, POS_STANDING,  /* Crysist */
  	TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_SUMMON, 375, 225, 10, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  spello(SPELL_WORD_OF_RECALL, 40, 20, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_REMOVE_POISON, 40, 8, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE,
        MAG_UNAFFECTS | MAG_ALTER_OBJS);

  spello(SPELL_SENSE_LIFE, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SENSE_SPAWN, 100, 90, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_MANUAL);

  spello(SPELL_WATERWALK, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CUREALL, 500, 500, 0, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);
  
  spello(SPELL_PROTFROMGOOD, 40, 10, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_DARKWARD, 70, 45, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_AEGIS, 50, 25, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_FIRESHIELD, 65, 40, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CHAOSARMOR, 70, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_REGENERATE, 120, 60, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_DEMONFIRE, 80, 50, 0, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_BLACKDART, 50, 30, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_BLACKBREATH, 40, 20, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_WRATHOFGOD, 550, 550, 0, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EYESOFTHEDEAD, 50, 25, 1, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_FIRESTORM, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FIRE_BREATH, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_GAS_BREATH, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_FROST_BREATH, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ACID_BREATH, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_LIGHTNING_BREATH, 75, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);

  spello(SPELL_ICESTORM, 80, 35, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS);
 
  spello(SPELL_CREATESPRING, 70, 60, 1, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_CREATIONS);

/*  finish this spell when conjured monsters are enabled

  spello(SPELL_BANISH, 150, 125, 1, POS_STANDING,

*/
  spello(SPELL_SILENCE, 70, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_PARALYZE, 180, 150, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

/* Finish this when I decide what its gonna do

  spello(SPELL_HOLYWORD, 250, 225, 1, POS_STANDING,

*/

  spello(SPELL_BREATHE,  20, 15, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_CAUSE_LIGHT, 30, 10, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CAUSE_CRITIC, 30, 10, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_LAY_HANDS, 40, 20, 4, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_REMOVE_INVIS, 53, 45, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_ALTER_OBJS);

  spello(SPELL_DUMBNESS, 30, 10, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS);

  spello(SPELL_PHANTOM_ARMOR, 53, 30, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_SPECTRAL_WINGS, 50, 30, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_UNDEAD_MOUNT, 175, 175, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_HOLY_FURY, 100, 70, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_CHAMPION_STRENGTH, 150, 70, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_HOLY_MACE, 30, 10, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_RAY_OF_LIGHT, 50, 20, 3, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_GROWTH, 350, 200, 4, POS_FIGHTING,
         TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_IDENTIFY, 30, 10, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_RESTORE_MANA, 0, 0, 0, 0,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_RESTORE_MEGAMANA, 0, 0, 0, 0,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_PLAGUE, 1, 1, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  spello(SPELL_SONIC_WALL, 80, 60, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS | MAG_DAMAGE);

  spello(SPELL_MANA_ECONOMY, 120, 90, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  spello(SPELL_STONE_HAIL, 40, 32, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLYING_FIST, 55, 47, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SHOCK_SPHERE, 75, 58, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CAUSTIC_RAIN, 85, 70, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_WITHER, 100, 85, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_METEOR_SWARM, 80, 50, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_BALEFIRE, 90, 55, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SCORCH, 100, 60, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_IMMOLATE, 110, 65, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_HELLFIRE, 44, 35, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FROST_BLADE, 60, 50, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLAME_BLADE, 80, 42, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_ACID_STREAM, 90, 50, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLAME_STRIKE, 100, 55, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FINGER_OF_DEATH, 110, 60, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SCOURGE, 120, 70, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_SOUL_RIP, 130, 80, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FELLBLADE, 140, 90, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_EXTERMINATE, 150, 95, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_BLADE_OF_LIGHT, 100, 95, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_FLAMES_PURE, 110, 99, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_PRISMATIC_BEAM, 120, 110, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE | MAG_AFFECTS);

  spello(SPELL_HOLOCAUST, 90, 60, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_CRY_FOR_JUSTICE, 100, 65, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  spello(SPELL_VIM_RESTORE, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_QI_RESTORE, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_ARIA_RESTORE, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_MANUAL);

  spello(SPELL_CONE_COLD, 500, 425, 2, POS_STANDING,
        TAR_CHAR_ROOM, TRUE, MAG_AFFECTS);

  spello(SPELL_OPAQUE, 103, 85, 5, POS_STANDING,
        TAR_OBJ_INV, FALSE, MAG_MANUAL);

  spello(SPELL_INSOMNIA, 215, 176, 2, POS_STANDING,
         TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_BLAZEWARD, 110, 85, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_POTION_SLEEP, 110, 85, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_INDESTR_AURA, 110, 85, 5, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  spello(SPELL_ENTANGLE, 100, 40, 1, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_POINTS);

  spello(SPELL_ANIMATE_CORPSE, 575, 455, 10, POS_STANDING,
        TAR_OBJ_ROOM, FALSE, MAG_SUMMONS);

  spello(SPELL_HOLY_STEED, 175, 175, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_FIND_FAMILIAR, 75, 75, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_INVIS_STALKER, 75, 75, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_SUMMON_DHALIN, 300, 225, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_SUMMON_FEHZOU, 345, 290, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_SUMMON_NECROLIEU, 490, 450, 5, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);
 
  spello(SPELL_CONJURE_FIREDRAKE, 600, 500, 2, POS_STANDING,
        TAR_IGNORE, FALSE, MAG_SUMMONS);

  spello(SPELL_RAIN_OF_FIRE, 95, 70, 2, POS_FIGHTING,
        TAR_IGNORE, TRUE, MAG_AREAS); 

  spello(SPELL_SNARE, 50, 40, 1, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  /*
   * Chant setup information.
   */
  chanto(CHANT_HEALING_HAND, 15, 7, 1, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  chanto(CHANT_ANCIENT_PROT, 30, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  chanto(CHANT_FIND_CENTER, 30, 20, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  chanto(CHANT_OMM, 30, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  chanto(CHANT_YIN_XU, 40, 35, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  chanto(CHANT_YANG_XU, 50, 40, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  chanto(CHANT_MENTAL_BARRIER, 40, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  chanto(CHANT_PSIONIC_DRAIN, 25, 20, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_AFFECTS | MAG_DAMAGE);

  chanto(CHANT_LEND_HEALTH, 24, 8, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_POINTS);

  chanto(CHANT_PSYCHIC_FURY, 35, 30, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_DAMAGE);

  chanto(CHANT_BALANCE, 50, 40, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);


  /*
   * Prayer setup information.
   */
  prayo(PRAY_CURE_LIGHT, 15, 7, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  prayo(PRAY_CURE_SERIOUS, 30, 10, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  prayo(PRAY_CURE_CRITICAL, 45, 15, 2, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  prayo(PRAY_REMOVE_POISON, 40, 8, 4, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE,
        MAG_UNAFFECTS | MAG_ALTER_OBJS);

  prayo(PRAY_HOLY_ARMOR, 70, 50, 2, POS_STANDING,
        TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  prayo(PRAY_HOLY_WATER, 70, 50, 10, POS_STANDING,
        TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  prayo(PRAY_LIFT_CURSE, 45, 25, 5, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_UNAFFECTS | MAG_ALTER_OBJS);

  prayo(PRAY_GODS_FURY, 100, 60, 0, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_BLESSING, 35, 5, 3, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV, FALSE, MAG_AFFECTS | MAG_ALTER_OBJS);

  prayo(PRAY_DISPEL_GOOD, 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_DISPEL_EVIL, 40, 25, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_HARM, 75, 45, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_FOOD, 30, 5, 4, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_CREATIONS);

  prayo(PRAY_WATER, 30, 5, 4, POS_STANDING,
	TAR_OBJ_INV | TAR_OBJ_EQUIP, FALSE, MAG_MANUAL);

  prayo(PRAY_DETECT_POISON, 15, 5, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, FALSE, MAG_MANUAL);

  prayo(PRAY_HEAVY_SKIN, 30, 15, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  prayo(PRAY_GUIDING_LIGHT, 30, 10, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  prayo(PRAY_SANCTUARY, 310, 285, 10, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_DEFLECT, 400, 290, 10, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_SECOND_SIGHT, 20, 10, 2, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_INFRAVISION, 25, 10, 1, POS_STANDING,
	TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_BLINDNESS, 35, 25, 1, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, FALSE, MAG_AFFECTS);

  prayo(PRAY_CURE_BLIND, 30, 5, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_UNAFFECTS);

  prayo(PRAY_GROUP_ARMOR, 70, 50, 2, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  prayo(PRAY_GROUP_HEAL, 80, 60, 5, POS_STANDING,
	TAR_IGNORE, FALSE, MAG_GROUPS);

  prayo(PRAY_EARTHQUAKE, 60, 25, 3, POS_FIGHTING,
	TAR_IGNORE, TRUE, MAG_AREAS);

  prayo(PRAY_CALL_LIGHTNING, 50, 35, 3, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_HASTE, 85, 55, 3, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  prayo(PRAY_SUMMON, 75, 50, 3, POS_STANDING,
	TAR_CHAR_WORLD | TAR_NOT_SELF, FALSE, MAG_MANUAL);

  prayo(PRAY_REFRESH, 45, 25, 2, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS);

  prayo(PRAY_SACRED_SHIELD, 210, 185, 5, POS_STANDING,
	TAR_CHAR_ROOM, FALSE, MAG_AFFECTS);

  prayo(PRAY_HEAL, 60, 40, 3, POS_FIGHTING,
	TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS);

  prayo(PRAY_PACIFY, 140, 100, 6, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, FALSE, MAG_MANUAL);

  prayo(PRAY_HAND_BALANCE, 70, 56, 4, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_ELEMENTAL_BURST, 35, 25, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_WINDS_PAIN, 45, 35, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_SPIRIT_STRIKE, 90, 40, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_ABOMINATION, 100, 50, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_WINDS_RECKONING, 110, 60, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF, TRUE, MAG_DAMAGE);

  prayo(PRAY_ANGEL_BREATH, 130, 70, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_SOUL_SCOURGE, 150, 75, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_PLAGUE, 140, 80, 2, POS_FIGHTING,
	TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_WEAKENED_FLESH, 120, 90, 4, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  prayo(PRAY_DROWSE, 200, 170, 2, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_AFFECTS);

  prayo(PRAY_VITALITY, 100,  80, 2, POS_FIGHTING,
        TAR_CHAR_ROOM, FALSE, MAG_POINTS | MAG_AFFECTS | MAG_UNAFFECTS); 

  prayo(PRAY_REVERSE_ALIGN, 200, 165, 2, POS_STANDING,
        TAR_CHAR_ROOM | TAR_NOT_SELF, FALSE, MAG_MANUAL); 

  prayo(PRAY_RETRIEVE_CORPSE, 925, 900, 1, POS_STANDING,
        TAR_OBJ_WORLD, FALSE, MAG_MANUAL);

  prayo(PRAY_EXORCISM, 300, 200, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, TRUE, MAG_DAMAGE);

  prayo(PRAY_AFTERLIFE, 800, 725, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_HEAVENSBEAM, 300, 150, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_NOT_SELF | TAR_FIGHT_VICT, FALSE, MAG_AFFECTS | MAG_DAMAGE);

  prayo(PRAY_DIVINESHIELD, 400, 325, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_ETHEREAL, 250, 225, 3, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);

  prayo(PRAY_CHOIR, 750, 525, 5, POS_FIGHTING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_MANUAL);

  prayo(PRAY_ENLIGHTENMENT, 500, 350, 5, POS_STANDING,
        TAR_CHAR_ROOM | TAR_SELF_ONLY, FALSE, MAG_AFFECTS);
  /*
   * Declaration of skills - this actually doesn't do anything except
   * set it up so that immortals can use these skills by default.  The
   * min level to use the skill for other classes is set up in class.c.
   */

  skillo(SKILL_BACKSTAB);
  skillo(SKILL_BASH);
  skillo(SKILL_HIDE);
  skillo(SKILL_KICK);
  skillo(SKILL_PICK_LOCK);
  skillo(SKILL_PUNCH);
  skillo(SKILL_RESCUE);
  skillo(SKILL_SNEAK);
  skillo(SKILL_STEAL);
  skillo(SKILL_TRACK);
  skillo(SKILL_SECOND_ATTACK);
  skillo(SKILL_THIRD_ATTACK);
  skillo(SKILL_FOURTH_ATTACK);
  skillo(SKILL_DEATHBLOW);
  skillo(SKILL_CIRCLE);
  skillo(SKILL_BLOCK);
  skillo(SKILL_DODGE);
  skillo(SKILL_SPY);
  skillo(SKILL_STUN);
  skillo(SKILL_FORAGE);
  skillo(SKILL_EYEGOUGE);
  skillo(SKILL_BANDAGE);
  skillo(SKILL_CALL_WILD);
  skillo(SKILL_TRIP);
  skillo(SKILL_RETREAT);
  skillo(SKILL_INCOGNITO);
  skillo(SKILL_SWEEP);
  skillo(SKILL_GROINKICK);
  skillo(SKILL_BRAIN);
  skillo(SKILL_FIFTH_ATTACK);
  skillo(SKILL_DOORBASH);
  skillo(SKILL_HOLD_BREATH);
  skillo(SKILL_SWITCH);
  skillo(SKILL_MOTHEREARTH);
  skillo(SKILL_HONE);
  skillo(SKILL_WARCRY);
  skillo(SKILL_DISARM);
  skillo(SKILL_GUT);
  skillo(SKILL_KNOCKOUT);
  skillo(SKILL_THROW);
  skillo(SKILL_SIXTH_ATTACK);
  skillo(SKILL_SEVENTH_ATTACK);
  skillo(SKILL_RIDE);
  skillo(SKILL_TANGLEWEED);
  skillo(SKILL_RUN);
  skillo(SKILL_ROUNDHOUSE);
  skillo(SKILL_KICKFLIP);
  skillo(SKILL_CHOP);
  skillo(SKILL_SHADOW);
  skillo(SKILL_SHIELDPUNCH);
  skillo(SKILL_AWE);
  skillo(SKILL_BACKSLASH);
  skillo(SKILL_UPPERCUT);
  skillo(SKILL_CHARGE);
  skillo(SKILL_SLEIGHT);
  skillo(SKILL_FLAIL);
  skillo(SKILL_DROWN);
  skillo(SKILL_BLADED_FURY);
  skillo(SKILL_POWERSLASH);
  skillo(SKILL_SWARM);
  skillo(SKILL_ELEMENTAL);
  skillo(SKILL_MONSOON);
  skillo(SKILL_TACKLE);
  skillo(SKILL_PRESSURE);
  skillo(SKILL_KI);
  skillo(SKILL_HEADCUT);
  skillo(SKILL_KAMIKAZE);
  skillo(SKILL_HEADBUT);
  skillo(SKILL_BEARHUG);
  skillo(SKILL_BODYSLAM);
  skillo(SKILL_BEFRIEND);
  skillo(SKILL_INSET);
  skillo(SKILL_TRICKPUNCH);
  skillo(SKILL_LOWBLOW);
  skillo(SKILL_KNEE);
  skillo(SKILL_DIG);
  skillo(SKILL_BURY);
  skillo(SKILL_PUSH);
  skillo(SKILL_DUAL_WIELD);
  skillo(SKILL_CLIMB);
  skillo(SKILL_FAIRYFOLLOWER);
  skillo(SKILL_PSYCHOTIC_STABBING);
  skillo(SKILL_SWORD_THRUST);
  skillo(SKILL_BERSERK);
  skillo(SKILL_AURA_OF_NATURE);
  skillo(SKILL_FAST_HEAL);
  skillo(SKILL_HAGGLE);
  skillo(SKILL_TRAMPLE);
  skillo(SKILL_SECOND_BSTAB);
  skillo(SKILL_THIRD_BSTAB);
  skillo(SKILL_FOURTH_BSTAB);
  skillo(SKILL_MIX);
  skillo(SKILL_TAINT_FLUID);
  skillo(SKILL_VANISH);
  skillo(SKILL_MESMERIZE);
  skillo(SKILL_HISS);
  skillo(SKILL_HOVER);
  skillo(SKILL_SWOOP);
  skillo(SKILL_TEAR);
  skillo(SKILL_REND);
  skillo(SKILL_FIND_FLESH);
  skillo(SKILL_SACRED_EARTH);
  skillo(SKILL_DEATH_CALL);
  skillo(SKILL_SPIT_BLOOD);
  skillo(SKILL_HARNESS_WIND);
  skillo(SKILL_HARNESS_WATER);
  skillo(SKILL_HARNESS_FIRE);
  skillo(SKILL_HARNESS_EARTH);
  skillo(SKILL_SCORCH);
  skillo(SKILL_ABSORB);
  skillo(SKILL_CLOUD);
  skillo(SKILL_SUMMON_ELEM);
  skillo(SKILL_INTIMIDATE);
  skillo(SKILL_BELLOW);
  skillo(SKILL_RIPOSTE);
  skillo(SKILL_CRITICAL_HIT);
  skillo(SKILL_GAUGE);
  skillo(SKILL_TURN);
  skillo(SKILL_WHEEL);
  skillo(SKILL_QUICKDRAW);
  skillo(SKILL_RUB);
  skillo(SKILL_FOG_MIND);
  skillo(SKILL_FOG);
  skillo(SKILL_FOG_MIST_AURA);

/* Styles will need to be finished later.
  skillo(STYLE_SNAKE);
  skillo(STYLE_TIGER);
  skillo(STYLE_CRANE);
  skillo(STYLE_MONKEY);
  skillo(STYLE_RABID_SQUIRREL);
*/

}

