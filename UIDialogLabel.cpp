
#include "UIDialogLabel.h"
#include "Maelstrom_Globals.h"

UIElementType UIDialogLabel::s_elementType;


UIDialogLabel::UIDialogLabel(UIPanel *panel, const char *name) :
	UIElementLabel(panel, name)
{
	m_font = fonts[CHICAGO_12];
	m_color = m_screen->MapRGB(0x00, 0x00, 0x00);
}
