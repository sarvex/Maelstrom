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
#include "UIElement.h"
#include "UITemplates.h"
#include "UIDrawEngine.h"


UIDrawEngine::UIDrawEngine()
{
	m_screen = NULL;
	m_ui = NULL;
	m_element = NULL;
	m_textImage = NULL;
}

UIDrawEngine::~UIDrawEngine()
{
}

void
UIDrawEngine::Init(UIElement *element)
{
	m_ui = element->GetUI();
	m_screen = element->GetScreen();
	m_element = element;
}

bool
UIDrawEngine::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;

	child = templates->GetTemplateFor(node);
	if (child) {
		if (!Load(child, templates)) {
			return false;
		}
	}

	return true;
}

bool
UIDrawEngine::LoadBool(rapidxml::xml_node<> *node, const char *name, bool &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		const char *text = attr->value();
		if (*text == '\0' || *text == '0' ||
		    *text == 'f' || *text == 'F') {
			value = false;
		} else {
			value = true;
		}
		return true;
	}
	return false;
}

bool
UIDrawEngine::LoadNumber(rapidxml::xml_node<> *node, const char *name, int &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		value = (int)SDL_strtol(attr->value(), NULL, 0);
		return true;
	}
	return false;
}

bool
UIDrawEngine::LoadString(rapidxml::xml_node<> *node, const char *name, char *&value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		if (value) {
			SDL_free(value);
		}
		value = SDL_strdup(attr->value());
		return true;
	}
	return false;
}

void
UIDrawEngine::OnDraw()
{
	if (m_element->HasFill()) {
		m_screen->FillRect(m_element->X(), m_element->Y(),
				m_element->Width(), m_element->Height(),
				m_element->GetFillColor());
	}

	if (m_element->HasBorder()) {
		m_screen->DrawRect(m_element->X(), m_element->Y(),
				m_element->Width(), m_element->Height(),
				m_element->GetCurrentColor());
	}

	SDL_Texture *image = m_element->GetImage();
	if (image) {
		UIArea *area = m_element->GetImageArea();
		m_screen->QueueBlit(area->X(), area->Y(), image, NOCLIP);
	}

	if (m_textImage) {
		UIArea *area = m_element->GetTextArea();
		int x, y;
		if (m_element->GetTextShadowOffset(&x, &y)) {
			Uint8 r, g, b;

			m_screen->GetRGB(m_element->GetTextShadowColor(), &r, &g, &b);
			SDL_SetTextureColorMod(m_textImage, r, g, b);

			m_screen->QueueBlit(area->X()+x, area->Y()+y, m_textImage, NOCLIP);

			m_screen->GetRGB(m_element->GetCurrentColor(), &r, &g, &b);
			SDL_SetTextureColorMod(m_textImage, r, g, b);
		}
		m_screen->QueueBlit(area->X(), area->Y(), m_textImage, NOCLIP);
	}
}

void
UIDrawEngine::OnColorChanged()
{
	if (m_textImage) {
		m_ui->FreeText(m_textImage);

		m_textImage = m_ui->CreateText(m_element->GetText(),
						m_element->GetFontName(),
						m_element->GetFontSize(),
						m_element->GetFontStyle(),
						m_element->GetCurrentColor());
	}
}

void
UIDrawEngine::OnFontChanged()
{
	OnTextChanged();
}

void
UIDrawEngine::OnTextChanged()
{
	if (m_textImage) {
		m_ui->FreeText(m_textImage);
		m_textImage = NULL;
	}

	const char *text = m_element->GetText();
	if (text && *text && m_element->HasFont()) {
		int w, h;
		m_textImage = m_ui->CreateText(text,
						m_element->GetFontName(),
						m_element->GetFontSize(),
						m_element->GetFontStyle(),
						m_element->GetCurrentColor());

		w = m_screen->GetImageWidth(m_textImage);
		h = m_screen->GetImageHeight(m_textImage);
		m_element->GetTextArea()->SetSize(w, h);
		m_element->AutoSize(w, h);
	} else {
		m_element->GetTextArea()->SetWidth(0);
		m_element->AutoSize(0, m_element->Height());
	}
}

void
UIDrawEngine::OnImageChanged()
{
	SDL_Texture *image = m_element->GetImage();

	if (image) {
		int w, h;
		w = m_screen->GetImageWidth(image);
		h = m_screen->GetImageHeight(image);
		m_element->GetImageArea()->SetSize(w, h);
		m_element->AutoSize(w, h);
	}
}

//////////////////////////////////////////////////////////////////////////////

void
UIDrawEngineLine::OnDraw()
{
	m_screen->DrawLine(m_element->X(), m_element->Y(),
			m_element->X()+m_element->Width()-1,
			m_element->Y()+m_element->Height()-1,
			m_element->GetColor());
}

//////////////////////////////////////////////////////////////////////////////

void
UIDrawEngineRect::OnDraw()
{
	if (m_element->HasFill()) {
		m_screen->FillRect(m_element->X(), m_element->Y(),
				m_element->Width(), m_element->Height(),
				m_element->GetColor());
	} else {
		m_screen->DrawRect(m_element->X(), m_element->Y(),
				m_element->Width(), m_element->Height(),
				m_element->GetColor());
	}
}
