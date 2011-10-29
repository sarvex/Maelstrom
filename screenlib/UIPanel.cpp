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


UIPanel::UIPanel(UIManager *ui, const char *name) :
	UIBaseElement(ui, name)
{
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
}

bool
UIPanel::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIBaseElement::Load(node, templates)) {
		return false;
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

	UIBaseElement::Draw();

	if (m_delegate) {
		m_delegate->OnDraw();
	}
}
