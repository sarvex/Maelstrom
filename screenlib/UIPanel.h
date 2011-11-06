/*
  screenlib:  A simple window and UI library based on the SDL library
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
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

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
	override bool FinishLoading();

	void SetPanelDelegate(UIPanelDelegate *delegate, bool autodelete = true);

	override void Show();
	override void Hide();
	virtual bool ShouldSaveData() { return true; }

	void HideAll();

	virtual void Tick();
	override void Draw();
	override bool HandleEvent(const SDL_Event &event);

protected:
	bool m_fullscreen;
	bool m_cursorVisible;
	int m_enterSound;
	int m_leaveSound;
	UIPanelDelegate *m_delegate;
	bool m_deleteDelegate;
};

#endif // _UIPanel_h
