/*
  screenlib:  A simple window and UI library based on the SDL library
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include "../utils/physfsrwops.h"

#include "SDL_FrameBuf.h"


#define LOWER_PREC(X)	((X)/16)	/* Lower the precision of a value */
#define RAISE_PREC(X)	((X)/16)	/* Raise the precision of a value */

#define MIN(A, B)	((A < B) ? A : B)
#define MAX(A, B)	((A > B) ? A : B)

/* Constructors cannot fail. :-/ */
FrameBuf:: FrameBuf() : ErrorBase()
{
	/* Initialize various variables to null state */
	window = NULL;
	renderer = NULL;
	faded = 0;
}

int
FrameBuf:: Init(int width, int height, Uint32 window_flags, Uint32 render_flags,
		SDL_Color *colors, SDL_Surface *icon)
{
	int w, h;

#ifdef HACK_RESOLUTION
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
#endif
	AdjustCoordinates(width, height);

	window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, window_flags);
	if (!window) {
		SetError("Couldn't create %dx%d window: %s", 
					width, height, SDL_GetError());
		return(-1);
	}

	renderer = SDL_CreateRenderer(window, -1, render_flags);
	if (!renderer) {
		SetError("Couldn't create renderer: %s", SDL_GetError());
		return(-1);
	}

	/* Set the icon, if any */
	if ( icon ) {
		SDL_SetWindowIcon(window, icon);
	}

	/* Set the UI area */
	SDL_GetWindowSize(window, &w, &h);
	rect.x = (w - width) / 2;
	rect.y = (h - height) / 2;
	rect.w = width;
	rect.h = height;
	SDL_RenderSetViewport(renderer, &rect);

	/* Set the blit clipping rectangle */
	clip.x = 0;
	clip.y = 0;
	clip.w = AdjustCoordinateX(width, true);
	clip.h = AdjustCoordinateY(height, true);

	/* Copy the image colormap */
	if ( colors ) {
		SetPalette(colors);
	}

	return(0);
}

FrameBuf:: ~FrameBuf()
{
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (window) {
		SDL_DestroyWindow(window);
	}
}

void
FrameBuf:: SetPalette(SDL_Color *colors)
{
	int i;

	for ( i=0; i<256; ++i ) {
		image_map[i] = MapRGB(colors[i].r, colors[i].g, colors[i].b);
	}
}

// This routine or something like it should probably go in SDL
bool
FrameBuf::ConvertTouchCoordinates(const SDL_TouchFingerEvent &finger, int *x, int *y)
{
	int w, h;
	SDL_Touch* inTouch = SDL_GetTouch(finger.touchId);
	if (inTouch == NULL) {
		return false;
	}

	SDL_GetWindowSize(window, &w, &h);
	*x = (int)((((float)finger.x)/inTouch->xres)*w) - rect.x;
	*y = (int)((((float)finger.y)/inTouch->yres)*h) - rect.y;
	AdjustCoordinates(*x, *y, true);
	return true;
}

#ifdef __IPHONEOS__
extern "C" {
	extern int SDL_iPhoneKeyboardHide(SDL_Window * window);
	extern int SDL_iPhoneKeyboardShow(SDL_Window * window);
}
#endif

void
FrameBuf::EnableTextInput()
{
	SDL_StartTextInput();

#ifdef __IPHONEOS__
	SDL_iPhoneKeyboardShow(window);
#endif
}

void
FrameBuf::DisableTextInput()
{
#ifdef __IPHONEOS__
	SDL_iPhoneKeyboardHide(window);
#endif

	SDL_StopTextInput();
}

void
FrameBuf:: QueueBlit(int dstx, int dsty, SDL_Texture *src,
			int srcx, int srcy, int w, int h, clipval do_clip)
{
	SDL_Rect srcrect;
	SDL_Rect dstrect;

	srcrect.x = srcx;
	srcrect.y = srcy;
	srcrect.w = w;
	srcrect.h = h;
	dstrect.x = dstx;
	dstrect.y = dsty;
	dstrect.w = w;
	dstrect.h = h;
	if (do_clip == DOCLIP) {
		if (!SDL_IntersectRect(&clip, &dstrect, &dstrect)) {
			return;
		}

		/* Adjust the source rectangle to match */
		srcrect.x += (dstrect.x - dstx);
		srcrect.y += (dstrect.y - dsty);
		srcrect.w = dstrect.w;
		srcrect.h = dstrect.h;
	}
	AdjustCoordinates(dstrect.x, dstrect.y);
	AdjustCoordinates(dstrect.w, dstrect.h);
	SDL_RenderCopy(renderer, src, &srcrect, &dstrect);
}

void
FrameBuf:: Fade(void)
{
#ifdef FAST_ITERATION
	return;
#else
	const int max = 32;
	Uint16 ramp[256];   

	for ( int j = 1; j <= max; j++ ) {
		int v = faded ? j : max - j + 1;
		for ( int i = 0; i < 256; i++ ) {
			ramp[i] = (i * v / max) << 8;
		}
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
		SDL_Delay(10);
	}
	faded = !faded;

        if ( faded ) {
		for ( int i = 0; i < 256; i++ ) {
			ramp[i] = 0;
		}
		SDL_SetWindowGammaRamp(window, ramp, ramp, ramp);
	}
#endif
} 

int
FrameBuf:: ScreenDump(const char *prefix, int x, int y, int w, int h)
{
	SDL_Rect rect;
	SDL_Surface *dump;
	int which, found;
	char file[1024];
	int retval;

	/* Create a BMP format surface */
	dump = SDL_CreateRGBSurface(0, w, h, 24, 
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                   0x00FF0000, 0x0000FF00, 0x000000FF, 0);
#else
                   0x000000FF, 0x0000FF00, 0x00FF0000, 0);
#endif
	if (!dump) {
		SetError("%s", SDL_GetError());
		return -1;
	}

	/* Read the screen into it */
	if (!w) {
		w = Width();
	}
	if (!h) {
		h = Height();
	}
	AdjustCoordinates(x, y);
	AdjustCoordinates(w, h);
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	if (SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_BGR24, dump->pixels, dump->pitch) < 0) {
		SetError("%s", SDL_GetError());
		return -1;
	}

	/* Get a suitable new filename */
	found = 0;
	for ( which=0; !found; ++which ) {
		sprintf(file, "%s%d.bmp", prefix, which);
		if (!PHYSFS_exists(file)) {
			found = 1;
		}
	}
	retval = SDL_SaveBMP_RW(dump, PHYSFSRWOPS_openWrite(file), 1);
	if ( retval < 0 ) {
		SetError("%s", SDL_GetError());
	}
	SDL_FreeSurface(dump);

	return(retval);
}

SDL_Texture *
FrameBuf:: LoadImage(int w, int h, Uint8 *pixels, Uint8 *mask)
{
	SDL_Texture *texture;
	Uint32 *artwork;
	int i, j, pad;
	Uint8 *pix_mem;
	Uint32 *art_mem;

	/* Assume 8-bit artwork using the current palette */
	artwork = new Uint32[w*h];

	pad  = ((w%4) ? (4-(w%4)) : 0);
	if ( mask ) {
		Uint8 m;

		/* Copy over the pixels */
		pix_mem = pixels;
		art_mem = artwork;
		for ( i=0; i<h; ++i ) {
			for ( j=0; j<w; ++j ) {
				if ( (j%8) == 0 ) {
					m = *mask++;
				}
				if ( m & 0x80 ) {
					*art_mem++ = image_map[*pix_mem];
				} else {
					*art_mem++ = 0;
				}
				m <<= 1;
				pix_mem += 1;
			}
			pix_mem += pad;
		}
	} else {
		/* Copy over the pixels */
		pix_mem = pixels;
		art_mem = artwork;
		for ( i=0; i<h; ++i ) {
			for ( j=0; j<w; ++j ) {
				*art_mem++ = image_map[*pix_mem];
				pix_mem += 1;
			}
			pix_mem += pad;
		}
	}

	texture = LoadImage(w, h, artwork);
	if ( mask ) {
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	}
	delete[] artwork;
	return(texture);
}

SDL_Texture *
FrameBuf:: LoadImage(int w, int h, Uint32 *pixels)
{
	SDL_Texture *texture;

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	if (!texture) {
		SetError("%s", SDL_GetError());
		return NULL;
	}

	if (SDL_UpdateTexture(texture, NULL, pixels, w*sizeof(Uint32)) < 0) {
		SetError("%s", SDL_GetError());
		SDL_DestroyTexture(texture);
		return NULL;
	}
	return(texture);
}

SDL_Texture *
FrameBuf:: LoadImage(SDL_Surface *surface)
{
	return SDL_CreateTextureFromSurface(renderer, surface);
}

void
FrameBuf:: FreeImage(SDL_Texture *image)
{
	SDL_DestroyTexture(image);
}
