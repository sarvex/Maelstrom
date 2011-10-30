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

/* Game Logic interface routines and variables */

#include "netlogic/netlogic.h"

/* From logic.cpp */
extern void LogicUsage(void);
extern int  InitLogicData(void);
extern int  LogicParseArgs(char ***argvptr, int *argcptr);
extern int  InitLogic(void);
extern int  InitPlayerSprites(void);
extern void HaltLogic(void);
extern void SetControl(unsigned char which, int toggle);
extern int  SpecialKey(SDL_Keycode key);
extern int GetScore(void);

/* From game.cpp */
extern void NewGame(void);

/* From about.cpp */
extern void DoAbout(void);

/* From player.cpp */
extern Uint8 gPlayerShotColors[];
extern SDL_Texture *gPlayerShot;
extern Uint8 gEnemyShotColors[];
extern SDL_Texture *gEnemyShot;

