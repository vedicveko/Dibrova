/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define DEFAULT_STAFF_LVL	12
#define DEFAULT_WAND_LVL	12

#define CAST_UNDEFINED	-1
#define CAST_SPELL	0
#define CAST_POTION	1
#define CAST_WAND	2
#define CAST_STAFF	3
#define CAST_SCROLL	4

/* Ability types (sometimes used interchangably with the similar 
   SCMD's, so keep them in synch with interpreter.h. */
#define ABT_SKILL  0
#define ABT_SPELL  1
#define ABT_CHANT  2
#define ABT_PRAYER 3
#define ABT_SONG   4
#define ABT_WEAPON -1 /* Special case */

#define MAG_DAMAGE	(1 << 0)
#define MAG_AFFECTS	(1 << 1)
#define MAG_UNAFFECTS	(1 << 2)
#define MAG_POINTS	(1 << 3)
#define MAG_ALTER_OBJS	(1 << 4)
#define MAG_GROUPS	(1 << 5)
#define MAG_MASSES	(1 << 6)
#define MAG_AREAS	(1 << 7)
#define MAG_SUMMONS	(1 << 8)
#define MAG_CREATIONS	(1 << 9)
#define MAG_MANUAL	(1 << 10)


#define TYPE_UNDEFINED               -1
#define TYPE_RANGED                  -2

#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO -- RESERVED */

/* Spell numbers */
#define SPELL_UNUSED                  0 /* RESERVED - DON'T USE */
#define SPELL_ARMOR                   1 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_TELEPORT                2 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLESS                   3 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BLINDNESS               4 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_BURNING_HANDS           5 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CALL_LIGHTNING          6 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHARM                   7 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CHILL_TOUCH             8 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CLONE                   9 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_COLOR_SPRAY            10 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CONTROL_WEATHER        11 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_FOOD            12 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CREATE_WATER           13 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_BLIND             14 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_CRITIC            15 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURE_LIGHT             16 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_CURSE                  17 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_ALIGN           18 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_INVIS           19 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_MAGIC           20 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DETECT_POISON          21 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_EVIL            22 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_EARTHQUAKE             23 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENCHANT_WEAPON         24 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ENERGY_DRAIN           25 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FIREBALL               26 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HARM                   27 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_HEAL                   28 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INVISIBLE              29 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LIGHTNING_BOLT         30 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_LOCATE_OBJECT          31 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_MAGIC_MISSILE          32 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_POISON                 33 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_PROT_FROM_EVIL         34 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_CURSE           35 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SANCTUARY              36 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SHOCKING_GRASP         37 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SLEEP                  38 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_STRENGTH               39 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SUMMON                 40 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_VENTRILOQUATE          41 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WORD_OF_RECALL         42 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_REMOVE_POISON          43 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_SENSE_LIFE             44 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_ANIMATE_DEAD	     45 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_DISPEL_GOOD	     46 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_ARMOR	     47 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_HEAL	     48 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_GROUP_RECALL	     49 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_INFRAVISION	     50 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERWALK		     51 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_FLY		     52 /* Spell Fly                      */
#define SPELL_REFRESH                53 /* Refresh move spell             */
#define SPELL_DIMENSION_DOOR	     54 /* Spell Dimension door           */
#define SPELL_PORTAL		     55 /* Advanced version of Dimension Door */
#define SPELL_HASTE		     56 /* Spell Haste, char moves faster */
#define SPELL_CUREALL                57 /* Cure the whole group           */
#define SPELL_FIRESTORM              58 /* Creates a fire storm           */
#define SPELL_REGENERATE             59 /* Speedier regeneration          */
#define SPELL_CREATESPRING           60 /* Create a spring of clean water */
#define SPELL_FIRESHIELD             61 /* Surround with a shroud or fire */
#define SPELL_BANISH                 62 /*  */
#define SPELL_HOLYWORD               63 /*  */
#define SPELL_DEMONFIRE              64 /*  */
#define SPELL_SILENCE                65 /*  */
#define SPELL_PROTFROMGOOD           66 /*  */
#define SPELL_BLACKBREATH            67 /*   */
#define SPELL_CHAOSARMOR             68 /*   */
#define SPELL_PARALYZE               69 /*   */
#define SPELL_AEGIS                  70 /*   */
#define SPELL_DARKWARD               71 /*   */
#define SPELL_WRATHOFGOD             72 /*   */
#define SPELL_ICESTORM               73 /*   */
#define SPELL_BLACKDART              74 /*   */
#define SPELL_EYESOFTHEDEAD          75 /*   */
#define SPELL_BREATHE                76 /* Breathe under water            */
#define SPELL_SLOW		     77 /* Victim slows down              */
#define SPELL_CAUSE_LIGHT            78 /* Cause light wounds             */
#define SPELL_CAUSE_CRITIC           79 /* Cause critical wounds          */
#define SPELL_LAY_HANDS              80 /* Lay Hands                      */
#define SPELL_RESTORE_MANA           81 /* POTIONS ONLY!                  */
#define SPELL_RESTORE_MEGAMANA       82 /* POTIONS ONLY!                  */
#define SPELL_REMOVE_INVIS           83 /* Rem invis for OBJECT           */
#define SPELL_DUMBNESS               84 /* int -3                         */
#define SPELL_PHANTOM_ARMOR          85 /* Phantom Armor                  */
#define SPELL_SPECTRAL_WINGS         86 /* Spectral Wings                 */
#define SPELL_UNDEAD_MOUNT           87 /* Summon an undead mount         */
#define SPELL_HOLY_FURY              88 /* Holy Fury                      */
#define SPELL_CHAMPION_STRENGTH      89 /* Champion Strength              */
#define SPELL_HOLY_MACE              90
#define SPELL_PLAGUE                 91
#define SPELL_SONIC_WALL             92 /* Causes temporary deafness      */
#define SPELL_RAY_OF_LIGHT           93
#define SPELL_GROWTH                 94 /* Grow larger and stronger       */
#define SPELL_IDENTIFY               95 /* Identify spell (scrolls)       */
#define SPELL_FIRE_BREATH            96
#define SPELL_GAS_BREATH             97
#define SPELL_FROST_BREATH           98
#define SPELL_ACID_BREATH            99 
#define SPELL_LIGHTNING_BREATH      100
#define SPELL_MANA_ECONOMY          101 /* Spend less mana                  */
#define SPELL_STONE_HAIL            102 /* A hail of stone                  */
#define SPELL_FLYING_FIST           103 /* Flying Fist (straight damage)    */
#define SPELL_SHOCK_SPHERE          104 /*  */
#define SPELL_CAUSTIC_RAIN          105 /*  */
#define SPELL_WITHER                106 /*  */
#define SPELL_METEOR_SWARM          107 /*  */
#define SPELL_BALEFIRE              108 /*  */
#define SPELL_SCORCH                109 /*  */
#define SPELL_IMMOLATE              110 /*  */
#define SPELL_HELLFIRE              111 
#define SPELL_FROST_BLADE           112
#define SPELL_FLAME_BLADE           113
#define SPELL_ACID_STREAM           114
#define SPELL_FLAME_STRIKE          115
#define SPELL_FINGER_OF_DEATH       116
#define SPELL_SCOURGE               117
#define SPELL_SOUL_RIP              118
#define SPELL_FELLBLADE             119
#define SPELL_EXTERMINATE           120
#define SPELL_KNOCK                 121
#define SPELL_BLADE_OF_LIGHT        122
#define SPELL_FLAMES_PURE           123 /* Flames of purification */
#define SPELL_PRISMATIC_BEAM        124
#define SPELL_FREE2                 125
#define SPELL_HOLOCAUST             126
#define SPELL_CRY_FOR_JUSTICE       127
#define SPELL_VIM_RESTORE           128
#define SPELL_QI_RESTORE            129
#define SPELL_ARIA_RESTORE          130
#define SPELL_BLUR                  131
#define SPELL_CONE_COLD             132
#define SPELL_OPAQUE                133 /* Make objs invis */
#define SPELL_INSOMNIA              134
#define SPELL_BLAZEWARD             135 /* Reserved for quest potions only */
#define SPELL_INDESTR_AURA          136 /* Reserved for quest potions only */
#define SPELL_ENTANGLE              137
#define SPELL_MIST_FORM             138 /* Pass under doors */
#define SPELL_SOFTEN_FLESH	    139 /* Victim takes double damage - Crysist */
#define SPELL_AGITATION		    140 
#define SPELL_CONFUSE		    141
#define SPELL_CLEAR_SKY             142
#define SPELL_ANIMATE_CORPSE        143 /* change a corpse into a follower */
#define SPELL_SEAL                  144 /* seals victim in room            */
#define SPELL_POTION_SLEEP          145 /* potion-only sleep spell         */
#define SPELL_FIND_FAMILIAR         146     
#define SPELL_INVIS_STALKER         147
#define SPELL_HOLY_STEED            148 // pal mount summon
#define SPELL_DIVINE_NIMBUS         149 // pal vers of ancient prot
#define SPELL_CLAIRVOYANCE          150 // sorc look at room thru victims eyes
#define SPELL_SUMMON_DHALIN         151 // sorc mob summon
#define SPELL_SUMMON_FEHZOU         152
#define SPELL_SUMMON_NECROLIEU      153
#define SPELL_NETHERCLAW            154 /* Demon weapon dam increase spell */
#define SPELL_CONJURE_FIREDRAKE     155 /* Demon mount summon              */
#define SPELL_RAIN_OF_FIRE          156 /* Demon area attack               */
#define SPELL_TREEWALK_DEARTH       157
#define SPELL_TREEWALK_GLANDS       158
#define SPELL_TREEWALK_TWOOD        159
#define SPELL_TREEWALK_VALLEY       160
#define SPELL_TREEWALK_DWOOD        161
#define SPELL_TREEWALK_SINISTER     162
#define SPELL_TREEWALK_HERMIT       163
#define SPELL_TREEWALK_CMIR         164
#define SPELL_TREEWALK_THEW         165
#define SPELL_TREEWALK_KAI          166
#define SPELL_TREEWALK_SISTER       167
#define SPELL_SENSE_SPAWN           168
#define SPELL_MINOR_STRENGTH        169
#define SPELL_MAJOR_STRENGTH        170
#define SPELL_MINOR_DEXTERITY       171
#define SPELL_DEXTERITY             172
#define SPELL_MINOR_INTELLIGENCE    173
#define SPELL_INTELLIGENCE          174
#define SPELL_MINOR_WISDOM          175
#define SPELL_WISDOM                176
#define SPELL_MINOR_CONSTITUTION    177
#define SPELL_CONSTITUTION          178
#define SPELL_MINOR_CHARISMA        179
#define SPELL_CHARISMA              180
#define SPELL_MINOR_LUCK            181
#define SPELL_LUCK                  182
#define SPELL_SNARE                 183 /* Wimpy mobs can't flee */

/*used in dg_scripts, must increase as spells increase*/
#define MAX_SPELLS		168
/* Skill numbers */
#define SKILL_UNUSED                  0 /* RESERVED - DON'T USE             */
#define SKILL_BACKSTAB                1 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_BASH                    2 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_HIDE                    3 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_KICK                    4 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_PICK_LOCK               5 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_PUNCH                   6 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_RESCUE                  7 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_SNEAK                   8 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_STEAL                   9 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_TRACK		     10 /* Reserved Skill[] DO NOT CHANGE   */
#define SKILL_SECOND_ATTACK	     11 /* Second attack                    */
#define SKILL_THIRD_ATTACK	     12 /* Third attack                     */
#define SKILL_FOURTH_ATTACK	     13 /* Fourth attack                    */
#define SKILL_DEATHBLOW              14 /* Deathblow                        */
#define SKILL_CIRCLE                 15 /* Circle with a dagger             */
#define SKILL_BLOCK                  16 /* Block an attack                  */
#define SKILL_DODGE                  17 /* Dodge an attack                  */
#define SKILL_SPY                    18 /* Peek into another room           */
#define SKILL_STUN                   19 /* Stun                             */
#define SKILL_FORAGE                 20 /* Forage for food                  */
#define SKILL_EYEGOUGE               21 /* Gouge at your opponents eyes     */
#define SKILL_DIRTTHROW              22 /* Throw dirt in your opponents eyes */
#define SKILL_BANDAGE                23 /* Bandage wounds                   */
#define SKILL_CALL_WILD              24 /* Call one of several wild animals */
#define SKILL_TRIP                   25 /* Trip a victim                    */
#define SKILL_RETREAT                26 /* Retreat from battle              */
#define SKILL_INCOGNITO              27 /* Move anonymously                 */
#define SKILL_SWEEP                  28 /* Sweep all mobs in the room       */
#define SKILL_GROINKICK              29 /* Kicks ooponent in the nuts       */
#define SKILL_BRAIN                  30 /* Palm of hand to nose             */
#define SKILL_FIFTH_ATTACK	     31 /* Fifth attack                     */
#define SKILL_DOORBASH               32 /* Door bashing skill               */
#define SKILL_HOLD_BREATH            33 /* Hold breath for a while          */
#define SKILL_SWITCH                 34 /* Switch fighting to new vict      */
#define SKILL_MOTHEREARTH            35 /* Mother Earth - damage            */
#define SKILL_HONE                   36 /* Hone an edged weapon like a razor */
#define SKILL_WARCRY                 37 /* Warcry                           */
#define SKILL_DISARM                 38 /* Disarm opponent                  */
#define SKILL_GUT                    39 /* Gut                              */
#define SKILL_KNOCKOUT               40 /* knockout                         */
#define SKILL_THROW                  41 /* Throw an object at someone       */
#define SKILL_SIXTH_ATTACK           42 /* Sixth Attack                     */
#define SKILL_SEVENTH_ATTACK         43 /* Seventh Attack                   */
#define SKILL_RIPOSTE                44 /* Block + extra attack             */
#define SKILL_RIDE                   45 /* Horseback riding                 */
#define SKILL_TANGLEWEED             46 /* Snares victim in tangleweeds     */
#define SKILL_RUN                    47 /* Use less movement                */
#define SKILL_ROUNDHOUSE             48 /* Roundhouse kick                  */
#define SKILL_KICKFLIP               49 /* kick and flip!                   */
#define SKILL_CHOP                   50 /* Karate chop!                     */
#define SKILL_SHADOW                 51 /* Shadow form                      */
#define SKILL_SHIELDPUNCH            52 /* Punch with shield                */
#define SKILL_AWE                    53 /* Strikes the target dumb with awe */
#define SKILL_BACKSLASH              54 /* Spin move with a sword           */
#define SKILL_UPPERCUT               55 /* An uppercut                      */
#define SKILL_CHARGE                 56 /* Charge!! ...into battle          */
#define SKILL_SLEIGHT                57 /* Sleight of hand                  */
#define SKILL_FLAIL                  58
#define SKILL_DROWN                  59
#define SKILL_BLADED_FURY            60 /* Straight damage attack           */
#define SKILL_POWERSLASH             61 /* Heavy damage  with SLASH weapon  */
#define SKILL_SWARM                  62 /* Call a swarm of bugs!            */
#define SKILL_ELEMENTAL              63 /* Call an elemental to cause damage*/
#define SKILL_MONSOON                64 /* Call a monsoon to do damage.     */
#define SKILL_TACKLE                 65 /* Flying tackle                    */
#define SKILL_PRESSURE               66 /* Pressure Points                  */
#define SKILL_KI                     67 /* Damage attack                    */
#define SKILL_HEADCUT                68 /* Damage (karate chop)             */
#define SKILL_KAMIKAZE               69 /* Kamikaze attack                  */
#define SKILL_HEADBUT                70 /* Good old fashioned headbut       */
#define SKILL_BEARHUG                71 /* Damage the victim with a bearhug */
#define SKILL_BODYSLAM               72 /* Body Slam                        */
#define SKILL_BEFRIEND               73 /* Befriend skill                   */
#define SKILL_INSET                  74 /* Inject various chemicals into vict */
#define SKILL_SWAT                   75 /* Giant damage skill               */
#define SKILL_STOMP                  76 /* More Giant crap                  */
#define SKILL_TRICKPUNCH             77 /* Low level thief damage skill     */
#define SKILL_LOWBLOW                78 /* Mid level thief damage skill     */
#define SKILL_KNEE                   79 /* Mid level thief damage skill     */
#define SKILL_DIG                    80
#define SKILL_BURY                   81
#define SKILL_PUSH                   82
#define SKILL_BREW                   83
#define SKILL_SCRIBE                 84
#define SKILL_DUAL_WIELD             85
#define SKILL_CLIMB                  86
#define SKILL_FAIRYFOLLOWER          87 // Special Skill FF Quest
#define SKILL_PSYCHOTIC_STABBING     88 /* High level thief damage skill    */
#define SKILL_SWORD_THRUST           89 
#define SKILL_BERSERK                90
#define SKILL_AURA_OF_NATURE         91 /* Druid hp bonus skill             */
#define SKILL_FAST_HEAL              92 /* Remort skill adds to heal rate   */
#define SKILL_HAGGLE                 93 /* Auto-skill that get $ off        */
#define SKILL_TRAMPLE                94 /* Mounted damage skill             */
#define SKILL_SECOND_BSTAB           95 /* Second chance at backstab        */
#define SKILL_THIRD_BSTAB            96 /* Third chance at Backstab         */
#define SKILL_FOURTH_BSTAB           97 /* Fourth chance at backstab        */ 
#define SKILL_MIX                    98 /* Druid herb mixing skill          */ 
#define SKILL_BEG                    99 /* Thief skill for getting coin     */
#define SKILL_TAINT_FLUID           100 /* Thief skill to poison liquids    */
#define SKILL_VANISH                101 /* Vamp skill */
#define SKILL_MESMERIZE             102 /* Vamp Skill */
#define SKILL_HISS                  103 /* Vamp Skill */
#define SKILL_HOVER                 104 /* Vamp Skill */
#define SKILL_SWOOP                 105 /* Vamp dmg Skill */
#define SKILL_TEAR                  106 /* Vamp dmg Skill */
#define SKILL_REND                  107 /* Vamp dmg SKill */
#define SKILL_FIND_FLESH            108 /* Vamp skill */
#define SKILL_SACRED_EARTH          109 /* Vamp healing skill */
#define SKILL_DEATH_CALL            110 /* Vamp enhanced dmg skill */
#define SKILL_SPIT_BLOOD            111 /* Vamp dmg skill  */
#define SKILL_HARNESS_WIND          112 /* Titan dmg skill */
#define SKILL_HARNESS_WATER         113 /* Titan dmg skill */
#define SKILL_HARNESS_FIRE          114 /* Titan dmg skill */
#define SKILL_HARNESS_EARTH         115 /* Titan dmg skill */
#define SKILL_SCORCH                116 /* Titan room affect skill */
#define SKILL_ABSORB                117 /* Titan heal skill */
#define SKILL_CLOUD                 118 /* Titan locator skill */
#define SKILL_SUMMON_ELEM           119 /* Titan summoning skill */
#define SKILL_INTIMIDATE 	    120 /* Titan nohassle skill */
#define SKILL_BELLOW                121 /* Titan deafening skill */
#define SKILL_GAUGE                 122 /* +DR for thieves */
#define SKILL_CRITICAL_HIT          123 /* low chance, high dam hit */
#define SKILL_TURN                  124 /* change opponents */
#define SKILL_WHEEL                 125 /* mounted turn */
#define SKILL_QUICKDRAW             126 /* hit aggro before they hit player */
#define SKILL_DECEIVE               127 /* Demon disguising skill     */
#define SKILL_DARK_ALLEGIANCE       128 /* Evil aggr mobs help demons */
#define SKILL_ABSORB_FIRE           129 /* gain health thru fire attacks */
#define SKILL_SOULSUCK              130 /* Mana gain thru corpse */
#define SKILL_BESTOW_WINGS          131 /* Demons get their wings */
#define SKILL_RUB                   132 /* Rub skill for curing blindness */
#define SKILL_FOG					133 /* For being able to use fog skills */
#define SKILL_FOG_MIND				134	/* Skill for cleaning out a mobiles memory. */
#define SKILL_FOG_MIST_AURA			135 /* Sneaklike skill for druids */

#define MAX_SKILLS                  136

/* Chant numbers                                                            */
#define CHANT_UNUSED                  0 /* RESERVED - DON'T USE             */
#define CHANT_HEALING_HAND            1 /* Light healing                    */
#define CHANT_ANCIENT_PROT            2 /* Protective ward                  */
#define CHANT_FIND_CENTER             3 /* Find your centerpoint            */
#define CHANT_OMM                     4 /* Meditative chant                 */
#define CHANT_YIN_XU                  5 /* Cause Yin Xu (damage)            */
#define CHANT_YANG_XU                 6 /* Cause Yang Xu (damage)           */
#define CHANT_MENTAL_BARRIER          7 /*  */
#define CHANT_PSIONIC_DRAIN           8 /*  */
#define CHANT_LEND_HEALTH             9 /* Share HP's with a friend         */
#define CHANT_PSYCHIC_FURY           10 /* Big damage                       */
#define CHANT_BALANCE                11 /* !BASH, !TRIP, !SWEEP             */
#define MAX_CHANTS                   11

/* Prayer numbers */
#define PRAYER_UNUSED                 0 /* RESERVED - DON'T USE */
#define PRAY_CURE_LIGHT               1 /* Cure light wounds                */
#define PRAY_CURE_CRITICAL            2
#define PRAY_REMOVE_POISON            3
#define PRAY_LIFT_CURSE               4
#define PRAY_GODS_FURY                5
#define PRAY_BLESSING                 6
#define PRAY_DISPEL_GOOD              7
#define PRAY_DISPEL_EVIL              8
#define PRAY_HARM                     9
#define PRAY_FOOD                    10
#define PRAY_WATER                   11
#define PRAY_DETECT_POISON           12
#define PRAY_HEAVY_SKIN              13
#define PRAY_GUIDING_LIGHT           14
#define PRAY_SANCTUARY               15
#define PRAY_SECOND_SIGHT            16
#define PRAY_INFRAVISION             17
#define PRAY_BLINDNESS               18
#define PRAY_CURE_BLIND              19
#define PRAY_GROUP_ARMOR             20
#define PRAY_GROUP_HEAL              21
#define PRAY_EARTHQUAKE              22
#define PRAY_CALL_LIGHTNING          23
#define PRAY_HASTE                   24 /* Make very fast                  */
#define PRAY_SUMMON                  25
#define PRAY_REFRESH                 26
#define PRAY_SACRED_SHIELD           27
#define PRAY_HEAL                    28 /* Heal target                     */
#define PRAY_PACIFY                  29 /* Cool all burning jets in a room */
#define PRAY_HAND_BALANCE            30 /* Hand of Balance (damage)        */
#define PRAY_ELEMENTAL_BURST         31 /* Damage                          */
#define PRAY_WINDS_PAIN              32 /* Damage                          */
#define PRAY_SPIRIT_STRIKE           33 /* Damage                          */
#define PRAY_ABOMINATION             34 /* Damage                          */
#define PRAY_WINDS_RECKONING         35 /* Damage                          */
#define PRAY_ANGEL_BREATH            36 /* Damage                          */
#define PRAY_SOUL_SCOURGE            37 /* Damage                          */
#define PRAY_PLAGUE                  38
#define PRAY_WEAKENED_FLESH          39 /*Victim takes 1.25x damage in bat.*/
#define PRAY_DROWSE                  40 /*Victim loses hr and AC           */
#define PRAY_VITALITY                41 /*Victim gets big time heal        */
#define PRAY_REVERSE_ALIGN           42 /*Reverse align on player vict     */
#define PRAY_RETRIEVE_CORPSE         43 /*Retrieve a corpse                */
#define PRAY_AGITATION		     44
#define PRAY_HOLY_WATER              45 /* Saint prayer                    */
#define PRAY_EXORCISM                46 /* Saint 		           */
#define PRAY_AFTERLIFE               47 /* Saint 		           */
#define PRAY_HEAVENSBEAM             48 /* Saint 		           */
#define PRAY_DIVINESHIELD            49 /* Saint 		           */
#define PRAY_ETHEREAL                50 /* Saint 		           */
#define PRAY_CHOIR                   51 /* Saint 		           */
#define PRAY_ENLIGHTENMENT           52 /* Saint                           */
#define PRAY_CURE_SERIOUS            53
#define PRAY_HOLY_ARMOR              54
#define PRAY_DEFLECT                 55
#define MAX_PRAYERS                  56

/* Song numbers */
#define SONG_UNUSED                   0 /* RESERVED - DON'T USE */
#define SONG_BRAVERY                  1
#define SONG_HONOR                    2
#define SONG_HERO                     3
#define SONG_LULLABY                  4

/* These need to re-implemented as stand-alone.  The new abilities
  system will probably not accomodate these.  -- maybe as skills? */
#define STYLE_SNAKE                495
#define STYLE_TIGER                496
#define STYLE_CRANE                497
#define STYLE_MONKEY               498
#define STYLE_RABID_SQUIRREL       499
#define MIN_STYLES                 495

/* This doesn't mean anything anymore... */
#define TOP_SPELL_DEFINE	     599


/* WEAPON ATTACK TYPES */
#define TYPE_HIT                     1
#define TYPE_STING                   2
#define TYPE_WHIP                    3
#define TYPE_SLASH                   4
#define TYPE_BITE                    5
#define TYPE_BLUDGEON                6
#define TYPE_CRUSH                   7
#define TYPE_POUND                   8
#define TYPE_CLAW                    9
#define TYPE_MAUL                   10
#define TYPE_THRASH                 11
#define TYPE_PIERCE                 12
#define TYPE_BLAST		    13
#define TYPE_PUNCH		    14
#define TYPE_STAB		    15
#define TYPE_SPELL		    16
#define TYPE_EMBRACE		    17
#define TYPE_SUBMIT                 18

/* new attack types can be added here - up to TYPE_SUFFERING */
#define TYPE_SUFFERING		    99

#define SAVING_PARA   0
#define SAVING_ROD    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_NOT_SELF     64 /* Only a check, use with i.e. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024
#define TAR_RANGED     2048

struct abil_info_type {
   byte min_position; /* Position for caster	                              */
   int res_min;       /* Min amount of resource used by an abil (highest lev) */
   int res_max;       /* Max amount of resource used by an abil (lowest lev)  */
   int res_change;    /* Change in resource used by an abil from lev to lev   */

   int min_level[NUM_CLASSES];
   int routines;
   byte violent;
   int targets;         /* See below for use with TAR_XXX  */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


/* Attacktypes with grammar */
struct attack_hit_type {
   char	*singular;
   char	*plural;
};


#define ASPELL(spellname) \
void	spellname(int level, struct char_data *ch, \
		  struct char_data *victim, struct obj_data *obj)

#define MANUAL_SPELL(spellname)	spellname(level, caster, cvict, ovict);

ASPELL(spell_create_water);
ASPELL(spell_recall);
//ASPELL(spell_refresh);
ASPELL(spell_teleport);
ASPELL(spell_summon);
ASPELL(spell_locate_object);
ASPELL(spell_charm);
ASPELL(spell_information);
ASPELL(spell_identify);
ASPELL(spell_enchant_weapon);
ASPELL(spell_detect_poison);
ASPELL(spell_dimension_door);
ASPELL(spell_portal);
ASPELL(spell_createspring);
ASPELL(spell_restore_mana);
ASPELL(spell_restore_megamana);
ASPELL(spell_vim_restore);
ASPELL(spell_qi_restore);
ASPELL(spell_aria_restore);
ASPELL(spell_opaque);
ASPELL(spell_knock);
ASPELL(spell_treewalk_dearthwood);
ASPELL(spell_treewalk_tanglewood);
ASPELL(spell_treewalk_crystalmir);
ASPELL(spell_treewalk_darkwood);
ASPELL(spell_treewalk_hermit);
ASPELL(spell_treewalk_grasslands);
ASPELL(spell_treewalk_kailaani);
ASPELL(spell_treewalk_little_sister);
ASPELL(spell_treewalk_thewster);
ASPELL(spell_treewalk_sinister);
ASPELL(spell_treewalk_valley);
ASPELL(spell_sense_spawn);

ASPELL(pray_pacify);
ASPELL(pray_reverse_align);
ASPELL(pray_retrieve_corpse);
ASPELL(pray_holy_water);
ASPELL(pray_choir);

/* basic magic calling functions */

int find_abil_num(char *name, char **list);

void mag_damage(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, byte type, int savetype);

void mag_affects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, byte type, int savetype);

void mag_group_switch(int level, struct char_data *ch, struct char_data *tch, 
  int spellnum, byte type, int savetype);

void mag_groups(int level, struct char_data *ch, int spellnum, byte type, int savetype);

void mag_masses(int level, struct char_data *ch, int spellnum, byte type, int savetype);

void mag_areas(int level, struct char_data *ch, int spellnum, byte type, int savetype);

void mag_summons(int level, struct char_data *ch, struct obj_data *obj,
 int spellnum, byte type, int savetype);

void mag_points(int level, struct char_data *ch, struct char_data *victim,
 int spellnum, byte type, int savetype);

void mag_unaffects(int level, struct char_data *ch, struct char_data *victim,
  int spellnum, byte atype, int savetype);

void mag_alter_objs(int level, struct char_data *ch, struct obj_data *obj,
  int spellnum, byte atype, int savetype);

void mag_creations(int level, struct char_data *ch, int spellnum, byte type);

int	call_magic(struct char_data *caster, struct char_data *cvict,
  struct obj_data *ovict, int spellnum, byte type, int level, int casttype, int ranged);

void	mag_objectmagic(struct char_data *ch, struct obj_data *obj,
			char *argument, byte type);
int	cast_spell(struct char_data *ch, struct char_data *tch,
  struct obj_data *tobj, int spellnum, byte type, int ranged);


/* other prototypes */
void abil_level(int type, int spell, int class, int level);
void init_abil_levels(void);
char *skill_name(int num);
