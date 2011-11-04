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

#include "../Maelstrom_Globals.h"
#include "object.h"
#include "player.h"
#include "globals.h"
#include "netplay.h"

/* Extra options specific to this logic module */
void LogicUsage(void)
{
	error(
"	-player N[@host][:port]	# Designate player N (at host and/or port)\n"
"	-deathmatch [N]		# Play deathmatch to N frags (default = 8)\n"
	);
}

/* Initialize special logic data */
int InitLogicData(void)
{
	/* Initialize network player data */
	if ( InitNetData() < 0 ) {
		return(-1);
	}
	gDeathMatch = 0;
	return(0);
}

/* Parse logic-specific command line arguments */
int LogicParseArgs(char ***argvptr, int *argcptr)
{
	char **argv = *argvptr;

	/* Check for the '-player' option */
	if ( strcmp(argv[1], "-player") == 0 ) {
		if ( ! argv[2] ) {
			error(
			"The '-player' option requires an argument!\n");
			PrintUsage();
		}
		if ( AddPlayer(argv[2]) < 0 )
			exit(1);
		++(*argvptr);
		--(*argcptr);
		return(0);
	}

	/* Check for the '-deathmatch' option */
	if ( strcmp(argv[1], "-deathmatch") == 0 ) {
		if ( argv[2] && ((gDeathMatch=atoi(argv[2])) > 0) ) {
			++(*argvptr);
			--(*argcptr);
		} else
			gDeathMatch = 8;
		return(0);
	}
	return(-1);
}

/* Do final logic initialization */
int InitLogic(void)
{
	/* Make sure we have a valid player list */
	if ( CheckPlayers() < 0 )
		return(-1);
	return(0);
}

void HaltLogic(void)
{
	HaltNetData();
}

/* Initialize the player sprites */
int InitPlayerSprites(void)
{
	int index;

	OBJ_LOOP(index, gNumPlayers)
		gPlayers[index] = new Player(index);
	return(0);
}

int SpecialKey(SDL_Keycode key)
{
	if ( key == SDLK_F1 ) {
		/* Special key -- switch displayed player */
		if ( ++gDisplayed == gNumPlayers )
			gDisplayed = 0;
		return(0);
	}
	return(-1);
}

void SetControl(unsigned char which, int toggle)
{
	QueueKey(toggle ? KEY_PRESS : KEY_RELEASE, which);
}

int GetScore(void)
{
	return(OurShip->GetScore());
}

