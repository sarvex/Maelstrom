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

#ifndef _UIElementEditbox_h
#define _UIElementEditbox_h

// This is a simple editbox class
// It currently doesn't support multiline or editing within the line,
// though the latter could be supported fairly easily.

#include "UIElementButton.h"


class UIElementEditbox : public UIElementButton
{
DECLARE_TYPESAFE_CLASS(UIElement)
public:
	UIElementEditbox(UIBaseElement *parent, const char *name = "");
	virtual ~UIElementEditbox();

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
	override bool FinishLoading();

	override bool HandleEvent(const SDL_Event &event);

	override void OnClick() {
		SetFocus(true);
	}

	void SetFocus(bool focus);
	void SetFocusNext();

	void SetTextMax(int maxLen);

	override void SetText(const char *text);
	const char *GetText() const {
		return m_text;
	}

	void SetNumber(int value) {
		char buffer[32];
		sprintf(buffer, "%d", value);
		SetText(buffer);
	}
	int GetNumber() const {
		return SDL_atoi(m_text);
	}

protected:
	// These can be overridden by inheriting classes
	virtual void OnHighlightChanged() { }
	virtual void OnTextChanged();

	void SetHighlight(bool highlight);

protected:
	bool m_focus;
	bool m_highlight;
	bool m_numeric;
	int m_textMax;
	int m_textLen;
	char *m_text;
};

#endif // _UIElementEditbox_h
