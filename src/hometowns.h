/*
**    File: hometowns.h
**      By: Cj Stremick
**     For: Hometown functionality.
** History: -CMS- 10/21/98 Created
**          -CMS- 11/10/99 Ownership abondoned.  If this code is going to 
**          be subject to fucking shit-assed style, I will take no more 
**          credit or blame for what happens to it.
*/


/* Hometowns */
#define HOME_UNDEFINED   -1
#define HOME_MIDGAARD     0
#define HOME_JARETH       1
#define HOME_MCGINTEY     2
#define HOME_SILVERTHORNE 3
#define HOME_ELVENCITY    4
#define HOME_OFINGIA      5
#define HOME_MALATHAR     6

#define NUM_STARTROOMS    8

/* function prototypes for hometowns.c */
int parse_hometown(char *arg);
int get_default_room(struct char_data *ch);

