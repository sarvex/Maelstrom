
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

UIElementLabel *
MacDialogCheckbox::CreateLabel()
{
	MacDialogLabel *label;

	label = new MacDialogLabel(this, "label");
	label->SetTextColor(m_color);
	label->SetAnchor(TOPLEFT, TOPRIGHT, this, 3, -2);
	return label;
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
