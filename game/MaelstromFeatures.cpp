/*
    Maelstrom: Open Source version of the classic game by Ambrosia Software
    Copyright (C) 1997-2011  Sam Lantinga

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#include "Maelstrom_Globals.h"
#include "MaelstromFeatures.h"

#include "../screenlib/UIElementCheckbox.h"


void InitFeatures()
{
#ifdef __IPHONEOS__
	// You have to buy features on the iOS app
#else
	prefs->SetBool(FEATURE_KIDMODE, true);
	prefs->SetBool(FEATURE_NETWORK, true);
#endif
}

bool HasFeature(const char *feature)
{
	return prefs->GetBool(feature);
}

static const char *current_feature;

static void FeatureDialogDone(void*, UIDialog *dialog, int status)
{
	if (status == 1) {
		// Buy the feature for this platform
#if 0//def __IPHONEOS__
#error Implement Store Kit
#else
		CompleteFeatureDialog(current_feature, true);
#endif
	} else {
		CompleteFeatureDialog(current_feature, false);
	}
}

void ShowFeature(const char *feature)
{
	UIDialog *dialog;

	if (current_feature) {
		// Uh oh, somehow trying to show multiple features at once
		return;
	}

	dialog = ui->GetPanel<UIDialog>(DIALOG_FEATURE);
	if (dialog) {
		current_feature = feature;
		dialog->SetDialogDoneHandler(FeatureDialogDone);
		ui->ShowPanel(dialog);
	}
}

void CompleteFeatureDialog(const char *feature, bool success)
{
	UIPanel *panel;
	UIDialog *dialog;
	UIElementCheckbox *checkbox = NULL;

	if (success) {
		prefs->SetBool(feature, true);
		prefs->Save();

		if (SDL_strcmp(feature, FEATURE_KIDMODE) == 0) {
			// Enable kid mode
			panel = ui->GetPanel<UIPanel>(PANEL_MAIN);
			if (panel) {
				checkbox = panel->GetElement<UIElementCheckbox>("KidMode");
			}
			if (checkbox) {
				checkbox->SetChecked(true);
			}
		}
		if (SDL_strcmp(feature, FEATURE_NETWORK) == 0) {
			// Start multiplayer game
			dialog = ui->GetPanel<UIDialog>(DIALOG_LOBBY);
			if (dialog) {
				ui->ShowPanel(dialog);
			}
		}
	}
	current_feature = NULL;
}

