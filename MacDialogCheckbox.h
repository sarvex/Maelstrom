#ifndef _MacDialogCheckbox_h
#define _MacDialogCheckbox_h

#include "screenlib/UIElementCheckbox.h"


class MacDialogCheckbox : public UIElementCheckbox
{
DECLARE_TYPESAFE_CLASS(UIElementCheckbox)
public:
	MacDialogCheckbox(UIBaseElement *parent, const char *name = "");

	override void Draw();

protected:
	override UIElementLabel *CreateLabel();

protected:
	Uint32 m_color;
};

#endif // _MacDialogCheckbox_h
