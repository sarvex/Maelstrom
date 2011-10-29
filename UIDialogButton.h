#ifndef _UIDialogButton_h
#define _UIDialogButton_h

#include "screenlib/UIElementButton.h"


class UIDialogButton : public UIElementButton
{
public:
	UIDialogButton(UIBaseElement *parent, const char *name = "");
	virtual ~UIDialogButton();

	virtual bool IsA(UIElementType type) {
		return UIElementButton::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

	virtual void OnMouseDown();
	virtual void OnMouseUp();
	virtual void OnClick();

protected:
	Uint32 m_colors[2];
	int m_statusID;
	bool m_default;
	bool m_closeDialog;

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

#endif // _UIDialogButton_h
