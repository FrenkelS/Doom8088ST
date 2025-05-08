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
 *      Atari ST implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <mint/osbind.h>
#include <mint/sysvars.h>

#include "doomdef.h"
#include "doomtype.h"
#include "compiler.h"
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

static void *oldkeyboardisr;


#define ACIA_RX_OVERRUN (1<<5)
#define ACIA_RX_DATA    (1<<0)
static volatile uint8_t *pAciaCtrl = (void*) 0xfffc00; 
static volatile uint8_t *pAciaData = (void*) 0xfffc02;
static volatile uint8_t *pMfpIsrb  = (void*) 0xfffa11;

#define KBDQUESIZE 32
static uint8_t input_buffer[KBDQUESIZE];
static uint8_t *next_read = input_buffer, *next_write = input_buffer;


__attribute__((interrupt)) static void I_KeyboardISR(void)
{
	uint8_t aciaCtrl = *pAciaCtrl;
	if (aciaCtrl & ACIA_RX_OVERRUN) {
		// Handle RX overrun
		*pAciaData;
	} else {
		while (aciaCtrl & ACIA_RX_DATA) {
			*next_write++ = *pAciaData;
			if (next_write == input_buffer + sizeof(input_buffer)) {
				next_write = input_buffer;
			}
			aciaCtrl = *pAciaCtrl;
		}
	}

	// Clear interrupt
	*pMfpIsrb &= ~(1<<6);
}


void I_InitKeyboard(void)
{
	Super(0L);
	oldkeyboardisr = *(void**)0x118;
	*(void**)0x118 = I_KeyboardISR;
	isKeyboardIsrSet = true;
}


void I_StartTic(void)
{
	while (next_read != next_write) {
		uint8_t *next_read_copy = next_read;
		uint8_t k = *next_read++;
		if (next_read == input_buffer + sizeof(input_buffer))
			next_read = input_buffer;

		if (k == 0) {
			// Ignore. Slightly worrying that we're seeing these.
			continue;
		}

		event_t ev;
		if (k & 0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;

		// Atari ST keyboard layout: https://temlib.org/AtariForumWiki/index.php/Atari_ST_Scancode_diagram_by_Unseen_Menace
		k &= 0x7f;
		switch (k)
		{
			case 0x01:
				ev.data1 = KEYD_START;
				break;
			case 0x1c:
			case 0x39:
				ev.data1 = KEYD_A;
				break;
			case 0x2a:
			case 0x36:
				ev.data1 = KEYD_SPEED;
				break;
			case 0x48:
				ev.data1 = KEYD_UP;
				break;
			case 0x50:
				ev.data1 = KEYD_DOWN;
				break;
			case 0x4b:
				ev.data1 = KEYD_LEFT;
				break;
			case 0x4d:
				ev.data1 = KEYD_RIGHT;
				break;
			case 0x0f:
				ev.data1 = KEYD_SELECT;
				break;
			case 0x1d:
				ev.data1 = KEYD_B;
				break;
			case 0x38:
				ev.data1 = KEYD_STRAFE;
				break;
			case 0x33:
				ev.data1 = KEYD_L;
				break;
			case 0x34:
				ev.data1 = KEYD_R;
				break;
			case 0x0c:
				ev.data1 = KEYD_MINUS;
				break;
			case 0x0d:
				ev.data1 = KEYD_PLUS;
				break;
			case 0x1a:
				ev.data1 = KEYD_BRACKET_LEFT;
				break;
			case 0x1b:
				ev.data1 = KEYD_BRACKET_RIGHT;
				break;

			case 0x44:
				I_Quit();

			default:
				if (0x10 <= k && k <= 0x19)
				{
					ev.data1 = "qwertyuiop"[k - 0x10];
					break;
				}
				else if (0x1e <= k && k <= 0x26)
				{
					ev.data1 = "asdfghjkl"[k - 0x1e];
					break;
				}
				else if (0x2c <= k && k <= 0x32)
				{
					ev.data1 = "zxcvbnm"[k - 0xc];
					break;
				}
				else
					continue;
		}
		D_PostEvent(&ev);
	}
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

static int32_t basetime;


int32_t I_GetTime(void)
{
    return (*_hz_200 - basetime) * TICRATE / 200;
}


void I_InitTimer(void)
{
	basetime = *_hz_200;
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
		*(void **)0x118 = oldkeyboardisr;
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
	printf("Doom1ST System Startup\n");

	D_DoomMain(argc, argv);
	return 0;
}
