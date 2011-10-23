#ifndef _UIElementTitle_h
#define _UIElementTitle_h

#include "screenlib/UIElementTexture.h"


class UIElementTitle : public UIElementTexture
{
public:
	UIElementTitle(UIPanel *panel, const char *name = "");

	virtual bool Load(rapidxml::xml_node<> *node);
};

#endif // _UIElementTitle_h
