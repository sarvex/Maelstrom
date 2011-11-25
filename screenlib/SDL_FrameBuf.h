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

#ifndef _SDL_FrameBuf_h
#define _SDL_FrameBuf_h

/* A simple display management class based on SDL:

   It supports line drawing, rectangle filling, and fading,
   and it supports loading 8 bits-per-pixel masked images.
*/

// Define this if you're rapidly iterating on UI screens
#define FAST_ITERATION

// Define these if you want to try out the quick resolution hacks
//#define HACK_1024x768
//#define HACK_400x300

#ifdef HACK_1024x768
#define HACK_RESOLUTION
#define HACK_RESOLUTION_X 1024
#define HACK_RESOLUTION_Y 768
#endif
#ifdef HACK_400x300
#define HACK_RESOLUTION
#define HACK_RESOLUTION_X 400
#define HACK_RESOLUTION_Y 300
#endif
#ifdef HACK_RESOLUTION
#define ORIG_RESOLUTION_X 640
#define ORIG_RESOLUTION_Y 480
#endif

#include <stdio.h>

#include "SDL.h"
#include "ErrorBase.h"

typedef enum {
	DOCLIP,
	NOCLIP
} clipval;

class FrameBuf : public ErrorBase {

public:
	FrameBuf();
	int Init(int width, int height, Uint32 window_flags, Uint32 render_flags,
			SDL_Color *colors = NULL, SDL_Surface *icon = NULL);
	virtual ~FrameBuf();

	int AdjustCoordinateX(int &x, bool invert = false) const {
#ifdef HACK_RESOLUTION
		if (invert) {
			x = (x*ORIG_RESOLUTION_X)/HACK_RESOLUTION_X;
		} else {
			x = (x*HACK_RESOLUTION_X)/ORIG_RESOLUTION_X;
		}
#endif
		return x;
	}
	int AdjustCoordinateY(int &y, bool invert = false) const {
#ifdef HACK_RESOLUTION
		if (invert) {
			y = (y*ORIG_RESOLUTION_Y)/HACK_RESOLUTION_Y;
		} else {
			y = (y*HACK_RESOLUTION_Y)/ORIG_RESOLUTION_Y;
		}
#endif
		return y;
	}
	void AdjustCoordinates(int &x, int &y, bool invert = false) const {
		AdjustCoordinateX(x, invert);
		AdjustCoordinateY(y, invert);
	}

	/* Setup routines */
	/* Set the image palette -- 256 entries */
	void SetPalette(SDL_Color *colors);
	/* Map an RGB value to a color pixel */
	Uint32 MapRGB(Uint8 R, Uint8 G, Uint8 B) {
		return (0xFF000000 | ((Uint32)R << 16) | ((Uint32)G << 8) | B);
	}
	void GetRGB(Uint32 color, Uint8 *R, Uint8 *G, Uint8 *B) {
		*R = (Uint8)((color >> 16) & 0xFF);
		*G = (Uint8)((color >>  8) & 0xFF);
		*B = (Uint8)((color >>  0) & 0xFF);
	}
	/* Set the blit clipping rectangle */
	void   ClipBlit(SDL_Rect *cliprect) {
		clip = *cliprect;
	}

	/* Event Routines */
	int PollEvent(SDL_Event *event) {
		int result = SDL_PollEvent(event);
		if (result > 0) {
			ProcessEvent(event);
		}
		return result;
	}
	int WaitEvent(SDL_Event *event) {
		int result = SDL_WaitEvent(event);
		if (result > 0) {
			ProcessEvent(event);
		}
		return result;
	}
	void ProcessEvent(SDL_Event *event) {
		switch (event->type) {
			case SDL_MOUSEMOTION:
				event->motion.x -= rect.x;
				event->motion.y -= rect.y;
				AdjustCoordinates(event->motion.x,
						  event->motion.y, true);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				event->button.x -= rect.x;
				event->button.y -= rect.y;
				AdjustCoordinates(event->button.x,
						  event->button.y, true);
				break;
		}
	}
	bool ConvertTouchCoordinates(const SDL_TouchFingerEvent &finger, int *x, int *y);

	void EnableTextInput();
	void DisableTextInput();

	void ToggleFullScreen(void) {
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) {
			SDL_SetWindowFullscreen(window, SDL_FALSE);
		} else {
			SDL_SetWindowFullscreen(window, SDL_TRUE);
		}
	}

	/* Information routines */
	int Width() const {
		int w = rect.w;
		return AdjustCoordinateX(w, true);
	}
	int Height() const {
		int h = rect.h;
		return AdjustCoordinateY(h, true);
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
	void FadeOut(void) {
		if (!faded) {
			Fade();
		}
	}
	void FadeIn(void) {
		if (faded) {
			Fade();
		}
	}
	void Fade(void);		/* Fade screen out, then in */

	/* Drawing routines */
	void Clear(int x, int y, int w, int h) {
		AdjustCoordinates(x, y);
		AdjustCoordinates(w, h);

		FillRect(x, y, w, h, 0);
	}
	void Clear(void) {
		UpdateDrawColor(0);
		SDL_RenderClear(renderer);
	}
	void DrawPoint(int x, int y, Uint32 color) {
		AdjustCoordinates(x, y);

		UpdateDrawColor(color);
		SDL_RenderDrawPoint(renderer, x, y);
	}
	void DrawLine(int x1, int y1, int x2, int y2, Uint32 color) {
		AdjustCoordinates(x1, y1);
		AdjustCoordinates(x2, y2);

		UpdateDrawColor(color);
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}
	void DrawRect(int x1, int y1, int w, int h, Uint32 color) {
		UpdateDrawColor(color);

		AdjustCoordinates(x1, y1);
		AdjustCoordinates(w, h);

		SDL_Rect rect;
		rect.x = x1;
		rect.y = y1;
		rect.w = w;
		rect.h = h;
		SDL_RenderDrawRect(renderer, &rect);
	}
	void FillRect(int x1, int y1, int w, int h, Uint32 color) {
		UpdateDrawColor(color);

		AdjustCoordinates(x1, y1);
		AdjustCoordinates(w, h);

		SDL_Rect rect;
		rect.x = x1;
		rect.y = y1;
		rect.w = w;
		rect.h = h;
		SDL_RenderFillRect(renderer, &rect);
	}

	/* Load a texture image */
	SDL_Texture *LoadImage(const char *file);
	SDL_Texture *LoadImage(SDL_Surface *surface);
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

private:
	/* The current display */
	SDL_Window *window;
	SDL_Renderer *renderer;
	Uint32 image_map[256];
	int faded;
	SDL_Rect rect;
	SDL_Rect clip;

	void UpdateDrawColor(Uint32 color) {
		Uint8 r, g, b;
		r = (color >> 16) & 0xFF;
		g = (color >>  8) & 0xFF;
		b = (color >>  0) & 0xFF;
		SDL_SetRenderDrawColor(renderer, r, g, b, 0xFF);
	}
};

#endif /* _SDL_FrameBuf_h */
