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

#include "MacDialogLabel.h"
#include "Maelstrom_Globals.h"

UIElementType MacDialogLabel::s_elementType;


MacDialogLabel::MacDialogLabel(UIBaseElement *parent, const char *name) :
	UIElementLabel(parent, name)
{
	m_fontName = SDL_strdup("Chicago");
	m_fontSize = 12;
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);
}
