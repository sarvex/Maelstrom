
#include "SDL_FrameBuf.h"
#include "UIElementLine.h"

UIElementLine::UIElementLine(UIPanel *panel, const char *name) :
	UIElement(panel, name)
{
	m_color = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
}

bool
UIElementLine::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *child;

	child = node->first_node("color", 0, false);
	if (child) {
		m_color = LoadColor(child);
	}

	return UIElement::Load(node);
}

void
UIElementLine::Draw()
{
	m_screen->DrawLine(m_rect.x, m_rect.y, m_rect.x+m_rect.w, m_rect.y+m_rect.h, m_color);
}
