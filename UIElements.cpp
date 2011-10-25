
#include "UIElements.h"
#include "screenlib/UIElementButton.h"
#include "screenlib/UIElementLine.h"
#include "screenlib/UIElementRect.h"
#include "UIElementIcon.h"
#include "UIElementKeyButton.h"
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
	} else if (strcasecmp(name, "Button") == 0) {
		return new UIElementButton(panel);
	} else if (strcasecmp(name, "KeyButton") == 0) {
		return new UIElementKeyButton(panel);
	} else if (strcasecmp(name, "Icon") == 0) {
		return new UIElementIcon(panel);
	} else if (strcasecmp(name, "Title") == 0) {
		return new UIElementTitle(panel);
	}
	return NULL;
}
