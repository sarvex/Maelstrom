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
	texture = NULL;
	faded = 0;
}

int
FrameBuf:: Init(int width, int height, Uint32 window_flags, Uint32 render_flags,
		SDL_Color *colors, SDL_Surface *icon)
{
#ifdef FAST_ITERATION
	window_flags &= ~SDL_WINDOW_FULLSCREEN;
#endif
#ifdef __IPHONEOS__
	window_flags |= SDL_WINDOW_FULLSCREEN;
#endif
	window_flags |= SDL_WINDOW_RESIZABLE;

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

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

/*
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, width, height);
	if (!texture) {
		SetError("Couldn't create target texture: %s", SDL_GetError());
		return(-1);
	}

	if (SDL_SetRenderTarget(renderer, texture) < 0) {
		SetError("Couldn't set render target: %s", SDL_GetError());
		return(-1);
	}
*/

	/* Set the icon, if any */
	if ( icon ) {
		SDL_SetWindowIcon(window, icon);
	}

	/* Set the output area */
	if ( window_flags & SDL_WINDOW_RESIZABLE ) {
		int w, h;

		SDL_GetWindowSize(window, &w, &h);
		UpdateWindowSize(w, h);
	} else {
		UpdateWindowSize(width, height);
	}

	/* Copy the image colormap */
	if ( colors ) {
		SetPalette(colors);
	}

	return(0);
}

FrameBuf:: ~FrameBuf()
{
	if (texture) {
		SDL_DestroyTexture(texture);
	}
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

void
FrameBuf::ProcessEvent(SDL_Event *event)
{
	switch (event->type) {
	case SDL_WINDOWEVENT:
		if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
			SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
			int w, h;

			SDL_GetWindowSize(window, &w, &h);
			UpdateWindowSize(w, h);
		}
		break;
	case SDL_MOUSEMOTION:
		event->motion.x -= output.x;
		event->motion.y -= output.y;
		event->motion.x = (event->motion.x * rect.w) / output.w;
		event->motion.y = (event->motion.y * rect.h) / output.h;
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		event->button.x -= output.x;
		event->button.y -= output.y;
		event->button.x = (event->button.x * rect.w) / output.w;
		event->button.y = (event->button.y * rect.h) / output.h;
		break;
	}
}

// This routine or something like it should probably go in SDL
bool
FrameBuf::ConvertTouchCoordinates(const SDL_TouchFingerEvent &finger, int *x, int *y)
{
	SDL_Touch* inTouch = SDL_GetTouch(finger.touchId);
	if (inTouch == NULL) {
		return false;
	}

	*x = (int)((((float)finger.x)/inTouch->xres)*output.w) - output.x;
	*y = (int)((((float)finger.y)/inTouch->yres)*output.h) - output.y;
	*x = (*x * rect.w) / output.w;
	*y = (*y * rect.h) / output.h;
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
	SDL_RenderCopy(renderer, src, &srcrect, &dstrect);
}

void
FrameBuf:: StretchBlit(const SDL_Rect *dstrect, SDL_Texture *src, const SDL_Rect *srcrect)
{
	SDL_RenderCopy(renderer, src, srcrect, dstrect);
}

void
FrameBuf:: Update(void)
{
	/* Copy from our render texture to the screen and show it! */
	if (texture) {
		SDL_SetRenderTarget(renderer, NULL);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, &output);
	}
	SDL_RenderPresent(renderer);

	if (texture) {
		SDL_SetRenderTarget(renderer, texture);
	}
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
		SDL_snprintf(file, sizeof(file), "%s%d.bmp", prefix, which);
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
FrameBuf:: LoadImage(const char *file)
{
	SDL_Surface *surface;
	SDL_Texture *texture;
	
	texture = NULL;
	surface = SDL_LoadBMP_RW(PHYSFSRWOPS_openRead(file), 1);
	if (surface) {
		texture = LoadImage(surface);
		SDL_FreeSurface(surface);
	}
	return texture;
}

SDL_Texture *
FrameBuf:: LoadImage(SDL_Surface *surface)
{
	return SDL_CreateTextureFromSurface(renderer, surface);
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

void
FrameBuf:: FreeImage(SDL_Texture *image)
{
	SDL_DestroyTexture(image);
}

SDL_Texture *
FrameBuf:: CreateRenderTarget(int w, int h)
{
	SDL_Texture *texture;

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w, h);
	if (!texture) {
		SetError("Couldn't create target texture: %s", SDL_GetError());
		return NULL;
	}
	return texture;
}

int
FrameBuf:: SetRenderTarget(SDL_Texture *texture)
{
	if (SDL_SetRenderTarget(renderer, texture) < 0) {
		SetError("Couldn't set render target: %s", SDL_GetError());
		return(-1);
	}
	return 0;
}

void
FrameBuf:: FreeRenderTarget(SDL_Texture *texture)
{
	SDL_DestroyTexture(texture);
}
