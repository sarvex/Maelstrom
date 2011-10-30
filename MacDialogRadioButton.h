#ifndef _MacDialogRadioButton_h
#define _MacDialogRadioButton_h

#include "screenlib/UIElementRadio.h"


class MacDialogRadioButton : public UIElementRadioButton
{
DECLARE_TYPESAFE_CLASS(UIElementRadioButton)
public:
	MacDialogRadioButton(UIBaseElement *parent, const char *name = "");

	override bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	override void Draw();

protected:
	Uint32 m_color;
};

#endif // _MacDialogRadioButton_h
