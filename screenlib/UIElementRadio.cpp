
#include "UIElementRadio.h"

UIElementType UIElementRadioGroup::s_elementType;


UIElementRadioGroup::UIElementRadioGroup(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_value = -1;
}

void
UIElementRadioGroup::RadioButtonChecked(UIElementRadioButton *button)
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (m_elements[i] != button &&
		    m_elements[i]->IsA(UIElementRadioButton::GetType())) {
			static_cast<UIElementRadioButton*>(m_elements[i])->SetChecked(false);
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

void
UIElementRadioButton::OnChecked(bool checked)
{
	if (checked) {
		if (m_parent->IsA(UIElementRadioGroup::GetType())) {
			static_cast<UIElementRadioGroup*>(m_parent)->RadioButtonChecked(this);
		}
	}
}
