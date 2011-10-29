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

#include "SDL.h"
#include "UIBaseElement.h"
#include "UIManager.h"
#include "UIElement.h"
#include "UITemplates.h"


UIElementType UIBaseElement::s_elementTypeIndex;
UIElementType UIBaseElement::s_elementType;


UIBaseElement::UIBaseElement(UIManager *ui, const char *name) :
	UIArea(ui->GetScreen())
{
	m_ui = ui;
	m_parent = NULL;
	m_name = SDL_strdup(name);
}

UIBaseElement::UIBaseElement(UIBaseElement *parent, const char *name) :
	UIArea(parent->GetScreen())
{
	m_ui = parent->GetUI();
	m_parent = parent;
	m_name = SDL_strdup(name);
}

UIBaseElement::~UIBaseElement()
{
	SDL_free(m_name);

	for (unsigned i = 0; i < m_elements.length(); ++i) {
		delete m_elements[i];
	}
}

bool
UIBaseElement::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;

	child = templates->GetTemplateFor(node);
	if (child) {
		if (!Load(child, templates)) {
			return false;
		}
	}

	if (!UIArea::Load(node)) {
		return false;
	}

	child = node->first_node("elements", 0, false);
	if (child) {
		if (!LoadElements(child, templates)) {
			return false;
		}
	}

	return true;
}

void
UIBaseElement::Draw()
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (m_elements[i]->IsShown()) {
			m_elements[i]->Draw();
		}
	}
}

bool
UIBaseElement::HandleEvent(const SDL_Event &event)
{
	for (unsigned i = m_elements.length(); i--; ) {
		if (m_elements[i]->HandleEvent(event)) {
			return true;
		}
	}
	return false;
}

UIBaseElement *
UIBaseElement::GetElement(const char *name)
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (strcmp(name, m_elements[i]->GetName()) == 0) {
			return m_elements[i];
		}
	}
	return NULL;
}

UIBaseElement *
UIBaseElement::CreateElement(const char *name)
{
	return GetUI()->CreateElement(this, name);
}


bool
UIBaseElement::LoadElements(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	for (node = node->first_node(); node; node = node->next_sibling()) {
		UIBaseElement *element = CreateElement(node->name());
		if (!element) {
			fprintf(stderr, "Warning: Couldn't find handler for element %s\n", node->name());
			continue;
		}

		if (!element->Load(node, templates) ||
		    !element->FinishLoading()) {
			fprintf(stderr, "Warning: Couldn't load element %s: %s\n", node->name(), element->Error());
			delete element;
		} else {
			AddElement(element);
		}
	}
	return true;
}
