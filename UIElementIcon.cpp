
#include "UIElementIcon.h"
#include "load.h"

UIElementType UIElementIcon::s_elementType;


UIElementIcon::UIElementIcon(UIPanel *panel, const char *name) :
	UIElementTexture(panel, name)
{
}

bool
UIElementIcon::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute("id", 0, false);
	if (!attr) {
		SetError("Element '%s' missing attribute 'id'", node->name());
		return false;
	}

	SDL_Texture *texture = GetCIcon(m_screen, atoi(attr->value()));
	if (!texture) {
		SetError("Unable to load icon %d", atoi(attr->value()));
		return false;
	}
	SetTexture(texture);

	return UIElementTexture::Load(node);
}
