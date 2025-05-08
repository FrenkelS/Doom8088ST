/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023-2025 by
 *  Frenkel Smeijers
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
 *  System interface for sound.
 *
 *-----------------------------------------------------------------------------
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdint.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "z_zone.h"

#include "m_swap.h"
#include "i_sound.h"
#include "w_wad.h"
#include "s_sound.h"

#include "d_player.h"
#include "doomtype.h"

#include "d_main.h"

#include "m_fixed.h"

#include "globdata.h"


int16_t I_StartSound(sfxenum_t id, int16_t channel, int16_t vol, int16_t sep)
{
	UNUSED(id);
	UNUSED(vol);
	UNUSED(sep);

	return channel;
}


void I_InitSound(void)
{
	// Do nothing
}


void I_InitSound2(void)
{
	// Do nothing
}


void I_ShutdownSound(void)
{
	// Do nothing
}


void I_PlaySong(musicenum_t handle, boolean looping)
{
	UNUSED(handle);
	UNUSED(looping);
}


void I_StopSong(musicenum_t handle)
{
	UNUSED(handle);
}

void I_SetMusicVolume(int16_t volume)
{
	UNUSED(volume);
}
