
#include <stdio.h>
#include <string.h>

#include "SDL_endian.h"

#include "load.h"
#include "myerror.h"


SDL_Surface *Load_Title(FrameBuf *screen, int title_id)
{
	char file[256];
	SDL_Surface *bmp, *title;
	
	/* Open the title file -- we know its colormap is our global one */
	sprintf(file, "Images/Maelstrom_Titles#%d.bmp", title_id);
	bmp = SDL_LoadBMP_RW(PHYSFSRWOPS_openRead(file), 1);
	if ( bmp == NULL ) {
		return(NULL);
	}

	/* Create an image from the BMP */
	title = screen->LoadImage(bmp->w, bmp->h, (Uint8 *)bmp->pixels, NULL);
	SDL_FreeSurface(bmp);
	return(title);
}

SDL_Surface *GetCIcon(FrameBuf *screen, short cicn_id)
{
	char file[256];
	SDL_Surface *cicn;
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
        if ( SDL_RWread(cicn_src, pixels, 1, w*h) != (w*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
        mask = new Uint8[(w/8)*h];
        if ( SDL_RWread(cicn_src, mask, 1, (w/8)*h) != ((w/8)*h) ) {
		error("GetCIcon(%hd): Corrupt CICN!\n", cicn_id);
		delete[] pixels;
		delete[] mask;
		SDL_RWclose(cicn_src);
		return(NULL);
	}
	SDL_RWclose(cicn_src);

	cicn = screen->LoadImage(w, h, pixels, mask);
	delete[] pixels;
	delete[] mask;
	if ( cicn == NULL ) {
		error("GetCIcon(%hd): Couldn't convert CICN!\n", cicn_id);
	}
	return(cicn);
}
