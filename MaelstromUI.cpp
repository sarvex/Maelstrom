
#include "MaelstromUI.h"
#include "Maelstrom_Globals.h"
#include "main.h"
#include "netlogic/about.h"
#include "netlogic/game.h"
#include "MacDialog.h"
#include "MacDialogButton.h"
#include "MacDialogCheckbox.h"
#include "MacDialogLabel.h"
#include "UIElementIcon.h"
#include "UIElementKeyButton.h"
#include "UIElementSprite.h"
#include "UIElementTitle.h"
#include "screenlib/UIElementButton.h"
#include "screenlib/UIElementCheckbox.h"
#include "screenlib/UIElementLine.h"
#include "screenlib/UIElementRadio.h"
#include "screenlib/UIElementRect.h"
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

MaelstromUI::MaelstromUI(FrameBuf *screen) : UIManager(screen)
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
	char *key, *keycopy;
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
	char *key, *keycopy;
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
		SetError("Couldn't find font %s size %d", fontName, fontSize);
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
	} else if (strcasecmp(delegate, "GamePanel") == 0) {
		return new GamePanelDelegate(panel);
	}
	return UIManager::CreatePanelDelegate(panel, delegate);
}

UIElement *
MaelstromUI::CreateElement(UIBaseElement *parent, const char *type, const char *name)
{
	if (strcasecmp(type, "Line") == 0) {
		return new UIElementLine(parent, name);
	} else if (strcasecmp(type, "Rectangle") == 0) {
		return new UIElementRect(parent, name);
	} else if (strcasecmp(type, "Label") == 0) {
		return new UIElementLabel(parent, name);
	} else if (strcasecmp(type, "Button") == 0) {
		return new UIElementButton(parent, name);
	} else if (strcasecmp(type, "DialogLabel") == 0) {
		return new MacDialogLabel(parent, name);
	} else if (strcasecmp(type, "DialogButton") == 0) {
		return new MacDialogButton(parent, name);
	} else if (strcasecmp(type, "DialogCheckbox") == 0) {
		return new MacDialogCheckbox(parent, name);
	} else if (strcasecmp(type, "DialogRadioGroup") == 0) {
		return new UIElementRadioGroup(parent, name);
	} else if (strcasecmp(type, "DialogRadioButton") == 0) {
		return new UIElementRadioButton(parent, name);
	} else if (strcasecmp(type, "KeyButton") == 0) {
		return new UIElementKeyButton(parent, name);
	} else if (strcasecmp(type, "Icon") == 0) {
		return new UIElementIcon(parent, name);
	} else if (strcasecmp(type, "Sprite") == 0) {
		return new UIElementSprite(parent, name);
	} else if (strcasecmp(type, "Title") == 0) {
		return new UIElementTitle(parent, name);
	}
	return UIManager::CreateElement(parent, name, type);;
}
