#ifndef _UIElementCheckbox_h
#define _UIElementCheckbox_h

#include "UIElementButton.h"


class UIElementCheckbox : public UIElementButton
{
public:
	UIElementCheckbox(UIBaseElement *parent, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElementButton::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	void SetChecked(bool checked) {
		if (checked != m_checked) {
			m_checked = checked;
			OnChecked(checked);
		}
	}
	bool IsChecked() const {
		return m_checked;
	}
	virtual void OnChecked(bool checked) { }

	virtual void OnClick();

protected:
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

#endif // _UIElementCheckbox_h
