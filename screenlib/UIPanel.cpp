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
#include "UIPanel.h"
#include "UIManager.h"
#include "UIElement.h"
#include "UITemplates.h"

UIElementType UIPanel::s_elementType;


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

	LoadBool(node, "fullscreen", m_fullscreen);
	LoadBool(node, "cursor", m_cursorVisible);
	LoadNumber(node, "enterSound", m_enterSound);
	LoadNumber(node, "leaveSound", m_leaveSound);

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

bool
UIPanel::HandleEvent(const SDL_Event &event)
{
	if (UIBaseElement::HandleEvent(event)) {
		return true;
	}

	if (m_delegate) {
		return m_delegate->HandleEvent(event);
	}
	return false;
}
