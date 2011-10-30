#ifndef _UIElementSprite_h
#define _UIElementSprite_h

#include "screenlib/UIElementTexture.h"


class UIElementSprite : public UIElementTexture
{
DECLARE_TYPESAFE_CLASS(UIElementTexture)
public:
	UIElementSprite(UIBaseElement *parent, const char *name = "");

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

#endif // _UIElementSprite_h
