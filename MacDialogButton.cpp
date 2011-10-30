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
#include "MacDialogButton.h"
#include "MacDialogLabel.h"

/* Default dialog button size */
#define BUTTON_WIDTH	75
#define BUTTON_HEIGHT	19


UIElementType MacDialogButton::s_elementType;


MacDialogButton::MacDialogButton(UIBaseElement *parent, const char *name) :
	UIDialogButton(parent, name)
{
	m_colors[0] = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_colors[1] = m_screen->MapRGB(0x00, 0x00, 0x00);

	SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
}

UIElementLabel *
MacDialogButton::CreateLabel()
{
	MacDialogLabel *label;

	label = new MacDialogLabel(this, "label");
	label->SetTextColor(m_colors[1]);
	return label;
}

void
MacDialogButton::Draw()
{
	Uint32 bg, fg;
	int x, y, maxx, maxy;

	/* The colors are inverted when the mouse is pressed */
	bg = m_colors[m_mousePressed];
	fg = m_colors[!m_mousePressed];

	/* First draw the background */
	m_screen->FillRect(X(), Y(), Width(), Height(), bg);

	/* Draw the beveled edge */
	x = X();
	maxx = x+Width()-1;
	y = Y();
	maxy = y+Height()-1;

	/* Top and upper corners */
	m_screen->DrawLine(x+3, y, maxx-3, y, fg);
	m_screen->DrawLine(x+1, y+1, x+2, y+1, fg);
	m_screen->DrawLine(maxx-2, y+1, maxx-1, y+1, fg);
	m_screen->DrawLine(x+1, y+2, x+1, y+2, fg);
	m_screen->DrawLine(maxx-1, y+2, maxx-1, y+2, fg);

	/* Sides */
	m_screen->DrawLine(x, y+3, x, maxy-3, fg);
	m_screen->DrawLine(maxx, y+3, maxx, maxy-3, fg);

	/* Bottom and lower corners */
	m_screen->DrawLine(x+1, maxy-2, x+1, maxy-2, fg);
	m_screen->DrawLine(maxx-1, maxy-2, maxx-1, maxy-2, fg);
	m_screen->DrawLine(x+1, maxy-1, x+2, maxy-1, fg);
	m_screen->DrawLine(maxx-2, maxy-1, maxx-1, maxy-1, fg);
	m_screen->DrawLine(x+3, maxy, maxx-3, maxy, fg);

	if (m_default) {
		/* Show the thick edge */
		x = X()-4;
		maxx = x+4+Width()+4-1;
		y = Y()-4;
		maxy = y+4+Height()+4-1;

		/* The edge always uses the real foreground color */
		fg = m_colors[1];

		m_screen->DrawLine(x+5, y, maxx-5, y, fg);
		m_screen->DrawLine(x+3, y+1, maxx-3, y+1, fg);
		m_screen->DrawLine(x+2, y+2, maxx-2, y+2, fg);
		m_screen->DrawLine(x+1, y+3, x+5, y+3, fg);
		m_screen->DrawLine(maxx-5, y+3, maxx-1, y+3, fg);
		m_screen->DrawLine(x+1, y+4, x+3, y+4, fg);
		m_screen->DrawLine(maxx-3, y+4, maxx-1, y+4, fg);
		m_screen->DrawLine(x, y+5, x+3, y+5, fg);
		m_screen->DrawLine(maxx-3, y+5, maxx, y+5, fg);

		m_screen->DrawLine(x, y+6, x, maxy-6, fg);
		m_screen->DrawLine(maxx, y+6, maxx, maxy-6, fg);
		m_screen->DrawLine(x+1, y+6, x+1, maxy-6, fg);
		m_screen->DrawLine(maxx-1, y+6, maxx-1, maxy-6, fg);
		m_screen->DrawLine(x+2, y+6, x+2, maxy-6, fg);
		m_screen->DrawLine(maxx-2, y+6, maxx-2, maxy-6, fg);

		m_screen->DrawLine(x, maxy-5, x+3, maxy-5, fg);
		m_screen->DrawLine(maxx-3, maxy-5, maxx, maxy-5, fg);
		m_screen->DrawLine(x+1, maxy-4, x+3, maxy-4, fg);
		m_screen->DrawLine(maxx-3, maxy-4, maxx-1, maxy-4, fg);
		m_screen->DrawLine(x+1, maxy-3, x+5, maxy-3, fg);
		m_screen->DrawLine(maxx-5, maxy-3, maxx-1, maxy-3, fg);
		m_screen->DrawLine(x+2, maxy-2, maxx-2, maxy-2, fg);
		m_screen->DrawLine(x+3, maxy-1, maxx-3, maxy-1, fg);
		m_screen->DrawLine(x+5, maxy, maxx-5, maxy, fg);
	}

	UIDialogButton::Draw();
}

void
MacDialogButton::OnMouseDown()
{
	SetElementColor(m_colors[0]);

	UIDialogButton::OnMouseDown();
}

void
MacDialogButton::OnMouseUp()
{
	SetElementColor(m_colors[1]);

	UIDialogButton::OnMouseUp();
}

void
MacDialogButton::SetElementColor(Uint32 color)
{
	array<UIElementLabel*> labels;
	Uint8 R, G, B;

	m_screen->GetRGB(color, &R, &G, &B);
	FindElements<UIElementLabel>(labels);
	for (unsigned i = 0; i < labels.length(); ++i) {
		labels[i]->SetTextColor(R, G, B);
	}
}
