#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "spells.h"

/* Set this define to wherever you want to save your corpses */
#define CORPSE_FILE "misc/corpse.save"

/* External Structures / Variables */
extern struct obj_data *object_list;
extern struct room_data *world;
struct index_data *obj_index;	/* index table for object file   */
extern room_vnum frozen_start_room;

/* External Functions */
void obj_from_room(struct obj_data *obj);
void obj_to_room(struct obj_data *object, room_rnum room);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);

/* Local Function Declerations */
void save_corpses(void);
void load_corpses(void);
int corpse_save(struct obj_data *obj, FILE * fp, int location,
		bool recurse_this_tree);
int write_corpse_to_disk(FILE * fp, struct obj_data *obj, int locate);
void clean_string(char *buffer);
extern int max_pc_corpse_time;

/* Tada! THE FUNCTIONS ! Yaaa! */

void clean_string(char *buffer)
{
    register char *ptr, *str;

    ptr = buffer;
    str = ptr;

    while ((*str = *ptr)) {
	str++;
	ptr++;
	if (*ptr == '\r')
	    ptr++;
    }
}

int corpse_save(struct obj_data *obj, FILE * fp, int location,
		bool recurse_this_tree)
{
    /* This function basically is responsible for taking the    */
    /* supplied obj and figuring out if it has any contents. If */
    /* it does then we write those to disk.. Ad Nasum.          */

    struct obj_data *tmp;
    int result;

    if (obj) {			/* a little recursion (can be a dangerous thing:) */

	/* recurse_this_tree causes the recursion to branch only
	   down the corpses content's tree and not the contents of the
	   room. obj->next_content points to the rooms contents
	   the first time this function is called from save_corpses
	   hence we avoid going down there otherwise we will save
	   the rooms contents as well as the corpses contents in the 
	   corpse.save file. 
	 */

	if (recurse_this_tree != FALSE) {
	    corpse_save(obj->next_content, fp, location,
			recurse_this_tree);
	}
	recurse_this_tree = TRUE;
	corpse_save(obj->contains, fp, MIN(0, location) - 1,
		    recurse_this_tree);
	result = write_corpse_to_disk(fp, obj, location);

	/* readjust the wieght while we do this */
	for (tmp = obj->in_obj; tmp; tmp = tmp->in_obj)
	    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

	if (!result)
	    return (0);
    }
    return (TRUE);
}


int write_corpse_to_disk(FILE * fp, struct obj_data *obj, int locate)
{
    /* This is basically Patrick's my_obj_save_to_disk function with    */
    /* a few minor tweaks to make it work for corpses. Basically it     */
    /* writes one object out to the corpse file every time it is called. */
    /* It can handle regular obj's and XAP objects.                     */

    int counter, pc_corpse = 0;
    struct extra_descr_data *ex_desc;
    char buf1[MAX_STRING_LENGTH + 1];

    if (obj->action_description) {
	strcpy(buf1, obj->action_description);
	clean_string(buf1);
    } else
	*buf1 = 0;

    if(IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_PC_CORPSE)) {
      pc_corpse = 1;
    }

    fprintf(fp,
	    "#%d\n"
	    "%d %d %d %d %d %d %d\n",
	    GET_OBJ_VNUM(obj),
	    locate,
	    GET_OBJ_VAL(obj, 0),
	    GET_OBJ_VAL(obj, 1),
	    GET_OBJ_VAL(obj, 2),
	    GET_OBJ_VAL(obj, 3), GET_OBJ_VROOM(obj), pc_corpse);	
              /*vroom is the virtual room a corpse */


    if (!(IS_OBJ_STAT(obj, ITEM_UNIQUE_SAVE))) {
	return 1;
    }
    fprintf(fp,
	    "XAP\n"
	    "%s~\n"
	    "%s~\n"
	    "%s~\n"
	    "%s~\n"
	    "%d %d %d %d\n",
	    obj->name ? obj->name : "undefined",
	    obj->short_description ? obj->short_description : "undefined",
	    obj->description ? obj->description : "undefined",
	    buf1,
	    GET_OBJ_TYPE(obj),
	    GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), GET_OBJ_RENT(obj));
    /* Do we have affects? */
    for (counter = 0; counter < MAX_OBJ_AFFECT; counter++)
	if (obj->affected[counter].modifier)
	    fprintf(fp, "A\n"
		    "%d %d\n",
		    obj->affected[counter].location,
		    obj->affected[counter].modifier);

    /* Do we have extra descriptions? */
    if (obj->ex_description) {	/*. Yep, save them too . */
	for (ex_desc = obj->ex_description; ex_desc;
	     ex_desc = ex_desc->next) {
	    /*. Sanity check to prevent nasty protection faults . */
	    if (!*ex_desc->keyword || !*ex_desc->description) {
		continue;
	    }
	    strcpy(buf1, ex_desc->description);
	    clean_string(buf1);
	    fprintf(fp, "E\n" "%s~\n" "%s~\n", ex_desc->keyword, buf1);
	}
    }
    return 1;
}

void save_corpses(void)
{
    /* This is basically the mother of all the save corpse functions */
    /* You can call it from anywhere in the game with no arguments */
    /* Basically any time a corpse is manipulated in any way..either */
    /* directly or indirectly you need to call this function */

    FILE *fp;
    struct obj_data *i, *next;
    int location = 0;

    /* Open corpse file */
    if (!(fp = fopen(CORPSE_FILE, "wb"))) {
	if (errno != ENOENT)	/* if it fails, NOT because of no file */
	    log("SYSERR: checking for corpse file %s : %s", CORPSE_FILE,
		strerror(errno));
	return;
    }

    /* Scan the object list */
    for (i = object_list; i; i = next) {
	next = i->next;

	/* Check if its a players corpse */
	if (IS_OBJ_STAT(i, ITEM_PC_CORPSE)) {
	    /* It is, so save it to a file */
	    if (!corpse_save(i, fp, location, FALSE)) {
		log("SYSERR: A corpse didnt save for some reason");
		fclose(fp);
		return;
	    }
	}
    }
    /* Close the corpse file */
    fclose(fp);
}

void load_corpses(void)
{
    /* Ahh load corpses.. it was cake to write them out to a file      */
    /* it was a pain to load them back up through without a character. */
    /* Because I dont have a character I couldnt figure out how to     */
    /* put objects back into the corpse the exact way they came out..  */
    /* like objects back in their container, etc. So I just decided to */
    /* Dump it all in the corpse and let the character sort it out.    */
    /* If they dont like it, screwum. They are lucky I coded this:)    */
    /* Oh, and a bunch of this code is from Patricks XAP obj's code    */

    FILE *fp;
    char line[256];
    int t[10], danger, zwei = 0;
    int locate = 0, j, k, nr, num_objs = 0;
    struct obj_data *temp = NULL, *obj = NULL, *next_obj = NULL;
    struct extra_descr_data *new_descr;

    if (!(fp = fopen(CORPSE_FILE, "r+b"))) {
	if (errno != ENOENT) {
	    sprintf(buf1, "SYSERR: READING CORPSE FILE %s in load_corpses",
		    CORPSE_FILE);
	    perror(buf1);
	}
    }

    if (!feof(fp))
	get_line(fp, line);
    while (!feof(fp)) {
	temp = NULL;
	/* first, we get the number. Not too hard. */
	if (*line == '#') {
	    if (sscanf(line, "#%d", &nr) != 1) {
		continue;
	    }
	    /* we have the number, check it, load obj. */
	    if (nr == NOTHING) {	/* then it is unique */
		temp = create_obj();
		temp->item_number = NOTHING;
	    } else if (nr < 0) {
		continue;
	    } else {
		if (nr >= 999999)
		    continue;
		temp = read_object(nr, VIRTUAL);
		if (!temp) {
		    continue;
		}
	    }

	    get_line(fp, line);
	    sscanf(line, "%d %d %d %d %d %d %d", t, t + 1, t + 2, t + 3,
		   t + 4, t + 5, t + 6);
	    locate = t[0];
	    GET_OBJ_VAL(temp, 0) = t[1];
	    GET_OBJ_VAL(temp, 1) = t[2];
	    GET_OBJ_VAL(temp, 2) = t[3];
	    GET_OBJ_VAL(temp, 3) = t[4];
            SET_BIT_AR(GET_OBJ_EXTRA(temp), ITEM_NODONATE);
            if(t[6]) {
               SET_BIT_AR(GET_OBJ_EXTRA(temp), ITEM_PC_CORPSE);
            }
            else {
               SET_BIT_AR(GET_OBJ_EXTRA(temp), ITEM_NPC_CORPSE);
            }
            SET_BIT_AR(GET_OBJ_EXTRA(temp), ITEM_UNIQUE_SAVE);
	    GET_OBJ_VROOM(temp) = t[5];
       //     GET_OBJ_TIMER(temp) = max_pc_corpse_time;

	    get_line(fp, line);
	    /* read line check for xap. */
	    if (!strcasecmp("XAP", line)) {  /* then this is a Xap Obj, requires
						   special care */
		if ((temp->name = fread_string(fp, buf2)) == NULL) {
		    temp->name = "undefined";
		}

		if ((temp->short_description = fread_string(fp, buf2)) ==
		    NULL) {
		    temp->short_description = "undefined";
		}

		if ((temp->description = fread_string(fp, buf2)) == NULL) {
		    temp->description = "undefined";
		}

		if ((temp->action_description = fread_string(fp, buf2)) ==
		    NULL) {
		    temp->action_description = 0;
		}
		if (!get_line(fp, line) ||
		    (sscanf
		     (line, "%d %d %d %d", t, t + 1, t + 2, t + 3
		      ) != 4)) {
		    log
		("Format error in first numeric line (expecting 4 args)");
		}
		temp->obj_flags.type_flag = t[0];
		temp->obj_flags.weight = t[1];
		temp->obj_flags.cost = t[2];
		temp->obj_flags.cost_per_day = t[3];
                SET_BIT_AR(GET_OBJ_WEAR(temp), ITEM_WEAR_TAKE);

		/* buf2 is error codes pretty much */
		strcat(buf2,
		       ", after numeric constants (expecting E/#xxx)");

		/* we're clearing these for good luck */

		for (j = 0; j < MAX_OBJ_AFFECT; j++) {
		    temp->affected[j].location = APPLY_NONE;
		    temp->affected[j].modifier = 0;
		}

		/* You have to null out the extradescs when you're parsing a xap_obj.
		   This is done right before the extradescs are read. */

		if (temp->ex_description) {
		    temp->ex_description = NULL;
		}

		get_line(fp, line);
		for (k = j = zwei = 0; !zwei && !feof(fp);) {
		    switch (*line) {
		    case 'E':
			CREATE(new_descr, struct extra_descr_data, 1);
			new_descr->keyword = fread_string(fp, buf2);
			new_descr->description = fread_string(fp, buf2);
			new_descr->next = temp->ex_description;
			temp->ex_description = new_descr;
			get_line(fp, line);
			break;
		    case 'A':
			if (j >= MAX_OBJ_AFFECT) {
			    log
				("SYSERR: Too many object affectations in loading rent file");
			    danger = 1;
			}
			get_line(fp, line);
			sscanf(line, "%d %d", t, t + 1);

			temp->affected[j].location = t[0];
			temp->affected[j].modifier = t[1];
			j++;
			get_line(fp, line);
			break;

		    case '$':
		    case '#':
			zwei = 1;
			break;
		    default:
			zwei = 1;
			break;
		    }
		}		/* exit our for loop */
	    }			/* exit our xap loop */
	    if (temp != NULL) {
		num_objs++;
		/* Check if our object is a corpse */
		if (IS_OBJ_STAT(temp, ITEM_PC_CORPSE)) {	/* scan our temp room for objects */
		    for (obj =
			 world[real_room(frozen_start_room)].contents; obj;
			 obj = next_obj) {
			next_obj = obj->next_content;
			if (obj) {
			    obj_from_room(obj);	/* get those objs from that room */
			    obj_to_obj(obj, temp);	/* and put them in the corpse */
			}
		    }		/* exit the room scanning loop */
		    if (temp) {	/* put the corpse in the right room */
			obj_to_room(temp, real_room(GET_OBJ_VROOM(temp)));
		    }
		} else {
		    /* just a plain obj..send it to a temp room until we load a corpse */
		    obj_to_room(temp, real_room(frozen_start_room));
		}
	    }
	}
    }
    fclose(fp);
}
