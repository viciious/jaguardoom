/* DoomDef.h */

#ifndef DOOMDEF_H__
#define DOOMDEF_H__

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* JAGUAR should be defined on the compiler command line for console builds */
/* if MARS isn't defined, assume jaguar version */

/*define	MARS */



/* if rangecheck is undefined, most parameter validation debugging code */
/* will not be compiled */
#define	RANGECHECK

/* if SIMULATOR is defined, compile in extra code for things like console */
/* debugging aids */
#ifndef JAGUAR
#ifndef MARS
#define SIMULATOR
#endif
#endif

typedef unsigned short pixel_t;

#ifdef MARS
typedef unsigned char inpixel_t;
#else
typedef unsigned short inpixel_t;
#endif

/* the structure sizes are frozen at ints for jaguar asm code, but shrunk */
/* down for mars memory cramming */
#ifdef MARS
#define	VINT	short
#else
#define	VINT	int
#endif

#ifdef MARS
#define ATTR_DATA_CACHE_ALIGN __attribute__((section(".data"), aligned(16)))
#define ATTR_OPTIMIZE_SIZE __attribute__((optimize("Os")))
#define ATTR_OPTIMIZE_EXTREME __attribute__((optimize("O3", "no-align-loops", "no-align-functions", "no-align-jumps", "no-align-labels")))
#else
#define ATTR_DATA_CACHE_ALIGN
#define ATTR_OPTIMIZE_SIZE
#define ATTR_OPTIMIZE_EXTREME
#endif

/*============================================================================= */

/* all external data is defined here */
#include "doomdata.h"

/* header generated by multigen utility */
#include "info.h"

#define D_MAXCHAR  ((char)0x7f)
#define D_MAXSHORT ((short)0x7fff)
#define D_MAXINT   ((int)0x7fffffff)  // max pos 32-bit int
#define D_MAXLONG  ((long)0x7fffffff)

#define D_MINCHAR  ((char)0x80)
#define D_MINSHORT ((short)0x8000)
#define D_MININT   ((int)0x80000000) // max negative 32-bit integer
#define D_MINLONG  ((long)0x80000000)

#ifndef NULL
#define	NULL	0
#endif

int D_vsnprintf(char *str, size_t nmax, const char *format, va_list ap);
int D_snprintf(char *buf, size_t nsize, const char *fmt, ...);
void D_printf (char *str, ...);

void D_isort(int* a, int len) ATTR_OPTIMIZE_SIZE;

/*
===============================================================================

						GLOBAL TYPES

===============================================================================
*/

#define MAXPLAYERS	2

#define TICRATE		15				/* number of tics / second */
#define TICVBLS		(60/TICRATE)	/* vblanks per tic */
									/* change this to 'ticrate' if you want */
									/* to use a different rate on PAL */

#define	FRACBITS		16
#define	FRACUNIT		(1<<FRACBITS)
typedef int fixed_t;

#define	ANG45	0x20000000
#define	ANG90	0x40000000
#define	ANG180	0x80000000
#define	ANG270	0xc0000000
typedef unsigned angle_t;

#define	FINEANGLES			8192
#define	FINEMASK			(FINEANGLES-1)
#define	ANGLETOFINESHIFT	19	/* 0x100000000 to 0x2000 */

#ifdef MARS

fixed_t finesine(angle_t angle) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;;
fixed_t finecosine(angle_t angle) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

#else

extern	const fixed_t		finesine_[5*FINEANGLES/4];
extern	const fixed_t		*finecosine_;

#define finesine(x) finesine_[x]
#define finecosine(x) finecosine_[x]

#endif

typedef enum
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare
} skill_t;


typedef enum 
{
	ga_nothing, 
	ga_died,
	ga_completed,
	ga_secretexit,
	ga_warped,
	ga_exitdemo,
	ga_startnew
} gameaction_t;


/* */
/* library replacements */
/* */

static inline int D_abs(int x)
{
	if (x < 0)
		return -x;
	return x;
}

void D_memset (void *dest, int val, int count);
void D_memcpy (void *dest, const void *src, int count);
void D_strncpy (char *dest, const char *src, int maxcount);
int D_strncasecmp (const char *s1, const char *s2, int len);
int D_strcasecmp (const char *s1, const char *s2);
int mystrlen(const char *string);
int D_atoi(const char* str);
char* D_strchr(const char* str, char chr);

/*
===============================================================================

							MAPOBJ DATA

===============================================================================
*/

struct mobj_s;

/* think_t is a function pointer to a routine to handle an actor */
typedef void (*think_t) ();

/* a latecall is a function that needs to be called after p_base is done */
typedef void (*latecall_t) (struct mobj_s *mo);

typedef struct thinker_s
{
	struct		thinker_s	*prev, *next;
	think_t		function;
} thinker_t;

struct player_s;

typedef struct mobj_s
{
	struct	mobj_s* prev, * next;
	latecall_t		latecall;			/* set in p_base if more work needed */
	fixed_t			x, y, z;

	unsigned char	movedir;		/* 0-7 */
	char			movecount;		/* when 0, select a new dir */
	unsigned char		reactiontime;	/* if non 0, don't attack yet */
									/* used by player to freeze a bit after */
									/* teleporting */
	unsigned char		threshold;		/* if >0, the target will be chased */
									/* no matter what (even if shot) */
	unsigned char		sprite;				/* used to find patch_t and flip value */
	unsigned char		player;		/* only valid if type == MT_PLAYER */

	VINT			health;
	VINT			tics;				/* state tic counter	 */
	VINT 			state;
	VINT			frame;				/* might be ord with FF_FULLBRIGHT */

	unsigned short		type;

/* info for drawing */
	struct	mobj_s	*snext, *sprev;		/* links in sector (if needed) */
	angle_t			angle;

/* interaction info */
	struct mobj_s	*bnext, *bprev;		/* links in blocks (if needed) */
	struct subsector_s	*subsector;
	fixed_t			floorz, ceilingz;	/* closest together of contacted secs */
	fixed_t			radius, height;		/* for movement checking */
	fixed_t			momx, momy, momz;	/* momentums */
	
	unsigned 		speed;			/* mobjinfo[mobj->type].speed */
	int			flags;
	struct mobj_s	*target;		/* thing being chased/attacked (or NULL) */
									/* also the originator for missiles */
	intptr_t		extradata;		/* for latecall functions */
	unsigned short	spawnx, spawny;
	unsigned short	spawnangle;
	unsigned short	spawntype;
} mobj_t;

/* each sector has a degenmobj_t in it's center for sound origin purposes */
typedef struct
{
	struct	mobj_s	*prev, *next;
	latecall_t		latecall;
	fixed_t			x,y,z;
} degenmobj_t;		


/* */
/* frame flags */
/* */
#define	FF_FULLBRIGHT	0x8000		/* flag in thing->frame */
#define FF_FRAMEMASK	0x7fff

/* */
/* mobj flags */
/* */
#define	MF_SPECIAL		1			/* call P_SpecialThing when touched */
#define	MF_SOLID		2
#define	MF_SHOOTABLE	4
#define	MF_NOSECTOR		8			/* don't use the sector links */
									/* (invisible but touchable)  */
#define	MF_NOBLOCKMAP	16			/* don't use the blocklinks  */
									/* (inert but displayable) */
#define	MF_AMBUSH		32
#define	MF_JUSTHIT		64			/* try to attack right back */
#define	MF_JUSTATTACKED	128			/* take at least one step before attacking */
#define	MF_SPAWNCEILING	256			/* hang from ceiling instead of floor */
#define	MF_NOGRAVITY	512			/* don't apply gravity every tic */

/* movement flags */
#define	MF_DROPOFF		0x400		/* allow jumps from high places */
#define	MF_PICKUP		0x800		/* for players to pick up items */
#define	MF_NOCLIP		0x1000		/* player cheat */
#define	MF_SLIDE		0x2000		/* keep info about sliding along walls */
#define	MF_FLOAT		0x4000		/* allow moves to any height, no gravity */
#define	MF_TELEPORT		0x8000		/* don't cross lines or look at heights */
#define MF_MISSILE		0x10000		/* don't hit same species, explode on block */

#define	MF_DROPPED		0x20000		/* dropped by a demon, not level spawned */
#define	MF_SHADOW		0x40000		/* use fuzzy draw (shadow demons / invis) */
#define	MF_NOBLOOD		0x80000		/* don't bleed when shot (use puff) */
#define	MF_CORPSE		0x100000	/* don't stop moving halfway off a step */
#define	MF_INFLOAT		0x200000	/* floating to a height for a move, don't */
									/* auto float to target's height */

#define	MF_COUNTKILL	0x400000	/* count towards intermission kill total */
#define	MF_COUNTITEM	0x800000	/* count towards intermission item total */

#define	MF_SKULLFLY		0x1000000	/* skull in flight */
#define	MF_NOTDMATCH	0x2000000	/* don't spawn in death match (key cards) */

#define	MF_SEETARGET	0x4000000	/* is target visible? */

/*============================================================================= */
typedef enum
{
	PST_LIVE,			/* playing */
	PST_DEAD,			/* dead on the ground */
	PST_REBORN			/* ready to restart */
} playerstate_t;


/* psprites are scaled shapes directly on the view screen */
/* coordinates are given for a 320*200 view screen */
typedef enum
{
	ps_weapon,
	ps_flash,
	NUMPSPRITES
} psprnum_t;

typedef struct
{
	statenum_t	state;		/* a S_NULL state means not active */
	int		tics;
	fixed_t	sx, sy;
} pspdef_t;

typedef enum
{
	it_bluecard,
	it_yellowcard,
	it_redcard,
	it_blueskull,
	it_yellowskull,
	it_redskull,
	NUMCARDS
} card_t;

typedef enum
{
	wp_fist,
	wp_pistol,
	wp_shotgun,
	wp_chaingun,
	wp_missile,
	wp_plasma,
	wp_bfg,
	wp_chainsaw,
	NUMWEAPONS,
	wp_nochange
} weapontype_t;

typedef enum
{
	am_clip,		/* pistol / chaingun */
	am_shell,		/* shotgun */
	am_cell,		/* BFG */
	am_misl,		/* missile launcher */
	NUMAMMO,
	am_noammo		/* chainsaw / fist */
} ammotype_t;


typedef struct
{
	ammotype_t	ammo;
	int			upstate;
	int			downstate;
	int			readystate;
	int			atkstate;
	int			flashstate;
} weaponinfo_t;

extern	const weaponinfo_t	weaponinfo[NUMWEAPONS];

typedef enum
{
	pw_invulnerability,
	pw_strength,
	pw_ironfeet,
	pw_allmap,
	NUMPOWERS
} powertype_t;

#define	INVULNTICS		(30*15)
#define	INVISTICS		(60*15)
#define	INFRATICS		(120*15)
#define	IRONTICS		(60*15)

/*
================
=
= player_t
=
================
*/

typedef struct player_s
{
	mobj_t		*mo;
	playerstate_t	playerstate;
	
	fixed_t		forwardmove, sidemove;	/* built from ticbuttons */
	angle_t		angleturn;				/* built from ticbuttons */
	
	fixed_t		viewz;					/* focal origin above r.z */
	fixed_t		viewheight;				/* base height above floor for viewz */
	fixed_t		deltaviewheight;		/* squat speed */
	fixed_t		bob;					/* bounded/scaled total momentum */
	
	VINT		health;					/* only used between levels, mo->health */
										/* is used during levels	 */
	VINT		armorpoints, armortype;	/* armor type is 0-2 */
	
	VINT		powers[NUMPOWERS];		/* invinc and invis are tic counters	 */
	char		cards[NUMCARDS];
	char		backpack;
	VINT		frags;					/* kills of other player */
	VINT		readyweapon;
	VINT		pendingweapon;		/* wp_nochange if not changing */
	char		weaponowned[NUMWEAPONS];
	VINT		ammo[NUMAMMO];
	VINT		maxammo[NUMAMMO];
	VINT		attackdown, usedown;	/* true if button down last tic */
	VINT		cheats;					/* bit flags */
	
	int			refire;					/* refired shots are less accurate */
	
	VINT		killcount, itemcount, secretcount;		/* for intermission */
	char		*message;				/* hint messages */
	VINT		damagecount, bonuscount;/* for screen flashing */
	mobj_t		*attacker;				/* who did damage (NULL for floors) */
	int			extralight;				/* so gun flashes light up areas */
	int			colormap;				/* 0-3 for which color to draw player */
	pspdef_t	psprites[NUMPSPRITES];	/* view sprites (gun, etc) */
	boolean		didsecret;				/* true if secret level has been done */
	void		*lastsoundsector;		/* don't flood noise every time */
	
	int			automapx, automapy, automapscale, automapflags;
	int			turnheld;				/* for accelerative turning */
} player_t;

// stuff player keeps between respawns in single player
typedef struct
{
	VINT		health;
	VINT		armorpoints, armortype;
	VINT		ammo[NUMAMMO];
	VINT		maxammo[NUMAMMO];
	VINT		cheats;
	VINT		weapon;
	char		weaponowned[NUMWEAPONS];
	char		backpack;
} playerresp_t;

#define CF_NOCLIP		1
#define	CF_GODMODE		2

#define	AF_ACTIVE		1				/* automap active */
#define	AF_FOLLOW		2
#define	AF_ALLLINES		4
#define	AF_ALLMOBJ		8

#define	AF_OPTIONSACTIVE	128			/* options screen running */

/*
===============================================================================

					GLOBAL VARIABLES

===============================================================================
*/

/*================================== */

extern	int 	ticrate;	/* 4 for NTSC, 3 for PAL */
extern	int		ticsinframe;	/* how many tics since last drawer */
extern	int		ticon;
extern	int		frameon;
extern	int		ticbuttons[MAXPLAYERS];
extern	int		oldticbuttons[MAXPLAYERS];
extern	int		ticmousex[MAXPLAYERS], ticmousey[MAXPLAYERS];
extern	int		ticrealbuttons, oldticrealbuttons; /* buttons for the console player before reading the demo file */
extern	boolean		mousepresent;

int MiniLoop ( void (*start)(void),  void (*stop)(void)
		,  int (*ticker)(void), void (*drawer)(void) )
	ATTR_OPTIMIZE_SIZE;

int	G_Ticker (void);
void G_Drawer (void);
void G_RunGame (void);
void G_LoadGame(int saveslot);

/*================================== */

#include "d_mapinfo.h"

extern	gameaction_t	gameaction;

#define	SBARHEIGHT	40			/* status bar height at bottom of screen */

typedef enum
{
	gt_single,
	gt_coop,
	gt_deathmatch	
} gametype_t;

extern	gametype_t	netgame;

extern	boolean		playeringame[MAXPLAYERS];
extern	int			consoleplayer;		/* player taking events and displaying */
extern	int			displayplayer;
extern	player_t	players[MAXPLAYERS];
extern	playerresp_t	playersresp[MAXPLAYERS];

extern	int			maxammo[NUMAMMO];


extern	skill_t		gameskill;
extern	int			totalkills, totalitems, totalsecret;	/* for intermission */
extern	int			gamemaplump;
extern	dmapinfo_t	gamemapinfo;
extern	dgameinfo_t	gameinfo;

extern 	int 		gametic;
extern 	int 		prevgametic;

#define MAXDMSTARTS		10
extern	mapthing_t	*deathmatchstarts, *deathmatch_p;
extern	mapthing_t	playerstarts[MAXPLAYERS];

/*
===============================================================================

					GLOBAL FUNCTIONS

===============================================================================
*/


fixed_t	FixedMul (fixed_t a, fixed_t b);
fixed_t	FixedDiv (fixed_t a, fixed_t b);
#ifdef MARS
#define FixedMul2(c,a,b) \
       __asm volatile ( \
            "dmuls.l %1, %2\n\t" \
            "sts mach, r1\n\t" \
            "sts macl, r0\n\t" \
            "xtrct r1, r0\n\t" \
            "mov r0, %0\n\t" \
            : "=r" (c) \
            : "r" (a), "r" (b) \
            : "r0", "r1", "mach", "macl")
fixed_t IDiv (fixed_t a, fixed_t b);
#else
#define FixedMul2(c,a,b) (c = FixedMul(a,b))
#define IDiv(a,b) ((a) / (b))
#endif

#define	ACC_FIXEDMUL	4
#define	ACC_FIXEDDIV	8
#define	ACC_MULSI3		12
#define	ACC_UDIVSI3		16


#if defined(JAGUAR) || (defined(MARS)&&!defined(NeXT))
#ifndef __BIG_ENDIAN__
#define __BIG_ENDIAN__
#endif
#endif

short ShortSwap (short dat);
long LongSwap (long dat);

#ifdef __BIG_ENDIAN__

#define	BIGSHORT(x) (x)
#define	BIGLONG(x) (x)
/*define	LITTLESHORT(x) ShortSwap(x) */
#define	LITTLESHORT(x) (short)((((x)&255)<<8)+(((x)>>8)&255))
#define	LITTLELONG(x) LongSwap(x)

#else

#define	BIGSHORT(x) ShortSwap(x)
#define	BIGLONG(x) LongSwap(x)
#define	LITTLESHORT(x) (x)
#define	LITTLELONG(x) (x)

#endif



/*----------- */
/*MEMORY ZONE */
/*----------- */
/* tags < 100 are not overwritten until freed */
#define	PU_STATIC		1			/* static entire execution time */
#define	PU_SOUND		2			/* static while playing */
#define	PU_MUSIC		3			/* static while playing */
#define	PU_LEVEL		50			/* static until level exited */
#define	PU_LEVSPEC		51			/* a special thinker in a level */
/* tags >= 100 are purgable whenever needed */
#define	PU_PURGELEVEL	100
#define	PU_CACHE		101

#define	ZONEID	0x1d4a


typedef struct memblock_s
{
	int		size;           /* including the header and possibly tiny fragments */
	void    **user;         /* NULL if a free block */
	short   tag;            /* purgelevel */
	short   id;             /* should be ZONEID */
#ifndef MARS
	int		lockframe;		/* don't purge on the same frame */
#endif
	struct memblock_s   *next;
	struct memblock_s	*prev;
} memblock_t;

typedef struct
{
	int		size;				/* total bytes malloced, including header */
	memblock_t	*rover;
	memblock_t	blocklist;		/* start / end cap for linked list */
} memzone_t;

typedef void (*memblockcall_t) (void *, void*);

extern VINT framecount;

extern	memzone_t	*mainzone;
extern	memzone_t	*refzone;

void	Z_Init (void);
memzone_t *Z_InitZone (byte *base, int size);

void 	*Z_Malloc2 (memzone_t *mainzone, int size, int tag, void *user, boolean err);
void 	Z_Free2 (memzone_t *mainzone,void *ptr);

#define Z_Malloc(x,y,z) Z_Malloc2(mainzone,x,y,z,true)
#define Z_Free(x) Z_Free2(mainzone,x)

void 	Z_FreeTags (memzone_t *mainzone);
void	Z_CheckHeap (memzone_t *mainzone);
void	Z_ChangeTag (void *ptr, int tag);
int 	Z_FreeMemory (memzone_t *mainzone);
int 	Z_LargestFreeBlock(memzone_t *mainzone);
void 	Z_ForEachBlock(memzone_t *mainzone, memblockcall_t cb, void *userp);
int		Z_FreeBlocks(memzone_t* mainzone);

/*------- */
/*WADFILE */
/*------- */
typedef struct
{
	int			filepos;					/* also texture_t * for comp lumps */
	int			size;
	char		name[8];
} lumpinfo_t;

#define	MAXLUMPS	2048

extern	byte		*wadfileptr;

extern	lumpinfo_t	*lumpinfo;			/* points directly to rom image */
extern	int			numlumps;
extern	void		*lumpcache[MAXLUMPS];

void	W_Init (void);

int		W_CheckNumForName (const char *name) ATTR_OPTIMIZE_SIZE;
int		W_GetNumForName (const char *name) ATTR_OPTIMIZE_SIZE;

int		W_LumpLength (int lump);
void	W_ReadLump (int lump, void *dest);

void	*W_CacheLumpNum (int lump, int tag) ATTR_OPTIMIZE_SIZE;
void	*W_CacheLumpName (const char *name, int tag) ATTR_OPTIMIZE_SIZE;

const char *W_GetNameForNum (int lump) ATTR_OPTIMIZE_SIZE;

#define W_POINTLUMPNUM(x) (void*)(wadfileptr+BIGLONG(lumpinfo[x].filepos))


/*---------- */
/*BASE LEVEL */
/*---------- */
void D_DoomMain (void);
void D_DoomLoop (void);

extern	boolean	demoplayback, demorecording;
extern	int		*demo_p, *demobuffer;

extern	skill_t		startskill;
extern	int			startmap;
extern	gametype_t	starttype;
extern	int			startsave;

/*--------- */
/*SYSTEM IO */
/*--------- */
#define	SCREENWIDTH		252
#define	SCREENHEIGHT	180

void I_Init (void);
byte *I_WadBase (void);
byte *I_ZoneBase (int *size);

/* return a pointer to a 64k or so temp work buffer for level setup uses */
/*(non-displayed frame buffer) */
/* Vic: changed this to always return buffer memset to 0 */
byte *I_TempBuffer (void);

/* temp work buffer which may contain garbage data */
byte *I_WorkBuffer (void);

pixel_t *I_FrameBuffer (void);
pixel_t* I_OverwriteBuffer(void);

pixel_t *I_ViewportBuffer (void);
int I_ViewportYPos(void);

void I_ClearFrameBuffer (void);
void I_ClearWorkBuffer(void);

void I_SetPalette (const byte *palette);

int I_ReadControls(void);
int I_ReadMouse(int *pmx, int *pmy);

void I_NetSetup (void);
unsigned I_NetTransfer (unsigned buttons);



boolean	I_RefreshCompleted (void);
boolean	I_RefreshLatched (void);
int	I_GetTime (void);
int     I_GetFRTCounter (void);

void I_Update (void) ATTR_OPTIMIZE_SIZE;

void I_Error (char *error, ...) ATTR_OPTIMIZE_SIZE;

#ifdef MARS
//#define USE_C_DRAW

#ifdef USE_C_DRAW

#define I_DrawColumnLow I_DrawColumnLowC
#define I_DrawColumnNPo2Low I_DrawColumnNPo2LowC
#define I_DrawSpanLow I_DrawSpanLowC

#define I_DrawColumn I_DrawColumnC
#define I_DrawColumnNPo2 I_DrawColumnNPo2C
#define I_DrawSpan I_DrawSpanC

#else

#define I_DrawColumnLow I_DrawColumnLowA
#define I_DrawColumnNPo2Low I_DrawColumnNPo2LowA
#define I_DrawSpanLow I_DrawSpanLowA

#define I_DrawColumn I_DrawColumnA
#define I_DrawColumnNPo2 I_DrawColumnNPo2A
#define I_DrawSpan I_DrawSpanA

#endif

#endif

void I_DrawColumnLow(int dc_x, int dc_yl, int dc_yh, int light, fixed_t dc_iscale,
	fixed_t dc_texturemid, inpixel_t* dc_source, int dc_texheight, int *fuzzpos);

void I_DrawColumnNPo2Low(int dc_x, int dc_yl, int dc_yh, int light, fixed_t dc_iscale,
	fixed_t dc_texturemid, inpixel_t* dc_source, int dc_texheight, int *fuzzpos);

void I_DrawSpanLow(int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac,
	fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, inpixel_t* ds_source);

void I_DrawColumn(int dc_x, int dc_yl, int dc_yh, int light, fixed_t dc_iscale,
	fixed_t dc_texturemid, inpixel_t* dc_source, int dc_texheight, int* fuzzpos);

void I_DrawColumnNPo2(int dc_x, int dc_yl, int dc_yh, int light, fixed_t dc_iscale,
	fixed_t dc_texturemid, inpixel_t* dc_source, int dc_texheight, int* fuzzpos);

void I_DrawSpan(int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac,
	fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, inpixel_t* ds_source);

void I_DrawSpanPotato(int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac,
	fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, inpixel_t* ds_source)
	ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

void I_DrawSpanPotatoLow(int ds_y, int ds_x1, int ds_x2, int light, fixed_t ds_xfrac,
	fixed_t ds_yfrac, fixed_t ds_xstep, fixed_t ds_ystep, inpixel_t* ds_source)
	ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

void I_DrawFuzzColumn(int dc_x, int dc_yl, int dc_yh, int light, fixed_t frac_,
	fixed_t fracstep, inpixel_t* dc_source, int dc_texheight, int* fuzzpos);

void I_DrawFuzzColumnLow(int dc_x, int dc_yl, int dc_yh, int light, fixed_t frac_,
	fixed_t fracstep, inpixel_t* dc_source, int dc_texheight, int* fuzzpos);

void I_Print8 (int x, int y, const char *string);

void I_DebugScreen (void);

/*---- */
/*GAME */
/*---- */

void G_DeathMatchSpawnPlayer (int playernum);
void G_Init(void);
void G_InitNew (skill_t skill, int map, gametype_t gametype) ATTR_OPTIMIZE_SIZE;
void G_ExitLevel (void);
void G_SecretExitLevel (void);
void G_WorldDone (void);

void G_RecordDemo (void);
int G_PlayDemoPtr (int *demo);

/*----- */
/*PLAY */
/*----- */

void P_SetupLevel (int lumpnum, skill_t skill) ATTR_OPTIMIZE_SIZE;
void P_Init (void) ATTR_OPTIMIZE_SIZE;

void P_Start (void) ATTR_OPTIMIZE_SIZE;
void P_Stop (void) ATTR_OPTIMIZE_SIZE;
int P_Ticker (void);
void P_Drawer (void);

void IN_Start (void) ATTR_OPTIMIZE_SIZE;
void IN_Stop (void) ATTR_OPTIMIZE_SIZE;
int IN_Ticker (void) ATTR_OPTIMIZE_SIZE;
void IN_Drawer (void) ATTR_OPTIMIZE_SIZE;

void M_Start (void) ATTR_OPTIMIZE_SIZE;
void M_Start2(boolean startup) ATTR_OPTIMIZE_SIZE;
void M_Stop (void) ATTR_OPTIMIZE_SIZE;
int M_Ticker (void) ATTR_OPTIMIZE_SIZE;
void M_Drawer (void) ATTR_OPTIMIZE_SIZE;

void F_Start (void) ATTR_OPTIMIZE_SIZE;
void F_Stop (void) ATTR_OPTIMIZE_SIZE;
int F_Ticker (void) ATTR_OPTIMIZE_SIZE;
void F_Drawer (void) ATTR_OPTIMIZE_SIZE;

void AM_Control (player_t *player);
void AM_Drawer (void) ATTR_OPTIMIZE_SIZE;
void AM_Start (void) ATTR_OPTIMIZE_SIZE;

/*----- */
/*OPTIONS */
/*----- */

extern	VINT	o_musictype;

void O_Init (void) ATTR_OPTIMIZE_SIZE;
void O_Control (player_t *player) ATTR_OPTIMIZE_SIZE;
void O_Drawer (void) ATTR_OPTIMIZE_SIZE;
void O_SetButtonsFromControltype(void);


/*----- */
/*STATUS */
/*----- */

void ST_Init (void) ATTR_OPTIMIZE_SIZE;
void ST_Ticker (void) ATTR_OPTIMIZE_SIZE;
void ST_Drawer (void) ATTR_OPTIMIZE_SIZE;
void ST_ForceDraw(void) ATTR_OPTIMIZE_SIZE;
void ST_InitEveryLevel(void);


/*------- */
/*REFRESH */
/*------- */
struct seg_s;

void R_RenderPlayerView (void);
void R_Init (void);
int	R_FlatNumForName (const char *name);
int	R_TextureNumForName (const char *name);
int	R_CheckTextureNumForName (const char *name);
angle_t R_PointToAngle2 (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
struct subsector_s *R_PointInSubsector (fixed_t x, fixed_t y) ATTR_DATA_CACHE_ALIGN;


/*---- */
/*MISC */
/*---- */
int M_Random (void) ATTR_DATA_CACHE_ALIGN;
int P_Random (void) ATTR_DATA_CACHE_ALIGN;
void M_ClearRandom (void);
void M_ClearBox (fixed_t *box);
void M_AddToBox (fixed_t *box, fixed_t x, fixed_t y);


/* header generated by Dave's sound utility */
#include "sounds.h"

/*============================================================================ */
/* */
/* jag additions */
/* */
/*============================================================================ */

#ifndef MARS
extern	pixel_t	*workingscreen;
extern	int		junk, spincount;
#endif

#ifdef JAGUAR
extern	volatile int		ticcount, joybuttons;

#define BLITWAIT while ( ! ((junk=*(int *)0xf02238) & 1) )	;

#define	JP_NUM		1
#define	JP_9		2
#define	JP_6		4
#define	JP_3		8
#define	JP_0		0x10
#define	JP_8		0x20
#define	JP_5		0x40
#define	JP_2		0x80

#define	JP_OPTION	0x200
#define	JP_C		0x2000
#define JP_PWEAPN   0x4000 // CALICO: previous weapon input
#define JP_NWEAPN   0x8000 // CALICO: next weapon input
#define	JP_STAR		0x10000
#define	JP_7		0x20000
#define	JP_4		0x40000
#define	JP_1		0x80000
#define	JP_UP		0x100000
#define	JP_DOWN		0x200000
#define	JP_LEFT		0x400000
#define	JP_RIGHT	0x800000

#define	JP_B		0x2000000
#define	JP_PAUSE	0x10000000
#define	JP_A		0x20000000

#define	BT_A			JP_A
#define	BT_B			JP_B
#define	BT_C			JP_C
#define	BT_OPTION		JP_OPTION
#define	BT_PAUSE		JP_PAUSE
#define BT_STAR			JP_STAR
#define	BT_HASH			JP_NUM
#define	BT_1			JP_1
#define	BT_2			JP_2
#define	BT_3			JP_3
#define	BT_4			JP_4
#define	BT_5			JP_5
#define	BT_6			JP_6
#define	BT_7			JP_7
#define	BT_8			JP_8
#define	BT_9			JP_9
#define	BT_0			JP_0
#define	BT_PWEAPN		JP_PWEAPN
#define	BT_NWEAPN		JP_NWEAPN

#define	BT_LMBTN		0
#define	BT_RMBTN		0
#define	BT_MMBTN		0

#define BT_AUTOMAP		BT_9

#else

enum
{
	// hardware-agnostic game button actions
	// transmitted over network
	// should fit into a single word
	BT_RIGHT		= 0x1,
	BT_LEFT			= 0x2,
	BT_UP			= 0x4,
	BT_DOWN			= 0x8,

	BT_ATTACK		= 0x10,
	BT_USE			= 0x20,
	BT_STRAFE		= 0x40,
	BT_SPEED		= 0x80,

	BT_LMBTN		= 0x100,
	BT_RMBTN		= 0x200,
	BT_MMBTN		= 0x400,

	BT_PWEAPN		= 0x800,
	BT_NWEAPN		= 0x1000,

	BT_STRAFELEFT	= 0x2000,
	BT_STRAFERIGHT	= 0x4000,

	// hardware keys, which are never transmitted
	// to the other peer over network
	BT_A			= 0x10000,
	BT_B			= 0x20000,
	BT_C			= 0x40000,
	BT_START		= 0x80000,
	BT_PAUSE		= 0x100000,
	BT_OPTION		= 0x200000,
	BT_AUTOMAP		= 0x400000,
	BT_MODE			= 0x800000,
	BT_X			= 0x1000000,
	BT_Y			= 0x2000000,
	BT_Z			= 0x4000000,
};

#endif

typedef enum
{
	SFU,
	SUF,
	FSU,
	FUS,
	USF,
	UFS,
	NUMCONTROLOPTIONS
} control_t;

/* action buttons can be set to BT_A, BT_B, or BT_C */
/* strafe and use should be set to the same thing */
extern unsigned configuration[NUMCONTROLOPTIONS][3];
extern	VINT	controltype;				/* 0 to 5 */
extern	VINT	strafebtns;
extern	VINT	alwaysrun;

extern	VINT	sfxvolume, musicvolume;		/* range from 0 to 255 */

/* */
/* comnjag.c */
/*  */
extern	int		samplecount;


void C_Init (void);
void NumToStr (int num, char *str);
void PrintNumber (int x, int y, int num);


#define	BASEORGX	(7)
/*define	BASEORGY	(24) */
extern	unsigned BASEORGY;

/*================= */

typedef struct
{
	short	width;		/* in pixels */
	short	height;
	short	depth;		/* 1-5 */
	short	index;		/* location in palette of color 0 */
	short	pad1,pad2,pad3,pad4;	/* future expansion */
	byte	data[8];		/* as much as needed */
} jagobj_t;

void DoubleBufferSetup (void);
void EraseBlock (int x, int y, int width, int height);
void DrawJagobj (jagobj_t *jo, int x, int y);
void DrawJagobjLump(int lumpnum, int x, int y, int* ow, int* oh);
void UpdateBuffer (void);

#ifndef MARS
extern	byte	*bufferpage;		/* draw here */
extern	byte	*displaypage;		/* copied to here when finished */

extern	jagobj_t	*backgroundpic;

/*================= */

extern	int	gpucodestart, gpucodeend;		/* gpu funtion control */
extern int gpufinished;

extern		volatile int	dspcodestart, dspcodeend, dspfinished;

void ReloadWad (void);

int DSPFunction(void* start);

extern short *palette8;
extern int zero, ZERO, zero2;
int DSPRead(void volatile* adr);
#else

#define DSPRead(adr) (intptr_t)(*(adr))

#endif

extern	boolean		gamepaused;
extern	jagobj_t	*pausepic;

#ifndef MARS
extern	pixel_t	*screens[2];
extern	int		workpage;
#endif

void WriteEEProm (void);
void SaveGame(int slotnum);
void ReadGame(int slotnum);
void QuickSave(int nextmap);
int SaveCount(void);
int MaxSaveCount(void);
boolean GetSaveInfo(int slotnumber, VINT* mapnum, VINT* skill);

void PrintHex (int x, int y, unsigned num);
void DrawPlaque (jagobj_t *pl);
void DrawTiledBackground(void);
extern	int		maxlevel;			/* highest level selectable in menu (1-25) */

extern	int		gamevbls;			/* may not really be vbls in multiplayer */
extern	int		vblsinframe[MAXPLAYERS];		/* range from 4 to 8 */

#define MINTICSPERFRAME		2
#define MAXTICSPERFRAME		4
extern	int		ticsperframe;		/* 2 - 4 */

extern	boolean	spr_rotations;
extern int debugmode;
extern char clearscreen;

void I_InitMenuFire(void) ATTR_OPTIMIZE_SIZE;
void I_StopMenuFire(void);
int I_DrawMenuFire(void) ATTR_OPTIMIZE_EXTREME;
void I_DrawSbar(void);
void S_StartSong(int music_id, int looping, int cdtrack);
int S_SongForLump(int lump);
int S_SongForMapnum(int mapnum);
void S_StopSong(void);
void S_RestartSounds (void);

#endif
