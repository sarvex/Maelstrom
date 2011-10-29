#ifndef _UIDialogCheckbox_h
#define _UIDialogCheckbox_h

#include "screenlib/UIElementButton.h"


class UIDialogCheckbox : public UIElementButton
{
public:
	UIDialogCheckbox(UIBaseElement *parent, const char *name = "");
	virtual ~UIDialogCheckbox();

	virtual bool IsA(UIElementType type) {
		return UIElementButton::IsA(type) || type == GetType();
	}

	bool IsChecked() const {
		return m_checked;
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

	virtual void OnClick();

protected:
	Uint32 m_color;
	bool m_checked;

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
