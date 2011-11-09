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

#include "Maelstrom_Globals.h"

#include "../screenlib/UIManager.h"
#include "../screenlib/UIDrawEngine.h"

class HashTable;

class MaelstromUI : public UIManager
{
public:
	MaelstromUI(FrameBuf *screen, Prefs *prefs);
	virtual ~MaelstromUI();

	//
	// UIFontInterface
	//
	virtual SDL_Texture *CreateText(const char *text, const char *fontName, int fontSize, UIFontStyle fontStyle, Uint32 color);
	virtual void FreeText(SDL_Texture *texture);

	//
	// UISoundInterface
	//
	virtual void PlaySound(int soundID);

	//
	// UIManager functions
	//
	virtual UIPanel *CreatePanel(const char *type, const char *name);
	virtual UIPanelDelegate *CreatePanelDelegate(UIPanel *panel, const char *delegate);
	virtual UIElement *CreateElement(UIBaseElement *parent, const char *type, const char *name);

protected:
	HashTable *m_fonts;
	HashTable *m_strings;

protected:
	MFont *GetFont(const char *fontName, int fontSize);
};

//////////////////////////////////////////////////////////////////////////////
class UIElementControlButton : public UIElement
{
public:
	UIElementControlButton(UIBaseElement *parent, const char *name, UIDrawEngine *drawEngine);

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	override bool HandleEvent(const SDL_Event &event);
	override void OnMouseDown();
	override void OnMouseUp();

protected:
	unsigned char m_control;
	SDL_FingerID m_finger;
};

//////////////////////////////////////////////////////////////////////////////
class UIDrawEngineIcon : public UIDrawEngine
{
public:
	UIDrawEngineIcon() : UIDrawEngine() { }

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

//////////////////////////////////////////////////////////////////////////////
class UIDrawEngineSprite : public UIDrawEngine
{
public:
	UIDrawEngineSprite() : UIDrawEngine() { }

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

//////////////////////////////////////////////////////////////////////////////
class UIDrawEngineTitle : public UIDrawEngine
{
public:
	UIDrawEngineTitle() : UIDrawEngine() { }

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};
