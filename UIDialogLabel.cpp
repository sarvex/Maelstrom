
#include "UIDialogLabel.h"
#include "Maelstrom_Globals.h"

UIElementType UIDialogLabel::s_elementType;


UIDialogLabel::UIDialogLabel(UIBaseElement *parent, const char *name) :
	UIElementLabel(parent, name)
{
	m_fontName = SDL_strdup("Chicago");
	m_fontSize = 12;
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);
}
