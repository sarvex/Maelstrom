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
