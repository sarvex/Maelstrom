/*
    SCREENLIB:  A framebuffer library based on the SDL library
    Copyright (C) 1997  Sam Lantinga

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

#ifndef _SDL_FrameBuf_h
#define _SDL_FrameBuf_h

/* A simple display management class based on SDL:

   It supports line drawing, rectangle filling, and fading,
   and it supports loading 8 bits-per-pixel masked images.
*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "SDL.h"

typedef enum {
	DOCLIP,
	NOCLIP
} clipval;

class FrameBuf {

public:
	FrameBuf();
	int Init(int width, int height, Uint32 window_flags, Uint32 render_flags,
			SDL_Color *colors = NULL, SDL_Surface *icon = NULL);
	~FrameBuf();

	/* Setup routines */
	/* Set the image palette -- 256 entries */
	void SetPalette(SDL_Color *colors);
	/* Map an RGB value to a color pixel */
	Uint32 MapRGB(Uint8 R, Uint8 G, Uint8 B) {
		return (0xFF000000 | ((Uint32)R << 16) | ((Uint32)G << 8) | B);
	}
	/* Set the blit clipping rectangle */
	void   ClipBlit(SDL_Rect *cliprect) {
		clip = *cliprect;
	}

	/* Event Routines */
	int PollEvent(SDL_Event *event) {
		return(SDL_PollEvent(event));
	}
	int WaitEvent(SDL_Event *event) {
		return(SDL_WaitEvent(event));
	}
	void ToggleFullScreen(void) {
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
			SDL_SetWindowFullscreen(window, SDL_FALSE);
		} else {
			SDL_SetWindowFullscreen(window, SDL_TRUE);
		}
	}

	/* Blit and update routines */
	void QueueBlit(int dstx, int dsty, SDL_Texture *src,
			int srcx, int srcy, int w, int h, clipval do_clip);
	void QueueBlit(int x, int y, SDL_Texture *src, clipval do_clip = DOCLIP) {
		int w, h;
		SDL_QueryTexture(src, NULL, NULL, &w, &h);
		QueueBlit(x, y, src, 0, 0, w, h, do_clip);
	}
	void Update() {
		SDL_RenderPresent(renderer);
	}
	void Fade(void);		/* Fade screen out, then in */

	/* Informational routines */
	int Width(void) {
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		return w;
	}
	int Height(void) {
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		return h;
	}

	/* Drawing routines */
	void Clear(int x, int y, int w, int h) {
		FillRect(x, y, w, h, 0);
	}
	void Clear(void) {
		UpdateDrawColor(0);
		SDL_RenderClear(renderer);
	}
	void DrawPoint(int x, int y, Uint32 color) {
		UpdateDrawColor(color);
		SDL_RenderDrawPoint(renderer, x, y);
	}
	void DrawLine(int x1, int y1, int x2, int y2, Uint32 color) {
		UpdateDrawColor(color);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
	void DrawRect(int x1, int y1, int w, int h, Uint32 color) {
		UpdateDrawColor(color);

		SDL_Rect rect;
		rect.x = x1;
		rect.y = y1;
		rect.w = w;
		rect.h = h;
		SDL_RenderDrawRect(renderer, &rect);
	}
	void FillRect(int x1, int y1, int w, int h, Uint32 color) {
		UpdateDrawColor(color);

		SDL_Rect rect;
		rect.x = x1;
		rect.y = y1;
		rect.w = w;
		rect.h = h;
		SDL_RenderFillRect(renderer, &rect);
	}

	/* Load and convert an 8-bit image with the given mask */
	SDL_Texture *LoadImage(int w, int h, Uint8 *pixels, Uint8 *mask = NULL);
	SDL_Texture *LoadImage(int w, int h, Uint32 *pixels);
	int GetImageWidth(SDL_Texture *image) {
		int w, h;
		SDL_QueryTexture(image, NULL, NULL, &w, &h);
		return w;
	}
	int GetImageHeight(SDL_Texture *image) {
		int w, h;
		SDL_QueryTexture(image, NULL, NULL, &w, &h);
		return h;
	}
	void FreeImage(SDL_Texture *image);

	/* Screen dump routines */
	int ScreenDump(const char *prefix, int x, int y, int w, int h);

	/* Cursor handling routines */
	void ShowCursor(void) {
		SDL_ShowCursor(1);
	}
	void HideCursor(void) {
		SDL_ShowCursor(0);
	}
	void SetCaption(const char *caption, const char *icon = NULL) {
		SDL_SetWindowTitle(window, caption);
	}

	/* Error message routine */
	char *Error(void) {
		return(errstr);
	}

private:
	/* The current display */
	SDL_Window *window;
	SDL_Renderer *renderer;
	Uint32 image_map[256];
	int faded;
	SDL_Rect clip;

	void UpdateDrawColor(Uint32 color) {
		Uint8 r, g, b;
		r = (color >> 16) & 0xFF;
		g = (color >>  8) & 0xFF;
		b = (color >>  0) & 0xFF;
		SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
	}

	/* Error message */
	void SetError(const char *fmt, ...) {
		va_list ap;

		va_start(ap, fmt);
		vsprintf(errbuf, fmt, ap);
		va_end(ap);
		errstr = errbuf;
        }
	char *errstr;
	char  errbuf[1024];
};

#endif /* _SDL_FrameBuf_h */
