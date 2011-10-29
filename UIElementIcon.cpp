
#include "UIElementIcon.h"
#include "load.h"

UIElementType UIElementIcon::s_elementType;


UIElementIcon::UIElementIcon(UIBaseElement *parent, const char *name) :
	UIElementTexture(parent, name)
{
}

bool
UIElementIcon::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIElementTexture::Load(node, templates)) {
		return false;
	}

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

	return true;
}
