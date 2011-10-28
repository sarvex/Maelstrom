#ifndef _UIElementTitle_h
#define _UIElementTitle_h

#include "screenlib/UIElementTexture.h"


class UIElementTitle : public UIElementTexture
{
public:
	UIElementTitle(UIPanel *panel, const char *name = "");

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

#endif // _UIElementTitle_h
