#ifndef _UIDialogCheckbox_h
#define _UIDialogCheckbox_h

#include "screenlib/UIElementCheckbox.h"


class UIDialogCheckbox : public UIElementCheckbox
{
public:
	UIDialogCheckbox(UIBaseElement *parent, const char *name = "");
	virtual ~UIDialogCheckbox();

	virtual bool IsA(UIElementType type) {
		return UIElementCheckbox::IsA(type) || type == GetType();
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

#endif // _UIDialogCheckbox_h
