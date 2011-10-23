#ifndef _UIElementLabel_h
#define _UIElementLabel_h

#include "screenlib/UIElement.h"

class MFont;

class UIElementLabel : public UIElement
{
public:
	UIElementLabel(UIPanel *panel, const char *name = "");
	virtual ~UIElementLabel();

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual void SetText(const char *text);

	virtual void Draw();

protected:
	MFont *m_font;
	Uint8 m_style;
	Uint32 m_color;
	SDL_Texture *m_texture;
};

#endif // _UIElementLabel_h
