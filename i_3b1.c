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
 *      AT&T UNIX PC / PC 7300 / 3B1 implementation of i_system.h
 *
 *-----------------------------------------------------------------------------*/

#include <malloc.h>
#include <stdarg.h>
#include <termio.h>
#include <sys/param.h>
#include <sys/times.h>

#include "doomdef.h"
#include "a_pcfx.h"
#include "d_main.h"
#include "i_sound.h"
#include "i_system.h"


int ioctl( int fd, int cmd, ...);
int read(int _fildes, void *_buf, size_t _nbyte);
time_t times(struct tms *buffer);
int vprintf(const char *_format, va_list _ap);


void I_InitGraphicsHardwareSpecificCode(void);
void I_ShutdownGraphics(void);


static boolean isGraphicsModeSet = false;


//**************************************************************************************
//
// Functions that are available on other operating systems, but not on AT&T UNIX
//

int stricmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		uint8_t c1 = (uint8_t) _toupper(*s1);
		uint8_t c2 = (uint8_t) _toupper(*s2);

		if (c1 != c2)
			return c1 - c2;

		s1++;
		s2++;
	}

	return (uint8_t)_toupper(*s1) - (uint8_t)_toupper(*s2);
}


int abs(int x)
{
	return x >= 0 ? x: -x;
}


long labs(long x)
{
	return x >= 0 ? x : -x;
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
// Returns time in 1/35th second tics.
//

static time_t clock(void)
{
	struct tms temp;
	return times(&temp);
}


int32_t I_GetTime(void)
{
	return clock() * TICRATE / HZ;
}


void I_InitTimer(void)
{
	// Do nothing
}


//**************************************************************************************
//
// Keyboard code
//

static struct termio oldt;

static boolean isKeyboardIsrSet = false;


void I_InitKeyboard(void)
{
	struct termio newt;

	// Save old terminal settings
	ioctl(0, TCGETA, &oldt);
	newt = oldt;

	// Disable canonical mode & echo
	newt.c_lflag &= ~(ICANON | ECHO);
	newt.c_cc[VMIN]  = 0;
	newt.c_cc[VTIME] = 0;
	ioctl(0, TCSETA, &newt);

	isKeyboardIsrSet = true;
}


static void I_ShutdownKeyboard(void)
{
	ioctl(0, TCSETA, &oldt);
}


void I_StartTic(void)
{
	static time_t gamekeytimestamps[NUMKEYS];

	uint8_t buf[16];
	int n = read(0, buf, sizeof(buf));
	int i = 0;
	while (i < n)
	{
		d_event_t ev;
		ev.type = ev_keydown;

		if (buf[i] == 27)
		{
			if (i + 2 < n && buf[i + 1] == '[')
			{
				switch (buf[i + 2])
				{
					case 'A': ev.data1 = KEYD_UP;    break;
					case 'B': ev.data1 = KEYD_DOWN;  break;
					case 'C': ev.data1 = KEYD_RIGHT; break;
					case 'D': ev.data1 = KEYD_LEFT;  break;
					default: i += 3; continue;
				}
				i += 3;
			}
			else if (i + 2 < n && buf[i + 1] == 'N')
			{
				i += 3;
				continue;
			}
			else if (i + 2 < n && buf[i + 1] == 'O')
			{
				i += 3;
				continue;
			}
			else if (i + 1 < n)
			{
				i += 2;
				continue;
			}
			else
			{
				ev.data1 = KEYD_START;
				i++;
			}
		}
		else
		{
			switch (buf[i])
			{
				case 0x0a:
				case ' ': ev.data1 = KEYD_A;             break;
				case '8': ev.data1 = KEYD_UP;            break;
				case '2': ev.data1 = KEYD_DOWN;          break;
				case '4': ev.data1 = KEYD_LEFT;          break;
				case '6': ev.data1 = KEYD_RIGHT;         break;
				case   9: ev.data1 = KEYD_SELECT;        break;
				case '/': ev.data1 = KEYD_B;             break;
				case ',': ev.data1 = KEYD_L;             break;
				case '.': ev.data1 = KEYD_R;             break;
				case '-': ev.data1 = KEYD_MINUS;         break;
				case '=': ev.data1 = KEYD_PLUS;          break;
				case '[': ev.data1 = KEYD_BRACKET_LEFT;  break;
				case ']': ev.data1 = KEYD_BRACKET_RIGHT; break;
				case 'Q': I_Quit();                      break;
				default:  ev.data1 = buf[i];             break;
			}
			i++;
		}

		if (ev.data1 < NUMKEYS)
			gamekeytimestamps[ev.data1] = clock();

		D_PostEvent(&ev);
	}

	for (i = 0; i < NUMKEYS; i++)
	{
		if (gamekeytimestamps[i] != 0 && clock() - gamekeytimestamps[i] > (42 * HZ + 999) / 1000)
		{
			gamekeytimestamps[i] = 0;
			d_event_t ev;
			ev.type  = ev_keyup;
			ev.data1 = i;
			D_PostEvent(&ev);
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
	// Do nothing
}


void PCFX_Shutdown(void)
{
	// Do nothing
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	uint32_t availableMemory = 4 * 1024 * 1024L;
	uint32_t paragraphs = availableMemory / PARAGRAPH_SIZE;
	uint8_t *ptr = malloc(paragraphs * PARAGRAPH_SIZE);
	while (!ptr)
	{
		paragraphs -= NBPC / PARAGRAPH_SIZE;
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
	printf("Doom8088: AT&T UNIX PC Edition\n");

	D_DoomMain(argc, argv);
	return 0;
}
