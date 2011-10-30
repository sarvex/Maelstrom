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

#ifndef _UIElementTexture_h
#define _UIElementTexture_h

#include "UIElement.h"


class UIElementTexture : public UIElement
{
DECLARE_TYPESAFE_CLASS(UIElement)
public:
	UIElementTexture(UIBaseElement *parent, const char *name = "");
	virtual ~UIElementTexture();

	void SetTexture(SDL_Texture *texture);

	virtual void Draw();

private:
	SDL_Texture *m_texture;
};

#endif // _UIElementTexture_h
