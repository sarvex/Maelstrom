
#include "SDL_FrameBuf.h"
#include "UIElementLine.h"

UIElementType UIElementLine::s_elementType;


UIElementLine::UIElementLine(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
}

bool
UIElementLine::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	LoadColor(node, "color", m_color);

	return true;
}

void
UIElementLine::Draw()
{
	m_screen->DrawLine(X(), Y(), X()+Width()-1, Y()+Height()-1, m_color);
}
