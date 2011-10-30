/*
    Maelstrom: Open Source version of the classic game by Ambrosia Software
    Copyright (C) 1997-2011  Sam Lantinga

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/* 
   This file handles the cheat dialogs and the high score file
*/

#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdio.h>

#include "SDL_endian.h"

#include "Maelstrom_Globals.h"
#include "load.h"

#define MAELSTROM_SCORES	"Maelstrom-Scores"
#define NUM_SCORES		10		// Do not change this!

/* Everyone can write to scores file if defined to 0 */
#define SCORES_PERMMASK		0

Scores hScores[NUM_SCORES];

void LoadScores(void)
{
	SDL_RWops *scores_src;
	int i;

	memset(&hScores, 0, sizeof(hScores));

	scores_src = PHYSFSRWOPS_openRead(MAELSTROM_SCORES);
	if ( scores_src != NULL ) {
		for ( i=0; i<NUM_SCORES; ++i ) {
			SDL_RWread(scores_src, hScores[i].name,
			           sizeof(hScores[i].name), 1);
			hScores[i].wave = SDL_ReadBE32(scores_src);
			hScores[i].score = SDL_ReadBE32(scores_src);
		}
		SDL_RWclose(scores_src);
	}
}

void SaveScores(void)
{
	SDL_RWops *scores_src;
	int i;
#ifdef unix
	int omask;
#endif

#ifdef unix
	omask=umask(SCORES_PERMMASK);
#endif
	scores_src = PHYSFSRWOPS_openWrite(MAELSTROM_SCORES);
	if ( scores_src != NULL ) {
		for ( i=0; i<NUM_SCORES; ++i ) {
			SDL_RWwrite(scores_src, hScores[i].name,
			            sizeof(hScores[i].name), 1);
			SDL_WriteBE32(scores_src, hScores[i].wave);
			SDL_WriteBE32(scores_src, hScores[i].score);
		}
		SDL_RWclose(scores_src);
	} else {
		error("Warning: Couldn't save scores to %s\n",
						MAELSTROM_SCORES);
	}
#ifdef unix
	umask(omask);
#endif
}

void ZapHighScores(UIDialog *dialog, int status)
{
	if (status) {
		memset(hScores, 0, sizeof(hScores));
		SaveScores();
		gLastHigh = -1;

		/* Fade the screen and redisplay scores */
		screen->FadeOut();
		Delay(SOUND_DELAY);
		sound->PlaySound(gExplosionSound, 5);
		gUpdateBuffer = true;
	}
}

