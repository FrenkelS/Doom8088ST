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
#include <clib/graphics_protos.h>
#include <devices/keyboard.h>
#include <hardware/dmabits.h>
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

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	kb_mp = CreatePort(0, 0);
	kb_io = (struct IOStdReq *)CreateExtIO(kb_mp, sizeof(struct IOStdReq));
	OpenDevice("keyboard.device", 0L, (struct IORequest *) kb_io, 0);
	kb_io->io_Command = KBD_READMATRIX;
	kb_io->io_Length  = SysBase->LibNode.lib_Version >= 36 ? KB_MATRIX_SIZE : 13;

	isKeyboardIsrSet = true;
}


static void I_ShutdownKeyboard(void)
{
	CloseDevice((struct IORequest *)kb_io);
	DeleteExtIO((struct IORequest *)kb_io);

	DeletePort(kb_mp);
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
	static uint8_t kb_matrix[KB_MATRIX_SIZE * 2];
	static uint8_t *kb_matrix_cur = &kb_matrix[0];
	static uint8_t *kb_matrix_prv = &kb_matrix[KB_MATRIX_SIZE];

	uint8_t diff;
	uint8_t *tmp = kb_matrix_cur;
	kb_matrix_cur = kb_matrix_prv;
	kb_matrix_prv = tmp;

	// read keyboard
	kb_io->io_Data = (APTR) kb_matrix_cur;
	DoIO((struct IORequest *)kb_io);


	diff = kb_matrix_prv[1] ^ kb_matrix_cur[1];
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[1] & (1 << 3), KEYD_MINUS);			// -
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[1] & (1 << 4), KEYD_PLUS);			// +

	diff = kb_matrix_prv[2] ^ kb_matrix_cur[2];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[2] & (1 << 0), 'q');					// q

	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[2] & (1 << 2), 'e');					// e
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[2] & (1 << 3), 'r');					// r
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[2] & (1 << 4), 't');					// t
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[2] & (1 << 5), 'y');					// y

	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[2] & (1 << 7), 'i');					// i

	diff = kb_matrix_prv[3] ^ kb_matrix_cur[3];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[3] & (1 << 0), 'o');					// o
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[3] & (1 << 1), 'p');					// p
	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[3] & (1 << 2), KEYD_BRACKET_LEFT);	// [
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[3] & (1 << 3), KEYD_BRACKET_RIGHT);	// ]

	diff = kb_matrix_prv[4] ^ kb_matrix_cur[4];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[4] & (1 << 0), 'a');					// a
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[4] & (1 << 1), 's');					// s
	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[4] & (1 << 2), 'd');					// d
	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[4] & (1 << 3), 'f');					// f

	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[4] & (1 << 5), 'h');					// h

	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[4] & (1 << 7), 'k');					// k

	diff = kb_matrix_prv[5] ^ kb_matrix_cur[5];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[5] & (1 << 0), 'l');					// l

	diff = kb_matrix_prv[6] ^ kb_matrix_cur[6];

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[6] & (1 << 3), 'c');					// c
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[6] & (1 << 4), 'v');					// v
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[6] & (1 << 5), 'b');					// b
	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[6] & (1 << 6), 'n');					// n

	diff = kb_matrix_prv[7] ^ kb_matrix_cur[7];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[7] & (1 << 0), KEYD_L);				// ,
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[7] & (1 << 1), KEYD_R);				// .

	diff = kb_matrix_prv[8] ^ kb_matrix_cur[8];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[8] & (1 << 0), KEYD_A);				// Space

	if (diff & (1 << 2)) I_PostEvent(kb_matrix_cur[8] & (1 << 2), KEYD_SELECT);			// Tab

	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[8] & (1 << 4), KEYD_A); 				// Enter
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[8] & (1 << 5), KEYD_START);			// Esc

	diff = kb_matrix_prv[9] ^ kb_matrix_cur[9];

	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[9] & (1 << 4), KEYD_UP);				// Up
	if (diff & (1 << 5)) I_PostEvent(kb_matrix_cur[9] & (1 << 5), KEYD_DOWN);			// Down
	if (diff & (1 << 6)) I_PostEvent(kb_matrix_cur[9] & (1 << 6), KEYD_RIGHT);			// Right
	if (diff & (1 << 7)) I_PostEvent(kb_matrix_cur[9] & (1 << 7), KEYD_LEFT);			// Left

	diff = kb_matrix_prv[11] ^ kb_matrix_cur[11];

	if (diff & (1 << 1)) I_Quit();														// F10

	diff = kb_matrix_prv[12] ^ kb_matrix_cur[12];
	if (diff & (1 << 0)) I_PostEvent(kb_matrix_cur[12] & (1 << 0), KEYD_SPEED);			// Left Shift
	if (diff & (1 << 1)) I_PostEvent(kb_matrix_cur[12] & (1 << 1), KEYD_SPEED);			// Right Shift

	if (diff & (1 << 3)) I_PostEvent(kb_matrix_cur[12] & (1 << 3), KEYD_B);				// Ctrl
	if (diff & (1 << 4)) I_PostEvent(kb_matrix_cur[12] & (1 << 4), KEYD_STRAFE);		// Alt
}


//**************************************************************************************
//
// Audio
//

extern struct Custom custom;

static uint16_t zeroIndex = 0;
static int8_t __chip sndBuffer[18600 - 8 - 32];


static void PCFX_Stop(void)
{
	custom.dmacon = DMAF_AUD0;
}


void PCFX_Play(int16_t lumpnum)
{
	PCFX_Stop();

	const uint16_t *sndLump = W_GetLumpByNum(lumpnum);
	uint16_t sndLength = sndLump[0];
	memcpy(sndBuffer, sndLump + 1, sndLength);
	if (sndLength < zeroIndex)
		memset(sndBuffer + sndLength, 0, zeroIndex - sndLength);
	zeroIndex = sndLength;
	Z_ChangeTagToCache(sndLump);

	//custom.aud[0].ac_len = sndLength / 2;
	custom.dmacon = DMAF_SETCLR | DMAF_AUD0;
}


void PCFX_Init(void)
{
	PCFX_Stop();

	extern struct GfxBase *GfxBase;
	boolean pal = (((struct GfxBase *) GfxBase)->DisplayFlags & PAL) == PAL;

	custom.aud[0].ac_ptr = (uint16_t *)sndBuffer;
	custom.aud[0].ac_len = sizeof(sndBuffer) / sizeof(uint16_t);
	custom.aud[0].ac_per = (pal ? 3546895 : 3579545) / 11025;
	custom.aud[0].ac_vol = 64;
}


void PCFX_Shutdown(void)
{
	PCFX_Stop();
}


//**************************************************************************************
//
// Returns time in 1/35th second tics.
//

static boolean isTimerSet;


int32_t I_GetTime(void)
{
	return clock() * TICRATE / CLOCKS_PER_SEC;
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
	uint32_t availableMemory = AvailMem(MEMF_ANY | MEMF_LARGEST);
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
