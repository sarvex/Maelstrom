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

#ifndef _MacDialogButton_h
#define _MacDialogButton_h

#include "screenlib/UIDialogButton.h"


class MacDialogButton : public UIDialogButton
{
DECLARE_TYPESAFE_CLASS(UIDialogButton)
public:
	MacDialogButton(UIBaseElement *parent, const char *name = "");

	override void Draw();

	override void OnMouseDown();
	override void OnMouseUp();

protected:
	override UIElementLabel *CreateLabel();

	void SetElementColor(Uint32 color);

protected:
	Uint32 m_colors[2];
};

#endif // _MacDialogButton_h
