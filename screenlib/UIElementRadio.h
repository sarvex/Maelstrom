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

#ifndef _UIElementRadio_h
#define _UIElementRadio_h

#include "UIElement.h"
#include "UIElementCheckbox.h"

//
// This file has two classes:
//
// UIElementRadioGroup maintains the state of radio buttons
//
// UIElementRadioButton is a member of the radio group and notifies the group
// when it is clicked.

class UIElementRadioButton;

class UIElementRadioGroup : public UIElement
{
public:
	UIElementRadioGroup(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	void RadioButtonChecked(UIElementRadioButton *button);

	int GetValue() const {
		return m_value;
	}

protected:
	int m_value;

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

class UIElementRadioButton : public UIElementCheckbox
{
public:
	UIElementRadioButton(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElementCheckbox::IsA(type) || type == GetType();
	}

	int GetID() const {
		return m_id;
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void OnChecked(bool checked);

protected:
	int m_id;

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

#endif // _UIElementRadio_h
