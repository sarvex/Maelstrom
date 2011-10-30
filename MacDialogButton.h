#ifndef _MacDialogButton_h
#define _MacDialogButton_h

#include "screenlib/UIDialogButton.h"


class MacDialogButton : public UIDialogButton
{
DECLARE_TYPESAFE_CLASS(UIDialogButton)
public:
	MacDialogButton(UIBaseElement *parent, const char *name = "");

	virtual void Draw();

	virtual void OnMouseDown();
	virtual void OnMouseUp();

protected:
	Uint32 m_colors[2];

protected:
	void SetElementColor(Uint32 color);
};

#endif // _MacDialogButton_h
