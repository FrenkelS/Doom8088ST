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
#include <time.h>
#include <clib/alib_protos.h>
#include <devices/keyboard.h>
#include <proto/exec.h>

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

#define KB_MATRIX_SIZE 16

static struct MsgPort  *kb_mp;
static struct IOStdReq *kb_io;
static uint8_t kb_matrix[KB_MATRIX_SIZE * 2];
static uint8_t *kb_matrix_cur = &kb_matrix[0];
static uint8_t *kb_matrix_prv = &kb_matrix[KB_MATRIX_SIZE];

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	kb_mp = CreatePort(0, 0);
	kb_io = (struct IOStdReq *) CreateExtIO(kb_mp, sizeof(struct IOStdReq));
	OpenDevice("keyboard.device", 0L, (struct IORequest *) kb_io, 0);
	kb_io->io_Command = KBD_READMATRIX;
	kb_io->io_Length  = SysBase->LibNode.lib_Version >= 36 ? KB_MATRIX_SIZE : 13;

	isKeyboardIsrSet = true;
}


static void I_ShutdownKeyboard(void)
{
	CloseDevice((struct IORequest *) kb_io);
	DeleteExtIO((struct IORequest *) kb_io);

	DeletePort(kb_mp);
}


#define SC_ESCAPE			0x45
#define SC_MINUS			0x0b
#define SC_PLUS				0x0c
#define SC_TAB				0x42
#define SC_BRACKET_LEFT		0x1a
#define SC_BRACKET_RIGHT	0x1b
#define SC_ENTER			0x44
#define SC_CTRL				0x63
#define SC_LSHIFT			0x60
#define SC_RSHIFT			0x61
#define SC_COMMA			0x38
#define SC_PERIOD			0x39
#define SC_ALT				0x64
#define SC_SPACE			0x40
#define SC_F10				0x59
#define SC_UPARROW			0x4c
#define SC_DOWNARROW		0x4d
#define SC_LEFTARROW		0x4f
#define SC_RIGHTARROW		0x4e

#define SC_Q	0x10
#define SC_P	0x19
#define SC_A	0x20
#define SC_L	0x28
#define SC_Z	0x31
#define SC_M	0x37


void I_StartTic(void)
{
	uint8_t *tmp = kb_matrix_cur;
	kb_matrix_cur = kb_matrix_prv;
	kb_matrix_prv = tmp;

	// read keyboard
	kb_io->io_Data = (APTR) kb_matrix_cur;
	DoIO((struct IORequest *) kb_io);

	for (int16_t i = 0; i < KB_MATRIX_SIZE; i++)
	{
		uint8_t diff = kb_matrix_prv[i] ^ kb_matrix_cur[i];
		for (int16_t bit = 0; bit < 8; bit++)
		{
			if (diff & (1 << bit))
			{
				d_event_t ev;
				ev.type = kb_matrix_cur[i] & (1 << bit) ? ev_keydown : ev_keyup;
				uint8_t k = (i * 8) | bit;

				switch (k)
				{
					case SC_ESCAPE:
						ev.data1 = KEYD_START;
						break;
					case SC_ENTER:
					case SC_SPACE:
						ev.data1 = KEYD_A;
						break;
					case SC_LSHIFT:
					case SC_RSHIFT:
						ev.data1 = KEYD_SPEED;
						break;
					case SC_UPARROW:
						ev.data1 = KEYD_UP;
						break;
					case SC_DOWNARROW:
						ev.data1 = KEYD_DOWN;
						break;
					case SC_LEFTARROW:
						ev.data1 = KEYD_LEFT;
						break;
					case SC_RIGHTARROW:
						ev.data1 = KEYD_RIGHT;
						break;
					case SC_TAB:
						ev.data1 = KEYD_SELECT;
						break;
					case SC_CTRL:
						ev.data1 = KEYD_B;
						break;
					case SC_ALT:
						ev.data1 = KEYD_STRAFE;
						break;
					case SC_COMMA:
						ev.data1 = KEYD_L;
						break;
					case SC_PERIOD:
						ev.data1 = KEYD_R;
						break;
					case SC_MINUS:
						ev.data1 = KEYD_MINUS;
						break;
					case SC_PLUS:
						ev.data1 = KEYD_PLUS;
						break;
					case SC_BRACKET_LEFT:
						ev.data1 = KEYD_BRACKET_LEFT;
						break;
					case SC_BRACKET_RIGHT:
						ev.data1 = KEYD_BRACKET_RIGHT;
						break;

					case SC_F10:
						I_Quit();

					default:
						if (SC_Q <= k && k <= SC_P)
						{
							ev.data1 = "qwertyuiop"[k - SC_Q];
							break;
						}
						else if (SC_A <= k && k <= SC_L)
						{
							ev.data1 = "asdfghjkl"[k - SC_A];
							break;
						}
						else if (SC_Z <= k && k <= SC_M)
						{
							ev.data1 = "zxcvbnm"[k - SC_Z];
							break;
						}
						else
							continue;
				}
				D_PostEvent(&ev);
			}
		}
	}
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

static clock_t basetime;

static boolean isTimerSet;


int32_t I_GetTime(void)
{
	return (clock() - basetime) * TICRATE / CLOCKS_PER_SEC;
}


void I_InitTimer(void)
{
	basetime = clock();

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
	uint32_t availableMemory = AvailMem(MEMF_ANY);
	uint32_t paragraphs = availableMemory < 8 * 1024 * 1024L ? availableMemory / PARAGRAPH_SIZE : 8 * 1024 * 1024L / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
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
