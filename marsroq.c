/*
  Victor Luchits

  The MIT License (MIT)

  Copyright (c) 2024 Victor Luchits, Derek John Evans, id Software and ZeniMax Media

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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "32x.h"
#include "marshw.h"
#include "mars_newrb.h"
#include "roq.h"

#define RoQ_VID_BUF_SIZE        0xE000
#define RoQ_SND_BUF_SIZE        0x5000

#define RoQ_SAMPLE_RATE         22050
#define RoQ_SAMPLE_MIN          2
#define RoQ_SAMPLE_MAX          1032
#define RoQ_SAMPLE_CENTER       (RoQ_SAMPLE_MAX-RoQ_SAMPLE_MIN)/2

#define RoQ_MAX_SAMPLES         736 // 50Hz

//#define RoQ_ATTR_SDRAM  __attribute__((section(".data"), aligned(16)))
#ifndef RoQ_ATTR_SDRAM
#define RoQ_ATTR_SDRAM 
#endif

static marsrbuf_t *schunks;

static unsigned *snd_samples[2];
static int16_t snd_flip = 0;
static int16_t snd_channels = 0;
static int16_t snd_samples_rem = 0;
static int snd_lr[2];

static marsrbuf_t *vchunks;

/*
===============================================================================

						SOUND DMA CODE FOR THE SECONDARY SH-2

===============================================================================
*/

static void roq_snddma1_handler(void) RoQ_ATTR_SDRAM;

void Mars_Sec_RoQ_InitSound(int init) __attribute__((noinline));

static inline uint16_t s16pcm_to_u16pwm(int s) {
    return RoQ_SAMPLE_MIN + ((unsigned)(s+32768) >> 6);
}

static void roq_snddma_center(int f)
{
    int i;
    uint16_t *s;

    s = (uint16_t *)snd_samples[f];
    for (i = 0; i < RoQ_MAX_SAMPLES; i++)
        *s++ = RoQ_SAMPLE_CENTER;
    snd_channels = 1;
}

static void roq_snddma1_kickstart(void)
{
    snd_samples_rem = 0;

    snd_flip = 0;
    roq_snddma_center(0);

    snd_flip = 1;

    SH2_DMA_SAR1 = (intptr_t)snd_samples[0];
    SH2_DMA_TCR1 = RoQ_MAX_SAMPLES;
    SH2_DMA_DAR1 = (intptr_t)&MARS_PWM_MONO; // storing a word here will the MONO channel
    SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
}

static void roq_snddma1_handler(void)
{
    int i;
    int num_samples;
    uint8_t *buf_start;

    SH2_DMA_CHCR1; // read TE
    SH2_DMA_CHCR1 = 0; // clear TE

    if (!snd_channels)
        return;

    SH2_DMA_SAR1 = (uintptr_t)snd_samples[snd_flip];
    SH2_DMA_TCR1 = RoQ_MAX_SAMPLES;

    if (snd_channels == 2)
    {
        SH2_DMA_DAR1 = (intptr_t)&MARS_PWM_LEFT; // storing a long here will set left and right
        SH2_DMA_CHCR1 = 0x18e5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    }
    else
    {
        SH2_DMA_DAR1 = (intptr_t)&MARS_PWM_MONO; // storing a word here will set the MONO channel
        SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    }

    snd_flip = (snd_flip + 1) % 2;
    uint16_t *s = (uint16_t *)snd_samples[snd_flip];

    for (i = 0; i < RoQ_MAX_SAMPLES; )
    {
        int j, l, c;
        uint8_t *b;
        register int clamp;

        if (!snd_samples_rem)
        {
            int chunk_id;
            uint8_t *header;

            buf_start = (uint8_t *)ringbuf_ralloc(schunks, 9);
            if (!buf_start) {
                // no data yet
                break;
            }

            header = buf_start;

            Mars_ClearCacheLines(header, 2);

            chunk_id = (header[0]) | (header[1] << 8);
            if (chunk_id != RoQ_SOUND_MONO && chunk_id != RoQ_SOUND_STEREO)
            {
                header++;
                chunk_id = (header[0]) | (header[1] << 8);
            }
            header += 2;

            if (chunk_id != RoQ_SOUND_MONO && chunk_id != RoQ_SOUND_STEREO)
            {
                ringbuf_rcommit(schunks, header - buf_start);
                break;
            }

            snd_channels = (chunk_id & 1) + 1;
            snd_samples_rem = (header[0]) | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
            header += 4;

            if (snd_channels == 1)
            {
                snd_lr[0] = (int16_t)((header[0]) | (header[1] << 8));
            }
            else
            {
                snd_samples_rem /= 2;
                snd_lr[0] = (int16_t)((header[1] << 8));
                snd_lr[1] = (int16_t)((header[0] << 8));
            }
            header += 2;

            ringbuf_rcommit(schunks, header - buf_start);
        }

        num_samples = snd_samples_rem;
        if (num_samples > RoQ_MAX_SAMPLES - i)
            num_samples = RoQ_MAX_SAMPLES - i;

        l = num_samples * snd_channels;
        c = snd_channels - 1;
        buf_start = ringbuf_ralloc(schunks, l);
        b = buf_start;

        Mars_ClearCacheLines(b, ((unsigned)l >> 4) + 2);

        clamp = 32767;
        for (j = 0; j < l; j++)
        {
            int16_t v;

            v = *b & 127;
            v *= v;
            if (*b++ & 128) v = -v;

            snd_lr[j&c] += v;

            if (snd_lr[j&c] > clamp) snd_lr[j&c] = clamp;
            clamp = ~clamp;

            if (snd_lr[j&c] < clamp) snd_lr[j&c] = clamp;
            clamp = ~clamp;

            *s++ = s16pcm_to_u16pwm(snd_lr[j&c]);
        }

        snd_samples_rem -= num_samples;

        if (!snd_samples_rem) {
            // done with the current chunk
            // request a new one on the next iteration
            ringbuf_rcommit(schunks, (b - buf_start + ((intptr_t)b&1)));
        } else {
            ringbuf_rcommit(schunks, b - buf_start);
        }

        i += num_samples;
    }

    if (snd_channels == 1)
    {
        for (; i < RoQ_MAX_SAMPLES; i++)
            *s++ = RoQ_SAMPLE_CENTER;
    }
    else
    {
        for (; i < RoQ_MAX_SAMPLES; i++)
        {
            *s++ = RoQ_SAMPLE_CENTER;
            *s++ = RoQ_SAMPLE_CENTER;
        }
    }
}

void Mars_Sec_RoQ_InitSound(int init)
{
    int i;
    uint16_t sample, ix;

    Mars_ClearCache();

    if (!init)
    {
        snd_channels = 0;
        return;
    }

    for (i = 0; i < 2; i++)
    {
        roq_snddma_center(i);
    }

    // init DMA
    SH2_DMA_DAR1 = (intptr_t)&MARS_PWM_MONO; // storing a word here will the MONO channel
    SH2_DMA_TCR1 = 0;
    SH2_DMA_CHCR1 = 0;
    SH2_DMA_DRCR1 = 0;
    SH2_DMA_DMAOR = 1; // enable DMA

    // init the sound hardware
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
        MARS_PWM_CYCLE = (((23011361 << 1) / (RoQ_SAMPLE_RATE) + 1) >> 1) + 1; // for NTSC clock
    else
        MARS_PWM_CYCLE = (((22801467 << 1) / (RoQ_SAMPLE_RATE) + 1) >> 1) + 1; // for PAL clock
    MARS_PWM_CTRL = 0x0185; // TM = 1, RTP, RMD = right, LMD = left

    sample = RoQ_SAMPLE_MIN;

    // ramp up to RoQ_SAMPLE_CENTER to avoid click in audio (real 32X)
    while (sample < RoQ_SAMPLE_CENTER)
    {
        for (ix = 0; ix < (RoQ_SAMPLE_RATE * 2) / (RoQ_SAMPLE_CENTER - RoQ_SAMPLE_MIN); ix++)
        {
            while (MARS_PWM_MONO & 0x8000); // wait while full
            MARS_PWM_MONO = sample;
        }
        sample++;
    }

    Mars_SetSecDMA1Callback(&roq_snddma1_handler);

    roq_snddma1_kickstart();
}

/*
===============================================================================

						RoQ SUPPORT CODE FOR THE MAIN SH-2

===============================================================================
*/

static void roq_init_video(roq_info *ri)
{
    int i, j;
    int stretch, pitch, header, footer, stretch_height;
    unsigned short* lines = (unsigned short *) &MARS_FRAMEBUFFER;
    short *framebuffer;

    framebuffer = (short *)&MARS_FRAMEBUFFER;
    framebuffer += 0x100;

    stretch = 1;
    pitch = ri->canvas_pitch;
    stretch_height = ri->display_height;
    header = (224 - stretch_height) / 2;
    footer = header + stretch_height;

    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 0x10000; j++)
            framebuffer[j] = 0;

        for (j = 0; j < 256; j++)
        {
            if (j < header)
                lines[j] = stretch_height * pitch + 0x100;
            else if (j < footer)
                lines[j] = ((j-header)/stretch) * pitch + 0x100;
            else
                lines[j] = stretch_height * pitch + 0x100;
        }

        Mars_FlipFrameBuffers(1);
    }

    while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);
    MARS_VDP_DISPMODE = MARS_VDP_MODE_32K;
}

static void *roq_dma_dest(roq_file *fp, void *dest, int length, int dmaarg)
{
    int chunk_id = dmaarg;
    unsigned chunk_size;
    unsigned char *dma_dest;
    unsigned dstarg = (unsigned)dest;

    dstarg >>= 1; // the first bit is always 0
    dstarg >>= 1;
    chunk_size = dstarg;

    switch (chunk_id)
    {
    case RoQ_SOUND_MONO:
    case RoQ_SOUND_STEREO:
        if (!fp->snddma_base) {
            int starttics = Mars_GetTicCount();
            do {
                fp->snddma_base = (unsigned char *)ringbuf_walloc(schunks, (chunk_size + 8 + 1) & ~1);
                if (Mars_GetTicCount() - starttics > 120) {
                    // an emergency way out of a broken stream
                    break;
                }
            } while (!fp->snddma_base);

            if (!fp->snddma_base) {
                // broken file?
                fp->eof = 1;
                fp->snddma_dest = fp->backupdma_dest;
            } else {
                fp->snddma_dest = fp->snddma_base;
            }
        }

        dma_dest = fp->snddma_dest;
        fp->snddma_dest += length;
        break;
    default:
        if (!fp->dma_base) {
            fp->dma_base = ringbuf_walloc(vchunks, (chunk_size + 8 + 1) & ~1);
            if (!fp->dma_base) {
                // FIXME
                fp->eof = 1;
                fp->dma_dest = fp->backupdma_dest;
            } else {
                fp->dma_dest = fp->dma_base;
            }
        }

        dma_dest = fp->dma_dest;
        fp->dma_dest += length;
        break;
    }

    return dma_dest;
}

static int roq_commit(roq_file* fp)
{
    int audio = 0;

    // wait for ongoing transfer to finish
    while (MARS_SYS_COMM0 & 1);

    // EOF is reached and there's no data left
    if ((MARS_SYS_COMM0 & (4|8)) == (4|8))
    {
        fp->eof = 1;
    }

    if (fp->eof)
    {
        return 0;
    }

    if (fp->snddma_dest != NULL)
    {
        audio = 1;
        ringbuf_wcommit(schunks, fp->snddma_dest - fp->snddma_base);
        fp->snddma_base = NULL;
        fp->snddma_dest = NULL;
    }
    else if (fp->dma_dest != NULL)
    {
        audio = 0;
        ringbuf_wcommit(vchunks, fp->dma_dest - fp->dma_base);
        fp->dma_base = NULL;
        fp->dma_dest = NULL;
    }

    // request a new chunk
    MARS_SYS_COMM0 |= 1;

    return audio;
}

static void roq_get_chunk(roq_file* fp)
{
    uint8_t *header;
    uint8_t *chunk;
    unsigned chunk_size = 0;
    unsigned pad;

    // the initial header is strictly 8 bytes
    // other chunks may have a padding byte at the start
get_header:
    header = ringbuf_ralloc(vchunks, fp->bof ? 8 : 9);

    if (!header) {
        if (fp->eof) {
            fp->data = NULL;
            return;
        }

        while (roq_commit(fp) == 1) {
            // skip audio chunks
        }
        goto get_header;
    }

    Mars_ClearCacheLines(header, 2);

    pad = 0;
    if (header[1] != 0x10) {
        // if this is a valid chunk, the second byte must be a 0x10
        pad++;
        header++;
    }

    chunk_size = (header[2]) | (header[3] << 8) | (header[4] << 16) | (header[5] << 24);

    if (fp->bof) {
        // must be the file header
        chunk = header;
        chunk_size = 0;
    }
    else {
        chunk = ringbuf_ralloc(vchunks, (chunk_size + 8 + 1) & ~1);
        chunk += pad;
    }

    // clear the whole cache to avoid he hassle
    // we also have to clear the cache for screen copy after 
    // it has been DMA'ed to memory from DRAM, so kill two
    // birds with one stone
    Mars_ClearCache();
    //Mars_ClearCacheLines(chunk + 8, (chunk_size >> 4) + 2);

    fp->page_base = chunk;
    fp->data = chunk;
    fp->page_end = fp->data + chunk_size + 8;
}

static void roq_return_chunk(roq_file* fp)
{
    int size;

    if (!fp->data) {
        return;
    }

    size = (fp->page_end - fp->page_base + ((intptr_t)fp->page_end & 1));
    size = (size + 1) & ~1;

    fp->bof = 0;
    ringbuf_rcommit(vchunks, size);
}

static int roq_buffer(roq_file* fp)
{
    if (ringbuf_nfree(schunks) > RoQ_SND_BUF_SIZE/2 && ringbuf_nfree(vchunks) > RoQ_VID_BUF_SIZE/2) {
        roq_commit(fp);
        return 1;
    }

    return 0;
}

static int roq_open(const char *file, roq_file *fp, char *buf)
{
    int offset, length;

    length = Mars_OpenCDFileByName(file, &offset);
    if (length < 0) {
        return -1;
    }

    memset(fp, 0, sizeof(*fp));
    fp->bof = 1;
    fp->page_base = (unsigned char *)buf;
    fp->data = fp->page_base;
    fp->page_end = fp->data;

    Mars_ClearCache();

    Mars_SetPriDreqDMACallback((void *(*)(void *, void *, int , int))roq_dma_dest, fp);

    MARS_SYS_COMM8 = 0;
    MARS_SYS_COMM0 = 0x2E01; // request transfer of the first RoQ page

    return 0;
}

static void roq_close(roq_info *ri, void (*secsnd)(int init))
{
    ri->fp->eof = 1;

    while (MARS_SYS_COMM0 & 1);

    Mars_SetPriDreqDMACallback(NULL, NULL);

    ringbuf_wait(schunks);

    secsnd(0);

    MARS_SYS_COMM0 = 0;
}

int Mars_PlayRoQ(const char *fn, void *mem, size_t size, int allowpause, void (*secsnd)(int init))
{
    int displayrate;
    volatile short *framebuffer;
    char *viddata, *buf;
    uint8_t *snddata;
    roq_info *ri;
    roq_file fp;
    int paused = 0;
    int ctrl = 0, prev_ctrl = 0, ch_ctrl = 0;
    int framecount = 0;
    int extratics = 0;

    if (!allowpause && (Mars_ReadController(0) & SEGA_CTRL_START)) {
        return 0;
    }

    framebuffer = (short *)&MARS_FRAMEBUFFER;
    framebuffer += 0x100;

    ri = mem;
    ri = (void *)(((intptr_t)mem + 15) & ~15);
    memset(ri, 0, sizeof(*ri));
    buf = (char *)ri + sizeof(*ri);

    viddata = buf;
    viddata = (void *)(((intptr_t)buf + 15) & ~15);
    buf = viddata + RoQ_VID_BUF_SIZE;

    snddata = (void *)(((intptr_t)buf + 1) & ~1);
    buf = (void *)(snddata + RoQ_SND_BUF_SIZE);

    snd_samples[0] = (unsigned *)(((intptr_t)buf + 15) & ~15);
    buf = (void *)((char *)snd_samples[0] + sizeof(int)*RoQ_MAX_SAMPLES);
    snd_samples[1] = (unsigned *)(((intptr_t)buf + 15) & ~15);
    buf = (void *)((char *)snd_samples[1] + sizeof(int)*RoQ_MAX_SAMPLES);

    vchunks = (void *)(((intptr_t)buf + 15) & ~15);
    buf = (void *)((char *)vchunks + sizeof(*vchunks));

    schunks = (void *)(((intptr_t)buf + 15) & ~15);
    buf = (void *)((char *)schunks + sizeof(*schunks));

    if (buf > (char *)mem + size) {
        return -1;
    }

    ringbuf_init(vchunks, viddata, RoQ_VID_BUF_SIZE, 0);

    ringbuf_init(schunks, snddata, RoQ_SND_BUF_SIZE, 1);

    if (roq_open(fn, &fp, viddata) < 0) {
        return -1;
    }

    displayrate = MARS_VDP_DISPMODE & MARS_NTSC_FORMAT ? 60 : 50;

    if ((ri = roq_init(ri, &fp, roq_get_chunk, roq_return_chunk, displayrate, (short *)framebuffer)) == NULL) {
        return -1;
    }

	if (roq_read_info(ri->fp, ri)) {
        return -1;
    }

    // buffer some initial data
    while (roq_buffer(ri->fp) == 1) {
        ;
    }

    // init sound DMA on the secondary CPU
    secsnd(1);

    roq_init_video(ri);

    Mars_FlipFrameBuffers(0);

    extratics = 0;
    framecount = 0;

    while(1)
    {
        int ret;
        unsigned starttics;
        unsigned waittics, extrawait;
        unsigned extratwait_int, extratwait_frac;

        starttics = Mars_GetTicCount();

        prev_ctrl = ctrl;
        ctrl = Mars_ReadController(0);
        ch_ctrl = ctrl ^ prev_ctrl;

        if (framecount > 15) // ignore key presses for the first few frames
        {
            if (allowpause) {
                if (ch_ctrl & (SEGA_CTRL_A|SEGA_CTRL_B|SEGA_CTRL_C)) {
                    ri->fp->eof = 1;
                } else if (ctrl & SEGA_CTRL_START)  {
                    if (ch_ctrl & SEGA_CTRL_START) {
                        paused = !paused;
                    }
                }
            }
            else {
                if (ch_ctrl & (SEGA_CTRL_START|SEGA_CTRL_A|SEGA_CTRL_B|SEGA_CTRL_C)) {
                    ri->fp->eof = 1;
                }
            }
        }
        else
        {
            ctrl = prev_ctrl = 0;
        }

        if (!paused) {
            ret = roq_read_frame(ri, 0, Mars_WaitFrameBuffersFlip);
            if (ret <= 0) {
                roq_close(ri, secsnd);
                roq_init_video(ri);
                return 1;
            }

            Mars_FlipFrameBuffers(0);
        }

        extratwait_int = extratics >> 16;
        extratwait_frac = extratics & 0xffff;

        extrawait = extratwait_int;
        if (extrawait != 0)
            extratics = extratwait_frac;
        else
            extratics += (int)ri->frametics_frac;

        waittics = Mars_GetTicCount() - starttics;
        while (waittics < ri->frametics + extrawait) {
            if (!(MARS_SYS_COMM0 & 1)) {
                roq_buffer(ri->fp);
            }
            waittics = Mars_GetTicCount() - starttics;
        }

        framecount++;
    }

    return 0;
}
