
#include "MaelstromUI.h"
#include "Maelstrom_Globals.h"
#include "UIDialog.h"
#include "main.h"
#include "netlogic/about.h"
#include "netlogic/game.h"
#include "utils/hashtable.h"
#include "UIDialogLabel.h"
#include "UIElementIcon.h"
#include "UIElementKeyButton.h"
#include "UIElementLabel.h"
#include "UIElementSprite.h"
#include "UIElementTitle.h"


MaelstromUI::MaelstromUI(FrameBuf *screen) : UIManager(screen)
{
	/* Load up our UI templates */
	SetLoadPath("UI");
	LoadTemplates("UITemplates.xml");
}

MaelstromUI::~MaelstromUI()
{
}

SDL_Texture *
MaelstromUI::CreateText(const char *text, const char *fontName, int fontSize, UIFontStyle fontStyle)
{
}

void 
MaelstromUI::FreeText(SDL_Texture *texture)
{
	fontserv->FreeText(texture);
}

void
MaelstromUI::PlaySound(int soundID)
{
	sound->PlaySound(soundID, 5);
}

UIPanel *
MaelstromUI::CreatePanel(const char *type, const char *name)
{
	if (strcasecmp(type, "Dialog") == 0) {
		return new UIDialog(ui, name);
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
MaelstromUI::CreateElement(UIPanel *panel, const char *type)
{
	if (strcasecmp(type, "Label") == 0) {
		return new UIElementLabel(panel);
	} else if (strcasecmp(type, "DialogLabel") == 0) {
		return new UIDialogLabel(panel);
	} else if (strcasecmp(type, "KeyButton") == 0) {
		return new UIElementKeyButton(panel);
	} else if (strcasecmp(type, "Icon") == 0) {
		return new UIElementIcon(panel);
	} else if (strcasecmp(type, "Sprite") == 0) {
		return new UIElementSprite(panel);
	} else if (strcasecmp(type, "Title") == 0) {
		return new UIElementTitle(panel);
	}
	return UIManager::CreateElement(panel, type);;
}
