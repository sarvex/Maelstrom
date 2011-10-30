#ifndef _MacDialogCheckbox_h
#define _MacDialogCheckbox_h

#include "screenlib/UIElementCheckbox.h"


class MacDialogCheckbox : public UIElementCheckbox
{
DECLARE_TYPESAFE_CLASS(UIElementCheckbox)
public:
	MacDialogCheckbox(UIBaseElement *parent, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

protected:
	Uint32 m_color;
};

#endif // _MacDialogCheckbox_h
