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

int32_t I_GetTime(void)
{
	// TODO implement timer
	static int32_t ticcount;
	return ticcount++;
}


void I_InitTimer(void)
{
	// Do nothing
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


static void I_ShutdownKeyboard(void)
{
	// Do nothing
}


void I_StartTic(void)
{
	// TODO
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

	if (isKeyboardIsrSet)
		I_ShutdownKeyboard();
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
