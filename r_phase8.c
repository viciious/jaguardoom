/*
  CALICO

  Renderer phase 8 - Sprites
*/

#include "r_local.h"
#ifdef MARS
#include "mars.h"
#endif
#include <stdlib.h>

static int fuzzpos[2];

static boolean R_SegBehindPoint(viswall_t *viswall, int dx, int dy) ATTR_DATA_CACHE_ALIGN;
void R_DrawVisSprite(vissprite_t* vis, unsigned short* spropening, int *fuzzpos, int sprscreenhalf) ATTR_DATA_CACHE_ALIGN;
void R_ClipVisSprite(vissprite_t *vis, unsigned short *spropening, int sprscreenhalf, int16_t *walls) ATTR_DATA_CACHE_ALIGN;
static void R_DrawSortedSprites(int* sortedsprites, int *fuzzpos, int sprscreenhalf) ATTR_DATA_CACHE_ALIGN;
static void R_DrawPSprites(int *fuzzpos, int sprscreenhalf) ATTR_DATA_CACHE_ALIGN;
void R_Sprites(void) ATTR_DATA_CACHE_ALIGN __attribute__((noinline));

void R_DrawVisSprite(vissprite_t *vis, unsigned short *spropening, int *fuzzpos, int sprscreenhalf)
{
   patch_t *patch;
   fixed_t  iscale, xfrac, spryscale, sprtop, fracstep;
   int light, x, stopx;
   drawcol_t dcol;
#ifdef MARS
	inpixel_t 	*pixels;
#else
	pixel_t		*pixels;		/* data patch header references */
#endif

   patch     = W_POINTLUMPNUM(vis->patchnum);
#ifdef MARS
   pixels    = W_POINTLUMPNUM(vis->patchnum+1);
#else
   pixels    = vis->pixels;
#endif
   iscale    = FixedDiv(FRACUNIT, vis->yscale); // CALICO_FIXME: -1 in GAS... test w/o.
   xfrac     = vis->startfrac;
   spryscale = vis->yscale;
   dcol      = vis->colormap < 0 ? drawfuzzcol : drawcol;

   sprtop = FixedMul(vis->texturemid, spryscale);
   sprtop = centerYFrac - sprtop;

   // blitter iinc
   light    = vis->colormap < 0 ? -vis->colormap : vis->colormap;
   x        = vis->x1;
   stopx    = vis->x2 + 1;
   fracstep = vis->xiscale;

#ifdef MARS
   if (sprscreenhalf > 0)
   {
      if (stopx > sprscreenhalf)
         stopx = sprscreenhalf;
   }
   else if (sprscreenhalf < 0)
   {
      sprscreenhalf =- sprscreenhalf;
      if (x < sprscreenhalf)
      {
         xfrac += (sprscreenhalf - x) * fracstep;
         x = sprscreenhalf;
      }
   }
#endif

   for(; x < stopx; x++, xfrac += fracstep)
   {
      column_t *column = (column_t *)((byte *)patch + BIGSHORT(patch->columnofs[xfrac>>FRACBITS]));
      int topclip      = (spropening[x] >> 8);
      int bottomclip   = (spropening[x] & 0xff) - 1;

      // column loop
      // a post record has four bytes: topdelta length pixelofs*2
      for(; column->topdelta != 0xff; column++)
      {
         int top    = column->topdelta * spryscale + sprtop;
         int bottom = column->length   * spryscale + top;
         int count;
         fixed_t frac;

         top += (FRACUNIT - 1);
         top /= FRACUNIT;
         bottom -= 1;
         bottom /= FRACUNIT;

         // clip to bottom
         if(bottom > bottomclip)
            bottom = bottomclip;

         frac = 0;

         // clip to top
         if(topclip > top)
         {
            frac += (topclip - top) * iscale;
            top = topclip;
         }

         // calc count
         count = bottom - top + 1;
         if(count <= 0)
            continue;

         // CALICO: invoke column drawer
         dcol(x, top, bottom, light, frac, iscale, pixels + BIGSHORT(column->dataofs), 128, fuzzpos);
      }
   }
}

//
// Compare the vissprite to a viswall. Similar to R_PointOnSegSide, but less accurate.
//
static boolean R_SegBehindPoint(viswall_t *viswall, int dx, int dy)
{
   fixed_t x1, y1, sdx, sdy;
   mapvertex_t *v1 = &viswall->v1, *v2 = &viswall->v2;

   x1  = v1->x;
   y1  = v1->y;
   sdx = v2->x;
   sdy = v2->y;

   sdx -= x1;
   sdy -= y1;

   dx  -= x1;
   dy  -= y1;

   dx  *= sdy;
   sdx *=  dy;

   return (sdx < dx);
}

//
// Clip a sprite to the openings created by walls
//
void R_ClipVisSprite(vissprite_t *vis, unsigned short *spropening, int sprscreenhalf, int16_t *walls)
{
   int     x;          // r15
   int     x1;         // FP+5
   int     x2;         // r22
   unsigned scalefrac; // FP+3
   int     r1;         // FP+7
   int     r2;         // r18
   unsigned silhouette; // FP+4
   uint16_t *sil;     // FP+6
   uint16_t *opening;
   int top;        // r19
   int bottom;     // r20
   unsigned short openmark = OPENMARK;
   viswall_t *ds;      // r17

   x1  = vis->x1;
   x2  = vis->x2;
   scalefrac = vis->yscale;  

#ifdef MARS
   if (sprscreenhalf > 0)
   {
      if (x2 >= sprscreenhalf)
         x2 = sprscreenhalf - 1;
   }
   else if (sprscreenhalf < 0)
   {
      sprscreenhalf = -sprscreenhalf;
      if (x1 < sprscreenhalf)
         x1 = sprscreenhalf;
   }

   if (x1 > x2)
       return;
#endif

   for(x = x1; x <= x2; x++)
      spropening[x] = viewportHeight;

   do
   {
      ds = vd.viswalls + *walls++;

      silhouette = (ds->actionbits & (AC_TOPSIL | AC_BOTTOMSIL | AC_SOLIDSIL));
      
      if(ds->start > x2 || ds->stop < x1 ||                         // does not intersect
         (ds->scalefrac < scalefrac && ds->scale2 < scalefrac))     // is completely behind
         continue;

      if(ds->scalefrac <= scalefrac || ds->scale2 <= scalefrac)
      {
         if(R_SegBehindPoint(ds, vis->gx, vis->gy))
            continue;
      }

      r1 = ds->start < x1 ? x1 : ds->start;
      r2 = ds->stop  > x2 ? x2 : ds->stop;
      if (r1 > r2)
         continue;

      sil = ds->clipbounds + r1;
      opening = spropening + r1;
      x = r2 - r1 + 1;

      silhouette /= AC_TOPSIL;
      if(silhouette == 1)
      {
         int8_t *popn = (int8_t *)opening;
         int8_t *psil = (int8_t *)sil;
         do
         {
            if(*popn == 0)
               *popn = *psil;
            popn += 2, psil += 2;
         } while (--x);
      }
      else if(silhouette == 2)
      {
         int8_t *popn = (int8_t *)opening;
         int8_t *psil = (int8_t *)sil;
         int vph = (int8_t)viewportHeight;
         popn++, psil++;
         do
         {
            if(*popn == vph)
               *popn = *psil;
            popn += 2, psil += 2;
         } while (--x);
      }
      else if (silhouette == 3)
      {
         do
         {
            uint16_t clip = *sil;
            top    = *opening;
            bottom = top & 0xff;
            top = top & openmark;
            if(bottom == viewportHeight)
               bottom = clip & 0xff;
            if(top == 0)
               top = clip & openmark;
            *opening = top | bottom;
            opening++, sil++;
         } while (--x);
      }
      else
      {
         opening += x;
         do {
            --opening;
            *opening = openmark;
         } while (--x);
      }
   } while (*walls != -1);
}

static void R_DrawSortedSprites(int* sortedsprites, int *fuzzpos, int sprscreenhalf)
{
   int i;
   int x1, x2;
   uint16_t spropening[SCREENWIDTH];
   int count = sortedsprites[0];
   int16_t walls[MAXWALLCMDS+1], *pwalls;
   viswall_t *ds;

#ifdef MARS
   if (sprscreenhalf > 0)
   {
      x1 = 0;
      x2 = sprscreenhalf - 1;
   } else
#else
   sprscreenhalf = 0;
#endif 
   {
      x1 = -sprscreenhalf;
      x2 = viewportWidth - 1;
   }

   // compile the list of walls that clip sprites for this side part of the screen
   pwalls = walls;

   ds = vd.lastwallcmd;
   if (ds == vd.viswalls)
       return;
   do
   {
      --ds;

      if(ds->start > x2 || ds->stop < x1 ||                             // does not intersect
         !(ds->actionbits & (AC_TOPSIL | AC_BOTTOMSIL | AC_SOLIDSIL)))  // does not clip sprites
         continue;

      *pwalls++ = ds - vd.viswalls;
   } while (ds != vd.viswalls);

   if (pwalls == walls)
      return;
   *pwalls = -1;

   sortedsprites++;
   for (i = 0; i < count; i++)
   {
      vissprite_t* ds;

      ds = (vissprite_t *)(vd.vissprites + (sortedsprites[i] & 0x7f));

      R_ClipVisSprite(ds, spropening, sprscreenhalf, walls);
      R_DrawVisSprite(ds, spropening, fuzzpos, sprscreenhalf);
   }
}

static void R_DrawPSprites(int *fuzzpos, int sprscreenhalf)
{
    unsigned i;
    unsigned short spropening[SCREENWIDTH];
    viswall_t *spr;
    unsigned vph = viewportHeight;

    // draw psprites
    for (spr = vd.lastsprite_p; spr < vd.vissprite_p; spr++)
    {
        vissprite_t *vis = (vissprite_t *)spr;
        unsigned stopx = vis->x2 + 1;
        i = vis->x1;

        if (vis->patchnum < 0)
            continue;

        // clear out the clipping array across the range of the psprite
        while (i < stopx)
        {
            spropening[i] = vph;
            ++i;
        }

        R_DrawVisSprite(vis, spropening, fuzzpos, sprscreenhalf);
    }
}

#ifdef MARS
void Mars_Sec_R_DrawSprites(int sprscreenhalf)
{
    Mars_ClearCacheLine(&vd.vissprites);
    Mars_ClearCacheLine(&vd.lastsprite_p);
    Mars_ClearCacheLine(&vd.vissprite_p);
    Mars_ClearCacheLine(&vd.gsortedsprites);

    // mobj sprites
    //Mars_ClearCacheLines(vd.gsortedsprites, ((lastsprite_p - vissprites + 1) * sizeof(*vd.gsortedsprites) + 31) / 16);

    R_DrawSortedSprites(vd.gsortedsprites, &fuzzpos[1], -sprscreenhalf);

    R_DrawPSprites(&fuzzpos[1], -sprscreenhalf);
}

#endif

//
// Render all sprites
//
void R_Sprites(void)
{
   int i = 0, count;
   int half, sortedcount;
   unsigned midcount;
   viswall_t *spr;
   int *sortedsprites = (void *)vd.vissectors;
   viswall_t *wc;
   mapvertex_t *verts;

   sortedcount = 0;
   count = vd.lastsprite_p - vd.vissprites;
   if (count > MAXVISSPRITES)
       count = MAXVISSPRITES;

   // sort mobj sprites by distance (back to front)
   // find approximate average middle point for all
   // sprites - this will be used to split the draw 
   // load between the two CPUs on the 32X
   half = 0;
   midcount = 0;
   for (i = 0; i < count; i++)
   {
       vissprite_t* ds = (vissprite_t *)(vd.vissprites + i);
       if (ds->patchnum < 0)
           continue;
       if (ds->x1 > ds->x2)
           continue;

       // average mid point
       unsigned xscale = ds->xscale;
       unsigned pixcount = ds->x2 + 1 - ds->x1;
       if (pixcount > 10) // FIXME: an arbitrary number
       {
           midcount += xscale;
           half += (ds->x1 + (pixcount >> 1)) * xscale;
       }

       // composite sort key: distance + id
       sortedsprites[1+sortedcount++] = (xscale << 7) + i;
   }

   // add the gun midpoint
   for (spr = vd.lastsprite_p; spr < vd.vissprite_p; spr++) {
        vissprite_t *pspr = (vissprite_t *)spr;
        unsigned xscale;
        unsigned pixcount = pspr->x2 + 1 - pspr->x1;

        xscale = pspr->xscale;
        if (pspr->patchnum < 0 || pspr->x2 < pspr->x1)
            continue;

        midcount += xscale;
        half += (pspr->x1 + (pixcount >> 1)) * xscale;
   }

   // average the mid point
   if (midcount > 0)
   {
      half /= midcount;
      if (!half || half > viewportWidth)
         half = viewportWidth / 2;
   }

   // draw mobj sprites
   sortedsprites[0] = sortedcount;
   D_isort(sortedsprites+1, sortedcount);

#ifdef MARS
   // bank switching
   verts = /*W_GetLumpData(gamemaplump+ML_VERTEXES)*/vertexes;
#else
   verts = vertexes;
#endif

   for (wc = vd.viswalls; wc < vd.lastwallcmd; wc++)
   {
      if (wc->actionbits & (AC_TOPSIL | AC_BOTTOMSIL | AC_SOLIDSIL))
      {
         volatile int v1 = wc->seg->v1, v2 = wc->seg->v2;
         wc->v1.x = verts[v1].x, wc->v1.y = verts[v1].y;
         wc->v2.x = verts[v2].x, wc->v2.y = verts[v2].y;
      }
   }

#ifdef MARS
   Mars_R_SecWait();
   for (i = 0; i < sortedcount+1; i++)
      vd.gsortedsprites[i] = sortedsprites[i];
#endif

#ifdef MARS
   Mars_R_BeginDrawSprites(half);
#endif

   R_DrawSortedSprites(sortedsprites, &fuzzpos[0], half);

   R_DrawPSprites(&fuzzpos[0], half);

#ifdef MARS
   Mars_R_EndDrawSprites();
#endif
}

// EOF

