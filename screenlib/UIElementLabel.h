#ifndef _UIElementLabel_h
#define _UIElementLabel_h

#include "UIElement.h"
#include "UIFontInterface.h"


class UIElementLabel : public UIElement
{
public:
	UIElementLabel(UIPanel *panel, const char *name = "");
	virtual ~UIElementLabel();

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	void SetText(const char *text);
	void SetTextColor(Uint8 R, Uint8 G, Uint8 B);

	virtual void Draw();

protected:
	char *m_fontName;
	int m_fontSize;
	UIFontStyle m_fontStyle;
	Uint32 m_color;
	char *m_text;
	SDL_Texture *m_texture;

protected:
	static UIElementType s_elementType;

public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _UIElementLabel_h
