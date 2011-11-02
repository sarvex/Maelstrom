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

#include "UIElementCheckbox.h"

UIElementType UIElementCheckbox::s_elementType;


UIElementCheckbox::UIElementCheckbox(UIBaseElement *parent, const char *name, UIDrawEngine *drawEngine) :
	UIElementButton(parent, name, drawEngine)
{
	m_checked = false;
}

bool
UIElementCheckbox::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	/* Call SetChecked() to trigger derived classes' behaviors */
	bool checked;
	if (LoadBool(node, "checked", checked)) {
		SetChecked(checked);
	}

	return true;
}

bool
UIElementCheckbox::FinishLoading()
{
	// Extend the sensitive area to encompass the label
	if (m_textArea.X() >= X()) {
		SetWidth((m_textArea.X()+m_textArea.Width()) - X());
	} else {
		assert(!"Need code for labels on the left");
	}
	return true;
}

void
UIElementCheckbox::OnClick()
{
	SetChecked(!m_checked);

	UIElementButton::OnClick();
}
