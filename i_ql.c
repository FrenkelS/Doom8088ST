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
 *      Sinclair QL implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <qdos.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "doomdef.h"
#include "a_pcfx.h"
#include "d_main.h"
#include "i_sound.h"
#include "i_system.h"





void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


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
// Returns time in 1/35th second tics.
//

static QL_LINK_t qlLink;
static volatile int32_t ticcount;

static boolean isTimerSet;


static void I_TimerISR(void)
{
	ticcount++;
}


int32_t I_GetTime(void)
{
	return ticcount * TICRATE / CLOCKS_PER_SEC;
}


void I_InitTimer(void)
{
	qlLink.l_next = NULL;
	qlLink.l_rtn  = I_TimerISR;
	mt_lpoll(&qlLink);

	isTimerSet = true;
}


static void I_ShutdownTimer(void)
{
	mt_rpoll(&qlLink);
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


void I_StartTic(void)
{
	static uint8_t keys_cur;
	static uint8_t keys_prv;

	uint8_t diff;
	uint8_t tmp = keys_cur;
	keys_cur = keys_prv;
	keys_prv = tmp;

	keys_cur = keyrow(1);
	diff = keys_prv ^ keys_cur;

	if (diff & (1 << 0)) I_PostEvent(keys_cur & (1 << 0), KEYD_A);		// Enter
	if (diff & (1 << 1)) I_PostEvent(keys_cur & (1 << 1), KEYD_LEFT);	// Left
	if (diff & (1 << 2)) I_PostEvent(keys_cur & (1 << 2), KEYD_UP);		// Up
	if (diff & (1 << 3)) I_PostEvent(keys_cur & (1 << 3), KEYD_START);	// Esc
	if (diff & (1 << 4)) I_PostEvent(keys_cur & (1 << 4), KEYD_RIGHT);	// Right

	if (diff & (1 << 6)) I_PostEvent(keys_cur & (1 << 6), KEYD_A);		// Space
	if (diff & (1 << 7)) I_PostEvent(keys_cur & (1 << 7), KEYD_DOWN);	// Down
}


//**************************************************************************************
//
// Audio
//

void PCFX_Play(int16_t lumpnum)
{
	UNUSED(lumpnum);
	// TODO
}


void PCFX_Init(void)
{
	// TODO
}


void PCFX_Shutdown(void)
{
	// TODO
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	uint32_t m;

	uint32_t availableMemory = mt_free();
	uint32_t paragraphs = availableMemory / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs--;
		ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	}

	// align ptr
	m = (uint32_t) ptr;
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
}


void I_Quit(void)
{
	I_Shutdown();

	printf("\n");
	exit(0);
}


void I_Error(const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	printf("Doom8088: Sinclair QL Edition\n");

	D_DoomMain(argc, argv);
	return 0;
}
