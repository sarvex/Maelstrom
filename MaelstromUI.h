
#include "screenlib/UIManager.h"

class HashTable;

class MaelstromUI : public UIManager
{
public:
	MaelstromUI(FrameBuf *screen);
	virtual ~MaelstromUI();

	//
	// UIFontInterface
	//
	virtual SDL_Texture *CreateText(const char *text, const char *fontName, int fontSize, UIFontStyle fontStyle);
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
	virtual UIElement *CreateElement(UIPanel *panel, const char *type);

protected:
	HashTable *m_fonts;
};
