/*****************************************************************************
**  color.c                                                                 **
**									    **
**  Handles embedded color codes.  This is a stripped-down version of my    **
**  color routine.  Certain options are not present (like "remembering"     **
**  colors) so that additional player data won't be needed.                 **
**									    **
** 			                	(C)opyright 1997 M.C. Lewis **
*****************************************************************************/

/* colors are /cr (red), /cR (extended red, which is either bold or light),
/Cu (cursor up), /Ch (home), /Cc (clear), etc.  Just read through it =) */

/* These are the same codes supported by default in CircEdit, a CircleMUD
area editor (builder) for Win95/NT.  It's available from the CircleMUD
FTP site and my website, http://www.geocities.com/SiliconValley/Park/6028 */

/* Note that when combining fg and bg colors, if you are using EXTENDED
fg colors, you can specify fg and bg in any order, i.e., /cW/bb or
/bb/cW.  When using NORMAL fg colors, you should specify the fg color
FIRST, since the ANSI code will turn off the current background color.
That is, /cw/bb will be normal white text on blue background, but 
/bb/cw will just be normal white text WITHOUT background. */

/* e-mail:   mc2@geocities.com */
/* W W W :   http://www.geocities.com/SiliconValley/Park/6028 */


#define _color_c_

#include <stdio.h>
#include <string.h>


/* The standard ANSI colors */
#define COL_NONE	""
#define COL_OFF 	"\x1B[0;0m"
#define COL_BLACK	"\x1B[0;30m"
#define COL_RED 	"\x1B[0;31m"
#define COL_GREEN 	"\x1B[0;32m"
#define COL_YELLOW 	"\x1B[0;33m"
#define COL_BLUE 	"\x1B[0;34m"
#define COL_MAGENTA 	"\x1B[0;35m"
#define COL_CYAN 	"\x1B[0;36m"
#define COL_WHITE 	"\x1B[0;37m"
#define COL_FG_OFF	"\x1B[38m" /* works on normal terminals but not
color_xterms */

/* The bold or extended ANSI colors */
#define COL_E_BLACK	"\x1B[1;30m" /* /cl */
#define COL_E_RED 	"\x1B[1;31m"
#define COL_E_GREEN 	"\x1B[1;32m"
#define COL_E_YELLOW 	"\x1B[1;33m"
#define COL_E_BLUE 	"\x1B[1;34m"
#define COL_E_MAGENTA 	"\x1B[1;35m"
#define COL_E_CYAN 	"\x1B[1;36m"
#define COL_E_WHITE 	"\x1B[1;37m"

/* The background colors */

#define COL_BK_BLACK    "\x1B[40m" /* /cL */
#define COL_BK_RED 	"\x1B[41m"
#define COL_BK_GREEN 	"\x1B[42m"
#define COL_BK_YELLOW 	"\x1B[43m"
#define COL_BK_BLUE 	"\x1B[44m"
#define COL_BK_MAGENTA 	"\x1B[45m"
#define COL_BK_CYAN 	"\x1B[46m"
#define COL_BK_WHITE 	"\x1B[47m"

#define STYLE_UNDERLINE   "\x1B[4m" /* /cu */
#define STYLE_FLASH	  "\x1B[5m" /* /cf */
#define STYLE_REVERSE	  "\x1B[7m" /* /cv */

/* r is red, b is blue, etc.  L and l are black, you can easily change to/
add in K/k for black, which some people prefer since black is "KEY"
as in CMYK. */

/* Cursor controls */
#define C_UP    "\x1B[A"
#define C_DOWN  "\x1B[B"
#define C_RIGHT "\x1B[C"
#define C_LEFT  "\x1B[D"
#define C_HOME  "\x1B[H"
#define C_CLR   C_HOME "\x1B[J"

/* List of colors */
const char *COLORLIST[] = 
{
	 COL_OFF,	COL_NONE, 	COL_BLACK, 
     COL_RED,    	COL_GREEN,	COL_YELLOW,
     COL_BLUE,    COL_MAGENTA,    	COL_CYAN,    
     COL_WHITE,	COL_FG_OFF,
     
     COL_E_BLACK,	 COL_E_RED,  	COL_E_GREEN,
     COL_E_YELLOW,  COL_E_BLUE,  	COL_E_MAGENTA,
     COL_E_CYAN,  COL_E_WHITE,	COL_FG_OFF,
     
     COL_BK_BLACK,	 COL_BK_RED, 	COL_BK_GREEN,
     COL_BK_YELLOW, COL_BK_BLUE, 	COL_BK_MAGENTA,
     COL_BK_CYAN,	COL_BK_WHITE,	COL_OFF,
     
     C_UP,C_DOWN,C_RIGHT,C_LEFT,C_HOME,C_CLR,
     STYLE_UNDERLINE, STYLE_FLASH, STYLE_REVERSE
}; 

#define LAST_COLOR 37

#define BUFSPACE	24*1024 /* This should be equal to LARGE_BUFSIZE
in structs.h */

#define BUFSIZE BUFSPACE-1


#define START_CHAR '/'     /* a forward slash followed by c or C or b
and a number */

/******* You can change START_CHAR to whatever you wish, like '^' *******/


static char out_buf[BUFSPACE],insert_text[BUFSPACE];

void proc_color(char *inbuf, int color_lvl)
{
  int has_color  = (color_lvl>1), 
    has_cursor = ((color_lvl==1) || (color_lvl==3)),
    current_color = -1;
  register int inpos=0,outpos=0;
  int remaining,color=-2;
  
  if(*inbuf == '\0') return; /* if first char is null */ 
  
                             /* If color level is 1 (sparse), then character will get cursor controls
                             only.  If color level is 2, character will get color codes only.  If
                             color is complete (3), character will get both.   Color level 0 removes
  all color codes, of course. =) */
  
 
 *out_buf = '\0'; *insert_text = '\0';


  while(inbuf[inpos]!='\0')	{
    remaining = strlen(inbuf)-inpos;
    if(remaining>2){
      if(inbuf[inpos]==START_CHAR && 
        (inbuf[inpos+1]=='c' ||
        inbuf[inpos+1]=='b' ||
        inbuf[inpos+1]=='C' ||
        inbuf[inpos+1]==START_CHAR)){
        *insert_text = '\0';
        switch(inbuf[inpos+1]){
      		case 'c': /* foreground color */
            switch(inbuf[inpos+2]){
            case 'l': color =  2; break;
            case 'L': color = 11; break;
            case 'r': color =  3; break;
            case 'R': color = 12; break;
            case 'g': color =  4; break;
            case 'G': color = 13; break;
            case 'y': color =  5; break;
            case 'Y': color = 14; break;
            case 'b': color =  6; break;
            case 'B': color = 15; break;
            case 'm':
            case 'p': color =  7; break;
            case 'M':
            case 'P': color = 16; break;
            case 'c': color =  8; break;
            case 'C': color = 17; break;
            case 'w': color =  9; break;
            case 'W': color = 18; break;
            case 'O':
            case 'o': color = 10; break;
            case 'U':
            case 'u': color = 35; break;
/*
            case 'F':
            case 'f': color = 36; break;
*/
            case 'V':
            case 'v': color = 37; break;
            case '0': color =  0; break;
            default : color =  1; /* no change */
            }
            if(color != current_color)
              if(has_color) 
                strcpy(insert_text,COLORLIST[color]);
              inpos += 3;
              current_color = color;
              break;
            case 'b': /* background color */
              switch(inbuf[inpos+2]) {
              /*
              case 'l': color = 20; break;
              case 'r': color = 21; break;
              case 'g': color = 22; break;
              case 'y': color = 23; break;
              case 'b': color = 24; break;
              case 'm':
              case 'p': color = 25; break;
              case 'c': color = 26; break;
              case 'w': color = 27; break;
              case 'o': color =  0; break;
              */
              default : color =  1; /* no change */
              }
              if(color != current_color)
                if(has_color)
                  strcpy(insert_text,COLORLIST[color]);
                inpos += 3;
                current_color = color;
                break;
              case 'C': /* cursor control */
                switch(inbuf[inpos+2]){
                case 'u': color = 29; break;
                case 'd': color = 30; break;
                case 'r': color = 31; break;
                case 'l': color = 32; break;
                case 'h': color = 33; break;
                case 'c': color = 34; break;
                default : color = 1; /* no change */
                }
                if(has_cursor)
                  strcpy(insert_text,COLORLIST[color]);
                inpos += 3;
                break;
                case START_CHAR: /* just a slash */
                  *insert_text = START_CHAR;
                  insert_text[1] = '\0';
                  inpos +=2;
                  break;
                default:
                  strcpy(insert_text, "/\0");
                  
                  inpos+=3;
                  
        } /* switch */
        
        if(color_lvl==0) out_buf[outpos] = '\0';
        
        if((strlen(out_buf)+strlen(insert_text))<BUFSIZE)
          /* don't overfill buffer */
        { 
          out_buf[outpos] = '\0'; /* so strcat is not confused by whatever out_buf WAS */
          strcat(out_buf,insert_text);  
          outpos = strlen(out_buf);
        }
        
        }  /* if char is '/' (START_CHAR) */
        else 
        { if(outpos <BUFSIZE)
        {out_buf[outpos] = inbuf[inpos]; inpos++; outpos++; }}
        
  } /* if remaining > 2 */
  else
		{ if(outpos < BUFSIZE)
  {out_buf[outpos] = inbuf[inpos]; inpos++; outpos++; }}
  
  
  } /* while */


  out_buf[outpos] = '\0';
  if(has_color) strcat(out_buf, COL_OFF);
  
  /* printf("outbuf: %s\n",out_buf); */ /* for debugging */
  
  strcpy(inbuf, out_buf);

}
