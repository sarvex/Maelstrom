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

#ifndef _UIElement_h
#define _UIElement_h

#include "SDL.h"

#include "../utils/rapidxml.h"

#include "UIArea.h"

class FrameBuf;
class UIPanel;

class UIElement : public UIArea
{
public:
	UIElement(UIPanel *panel, const char *name = "");
	virtual ~UIElement();

	const char *GetName() const {
		return m_name;
	}

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual UIArea *GetAnchorElement(const char *name);

	virtual void SetText(const char *text) { }

	virtual void Draw() { }
	virtual bool HandleEvent(const SDL_Event &event) { return false; }

protected:
	char *m_name;
	UIPanel *m_panel;
};

#endif // _UIElement_h
