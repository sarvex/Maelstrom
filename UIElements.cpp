
#include "UIElements.h"
#include "screenlib/UIElementLine.h"
#include "screenlib/UIElementRect.h"
#include "UIElementLabel.h"
#include "UIElementTitle.h"


UIElement *
CreateMaelstromUIElement(UIPanel *panel, const char *name)
{
	if (strcasecmp(name, "Line") == 0) {
		return new UIElementLine(panel);
	} else if (strcasecmp(name, "Rectangle") == 0) {
		return new UIElementRect(panel);
	} else if (strcasecmp(name, "Label") == 0) {
		return new UIElementLabel(panel);
	} else if (strcasecmp(name, "Title") == 0) {
		return new UIElementTitle(panel);
	}
	return NULL;
}
