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


static uint8_t _s_screen[SCREENWIDTH * SCREENHEIGHT];
static uint8_t *videomemory;


void I_ReloadPalette(void)
{
	// TODO
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
	uint8_t palette_block[5] = {0, 16, 0, 0, 0};

	palette_block[2] = cdark;
	palette_block[3] = cdark << 4;

	_kernel_osword(OSWord_WritePalette, (int *)palette_block);
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	v_setMode(13);
	v_disableTextCursor();

	videomemory  = v_getScreenAddress();
	videomemory += (SCREENWIDTH_ARCHIMEDES - SCREENWIDTH) / 2;								// center horizontally
	videomemory += ((SCREENHEIGHT_ARCHIMEDES - SCREENHEIGHT) / 2) * SCREENWIDTH_ARCHIMEDES;	// center vertically
}


static boolean drawStatusBar = true;


void I_ShutdownGraphics(void)
{
	// Do nothing
}


static void I_DrawBuffer(uint8_t *buffer)
{
	uint8_t *src = buffer;
	uint8_t *dst = videomemory;

	for (uint_fast8_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		memcpy(dst, src, SCREENWIDTH);
		dst += SCREENWIDTH_ARCHIMEDES;
		src += SCREENWIDTH;
	}

	if (drawStatusBar)
	{
		for (uint_fast8_t y = 0; y < ST_HEIGHT; y++)
		{
			memcpy(dst, src, SCREENWIDTH);
			dst += SCREENWIDTH_ARCHIMEDES;
			src += SCREENWIDTH;
		}
	}
	drawStatusBar = true;
}


static int8_t newpal;


void I_SetPalette(int8_t p)
{
	newpal = p;
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
	// Do nothing
}


#define COLEXTRABITS (8 - 1)
#define COLBITS (8 + 1)


inline static void R_DrawColumnPixel(uint8_t *dest, const uint8_t* colormap, const byte *source, uint16_t frac)
{
#if VIEWWINDOWWIDTH == 60
	*((uint32_t*)dest) = colormap[source[frac >> COLBITS]] * 0x01010101u;

#elif VIEWWINDOWWIDTH == 120
	uint16_t color = colormap[source[frac>>COLBITS]];
	color = (color | (color << 8));

	uint16_t *d = (uint16_t *) dest;
	*d   = color;
#elif VIEWWINDOWWIDTH == 240
	*dest = colormap[source[frac>>COLBITS]];
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

	uint8_t *dest = &_s_screen[(dcvars->yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH)];
	int16_t l = count >> 4;
	while (l--)
	{
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;

		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
	}

	switch (count & 15)
	{
		case 15: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 14: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 13: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 12: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 11: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case 10: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  9: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  8: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  7: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  6: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  5: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  4: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  3: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
		case  2: R_DrawColumnPixel(dest, colormap, source, frac); dest += SCREENWIDTH; frac += fracstep;
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

	uint8_t *dest = &_s_screen[(dcvars->yl * SCREENWIDTH) + (dcvars->x * 4 * 60 / VIEWWINDOWWIDTH)];

	uint16_t l = count >> 4;

	while (l--)
	{
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;

		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;

		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;

		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
		*dest++ = col; *dest++ = col; *dest++ = col; *dest++ = col; dest += SCREENWIDTH - 4;
	}

	switch (count & 15)
	{
		case 15: dest[SCREENWIDTH * 14] = col; dest[SCREENWIDTH * 14 + 1] = col; dest[SCREENWIDTH * 14 + 2] = col; dest[SCREENWIDTH * 14 + 3] = col;
		case 14: dest[SCREENWIDTH * 13] = col; dest[SCREENWIDTH * 13 + 1] = col; dest[SCREENWIDTH * 13 + 2] = col; dest[SCREENWIDTH * 13 + 3] = col;
		case 13: dest[SCREENWIDTH * 12] = col; dest[SCREENWIDTH * 12 + 1] = col; dest[SCREENWIDTH * 12 + 2] = col; dest[SCREENWIDTH * 12 + 3] = col;
		case 12: dest[SCREENWIDTH * 11] = col; dest[SCREENWIDTH * 11 + 1] = col; dest[SCREENWIDTH * 11 + 2] = col; dest[SCREENWIDTH * 11 + 3] = col;
		case 11: dest[SCREENWIDTH * 10] = col; dest[SCREENWIDTH * 10 + 1] = col; dest[SCREENWIDTH * 10 + 2] = col; dest[SCREENWIDTH * 10 + 3] = col;
		case 10: dest[SCREENWIDTH *  9] = col; dest[SCREENWIDTH *  9 + 1] = col; dest[SCREENWIDTH *  9 + 2] = col; dest[SCREENWIDTH *  9 + 3] = col;
		case  9: dest[SCREENWIDTH *  8] = col; dest[SCREENWIDTH *  8 + 1] = col; dest[SCREENWIDTH *  8 + 2] = col; dest[SCREENWIDTH *  8 + 3] = col;
		case  8: dest[SCREENWIDTH *  7] = col; dest[SCREENWIDTH *  7 + 1] = col; dest[SCREENWIDTH *  7 + 2] = col; dest[SCREENWIDTH *  7 + 3] = col;
		case  7: dest[SCREENWIDTH *  6] = col; dest[SCREENWIDTH *  6 + 1] = col; dest[SCREENWIDTH *  6 + 2] = col; dest[SCREENWIDTH *  6 + 3] = col;
		case  6: dest[SCREENWIDTH *  5] = col; dest[SCREENWIDTH *  5 + 1] = col; dest[SCREENWIDTH *  5 + 2] = col; dest[SCREENWIDTH *  5 + 3] = col;
		case  5: dest[SCREENWIDTH *  4] = col; dest[SCREENWIDTH *  4 + 1] = col; dest[SCREENWIDTH *  4 + 2] = col; dest[SCREENWIDTH *  4 + 3] = col;
		case  4: dest[SCREENWIDTH *  3] = col; dest[SCREENWIDTH *  3 + 1] = col; dest[SCREENWIDTH *  3 + 2] = col; dest[SCREENWIDTH *  3 + 3] = col;
		case  3: dest[SCREENWIDTH *  2] = col; dest[SCREENWIDTH *  2 + 1] = col; dest[SCREENWIDTH *  2 + 2] = col; dest[SCREENWIDTH *  2 + 3] = col;
		case  2: dest[SCREENWIDTH *  1] = col; dest[SCREENWIDTH *  1 + 1] = col; dest[SCREENWIDTH *  1 + 2] = col; dest[SCREENWIDTH *  1 + 3] = col;
		case  1: dest[SCREENWIDTH *  0] = col; dest[SCREENWIDTH *  0 + 1] = col; dest[SCREENWIDTH *  0 + 2] = col; dest[SCREENWIDTH *  0 + 3] = col;
	}
}


void R_DrawFuzzColumn(const draw_column_vars_t *dcvars)
{
	// TODO
}


void V_ClearViewWindow(void)
{
	memset(_s_screen, 0, SCREENWIDTH * (SCREENHEIGHT - ST_HEIGHT));
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
		_s_screen[y0 * SCREENWIDTH + x0] = color;

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
			uint8_t *d = &_s_screen[y * SCREENWIDTH + x];
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

	byte *desttop = &_s_screen[y * SCREENWIDTH + x];

	int16_t width = patch->width;

	for (int16_t col = 0; col < width; col++, desttop++)
	{
		const column_t *column = (const column_t *)((const byte *)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte *)column + 3;
			byte *dest = desttop + (column->topdelta * SCREENWIDTH);

			uint16_t count = column->length;

			if (count == 7)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 3)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 5)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 6)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else if (count == 2)
			{
				*dest = *source++; dest += SCREENWIDTH;
				*dest = *source++;
			}
			else
			{
				while (count--)
				{
					*dest = *source++; dest += SCREENWIDTH;
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

			byte *dest = &_s_screen[dc_yl * SCREENWIDTH + dc_x];

			int16_t frac = 0;

			const byte *source = (const byte*)column + 3;

			int16_t count = dc_yh - dc_yl;
			while (count--)
			{
				*dest = source[frac >> 8];
				dest += SCREENWIDTH;
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
	frontbuffer = Z_TryMallocStatic(SCREENWIDTH * SCREENHEIGHT);
	if (frontbuffer)
		memcpy(frontbuffer, _s_screen, SCREENWIDTH * SCREENHEIGHT);
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

				uint16_t *s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * (SCREENWIDTH / 2));
				uint16_t *d = &frontbuffer[i] + ((SCREENHEIGHT      - 1) * (SCREENWIDTH / 2));

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -(SCREENWIDTH / 2);
					s += -(SCREENWIDTH / 2);
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * SCREENWIDTH / 2;
				d = &frontbuffer[i] + wipe_y_lookup[i] * SCREENWIDTH / 2;

				for (int16_t j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += (SCREENWIDTH / 2);
					s += (SCREENWIDTH / 2);
				}

				wipe_y_lookup[i] += dy;
				done = false;
			}
		}
	}

	I_DrawBuffer((uint8_t *)frontbuffer);

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
