
#include "UIElementTexture.h"
#include "screenlib/SDL_FrameBuf.h"

UIElementType UIElementTexture::s_elementType;


UIElementTexture::UIElementTexture(UIBaseElement *parent, const char *name) :
	UIElement(parent, name)
{
	m_texture = NULL;
}

UIElementTexture::~UIElementTexture()
{
	if (m_texture) {
		m_screen->FreeImage(m_texture);
	}
}

void
UIElementTexture::SetTexture(SDL_Texture *texture)
{
	if (m_texture) {
		m_screen->FreeImage(m_texture);
	}
	m_texture = texture;
	SetSize(m_screen->GetImageWidth(texture),
		m_screen->GetImageHeight(texture));
}

void
UIElementTexture::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(X(), Y(), m_texture, NOCLIP);
	}
}
