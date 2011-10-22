
#include "UIElements.h"
#include "UIElementTitle.h"


UIElement *
CreateMaelstromUIElement(UIPanel *panel, const char *name)
{
	if (strcasecmp(name, "Title") == 0) {
		return new UIElementTitle(panel);
	}
	return NULL;
}
