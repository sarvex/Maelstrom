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

#include "load.h"
#include "myerror.h"


SDL_Texture *Load_Title(FrameBuf *screen, int title_id)
{
	char file[256];
	SDL_Surface *bmp;
	SDL_Texture *title;
	
	/* Open the title file -- we know its colormap is our global one */
	sprintf(file, "Images/Maelstrom_Titles#%d.bmp", title_id);
	bmp = SDL_LoadBMP_RW(PHYSFSRWOPS_openRead(file), 1);
	if ( bmp == NULL ) {
		return(NULL);
	}

	/* Create an image from the BMP */
	title = screen->LoadImage(bmp);
	SDL_FreeSurface(bmp);
	return(title);
}

SDL_Texture *GetCIcon(FrameBuf *screen, short cicn_id)
{
	char file[256];
	SDL_Texture *cicn;
	SDL_RWops *cicn_src;
	Uint8 *pixels, *mask;
	Uint16 w, h;
	
	/* Open the cicn sprite file.. */
	sprintf(file, "Images/Maelstrom_Icon#%hd.cicn", cicn_id);
	if ( (cicn_src=PHYSFSRWOPS_openRead(file)) == NULL ) {
		error("GetCIcon(%hd): Can't open CICN %s: ",
					cicn_id, file);
		return(NULL);
	}

	w = SDL_ReadBE16(cicn_src);
	h = SDL_ReadBE16(cicn_src);
        pixels = new Uint8[w*h];
        if ( SDL_RWread(cicn_src, pixels, 1, w*h) != size_t(w*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
        mask = new Uint8[(w/8)*h];
        if ( SDL_RWread(cicn_src, mask, 1, (w/8)*h) != size_t((w/8)*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		delete[] mask;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
	SDL_RWclose(cicn_src);

	sprintf(file, "Maelstrom_Icon#%d.bmp", cicn_id);
	cicn = screen->LoadImage(w, h, pixels, mask, file);
	delete[] pixels;
	delete[] mask;
	if ( cicn == NULL ) {
		error("GetCIcon(%hd): Couldn't convert CICN!\n", cicn_id);
	}
	return(cicn);
}
