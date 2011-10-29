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

#ifndef _UIBaseElement_h
#define _UIBaseElement_h

#include "../utils/array.h"
#include "../utils/rapidxml.h"

#include "SDL.h"
#include "UIArea.h"

class UIManager;
class UITemplates;

typedef int UIElementType;

class UIBaseElement : public UIArea
{
public:
	UIBaseElement(UIManager *ui, const char *name = "");
	UIBaseElement(UIBaseElement *parent, const char *name = "");
	virtual ~UIBaseElement();

	/* This is used for type-safe casting */
	virtual bool IsA(UIElementType type) {
		return type == GetType();
	}

	UIManager *GetUI() {
		return m_ui;
	}
	UIBaseElement *GetParent() {
		return m_parent;
	}
	const char *GetName() const {
		return m_name;
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
	virtual bool FinishLoading() {
		return true;
	}

	virtual UIArea *GetAnchorElement(const char *name);

	void AddElement(UIBaseElement *element) {
		m_elements.add(element);
	}
	template <typename T>
	T *GetElement(const char *name) {
		UIBaseElement *element = GetElement(name);
		if (element && element->IsA(T::GetType())) {
			return (T*)element;
		}
		return NULL;
	}
	void RemoveElement(UIBaseElement *element) {
		m_elements.remove(element);
	}

	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	UIManager *m_ui;
	UIBaseElement *m_parent;
	char *m_name;
	array<UIBaseElement *> m_elements;

protected:
	UIBaseElement *GetElement(const char *name);
	UIBaseElement *CreateElement(const char *type);

	bool LoadElements(rapidxml::xml_node<> *node, const UITemplates *templates);

protected:
	static UIElementType s_elementTypeIndex;
	static UIElementType s_elementType;

	static UIElementType GenerateType() {
		return ++s_elementTypeIndex;
	}
public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _UIBaseElement_h
