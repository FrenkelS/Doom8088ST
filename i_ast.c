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
#include <mint/cookie.h>
#include <mint/osbind.h>

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

static void *oldkeyboardisr;


#define ACIA_RX_OVERRUN (1<<5)
#define ACIA_RX_DATA    (1<<0)
static volatile uint8_t *pAciaCtrl = (void*) 0xfffc00;
static volatile uint8_t *pAciaData = (void*) 0xfffc02;
static volatile uint8_t *pMfpIsrb  = (void*) 0xfffa11;

#define KBDQUESIZE 32
static uint8_t keyboardqueue[KBDQUESIZE];
static int16_t kbdtail, kbdhead;


__attribute__((interrupt)) static void I_KeyboardISR(void)
{
	uint8_t aciaCtrl = *pAciaCtrl;
	if (aciaCtrl & ACIA_RX_OVERRUN) {
		// Handle RX overrun
		*pAciaData;
	} else {
		while (aciaCtrl & ACIA_RX_DATA) {
			keyboardqueue[kbdhead & (KBDQUESIZE - 1)] = *pAciaData;
			kbdhead++;
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


// Atari ST keyboard layout: https://temlib.org/AtariForumWiki/index.php/Atari_ST_Scancode_diagram_by_Unseen_Menace
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
	while (kbdtail < kbdhead)
	{
		uint8_t k = keyboardqueue[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		// special codes
		if (k >= 0xf6)
		{
			char packetSize;
			switch (k)
			{
				case 0xf6: packetSize = (Kbdvbase())->kbstate; break;
				case 0xf7: packetSize = 4; break;
				case 0xf8:
				case 0xf9:
				case 0xfa:
				case 0xfb: packetSize = 2; break;
				case 0xfc: packetSize = 6; break;
				case 0xfd: packetSize = 2; break;
				case 0xfe:
				case 0xff: packetSize = 1; break;
			}
			kbdtail += packetSize;
			continue;
		}

		event_t ev;
		if (k & 0x80)
			ev.type = ev_keyup;
		else
			ev.type = ev_keydown;

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
// Audio
//
// This code is based on
// https://www.fxjavadevblog.fr/m68k-atari-st-ym-player/#interagir-avec-le-ym-2149

static volatile uint8_t *PSG_REGISTER_INDEX_ADDRESS = (void*) 0xFF8800;
static volatile uint8_t *PSG_REGISTER_DATA_ADDRESS  = (void*) 0xFF8802;
#define PSG_R0_TONE_A_PITCH_LOW_BYTE  0
#define PSG_R1_TONE_A_PITCH_HIGH_BYTE 1
#define PSG_R7_MIXER_MODE 7
#define PSG_R8_VOLUME_CHANNEL_A 8


static uint16_t	data[146];
static int16_t	PCFX_LengthLeft;
static const uint16_t *PCFX_Sound = NULL;
static uint16_t	PCFX_LastSample = 0;


inline static void write_PSG(uint8_t registerIndex, uint8_t registerValue)
{
	*PSG_REGISTER_INDEX_ADDRESS = registerIndex;
	*PSG_REGISTER_DATA_ADDRESS  = registerValue;
}


#define	lobyte(x)	(((uint8_t *)&(x))[1])
#define	hibyte(x)	(((uint8_t *)&(x))[0])

static void PCFX_Service(void)
{
	if (PCFX_Sound)
	{
		uint16_t value = *PCFX_Sound++;

		if (value != PCFX_LastSample)
		{
			PCFX_LastSample = value;
			// now write 12 bits value into R0 and R1
			write_PSG(PSG_R0_TONE_A_PITCH_LOW_BYTE,  lobyte(value)); // first 8 bits into R0. PSG_APITCHLOW = 0 (R0)
			write_PSG(PSG_R1_TONE_A_PITCH_HIGH_BYTE, hibyte(value)); // last  4 bits into R1, so let's ignore first 8 bits. PSG_APITCHHIGH = 1 (R1)
		}

		if (--PCFX_LengthLeft == 0)
		{
			write_PSG(PSG_R8_VOLUME_CHANNEL_A, 0b00000000); // volume channel A, OFF

			PCFX_Sound      = NULL;
			PCFX_LastSample = 0;
		}
	}
}


static void PCFX_Stop(void)
{
	if (PCFX_Sound == NULL)
		return;

	Jdisint(13);

	write_PSG(PSG_R8_VOLUME_CHANNEL_A, 0b00000000); // volume channel A, OFF

	PCFX_Sound      = NULL;
	PCFX_LastSample = 0;

	Jenabint(13);
}


typedef struct {
	uint16_t	length;
	uint16_t	data[];
} pcspkmuse_t;


void PCFX_Play(int16_t lumpnum)
{
	PCFX_Stop();

	const pcspkmuse_t *pcspkmuse = W_GetLumpByNum(lumpnum);
	PCFX_LengthLeft = pcspkmuse->length;
	memcpy(data, pcspkmuse->data, pcspkmuse->length * sizeof(uint16_t));
	Z_ChangeTagToCache(pcspkmuse);

	Jdisint(13);

	PCFX_Sound = &data[0];

	write_PSG(PSG_R8_VOLUME_CHANNEL_A, 0b00000111); // volume channel A, ON

	Jenabint(13);
}


void PCFX_Init(void)
{
	write_PSG(PSG_R7_MIXER_MODE, 0b00111110); // activate (0) only Tone on channel A, yes activation is 0 !
}


void PCFX_Shutdown(void)
{
	PCFX_Stop();
	write_PSG(PSG_R7_MIXER_MODE, 0b00111111); // mixer, deactivate (1 !) all
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

static volatile int32_t ticcount;

static boolean isTimerSet;


static volatile uint8_t *pMfpIsra  = (void*) 0xfffa0f;


__attribute__((interrupt)) static void I_TimerISR(void)
{
	ticcount++;
	PCFX_Service();

	// Clear interrupt
	*pMfpIsra &= ~(1<<5);
}


int32_t I_GetTime(void)
{
	return ticcount >> 2;
}


void I_InitTimer(void)
{
	// 7 -> 200
	// 2457600 / (200 * 88) ~= 140 Hz
	Xbtimer(XB_TIMERA, 7, 88, I_TimerISR);

	isTimerSet = true;
}


static void I_ShutdownTimer(void)
{
	Xbtimer(XB_TIMERA, 0, 0, NULL);
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	uint8_t* ptr;
	uint32_t paragraphs;

	if (Getcookie(C__FRB, NULL) == C_FOUND)
	{
		uint32_t availableMemory = Mxalloc(-1, MX_TTRAM);
		paragraphs = availableMemory < 8 * 1024 * 1024L ? availableMemory / PARAGRAPH_SIZE : 8 * 1024 * 1024L / PARAGRAPH_SIZE;
		ptr = (uint8_t*)Mxalloc(paragraphs * PARAGRAPH_SIZE, MX_TTRAM);
		while (!ptr)
		{
			paragraphs--;
			ptr = (uint8_t*)Mxalloc(paragraphs * PARAGRAPH_SIZE, MX_TTRAM);
		}
	}
	else
	{
		uint32_t availableMemory = Malloc(-1);
		paragraphs = availableMemory < 8 * 1024 * 1024L ? availableMemory / PARAGRAPH_SIZE : 8 * 1024 * 1024L / PARAGRAPH_SIZE;
		ptr = malloc(paragraphs * PARAGRAPH_SIZE);
		while (!ptr)
		{
			paragraphs--;
			ptr = malloc(paragraphs * PARAGRAPH_SIZE);
		}
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
		*(void **)0x118 = oldkeyboardisr;

	I_ShutdownSound();

	if (isTimerSet)
		I_ShutdownTimer();
}


void I_Quit(void)
{
	I_Shutdown();

	printf("\r\n");
	exit(0);
}


void I_Error(const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\r\n");
	exit(1);
}


int main(int argc, const char * const * argv)
{
	printf("Doom8088: Atari ST Edition\r\n");

	Super(0L);

	D_DoomMain(argc, argv);
	return 0;
}
