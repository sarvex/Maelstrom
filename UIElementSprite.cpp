
#include "Maelstrom_Globals.h"
#include "UIElementSprite.h"

UIElementType UIElementSprite::s_elementType;


UIElementSprite::UIElementSprite(UIBaseElement *parent, const char *name) :
	UIElementTexture(parent, name)
{
}

static SDL_Texture *
LoadSprite(FrameBuf *screen, int baseID)
{
	Mac_ResData *S, *M;

	S = spriteres->Resource("icl8", baseID);
	if ( S == NULL ) {
		return NULL;
	}

	M = spriteres->Resource("ICN#", baseID);
	if ( M == NULL ) {
		return NULL;
	}

	/* Load the image */
	return screen->LoadImage(32, 32, S->data, M->data+128);
}

bool
UIElementSprite::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
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

	SDL_Texture *texture = LoadSprite(m_screen, atoi(attr->value()));
	if (!texture) {
		SetError("Unable to load sprite %d", atoi(attr->value()));
		return false;
	}
	SetTexture(texture);

	return true;
}
