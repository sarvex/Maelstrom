
#include "UIElements.h"
#include "screenlib/UIElementButton.h"
#include "screenlib/UIElementLine.h"
#include "screenlib/UIElementRect.h"
#include "UIElementIcon.h"
#include "UIElementKeyButton.h"
#include "UIElementLabel.h"
#include "UIElementTitle.h"


UIElement *
CreateMaelstromUIElement(UIPanel *panel, const char *type)
{
	if (strcasecmp(type, "Line") == 0) {
		return new UIElementLine(panel);
	} else if (strcasecmp(type, "Rectangle") == 0) {
		return new UIElementRect(panel);
	} else if (strcasecmp(type, "Label") == 0) {
		return new UIElementLabel(panel);
	} else if (strcasecmp(type, "Button") == 0) {
		return new UIElementButton(panel);
	} else if (strcasecmp(type, "KeyButton") == 0) {
		return new UIElementKeyButton(panel);
	} else if (strcasecmp(type, "Icon") == 0) {
		return new UIElementIcon(panel);
	} else if (strcasecmp(type, "Title") == 0) {
		return new UIElementTitle(panel);
	}
	return NULL;
}
