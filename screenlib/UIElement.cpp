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

#include "UIPanel.h"
#include "UIElement.h"

UIElement::UIElement(UIPanel *panel, const char *name) : UIArea()
{
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);

	m_screen = panel->GetScreen();
	m_panel = panel;
}

UIElement::~UIElement()
{
	delete[] m_name;
}

bool
UIElement::Load(rapidxml::xml_node<> *node)
{
	return UIArea::Load(node);
}

UIArea *
UIElement::GetAnchorElement(const char *name)
{
	if (!name) {
		return m_panel;
	}
	return m_panel->GetElement(name);
}
