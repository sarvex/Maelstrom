
#include "UIElementCheckbox.h"

UIElementType UIElementCheckbox::s_elementType;


UIElementCheckbox::UIElementCheckbox(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
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

void
UIElementCheckbox::OnClick()
{
	UIElementButton::OnClick();

	SetChecked(!m_checked);
}
