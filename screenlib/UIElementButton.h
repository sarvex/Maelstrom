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

#ifndef _UIElementButton_h
#define _UIElementButton_h

#include "UIElement.h"

class UIButtonCallback
{
public:
	virtual void OnClick() = 0;
};

class UIElementButton : public UIElement
{
public:
	UIElementButton(UIPanel *panel, const char *name = "");
	virtual ~UIElementButton();

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual bool HandleEvent(const SDL_Event &event);

	/* These should be overridden by inherited classes */
	virtual void OnMouseEnter() { }
	virtual void OnMouseLeave() { }
	virtual void OnMouseDown() { }
	virtual void OnMouseUp() { }
	virtual void OnClick();

	/* This class owns this callback object and will delete it */
	void SetClickCallback(void (*callback)(void));
	void SetClickCallback(UIButtonCallback *callback);

protected:
	SDL_Keycode m_hotkey;
	int m_hotkeyMod;
	bool m_mouseInside;
	bool m_mousePressed;
	int m_clickSound;
	char *m_clickPanel;
	UIButtonCallback *m_callback;

protected:
	bool ShouldHandleKey(SDL_Keycode key);

protected:
	static UIElementType s_elementType;

public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _UIElementButton_h
