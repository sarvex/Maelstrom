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

#include "MaelstromUI.h"
#include "Maelstrom_Globals.h"
#include "main.h"
#include "load.h"
#include "controls.h"
#include "netlogic/about.h"
#include "netlogic/game.h"
#include "netlogic/lobby.h"
#include "MacDialog.h"
#include "screenlib/UIElementButton.h"
#include "screenlib/UIElementCheckbox.h"
#include "screenlib/UIElementEditbox.h"
#include "screenlib/UIElementRadio.h"
#include "screenlib/UIDialogButton.h"
#include "utils/hashtable.h"


static void
hash_nuke_string_font(const void *key, const void *value, void *data)
{
	SDL_free((char*)key);
	fontserv->FreeFont((MFont *)value);
}

static void
hash_nuke_string_text(const void *key, const void *value, void *data)
{
	SDL_free((char*)key);
	fontserv->FreeText((SDL_Texture *)value);
}

MaelstromUI::MaelstromUI(FrameBuf *screen, Prefs *prefs) : UIManager(screen, prefs)
{
	/* Create our font hashtables */
	m_fonts = hash_create(screen, hash_hash_string, hash_keymatch_string, hash_nuke_string_font);
	m_strings = hash_create(screen, hash_hash_string, hash_keymatch_string, hash_nuke_string_text);

	/* Load up our UI templates */
	SetLoadPath("UI");
	LoadTemplates("UITemplates.xml");
}

MaelstromUI::~MaelstromUI()
{
	hash_destroy(m_fonts);
	hash_destroy(m_strings);
}

MFont *
MaelstromUI::GetFont(const char *fontName, int fontSize)
{
	char *key;
	int keysize;
	MFont *font;

	keysize = strlen(fontName)+1+2+1;
	key = SDL_stack_alloc(char, keysize);
	sprintf(key, "%s:%d", fontName, fontSize);
	if (hash_find(m_fonts, key, (const void**)&font)) {
		SDL_stack_free(key);
		return font;
	}

	font = fontserv->NewFont(fontName, fontSize);
	if (font) {
		/* Add it to our cache */
		hash_insert(m_fonts, SDL_strdup(key), font);
	}
	SDL_stack_free(key);

	return font;
}

SDL_Texture *
MaelstromUI::CreateText(const char *text, const char *fontName, int fontSize, UIFontStyle fontStyle, Uint32 color)
{
	MFont *font;
	Uint8 style;
	char *key;
	int keysize;
	SDL_Texture *texture;

	/* First see if we can find it in our cache */
	keysize = strlen(fontName)+1+2+1+1+1+8+1+strlen(text)+1;
	key = SDL_stack_alloc(char, keysize);
	sprintf(key, "%s:%d:%c:%8.8x:%s", fontName, fontSize, '0'+fontStyle, color, text);
	if (hash_find(m_strings, key, (const void**)&texture)) {
		SDL_stack_free(key);
		return texture;
	}

	font = GetFont(fontName, fontSize);
	if (!font) {
		error("Couldn't find font %s size %d\n", fontName, fontSize);
		return NULL;
	}

	switch (fontStyle) {
		case UIFONT_STYLE_NORMAL:
			style = STYLE_NORM;
			break;
		case UIFONT_STYLE_BOLD:
			style = STYLE_BOLD;
			break;
		case UIFONT_STYLE_UNDERLINE:
			style = STYLE_ULINE;
			break;
		case UIFONT_STYLE_ITALIC:
			style = STYLE_ITALIC;
			break;
	}

	texture = fontserv->TextImage(text, font, style, color);
	if (texture) {
		/* Add it to our cache */
		hash_insert(m_strings, SDL_strdup(key), texture);
	}
	SDL_stack_free(key);

	return texture;
}

void 
MaelstromUI::FreeText(SDL_Texture *texture)
{
	/* We'll likely be asked for this again soon, leave it alone */
	return;
}

void
MaelstromUI::PlaySound(int soundID)
{
	sound->PlaySound(soundID, 5);
}

UIPanel *
MaelstromUI::CreatePanel(const char *type, const char *name)
{
	if (strcasecmp(type, "Panel") == 0) {
		return new UIPanel(this, name);
	} else if (strcasecmp(type, "Dialog") == 0) {
		return new MacDialog(ui, name);
	}
	return UIManager::CreatePanel(type, name);
}

UIPanelDelegate *
MaelstromUI::CreatePanelDelegate(UIPanel *panel, const char *delegate)
{
	if (strcasecmp(delegate, "MainPanel") == 0) {
		return new MainPanelDelegate(panel);
	} else if (strcasecmp(delegate, "AboutPanel") == 0) {
		return new AboutPanelDelegate(panel);
	} else if (strcasecmp(delegate, "LobbyDialog") == 0) {
		return new LobbyDialogDelegate(panel);
	} else if (strcasecmp(delegate, "GamePanel") == 0) {
		return new GamePanelDelegate(panel);
	} else if (strcasecmp(delegate, "ControlsDialog") == 0) {
		return new ControlsDialogDelegate(panel);
	}
	return UIManager::CreatePanelDelegate(panel, delegate);
}

UIElement *
MaelstromUI::CreateElement(UIBaseElement *parent, const char *type, const char *name)
{
	UIElement *element;

	if (strcasecmp(type, "Area") == 0) {
		element = new UIElement(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "Line") == 0) {
		element = new UIElement(parent, name, new UIDrawEngineLine());
	} else if (strcasecmp(type, "Rectangle") == 0) {
		element = new UIElement(parent, name, new UIDrawEngineRect());
	} else if (strcasecmp(type, "Label") == 0) {
		element = new UIElement(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "Image") == 0) {
		element = new UIElement(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "Button") == 0) {
		element = new UIElementButton(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "Icon") == 0) {
		element = new UIElement(parent, name, new UIDrawEngineIcon());
	} else if (strcasecmp(type, "Sprite") == 0) {
		element = new UIElement(parent, name, new UIDrawEngineSprite());
	} else if (strcasecmp(type, "Title") == 0) {
		element = new UIElement(parent, name, new UIDrawEngineTitle());
	} else if (strcasecmp(type, "ControlButton") == 0) {
		element = new UIElementControlButton(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "DialogLabel") == 0) {
		element = new UIElement(parent, name, new MacDialogDrawEngine());
	} else if (strcasecmp(type, "DialogButton") == 0) {
		element = new UIDialogButton(parent, name, new MacDialogButton());
	} else if (strcasecmp(type, "DialogCheckbox") == 0) {
		element = new UIElementCheckbox(parent, name, new MacDialogCheckbox());
	} else if (strcasecmp(type, "DialogRadioGroup") == 0) {
		element = new UIElementRadioGroup(parent, name, new UIDrawEngine());
	} else if (strcasecmp(type, "DialogRadioButton") == 0) {
		element = new UIElementRadioButton(parent, name, new MacDialogRadioButton());
	} else if (strcasecmp(type, "DialogEditbox") == 0) {
		element = new UIElementEditbox(parent, name, new MacDialogEditbox());
	} else {
		element = UIManager::CreateElement(parent, name, type);
	}
	return element;
}

//////////////////////////////////////////////////////////////////////////////
UIElementControlButton::UIElementControlButton(UIBaseElement *parent, const char *name, UIDrawEngine *drawEngine) :
	UIElement(parent, name, drawEngine)
{
	m_mouseEnabled = true;
}

bool
UIElementControlButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("action", 0, false);
	if (!attr) {
		error("Element '%s' missing attribute 'action'", node->name());
		return false;
	}

	if (SDL_strcasecmp(attr->value(), "THRUST") == 0) {
		m_control = THRUST_KEY;
	} else if (SDL_strcasecmp(attr->value(), "RIGHT") == 0) {
		m_control = RIGHT_KEY;
	} else if (SDL_strcasecmp(attr->value(), "LEFT") == 0) {
		m_control = LEFT_KEY;
	} else if (SDL_strcasecmp(attr->value(), "SHIELD") == 0) {
		m_control = SHIELD_KEY;
	} else if (SDL_strcasecmp(attr->value(), "FIRE") == 0) {
		m_control = FIRE_KEY;
	} else if (SDL_strcasecmp(attr->value(), "PAUSE") == 0) {
		m_control = PAUSE_KEY;
	} else if (SDL_strcasecmp(attr->value(), "ABORT") == 0) {
		m_control = ABORT_KEY;
	} else {
		error("Element '%s' has unknown action '%s'", node->name(), attr->value());
		return false;
	}

	return true;
}

void
UIElementControlButton::OnMouseDown()
{
	SetControl(m_control, 1);
}

void
UIElementControlButton::OnMouseUp()
{
	SetControl(m_control, 0);
}

//////////////////////////////////////////////////////////////////////////////
bool
UIDrawEngineIcon::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIDrawEngine::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("id", 0, false);
	if (!attr) {
		error("Element '%s' missing attribute 'id'", node->name());
		return false;
	}

	SDL_Texture *image = GetCIcon(m_screen, atoi(attr->value()));
	if (!image) {
		error("Unable to load icon %d", atoi(attr->value()));
		return false;
	}
	m_element->SetImage(image);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool
UIDrawEngineSprite::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;
	int baseID;
	Mac_ResData *S, *M;

	if (!UIDrawEngine::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("id", 0, false);
	if (!attr) {
		error("Element '%s' missing attribute 'id'", node->name());
		return false;
	}
	baseID = SDL_atoi(attr->value());

	/* Load the image */
	SDL_Texture *image = NULL;
	if ((S = spriteres->Resource("icl8", baseID)) != NULL &&
	    (M = spriteres->Resource("ICN#", baseID)) != NULL) {
		image = m_screen->LoadImage(32, 32, S->data, M->data+128);
	}
	if (!image) {
		error("Unable to load sprite %d", baseID);
		return false;
	}
	m_element->SetImage(image);

	return true;
}

//////////////////////////////////////////////////////////////////////////////
bool
UIDrawEngineTitle::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIDrawEngine::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("id", 0, false);
	if (!attr) {
		error("Element '%s' missing attribute 'id'", node->name());
		return false;
	}

	SDL_Texture *image = Load_Title(m_screen, SDL_atoi(attr->value()));
	if (!image) {
		error("Unable to load icon %d", SDL_atoi(attr->value()));
		return false;
	}
	m_element->SetImage(image);

	return true;
}
