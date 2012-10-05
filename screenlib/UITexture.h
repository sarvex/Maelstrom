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

#ifndef _UITexture_h
#define _UITexture_h

struct SDL_Texture;

/* The scale in this class represents the scale of the pixels relative to
   the layout resolution.  For example, if the layout resolution is 1024x768,
   and the image has twice the pixel density in each dimension for resolution
   2048x1536, then the scale would be 2.0.
*/

class UITexture
{
public:
	UITexture(SDL_Texture *texture, float scale = 1.0f);

	SDL_Texture *Texture() const {
		return m_texture;
	}
	int TextureWidth() const {
		return m_textureWidth;
	}
	int TextureHeight() const {
		return m_textureHeight;
	}
	int Width() const {
		return (int)(m_textureWidth / m_scale);
	}
	int Height() const {
		return (int)(m_textureHeight / m_scale);
	}
	float Scale() const {
		return m_scale;
	}
	void SetScale(float scale) {
		m_scale = scale;
	}

protected:
	SDL_Texture *m_texture;
	int m_textureWidth;
	int m_textureHeight;
	float m_scale;
};

#endif // _UITexture_h
