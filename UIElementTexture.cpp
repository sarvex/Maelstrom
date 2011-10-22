
#include "UIElementTexture.h"
#include "screenlib/SDL_FrameBuf.h"


UIElementTexture::UIElementTexture(UIPanel *panel, const char *name) :
	UIElement(panel, name)
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
	m_texture = texture;
	m_rect.w = m_screen->GetImageWidth(texture);
	m_rect.h = m_screen->GetImageHeight(texture);
}

void
UIElementTexture::Draw()
{
	if (m_texture) {
		m_screen->QueueBlit(m_rect.x, m_rect.y, m_texture, NOCLIP);
	}
}
