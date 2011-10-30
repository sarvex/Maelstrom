#ifndef _UIElementTitle_h
#define _UIElementTitle_h

#include "screenlib/UIElementTexture.h"


class UIElementTitle : public UIElementTexture
{
DECLARE_TYPESAFE_CLASS(UIElementTexture)
public:
	UIElementTitle(UIBaseElement *parent, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

#endif // _UIElementTitle_h
