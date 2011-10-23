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

#ifndef _UIPanel_h
#define _UIPanel_h

#include "SDL.h"

#include "../utils/rapidxml.h"
#include "../utils/array.h"

#include "UIArea.h"

class FrameBuf;
class UIManager;
class UIPanel;
class UIElement;

class UIPanel : public UIArea
{
public:
	UIPanel(UIManager *ui, const char *name);
	virtual ~UIPanel();

	FrameBuf *GetScreen() const {
		return m_screen;
	}
	const char *GetName() const {
		return m_name;
	}

	bool Load(const char *file);

	virtual UIArea *GetAnchorElement(const char *name);

	void AddElement(UIElement *element) {
		m_elements.add(element);
	}
	UIElement *GetElement(const char *name);
	void RemoveElement(UIElement *element) {
		m_elements.remove(element);
	}

	void Draw();
	bool HandleEvent(const SDL_Event &event);

protected:
	UIManager *m_ui;
	FrameBuf *m_screen;
	char *m_name;
	array<UIElement *> m_elements;

	bool LoadElements(rapidxml::xml_node<> *node);
};

#endif // _UIPanel_h
