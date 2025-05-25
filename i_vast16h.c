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
 *      Video code for Atari ST 320x200 16 color, 60x128 effective resolution
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>
#include <mint/osbind.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define PLANEWIDTH				160
#define VIEWWINDOWPLANEWIDTH	120

extern const int16_t CENTERY;


static uint8_t mem_chunk[2 * 320 * 200 / 2 + 256];
static uint16_t page;
static uint8_t *page0;
static uint8_t *page1;
static uint8_t *page2;
static uint8_t *_s_screen;

static uint32_t lutc[256];

static int16_t lutx[VIEWWINDOWWIDTH / 2];
static int16_t luty[SCREENHEIGHT];
#define OFFSET(x,y) (lutx[(x)]+luty[(y)])


static  int16_t oldrez;
static uint16_t oldcolors[16];

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
	Setcolor(0, colors[pal]);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	oldrez = Getrez();
	Setscreen(-1L, -1L, 0);
	oldcolors[ 0] = Setcolor( 0, 0x000); //   0   0   0
	oldcolors[ 1] = Setcolor( 1, 0x002); //   0   0  68
	oldcolors[ 2] = Setcolor( 2, 0x121); //  51  68  34
	oldcolors[ 3] = Setcolor( 3, 0x443); // 153 136 102
	oldcolors[ 4] = Setcolor( 4, 0x300); // 119  17  17
	oldcolors[ 5] = Setcolor( 5, 0x321); // 119  68  34
	oldcolors[ 6] = Setcolor( 6, 0x432); // 136 102  85
	oldcolors[ 7] = Setcolor( 7, 0x555); // 170 170 170
	oldcolors[ 8] = Setcolor( 8, 0x333); // 102 102 102
	oldcolors[ 9] = Setcolor( 9, 0x006); //   0   0 204
	oldcolors[10] = Setcolor(10, 0x141); //  51 136  34
	oldcolors[11] = Setcolor(11, 0x753); // 238 170 119
	oldcolors[12] = Setcolor(12, 0x620); // 221  85   0
	oldcolors[13] = Setcolor(13, 0x606); // 204   0 204
	oldcolors[14] = Setcolor(14, 0x772); // 255 238  68
	oldcolors[15] = Setcolor(15, 0x777); // 255 255 255

	page0 = Physbase();
	page1 = (uint8_t *)(((uint32_t)mem_chunk | 0xff) + 1);
	page2 = page1 + (320 * 200 / 2);
	page = 1;
	_s_screen = page1;

	int16_t i = 0;
	for (int16_t y = 0; y < 16; y++)
	{
		for (int16_t x = 0; x < 16; x++)
		{
			uint32_t c = 0;
			int16_t plane = 1;
			for (int16_t p = 1; p <= 4; p++)
			{
				if (x & plane)
					if (y & plane)
						c |= (0x000000ff << (8 * (4 - p)));
					else
						c |= (0x00000055 << (8 * (4 - p)));
				else
					if (y & plane)
						c |= (0x000000aa << (8 * (4 - p)));
				plane <<= 1;
			}

			lutc[i] = c;
			i++;
		}
	}

	for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
		lutx[x] = 4 * x - 3 * (x & 1);

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
		luty[y] = y * PLANEWIDTH;
}


void I_ShutdownGraphics(void)
{
	Setscreen(-1L, page0, oldrez);
	for (int16_t c = 0; c < 16; c++)
		Setcolor(c, oldcolors[c]);
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

		if (st_needrefresh != 2)
		{
			uint8_t *s;
			if (page == 0)
				s = page2;
			else if (page == 1)
				s = page0;
			else
				s = page1;
			uint8_t *d = _s_screen;
			s += (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			d += (SCREENHEIGHT - ST_HEIGHT) * PLANEWIDTH;
			for (int16_t y = 0; y < ST_HEIGHT; y++)
			{
				memcpy(d, s, VIEWWINDOWPLANEWIDTH);
				s += PLANEWIDTH;
				d += PLANEWIDTH;
			}
		}
	}

	// page flip
	Setscreen(-1L, _s_screen, -1L);
	page++;
	if (page == 3)
		page = 0;

	if (page == 0)
		_s_screen = page0;
	else if (page == 1)
		_s_screen = page1;
	else
		_s_screen = page2;
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)

static const uint8_t* colormap;

static const uint8_t *source;
static       uint8_t *dst;
static boolean odd;

static void movep(uint32_t d, uint8_t *a)
{
#if 0
	asm (
		"movep.l %0, 0(%1)"
		:
		: "d"(d), "a"(a)
	);
#else
	if (odd)
	{
		a[0] &= 0xf0;
		a[2] &= 0xf0;
		a[4] &= 0xf0;
		a[6] &= 0xf0;
		d &= 0x0f0f0f0f;
		a[0] |= (d >> 24) & 0xff;
		a[2] |= (d >> 16) & 0xff;
		a[4] |= (d >>  8) & 0xff;
		a[6] |= (d >>  0) & 0xff;
	}
	else
	{
		a[0] &= 0x0f;
		a[2] &= 0x0f;
		a[4] &= 0x0f;
		a[6] &= 0x0f;
		d &= 0xf0f0f0f0;
		a[0] |= (d >> 24) & 0xff;
		a[2] |= (d >> 16) & 0xff;
		a[4] |= (d >>  8) & 0xff;
		a[6] |= (d >>  0) & 0xff;
	}
#endif
}


static void R_DrawColumn2(uint16_t fracstep, uint16_t frac, int16_t count)
{
	const uint8_t *colmap = colormap;
	const uint8_t *src = source;
	uint8_t *dest = dst;
	int16_t l = count >> 4;
	while (l--)
	{
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;

		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;

		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;

		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 14: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 13: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 12: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 11: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 10: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  9: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  8: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  7: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  6: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  5: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  4: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  3: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  2: movep(lutc[colmap[src[frac >> COLBITS]]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  1: movep(lutc[colmap[src[frac >> COLBITS]]], dest);
	}
}


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source = dcvars->source;

	colormap = dcvars->colormap;

	dst = &_s_screen[OFFSET(dcvars->x / 2, dcvars->yl)];
	odd = dcvars->x & 1;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	R_DrawColumn2(fracstep, frac, count);
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


static uint8_t swapNibbles(uint8_t color)
{
	return (color << 4) | (color >> 4);
}


static void R_DrawColumnFlat2(uint8_t color, int16_t yl, int16_t count)
{
	uint8_t *dest = dst;
	int16_t l = count >> 4;

	uint32_t color0;
	uint32_t color1;

	if (yl & 1)
	{
		color0 = lutc[swapNibbles(color)];
		color1 = lutc[color];
	}
	else
	{
		color0 = lutc[color];
		color1 = lutc[swapNibbles(color)];
	}

	while (l--)
	{
		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;
		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;

		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;
		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;

		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;
		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;

		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;
		movep(color0, dest); dest += PLANEWIDTH;
		movep(color1, dest); dest += PLANEWIDTH;
	}

	switch (count & 15)
	{
		case 15: movep(color0, &dest[PLANEWIDTH * 14]);
		case 14: movep(color1, &dest[PLANEWIDTH * 13]);
		case 13: movep(color0, &dest[PLANEWIDTH * 12]);
		case 12: movep(color1, &dest[PLANEWIDTH * 11]);
		case 11: movep(color0, &dest[PLANEWIDTH * 10]);
		case 10: movep(color1, &dest[PLANEWIDTH *  9]);
		case  9: movep(color0, &dest[PLANEWIDTH *  8]);
		case  8: movep(color1, &dest[PLANEWIDTH *  7]);
		case  7: movep(color0, &dest[PLANEWIDTH *  6]);
		case  6: movep(color1, &dest[PLANEWIDTH *  5]);
		case  5: movep(color0, &dest[PLANEWIDTH *  4]);
		case  4: movep(color1, &dest[PLANEWIDTH *  3]);
		case  3: movep(color0, &dest[PLANEWIDTH *  2]);
		case  2: movep(color1, &dest[PLANEWIDTH *  1]);
		case  1: movep(color0, &dest[PLANEWIDTH *  0]);
	}
}


void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dst = &_s_screen[OFFSET(dcvars->x / 2, dcvars->yl)];
	odd = dcvars->x & 1;

	R_DrawColumnFlat2(color, dcvars->yl, count);
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x08
#define FUZZCOLOR3 0x80
#define FUZZCOLOR4 0x88
#define FUZZTABLE 50

static const uint8_t fuzzcolors[FUZZTABLE] =
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

	uint8_t *dest = &_s_screen[OFFSET(dcvars->x / 2, dcvars->yl)];
	odd = dcvars->x & 1;

	static int16_t fuzzpos = 0;

	do
	{
		movep(lutc[fuzzcolors[fuzzpos]], dest);
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}


void V_ClearViewWindow(void)
{
	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
		memset(&_s_screen[y * PLANEWIDTH], 0, VIEWWINDOWPLANEWIDTH);
}


void V_InitDrawLine(void)
{
	// Do nothing
}


void V_ShutdownDrawLine(void)
{
	// Do nothing
}


static void setPixel(uint8_t *address, uint8_t bit, uint8_t color)
{
	if (color & 1)
		address[0] |=  bit;
	else
		address[0] &= ~bit;

	if (color & 2)
		address[2] |=  bit;
	else
		address[2] &= ~bit;

	if (color & 4)
		address[4] |=  bit;
	else
		address[4] &= ~bit;

	if (color & 8)
		address[6] |=  bit;
	else
		address[6] &= ~bit;
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		uint8_t *address = &_s_screen[OFFSET(x0 >> 3, y0)];
		uint8_t bit = 0x80 >> (x0 & 7);
		setPixel(address, bit, color);

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;
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
		for (int16_t x = 0; x < VIEWWINDOWPLANEWIDTH; x += 32)
		{
			uint8_t *d = &_s_screen[y * PLANEWIDTH + x];
			const byte *s = &src[((y & 63) * 32)];

			size_t len = 32;

			if (VIEWWINDOWPLANEWIDTH - x < 32)
				len = VIEWWINDOWPLANEWIDTH - x;

			memcpy(d, s, len);
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
		const uint8_t *src = lump;
		uint8_t *dest = &_s_screen[offset];
		uint16_t lumpLength = W_LumpLength(num);
		while (lumpLength)
		{
			memcpy(dest, src, VIEWWINDOWPLANEWIDTH);
			src  += VIEWWINDOWPLANEWIDTH;
			dest += PLANEWIDTH;
			lumpLength -= VIEWWINDOWPLANEWIDTH;
		}
		Z_ChangeTagToCache(lump);
	}
}


void ST_Drawer(void)
{
	if (ST_NeedUpdate())
	{
		ST_doRefresh();
		st_needrefresh = 3; // 3 screen pages
	}
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = &_s_screen[OFFSET(x >> 3, y)];
	boolean odd = (x >> 3) & 1;
	uint8_t bit = 0x80 >> (x & 7);

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + luty[column->topdelta];

			uint16_t count = column->length;

			if (count == 7)
			{
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				setPixel(dest, bit, *source++);
			}
			else
			{
				while (count--)
				{
					setPixel(dest, bit, *source++); dest += PLANEWIDTH;
				}
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}

		bit >>= 1;
		if (bit == 0)
		{
			bit = 0x80;
			desttop += odd ? 7 : 1;
			odd = !odd;
		}
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

		uint8_t bit = 0x80 >> (dc_x & 7);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte *dest = &_s_screen[OFFSET(dc_x >> 3, dc_yl)];

			int16_t frac = 0;

			const byte *source = (const byte *)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				setPixel(dest, bit, source[frac >> 8]);
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

	uint8_t *backbuffer = _s_screen;

	while (ticks--)
	{
		for (int16_t i = 0; i < VIEWWINDOWWIDTH / 2; i++)
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
					d[0] = s[0];
					d[2] = s[2];
					d[4] = s[4];
					d[6] = s[6];
					d += -PLANEWIDTH;
					s += -PLANEWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[OFFSET(i, wipe_y_lookup[i])];
				d = &frontbuffer[OFFSET(i, wipe_y_lookup[i])];

				for (int16_t j = 0 ; j < dy; j++)
				{
					d[0] = s[0];
					d[2] = s[2];
					d[4] = s[4];
					d[6] = s[6];
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
	wipe_y_lookup = Z_MallocStatic(VIEWWINDOWWIDTH / 2 * sizeof(int16_t));

	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int16_t i = 1; i < VIEWWINDOWWIDTH / 2; i++)
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
	if (page == 0)
		frontbuffer = page2;
	else if (page == 1)
		frontbuffer = page0;
	else
		frontbuffer = page1;

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

	Z_Free(frontbuffer);
	Z_Free(wipe_y_lookup);
}
