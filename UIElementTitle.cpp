/*
    Maelstrom: Open Source version of the classic game by Ambrosia Software
    Copyright (C) 1997-2011  Sam Lantinga

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

#include "UIElementTitle.h"
#include "load.h"

UIElementType UIElementTitle::s_elementType;


UIElementTitle::UIElementTitle(UIBaseElement *parent, const char *name) :
	UIElementTexture(parent, name)
{
}

bool
UIElementTitle::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElementTexture::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("id", 0, false);
	if (!attr) {
		SetError("Element '%s' missing attribute 'id'", node->name());
		return false;
	}

	SDL_Texture *texture = Load_Title(m_screen, atoi(attr->value()));
	if (!texture) {
		SetError("Unable to load title %d", atoi(attr->value()));
		return false;
	}
	SetTexture(texture);

	return true;
}
