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

#include "UIBaseElement.h"

class FrameBuf;
class UIManager;
class UIPanel;
class UITemplates;

class UIPanelDelegate
{
public:
	UIPanelDelegate(UIPanel *panel) { m_panel = panel; }

	virtual bool OnLoad() { return true; }
	virtual void OnShow() { }
	virtual void OnHide() { }
	virtual void OnTick() { }
	virtual void OnDraw() { }
	virtual bool HandleEvent(const SDL_Event &event) { return false; }

protected:
	UIPanel *m_panel;
};

class UIPanel : public UIBaseElement
{
DECLARE_TYPESAFE_CLASS(UIBaseElement)
public:
	UIPanel(UIManager *ui, const char *name);
	virtual ~UIPanel();

	bool IsFullscreen() const {
		return m_fullscreen;
	}
	bool IsCursorVisible() const {
		return m_cursorVisible;
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
	virtual bool FinishLoading();

	void SetPanelDelegate(UIPanelDelegate *delegate, bool autodelete = true);

	virtual void Show();
	virtual void Hide();

	void HideAll();

	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	bool m_fullscreen;
	bool m_cursorVisible;
	int m_enterSound;
	int m_leaveSound;
	UIPanelDelegate *m_delegate;
	bool m_deleteDelegate;
};

#endif // _UIPanel_h
