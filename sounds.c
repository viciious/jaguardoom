#include "doomdef.h"

/*
 *  Information about all the music
 */

musicinfo_t		*S_music;
VINT			num_music = 0;
const VINT		mus_none = 0;

/*
 *  Information about all the sfx
 */
const char * const S_sfxnames[] =
{
    0,
    "pistol",
    "shotgn",
    "sgcock",
    "plasma",
    "bfg",
    "sawup",
    "sawidl",
    "sawful",
    "sawhit",
    "rlaunc",
    "rfly",
    "rxplod",
    "firsht",
    "firbal",
    "firxpl",
    "pstart",
    "pstop",
    "doropn",
    "dorcls",
    "stnmov",
    "swtchn",
    "swtchx",
    "plpain",
    "dmpain",
    "popain",
    "slop",
    "itemup",
    "wpnup",
    "oof",
    "telept",
    "posit1",
    "posit2",
    "posit3",
    "bgsit1",
    "bgsit2",
    "sgtsit",
    "cacsit",
    "brssit",
    "cybsit",
    "spisit",
    "sklatk",
    "sgtatk",
    "claw",
    "pldeth",
    "podth1",
    "podth2",
    "podth3",
    "bgdth1",
    "bgdth2",
    "sgtdth",
    "cacdth",
    "skldth",
    "brsdth",
    "cybdth",
    "spidth",
    "posact",
    "bgact",
    "dmact",
    "noway",
    "barexp",
    "punch",
    "hoof",
    "metal",
    "itmbk",
};

#ifdef MARS
#define SOUND(sing,pri) { sing, pri, -1 }
#else
#define SOUND(sing,pri) { sing, pri, -1, -1, NULL, NULL }
#endif

sfxinfo_t S_sfx[] =
{
    SOUND(0, 0),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 128),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(false, 64),
    SOUND(true, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(true, 70),
    SOUND(false, 100),
    SOUND(false, 100),
    SOUND(false, 100),
    SOUND(false, 100),
    SOUND(false, 100),
    SOUND(false, 78),
    SOUND(false, 78),
    SOUND(false, 96),
    SOUND(false, 96),
    SOUND(false, 96),
    SOUND(false, 78),
    SOUND(true, 78),
    SOUND(true, 78),
    SOUND(false, 96),
    SOUND(false, 32),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 98),
    SOUND(true, 94),
    SOUND(true, 92),
    SOUND(true, 90),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 32),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 70),
    SOUND(false, 32),
    SOUND(false, 32),
    SOUND(false, 32),
    SOUND(true, 120),
    SOUND(true, 120),
    SOUND(true, 120),
    SOUND(false, 78),
    SOUND(false, 60),
    SOUND(false, 64),
    SOUND(false, 120),
    SOUND(false, 120),
    SOUND(false, 120)
};

