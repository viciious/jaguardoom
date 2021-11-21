/*
  Victor Luchits

  The MIT License (MIT)

  Copyright (c) 2021 Victor Luchits

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef _MARS_H
#define _MARS_H

#ifdef MARS

#include "marshw.h"

void Mars_Secondary(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

void Mars_Sec_R_Setup(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
void Mars_Sec_R_WallPrep(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
void Mars_Sec_R_SegCommands(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
void Mars_Sec_R_DrawPlanes(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
void Mars_Sec_R_DrawSprites(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;
void Mars_Sec_R_DrawPSprites(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

void Mars_Sec_M_AnimateFire(void) ATTR_OPTIMIZE_EXTREME;
void Mars_Sec_InitSoundDMA(void) ATTR_OPTIMIZE_SIZE;
void Mars_Sec_StopSoundMixer(void) ATTR_OPTIMIZE_SIZE;
void Mars_Sec_StartSoundMixer(void) ATTR_OPTIMIZE_SIZE;
void Mars_Sec_ReadSoundCmds(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

void secondary_dma1_handler(void) ATTR_DATA_CACHE_ALIGN ATTR_OPTIMIZE_SIZE;

static inline void Mars_R_SecWait(void)
{
	while (MARS_SYS_COMM4 != 0);
}

static inline void Mars_CommSlaveClearCache(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 1;
}

static inline void Mars_R_SecSetup(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 2;
}

// r_phase2
static inline void Mars_R_BeginWallPrep(boolean draw)
{
	Mars_R_SecWait();
	MARS_SYS_COMM6 = 0; // next seg
	MARS_SYS_COMM4 = draw ? 4 : 3;
}

static inline void Mars_R_WallNext(void)
{
	MARS_SYS_COMM6++;
}

static inline void Mars_R_EndWallPrep(int maxsegs)
{
	MARS_SYS_COMM6 = maxsegs;
}

// r_phase7
static inline void Mars_R_BeginDrawPlanes(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 5;
}

static inline void Mars_R_EndDrawPlanes(void)
{
}

// r_phase8
static inline void Mars_R_ResetNextSprite(void)
{
	MARS_SYS_COMM6 = 0;
}

static inline void Mars_R_WaitNextSprite(int l)
{
	while (MARS_SYS_COMM6 != l) {}
}

static inline void Mars_R_AdvanceNextSprite(void)
{
	MARS_SYS_COMM6 = MARS_SYS_COMM6 + 1;
}

static inline void Mars_R_BeginDrawSprites(void)
{
	Mars_R_SecWait();
	Mars_R_ResetNextSprite();
	MARS_SYS_COMM4 = 6;
}

static inline void Mars_R_EndDrawSprites(void)
{
	while (MARS_SYS_COMM4 != 0);
}

static inline void Mars_R_BeginDrawPSprites(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 7;
}

static inline void Mars_R_EndDrawPSprites(void)
{
	Mars_R_SecWait();
}

static inline void Mars_M_BeginDrawFire(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 8;
}

static inline void Mars_M_EndDrawFire(void)
{
	MARS_SYS_COMM4 = 9;
	Mars_R_SecWait();
}

static inline void Mars_InitSoundDMA(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 10;
	Mars_R_SecWait();
}

static inline void Mars_StopSoundMixer(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 11;
	Mars_R_SecWait();
}

static inline void Mars_StartSoundMixer(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 12;
	Mars_R_SecWait();
}

#endif 

#endif // _MARS_H
