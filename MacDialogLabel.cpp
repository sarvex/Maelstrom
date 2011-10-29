
#include "MacDialogLabel.h"
#include "Maelstrom_Globals.h"

UIElementType MacDialogLabel::s_elementType;


MacDialogLabel::MacDialogLabel(UIBaseElement *parent, const char *name) :
	UIElementLabel(parent, name)
{
	m_fontName = SDL_strdup("Chicago");
	m_fontSize = 12;
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);
}
