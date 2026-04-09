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
 *      Archimedes implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>
#include <swis.h>
#include <time.h>

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
#include "d_main.h"
#include "i_system.h"

#include "globdata.h"

#include <archie/keyboard.h>


//#define TIMEDEMO


void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


static boolean isGraphicsModeSet = false;


//**************************************************************************************
//
// Functions that are available on other operating systems, but not on Archimedes
//

int stricmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		uint8_t c1 = (uint8_t)toupper(*s1);
		uint8_t c2 = (uint8_t)toupper(*s2);

		if (c1 != c2)
			return c1 - c2;

		s1++;
		s2++;
	}

	return (uint8_t)toupper(*s1) - (uint8_t)toupper(*s2);
}


// There's a bug in ArchieSDK's implementation of strncpy
char* strncpy(char* dest, const char* src, size_t n)
{
	char* tmp = dest;

	while (n--)
	{
		if (!(*dest++ = *src++))
		{
			while (n--)
				*dest++ = '\0';

			break;
		}
	}

	return tmp;
}


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


#define KB_MATRIX_SIZE 128

void I_StartTic(void)
{
	static uint8_t kb_matrix[KB_MATRIX_SIZE * 2];
	static uint8_t *kb_matrix_cur = &kb_matrix[0];
	static uint8_t *kb_matrix_prv = &kb_matrix[KB_MATRIX_SIZE];

	uint8_t *tmp = kb_matrix_cur;
	kb_matrix_cur = kb_matrix_prv;
	kb_matrix_prv = tmp;

	for (int_fast16_t key = 0; key < KB_MATRIX_SIZE; key++)
	{
		int16_t data1;
		switch (key)
		{
			case KEY_SHIFT:              data1 = KEYD_SPEED; break;
			case KEY_CTRL:               data1 = KEYD_B; break;
			case KEY_ALT:                data1 = KEYD_STRAFE; break;
			case KEY_LSHIFT:             data1 = KEYD_SPEED; break;
			case KEY_LCTRL:              data1 = KEYD_B; break;
			case KEY_LALT:               data1 = KEYD_STRAFE; break;
			case KEY_RSHIFT:             data1 = KEYD_SPEED; break;
			case KEY_RCTRL:              data1 = KEYD_B; break;
			case KEY_RALT:               data1 = KEYD_STRAFE; break;
			case KEY_MINUS:              data1 = KEYD_MINUS; break;
			case KEY_LEFT:               data1 = KEYD_LEFT; break;
			case KEY_DOWN:               data1 = KEYD_DOWN; break;
			case KEY_SQUAREBRACKETSTART: data1 = KEYD_BRACKET_LEFT; break;
			case KEY_UP:                 data1 = KEYD_UP; break;
			case KEY_RETURN:             data1 = KEYD_A; break;
			case KEY_SQUAREBRACKETEND:   data1 = KEYD_BRACKET_RIGHT; break;
			case KEY_PLUS:               data1 = KEYD_PLUS; break;
			case KEY_TAB:                data1 = KEYD_SELECT; break;
			case KEY_SPACE:              data1 = KEYD_A; break;
			case KEY_COMMA:              data1 = KEYD_L; break;
			case KEY_PERIOD:             data1 = KEYD_R; break;
			case KEY_ESC:                data1 = KEYD_START; break;
			case KEY_RIGHT:              data1 = KEYD_RIGHT; break;
			case KEY_A:                  data1 = 'a'; break;
			case KEY_B:                  data1 = 'b'; break;
			case KEY_C:                  data1 = 'c'; break;
			case KEY_D:                  data1 = 'd'; break;
			case KEY_E:                  data1 = 'e'; break;
			case KEY_F:                  data1 = 'f'; break;
			//case KEY_G:                  data1 = ; break;
			case KEY_H:                  data1 = 'h'; break;
			case KEY_I:                  data1 = 'i'; break;
			//case KEY_J:                  data1 = ; break;
			case KEY_K:                  data1 = 'k'; break;
			case KEY_L:                  data1 = 'l'; break;
			//case KEY_M:                  data1 = ; break;
			case KEY_N:                  data1 = 'n'; break;
			case KEY_O:                  data1 = 'o'; break;
			case KEY_P:                  data1 = 'p'; break;
			case KEY_Q:                  data1 = 'q'; break;
			case KEY_R:                  data1 = 'r'; break;
			case KEY_S:                  data1 = 's'; break;
			case KEY_T:                  data1 = 't'; break;
			//case KEY_U:                  data1 = ; break;
			case KEY_V:                  data1 = 'v'; break;
			//case KEY_W:                  data1 = ; break;
			//case KEY_X:                  data1 = ; break;
			case KEY_Y:                  data1 = 'y'; break;
			//case KEY_Z:                  data1 = ; break;
			//case KEY_F10: I_Quit(); break;
			case KEY_KP6:                data1 = KEYD_RIGHT; break;
			case KEY_KP8:                data1 = KEYD_UP; break;
			case KEY_KPENTER:            data1 = KEYD_A; break;
			case KEY_KP4:                data1 = KEYD_LEFT; break;
			case KEY_KP2:                data1 = KEYD_DOWN; break;
			default: continue;
		}

		kb_matrix_cur[key] = k_checkKeypress(key);
		if (kb_matrix_cur[key] && !kb_matrix_prv[key])
			I_PostEvent(true,  data1);
		if (!kb_matrix_cur[key] && kb_matrix_prv[key])
			I_PostEvent(false, data1);
	}

	if (k_checkKeypress(KEY_F10))
		I_Quit();
}


//**************************************************************************************
//
// Audio
//

void DMX_Play(sfxenum_t id)
{
}


void DMX_Init(void)
{
}


void DMX_Init2(void)
{
}


void DMX_Shutdown(void)
{
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

int32_t I_GetTime(void)
{
    return clock() * TICRATE / CLOCKS_PER_SEC;
}


void I_InitTimer(void)
{
	// Do nothing
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	_kernel_swi_regs regs;

	_kernel_swi(OS_ReadMemMapInfo, &regs, &regs);

	uint32_t availableMemory = regs.r[0] * regs.r[1];
	uint32_t paragraphs = availableMemory < 8 * 1024 * 1024L ? availableMemory / PARAGRAPH_SIZE : 8 * 1024 * 1024L / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs -= regs.r[0] / PARAGRAPH_SIZE;
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
	printf("Doom8088: Archimedes Edition\n");

#if defined TIMEDEMO
	const char * const args[] = {"DOOM8088", "-timedemo", "demo3"};
	D_DoomMain(3, args);
#else
	D_DoomMain(argc, argv);
#endif

	return 0;
}
