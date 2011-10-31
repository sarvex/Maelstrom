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
#include "MacDialogRadioButton.h"
#include "MacDialogLabel.h"

/* Default checkbox size */
#define RADIOBUTTON_SIZE	20

UIElementType MacDialogRadioButton::s_elementType;


MacDialogRadioButton::MacDialogRadioButton(UIBaseElement *parent, const char *name) :
	UIElementRadioButton(parent, name)
{
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);

	SetSize(RADIOBUTTON_SIZE, RADIOBUTTON_SIZE);
}


UIElementLabel *
MacDialogRadioButton::CreateLabel()
{
	MacDialogLabel *label;

	label = new MacDialogLabel(this, "label");
	label->SetAnchor(TOPLEFT, TOPLEFT, this, RADIOBUTTON_SIZE+1, 3);
	return label;
}

void
MacDialogRadioButton::Draw()
{
	int x = X() + 5;
	int y = Y() + 5;

	/* Draw the circle */
	m_screen->DrawLine(x+4, y, x+7, y, m_color);
	m_screen->DrawLine(x+2, y+1, x+3, y+1, m_color);
	m_screen->DrawLine(x+8, y+1, x+9, y+1, m_color);
	m_screen->DrawLine(x+1, y+2, x+1, y+3, m_color);
	m_screen->DrawLine(x+10, y+2, x+10, y+3, m_color);
	m_screen->DrawLine(x, y+4, x, y+7, m_color);
	m_screen->DrawLine(x+11, y+4, x+11, y+7, m_color);
	m_screen->DrawLine(x+1, y+8, x+1, y+9, m_color);
	m_screen->DrawLine(x+10, y+8, x+10, y+9, m_color);
	m_screen->DrawLine(x+2, y+10, x+3, y+10, m_color);
	m_screen->DrawLine(x+8, y+10, x+9, y+10, m_color);
	m_screen->DrawLine(x+4, y+11, x+7, y+11, m_color);

	if ( IsChecked() ) {
		/* Draw the spot in the center */
		x += 3;
		y += 3;

		m_screen->DrawLine(x+1, y, x+4, y, m_color);
		++y;
		m_screen->DrawLine(x, y, x+5, y, m_color);
		++y;
		m_screen->DrawLine(x, y, x+5, y, m_color);
		++y;
		m_screen->DrawLine(x, y, x+5, y, m_color);
		++y;
		m_screen->DrawLine(x, y, x+5, y, m_color);
		++y;
		m_screen->DrawLine(x+1, y, x+4, y, m_color);
	}

	UIElementRadioButton::Draw();
}
