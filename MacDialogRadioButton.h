#ifndef _MacDialogRadioButton_h
#define _MacDialogRadioButton_h

#include "screenlib/UIElementRadio.h"


class MacDialogRadioButton : public UIElementRadioButton
{
DECLARE_TYPESAFE_CLASS(UIElementRadioButton)
public:
	MacDialogRadioButton(UIBaseElement *parent, const char *name = "");

	override void Draw();

protected:
	override UIElementLabel *CreateLabel();

protected:
	Uint32 m_color;
};

#endif // _MacDialogRadioButton_h
