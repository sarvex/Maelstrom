
#include "screenlib/UIPanel.h"
#include "UIPanels.h"
#include "main.h"
#include "netlogic/about.h"
#include "netlogic/game.h"


static UIPanelDelegate *
CreateMaelstromUIDelegate(UIPanel *panel, const char *delegate)
{
	if (strcasecmp(delegate, "MainPanel") == 0) {
		return new MainPanelDelegate(panel);
	} else if (strcasecmp(delegate, "AboutPanel") == 0) {
		return new AboutPanelDelegate(panel);
	} else if (strcasecmp(delegate, "GamePanel") == 0) {
		return new GamePanelDelegate(panel);
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
		panel->SetPanelDelegate(CreateMaelstromUIDelegate(panel, delegate));
	}

	return panel;
}
