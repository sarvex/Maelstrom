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

#ifndef _UIBaseElement_h
#define _UIBaseElement_h

#include "../utils/array.h"
#include "../utils/rapidxml.h"

#include "SDL.h"
#include "UIArea.h"

class FrameBuf;
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

	FrameBuf *GetScreen() const {
		return m_screen;
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
	template <typename T>
	T *FindElement(UIBaseElement *start = NULL) {
		unsigned i, j;
		if (start) {
			// Find the starting element
			for (i = 0; i < m_elements.length(); ++i) {
				if (m_elements[i] == start) {
					break;
				}
			}
			if (i == m_elements.length()) {
				return NULL;
			}
			// Find the next element of that type
			j = (i+1)%m_elements.length();
			for ( ; j != i; j = (j+1)%m_elements.length()) {
				UIBaseElement *element = m_elements[j];
				if (element->IsA(T::GetType())) {
					return (T*)element;
				}
			}
		} else {
			for (i = 0; i < m_elements.length(); ++i) {
				UIBaseElement *element = m_elements[i];
				if (element->IsA(T::GetType())) {
					return (T*)element;
				}
			}
		}
		return NULL;
	}
	template <typename T>
	void FindElements(array<T*> &elements) {
		for (unsigned i = 0; i < m_elements.length(); ++i) {
			UIBaseElement *element = m_elements[i];
			if (element->IsA(T::GetType())) {
				elements.add((T*)element);
			}
		}
	}
	void RemoveElement(UIBaseElement *element) {
		m_elements.remove(element);
	}

	virtual void Show() {
		m_shown = true;
	}
	virtual void Hide() {
		m_shown = false;
	}
	bool IsShown() const {
		return m_shown;
	}

	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	FrameBuf *m_screen;
	UIManager *m_ui;
	UIBaseElement *m_parent;
	char *m_name;
	bool m_shown;
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

/////////////////////////////////////////////////////////////////////////
#define DECLARE_TYPESAFE_CLASS(BASECLASS)				\
protected:								\
	static UIElementType s_elementType;				\
									\
public:									\
	static UIElementType GetType() {				\
		if (!s_elementType) {					\
			s_elementType = GenerateType();			\
		}							\
		return s_elementType;					\
	}								\
	virtual bool IsA(UIElementType type) {				\
		return BASECLASS::IsA(type) || type == GetType();	\
	}
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// So we can be explicit about what methods are being newly exposed and
// which methods are overriding a base class method, I'm going to introduce
// the "override" keyword in this inheritance hierarchy.
#define override virtual
/////////////////////////////////////////////////////////////////////////

#endif // _UIBaseElement_h
