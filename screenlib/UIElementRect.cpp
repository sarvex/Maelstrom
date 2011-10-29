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
#include "UIElementRect.h"

UIElementType UIElementRect::s_elementType;


UIElementRect::UIElementRect(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_fill = false;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
}

bool
UIElementRect::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	LoadBool(node, "fill", m_fill);
	LoadColor(node, "color", m_color);

	return true;
}

void
UIElementRect::Draw()
{
	if (m_fill) {
		m_screen->FillRect(X(), Y(), Width(), Height(), m_color);
	} else {
		m_screen->DrawRect(X(), Y(), Width(), Height(), m_color);
	}
}
