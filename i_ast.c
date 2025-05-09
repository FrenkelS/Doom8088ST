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
	oldkeyboardisr = *(void**)0x118;
	*(void**)0x118 = I_KeyboardISR;
	isKeyboardIsrSet = true;
}


#define SC_ESCAPE			0x01
#define SC_MINUS			0x0c
#define SC_PLUS				0x0d
#define SC_TAB				0x0f
#define SC_BRACKET_LEFT		0x1a
#define SC_BRACKET_RIGHT	0x1b
#define SC_ENTER			0x1c
#define SC_CTRL				0x1d
#define SC_LSHIFT			0x2a
#define SC_RSHIFT			0x36
#define SC_COMMA			0x33
#define SC_PERIOD			0x34
#define SC_ALT				0x38
#define SC_SPACE			0x39
#define SC_F10				0x44
#define SC_UPARROW			0x48
#define SC_DOWNARROW		0x50
#define SC_LEFTARROW		0x4b
#define SC_RIGHTARROW		0x4d

#define SC_Q	0x10
#define SC_P	0x19
#define SC_A	0x1e
#define SC_L	0x26
#define SC_Z	0x2c
#define SC_M	0x32


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
	printf("Doom8088: Atari ST Edition\n");

	Super(0L);

	D_DoomMain(argc, argv);
	return 0;
}
