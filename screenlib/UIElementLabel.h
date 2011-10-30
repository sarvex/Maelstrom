/*
  screenlib:  A simple window and UI library based on the SDL library
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _UIElementLabel_h
#define _UIElementLabel_h

#include "UIElement.h"
#include "UIFontInterface.h"


class UIElementLabel : public UIElement
{
DECLARE_TYPESAFE_CLASS(UIElement)
public:
	UIElementLabel(UIBaseElement *parent, const char *name = "");
	virtual ~UIElementLabel();

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	void SetText(const char *text);
	void SetTextColor(Uint8 R, Uint8 G, Uint8 B);
	void SetTextColor(Uint32 color);

	override void Draw();

protected:
	char *m_fontName;
	int m_fontSize;
	UIFontStyle m_fontStyle;
	Uint32 m_color;
	char *m_text;
	SDL_Texture *m_texture;
};

#endif // _UIElementLabel_h
