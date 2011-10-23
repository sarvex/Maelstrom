#ifndef _UIElementLine_h
#define _UIElementLine_h

#include "screenlib/UIElement.h"


class UIElementLine : public UIElement
{
public:
	UIElementLine(UIPanel *panel, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual void Draw();

private:
	Uint32 m_color;
};

#endif // _UIElementLine_h
