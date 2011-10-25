#ifndef _UIElementKeyButton_h
#define _UIElementKeyButton_h

#include "screenlib/UIElementButton.h"


class UIElementKeyButton : public UIElementButton
{
public:
	UIElementKeyButton(UIPanel *panel, const char *name = "");
	virtual ~UIElementKeyButton();

	virtual bool IsA(UIElementType type) {
		return UIElementButton::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual void Draw();

protected:
	SDL_Texture *m_text;

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

#endif // _UIElementKeyButton_h
