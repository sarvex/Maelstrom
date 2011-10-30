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

#include "UIElementRadio.h"
#include "UIElementLabel.h"

UIElementType UIElementRadioGroup::s_elementType;


UIElementRadioGroup::UIElementRadioGroup(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_value = -1;
}

UIElementRadioButton *
UIElementRadioGroup::GetRadioButton(int id)
{
	UIElementRadioButton *button;

	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (!m_elements[i]->IsA(UIElementRadioButton::GetType())) {
			continue;
		}

		button = static_cast<UIElementRadioButton*>(m_elements[i]);
		if (button->GetID() == id) {
			return button;
		}
	}
	return NULL;
}

void
UIElementRadioGroup::RadioButtonChecked(UIElementRadioButton *button)
{
	array<UIElementRadioButton*> buttons;

	FindElements<UIElementRadioButton>(buttons);
	for (unsigned i = 0; i < buttons.length(); ++i) {
		if (buttons[i] != button && buttons[i]->IsChecked()) {
			buttons[i]->OnClick();
		}
	}
	m_value = button->GetID();
}


UIElementType UIElementRadioButton::s_elementType;


UIElementRadioButton::UIElementRadioButton(UIBaseElement *parent, const char *name) :
	UIElementCheckbox(parent, name)
{
	m_id = -1;
}

bool
UIElementRadioButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	if (!UIElementCheckbox::Load(node, templates)) {
		return false;
	}

	LoadNumber(node, "id", m_id);

	return true;
}

bool
UIElementRadioButton::FinishLoading()
{
	if (m_label) {
		// Extend the sensitive area to encompass the label
		if (m_label->X() >= X()) {
			SetWidth((m_label->X()+m_label->Width()) - X());
		} else {
			assert(!"Need code for labels on the left");
		}
	}
}

void
UIElementRadioButton::OnChecked(bool checked)
{
	if (checked) {
		if (m_parent->IsA(UIElementRadioGroup::GetType())) {
			static_cast<UIElementRadioGroup*>(m_parent)->RadioButtonChecked(this);
		}
	}
}
