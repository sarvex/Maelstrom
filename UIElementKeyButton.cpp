
#include "UIElementKeyButton.h"
#include "Maelstrom_Globals.h"

UIElementType UIElementKeyButton::s_elementType;

UIElementKeyButton::UIElementKeyButton(UIBaseElement *parent, const char *name) :
	UIElementButton(parent, name)
{
	m_text = NULL;
	m_textShadow = NULL;
}

UIElementKeyButton::~UIElementKeyButton()
{
	if (m_text) {
		fontserv->FreeText(m_text);
	}
	if (m_textShadow) {
		fontserv->FreeText(m_textShadow);
	}
}

bool
UIElementKeyButton::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	m_rect.w = m_screen->GetImageWidth(gKeyIcon);
	m_rect.h = m_screen->GetImageHeight(gKeyIcon);

	if (!UIElementButton::Load(node, templates)) {
		return false;
	}

	if (m_hotkey != SDLK_UNKNOWN) {
		m_text = fontserv->TextImage(SDL_GetKeyName(m_hotkey),
				fonts[GENEVA_9], STYLE_BOLD, 0x00, 0x00, 0x00);
		m_textShadow = fontserv->TextImage(SDL_GetKeyName(m_hotkey),
				fonts[GENEVA_9], STYLE_BOLD, 0xFF, 0xFF, 0xFF);
	}
	return true;
}

void
UIElementKeyButton::Draw()
{
	m_screen->QueueBlit(m_rect.x, m_rect.y, gKeyIcon, NOCLIP);
	m_screen->QueueBlit(m_rect.x+14, m_rect.y+10, m_textShadow, NOCLIP);
	m_screen->QueueBlit(m_rect.x+13, m_rect.y+9, m_text, NOCLIP);
}
