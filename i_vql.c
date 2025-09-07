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
 *      Video code for Sinclair QL 512x256 4 color (2 colors used)
 *                                 256x256 8 color (4 colors used)
 *
 *-----------------------------------------------------------------------------*/

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"

#include "globdata.h"


#define PLANEWIDTH		128
#define SCREENHEIGHT_QL	256


#define SCREENMODE_LO	(1<<3)
#define SCREENMODE_HI	(0<<3)

#if !defined SCREENMODE
#define SCREENMODE SCREENMODE_HI
#endif


extern const int16_t CENTERY;


static uint8_t _s_screen[VIEWWINDOWWIDTH * SCREENHEIGHT];
static uint8_t *videomemory;


void I_ReloadPalette(void)
{
	char lumpName[8] = "COLORMAP";
	if (_g_gamma != 0)
	{
		lumpName[6] = 'P';
		lumpName[7] = '0' + _g_gamma;
	}

	W_ReadLumpByNum(W_GetNumForName(lumpName), (void *)fullcolormap);
}


#if SCREENMODE == SCREENMODE_HI
static const uint16_t redColorsLeft[8] =
{
	0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff
};

static const uint16_t redColorsRight[8] =
{
	0x0080, 0x00c0, 0x00e0, 0x00f0, 0x00f8, 0x00fc, 0x00fe, 0x00ff
};

static const uint16_t yellowColorsLeft[4] =
{
	0x1818, 0x2424, 0x5a5a, 0xa5a5
};
#define yellowColorsRight yellowColorsLeft
#define GREEN_LEFT	0x5500
#define GREEN_RIGHT	0xaa00
#else
static const uint16_t redColorsLeft[8] =
{
	0x0002, 0x0002, 0x000a, 0x000a, 0x002a, 0x002a, 0x00aa, 0x00aa
};

static const uint16_t redColorsRight[8] =
{
	0x0080, 0x0080, 0x00a0, 0x00a0, 0x00a8, 0x00a8, 0x00aa, 0x00aa
};

static const uint16_t yellowColorsLeft[4] =
{
	0x0202, 0x0a0a, 0x2a2a, 0xaaaa
};

static const uint16_t yellowColorsRight[4] =
{
	0x8080, 0xa0a0, 0x8a8a, 0xaaaa
};
#define GREEN_LEFT	0x2200
#define GREEN_RIGHT	0x8800
#endif


static void I_UploadNewPalette(int8_t pal)
{
	uint16_t *leftPtr  = (uint16_t *)(videomemory - 2);
	uint16_t *rightPtr = (uint16_t *)(videomemory + VIEWWINDOWWIDTH * 2);
	uint16_t leftColor  = 0;
	uint16_t rightColor = 0;

	int16_t y;

	if (1 <= pal && pal <= 8)
	{
		leftColor  = redColorsLeft[ pal - 1];
		rightColor = redColorsRight[pal - 1];
	}
	else if (9 <= pal && pal <= 12)
	{
		leftColor  = yellowColorsLeft[ pal - 9];
		rightColor = yellowColorsRight[pal - 9];
	}
	else if (pal == 13)
	{
		leftColor  = GREEN_LEFT;
		rightColor = GREEN_RIGHT;
	}

	for (y = 0; y < SCREENHEIGHT; y++)
	{
		*leftPtr  = leftColor;
		*rightPtr = rightColor;
		leftPtr  += PLANEWIDTH / sizeof(uint16_t);
		rightPtr += PLANEWIDTH / sizeof(uint16_t);
	}
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	uint8_t *rezPtr = (uint8_t*)0x18063;
	*rezPtr = SCREENMODE;

	videomemory = (uint8_t*)0x20000;

	memset(videomemory, 0, PLANEWIDTH * SCREENHEIGHT_QL);

	videomemory += (((PLANEWIDTH / 2) - VIEWWINDOWWIDTH) / 2) * sizeof(uint16_t);	// center horizontally
	videomemory += ((SCREENHEIGHT_QL - SCREENHEIGHT) / 2) * PLANEWIDTH;				// center vertically
}


void I_ShutdownGraphics(void)
{
	I_SetPalette(0);
	memset(_s_screen, 0, VIEWWINDOWWIDTH * SCREENHEIGHT);
	I_FinishUpdate();
}


static boolean drawStatusBar = true;


static void I_DrawBuffer(uint8_t *buffer)
{
	uint8_t *src = buffer;
	uint8_t *dst = videomemory;

	int16_t x, y;

	for (y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		for (x = 0; x < VIEWWINDOWWIDTH; x++)
		{
#if SCREENMODE == SCREENMODE_HI
			*dst++ = *src;
			*dst++ = *src++;
#else
			dst++;
			*dst++ = *src++;
#endif
		}
		dst += PLANEWIDTH - (VIEWWINDOWWIDTH * 2);
	}

	if (drawStatusBar)
	{
		for (y = 0; y < ST_HEIGHT; y++)
		{
			for (x = 0; x < VIEWWINDOWWIDTH; x++)
			{
#if SCREENMODE == SCREENMODE_HI
				*dst++ = *src;
				*dst++ = *src++;
#else
				dst++;
				*dst++ = *src++;
#endif
			}
			dst += PLANEWIDTH - (VIEWWINDOWWIDTH * 2);
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


void R_DrawColumnSprite(const draw_column_vars_t *dcvars)
{
	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	const uint8_t *source;
	const uint8_t *colormap;
	uint16_t fracstep, frac;
	uint8_t *dest;
	int16_t l;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	source   = dcvars->source;
	colormap = dcvars->colormap;

	fracstep = dcvars->fracstep;
	frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];
	l = count >> 4;
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

	uint8_t color0, color1;
	uint8_t *dest;
	int16_t i;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

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

	for (i = 0; i < count / 2; i++)
	{
		*dest = color0; dest += VIEWWINDOWWIDTH;
		*dest = color1; dest += VIEWWINDOWWIDTH;
	}

	if (count & 1)
		*dest = color0;
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
	static int16_t fuzzpos = 0;

	int16_t count = (dcvars->yh - dcvars->yl) + 1;

	uint8_t *dest;

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

	do
	{
		*dest = fuzzcolors[fuzzpos];
		dest += VIEWWINDOWWIDTH;

		fuzzpos++;
		if (fuzzpos >= FUZZTABLE)
			fuzzpos = 0;
	} while(--count);
}


void V_ClearViewWindow(void)
{
	memset(_s_screen, 0, VIEWWINDOWWIDTH * (SCREENHEIGHT - ST_HEIGHT));
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
#if SCREENMODE == SCREENMODE_HI
	int16_t dx = abs(x1 - x0);
	int16_t sx = x0 < x1 ? 1 : -1;

	int16_t dy = -abs(y1 - y0);
	int16_t sy = y0 < y1 ? 1 : -1;

	int16_t err = dx + dy;

	uint8_t bitmask = 0x80 >> (x0 & 7);

	UNUSED(color);

	while (true)
	{
		int16_t e2;
		uint8_t c = _s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)];
		_s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)] = (c & ~bitmask) | bitmask;

		if (x0 == x1 && y0 == y1)
			break;

		e2 = 2 * err;

		if (e2 >= dy)
		{
			err += dy;
			x0  += sx;

			bitmask = 0x80 >> (x0 & 7);
		}

		if (e2 <= dx)
		{
			err += dx;
			y0  += sy;
		}
	}
#else
// TODO SCREENMODE_LO not implemented yet
#endif
}


void V_DrawBackground(int16_t backgroundnum)
{
	int16_t x, y;
	const byte *src = W_GetLumpByNum(backgroundnum);

	for (y = 0; y < SCREENHEIGHT; y++)
	{
		for (x = 0; x < VIEWWINDOWWIDTH; x += 16)
		{
			uint8_t *d = &_s_screen[y * VIEWWINDOWWIDTH + x];
			const byte *s = &src[((y & 63) * 16)];

			size_t len = 16;

			if (VIEWWINDOWWIDTH - x < 16)
				len = VIEWWINDOWWIDTH - x;

			memcpy(d, s, len);
		}
	}

	Z_ChangeTagToCache(src);
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
	if (ST_NeedUpdate())
		ST_doRefresh();
	else
		drawStatusBar = false;
}


static const uint8_t bitmasks4[4] = {0x3f, 0xcf, 0xf3, 0xfc};


void V_DrawPatchNotScaled(int16_t x, int16_t y, const patch_t __far* patch)
{
	byte *desttop;
	int16_t width, p, col;
	uint8_t bitmask;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	desttop = &_s_screen[(y * VIEWWINDOWWIDTH) + (x >> 2)];

	width = patch->width;

	p = x & 3;
	bitmask = bitmasks4[p];

	for (col = 0; col < width; col++)
	{
		const column_t *column = (const column_t*)((const byte*)patch + (uint16_t)patch->columnofs[col]);

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			const byte *source = (const byte*)column + 3;
			byte *dest = desttop + (column->topdelta * VIEWWINDOWWIDTH);

			uint16_t count = column->length;

			uint8_t c;
			uint8_t color;

			if (count == 7)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 3)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 5)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else if (count == 6)
			{
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2));
			}
			else
			{
				while (count--)
				{
					c = *dest; color = *source++; *dest = (c & bitmask) | (color >> (p * 2)); dest += VIEWWINDOWWIDTH;
				}
			}

			column = (const column_t*)((const byte*)column + column->length + 4);
		}

		p++;
		if (p == 4)
		{
			p = 0;
			desttop++;
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

	int16_t left, right, bottom, dc_x;
	uint16_t col = 0;

	y -= patch->topoffset;
	x -= patch->leftoffset;

	left   = ( x * DX ) >> FRACBITS;
	right  = ((x + patch->width)  * DX) >> FRACBITS;
	bottom = ((y + patch->height) * DY) >> FRACBITS;

	for (dc_x = left; dc_x < right; dc_x++, col += DXI)
	{
		const column_t *column;
		int16_t p;
		uint8_t bitmask;

		if (dc_x < 0)
			continue;
		else if (dc_x >= SCREENWIDTH)
			break;

		column = (const column_t*)((const byte*)patch + (uint16_t)patch->columnofs[col >> 8]);

		p = dc_x & 3;
		bitmask = bitmasks4[p];

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);
			int16_t dc_yh;
			byte *dest;
			int16_t frac = 0;
			const byte *source;
			int16_t count;

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			dest = &_s_screen[(dc_yl * VIEWWINDOWWIDTH) + (dc_x >> 2)];

			source = (const byte*)column + 3;

			count = dc_yh - dc_yl;
			while (count--)
			{
				uint8_t c = *dest;
				uint8_t color = source[frac >> 8];
				*dest = (c & bitmask) | (color >> (p * 2));
				dest += VIEWWINDOWWIDTH;
				frac += DYI;
			}

			column = (const column_t*)((const byte*)column + column->length + 4);
		}
	}
}


static uint8_t *frontbuffer;
static int16_t *wipe_y_lookup;


void wipe_StartScreen(void)
{
	frontbuffer = Z_TryMallocStatic(VIEWWINDOWWIDTH * SCREENHEIGHT);
	if (frontbuffer)
		memcpy(frontbuffer, _s_screen, VIEWWINDOWWIDTH * SCREENHEIGHT);
}


static boolean wipe_ScreenWipe(int16_t ticks)
{
	boolean done = true;

	uint8_t *backbuffer = _s_screen;

	while (ticks--)
	{
		int16_t i;
		I_DrawBuffer(frontbuffer);
		for (i = 0; i < VIEWWINDOWWIDTH; i++)
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
				uint8_t *s;
				uint8_t *d;
				int16_t j;
		
				// At most dy shall be so that the column is shifted by SCREENHEIGHT (i.e. just invisible)
				if (wipe_y_lookup[i] + dy >= SCREENHEIGHT)
					dy = SCREENHEIGHT - wipe_y_lookup[i];

				s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * VIEWWINDOWWIDTH);
				d = &frontbuffer[i] + ((SCREENHEIGHT      - 1) * VIEWWINDOWWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -VIEWWINDOWWIDTH;
					s += -VIEWWINDOWWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * VIEWWINDOWWIDTH;
				d = &frontbuffer[i] + wipe_y_lookup[i] * VIEWWINDOWWIDTH;

				for (j = 0 ; j < dy; j++)
				{
					*d = *s;
					d += VIEWWINDOWWIDTH;
					s += VIEWWINDOWWIDTH;
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
	int16_t i;

	wipe_y_lookup = Z_MallocStatic(VIEWWINDOWWIDTH * sizeof(int16_t));

	wipe_y_lookup[0] = -(M_Random() % 16);
	for (i = 1; i < VIEWWINDOWWIDTH; i++)
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
	boolean done;
	int32_t wipestart;

	if (!frontbuffer)
		return;

	wipe_initMelt();

	wipestart = I_GetTime() - 1;

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
