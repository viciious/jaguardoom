/* generated by multigen */

typedef enum {
SPR_PLAY,
SPR_BBLS,
SPR_BOM1,
SPR_BOM2,
SPR_BOM3,
SPR_BUBL,
SPR_BUS1,
SPR_BUS2,
SPR_DRWN,
SPR_EGGM,
SPR_FISH,
SPR_FL01,
SPR_FL02,
SPR_FL03,
SPR_FL12,
SPR_FWR1,
SPR_FWR2,
SPR_FWR3,
SPR_GFZD,
SPR_JETF,
SPR_MISL,
SPR_MSTV,
SPR_POSS,
SPR_RING,
SPR_ROIA,
SPR_RSPR,
SPR_SCOR,
SPR_SIGN,
SPR_SPLH,
SPR_SPRK,
SPR_SPRR,
SPR_SPRY,
SPR_STPT,
SPR_TOKE,
SPR_TV1U,
SPR_TVAR,
SPR_TVAT,
SPR_TVEL,
SPR_TVFO,
SPR_TVIV,
SPR_TVRI,
SPR_TVSS,
SPR_YSPR,
SPR_GFZFENCE,
SPR_GFZGRASS,
SPR_GFZRAIL,
NUMSPRITES
} spritenum_t;

typedef enum {
S_NULL,
S_PLAY_STND,
S_PLAY_TAP1,
S_PLAY_TAP2,
S_PLAY_RUN1,
S_PLAY_RUN2,
S_PLAY_RUN3,
S_PLAY_RUN4,
S_PLAY_RUN5,
S_PLAY_RUN6,
S_PLAY_RUN7,
S_PLAY_RUN8,
S_PLAY_SPD1,
S_PLAY_SPD2,
S_PLAY_SPD3,
S_PLAY_SPD4,
S_PLAY_PAIN,
S_PLAY_DIE,
S_PLAY_DROWN,
S_PLAY_ATK1,
S_PLAY_ATK2,
S_PLAY_ATK3,
S_PLAY_ATK4,
S_PLAY_ATK5,
S_PLAY_DASH1,
S_PLAY_DASH2,
S_PLAY_DASH3,
S_PLAY_DASH4,
S_PLAY_GASP,
S_PLAY_SPRING,
S_PLAY_FALL1,
S_PLAY_FALL2,
S_PLAY_TEETER1,
S_PLAY_TEETER2,
S_PLAY_HANG,
S_POSS_STND,
S_POSS_STND2,
S_POSS_RUN1,
S_POSS_RUN2,
S_POSS_RUN3,
S_POSS_RUN4,
S_POSS_RUN5,
S_POSS_RUN6,
S_POSS_RUN7,
S_POSS_RUN8,
NUMSTATES
} statenum_t;

typedef struct
{
	 uint16_t	sprite;
	 uint16_t	frame;
	 int16_t	tics;
	 uint16_t	nextstate;
	 void		(*action) ();
} state_t;

extern const state_t	states[NUMSTATES];
extern const char * const sprnames[NUMSPRITES];


typedef enum {
MT_PLAYER,
MT_POSSESSED,
NUMMOBJTYPES
} mobjtype_t;

typedef struct {
	int16_t		doomednum;
	uint16_t	spawnstate;
	int16_t		spawnhealth;
	uint16_t	seestate;
	uint8_t		seesound;
	uint8_t		reactiontime;
	uint16_t	attacksound;
	uint16_t	painstate;
	uint16_t	painchance;
	uint16_t	painsound;
	uint16_t	meleestate;
	uint16_t	missilestate;
	uint16_t	deathstate;
	uint16_t	xdeathstate;
	uint16_t	deathsound;
	int		speed;
	int		radius;
	int		height;
	uint16_t	mass;
	uint16_t	damage;
	uint16_t	activesound;
	int		flags;
} mobjinfo_t;

extern const mobjinfo_t mobjinfo[NUMMOBJTYPES];

