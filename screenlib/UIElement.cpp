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
#include "UIPanel.h"
#include "UIElement.h"

UIElementType UIElement::s_elementType;


UIElement::UIElement(UIBaseElement *parent, const char *name) :
	UIBaseElement(parent, name)
{
}

bool
UIElement::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIBaseElement::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("name", 0, false);
	if (attr) {
		SDL_free(m_name);
		m_name = SDL_strdup(attr->value());
	}

	return true;
}

Uint32
UIElement::LoadColor(rapidxml::xml_node<> *node) const
{
	Uint8 r, g, b;
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute("r", 0, false);
	if (attr) {
		r = (Uint8)strtol(attr->value(), NULL, 0);
	}
	attr = node->first_attribute("g", 0, false);
	if (attr) {
		g = (Uint8)strtol(attr->value(), NULL, 0);
	}
	attr = node->first_attribute("b", 0, false);
	if (attr) {
		b = (Uint8)strtol(attr->value(), NULL, 0);
	}
	return m_screen->MapRGB(r, g, b);
}
