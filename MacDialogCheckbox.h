#ifndef _MacDialogCheckbox_h
#define _MacDialogCheckbox_h

#include "screenlib/UIElementCheckbox.h"


class MacDialogCheckbox : public UIElementCheckbox
{
public:
	MacDialogCheckbox(UIBaseElement *parent, const char *name = "");
	virtual ~MacDialogCheckbox();

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

#endif // _MacDialogCheckbox_h
