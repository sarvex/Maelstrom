#ifndef _MacDialogEditbox_h
#define _MacDialogEditbox_h

#include "screenlib/UIElementEditbox.h"


class MacDialogEditbox : public UIElementEditbox
{
DECLARE_TYPESAFE_CLASS(UIElementEditbox)
public:
	MacDialogEditbox(UIBaseElement *parent, const char *name = "");

	override void Draw();

protected:
	override UIElementLabel *CreateLabel();
	override void OnHighlightChanged();

	void SetElementColor(Uint32 color);

protected:
	Uint32 m_colors[2];
};

#endif // _MacDialogEditbox_h
