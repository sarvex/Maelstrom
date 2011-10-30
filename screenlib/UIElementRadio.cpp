
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
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (!m_elements[i]->IsA(UIElementRadioButton::GetType())) {
			continue;
		}
		if (m_elements[i] != button) {
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
