#ifndef _MacDialogButton_h
#define _MacDialogButton_h

#include "screenlib/UIDialogButton.h"


class MacDialogButton : public UIDialogButton
{
public:
	MacDialogButton(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIDialogButton::IsA(type) || type == GetType();
	}

	virtual void Draw();

	virtual void OnMouseDown();
	virtual void OnMouseUp();

protected:
	Uint32 m_colors[2];

protected:
	void SetElementColor(Uint32 color);

protected:
	static UIElementType s_elementType;

public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _MacDialogButton_h
