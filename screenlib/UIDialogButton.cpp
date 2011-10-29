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
#include "UIManager.h"
#include "UIDialog.h"
#include "UIDialogButton.h"
#include "UIElementLabel.h"

UIElementType UIDialogButton::s_elementType;


UIDialogButton::UIDialogButton(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
{
	m_statusID = 0;
	m_default = false;
	m_closeDialog = true;
}

bool
UIDialogButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	LoadNumber(node, "id", m_statusID);

	LoadBool(node, "default", m_default);
	if (m_default) {
		m_hotkey = SDLK_RETURN;
	}

	LoadBool(node, "closeDialog", m_closeDialog);

	attr = node->first_attribute("text", 0, false);
	if (attr) {
		UIElement *label;

		label = GetUI()->CreateElement(this, "DialogLabel", "label");
		if (label && label->IsA(UIElementLabel::GetType())) {
			static_cast<UIElementLabel*>(label)->SetText(attr->value());
			AddElement(label);
		} else {
			fprintf(stderr, "Warning: Couldn't create dialog button label");
			delete label;
		}
	}

	return true;
}

void
UIDialogButton::OnClick()
{
	UIElementButton::OnClick();

	if (m_statusID) {
		UIPanel *panel = GetUI()->GetCurrentPanel();
		if (panel->IsA(UIDialog::GetType())) {
			static_cast<UIDialog*>(panel)->SetDialogStatus(m_statusID);
		}
	}
	if (m_closeDialog) {
		GetUI()->HidePanel(GetUI()->GetCurrentPanel());
	}
}
