#ifndef _UIElementLine_h
#define _UIElementLine_h

#include "screenlib/UIElement.h"


class UIElementLine : public UIElement
{
public:
	UIElementLine(UIPanel *panel, const char *name = "");

	virtual bool IsA(UIElementType type) {
		return UIElement::IsA(type) || type == GetType();
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Draw();

private:
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

#endif // _UIElementLine_h
