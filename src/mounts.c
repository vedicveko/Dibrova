/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 | mounts.c                                |
 | sourcode for mount-type system.         |
 | written by: Raiker.					   |
 *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* includes */
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "mounts.h"
/* external variables */
extern char *pc_class_types[];
extern char *pc_race_types[];
/* external functions */
void colorless_str(char *str);
int mountt_clear_deleted_types(int id);
/* local variables */
int nr_mount_types = 0;
/* main structure */
struct mount_types mountt[MAX_MOUNT_TYPES];

ACMD(do_madmin);

const char *pc_align_types[] = {
	"\n",
	"Neutral",
	"Evil",
	"Good",
	"\n"
};
const char *pc_pty_types[] = {
	"\n",
	"Mort",
	"Remort",
	"Immort",
	"\n"
};
const char *pc_size_types[] = {
	"\n",
	"Tiny",
	"Small",
	"Medium",
	"Large",
	"Enormous",
	"\n"
};

int mountt_save(void)
{
	FILE *fl;
	if (!(fl = fopen(MOUNTT_FILE, "wb")))
	{
		log("SYSERR: Mount-Type file does not exist.");
		return FALSE;
	} else {
		fwrite(&nr_mount_types, sizeof(int), 1, fl);
		fwrite(mountt, sizeof(struct mount_types), nr_mount_types, fl);
		fclose(fl);
		return TRUE;
	}
}

int mountt_load(void)
{
	FILE *fl;
	int i = 0;
	char name[MAX_INPUT_LENGTH];
	*name = '\0';
	
	memset(mountt, 0, sizeof(struct mount_types) * MAX_MOUNT_TYPES);
	nr_mount_types = 0;
	i = 0;

	if (!(fl = fopen(MOUNTT_FILE, "rb")))
	{
		log("   Mount-types file does not exist!!");
		return FALSE;
	} else {
		fread(&nr_mount_types, sizeof(int), 1, fl);
		fread(mountt, sizeof(struct mount_types), nr_mount_types, fl);
		fclose(fl);
		sprintf(name, "Not Set");
		strncpy(mountt[0].name, CAP((char *)name), 40);
		mountt_save();
		return TRUE;
	}
}

int mountt_find_by_id(int id)
{
	if (id > (nr_mount_types-1))
		return 0;
	else if (id <= 0)
		return 0;
	else
		return id;
}

int mountt_find_by_name(char *name)
{
	int i = 0;
	if (name == NULL)
		return 0;
	else {
		for (i = 1; i < (nr_mount_types-1);i++)
			if (is_abbrev(name, mountt[i].name))
				return (int)mountt[i].id;
		return 0;
	}
}

int mountt_disallow_race(int mountt_id, int race)
{
	if (race < 0)
		return FALSE;
	else if (mountt_find_by_id(mountt_id) <= 0)
		return FALSE;
	else
	{
		if (mountt[mountt_id].disallowed_rac[race] >= 1)
			return TRUE;
		return FALSE;
	}
}

int mountt_is_size(int mountt_id, int size)
{
	if (size <= 0 || size > 5)
		return FALSE;
	else if (mountt_find_by_id(mountt_id) <= 0)
		return FALSE;
	else
	{
		if (mountt[mountt_id].size == size)
			return TRUE;
		return FALSE;
	}
}

int mountt_disallow_class(int mountt_id, int chclass)
{
	if (chclass < 0)
		return FALSE;
	else if (mountt_find_by_id(mountt_id) <= 0)
		return FALSE;
	else
	{
		if (mountt[mountt_id].disallowed_cls[chclass] >= 1)
			return TRUE;
		return FALSE;
	}
}

int mountt_disallow_align(int mountt_id, int align)
{
	if (align <= 0 || align > 3)
		return FALSE;
	else if (mountt_find_by_id(mountt_id) <= 0)
		return FALSE;
	else
	{
		if (mountt[mountt_id].disallowed_alg[align] >= 1)
			return TRUE;
		return FALSE;
	}
}

int mountt_disallow_pty(int mountt_id, int pty)
{
	if (pty <= 0 || pty > 3)
		return FALSE;
	else if (mountt_find_by_id(mountt_id) <= 0)
		return FALSE;
	else
	{
		if (mountt[mountt_id].disallowed_pty[pty] >= 1)
			return TRUE;
		return FALSE;
	}
}


void mountt_list(struct char_data *ch, bool lst_all, int id)
{
	int i = 0;
	if (ch == NULL)
		return;
	else if (lst_all == FALSE && id == 0)
		return;
	else {
		if (lst_all == FALSE)
			mountt_view_info(ch, id, MOUNTT_LST_LONG);
		else
			for(i = 1; i < nr_mount_types;i++)
				mountt_view_info(ch, i, MOUNTT_LST_SHORT);
		return;
	}
}

int mountt_view_info(struct char_data *ch, int id, byte type)
{
	char output[MAX_STRING_LENGTH];
	int i = 0;
	*output = '\0';
	if (ch == NULL || id == 0 || type <= 0 || type > 2)
		return FALSE;
	else {
		if (mountt_find_by_id(id) <= 0)
			return FALSE;
		else if (type == MOUNTT_LST_LONG)
		{
			sprintf(output, "%s/cgId: (/cw%d/cg) Name: (/cw%s/cg)  /c0 \r\n", output, mountt[id].id, mountt[id].name);
			sprintf(output, "%s/cgCan't be ridden by: /c0\r\n", output);
			sprintf(output, "%s/cg                 Races  :/c0\r\n", output);
			for(i = 0; i < NUM_RACES;i++) {
				if (mountt_disallow_race(id, i))
			sprintf(output, "%s/cg                          /cw%s/c0\r\n", output, pc_race_types[i]);
			}
			sprintf(output, "%s/cg                 Classes:/c0\r\n", output);
			for(i = 0; i < NUM_CLASSES;i++) {
				if (mountt_disallow_class(id, i))
			sprintf(output, "%s/cg                          /cw%s/c0\r\n", output, pc_class_types[i]);
			}
			sprintf(output, "%s/cg                 Aligns :/c0\r\n", output);
			for(i = 1; i < 4;i++) {
				if (mountt_disallow_align(id, i))
			sprintf(output, "%s/cg                          /cw%s/c0\r\n", output, pc_align_types[i]);
			}
			sprintf(output, "%s/cg                 Ptypes :/c0\r\n", output);
			for(i = 1; i < 4;i++) {
				if (mountt_disallow_pty(id, i))
			sprintf(output, "%s/cg                          /cw%s/c0\r\n", output, pc_pty_types[i]);
			}
			sprintf(output, "%s/cg                 Size  :/c0\r\n", output);
			for(i = 1; i < 5;i++) {
				if (mountt_is_size(id, i))
			sprintf(output, "%s/cg                          /cw%s/c0\r\n", output, pc_size_types[i]);
			}
			sprintf(output, "%s\r\n/cg/c0\r\n", output);
			page_string(ch->desc, output, 0);
			return TRUE;
		}
		else if (type == MOUNTT_LST_SHORT)
		{
			sprintf(output, "%s/cgId: (/cw%d/cg) Name: (/cw%s/cg)  /c0 \r\n", output, mountt[id].id, mountt[id].name);
			page_string(ch->desc, output, 0);
			return TRUE;
		}
	}
	return FALSE;
}

void mountt_create(struct char_data *ch)
{
	char output[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];
	colorless_str(arg);
	if (nr_mount_types == MAX_MOUNT_TYPES)
	{
		send_to_char("Maximum number of mount-types has been reached.\r\n",ch);
		return; 
	}
	sprintf(name, "New mount-type");
	strncpy(mountt[nr_mount_types].name, CAP((char *)name), 40);
	mountt[nr_mount_types].id = nr_mount_types;
	nr_mount_types++;
	mountt_save();
	sprintf(output, "/cgId: (/cw%d/cg)/c0\r\n", mountt[(nr_mount_types-1)].id); 
	send_to_char(output, ch);
	send_to_char("A new mount-type has been created.\r\n", ch);
	return;
}

void mountt_delete(struct char_data *ch, int id)
{
	int i = 0;
	if (ch == NULL)
		return;
	else if (mountt_find_by_id(id) <= 0)
	{
		send_to_char("No mount-type found to delete.\r\n", ch);
		return;
	} else {
		memset(&mountt[id], sizeof(struct mount_types), 0);
		for (i = id; i < (nr_mount_types-1); i++)
			mountt[i] = mountt[i + 1];
		nr_mount_types--;
		mountt_clear_deleted_types(id);
		send_to_char("Mount-type deleted..\r\n", ch);
		mountt_save();
		return;
	}
}

int mountt_toggle(int id, byte type, int bit)
{
	if ((mountt_find_by_id(id) <= 0) || (type >= 5) || (type <= 0) || (bit < 0))
		return FALSE;
	else {
		if (type == MOUNTT_TYPE_RACE)
		{
			if (mountt[id].disallowed_rac[bit] <= 0)
				mountt[id].disallowed_rac[bit] = 1;
			else if (mountt[id].disallowed_rac[bit] >= 1)
				mountt[id].disallowed_rac[bit] = 0;
			return TRUE;
		}
		else if (type == MOUNTT_TYPE_CLASS)
		{
			if (mountt[id].disallowed_cls[bit] <= 0)
				mountt[id].disallowed_cls[bit] = 1;
			else if (mountt[id].disallowed_cls[bit] >= 1)
				mountt[id].disallowed_cls[bit] = 0;
			return TRUE;
		}
		else if (type == MOUNTT_TYPE_ALIGN)
		{
			if (mountt[id].disallowed_alg[bit] <= 0)
				mountt[id].disallowed_alg[bit] = 1;
			else if (mountt[id].disallowed_alg[bit] >= 1)
				mountt[id].disallowed_alg[bit] = 0;
			return TRUE;
		}
		else if (type == MOUNTT_TYPE_PTYPE)
		{
			if (mountt[id].disallowed_pty[bit] <= 0)
				mountt[id].disallowed_pty[bit] = 1;
			else if (mountt[id].disallowed_pty[bit] >= 1)
				mountt[id].disallowed_pty[bit] = 0;
			return TRUE;
		} else {
			return FALSE;
		}
		return TRUE;
	}
}

void mountt_set_bits(struct char_data *ch, int id, byte type, int bit)
{
	char output[MAX_STRING_LENGTH];
	int i = 0;
	*output = '\0';
	if (ch == NULL)
		return;
	else if (type <= 0 || type >= 5)
	{
		send_to_char("No such type.\r\n", ch);
		return;
	}
	else if (mountt_find_by_id(id) <= 0)
	{
		send_to_char("No such mount-type.\r\n", ch);
		return;
	}
	else if (bit < 0)
	{
		if (type == MOUNTT_TYPE_RACE)
		{
			sprintf(output, "%s/cgTheese are the races:/c0\r\n", output);
			for(i = 0;i < NUM_RACES;i++)
			sprintf(output, "%s/cw%d)/cg %s./c0\r\n", output, i, pc_race_types[i]);
			send_to_char(output, ch);
			return;
		}
		else if (type == MOUNTT_TYPE_CLASS)
		{
			sprintf(output, "%s/cgTheese are the classes:/c0\r\n", output);
			for(i = 0;i < NUM_CLASSES;i++)
			sprintf(output, "%s/cw%d)/cg %s./c0\r\n", output, i, pc_class_types[i]);
			send_to_char(output, ch);
			return;
		}
		else if (type == MOUNTT_TYPE_ALIGN)
		{
			sprintf(output, "%s/cgTheese are the aligns:/c0\r\n", output);
			for(i = 1;i < 4;i++)
			sprintf(output, "%s/cw%d)/cg %s./c0\r\n", output, i, pc_align_types[i]);
			send_to_char(output, ch);
			return;
		}
		else if (type == MOUNTT_TYPE_PTYPE)
		{
			sprintf(output, "%s/cgTheese are the playertypes:/c0\r\n", output);
			for(i = 1;i < 4;i++)
			sprintf(output, "%s/cw%d)/cg %s./c0\r\n", output, i, pc_pty_types[i]);
			send_to_char(output, ch);
			return;
		}

	} else {
		mountt_toggle(id, type, bit);
		send_to_char("Done.\r\n", ch);
		mountt_save();
		return;
	}
}

void mountt_set_name(struct char_data *ch, int id, char *name)
{
	if (ch == NULL || !*name)
		return;
	else if (mountt_find_by_id(id) <= 0)
		return;
	else {
		strncpy(mountt[id].name, CAP((char *)name), 40);
		mountt_save();
		return;
	}
}

ACMD(do_madmin)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	int mount_nr = 0, arg4_nr = 0;
	argument = two_arguments(argument, arg1, arg2);
	argument = one_argument(argument, arg3);
	if (!*arg1)
	{
		mountt_list(ch, TRUE, 0);
		send_to_char("\r\nSyntax: (madmin create | delete || set || info).\r\n", ch);
		return;
	}
	else {
		if (is_abbrev(arg1, "create"))
		{
			mountt_create(ch);
			return;
		}
		else if (is_abbrev(arg1, "delete"))
		{
			if (!*arg2)
			{
				send_to_char("Please enter what mount-type you wish to delete.\r\n", ch);
				return;
			} else {
				mountt_delete(ch, atoi(arg2));
				return;
			}
		}
		else if (is_abbrev(arg1, "info"))
		{
			if (!*arg2)
				mountt_list(ch, TRUE, 0);
			else
				mountt_list(ch, FALSE, atoi(arg2));
			return;
		}
		else if (is_abbrev(arg1, "set"))
		{
			mount_nr = atoi(arg2);
			if (mount_nr == 0)
			{
				send_to_char("You must enter a mount-type number.\r\n", ch);
				send_to_char("You can set the following bits:\r\n", ch);
				send_to_char("                               - Name        \"Name of Mount-type\"\r\n", ch);
				send_to_char("                               - Race        #racenum\r\n", ch);
				send_to_char("                               - Class       #classnum\r\n", ch);
				send_to_char("                               - Align       #align\r\n", ch);
				send_to_char("                               - PlayerType  #playertype\r\n", ch);
				return;
			}
			else if (!*arg3)
			{
				send_to_char("You can set the following bits:\r\n", ch);
				send_to_char("                               - Name        \"Name of Mount-type\"\r\n", ch);
				send_to_char("                               - Race        #racenum\r\n", ch);
				send_to_char("                               - Class       #classnum\r\n", ch);
				send_to_char("                               - Align       #align\r\n", ch);
				send_to_char("                               - PlayerType  #playertype\r\n", ch);
				return;
			} else {
				skip_spaces(&argument);
				delete_doubledollar(argument);
				arg4_nr = atoi(argument);
				if (is_abbrev(arg3, "name"))
				{
					if (!*argument)
						send_to_char("You must enter a name!\r\n", ch);
					else {
						mountt_set_name(ch, mount_nr, argument);
						send_to_char("You change the name.\r\n", ch);
					}
					return;
				}
				else if (is_abbrev(arg3, "race"))
				{
					if (!*argument)
					{
						send_to_char("You must enter a #racenum!\r\n", ch);
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_RACE, -1);
						return;
					} else {
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_RACE, arg4_nr);
						return;
					}
				}
				else if (is_abbrev(arg3, "class"))
				{
					if (!*argument)
					{
						send_to_char("You must enter a #classnum!\r\n", ch);
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_CLASS, -1);
						return;
					} else {
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_CLASS, arg4_nr);
						return;
					}
				}
				else if (is_abbrev(arg3, "align"))
				{
					if ((arg4_nr > 3) || (arg4_nr <= 0))
					{
						send_to_char("You must enter a #align!\r\n", ch);
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_ALIGN, -1);
						return;
					} else {
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_ALIGN, arg4_nr);
						return;
					}
				}
				else if (is_abbrev(arg3, "playertype"))
				{
					if ((arg4_nr > 3) || (arg4_nr <= 0))
					{
						send_to_char("You must enter a #playertype!\r\n", ch);
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_PTYPE, -1);
						return;
					} else {
						mountt_set_bits(ch, mount_nr, MOUNTT_TYPE_PTYPE, arg4_nr);
						return;
					}
				}
				else if (is_abbrev(arg3, "size"))
				{
					if ((arg4_nr > 5) || (arg4_nr <= 0))
					{
						send_to_char("You must enter a #size!\r\n", ch);
						send_to_char("Theese are the sizes:\r\n", ch);
						send_to_char("                     /cw1./cg Tiny /c0\r\n", ch);
						send_to_char("                     /cw2./cg Small /c0\r\n", ch);
						send_to_char("                     /cw3./cg Medium /c0\r\n", ch);
						send_to_char("                     /cw4./cg Large /c0\r\n", ch);
						send_to_char("                     /cw5./cg Enormous /c0\r\n", ch);
						return;
					} else {
						mountt[mount_nr].size = arg4_nr;
						return;
					}
				} else {
					send_to_char("You can not set that.\r\n", ch);
					return;
				}
			}
		} else {
			mountt_list(ch, TRUE, 0);
			send_to_char("\r\nSyntax: (madmin create | delete || set || info).\r\n", ch);
			return;
		}
	}
}