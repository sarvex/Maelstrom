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
#include "MacDialogEditbox.h"
#include "MacDialogLabel.h"

UIElementType MacDialogEditbox::s_elementType;


MacDialogEditbox::MacDialogEditbox(UIBaseElement *parent, const char *name) :
	UIElementEditbox(parent, name)
{
	m_colors[0] = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_colors[1] = m_screen->MapRGB(0x00, 0x00, 0x00);
}

UIElementLabel *
MacDialogEditbox::CreateLabel()
{
	MacDialogLabel *label;

	label = new MacDialogLabel(this, "label");
	label->SetTextColor(m_colors[1]);
	label->SetAnchor(LEFT, LEFT, this, 3, 0);
	return label;
}

void
MacDialogEditbox::OnHighlightChanged()
{
	SetElementColor(m_colors[!m_highlight]);
}

void
MacDialogEditbox::Draw()
{
	Uint32 bg, fg;

	/* The colors are inverted when the editbox is highlighted */
	bg = m_colors[m_highlight];
	fg = m_colors[!m_highlight];

	// Draw the outline, always in the real foreground color
	m_screen->DrawRect(X(), Y(), Width(), Height(), m_colors[1]);

	if (m_highlight) {
		// Draw the highlight
		m_screen->FillRect(X()+3, Y()+3, Width()-6, Height()-6, bg);
	} else if (m_focus) {
		// Draw the cursor
		int x = m_label->X() + m_label->Width();

		m_screen->DrawLine(x, Y()+3, x, Y()+3+Height()-6-1, fg);
	}

	UIElementEditbox::Draw();
}

void
MacDialogEditbox::SetElementColor(Uint32 color)
{
	array<UIElementLabel*> labels;
	Uint8 R, G, B;

	m_screen->GetRGB(color, &R, &G, &B);
	FindElements<UIElementLabel>(labels);
	for (unsigned i = 0; i < labels.length(); ++i) {
		labels[i]->SetTextColor(R, G, B);
	}
}
