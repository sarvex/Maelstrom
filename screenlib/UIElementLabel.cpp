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

#include "SDL_FrameBuf.h"
#include "UIManager.h"
#include "UIElementLabel.h"

UIElementType UIElementLabel::s_elementType;


UIElementLabel::UIElementLabel(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_fontName = NULL;
	m_fontSize = 0;
	m_fontStyle = UIFONT_STYLE_NORMAL;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_text = NULL;
	m_texture = NULL;
}

UIElementLabel::~UIElementLabel()
{
	if (m_fontName) {
		SDL_free(m_fontName);
	}
	if (m_text) {
		SDL_free(m_text);
	}
	if (m_texture) {
		GetUI()->FreeText(m_texture);
	}
}

static UIFontStyle ParseStyle(const char *text)
{
	if (strcasecmp(text, "BOLD") == 0) {
		return UIFONT_STYLE_BOLD;
	} else if (strcasecmp(text, "UNDERLINE") == 0) {
		return UIFONT_STYLE_UNDERLINE;
	} else if (strcasecmp(text, "ITALIC") == 0) {
		return UIFONT_STYLE_UNDERLINE;
	}
	return UIFONT_STYLE_NORMAL;
}

bool
UIElementLabel::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	LoadString(node, "fontName", m_fontName);
	LoadNumber(node, "fontSize", m_fontSize);

	attr = node->first_attribute("fontStyle", 0, false);
	if (attr) {
		m_fontStyle = ParseStyle(attr->value());
	}

	LoadColor(node, "color", m_color);

	attr = node->first_attribute("text", 0, false);
	if (attr) {
		SetText(attr->value());
	}

	return true;
}

void
UIElementLabel::SetText(const char *text)
{
	if (!m_fontName || !m_fontSize) {
		SetError("You must set a font first");
		return;
	}

	if (m_text && SDL_strcmp(text, m_text) == 0) {
		return;
	}

	if (m_text) {
		SDL_free(m_text);
	}
	if (m_texture) {
		GetUI()->FreeText(m_texture);
		m_texture = NULL;
	}

	m_text = SDL_strdup(text);
	if (*m_text) {
		m_texture = GetUI()->CreateText(m_text, m_fontName, m_fontSize, m_fontStyle, m_color);

		SetSize(m_screen->GetImageWidth(m_texture), 
			m_screen->GetImageHeight(m_texture));
	} else {
		SetWidth(0);
	}
}

void
UIElementLabel::SetTextColor(Uint8 R, Uint8 G, Uint8 B)
{
	SetTextColor(m_screen->MapRGB(R, G, B));
}

void
UIElementLabel::SetTextColor(Uint32 color)
{
	if (color == m_color) {
		return;
	}
	m_color = color;

	if (m_text) {
		if (m_texture) {
			GetUI()->FreeText(m_texture);
		}
		m_texture = GetUI()->CreateText(m_text, m_fontName, m_fontSize, m_fontStyle, m_color);
	}
}

void
UIElementLabel::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(X(), Y(), m_texture, NOCLIP);
	}
}
