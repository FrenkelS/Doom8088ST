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
 *      Video code for AT&T UNIX PC / PC 7300 / 3B1 720x348 2 color
 *
 *-----------------------------------------------------------------------------*/

#include <sys/iohw.h>

#include "i_system.h"
#include "i_video.h"
#include "m_random.h"

#include "globdata.h"


extern const int16_t CENTERY;


static uint8_t _s_screen[VIEWWINDOWWIDTH * SCREENHEIGHT];
static uint8_t *videomemory;


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


static const uint8_t leftcolors[8] =
{
	0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff
};

static const uint8_t rightcolors[8] =
{
	0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};


static void I_UploadNewPalette(int8_t pal)
{
	uint8_t *leftPtr  = videomemory - 2;
	uint8_t *rightPtr = videomemory + 1 + VIEWWINDOWWIDTH;
	uint8_t leftColor  = 0;
	uint8_t rightColor = 0;

	if (1 <= pal && pal <= 9)
	{
		leftColor  = leftcolors[ pal - 1];
		rightColor = rightcolors[pal - 1];
	}

	for (int16_t y = 0; y < SCREENHEIGHT * 2; y++)
	{
		*leftPtr  = leftColor;
		*rightPtr = rightColor;
		leftPtr  += VIDBYTES;
		rightPtr += VIDBYTES;
	}
}


void I_InitGraphicsHardwareSpecificCode(void)
{
	videomemory = (uint8_t*)VIDMEM;
	videomemory += (VIDBYTES - VIEWWINDOWWIDTH) / 2 - 1;			// center horizontally
	videomemory += ((VIDHEIGHT - SCREENHEIGHT * 2) / 2) * VIDBYTES;	// center vertically
}


void I_ShutdownGraphics(void)
{
	memset(_s_screen, 0, VIEWWINDOWWIDTH * SCREENHEIGHT);
	I_FinishUpdate();
}


static boolean drawStatusBar = true;

static const uint8_t lut[256] =
{
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};


static void I_DrawBuffer(uint8_t *buffer)
{
	uint8_t *src = buffer;
	uint8_t *dst = videomemory;

	for (int16_t y = 0; y < SCREENHEIGHT - ST_HEIGHT; y++)
	{
		for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
		{
			dst[1] = lut[*src++];
			dst[0] = lut[*src++];
			dst += 2;
		}
		dst += VIDBYTES - VIEWWINDOWWIDTH;
		memcpy(dst, dst - VIDBYTES, VIEWWINDOWWIDTH);
		dst += VIDBYTES;
	}

	if (drawStatusBar)
	{
		for (int16_t y = 0; y < ST_HEIGHT; y++)
		{
			for (int16_t x = 0; x < VIEWWINDOWWIDTH / 2; x++)
			{
				dst[1] = lut[*src++];
				dst[0] = lut[*src++];
				dst += 2;
			}
			dst += VIDBYTES - VIEWWINDOWWIDTH;
			memcpy(dst, dst - VIDBYTES, VIEWWINDOWWIDTH);
			dst += VIDBYTES;
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

	// Zero length, column does not exceed a pixel.
	if (count <= 0)
		return;

	const uint8_t *source = dcvars->source;

	const uint8_t *colormap = dcvars->colormap;

	const uint16_t fracstep = dcvars->fracstep;
	uint16_t frac = (dcvars->texturemid >> COLEXTRABITS) + (dcvars->yl - CENTERY) * fracstep;

	// Inner loop that does the actual texture mapping,
	//  e.g. a DDA-lile scaling.
	// This is as fast as it gets.

	uint8_t *dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];;
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

	uint8_t *dest = &_s_screen[(dcvars->yl * VIEWWINDOWWIDTH) + dcvars->x];

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
		uint8_t c = _s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)];
		_s_screen[y0 * VIEWWINDOWWIDTH + (x0 >> 3)] = (c & bitmask) | (color >> p);

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
	y -= patch->topoffset;
	x -= patch->leftoffset;

	byte *desttop = &_s_screen[(y * VIEWWINDOWWIDTH) + (x >> 2)];

	int16_t width = patch->width;

	int16_t p = x & 3;
	uint8_t bitmask = bitmasks4[p];

	for (int16_t col = 0; col < width; col++)
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

		int16_t p = dc_x & 3;
		uint8_t bitmask = bitmasks4[p];

		// step through the posts in a column
		while (column->topdelta != 0xff)
		{
			int16_t dc_yl = (((y + column->topdelta) * DY) >> FRACBITS);

			if ((dc_yl >= SCREENHEIGHT) || (dc_yl > bottom))
				break;

			int16_t dc_yh = (((y + column->topdelta + column->length) * DY) >> FRACBITS);

			byte *dest = &_s_screen[(dc_yl * VIEWWINDOWWIDTH) + (dc_x >> 2)];

			int16_t frac = 0;

			const byte *source = (const byte*)column + 3;

			int16_t count = dc_yh - dc_yl;
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

				uint8_t *s = &frontbuffer[i] + ((SCREENHEIGHT - dy - 1) * VIEWWINDOWWIDTH);
				uint8_t *d = &frontbuffer[i] + ((SCREENHEIGHT      - 1) * VIEWWINDOWWIDTH);

				// scroll down the column. Of course we need to copy from the bottom... up to
				// SCREENHEIGHT - yLookup - dy

				for (int16_t j = SCREENHEIGHT - wipe_y_lookup[i] - dy; j; j--)
				{
					*d = *s;
					d += -VIEWWINDOWWIDTH;
					s += -VIEWWINDOWWIDTH;
				}

				// copy new screen. We need to copy only between y_lookup and + dy y_lookup
				s = &backbuffer[i]  + wipe_y_lookup[i] * VIEWWINDOWWIDTH;
				d = &frontbuffer[i] + wipe_y_lookup[i] * VIEWWINDOWWIDTH;

				for (int16_t j = 0 ; j < dy; j++)
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
