
#include "screenlib/UIPanel.h"
#include "UIPanels.h"
#include "netlogic/about.h"


static UIPanelDelegate *
CreateMaelstromUIDelegate(const char *delegate)
{
	if (strcasecmp(delegate, "AboutPanel") == 0) {
		return new AboutPanelDelegate();
	} else {
		fprintf(stderr, "Warning: Couldn't find delegate '%s'\n", delegate);
		return NULL;
	}
}

UIPanel *
CreateMaelstromUIPanel(UIManager *ui, const char *type, const char *name, const char *delegate)
{
	UIPanel *panel;

	if (strcasecmp(type, "UIPanel") == 0) {
		panel = new UIPanel(ui, name);
	} else {
		panel = NULL;
	}

	if (panel && delegate && *delegate) {
		panel->SetPanelDelegate(CreateMaelstromUIDelegate(delegate));
	}

	return panel;
}
