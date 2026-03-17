/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2026 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Macintosh video code
 *
 *-----------------------------------------------------------------------------*/

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"

#include <Quickdraw.h>


#define PLANEWIDTH			 64
#define SCREENHEIGHT_MAC	342


extern const int16_t CENTERY;


static uint8_t _s_screen[VIEWWINDOWWIDTH * SCREENHEIGHT];
static uint8_t *videomemory;


void I_ReloadPalette(void)
{
}


static void I_UploadNewPalette(int8_t pal)
{
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	videomemory = qd.screenBits.baseAddr;

	//memset(videomemory, 0, VIDBYTES * VIDHEIGHT);

	videomemory += (PLANEWIDTH - VIEWWINDOWWIDTH) / 2;						// center horizontally
	videomemory += ((SCREENHEIGHT_MAC - SCREENHEIGHT) / 2) * PLANEWIDTH;	// center vertically
}


static boolean drawStatusBar = true;


void I_ShutdownGraphics(void)
{
}


static void I_DrawBuffer(uint8_t *buffer)
{
	uint8_t *src = buffer;
	uint8_t *dst = videomemory;

	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		BlockMoveData(src, dst, VIEWWINDOWWIDTH);
		dst += PLANEWIDTH;
		src += VIEWWINDOWWIDTH;
	}

	if (drawStatusBar)
	{
		for (int16_t y = 0; y < ST_HEIGHT; y++)
		{
			BlockMoveData(src, dst, VIEWWINDOWWIDTH);
			dst += PLANEWIDTH;
			src += VIEWWINDOWWIDTH;
		}
	}
	drawStatusBar = true;
}


static int8_t newpal;


void I_SetPalette(int8_t p)
{
}


#define NO_PALETTE_CHANGE 100

void I_FinishUpdate(void)
{
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	I_DrawBuffer(_s_screen);
}


void I_FinishViewWindow(void)
{
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source   = dcvars->source;
	const uint8_t *colormap = dcvars->colormap;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	uint8_t *dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];
	int16_t l = count >> 4;
	while (l--)
	{
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dest = colormap[source[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  1: *dest = colormap[source[frac >> COLBITS]];
	}
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


static uint8_t swapNibbles(uint8_t color)
{
	return (color << 4) | (color >> 4);
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t color0;
	uint8_t color1;
	uint8_t *dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

	if (dcvars->yl & 1)
	{
		color0 = swapNibbles(color);
		color1 = color;
	}
	else
	{
		color0 = color;
		color1 = swapNibbles(color);
	}

	for (int16_t i = 0; i < count / 2; i++)
	{
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
	}

	if (count & 1)
		*dest = color0;
}


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
}


void V_ClearViewWindow(void)
{
}


void V_InitDrawLine(void)
{
}


void V_ShutdownDrawLine(void)
{
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
}


void V_DrawBackground(int16_t backgroundnum)
{
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t *lump = W_TryGetLumpByNum(num);

	offset = (offset / SCREENWIDTH) * VIEWWINDOWWIDTH;

	if (lump != NULL)
	{
		uint16_t lumpLength = W_LumpLength(num);
		memcpy(&_s_screen[offset], lump, lumpLength);
		Z_ChangeTagToCache(lump);
	}
	else
		W_ReadLumpByNum(num, &_s_screen[offset]);
}


void ST_Drawer(void)
{
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
}


void wipe_StartScreen(void)
{
}


void D_Wipe(void)
{
}
