
#include "UIElementKeyButton.h"
#include "Maelstrom_Globals.h"

UIElementType UIElementKeyButton::s_elementType;

UIElementKeyButton::UIElementKeyButton(UIPanel *panel, const char *name) :
	UIElementButton(panel, name)
{
	m_text = NULL;
}

UIElementKeyButton::~UIElementKeyButton()
{
	if (m_text) {
		fontserv->FreeText(m_text);
	}
}

bool
UIElementKeyButton::Load(rapidxml::xml_node<> *node)
{
	m_rect.w = m_screen->GetImageWidth(gKeyIcon);
	m_rect.h = m_screen->GetImageHeight(gKeyIcon);

	if (!UIElementButton::Load(node)) {
		return false;
	}

	if (m_hotkey != SDLK_UNKNOWN) {
		m_text = fontserv->TextImage(SDL_GetKeyName(m_hotkey),
				fonts[GENEVA_9], STYLE_BOLD, 0xFF, 0xFF, 0xFF);
	}
}

void
UIElementKeyButton::Draw()
{
	m_screen->QueueBlit(m_rect.x, m_rect.y, gKeyIcon, NOCLIP);
#ifdef UI_DEBUG
printf("KeyButton: %s at %d,%d\n", SDL_GetKeyName(m_hotkey), m_rect.x+14, m_rect.y+10);
printf("KeyButton: %s at %d,%d\n", SDL_GetKeyName(m_hotkey), m_rect.x+13, m_rect.y+9);
#endif
	SDL_SetTextureColorMod(m_text, 0xFF, 0xFF, 0xFF);
	m_screen->QueueBlit(m_rect.x+14, m_rect.y+10, m_text, NOCLIP);
	SDL_SetTextureColorMod(m_text, 0x00, 0x00, 0x00);
	m_screen->QueueBlit(m_rect.x+13, m_rect.y+9, m_text, NOCLIP);
}
