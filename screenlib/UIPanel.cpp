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

#include "SDL_FrameBuf.h"
#include "UIPanel.h"
#include "UIManager.h"
#include "UIElement.h"
#include "UITemplates.h"


UIPanel::UIPanel(UIManager *ui, const char *name) : UIArea(ui->GetScreen())
{
	m_ui = ui;
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);

	m_rect.w = m_screen->Width();
	m_rect.h = m_screen->Height();
	m_shown = false;
	m_fullscreen = true;
	m_cursorVisible = true;
	m_enterSound = 0;
	m_leaveSound = 0;
	m_delegate = NULL;
	m_deleteDelegate = false;

	m_ui->AddPanel(this);
}

UIPanel::~UIPanel()
{
	m_ui->RemovePanel(this);

	SetPanelDelegate(NULL);

	delete[] m_name;

	for (unsigned i = 0; i < m_elements.length(); ++i) {
		delete m_elements[i];
	}
}

bool
UIPanel::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;

	child = templates->GetTemplateFor(node);
	if (child) {
		if (!Load(child, templates)) {
			return false;
		}
	}

	attr = node->first_attribute("fullscreen", 0, false);
	if (attr) {
		const char *value = attr->value();
		if (*value == '0' || *value == 'f' || *value == 'F') {
			m_fullscreen = false;
		} else {
			m_fullscreen = true;
		}
	}
	attr = node->first_attribute("cursor", 0, false);
	if (attr) {
		const char *value = attr->value();
		if (*value == '0' || *value == 'f' || *value == 'F') {
			m_cursorVisible = false;
		} else {
			m_cursorVisible = true;
		}
	}
	attr = node->first_attribute("enterSound", 0, false);
	if (attr) {
		m_enterSound = atoi(attr->value());
	}
	attr = node->first_attribute("leaveSound", 0, false);
	if (attr) {
		m_leaveSound = atoi(attr->value());
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

bool
UIPanel::FinishLoading()
{
	if (m_delegate) {
		return m_delegate->OnLoad();
	} else {
		return true;
	}
}

bool
UIPanel::LoadElements(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	for (node = node->first_node(); node; node = node->next_sibling()) {
		UIElement *element = m_ui->CreateElement(this, node->name());
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

UIArea *
UIPanel::GetAnchorElement(const char *name)
{
	if (!name) {
		return m_ui;
	}
	return NULL;
}

UIElement *
UIPanel::GetElement(const char *name)
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (strcmp(name, m_elements[i]->GetName()) == 0) {
			return m_elements[i];
		}
	}
	return NULL;
}

void
UIPanel::SetPanelDelegate(UIPanelDelegate *delegate, bool autodelete)
{
	if (m_delegate && m_deleteDelegate) {
		delete m_delegate;
	}
	m_delegate = delegate;
	m_deleteDelegate = autodelete;
}

void
UIPanel::Show()
{
	if (m_enterSound) {
		m_ui->PlaySound(m_enterSound);
	}
	UIArea::Show();

	if (m_delegate) {
		m_delegate->OnShow();
	}
}

void
UIPanel::Hide()
{
	if (m_leaveSound) {
		m_ui->PlaySound(m_leaveSound);
	}
	UIArea::Hide();

	if (m_delegate) {
		m_delegate->OnHide();
	}
}

void
UIPanel::HideAll()
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		m_elements[i]->Hide();
	}
}

void
UIPanel::Draw()
{
	if (m_delegate) {
		m_delegate->OnTick();
	}
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (m_elements[i]->IsShown()) {
			m_elements[i]->Draw();
		}
	}
	if (m_delegate) {
		m_delegate->OnDraw();
	}
}

bool
UIPanel::HandleEvent(const SDL_Event &event)
{
	for (unsigned i = m_elements.length(); i--; ) {
		if (m_elements[i]->HandleEvent(event)) {
			return true;
		}
	}
	return false;
}
