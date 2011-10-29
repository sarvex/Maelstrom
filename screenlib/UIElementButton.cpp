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

#include "UIManager.h"
#include "UIPanel.h"
#include "UIElementButton.h"


class SimpleButtonDelegate : public UIButtonDelegate
{
public:
	SimpleButtonDelegate(void (*callback)(void)) {
		m_callback = callback;
	}

	virtual void OnClick() {
		m_callback();
	}

protected:
	void (*m_callback)(void);
};

UIElementType UIElementButton::s_elementType;


UIElementButton::UIElementButton(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_hotkey = SDLK_UNKNOWN;
	m_hotkeyMod = KMOD_NONE;
	m_mouseInside = false;
	m_mousePressed = false;
	m_clickSound = 0;
	m_clickPanel = NULL;
	m_delegate = NULL;
	m_deleteDelegate = false;
}

UIElementButton::~UIElementButton()
{
	SetButtonDelegate(NULL);

	if (m_clickPanel) {
		SDL_free(m_clickPanel);
	}
}

bool
UIElementButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("hotkey", 0, false);
	if (attr) {
		const char *value = attr->value();
		const char *hyphen = SDL_strchr(value, '-');

		if (hyphen) {
			size_t len = size_t(ptrdiff_t(hyphen-value));
			if (SDL_strncasecmp(value, "ALT", len) == 0) {
				m_hotkeyMod = KMOD_ALT;
			} else if (SDL_strncasecmp(value, "CTRL", len) == 0 ||
			           SDL_strncasecmp(value, "Control", len) == 0) {
				m_hotkeyMod = KMOD_CTRL;
			} else if (SDL_strncasecmp(value, "SHIFT", len) == 0) {
				m_hotkeyMod = KMOD_SHIFT;
			} else {
				SetError("Couldn't interpret hotkey value '%s'", value);
				return false;
			}
			value = hyphen+1;
		}

		if (strcmp(value, "any") == 0) {
			/* This will be a catch-all button */
			m_hotkey = ~0;
		} else {
			m_hotkey = SDL_GetKeyFromName(value);
			if (m_hotkey == SDLK_UNKNOWN) {
				SetError("Couldn't interpret hotkey value '%s'", value);
				return false;
			}
		}
	}

	
	LoadNumber(node, "clickSound", m_clickSound);
	LoadString(node, "clickPanel", m_clickPanel);

	return true;
}

bool
UIElementButton::ShouldHandleKey(SDL_Keycode key)
{
	if (key == m_hotkey) {
		return true;
	}
	if (m_hotkey == ~0) {
		switch (key) {
			// Ignore modifier keys
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LGUI:
			case SDLK_RGUI:
				return false;
			default:
				return true;
		}
	}
	return false;
}

bool
UIElementButton::HandleEvent(const SDL_Event &event)
{
	bool checkMouseLocation = false;
	int x, y;

	if (event.type == SDL_MOUSEMOTION) {
		x = event.motion.x;
		y = event.motion.y;
		checkMouseLocation = true;
	}
	if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
		x = event.button.x;
		y = event.button.y;
		checkMouseLocation = true;
	}

	if (checkMouseLocation) {
		if (ContainsPoint(x, y)) {
			if (!m_mouseInside) {
				m_mouseInside = true;
				OnMouseEnter();
			}
		} else {
			if (m_mouseInside) {
				m_mouseInside = false;
				OnMouseLeave();
			}
		}
	}
	if (event.type == SDL_MOUSEMOTION) {
		return m_mouseInside;
	}

	if (event.type == SDL_MOUSEBUTTONDOWN &&
	    event.button.button == SDL_BUTTON_LEFT && m_mouseInside) {
		m_mousePressed = true;
		OnMouseDown();
		return true;
	}

	if (event.type == SDL_MOUSEBUTTONUP &&
	    event.button.button == SDL_BUTTON_LEFT && m_mousePressed) {
		m_mousePressed = false;
		OnMouseUp();
		if (m_mouseInside) {
			OnClick();
		}
		return true;
	}

	if (event.type == SDL_KEYDOWN &&
	    ShouldHandleKey(event.key.keysym.sym)) {
		if (!m_mousePressed) {
			m_mousePressed = true;
			OnMouseDown();
		}
		return true;
	}

	if (event.type == SDL_KEYUP &&
	    ShouldHandleKey(event.key.keysym.sym)) {
		if (!m_hotkeyMod || (event.key.keysym.mod & m_hotkeyMod)) {
			if (m_mousePressed) {
				m_mousePressed = false;
				OnMouseUp();
			}
			OnClick();
			return true;
		}
	}

	return false;
}

void
UIElementButton::OnClick()
{
	if (m_clickSound) {
		GetUI()->PlaySound(m_clickSound);
	}
	if (m_clickPanel) {
		GetUI()->ShowPanel(m_clickPanel);
	}
	if (m_delegate) {
		m_delegate->OnClick();
	}
}

void
UIElementButton::SetClickCallback(void (*callback)(void))
{
	SetButtonDelegate(new SimpleButtonDelegate(callback));
}

void
UIElementButton::SetButtonDelegate(UIButtonDelegate *delegate, bool autodelete)
{
	if (m_delegate && m_deleteDelegate) {
		delete m_delegate;
	}
	m_delegate = delegate;
	m_deleteDelegate = autodelete;
}
