/*
    Maelstrom: Open Source version of the classic game by Ambrosia Software
    Copyright (C) 1997-2011  Sam Lantinga

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
	label->SetAnchor(TOPLEFT, TOPLEFT, this, CHECKBOX_SIZE+3, -2);
	return label;
}

void
MacDialogCheckbox::Draw()
{
	m_screen->DrawRect(X(), Y(), CHECKBOX_SIZE, CHECKBOX_SIZE, m_color);

	if ( IsChecked() ) {
		m_screen->DrawLine(X(), Y(), X()+CHECKBOX_SIZE-1,
					Y()+CHECKBOX_SIZE-1, m_color);
		m_screen->DrawLine(X(), Y()+CHECKBOX_SIZE-1,
					X()+CHECKBOX_SIZE-1, Y(), m_color);
	}

	UIElementButton::Draw();
}
