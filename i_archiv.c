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
 *      Video code for Archimedes 320x256 256 color
 *
 *-----------------------------------------------------------------------------*/

#include <archie/SWI.h>
#include <archie/video.h>

#include "compiler.h"

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"

#include "globdata.h"


#define SCREENWIDTH_ARCHIMEDES  320
#define SCREENHEIGHT_ARCHIMEDES 256


extern const int16_t CENTERY;


static int16_t page;
static uint8_t *pages[3];
static uint8_t *_s_screen;


void I_ReloadPalette(void)
{
	const uint16_t *playpal = W_GetLumpByName("PLAYPAL");
	for (int16_t c = 1; c < 16; c++)
	{
		uint16_t p = playpal[c];
		uint16_t r = (p >> 8);
		uint16_t g = (p >> 4) & 0x00f;
		uint16_t b = (p >> 0) & 0x00f;

		r += _g_gamma;
		g = (g * 2 + _g_gamma) / 2;
		b += _g_gamma;
		if (r > 7) r = 7;
		if (g > 3) g = 3;
		if (b > 7) b = 7;
		uint8_t palette_block[5] = {c, 16, r << 4, g << 4, b << 4};
		_kernel_osword(OSWord_WritePalette, (int *)palette_block);
	}
	Z_ChangeTagToCache(playpal);
}


static const uint8_t colors[14] =
{
	0x00,											// normal
	0x10, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,	// red
	0x43, 0x53, 0x63, 0x73,							// yellow
	0x03											// green
};


static void I_UploadNewPalette(int8_t pal)
{
	uint8_t cdark = colors[pal];
	uint8_t palette_block[5] = {0, 16, cdark, cdark << 4, 0};
	_kernel_osword(OSWord_WritePalette, (int *)palette_block);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	v_setMode(13);
	v_disableTextCursor();

	_kernel_swi_regs regs;
	regs.r[0] = 2;
	regs.r[1] = +1 * 2 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES;
	_kernel_swi(OS_ChangeDynamicArea, &regs, &regs);
	if (regs.r[1] != 2 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES)
		I_Error("Failed to allocate 240 kB of video memory");

	uint8_t *videomemory = v_getScreenAddress();
	memset(videomemory, 0, 3 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES);
	videomemory += (SCREENWIDTH_ARCHIMEDES - SCREENWIDTH) / 2;								// center horizontally
	videomemory += ((SCREENHEIGHT_ARCHIMEDES - SCREENHEIGHT) / 2) * SCREENWIDTH_ARCHIMEDES;	// center vertically

	pages[0] = videomemory + 0 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES;
	pages[1] = videomemory + 1 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES;
	pages[2] = videomemory + 2 * SCREENWIDTH_ARCHIMEDES * SCREENHEIGHT_ARCHIMEDES;
	page = 1;
	_s_screen = pages[page];

	const uint16_t *playpal = W_GetLumpByName("PLAYPAL");
	for (int16_t c = 0; c < 16; c++)
	{
		uint16_t p = playpal[c];
		uint16_t r = p >> 4;
		uint16_t g = p & 0x0f0;
		uint16_t b = (p << 4) & 0x0f0;
		uint8_t palette_block[5] = {c, 16, r, g, b};
		_kernel_osword(OSWord_WritePalette, (int *)palette_block);
	}
	Z_ChangeTagToCache(playpal);
}


void I_ShutdownGraphics(void)
{
	_kernel_osbyte(OSByte_WriteDisplayBank, 0, 0);
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

			uint32_t *s = (uint32_t*)pages[prevpage];
			uint32_t *d = (uint32_t*)_s_screen;
			s += (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH_ARCHIMEDES / 4;
			d += (SCREENHEIGHT - ST_HEIGHT) * SCREENWIDTH_ARCHIMEDES / 4;
			for (int16_t y = 0; y < ST_HEIGHT; y++)
			{
				for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
					*d++ = *s++;

				s += (SCREENWIDTH_ARCHIMEDES - SCREENWIDTH) / 4;
				d += (SCREENWIDTH_ARCHIMEDES - SCREENWIDTH) / 4;
			}
		}
	}

	// page flip
	_kernel_osbyte(OSByte_WriteDisplayBank, page + 1, 0);
	page++;
	if (page == 3)
		page = 0;

	_s_screen = pages[page];
}


void I_FinishViewWindow(void)
{
	// Do nothing
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


inline static void R_DrawColumnPixel(uint8_t *dest, const uint8_t* colormap, const byte *source, uint16_t frac)
{
#if VIEWWINDOWWIDTH == 60
	*((uint32_t*)dest) = colormap[source[frac >> COLBITS]] * 0x01010101u;
#elif VIEWWINDOWWIDTH == 120
	*((uint16_t*)dest) = colormap[source[frac >> COLBITS]] * 0x0101u;
#elif VIEWWINDOWWIDTH == 240
	*dest = colormap[source[frac >> COLBITS]];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif
}


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

	uint8_t *dest = &_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH)];
	int16_t l = count >> 4;
	while (l--)
	{
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case 14: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case 13: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case 12: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case 11: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case 10: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  9: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  8: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  7: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  6: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  5: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  4: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  3: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  2: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH_ARCHIMEDES; frac += fracstep;
		case  1: R_DrawColumnPixel(dest, colormap, source, frac);
	}
}


void R_DrawColumnWall(const draw_column_vars_t *dcvars)
{
	R_DrawColumnSprite(dcvars);
}


void R_DrawColumnFlat(uint8_t col, const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

#if VIEWWINDOWWIDTH == 60
	uint32_t color = col * 0x01010101u;
	uint32_t *dest = (uint32_t*)&_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + (dcvars->x * 4)];
#elif VIEWWINDOWWIDTH == 120
	uint16_t color = col * 0x0101u;
	uint16_t *dest = (uint16_t*)&_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + (dcvars->x * 2)];
#elif VIEWWINDOWWIDTH == 240
	uint8_t color = col;
	uint8_t *dest = &_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + dcvars->x];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

	uint16_t l = count >> 4;

	while (l--)
	{
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);

		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);

		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);

		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
		*dest = color; dest += SCREENWIDTH_ARCHIMEDES / sizeof(color);
	}

	switch (count & 15)
	{
		case 15: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) * 14] = color;
		case 14: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) * 13] = color;
		case 13: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) * 12] = color;
		case 12: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) * 11] = color;
		case 11: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) * 10] = color;
		case 10: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  9] = color;
		case  9: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  8] = color;
		case  8: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  7] = color;
		case  7: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  6] = color;
		case  6: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  5] = color;
		case  5: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  4] = color;
		case  4: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  3] = color;
		case  3: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  2] = color;
		case  2: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  1] = color;
		case  1: dest[SCREENWIDTH_ARCHIMEDES / sizeof(color) *  0] = color;
	}
}


#define FUZZCOLOR1 0x00
#define FUZZCOLOR2 0x01
#define FUZZCOLOR3 0x02
#define FUZZCOLOR4 0x03
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

#if VIEWWINDOWWIDTH == 60
	const uint32_t c = 0x01010101u;
	uint32_t *dest = (uint32_t*)&_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + (dcvars->x * 4)];
#elif VIEWWINDOWWIDTH == 120
	const uint16_t c = 0x0101u;
	uint16_t *dest = (uint16_t*)&_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + (dcvars->x * 2)];
#elif VIEWWINDOWWIDTH == 240
	const uint8_t c = 0x01u;
	uint8_t *dest = &_s_screen[(dcvars->yl * SCREENWIDTH_ARCHIMEDES) + dcvars->x];
#else
#error unsupported VIEWWINDOWWIDTH value
#endif

	static int16_t fuzzpos = 0;

	do
	{
		*dest = c * fuzzcolors[fuzzpos];
		dest += SCREENWIDTH_ARCHIMEDES / sizeof(c);

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;

	} while (--count);
}


void V_ClearViewWindow(void)
{
	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
		memset(&_s_screen[y * SCREENWIDTH_ARCHIMEDES], 0, SCREENWIDTH);
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
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	while (true)
	{
		_s_screen[y0 * SCREENWIDTH_ARCHIMEDES + x0] = color;

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
		for (int16_t x = 0; x < SCREENWIDTH; x += 64)
		{
			uint8_t *d = &_s_screen[y * SCREENWIDTH_ARCHIMEDES + x];
			const byte *s = &src[((y & 63) * 64)];

			size_t len = 64;

			if (SCREENWIDTH - x < 64)
				len = SCREENWIDTH - x;

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
		offset = (offset / SCREENWIDTH) * SCREENWIDTH_ARCHIMEDES;
		const uint32_t *src = (const uint32_t*)lump;
		uint32_t *dest = (uint32_t*)&_s_screen[offset];
		uint16_t lumpLength = W_LumpLength(num);
		while (lumpLength)
		{
			for (int16_t x = 0; x < SCREENWIDTH / 4; x++)
				*dest++ = *src++;

			dest += (SCREENWIDTH_ARCHIMEDES - SCREENWIDTH) / 4;

			lumpLength -= SCREENWIDTH;
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

	byte *desttop = &_s_screen[y * SCREENWIDTH_ARCHIMEDES + x];

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + (column->topdelta * SCREENWIDTH_ARCHIMEDES);

			uint16_t count = column->length;

			if (count == 7)
			{
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++;
			}
			else if (count == 3)
			{
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++;
			}
			else if (count == 5)
			{
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++;
			}
			else if (count == 6)
			{
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++;
			}
			else if (count == 2)
			{
				*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				*dest = *source++;
			}
			else
			{
				while (count--)
				{
					*dest = *source++; dest += SCREENWIDTH_ARCHIMEDES;
				}
			}

			column = (const column_t *)((const byte *)column + column->length + 4);
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

		const column_t *column = (const column_t*)((const byte*)patch + (uint16_t)patch->columnofs[col >> 8]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte *dest = &_s_screen[dc_yl * SCREENWIDTH_ARCHIMEDES + dc_x];

			int16_t frac = 0;

			const byte *source = (const byte*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				*dest = source[frac >> 8];
				dest += SCREENWIDTH_ARCHIMEDES;
				frac += DYI;
			}

			column = (const column_t*)((const byte*)column + column->length + 4);
		}
	}
}


static uint16_t *frontbuffer;
static  int16_t *wipe_y_lookup;


void wipe_StartScreen(void)
{
	// Do nothing
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint16_t *backbuffer = (uint16_t *)_s_screen;

	while (ticks--)
	{
		for (int16_t i = 0; i < SCREENWIDTH / 2; i++)
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

				uint16_t *s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * (SCREENWIDTH_ARCHIMEDES / 2));
				uint16_t *d = &frontbuffer[i] + ((SCREENHEIGHT      - 1) * (SCREENWIDTH_ARCHIMEDES / 2));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -(SCREENWIDTH_ARCHIMEDES / 2);
					s += -(SCREENWIDTH_ARCHIMEDES / 2);
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * SCREENWIDTH_ARCHIMEDES / 2;
				d = &frontbuffer[i] + wipe_y_lookup[i] * SCREENWIDTH_ARCHIMEDES / 2;

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += (SCREENWIDTH_ARCHIMEDES / 2);
					s += (SCREENWIDTH_ARCHIMEDES / 2);
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
	wipe_y_lookup = Z_MallocStatic((SCREENWIDTH / 2) * sizeof(int16_t));

	wipe_y_lookup[0] = -(M_Random() % 16);
	for (int8_t i = 1; i < SCREENWIDTH / 2; i++)
	{
		int8_t r = (M_Random() % 3) - 1;

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

	frontbuffer = (uint16_t*)pages[prevpage];

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
