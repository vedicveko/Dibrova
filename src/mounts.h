/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 | mounts.h                                |
 | headers for mount-type system.          |
 | written by: Raiker.					   |
 *~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*---------------------------------------------------*/
#define _MOUNTS_H_
/*---------------------------------------------------*/
#define MOUNTT_FILE			"etc/mounts"
#define MAX_MOUNT_TYPES		200
/*---------------------------------------------------*/
#define MOUNTT_LST_SHORT	1
#define MOUNTT_LST_LONG		2
/*---------------------------------------------------*/
#define MOUNTT_MORT			1
#define MOUNTT_REMORT		2
#define MOUNTT_IMMORT		3
/*---------------------------------------------------*/
#define MOUNTT_NEUTRAL		1
#define MOUNTT_EVIL			2
#define MOUNTT_GOOD			3
/*---------------------------------------------------*/
#define MOUNTT_TYPE_RACE	1
#define MOUNTT_TYPE_CLASS	2
#define MOUNTT_TYPE_ALIGN	3
#define MOUNTT_TYPE_PTYPE	4
/*---------------------------------------------------*/
#define MMOUNT_SIZE_TINY		1
#define MMOUNT_SIZE_SMALL		2
#define MMOUNT_SIZE_MEDIUM		3
#define MMOUNT_SIZE_LARGE		4
#define MMOUNT_SIZE_ENOURMOUS	5
/*---------------------------------------------------*/
struct mount_types {
	int id;							/* Identifier for the mount-type.		*/
	char name[MAX_INPUT_LENGTH];	/* Name of the mount-type.				*/
	char *description;				/* Description of the mount-type.		*/
	int disallowed_rac[50];			/* Races  : What races can mount it.	*/
	int disallowed_cls[50];			/* Classes: What classes can mount it.	*/
	int disallowed_alg[3];			/* Aligns : Good, Neutral, Evil			*/
	int disallowed_pty[3];			/* Player types: Mort,Remort,Immort		*/
	byte size;						/* Size of mount						*/
	int spare1;
	int spare2;
	int spare3;
	int spare4;
	int spare5;
	int spare6[100];
	int spare7[100];
	int spare8[100];
	int spare9[100];
	int spare10[100];
	char *spare_cha1;
	char *spare_cha2;
	char *spare_cha3;
};

/* functions in mounts.c */
int mountt_save(void);
int mountt_load(void);
int mountt_find_by_id(int id);
int mountt_find_by_name(char *name);
int mountt_is_size(int mountt_id, int size);
int mountt_disallow_race(int mountt_id, int race);
int mountt_disallow_class(int mountt_id, int chclass);
int mountt_disallow_align(int mountt_id, int align);
int mountt_disallow_pty(int mountt_id, int pty);
void mountt_list(struct char_data *ch, bool lst_all, int id);
int mountt_view_info(struct char_data *ch, int id, byte type);
void mountt_create(struct char_data *ch);
void mountt_delete(struct char_data *ch, int id);
int mountt_toggle(int id, byte type, int bit);
void mountt_set_bits(struct char_data *ch, int id, byte type, int bit);
void mountt_set_name(struct char_data *ch, int id, char *name);