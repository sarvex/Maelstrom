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

UIElementType UIElement::s_elementTypeIndex;
UIElementType UIElement::s_elementType;


UIElement::UIElement(UIPanel *panel, const char *name) : UIArea(panel->GetScreen())
{
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);

	m_panel = panel;
}

UIElement::~UIElement()
{
	delete[] m_name;
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

bool
UIElement::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute("name", 0, false);
	if (attr) {
		delete[] m_name;
		m_name = new char[strlen(attr->value())+1];
		strcpy(m_name, attr->value());
	}

	return UIArea::Load(node);
}

UIArea *
UIElement::GetAnchorElement(const char *name)
{
	if (!name) {
		return m_panel;
	}
	return m_panel->GetElement<UIElement>(name);
}
