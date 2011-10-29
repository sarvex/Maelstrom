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

	LoadString(node, "name", m_name);

	return true;
}

bool
UIElement::LoadBool(rapidxml::xml_node<> *node, const char *name, bool &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		const char *text = attr->value();
		if (*text == '\0' || *text == '0' ||
		    *text == 'f' || *text == 'F') {
			value = false;
		} else {
			value = true;
		}
	}
}

bool
UIElement::LoadNumber(rapidxml::xml_node<> *node, const char *name, int &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		value = (int)strtol(attr->value(), NULL, 0);
	}
}

bool
UIElement::LoadString(rapidxml::xml_node<> *node, const char *name, char *&value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		if (value) {
			SDL_free(value);
		}
		value = SDL_strdup(attr->value());
	}
}

bool
UIElement::LoadColor(rapidxml::xml_node<> *node, const char *name, Uint32 &value)
{
	rapidxml::xml_node<> *child;

	child = node->first_node("color", 0, false);
	if (child) {
		rapidxml::xml_attribute<> *attr;
		int r = 0xFF, g = 0xFF, b = 0xFF;

		LoadNumber(child, "r", r);
		LoadNumber(child, "g", g);
		LoadNumber(child, "b", b);
		value = m_screen->MapRGB(r, g, b);
	}
}
