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


class SimpleButtonCallback : public UIButtonCallback
{
public:
	SimpleButtonCallback(void (*callback)(void)) {
		m_callback = callback;
	}

	virtual void OnClick() {
		m_callback();
	}

protected:
	void (*m_callback)(void);
};

UIElementType UIElementButton::s_elementType;


UIElementButton::UIElementButton(UIPanel *panel, const char *name) :
	UIElement(panel, name)
{
	m_hotkey = SDLK_UNKNOWN;
	m_hotkeyMod = KMOD_NONE;
	m_mouseInside = false;
	m_mousePressed = false;
	m_clickSound = 0;
	m_clickPanel = NULL;
	m_callback = NULL;
}

UIElementButton::~UIElementButton()
{
	if (m_clickPanel) {
		delete[] m_clickPanel;
	}
	if (m_callback) {
		delete m_callback;
	}
}

bool
UIElementButton::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_attribute<> *attr;

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

	attr = node->first_attribute("clickSound", 0, false);
	if (attr) {
		m_clickSound = atoi(attr->value());
	}
	attr = node->first_attribute("clickPanel", 0, false);
	if (attr) {
		const char *value = attr->value();
		m_clickPanel = new char[strlen(value)+1];
		strcpy(m_clickPanel, value);
	}
	return UIElement::Load(node);
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
	if (event.type == SDL_MOUSEMOTION) {
		if (ContainsPoint(event.motion.x, event.motion.y)) {
			if (!m_mouseInside) {
				m_mouseInside = true;
				OnMouseEnter();
			}
			return true;
		} else {
			if (m_mouseInside) {
				m_mouseInside = false;
				OnMouseLeave();
			}
			return false;
		}
	}

	if (event.type == SDL_MOUSEBUTTONDOWN &&
	    event.button.button == SDL_BUTTON_LEFT &&
	    ContainsPoint(event.motion.x, event.motion.y)) {
		m_mousePressed = true;
		OnMouseDown();
		return true;
	}

	if (event.type == SDL_MOUSEBUTTONUP &&
	    event.button.button == SDL_BUTTON_LEFT &&
	    m_mousePressed) {
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
		m_panel->GetUI()->PlaySound(m_clickSound);
	}
	if (m_clickPanel) {
		m_panel->GetUI()->ShowPanel(m_clickPanel);
	}
	if (m_callback) {
		m_callback->OnClick();
	}
}

void
UIElementButton::SetClickCallback(void (*callback)(void))
{
	SetClickCallback(new SimpleButtonCallback(callback));
}

void
UIElementButton::SetClickCallback(UIButtonCallback *callback)
{
	if (m_callback) {
		delete m_callback;
	}
	m_callback = callback;
}
