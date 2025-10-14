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
 *      Video code for Amiga 320x200 16 color
 *      30x128 effective resolution
 *
 *-----------------------------------------------------------------------------*/

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <hardware/dmabits.h>

#include <stdint.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define HORIZONTAL_RESOLUTION_LO	320
#define HORIZONTAL_RESOLUTION_HI	640

#if !defined HORIZONTAL_RESOLUTION
#define HORIZONTAL_RESOLUTION HORIZONTAL_RESOLUTION_LO
#endif

#define PLANEWIDTH			 		(4*VIEWWINDOWWIDTH)

#if HORIZONTAL_RESOLUTION == HORIZONTAL_RESOLUTION_LO
#define DDFSTRT_VALUE	0x0038
#define DDFSTOP_VALUE	0x00d0
#define BPLCON0_VALUE	0b0100001000000000
#else
#define DDFSTRT_VALUE	0x003c
#define DDFSTOP_VALUE	0x00d4
#define BPLCON0_VALUE	0b1100001000000000
#endif

#define DW	(HORIZONTAL_RESOLUTION/HORIZONTAL_RESOLUTION_LO)

#if defined VERTICAL_RESOLUTION_DOUBLED
#define DH	2
#else
#define DH	1
#endif

extern struct GfxBase *GfxBase;
extern struct Custom custom;

extern const int16_t CENTERY;


static uint8_t *videoscreen;
static uint8_t __chip videomemory[4 * HORIZONTAL_RESOLUTION / 8 * 256];
static uint8_t __chip _s_screen[4 * VIEWWINDOWWIDTH * SCREENHEIGHT];

static uint8_t lutc[256][4];

#define FMODE	0x1fc

#define DDFSTRT	0x092
#define DDFSTOP	0x094

#define DIWSTRT				0x08e
#define DIWSTOP				0x090
#define DIWSTRT_VALUE		0x2c81
#define DIWSTOP_VALUE_PAL	0x2cc1
#define DIWSTOP_VALUE_NTSC	0xf4c1

#define BPLCON0	0x100

#define BPL1MOD	0x108
#define BPL2MOD	0x10a

#define BPL1PTH	0x0e0
#define BPL1PTL	0x0e2
#define BPL2PTH	0x0e4
#define BPL2PTL	0x0e6
#define BPL3PTH	0x0e8
#define BPL3PTL	0x0ea
#define BPL4PTH	0x0ec
#define BPL4PTL	0x0ee

#define COPLIST_IDX_DIWSTOP_VALUE 9
#define COPLIST_IDX_BPL1PTH_VALUE 17
#define COPLIST_IDX_BPL1PTL_VALUE 19
#define COPLIST_IDX_BPL2PTH_VALUE 21
#define COPLIST_IDX_BPL2PTL_VALUE 23
#define COPLIST_IDX_BPL3PTH_VALUE 25
#define COPLIST_IDX_BPL3PTL_VALUE 27
#define COPLIST_IDX_BPL4PTH_VALUE 29
#define COPLIST_IDX_BPL4PTL_VALUE 31

static uint16_t __chip coplist[] = {
	FMODE,   0,
	DDFSTRT, DDFSTRT_VALUE,
	DDFSTOP, DDFSTOP_VALUE,
	DIWSTRT, DIWSTRT_VALUE,
	DIWSTOP, DIWSTOP_VALUE_PAL,
	BPLCON0, BPLCON0_VALUE,
	BPL1MOD, 160-40,
	BPL2MOD, 160-40,

	BPL1PTH, 0,
	BPL1PTL, 0,
	BPL2PTH, 0,
	BPL2PTL, 0,
	BPL3PTH, 0,
	BPL3PTL, 0,
	BPL4PTH, 0,
	BPL4PTL, 0,

	0xffff, 0xfffe, // COP_WAIT_END
	0xffff, 0xfffe  // COP_WAIT_END
};


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
		if (r > 15) r = 15;
		if (g > 15) g = 15;
		if (b > 15) b = 15;
		p = (r << 8) | (g << 4) | b;
		custom.color[c] = p;
	}
	Z_ChangeTagToCache(playpal);
}


static const uint16_t colors[14] =
{
	0x000,													// normal
	0x100, 0x300, 0x500, 0x700, 0x800, 0xa00, 0xc00, 0xe00,	// red
	0x110, 0x321, 0x541, 0x652,								// yellow
	0x020													// green
};


static void I_UploadNewPalette(int8_t pal)
{
	custom.color[0] = colors[pal];
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	LoadView(NULL);
	WaitTOF();
	WaitTOF();

	boolean pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;
	int16_t screenHeightAmiga;
	if (pal) {
		coplist[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_PAL;
		screenHeightAmiga = 256;
	} else {
		coplist[COPLIST_IDX_DIWSTOP_VALUE] = DIWSTOP_VALUE_NTSC;
		screenHeightAmiga = 200;
	}

	uint32_t addr = (uint32_t) videomemory;
	coplist[COPLIST_IDX_BPL1PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL1PTL_VALUE] = addr;
	addr += 40;
	coplist[COPLIST_IDX_BPL2PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL2PTL_VALUE] = addr;
	addr += 40;
	coplist[COPLIST_IDX_BPL3PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL3PTL_VALUE] = addr;
	addr += 40;
	coplist[COPLIST_IDX_BPL4PTH_VALUE] = addr >> 16;
	coplist[COPLIST_IDX_BPL4PTL_VALUE] = addr;

	videoscreen = videomemory;
	videoscreen += (40 - 30) / 2;										// center horizontally
	videoscreen += (screenHeightAmiga - SCREENHEIGHT) * PLANEWIDTH / 2;	// center vertically

	custom.dmacon = BITCLR | DMAF_SPRITE;
	custom.cop1lc = (uint32_t) coplist;

	const uint16_t *playpal = W_GetLumpByName("PLAYPAL");
	for (int16_t c = 0; c < 16; c++)
		custom.color[c] = playpal[c];
	Z_ChangeTagToCache(playpal);

	int16_t i = 0;
	for (int16_t y = 0; y < 16; y++)
	{
		for (int16_t x = 0; x < 16; x++)
		{
			int16_t plane = 1;
			for (int16_t p = 0; p < 4; p++)
			{
				if (x & plane)
					if (y & plane)
						lutc[i][p] = 0xff;
					else
						lutc[i][p] = 0x55;
				else
					if (y & plane)
						lutc[i][p] = 0xaa;
					else
						lutc[i][p] = 0x00;
				plane <<= 1;
			}
			i++;
		}
	}

	OwnBlitter();
	WaitBlit();
	custom.bltcon0 = 0b0000100111110000;
	custom.bltcon1 = 0;

	custom.bltamod = 0;
	custom.bltdmod = 40 - 30;

	custom.bltafwm = 0xffff;
	custom.bltalwm = 0xffff;

	custom.bltbdat = 0xffff;
	custom.bltcdat = 0xffff;
}


void I_ShutdownGraphics(void)
{
	DisownBlitter();
	LoadView(((struct GfxBase *) GfxBase)->ActiView);
	WaitTOF();
	WaitTOF();
	custom.cop1lc = (uint32_t) ((struct GfxBase *) GfxBase)->copinit;
	RethinkDisplay();
}


static boolean drawStatusBar = true;


static void I_DrawBuffer(uint8_t *buffer)
{
	WaitBlit();

	custom.bltapt = buffer;
	custom.bltdpt = videoscreen;

	if (drawStatusBar)
		custom.bltsize = ((4 * SCREENHEIGHT)               << 6) | (VIEWWINDOWWIDTH / 2);
	else
		custom.bltsize = ((4 * (SCREENHEIGHT - ST_HEIGHT)) << 6) | (VIEWWINDOWWIDTH / 2);

	drawStatusBar = true;
}


static int8_t newpal;


void I_SetPalette(int8_t p)
{
	newpal = p;
}


void I_StartUpdate(void)
{
	WaitBlit();
}


#define NO_PALETTE_CHANGE 100

static uint16_t st_needrefresh = 0;

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
	// Do nothing
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


static void setcolor(uint8_t c, uint8_t *a)
{
	a[0 * VIEWWINDOWWIDTH] = lutc[c][0];
	a[1 * VIEWWINDOWWIDTH] = lutc[c][1];
	a[2 * VIEWWINDOWWIDTH] = lutc[c][2];
	a[3 * VIEWWINDOWWIDTH] = lutc[c][3];
}


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source = dcvars->source;

	const uint8_t *colormap = dcvars->colormap;

	uint8_t *dest = &_s_screen[(dcvars->yl * PLANEWIDTH) + dcvars->x];

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	int16_t l = count >> 4;
	while (l--)
	{
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;

		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;

		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;

		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 14: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 13: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 12: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 11: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case 10: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  9: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  8: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  7: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  6: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  5: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  4: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  3: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  2: setcolor(colormap[source[frac >> COLBITS]], dest); dest += PLANEWIDTH; frac += fracstep;
		case  1: setcolor(colormap[source[frac >> COLBITS]], dest);
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

	uint8_t *dest = &_s_screen[(dcvars->yl * PLANEWIDTH) + dcvars->x];

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
		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;
		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;

		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;
		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;

		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;
		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;

		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;
		setcolor(color0, dest); dest += PLANEWIDTH;
		setcolor(color1, dest); dest += PLANEWIDTH;
	}

	switch (count & 15)
	{
		case 15: setcolor(color0, &dest[PLANEWIDTH * 14]);
		case 14: setcolor(color1, &dest[PLANEWIDTH * 13]);
		case 13: setcolor(color0, &dest[PLANEWIDTH * 12]);
		case 12: setcolor(color1, &dest[PLANEWIDTH * 11]);
		case 11: setcolor(color0, &dest[PLANEWIDTH * 10]);
		case 10: setcolor(color1, &dest[PLANEWIDTH *  9]);
		case  9: setcolor(color0, &dest[PLANEWIDTH *  8]);
		case  8: setcolor(color1, &dest[PLANEWIDTH *  7]);
		case  7: setcolor(color0, &dest[PLANEWIDTH *  6]);
		case  6: setcolor(color1, &dest[PLANEWIDTH *  5]);
		case  5: setcolor(color0, &dest[PLANEWIDTH *  4]);
		case  4: setcolor(color1, &dest[PLANEWIDTH *  3]);
		case  3: setcolor(color0, &dest[PLANEWIDTH *  2]);
		case  2: setcolor(color1, &dest[PLANEWIDTH *  1]);
		case  1: setcolor(color0, &dest[PLANEWIDTH *  0]);
	}
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

	uint8_t *dest = &_s_screen[(dcvars->yl * PLANEWIDTH) + dcvars->x];

	static int16_t fuzzpos = 0;

	do
	{
		setcolor(fuzzcolors[fuzzpos], dest);
		dest += PLANEWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}


void V_ClearViewWindow(void)
{
	memset(_s_screen, 0, 4 * VIEWWINDOWWIDTH * (SCREENHEIGHT - ST_HEIGHT));
}


void V_InitDrawLine(void)
{
	// Do nothing
}


void V_ShutdownDrawLine(void)
{
	// Do nothing
}


static void setPixel(uint8_t *a, int16_t x, uint8_t andmask, uint8_t color)
{
	static const uint8_t cols[16][4] = {
		{0x00, 0x00, 0x00, 0x00},
		{0x80, 0x00, 0x00, 0x00},
		{0x00, 0x80, 0x00, 0x00},
		{0x80, 0x80, 0x00, 0x00},
		{0x00, 0x00, 0x80, 0x00},
		{0x80, 0x00, 0x80, 0x00},
		{0x00, 0x80, 0x80, 0x00},
		{0x80, 0x80, 0x80, 0x00},
		{0x00, 0x00, 0x00, 0x80},
		{0x80, 0x00, 0x00, 0x80},
		{0x00, 0x80, 0x00, 0x80},
		{0x80, 0x80, 0x00, 0x80},
		{0x00, 0x00, 0x80, 0x80},
		{0x80, 0x00, 0x80, 0x80},
		{0x00, 0x80, 0x80, 0x80},
		{0x80, 0x80, 0x80, 0x80}
	};

	const uint8_t *ormask = &cols[color][0];

	a[0 * VIEWWINDOWWIDTH] &= andmask;
	a[0 * VIEWWINDOWWIDTH] |=  ormask[0] >> x;

	a[1 * VIEWWINDOWWIDTH] &= andmask;
	a[1 * VIEWWINDOWWIDTH] |=  ormask[1] >> x;

	a[2 * VIEWWINDOWWIDTH] &= andmask;
	a[2 * VIEWWINDOWWIDTH] |=  ormask[2] >> x;

	a[3 * VIEWWINDOWWIDTH] &= andmask;
	a[3 * VIEWWINDOWWIDTH] |=  ormask[3] >> x;
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
		uint8_t *address = &_s_screen[y0 * PLANEWIDTH + (x0 >> 3)];
		int16_t x = x0 & 7;
		uint8_t andmask = ~(0x80 >> x);
		setPixel(address, x, andmask, color);

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
		for (int16_t x = 0; x < VIEWWINDOWWIDTH; x += 8)
		{
			uint8_t *d = &_s_screen[y * PLANEWIDTH + x];
			const byte *s = &src[((y & 63) * 4 * 8)];

			size_t len = 8;

			if (VIEWWINDOWWIDTH - x < 8)
				len = VIEWWINDOWWIDTH - x;

			memcpy(d, s, len);
			d += VIEWWINDOWWIDTH;
			s +=  8;
			memcpy(d, s, len);
			d += VIEWWINDOWWIDTH;
			s +=  8;
			memcpy(d, s, len);
			d += VIEWWINDOWWIDTH;
			s +=  8;
			memcpy(d, s, len);
		}
	}

	Z_ChangeTagToCache(src);
}


void V_DrawRaw(int16_t num, uint16_t offset)
{
	const uint8_t *lump = W_TryGetLumpByNum(num);

	offset = (offset / SCREENWIDTH) * 4 * VIEWWINDOWWIDTH;

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
	if (ST_NeedUpdate())
		ST_doRefresh();
	else
		drawStatusBar = false;
}


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = &_s_screen[(y * PLANEWIDTH) + (x >> 3)];
	x &= 7;

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		uint8_t andmask = ~(0x80 >> x);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + (column->topdelta * PLANEWIDTH);

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
			desttop++;
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
		uint8_t andmask = ~(0x80 >> x);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte *dest = &_s_screen[(dc_yl * PLANEWIDTH) + (dc_x >> 3)];

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
	frontbuffer = Z_TryMallocStatic(4 * VIEWWINDOWWIDTH * SCREENHEIGHT);
	if (frontbuffer)
		memcpy(frontbuffer, _s_screen, 4 * VIEWWINDOWWIDTH * SCREENHEIGHT);
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t *backbuffer = _s_screen;

	while (ticks--)
	{
		I_DrawBuffer(frontbuffer);
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

				uint8_t *s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * PLANEWIDTH);

				uint8_t *d = &frontbuffer[i] + ((SCREENHEIGHT - 1) * PLANEWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					d[0 * VIEWWINDOWWIDTH] = s[0 * VIEWWINDOWWIDTH];
					d[1 * VIEWWINDOWWIDTH] = s[1 * VIEWWINDOWWIDTH];
					d[2 * VIEWWINDOWWIDTH] = s[2 * VIEWWINDOWWIDTH];
					d[3 * VIEWWINDOWWIDTH] = s[3 * VIEWWINDOWWIDTH];
					d += -PLANEWIDTH;
					s += -PLANEWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * PLANEWIDTH;
				d = &frontbuffer[i] + wipe_y_lookup[i] * PLANEWIDTH;

				for (int16_t j = 0 ; j < dy; j++)
				{
					d[0 * VIEWWINDOWWIDTH] = s[0 * VIEWWINDOWWIDTH];
					d[1 * VIEWWINDOWWIDTH] = s[1 * VIEWWINDOWWIDTH];
					d[2 * VIEWWINDOWWIDTH] = s[2 * VIEWWINDOWWIDTH];
					d[3 * VIEWWINDOWWIDTH] = s[3 * VIEWWINDOWWIDTH];
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
	if (!frontbuffer)
		return;

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
