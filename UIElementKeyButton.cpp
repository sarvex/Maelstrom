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

#include "UIElementKeyButton.h"
#include "screenlib/UIElementLabel.h"

UIElementType UIElementKeyButton::s_elementType;

UIElementKeyButton::UIElementKeyButton(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
{
}

bool
UIElementKeyButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	if (m_hotkey != SDLK_UNKNOWN) {
		UIElementLabel *label;
		const char *text;

		text = SDL_GetKeyName(m_hotkey);
		label = GetElement<UIElementLabel>("label");
		if (label) {
			label->SetText(text);
		}
		label = GetElement<UIElementLabel>("shadow");
		if (label) {
			label->SetText(text);
		}
	}
	return true;
}
