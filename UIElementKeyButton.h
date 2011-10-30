#ifndef _UIElementKeyButton_h
#define _UIElementKeyButton_h

#include "screenlib/UIElementButton.h"


class UIElementKeyButton : public UIElementButton
{
DECLARE_TYPESAFE_CLASS(UIElementButton)
public:
	UIElementKeyButton(UIBaseElement *parent, const char *name = "");

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);
};

#endif // _UIElementKeyButton_h
