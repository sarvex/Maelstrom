#ifndef _MacDialogRadioButton_h
#define _MacDialogRadioButton_h

#include "screenlib/UIElementRadio.h"


class MacDialogRadioButton : public UIElementRadioButton
{
public:
	MacDialogRadioButton(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElementRadioButton::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

protected:
	Uint32 m_color;

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

#endif // _MacDialogRadioButton_h
