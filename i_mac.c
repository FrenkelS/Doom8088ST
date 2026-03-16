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

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	isKeyboardIsrSet = true;
}


void I_StartTic(void)
{
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

static boolean isTimerSet;


int32_t I_GetTime(void)
{
    return TickCount() * TICRATE / 60;
}


void I_InitTimer(void)
{
	isTimerSet = true;
}


static void I_ShutdownTimer(void)
{
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
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

	if (isTimerSet)
		I_ShutdownTimer();

	if (isKeyboardIsrSet)
	{
	}
}


void I_Quit(void)
{
	I_Shutdown();

	printf("\n");
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

	D_DoomMain(argc, argv);

	return 0;
}
