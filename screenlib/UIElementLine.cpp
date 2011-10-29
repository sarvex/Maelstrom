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
#include "UIElementLine.h"

UIElementType UIElementLine::s_elementType;


UIElementLine::UIElementLine(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
}

bool
UIElementLine::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	LoadColor(node, "color", m_color);

	return true;
}

void
UIElementLine::Draw()
{
	m_screen->DrawLine(X(), Y(), X()+Width()-1, Y()+Height()-1, m_color);
}
