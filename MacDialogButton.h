#ifndef _MacDialogButton_h
#define _MacDialogButton_h

#include "screenlib/UIDialogButton.h"


class MacDialogButton : public UIDialogButton
{
DECLARE_TYPESAFE_CLASS(UIDialogButton)
public:
	MacDialogButton(UIBaseElement *parent, const char *name = "");

	override void Draw();

	override void OnMouseDown();
	override void OnMouseUp();

protected:
	override UIElementLabel *CreateLabel();

	void SetElementColor(Uint32 color);

protected:
	Uint32 m_colors[2];
};

#endif // _MacDialogButton_h
