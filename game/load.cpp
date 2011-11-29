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

#include <stdio.h>
#include <string.h>

#include "SDL_endian.h"

#include "myerror.h"
#include "load.h"


SDL_Texture *Load_Title(FrameBuf *screen, int title_id)
{
	char file[256];
	
	SDL_snprintf(file, sizeof(file), "Images/Maelstrom_Titles#%d.bmp", title_id);
	return screen->LoadImage(file);
}

SDL_Texture *GetCIcon(FrameBuf *screen, short id)
{
	char file[256];

	SDL_snprintf(file, sizeof(file), "Images/Maelstrom_Icon#%d.bmp", id);
	return screen->LoadImage(file);
}

SDL_Texture *GetSprite(FrameBuf *screen, short id, bool large)
{
	char file[256];
	
	/* Open the image file -- we know its colormap is our global one */
	SDL_snprintf(file, sizeof(file), "Sprites/Maelstrom_%s#%d.bmp", large ? "icl" : "ics", id);
	return screen->LoadImage(file);
}
