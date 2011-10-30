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

#include "UIElementEditbox.h"
#include "UIElementLabel.h"

UIElementType UIElementEditbox::s_elementType;


UIElementEditbox::UIElementEditbox(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
{
	m_focus = false;
	m_highlight = false;
	m_numeric = false;

	// This is a reasonable default for a non-wrapping editbox
	m_textMax = 128;
	m_textLen = 0;
	m_text = new char[m_textMax];
	m_text[0] = '\0';
}

UIElementEditbox::~UIElementEditbox()
{
	delete[] m_text;
}

bool
UIElementEditbox::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	LoadBool(node, "numeric", m_numeric);

	int maxLen;
	if (LoadNumber(node, "maxLen", maxLen)) {
		SetTextMax(maxLen);
	}

	return true;
}

bool
UIElementEditbox::FinishLoading()
{
	if (!m_label) {
		m_label = CreateLabel();
		if (!m_label) {
			fprintf(stderr, "Warning: Couldn't create editbox label\n");
			return false;
		}
		AddElement(m_label);
	}
}
			
bool
UIElementEditbox::HandleEvent(const SDL_Event &event)
{
	if (!m_focus) {
		return UIElementButton::HandleEvent(event);
	}

	if (event.type == SDL_KEYUP) {
		switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				SetFocus(false);
				return true;
			case SDLK_TAB:
				SetFocusNext();
				return true;
			case SDLK_DELETE:
			case SDLK_BACKSPACE:
				if (m_textLen > 0) {
					if (m_highlight) {
						m_textLen = 0;
						m_text[0] = '\0';
					} else {
						--m_textLen;
						m_text[m_textLen] = '\0';
					}
				}
				SetHighlight(false);
				OnTextChanged();
				return true;
			default:
				break;
		}
	}

	if (event.type == SDL_TEXTINPUT) {
		// Note, this doesn't support non-ASCII characters at the moment
		// To do that we would have to separate m_textMax and the size
		// of the text buffer and it gets complicated for in-line editing.
		char ch = event.text.text[0];
		bool valid;
		if (m_numeric) {
			valid = (ch >= '0' && ch <= '9');
		} else {
			valid = (ch >= ' ' && ch <= '~');
		}
		if (valid && (m_highlight || (m_textLen < m_textMax))) {
			if (m_highlight) {
				m_textLen = 0;
			}
			m_text[m_textLen++] = ch;
			m_text[m_textLen] = '\0';
			SetHighlight(false);
			OnTextChanged();
		}
		return true;
	}

	return UIElementButton::HandleEvent(event);
}

void
UIElementEditbox::SetFocus(bool focus)
{
	m_focus = focus;

	if (m_focus) {
		array<UIElementEditbox*> editboxes;

		SetHighlight(true);

		// Take focus away from other editboxes
		m_parent->FindElements<UIElementEditbox>(editboxes);
		for (unsigned i = 0; i < editboxes.length(); ++i) {
			if (editboxes[i] != this) {
				editboxes[i]->SetFocus(false);
			}
		}
	} else {
		SetHighlight(false);
	}
}

void
UIElementEditbox::SetHighlight(bool highlight)
{
	if (highlight != m_highlight) {
		m_highlight = highlight;
		OnHighlightChanged();
	}
}

void
UIElementEditbox::SetFocusNext()
{
	UIElementEditbox *editbox;
	unsigned i, j;

	// We always lose focus even if we don't find another editbox
	SetFocus(false);

	editbox = m_parent->FindElement<UIElementEditbox>(this);
	if (editbox) {
		editbox->SetFocus(true);
	}
}

void
UIElementEditbox::SetTextMax(int maxLen)
{
	char *oldText = m_text;

	m_textMax = maxLen;
	m_text = new char[m_textMax+1];
	if (m_textLen <= m_textMax) {
		SDL_strlcpy(m_text, oldText, m_textMax+1);
	} else {
		SetText(oldText);
	}
	delete[] oldText;
}

void
UIElementEditbox::SetText(const char *text)
{
	SDL_strlcpy(m_text, text, m_textMax+1);
	m_textLen = SDL_strlen(m_text);
	OnTextChanged();
}

void
UIElementEditbox::OnTextChanged()
{
	UIElementButton::SetText(m_text);
}
