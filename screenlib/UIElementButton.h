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

#ifndef _UIElementButton_h
#define _UIElementButton_h

#include "UIElement.h"

class UIElementLabel;

class UIButtonDelegate
{
public:
	virtual void OnClick() { }
};

class UIElementButton : public UIElement
{
DECLARE_TYPESAFE_CLASS(UIElement)
public:
	UIElementButton(UIBaseElement *parent, const char *name = "");
	virtual ~UIElementButton();

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	override bool HandleEvent(const SDL_Event &event);

	virtual void SetText(const char *text);

	// Setting a click callback sets a simplified delegate
	void SetClickCallback(void (*callback)(void));
	void SetButtonDelegate(UIButtonDelegate *delegate, bool autodelete = true);

protected:
	// These can be overridden by inheriting classes
	virtual void OnMouseEnter() { }
	virtual void OnMouseLeave() { }
	virtual void OnMouseDown() { }
	virtual void OnMouseUp() { }
	virtual void OnClick();
	virtual UIElementLabel *CreateLabel();

	bool ShouldHandleKey(SDL_Keycode key);

protected:
	SDL_Keycode m_hotkey;
	int m_hotkeyMod;
	bool m_mouseInside;
	bool m_mousePressed;
	int m_clickSound;
	char *m_clickPanel;
	UIElementLabel *m_label;
	UIButtonDelegate *m_delegate;
	bool m_deleteDelegate;
};

#endif // _UIElementButton_h
