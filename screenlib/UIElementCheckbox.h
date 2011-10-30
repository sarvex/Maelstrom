#ifndef _UIElementCheckbox_h
#define _UIElementCheckbox_h

#include "UIElementButton.h"


class UIElementCheckbox : public UIElementButton
{
DECLARE_TYPESAFE_CLASS(UIElementButton)
public:
	UIElementCheckbox(UIBaseElement *parent, const char *name = "");

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
};

#endif // _UIElementCheckbox_h
