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

#include <conio.h>
#include <dos.h>
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

void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


void I_InitGraphics(void)
{
	I_InitGraphicsHardwareSpecificCode();
	isGraphicsModeSet = true;
}


//**************************************************************************************
//
// Keyboard code
//

#define KEYBOARDINT 9
#define KBDQUESIZE 32
static byte keyboardqueue[KBDQUESIZE];
static int16_t kbdtail, kbdhead;
static boolean isKeyboardIsrSet = false;

#if defined __DJGPP__ 
static _go32_dpmi_seginfo oldkeyboardisr, newkeyboardisr;
#else
static void __interrupt __far (*oldkeyboardisr)(void);
#endif

static void __interrupt __far I_KeyboardISR(void)	
{
	// Get the scan code
	keyboardqueue[kbdhead & (KBDQUESIZE - 1)] = inp(0x60);
	kbdhead++;

	// Tell the XT keyboard controller to clear the key
	byte temp;
	outp(0x61, (temp = inp(0x61)) | 0x80);
	outp(0x61, temp);

	// acknowledge the interrupt
	outp(0x20, 0x20);
}


void I_InitKeyboard(void)
{
	replaceInterrupt(oldkeyboardisr, newkeyboardisr, KEYBOARDINT, I_KeyboardISR);
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
	//
	// process keyboard events
	//
	byte k;
	event_t ev;

	while (kbdtail < kbdhead)
	{
		k = keyboardqueue[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		// extended keyboard shift key bullshit
		if ((k & 0x7f) == SC_LSHIFT || (k & 0x7f) == SC_RSHIFT)
		{
			if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RSHIFT;
		}

		if (k == 0xe0)
			continue;               // special / pause keys
		if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k == 0xc5 && keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0x9d)
		{
			//ev.type  = ev_keydown;
			//ev.data1 = KEY_PAUSE;
			//D_PostEvent(&ev);
			continue;
		}

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
// Low level timer task scheduler.
//

/*---------------------------------------------------------------------
   Global variables
---------------------------------------------------------------------*/

typedef struct
{
	void				(*taskService)(void);
	int32_t				rate;
	volatile int32_t	count;
	boolean				active;
} task_t;


#define LOBYTE(w)	(((uint8_t *)&(w))[0])
#define HIBYTE(w)	(((uint8_t *)&(w))[1])


#define MAX_TASKS 2
#define MAX_SERVICE_RATE 0x10000L

static task_t tasks[MAX_TASKS];


#if defined __DJGPP__
static _go32_dpmi_seginfo OldInt8, NewInt8;
#else
static void __interrupt __far (*OldInt8)(void);
#endif


static volatile int32_t taskServiceRate  = MAX_SERVICE_RATE;
static volatile int32_t taskServiceCount = 0;

static boolean isTS_Installed = false;


/*---------------------------------------------------------------------
   Function: TS_SetClockSpeed

   Sets the rate of the 8253 timer.
---------------------------------------------------------------------*/

static void TS_SetClockSpeed(int32_t speed)
{
	_disable();

	if (0 < speed && speed < MAX_SERVICE_RATE)
		taskServiceRate = speed;
	else
		taskServiceRate = MAX_SERVICE_RATE;

	outp(0x43, 0x36);
	outp(0x40, LOBYTE(taskServiceRate));
	outp(0x40, HIBYTE(taskServiceRate));

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_ServiceSchedule

   Interrupt service routine
---------------------------------------------------------------------*/

static void __interrupt __far TS_ServiceSchedule (void)
{
	for (int16_t i = 0; i < MAX_TASKS; i++)
	{
		task_t *task = &tasks[i];
		if (task->active)
		{
			task->count += taskServiceRate;

			while (task->count >= task->rate)
			{
				task->count -= task->rate;
				task->taskService();
			}
		}
	}

	taskServiceCount += taskServiceRate;
	if (taskServiceCount > 0xffffL)
	{
		taskServiceCount &= 0xffff;
		_chain_intr(OldInt8);
	} else {
		outp(0x20, 0x20);
	}
}


/*---------------------------------------------------------------------
   Function: TS_Startup

   Sets up the task service routine.
---------------------------------------------------------------------*/

#define TIMERINT 8

static void TS_Startup(void)
{
	if (!isTS_Installed)
	{
		taskServiceRate  = MAX_SERVICE_RATE;
		taskServiceCount = 0;

		replaceInterrupt(OldInt8, NewInt8, TIMERINT, TS_ServiceSchedule);

		isTS_Installed = true;
	}
}


/*---------------------------------------------------------------------
   Function: TS_Shutdown

   Ends processing of all tasks.
---------------------------------------------------------------------*/

static void TS_Shutdown(void)
{
	if (isTS_Installed)
	{
		TS_SetClockSpeed(0);

		restoreInterrupt(TIMERINT, OldInt8, NewInt8);

		isTS_Installed = false;
	}
}


/*---------------------------------------------------------------------
   Function: TS_SetTimer

   Calculates the rate at which a task will occur and sets the clock
   speed if necessary.
---------------------------------------------------------------------*/

static uint16_t TS_SetTimer(int16_t tickBase)
{
	uint16_t speed = 1193182L / tickBase;
	if (speed < taskServiceRate)
		TS_SetClockSpeed(speed);

	return speed;
}


/*---------------------------------------------------------------------
   Function: TS_ScheduleTask

   Schedules a new task for processing.
---------------------------------------------------------------------*/

static void TS_ScheduleTask(void (*function)(void), int16_t rate, int16_t priority)
{
	if (!isTS_Installed)
		TS_Startup();

	tasks[priority].taskService = function;
	tasks[priority].rate        = TS_SetTimer(rate);
	tasks[priority].count       = 0;
	tasks[priority].active      = false;


	_disable();

	tasks[priority].active = true;

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_SetTimerToMaxTaskRate

   Finds the fastest running task and sets the clock to operate at
   that speed.
---------------------------------------------------------------------*/

static void TS_SetTimerToMaxTaskRate(void)
{
	_disable();

	int32_t maxServiceRate = MAX_SERVICE_RATE;

	for (int16_t i = 0; i < MAX_TASKS; i++)
		if (tasks[i].rate < maxServiceRate)
			maxServiceRate = tasks[i].rate;

	if (taskServiceRate != maxServiceRate)
		TS_SetClockSpeed(maxServiceRate);

	_enable();
}


/*---------------------------------------------------------------------
   Function: TS_Terminate

   Ends processing of a specific task.
---------------------------------------------------------------------*/

static void TS_Terminate(int16_t priority)
{
	_disable();

	tasks[priority].rate = MAX_SERVICE_RATE;

	TS_SetTimerToMaxTaskRate();

	_enable();
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

#define TIMER_PRIORITY 0

static volatile int32_t ticcount;

static boolean isTimerSet;


static void I_TimerISR(void)
{
	ticcount++;
}


int32_t I_GetTime(void)
{
    return ticcount;
}


void I_InitTimer(void)
{
	TS_ScheduleTask(I_TimerISR, TICRATE, TIMER_PRIORITY);

	isTimerSet = true;
}


static void I_ShutdownTimer(void)
{
	TS_Terminate(TIMER_PRIORITY);
	TS_Shutdown();
}


//**************************************************************************************
//
// Memory
//

uint8_t __far* I_ZoneBase(uint32_t *heapSize)
{
	unsigned int max, segment;
	_dos_allocmem(0xffff, &max);
	_dos_allocmem(max, &segment);
	*heapSize = (uint32_t)max * PARAGRAPH_SIZE;
	return D_MK_FP(segment, 0);
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

	if (isKeyboardIsrSet)
	{
		restoreInterrupt(KEYBOARDINT, oldkeyboardisr, newkeyboardisr);
	}
}


void I_Quit(void)
{
	I_Shutdown();

	int16_t lumpnum = W_GetNumForName("ENDOOM");
	W_ReadLumpByNum(lumpnum, D_MK_FP(0xb800, 0 + __djgpp_conventional_base));

	union REGS regs;
	regs.h.ah = 2;
	regs.h.bh = 0;
	regs.h.dl = 0;
	regs.h.dh = 23;
	int86(0x10, &regs, &regs);

	printf("\n");
	exit(0);
}


void I_Error (const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


static void tprintf(void)
{
	union REGS regs;

	char* msg = "                          DOOM8088 System Startup                           ";

	for (size_t i = 0; i < strlen(msg); )
	{
		regs.h.ah = 9;
		regs.h.al = msg[i];
		regs.w.cx = 1;
		regs.w.bx = (7 << 4) | 4;
		int86(0x10, &regs, &regs);

		regs.h.ah = 2;
		regs.h.bh = 0;
		regs.w.dx = ++i;
		int86(0x10, &regs, &regs);
	}

	printf("\n");
}


int main(int argc, const char * const * argv)
{
	I_SetScreenMode(3);

	tprintf();

	D_DoomMain(argc, argv);
	return 0;
}
