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

#ifndef _UIElementRect_h
#define _UIElementRect_h

#include "UIElement.h"


class UIElementRect : public UIElement
{
DECLARE_TYPESAFE_CLASS(UIElement)
public:
	UIElementRect(UIBaseElement *parent, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	void SetColor(Uint32 color) {
		m_color = color;
	}

	virtual void Draw();

protected:
	bool m_fill;
	Uint32 m_color;
};

#endif // _UIElementRect_h
