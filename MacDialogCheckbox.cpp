
#include "screenlib/SDL_FrameBuf.h"
#include "MacDialogCheckbox.h"
#include "MacDialogLabel.h"

/* Default checkbox size */
#define CHECKBOX_SIZE	12


UIElementType MacDialogCheckbox::s_elementType;


MacDialogCheckbox::MacDialogCheckbox(UIBaseElement *parent, const char *name) :
	UIElementCheckbox(parent, name)
{
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);

	SetSize(CHECKBOX_SIZE, CHECKBOX_SIZE);
}

bool
MacDialogCheckbox::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElementCheckbox::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("text", 0, false);
	if (attr) {
		MacDialogLabel *label;

		label = new MacDialogLabel(this, "label");
		label->SetText(attr->value());
		label->SetTextColor(m_color);
		label->SetAnchor(TOPLEFT, TOPRIGHT, this, 3, -2);
		AddElement(label);
	}

	return true;
}

void
MacDialogCheckbox::Draw()
{
	m_screen->DrawRect(X(), Y(), Width(), Height(), m_color);

	if ( IsChecked() ) {
		m_screen->DrawLine(X(), Y(),
				X()+Width()-1, Y()+Height()-1, m_color);
		m_screen->DrawLine(X(), Y()+Height()-1,
					X()+Width()-1, Y(), m_color);
	}

	UIElementButton::Draw();
}
