
#include "SDL_FrameBuf.h"
#include "UIElementRect.h"

UIElementType UIElementRect::s_elementType;


UIElementRect::UIElementRect(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_fill = false;
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
}

bool
UIElementRect::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;

	if (!UIElement::Load(node, templates)) {
		return false;
	}

	LoadBool(node, "fill", m_fill);
	LoadColor(node, "color", m_color);

	return true;
}

void
UIElementRect::Draw()
{
	if (m_fill) {
		m_screen->FillRect(X(), Y(), Width(), Height(), m_color);
	} else {
		m_screen->DrawRect(X(), Y(), Width(), Height(), m_color);
	}
}
