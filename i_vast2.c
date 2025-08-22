/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
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
 *      Video code for Atari ST 640x200 4 color (2 colors used)
 *
 *-----------------------------------------------------------------------------*/

#include <mint/osbind.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH 160

extern const int16_t CENTERY;


static uint8_t *videomemory;
static int16_t page;
static uint16_t c1;
static uint16_t c2;


static  int16_t oldrez;
static uint16_t oldc0;
static uint16_t oldc1;
static uint16_t oldc2;
static uint16_t oldc3;


void I_ReloadPalette(void)
{
	char* lumpName;
	if (_g_gamma == 0)
		lumpName = "COLORMAP";
	else
	{
		lumpName = "COLORMP0";
		lumpName[7] = '0' + _g_gamma;
	}

	W_ReadLumpByNum(W_GetNumForName(lumpName), (void *)fullcolormap);
}


static const uint16_t colors[14] =
{
	0x000,													// normal
	0x100, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700,	// red
	0x110, 0x321, 0x541, 0x652,								// yellow
	0x020													// green
};


static void I_UploadNewPalette(int8_t pal)
{
	uint16_t cdark = colors[pal];
	Setcolor(0, cdark);

	if (page == 0)
		c2 = cdark;
	else
		c1 = cdark;
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	oldrez = Getrez();
	Setscreen(-1L, -1L, 1);
	oldc0 = Setcolor(0, 0x000);
	oldc1 = Setcolor(1, 0x777);
	oldc2 = Setcolor(2, 0x000);
	oldc3 = Setcolor(3, 0x777);

	videomemory = Physbase();
	videomemory += 20;				// center horizontally
	videomemory += 20 * PLANEWIDTH;	// center vertically

	videomemory += 2;
	page = 1;
	c1 = 0x000;
	c2 = 0x777;
}


void I_ShutdownGraphics(void)
{
	Setscreen(-1L, -1L, oldrez);
	Setcolor(0, oldc0);
	Setcolor(1, oldc1);
	Setcolor(2, oldc2);
	Setcolor(3, oldc3);
}


static int8_t newpal;


void I_SetPalette(int8_t p)
{
	newpal = p;
}


#define NO_PALETTE_CHANGE 100

static int16_t st_needrefresh = 0;

void I_FinishUpdate(void)
{
	// palette
	if (newpal != NO_PALETTE_CHANGE)
	{
		I_UploadNewPalette(newpal);
		newpal = NO_PALETTE_CHANGE;
	}

	// status bar
	if (st_needrefresh)
	{
		st_needrefresh--;

		if (st_needrefresh == 0)
		{
			uint16_t *s = (uint16_t *)videomemory;
			if (page == 0)
				s++;
			else
				s--;
			s += (SCREENHEIGHT - ST_HEIGHT) * (PLANEWIDTH / 2);
			uint16_t *d = (uint16_t *)videomemory;
			d += (SCREENHEIGHT - ST_HEIGHT) * (PLANEWIDTH / 2);
			for (int16_t y = 0; y < ST_HEIGHT; y++)
			{
				for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
				{
					*d = *s;
					s += 2;
					d += 2;
				}
				s += 20;
				d += 20;
			}
		}
	}

	// page flip
	Setcolor(1, c1);
	Setcolor(2, c2);
	uint16_t ctemp = c1;
	c1 = c2;
	c2 = ctemp;

	videomemory += (2 - 4 * page);
	page = 1 - page;
}


void I_FinishViewWindow(void)
{
	// Do nothing
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


#define OFFSET(x,y) ((y)*PLANEWIDTH+2*(x)-((x)&1))


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *src = dcvars->source;

	const uint8_t *colmap = dcvars->colormap;

	uint8_t *dest = &videomemory[OFFSET(dcvars->x, dcvars->yl)];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	int16_t l = count >> 4;
	while (l--)
	{
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 14: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 13: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 12: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 11: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case 10: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  9: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  8: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  7: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  6: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  5: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  4: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  3: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  2: *dest = colmap[src[frac >> COLBITS]]; dest += PLANEWIDTH; frac += fracstep;
		case  1: *dest = colmap[src[frac >> COLBITS]];
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

	uint8_t *dest = &videomemory[OFFSET(dcvars->x, dcvars->yl)];

	uint8_t color0;
	uint8_t color1;

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

	int16_t l = count >> 4;

	while (l--)
	{
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;

		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
		*dest = color0; dest += PLANEWIDTH;
		*dest = color1; dest += PLANEWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[PLANEWIDTH * 14] = color0;
		case 14: dest[PLANEWIDTH * 13] = color1;
		case 13: dest[PLANEWIDTH * 12] = color0;
		case 12: dest[PLANEWIDTH * 11] = color1;
		case 11: dest[PLANEWIDTH * 10] = color0;
		case 10: dest[PLANEWIDTH *  9] = color1;
		case  9: dest[PLANEWIDTH *  8] = color0;
		case  8: dest[PLANEWIDTH *  7] = color1;
		case  7: dest[PLANEWIDTH *  6] = color0;
		case  6: dest[PLANEWIDTH *  5] = color1;
		case  5: dest[PLANEWIDTH *  4] = color0;
		case  4: dest[PLANEWIDTH *  3] = color1;
		case  3: dest[PLANEWIDTH *  2] = color0;
		case  2: dest[PLANEWIDTH *  1] = color1;
		case  1: dest[PLANEWIDTH *  0] = color0;
	}
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x02
#define FUZZCOLOR3 0x20
#define FUZZCOLOR4 0x22
#define FUZZTABLE 50

static const int8_t fuzzcolors[FUZZTABLE] =
{
	FUZZCOLOR1,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR2,
	FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,
	FUZZCOLOR1,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR1,FUZZCOLOR4,
	FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,
	FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR4,FUZZCOLOR2,FUZZCOLOR1,FUZZCOLOR3,
	FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR4,FUZZCOLOR1,FUZZCOLOR3,FUZZCOLOR2,FUZZCOLOR1
};


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = &videomemory[OFFSET(dcvars->x, dcvars->yl)];

	static int16_t fuzzpos = 0;

	do
	{
		*dest = fuzzcolors[fuzzpos];
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}


void V_ClearViewWindow(void)
{
	uint16_t *dst = (uint16_t *)videomemory;
	for (int16_t y = 0; y < (SCREENHEIGHT - ST_HEIGHT); y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
		{
			*dst = 0;
			dst += 2;
		}
		dst += 20;
	}
}


void V_InitDrawLine(void)
{
	// Do nothing
}


void V_ShutdownDrawLine(void)
{
	// Do nothing
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	static const uint8_t bitmasks8[8] = {0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe};

	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	int16_t p = x0 & 7;
	uint8_t bitmask = bitmasks8[p];

	while (true)
	{
		uint8_t c = videomemory[OFFSET(x0 >> 3, y0)];
		videomemory[OFFSET(x0 >> 3, y0)] = (c & bitmask) | (color >> p);

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;

			p       = x0 & 7;
			bitmask = bitmasks8[p];
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}
}


void V_DrawBackground(int16_t backgroundnum)
{
	const byte *src = W_GetLumpByNum(backgroundnum);

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x += 16)
		{
			      uint16_t *d = (uint16_t *)&videomemory[OFFSET(x, y)];
			const uint16_t *s = (uint16_t *)&src[((y & 63) * 16)];

			size_t len = 16;

			if (VIEWWINDOWWIDTH - x < 16)
				len = VIEWWINDOWWIDTH - x;

			for (int16_t tx = 0; tx < len / 2; tx++)
			{
				*d = *s++;
				d += 2;
			}
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t *lump = W_TryGetLumpByNum(num);

	if (lump != NULL)
	{
		offset = (offset / SCREENWIDTH) * PLANEWIDTH;
		uint16_t *src  = (uint16_t *)lump;
		uint16_t *dest = (uint16_t *)&videomemory[offset];
		uint16_t lumpLength = W_LumpLength(num);
		while (lumpLength)
		{
			for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
			{
				*dest = *src++;
				dest += 2;
			}
			dest += 20;
			lumpLength -= VIEWWINDOWWIDTH;
		}
		Z_ChangeTagToCache(lump);
	}
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = 2; // 2 screen pages
	}
}


static const uint8_t bitmasks4[4] = {0x3f, 0xcf, 0xf3, 0xfc};


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = &videomemory[OFFSET(x >> 2, y)];
	boolean odd = (x >> 2) & 1;

	int16_t width = patch->width;

	int16_t p = x & 3;
	uint8_t bitmask = bitmasks4[p];

	for (int16_t col = 0; col < width; col++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + (column->topdelta * PLANEWIDTH);

			uint16_t count = column->length;

			uint8_t c;
			uint8_t color;

			if (count == 7)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 3)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 5)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 6)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else
			{
				while (count--)
				{
					c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += PLANEWIDTH;
				}
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}

		p++;
		if (p == 4)
		{
			p = 0;
			desttop += odd ? 3 : 1;
			odd = !odd;
		}
		bitmask = bitmasks4[p];
	}
}


void V_DrawPatchScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	static const int32_t   DX  = (((int32_t)SCREENWIDTH)<<FRACBITS) / SCREENWIDTH_VGA;
	static const int16_t   DXI = ((((int32_t)SCREENWIDTH_VGA)<<FRACBITS) / SCREENWIDTH) >> 8;
	static const int32_t   DY  = ((((int32_t)SCREENHEIGHT)<<FRACBITS)+(FRACUNIT-1)) / SCREENHEIGHT_VGA;
	static const int16_t   DYI = ((((int32_t)SCREENHEIGHT_VGA)<<FRACBITS) / SCREENHEIGHT) >> 8;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	const int16_t left   = ( x * DX ) >> FRACBITS;
	const int16_t right  = ((x + patch->width)  * DX) >> FRACBITS;
	const int16_t bottom = ((y + patch->height) * DY) >> FRACBITS;

	uint16_t   col = 0;

	for (int16_t dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col >> 8]);

		int16_t p = dc_x & 3;
		uint8_t bitmask = bitmasks4[p];

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte *dest = &videomemory[OFFSET(dc_x >> 2, dc_yl)];

			int16_t frac = 0;

			const byte *source = (const byte *)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				uint8_t c = *dest;
				uint8_t color = source[frac >> 8];
				*dest = (c & bitmask) | (color >> (p * 2));
				dest += PLANEWIDTH;
				frac += DYI;
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}
	}
}


static uint8_t *frontbuffer;
static int16_t *wipe_y_lookup;


void wipe_StartScreen(void)
{
	// Do nothing
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t *backbuffer = videomemory;

	while (ticks--)
	{
		for (int16_t i = 0; i < VIEWWINDOWWIDTH; i++)
		{
			if (wipe_y_lookup[i] < 0)
			{
				wipe_y_lookup[i]++;
				done = false;
				continue;
			}

			// scroll down columns, which are still visible
			if (wipe_y_lookup[i] < SCREENHEIGHT)
			{
				int16_t dy = (wipe_y_lookup[i] < 16) ? wipe_y_lookup[i] + 1 : SCREENHEIGHT / 25;
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				uint8_t *s = &frontbuffer[OFFSET(i, SCREENHEIGHT - dy - 1)];

				uint8_t *d = &frontbuffer[OFFSET(i, SCREENHEIGHT - 1)];

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -PLANEWIDTH;
					s += -PLANEWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[OFFSET(i, wipe_y_lookup[i])];
				d = &frontbuffer[OFFSET(i, wipe_y_lookup[i])];

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += PLANEWIDTH;
					s += PLANEWIDTH;
				}

				wipe_y_lookup[i] += dy;
				done = false;
			}
		}
	}

	return done;
}


static void wipe_initMelt()
{
	wipe_y_lookup = Z_MallocStatic(VIEWWINDOWWIDTH * sizeof(int16_t));

	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int16_t i = 1; i < VIEWWINDOWWIDTH; i++)
	{
		int16_t r = (M_Random() % 3) - 1;

		wipe_y_lookup[i] = wipe_y_lookup[i - 1] + r;

		if (wipe_y_lookup[i] > 0)
			wipe_y_lookup[i] = 0;
		else if (wipe_y_lookup[i] == -16)
			wipe_y_lookup[i] = -15;
	}
}


void D_Wipe(void)
{
	frontbuffer = videomemory;
	if (page == 0)
		frontbuffer += 2;
	else
		frontbuffer -= 2;

	wipe_initMelt();

	boolean done;
	int32_t wipestart = I_GetTime() - 1;

	do
	{
		int32_t nowtime;
		int16_t tics;
		do
		{
			nowtime = I_GetTime();
			tics = nowtime - wipestart;
		} while (!tics);

		wipestart = nowtime;
		done = wipe_ScreenWipe(tics);

		M_Drawer();                   // menu is drawn even on top of wipes

	} while (!done);

	Z_Free(wipe_y_lookup);
}
