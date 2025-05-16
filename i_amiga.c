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
 *      Amiga implementation of i_system.h
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
// Keyboard code
//

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	isKeyboardIsrSet = true;
}


static void I_ShutdownKeyboard(void)
{

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
	UNUSED(lumpnum);
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

static volatile int32_t ticcount;

static boolean isTimerSet;


int32_t I_GetTime(void)
{
	return ticcount++;
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
// Exit code
//

static void I_Shutdown(void)
{
	if (isGraphicsModeSet)
		I_ShutdownGraphics();

	if (isKeyboardIsrSet)
		I_ShutdownKeyboard();

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


__stdargs int main(int argc, const char * const * argv)
{
	printf("Doom8088: Amiga Edition\n");

	D_DoomMain(argc, argv);
	return 0;
}
