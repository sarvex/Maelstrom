
#include "SDL_FrameBuf.h"
#include "UIElementLine.h"

UIElementType UIElementLine::s_elementType;


UIElementLine::UIElementLine(UIPanel *panel, const char *name) :
	UIElement(panel, name)
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

	child = node->first_node("color", 0, false);
	if (child) {
		m_color = LoadColor(child);
	}

	return true;
}

void
UIElementLine::Draw()
{
	m_screen->DrawLine(m_rect.x, m_rect.y, m_rect.x+m_rect.w-1, m_rect.y+m_rect.h-1, m_color);
}
