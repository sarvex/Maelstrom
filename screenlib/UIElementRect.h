#ifndef _UIElementRect_h
#define _UIElementRect_h

#include "screenlib/UIElement.h"


class UIElementRect : public UIElement
{
public:
	UIElementRect(UIPanel *panel, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

	void SetColor(Uint32 color) {
		m_color = color;
	}

private:
	bool m_fill;
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

#endif // _UIElementRect_h
