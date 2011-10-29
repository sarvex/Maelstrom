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

#ifndef _UIManager_h
#define _UIManager_h

#include "SDL.h"
#include "../utils/array.h"
#include "UIArea.h"
#include "UIPanel.h"
#include "UIFontInterface.h"
#include "UISoundInterface.h"
#include "UITemplates.h"

class FrameBuf;
class UIBaseElement;
class UIElement;

class UIManager : public UIArea, public UIFontInterface, public UISoundInterface
{
public:
	UIManager(FrameBuf *screen);
	virtual ~UIManager();

	FrameBuf *GetScreen() const {
		return m_screen;
	}
	const UITemplates *GetTemplates() const {
		return &m_templates;
	}

	void SetLoadPath(const char *path);
	bool LoadTemplates(const char *file);
	UIPanel *LoadPanel(const char *name);
	UIPanel *GetPanel(const char *name, bool allowLoad = true);
	template <typename T>
	T *GetPanel(const char *name) {
		UIPanel *panel = GetPanel(name);
		if (panel && panel->IsA(T::GetType())) {
			return (T*)panel;
		}
		return NULL;
	}
	UIPanel *GetCurrentPanel();

	/* These are called by the UIPanel class */
	void AddPanel(UIPanel *panel) {
		if (!m_panels.find(panel)) {
			m_panels.add(panel);
		}
	}
	void RemovePanel(UIPanel *panel) {
		m_visible.remove(panel);
		m_panels.remove(panel);
	}

	void ShowPanel(UIPanel *panel);
	void ShowPanel(const char *name) {
		ShowPanel(GetPanel(name));
	}
	void HidePanel(UIPanel *panel);
	void HidePanel(const char *name) {
		HidePanel(GetPanel(name));
	}
	void DeletePanel(UIPanel *panel);
	void DeletePanel(const char *name) {
		DeletePanel(GetPanel(name));
	}

	void Draw(bool fullUpdate = true);
	bool HandleEvent(const SDL_Event &event);

public:
	/* These should be implemented to load UI from XMl */
	virtual UIPanel *CreatePanel(const char *type, const char *name) {
		return NULL;
	}
	virtual UIPanelDelegate *CreatePanelDelegate(UIPanel *panel, const char *delegate) {
		return NULL;
	}
	virtual UIElement *CreateElement(UIBaseElement *parent, const char *type, const char *name = "") {
		return NULL;
	}

protected:
	char *m_loadPath;
	UITemplates m_templates;
	array<UIPanel *> m_panels;
	array<UIPanel *> m_visible;
};

#endif // _UIManager_h
