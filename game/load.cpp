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

#include "SDL_image.h"

#include "Maelstrom_Globals.h"
#include "load.h"

#include "../utils/physfsrwops.h"


SDL_Texture *Load_Texture(FrameBuf *screen, const char *folder, const char *name)
{
	static const char *extensions[] = {
		"png",
		"bmp",
	};
	static struct {
		int w, h;
		const char *suffix;
	} entries[] = {
		{ 2048, 1536, "_2048x1536" },
		{ 1024, 768, "_1024x768" },
		{ 960, 640, "_960x640" },
		{ 640, 480, "" },
		{ 480, 320, "_480x320" },
		{ 0, 0, "" }
	};
	int w, h;
	char file[256];

	// Use the game display area for determining which art set to use
	w = gScrnRect.w;
	h = gScrnRect.h;
	for (int i = 0; i < SDL_arraysize(entries); ++i) {
		if ( w < entries[i].w || h < entries[i].h ) {
			continue;
		}
		for (int j = 0; j < SDL_arraysize(extensions); ++j) {
			SDL_snprintf(file, sizeof(file), "%s%s/%s.%s",
					folder, entries[i].suffix, name, extensions[j]);
			SDL_Surface *surface = IMG_Load_RW(PHYSFSRWOPS_openRead(file), 1);
			if (surface) {
				SDL_Texture *texture = screen->LoadImage(surface);
				SDL_FreeSurface(surface);
				return texture;
			}
		}
	}
	return NULL;
}

SDL_Texture *Load_Image(FrameBuf *screen, const char *name)
{
	return Load_Texture(screen, "Images", name);
}

SDL_Texture *Load_Title(FrameBuf *screen, int title_id)
{
	char name[256];
	SDL_snprintf(name, sizeof(name), "Maelstrom_Titles#%d", title_id);
	return Load_Texture(screen, "Images", name);
}

SDL_Texture *GetCIcon(FrameBuf *screen, short id)
{
	char name[256];
	SDL_snprintf(name, sizeof(name), "Maelstrom_Icon#%d", id);
	return Load_Texture(screen, "Images", name);
}

SDL_Texture *GetSprite(FrameBuf *screen, short id, bool large)
{
	char name[256];
	SDL_snprintf(name, sizeof(name), "Maelstrom_%s#%d", large ? "icl" : "ics", id);
	return Load_Texture(screen, "Sprites", name);
}
