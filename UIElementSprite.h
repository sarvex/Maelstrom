#ifndef _UIElementSprite_h
#define _UIElementSprite_h

#include "screenlib/UIElementTexture.h"


class UIElementSprite : public UIElementTexture
{
public:
	UIElementSprite(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElementTexture::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

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

#endif // _UIElementSprite_h
