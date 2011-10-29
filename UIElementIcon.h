#ifndef _UIElementIcon_h
#define _UIElementIcon_h

#include "screenlib/UIElementTexture.h"


class UIElementIcon : public UIElementTexture
{
public:
	UIElementIcon(UIBaseElement *parent, const char *name = "");

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

#endif // _UIElementIcon_h
