#ifndef _UIElementRect_h
#define _UIElementRect_h

#include "screenlib/UIElement.h"


class UIElementRect : public UIElement
{
public:
	UIElementRect(UIPanel *panel, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node);

	virtual void Draw();

private:
	bool m_fill;
	Uint32 m_color;
};

#endif // _UIElementRect_h
