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
 *      Video code for Atari ST 320x200 16 color
 *      30x128, 60x128 and 120x128 effective resolution
 *
 *-----------------------------------------------------------------------------*/

#include <mint/cookie.h>
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
#define THIRTY					 30

extern const int16_t CENTERY;


static int16_t page;
static uint8_t *pages[3];
static uint8_t *_s_screen;


#if VIEWWINDOWWIDTH == 120
static uint8_t viewwindow[VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT];
static uint32_t lutc0[256];
static uint32_t lutc1[256];
static uint32_t lutc2[256];
static uint32_t lutc3[256];
#elif VIEWWINDOWWIDTH == 60
static uint8_t viewwindow[VIEWWINDOWWIDTH * VIEWWINDOWHEIGHT];
static uint32_t lutce[256];
static uint32_t lutco[256];
#elif VIEWWINDOWWIDTH == 30
static uint32_t lutc[256];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


static int16_t lutx[THIRTY];
static int16_t luty[SCREENHEIGHT];
#define OFFSET(x,y) (lutx[(x)]+luty[(y)])


static  int16_t oldrez;
static uint16_t oldcolors[16];

void I_ReloadPalette(void)
{
	const uint16_t *playpal = W_GetLumpByName("PLAYPAL");
	for (int16_t c = 1; c < 16; c++)
	{
		uint16_t p = playpal[c];
		uint16_t r = p >> 8;
		uint16_t g = (p >> 4) & 0x00f;
		uint16_t b = p & 0x00f;
		r += _g_gamma;
		g += _g_gamma;
		b += _g_gamma;
		if (r > 7) r = 7;
		if (g > 7) g = 7;
		if (b > 7) b = 7;
		p = (r << 8) | (g << 4) | b;
		Setcolor(c, p);
	}
	Z_ChangeTagToCache(playpal);
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

	const uint16_t *playpal = W_GetLumpByName("PLAYPAL");
	for (int16_t c = 0; c < 16; c++)
		oldcolors[c] = Setcolor(c, playpal[c]);
	Z_ChangeTagToCache(playpal);

	uint8_t *mem_chunk;
	long fastRamBuffer;
	if (Getcookie(C__FRB, &fastRamBuffer) == C_FOUND)
		mem_chunk = (uint8_t *)fastRamBuffer;
	else
		mem_chunk = Z_MallocStatic(2 * 320 * 200 / 2 + 256);

	memset(mem_chunk, 0, 2 * 320 * 200 / 2 + 256);

	pages[0] = Physbase();
	pages[1] = (uint8_t *)(((uint32_t)mem_chunk | 0xff) + 1);
	pages[2] = pages[1] + (320 * 200 / 2);
	page = 1;
	_s_screen = pages[page] + PLANEWIDTH * 20 + 16;

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

#if VIEWWINDOWWIDTH == 120
			lutc0[i] = c & 0xc0c0c0c0;
			lutc1[i] = c & 0x30303030;
			lutc2[i] = c & 0x0c0c0c0c;
			lutc3[i] = c & 0x03030303;
#elif VIEWWINDOWWIDTH == 60
			lutce[i] = c & 0xf0f0f0f0;
			lutco[i] = c & 0x0f0f0f0f;
#elif VIEWWINDOWWIDTH == 30
			lutc[i] = c;
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

			i++;
		}
	}

	for (int16_t x = 0; x < THIRTY; x++)
		lutx[x] = 4 * x - 3 * (x & 1);

	for (int16_t y = 0; y < SCREENHEIGHT; y++)
		luty[y] = y * PLANEWIDTH;
}


void I_ShutdownGraphics(void)
{
	Setscreen(-1L, pages[0], oldrez);
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
			int16_t prevpage = page - 1;
			if (prevpage == -1)
				prevpage = 2;

			uint8_t *s = pages[prevpage] + PLANEWIDTH * 20 + 16;
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
	Setscreen(-1L, pages[page], -1L);
	page++;
	if (page == 3)
		page = 0;

	_s_screen = pages[page] + PLANEWIDTH * 20 + 16;
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


static void movep(uint32_t d, uint8_t *a)
{
#if defined C_ONLY
	a[0] = (d >> 24) & 0xff;
	a[2] = (d >> 16) & 0xff;
	a[4] = (d >>  8) & 0xff;
	a[6] = (d >>  0) & 0xff;
#else
	__asm__ (
		"movep.l %[d], 0(%[a])"
		:
		: [d]"d"(d), [a]"a"(a)
	);
#endif
}


void I_FinishViewWindow(void)
{
#if VIEWWINDOWWIDTH == 120
	uint8_t *s = viewwindow;
	uint8_t *a = _s_screen;
	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH / 8; x++)
		{
			uint32_t d;
			d = lutc0[*s++]
			  | lutc1[*s++]
			  | lutc2[*s++]
			  | lutc3[*s++];
			movep(d, a);
			a += 1;

			d = lutc0[*s++]
			  | lutc1[*s++]
			  | lutc2[*s++]
			  | lutc3[*s++];
			movep(d, a);
			a += 7;
		}
		a += 40;
	}
#elif VIEWWINDOWWIDTH == 60
	uint8_t *s = viewwindow;
	uint8_t *a = _s_screen;
	for (int16_t y = 0; y < VIEWWINDOWHEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH / 4; x++)
		{
			uint32_t d;
			d = lutce[*s++]
			  | lutco[*s++];
			movep(d, a);
			a += 1;

			d = lutce[*s++]
			  | lutco[*s++];
			movep(d, a);
			a += 7;
		}
		a += 40;
	}
#endif
}


#if VIEWWINDOWWIDTH == 120 || VIEWWINDOWWIDTH == 60
void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *src = dcvars->source;

	const uint8_t *colmap = dcvars->colormap;

	uint8_t *dest = &viewwindow[dcvars->x + dcvars->yl * VIEWWINDOWWIDTH];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	int16_t l = count >> 4;
	while (l--)
	{
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;

		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		*dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 14: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 13: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 12: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 11: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case 10: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  9: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  8: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  7: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  6: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  5: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  4: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  3: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  2: *dest = colmap[src[frac >> COLBITS]]; dest += VIEWWINDOWWIDTH; frac += fracstep;
		case  1: *dest = colmap[src[frac >> COLBITS]];
	}
}
#elif VIEWWINDOWWIDTH == 30
void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *src = dcvars->source;

	const uint8_t *colmap = dcvars->colormap;

	uint8_t *dest = &_s_screen[OFFSET(dcvars->x, dcvars->yl)];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

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
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


static uint8_t swapNibbles(uint8_t color)
{
	return (color << 4) | (color >> 4);
}


#if VIEWWINDOWWIDTH == 120 || VIEWWINDOWWIDTH == 60
void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = &viewwindow[dcvars->x + dcvars->yl * VIEWWINDOWWIDTH];

	int16_t l = count >> 4;

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

	while (l--)
	{
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;

		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
	}

	switch (count & 15)
	{
		case 15: dest[VIEWWINDOWWIDTH * 14] = color0;
		case 14: dest[VIEWWINDOWWIDTH * 13] = color1;
		case 13: dest[VIEWWINDOWWIDTH * 12] = color0;
		case 12: dest[VIEWWINDOWWIDTH * 11] = color1;
		case 11: dest[VIEWWINDOWWIDTH * 10] = color0;
		case 10: dest[VIEWWINDOWWIDTH *  9] = color1;
		case  9: dest[VIEWWINDOWWIDTH *  8] = color0;
		case  8: dest[VIEWWINDOWWIDTH *  7] = color1;
		case  7: dest[VIEWWINDOWWIDTH *  6] = color0;
		case  6: dest[VIEWWINDOWWIDTH *  5] = color1;
		case  5: dest[VIEWWINDOWWIDTH *  4] = color0;
		case  4: dest[VIEWWINDOWWIDTH *  3] = color1;
		case  3: dest[VIEWWINDOWWIDTH *  2] = color0;
		case  2: dest[VIEWWINDOWWIDTH *  1] = color1;
		case  1: dest[VIEWWINDOWWIDTH *  0] = color0;
	}
}
#elif VIEWWINDOWWIDTH == 30
void R_DrawColumnFlat(uint8_t color, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = &_s_screen[OFFSET(dcvars->x, dcvars->yl)];

	int16_t l = count >> 4;

	uint32_t color0;
	uint32_t color1;

	if (dcvars->yl & 1)
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
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


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


#if VIEWWINDOWWIDTH == 120 || VIEWWINDOWWIDTH == 60
void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = &viewwindow[dcvars->x + dcvars->yl * VIEWWINDOWWIDTH];

	static int16_t fuzzpos = 0;

	do
	{
		*dest = fuzzcolors[fuzzpos];
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}
#elif VIEWWINDOWWIDTH == 30
void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	uint8_t *dest = &_s_screen[OFFSET(dcvars->x, dcvars->yl)];

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
#else
#error unsupported VIEWWINDOWWIDTH value
#endif


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


static void setPixel(uint8_t *a, int16_t x, uint32_t andmask, uint8_t color)
{
	static const uint32_t cols[16] = {
		0x00000000,
		0x80000000,
		0x00800000,
		0x80800000,
		0x00008000,
		0x80008000,
		0x00808000,
		0x80808000,
		0x00000080,
		0x80000080,
		0x00800080,
		0x80800080,
		0x00008080,
		0x80008080,
		0x00808080,
		0x80808080
	};

	uint32_t ormask = cols[color] >> x;

#if defined C_ONLY
	uint32_t d = (a[0] << 24)
	           | (a[2] << 16)
	           | (a[4] <<  8)
	           | (a[6] <<  0);

	d &= andmask;
	d |=  ormask;

	a[0] = (d >> 24) & 0xff;
	a[2] = (d >> 16) & 0xff;
	a[4] = (d >>  8) & 0xff;
	a[6] = (d >>  0) & 0xff;
#else
	uint32_t tmp = 0;
	__asm__ (
		"movep.l 0(%[dest]), %[tmp]\n"
		"and.l %[andmask],   %[tmp]\n"
		" or.l %[ormask],    %[tmp]\n"
		"movep.l %[tmp], 0(%[dest])"
		:
		: [tmp]     "d" (tmp),
		  [dest]    "a" (a),
		  [andmask] "d" (andmask),
		  [ormask]  "d" (ormask)
	);
#endif
}


void V_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)
{
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	int16_t x = x0 & 7;
	uint32_t andmask = ~(0x80808080 >> x);

	while (true)
	{
		uint8_t *address = &_s_screen[OFFSET(x0 >> 3, y0)];
		setPixel(address, x, andmask, color);

		if (x0 == x1 && y0 == y1)
			break;

		int16_t e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;

			x = x0 & 7;
			andmask = ~(0x80808080 >> x);
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
	x &= 7;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		uint32_t andmask = ~(0x80808080 >> x);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + luty[column->topdelta];

			uint16_t count = column->length;

			if (count == 7)
			{
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++);
			}
			else if (count == 3)
			{
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++);
			}
			else if (count == 5)
			{
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++);
			}
			else if (count == 6)
			{
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				setPixel(dest, x, andmask, *source++);
			}
			else
			{
				while (count--)
				{
					setPixel(dest, x, andmask, *source++); dest += PLANEWIDTH;
				}
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
		}

		x++;
		if (x == 8)
		{
			x = 0;
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

		x = dc_x & 7;
		uint32_t andmask = ~(0x80808080 >> x);

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
				setPixel(dest, x, andmask, source[frac >> 8]);
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
		for (int16_t i = 0; i < THIRTY; i++)
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
	wipe_y_lookup = Z_MallocStatic(THIRTY * sizeof(int16_t));

	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int16_t i = 1; i < THIRTY; i++)
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
	int16_t prevpage = page - 1;
	if (prevpage == -1)
		prevpage = 2;

	frontbuffer = pages[prevpage] + PLANEWIDTH * 20 + 16;

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
