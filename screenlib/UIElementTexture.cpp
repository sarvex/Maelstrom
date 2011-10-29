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

#include "SDL_FrameBuf.h"
#include "UIElementTexture.h"

UIElementType UIElementTexture::s_elementType;


UIElementTexture::UIElementTexture(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_texture = NULL;
}

UIElementTexture::~UIElementTexture()
{
	if (m_texture) {
		m_screen->FreeImage(m_texture);
	}
}

void
UIElementTexture::SetTexture(SDL_Texture *texture)
{
	if (m_texture) {
		m_screen->FreeImage(m_texture);
	}
	m_texture = texture;
	SetSize(m_screen->GetImageWidth(texture),
		m_screen->GetImageHeight(texture));
}

void
UIElementTexture::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(X(), Y(), m_texture, NOCLIP);
	}
}
