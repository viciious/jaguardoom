/* marsonly.c */

#include "doomdef.h"
#include "r_local.h"
#include "mars.h"
#include <stdio.h>

void ReadEEProm (void);

/* 
================ 
= 
= Mars_main  
= 
================ 
*/ 
 
int main(void)
{
	int i;
	volatile unsigned short* palette;

	Mars_Init();

/* clear screen */
	if (Mars_IsPAL()) {
		/* use letter-boxed 240p mode */
		Mars_InitVideo(-240);
	} else {
		Mars_InitVideo(224);
	}

	/* set a two color palette */
	Mars_FlipFrameBuffers(false);
	palette = &MARS_CRAM;
	for (i = 0; i < 256; i++)
		palette[i] = 0;
	palette[COLOR_WHITE] = 0x7fff;
	Mars_WaitFrameBuffersFlip();

	ticrate = Mars_RefreshHZ() / TICRATE;

/* */
/* load defaults */
/* */
	ReadEEProm();
#if 0
	{
		char temp[2][12];
		int off;
		char *src;

		int res = Mars_OpenCDFile("MAPS.WAD");
		src = Mars_GetCDFileBuffer();
		D_memcpy(temp, src, mystrlen(src)+1);
		Mars_ReadCDFile(4);
		src[4] = 0;
		D_memcpy(temp[0], src, 12);

		off = Mars_SeekCDFile(12, SEEK_CUR);
		src = Mars_GetCDFileBuffer();
		Mars_ReadCDFile(4);
		src[4] = 0;
		off = Mars_SeekCDFile(0, SEEK_CUR);
		D_memcpy(temp[1], src, 4);

		I_Error("%d %d %02X%02X%02X%02X", res, off, temp[1][0] & 0xff, temp[1][1] & 0xff, temp[1][2] & 0xff, temp[1][3] & 0xff);
	}
#endif
/* */
/* start doom */
/* */
	D_DoomMain ();

	return 0;
}

void secondary()
{
	Mars_Secondary();
}

/*
==============================================================================

						DOOM INTERFACE CODE

==============================================================================
*/

static const byte font8[] =
{
   0,0,0,0,0,0,0,0,24,24,24,24,24,0,24,0,
   54,54,54,0,0,0,0,0,108,108,254,108,254,108,108,0,
   48,124,192,120,12,248,48,0,198,204,24,48,102,198,0,0,
   56,108,56,118,220,204,118,0,24,24,48,0,0,0,0,0,
   6,12,24,24,24,12,6,0,48,24,12,12,12,24,48,0,
   0,102,60,255,60,102,0,0,0,24,24,126,24,24,0,0,
   0,0,0,0,48,48,96,0,0,0,0,0,126,0,0,0,
   0,0,0,0,0,96,96,0,6,12,24,48,96,192,128,0,
   56,108,198,198,198,108,56,0,24,56,24,24,24,24,60,0,
   248,12,12,56,96,192,252,0,248,12,12,56,12,12,248,0,
   28,60,108,204,254,12,12,0,252,192,248,12,12,12,248,0,
   60,96,192,248,204,204,120,0,252,12,24,48,96,192,192,0,
   120,204,204,120,204,204,120,0,120,204,204,124,12,12,120,0,
   0,96,96,0,96,96,0,0,0,96,96,0,96,96,192,0,
   12,24,48,96,48,24,12,0,0,0,252,0,0,252,0,0,
   96,48,24,12,24,48,96,0,124,6,6,28,24,0,24,0,
   124,198,222,222,222,192,120,0,48,120,204,204,252,204,204,0,
   248,204,204,248,204,204,248,0,124,192,192,192,192,192,124,0,
   248,204,204,204,204,204,248,0,252,192,192,248,192,192,252,0,
   252,192,192,248,192,192,192,0,124,192,192,192,220,204,124,0,
   204,204,204,252,204,204,204,0,60,24,24,24,24,24,60,0,
   12,12,12,12,12,12,248,0,198,204,216,240,216,204,198,0,
   192,192,192,192,192,192,252,0,198,238,254,214,198,198,198,0,
   198,230,246,222,206,198,198,0,120,204,204,204,204,204,120,0,
   248,204,204,248,192,192,192,0,120,204,204,204,204,216,108,0,
   248,204,204,248,240,216,204,0,124,192,192,120,12,12,248,0,
   252,48,48,48,48,48,48,0,204,204,204,204,204,204,124,0,
   204,204,204,204,204,120,48,0,198,198,198,214,254,238,198,0,
   198,198,108,56,108,198,198,0,204,204,204,120,48,48,48,0,
   254,12,24,48,96,192,254,0,120,96,96,96,96,96,120,0,
   192,96,48,24,12,6,0,0,120,24,24,24,24,24,120,0,
   24,60,102,66,0,0,0,0,0,0,0,0,0,0,255,0,
   192,192,96,0,0,0,0,0,0,0,120,12,124,204,124,0,
   192,192,248,204,204,204,248,0,0,0,124,192,192,192,124,0,
   12,12,124,204,204,204,124,0,0,0,120,204,252,192,124,0,
   60,96,96,248,96,96,96,0,0,0,124,204,124,12,248,0,
   192,192,248,204,204,204,204,0,24,0,56,24,24,24,60,0,
   24,0,24,24,24,24,240,0,192,192,204,216,240,216,204,0,
   56,24,24,24,24,24,60,0,0,0,236,214,214,214,214,0,
   0,0,248,204,204,204,204,0,0,0,120,204,204,204,120,0,
   0,0,248,204,204,248,192,0,0,0,124,204,204,124,12,0,
   0,0,220,224,192,192,192,0,0,0,124,192,120,12,248,0,
   96,96,252,96,96,96,60,0,0,0,204,204,204,204,124,0,
   0,0,204,204,204,120,48,0,0,0,214,214,214,214,110,0,
   0,0,198,108,56,108,198,0,0,0,204,204,124,12,248,0,
   0,0,252,24,48,96,252,0,28,48,48,224,48,48,28,0,
   24,24,24,0,24,24,24,0,224,48,48,28,48,48,224,0,
   118,220,0,0,0,0,0,0,252,252,252,252,252,252,252,0
};

//
// Print a debug message.
// CALICO: Rewritten to expand 1-bit graphics
//
int I_Print8Len(const char* string)
{
	int c;
	int len = 0;
	int ckey = 0;

	while ((c = *string++))
	{
		if (ckey)
		{
			ckey--;
			continue;
		}

		if (c == '^')
		{
			ckey = 2;
			continue;
		}

		len++;
	}

	return len;
}

static int hextoi(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 0;
}

void I_Print8(int x, int y, const char* string)
{
	int c;
	int ckey = 0;
	int color = COLOR_WHITE;
	int16_t colortbl[4];
	const uint8_t* source;
	int16_t *dest;

	if (y > (224-16) / 8)
		return;

	colortbl[0] = 0;
	colortbl[1] = color;
	colortbl[2] = color << 8;
	colortbl[3] = (color << 8) | color;

	dest = (int16_t *)(I_OverwriteBuffer() + 320 + (y * 8) * 160 + x/2);
	while ((c = *string++) && x < 320-8)
	{
		int i;
		int16_t * d;

		if (ckey)
		{
			color = (color<<4)|hextoi(c);
			ckey--;
			if (!ckey)
			{
				colortbl[1] = color;
				colortbl[2] = color << 8;
				colortbl[3] = (color << 8) | color;
			}
			continue;
		}
		else
		{
			if (c == '^')
			{
				color = 0;
				ckey = 2;
				continue;
			}
		}

		if (c < 32 || c >= 128)
		{
			dest += 4;
			++x;
			continue;
		}

		source = (uint8_t *)font8 + ((unsigned)(c - 32) << 3);
		d = dest;
		for (i = 0; i < 7; i++)
		{
			uint8_t s = *source++;
			d[3] = colortbl[s & 3], s >>= 2;
			d[2] = colortbl[s & 3], s >>= 2;
			d[1] = colortbl[s & 3], s >>= 2;
			d[0] = colortbl[s & 3], s >>= 2;
			d += 160;
		}

		dest+=4;
		++x;
	}
}

/*
================ 
=
= I_Error
=
================
*/

void I_Error (char *error, ...) 
{
	va_list ap;
	char errormessage[80];

	va_start(ap, error);
	D_vsnprintf(errormessage, sizeof(errormessage), error, ap);
	va_end(ap);

	I_ClearFrameBuffer();
	I_Print8 (0,20,errormessage);
	I_Update ();

	while (1)
	;
} 
