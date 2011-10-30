#ifndef _UIElementIcon_h
#define _UIElementIcon_h

#include "screenlib/UIElementTexture.h"


class UIElementIcon : public UIElementTexture
{
DECLARE_TYPESAFE_CLASS(UIElementTexture)
public:
	UIElementIcon(UIBaseElement *parent, const char *name = "");

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

#endif // _UIElementIcon_h
