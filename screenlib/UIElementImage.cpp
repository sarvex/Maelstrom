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

#include "SDL_FrameBuf.h"
#include "../utils/physfsrwops.h"
#include "UIElementImage.h"

UIElementType UIElementImage::s_elementType;


UIElementImage::UIElementImage(UIBaseElement *parent, const char *name) :
	UIElementTexture(parent, name)
{
}

bool
UIElementImage::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;
	SDL_Surface *bmp;
	SDL_Texture *texture;
	

	if (!UIElementTexture::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("file", 0, false);
	if (!attr) {
		SetError("Element '%s' missing attribute 'file'", node->name());
		return false;
	}

	bmp = SDL_LoadBMP_RW(PHYSFSRWOPS_openRead(attr->value()), 1);
	if (!bmp) {
		SetError("Element '%s' Couldn't open file '%s'", node->name(), attr->value());
		return false;
	}

	texture = m_screen->LoadImage(bmp);
	SDL_FreeSurface(bmp);
	if (!texture) {
		SetError("Element '%s' Couldn't load image from '%s'", node->name(), attr->value());
		return false;
	}
	SetTexture(texture);

	return true;
}
