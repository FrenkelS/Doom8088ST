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
 *      Macintosh implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <stdarg.h>

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
#include "a_pcfx.h"
#include "d_main.h"
#include "i_system.h"

#include "globdata.h"

#include <Quickdraw.h>


//#define TIMEDEMO


void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


static Rect r;


static boolean isGraphicsModeSet = false;


//**************************************************************************************
//
// Screen code
//

void I_InitGraphics(void)
{
	I_InitGraphicsHardwareSpecificCode();
	isGraphicsModeSet = true;
}


//**************************************************************************************
//
// Keyboard code
//

void I_InitKeyboard(void)
{
	// Do nothing
}


static void I_PostEvent(boolean keydown, int16_t data1)
{
	d_event_t ev;
	ev.type  = keydown ? ev_keydown : ev_keyup;
	ev.data1 = data1;
	D_PostEvent(&ev);
}


#define KB_MATRIX_SIZE 16

void I_StartTic(void)
{
	static uint8_t kb_matrix[KB_MATRIX_SIZE * 2];
	static uint8_t *kb_matrix_cur = &kb_matrix[0];
	static uint8_t *kb_matrix_prv = &kb_matrix[KB_MATRIX_SIZE];

	uint8_t diff;
	uint8_t *tmp = kb_matrix_cur;
	kb_matrix_cur = kb_matrix_prv;
	kb_matrix_prv = tmp;


	GetKeys(kb_matrix_cur);

	diff = kb_matrix_prv[0] ^ kb_matrix_cur[0];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[0] & (1 << 0), 'a');					// a
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[0] & (1 << 1), 's');					// s
	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[0] & (1 << 2), 'd');					// d
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[0] & (1 << 3), 'f');					// f
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[0] & (1 << 4), 'h');					// h


	diff = kb_matrix_prv[1] ^ kb_matrix_cur[1];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[1] & (1 << 0), 'c');					// c
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[1] & (1 << 1), 'v');					// v

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[1] & (1 << 3), 'b');					// b
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[1] & (1 << 4), 'q');					// q

	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[1] & (1 << 6), 'e');					// e
	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[1] & (1 << 7), 'r');					// r


	diff = kb_matrix_prv[2] ^ kb_matrix_cur[2];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[2] & (1 << 0), 'y');					// y
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[2] & (1 << 1), 't');					// t


	diff = kb_matrix_prv[3] ^ kb_matrix_cur[3];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[3] & (1 << 0), KEYD_PLUS);			// +

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[3] & (1 << 3), KEYD_MINUS);			// -

	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[3] & (1 << 6), KEYD_BRACKET_RIGHT);	// ]
	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[3] & (1 << 7), 'o');					// o


	diff = kb_matrix_prv[4] ^ kb_matrix_cur[4];
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[4] & (1 << 1), KEYD_BRACKET_LEFT);	// [
	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[4] & (1 << 2), 'i');					// i
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[4] & (1 << 3), 'p');					// p
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[4] & (1 << 4), KEYD_A);				// Return
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[4] & (1 << 5), 'l');					// l


	diff = kb_matrix_prv[5] ^ kb_matrix_cur[5];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[5] & (1 << 0), 'k');			// k

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[5] & (1 << 3), KEYD_L);		// <

	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[5] & (1 << 5), 'n');			// n

	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[5] & (1 << 7), KEYD_R);		// >


	diff = kb_matrix_prv[6] ^ kb_matrix_cur[6];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[6] & (1 << 0), KEYD_SELECT);	// Tab
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[6] & (1 << 1), KEYD_A);		// Space

	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[6] & (1 << 5), KEYD_START);	// Esc


	diff = kb_matrix_prv[7] ^ kb_matrix_cur[7];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[7] & (1 << 0), KEYD_SPEED);	// Shift

	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[7] & (1 << 2), KEYD_STRAFE);	// Alt
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[7] & (1 << 3), KEYD_B);		// Ctrl


	diff = kb_matrix_prv[9] ^ kb_matrix_cur[9];
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[4] & (1 << 4), KEYD_A);		// Enter


	diff = kb_matrix_prv[10] ^ kb_matrix_cur[10];
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[10] & (1 << 4), KEYD_DOWN);	// 2

	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[10] & (1 << 6), KEYD_LEFT);	// 4


	diff = kb_matrix_prv[11] ^ kb_matrix_cur[11];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[11] & (1 << 0), KEYD_RIGHT);	// 6

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[11] & (1 << 3), KEYD_UP);	// 8


	diff = kb_matrix_prv[13] ^ kb_matrix_cur[13];
	if (diff & (1 << 5)) I_Quit();												// F10


	diff = kb_matrix_prv[15] ^ kb_matrix_cur[15];
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[15] & (1 << 3), KEYD_LEFT);	// Left
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[15] & (1 << 4), KEYD_RIGHT);	// Right
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[15] & (1 << 5), KEYD_DOWN);	// Down
	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[15] & (1 << 6), KEYD_UP);	// Up
}


//**************************************************************************************
//
// Audio
//

void PCFX_Play(int16_t lumpnum)
{
}


void PCFX_Init(void)
{
}


void PCFX_Shutdown(void)
{
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

int32_t I_GetTime(void)
{
	return TickCount() * TICRATE / 60;
}


void I_InitTimer(void)
{
	// do nothing
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	MaxApplZone();
	int32_t availableMemory = FreeMem();
	uint32_t paragraphs = availableMemory < 8 * 1024 * 1024L ? availableMemory / PARAGRAPH_SIZE : 8 * 1024 * 1024L / PARAGRAPH_SIZE;
	uint8_t* ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs--;
		ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	}

	// align ptr
	uint32_t m = (uint32_t) ptr;
	if ((m & (PARAGRAPH_SIZE - 1)) != 0)
	{
		paragraphs--;
		while ((m & (PARAGRAPH_SIZE - 1)) != 0)
			m = (uint32_t) ++ptr;
	}

	*heapSize = paragraphs * PARAGRAPH_SIZE;
	return ptr;
}


//**************************************************************************************
//
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
		I_ShutdownGraphics();

	I_ShutdownSound();
}


void I_Quit(void)
{
	I_Shutdown();
	exit(0);
}


void I_ErrorMac(const char *error, ...)
{
	va_list argptr;
	Str255 pstr;

	va_start(argptr, error);
	vsprintf(pstr, error, argptr);
	va_end(argptr);

	SetRect(&r, 10, 10, 10 + StringWidth(pstr) + 10, 30);
	PaintRect(&r);
	PenMode(patXor);
	FrameRect(&r);
	MoveTo(15, 25);
	TextMode(srcBic);
	DrawString(pstr);

	while (!Button())
	{
	}
	FlushEvents(everyEvent, -1);

	exit(1);
}


int main(int argc, const char * const * argv)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();

	r = qd.screenBits.bounds;

	SetRect(&r, r.left + 5, r.top + 45, r.right - 5, r.bottom - 5);
	WindowPtr win = NewWindow(NULL, &r, "\pDOOM8088: Macintosh Edition", true, 0, (WindowPtr)-1, false, 0);

	SetPort(win);
	r = win->portRect;

	EraseRect(&r);

#if defined TIMEDEMO
	const char * const args[] = {"DOOM8088", "-timedemo", "demo3"};
	D_DoomMain(3, args);
#else
	D_DoomMain(argc, argv);
#endif

	return 0;
}
