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

/* ------------------------------------------------------------- */
/* 								 */
/* Maelstrom							 */
/* By Andrew Welch						 */
/* 								 */
/* Ported to Linux  (Spring 1995)				 */
/* Ported to Win95  (Fall   1996) -- not releasable		 */
/* Ported to SDL    (Fall   1997)                                */
/* By Sam Lantinga  (slouken@libsdl.org)			 */
/* 								 */
/* ------------------------------------------------------------- */

#include "Maelstrom_Globals.h"
#include "load.h"
#include "init.h"
#include "fastrand.h"
#include "netlogic/about.h"
#include "main.h"

#include "screenlib/UIDialog.h"
#include "screenlib/UIElementCheckbox.h"
#include "screenlib/UIElementEditbox.h"
#include "screenlib/UIElementLabel.h"
#include "UIElementKeyButton.h"

#define MAELSTROM_PREFS	"com.galaxygameworks.Maelstrom"
#define MAELSTROM_DATA	"Maelstrom_Data.zip"

static const char *Version =
"Maelstrom v1.4.3 (GPL version 4.0.0) -- 10/08/2011 by Sam Lantinga\n";

// Global variables set in this file...
int	gStartLives;
int	gStartLevel;
Bool	gUpdateBuffer;
Bool	gRunning;
int	gNoDelay;


// Main Menu actions:
static void RunDoAbout(void)
{
	ui->ShowPanel(PANEL_ABOUT);
}
static void RunPlayGame(void)
{
	gStartLevel = 1;
	gStartLives = 3;
	gNoDelay = 0;
	NewGame();
}
static void RunQuitGame(void)
{
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	gRunning = false;
}
static void IncrementSound(void)
{
	if ( gSoundLevel < 8 ) {
		sound->Volume(++gSoundLevel);
		sound->PlaySound(gNewLife, 5);

		/* -- Draw the new sound level */
		gUpdateBuffer = true;
	}
}
static void DecrementSound(void)
{
	if ( gSoundLevel > 0 ) {
		sound->Volume(--gSoundLevel);
		sound->PlaySound(gNewLife, 5);

		/* -- Draw the new sound level */
		gUpdateBuffer = true;
	}
}
static void SetSoundLevel(int volume)
{
	/* Make sure the device is working */
	sound->Volume(volume);

	/* Set the new sound level! */
	gSoundLevel = volume;
	sound->PlaySound(gNewLife, 5);

	/* -- Draw the new sound level */
	gUpdateBuffer = true;
}
static void RunToggleFullscreen(void)
{
	screen->ToggleFullScreen();
}
static void CheatDialogInit(UIDialog *dialog)
{
	UIElementEditbox *editbox;

	editbox = dialog->GetElement<UIElementEditbox>("level");
	if (editbox) {
		editbox->SetFocus(true);
	}
}
static void CheatDialogDone(UIDialog *dialog, int status)
{
	UIElementEditbox *editbox;
	UIElementCheckbox *checkbox;

	if (status > 0) {
		editbox = dialog->GetElement<UIElementEditbox>("level");
		if (editbox) {
			gStartLevel = editbox->GetNumber();
			if (gStartLevel < 1 || gStartLevel > 40) {
				return;
			}
		}

		editbox = dialog->GetElement<UIElementEditbox>("lives");
		if (editbox) {
			gStartLives = editbox->GetNumber();
			if (gStartLives < 1 || gStartLives > 40) {
				gStartLives = 3;
			}
		}

		checkbox = dialog->GetElement<UIElementCheckbox>("turbofunk");
		gNoDelay = checkbox->IsChecked();

		Delay(SOUND_DELAY);
		sound->PlaySound(gNewLife, 5);
		Delay(SOUND_DELAY);
		NewGame();
	}
}
static void RunScreenshot(void)
{
	screen->ScreenDump("ScoreDump", 64, 48, 298, 384);
}

class SetVolumeDelegate : public UIButtonDelegate
{
public:
	SetVolumeDelegate(int volume) : m_volume(volume) { }

	virtual void OnClick() {
		SetSoundLevel(m_volume);
	}
private:
	int m_volume;
};

/* ----------------------------------------------------------------- */
/* -- Print a Usage message and quit.
      In several places we depend on this function exiting.
 */
static char *progname;
void PrintUsage(void)
{
	error("Usage: %s <options>\n\n", progname);
	error("Where <options> can be any of:\n\n"
"	-fullscreen		# Run Maelstrom in full-screen mode\n"
"	-windowed		# Run Maelstrom in windowed mode\n"
"	-gamma [0-8]		# Set the gamma correction\n"
"	-volume [0-8]		# Set the sound volume\n"
	);
	LogicUsage();
	error("\n");
	exit(1);
}

/* ----------------------------------------------------------------- */
/* -- Initialize PHYSFS and mount the data archive */
static bool
InitFilesystem(const char *argv0)
{
	char path[4096];
	const char *file = MAELSTROM_DATA;

	if (!PHYSFS_init(argv0)) {
		error("Couldn't initialize PHYSFS: %s\n", PHYSFS_getLastError());
		return false;
	}

	// Set up the write directory for this platform
	const char *home = PHYSFS_getUserDir();
#if defined(__MACOSX__) || defined(__IPHONEOS__)
	SDL_snprintf(path, SDL_arraysize(path), "%sLibrary/Application Support/%s/", home, MAELSTROM_PREFS);
#else
	SDL_snprintf(path, SDL_arraysize(path), "%s.%s/", home, MAELSTROM_PREFS);
#endif
	if (!PHYSFS_setWriteDir(path)) {
		if (!PHYSFS_setWriteDir(home) ||
		    !PHYSFS_mkdir(path+SDL_strlen(home)) ||
		    !PHYSFS_setWriteDir(path)) {
			error("Couldn't set write directory to %s: %s\n", path, PHYSFS_getLastError());
			return false;
		}
	}

	/* Put the write directory first in the search path */
	PHYSFS_mount(path, NULL, 0);

	/* Then add the base directory to the search path */
	PHYSFS_mount(PHYSFS_getBaseDir(), NULL, 0);

	/* Then add the data file, which could be appended to the executable */
	if (PHYSFS_mount(argv0, "/", 1)) {
		return true;
	}

	/* ... or not */
	SDL_snprintf(path, SDL_arraysize(path), "%s%s", PHYSFS_getBaseDir(), MAELSTROM_DATA);
	if (PHYSFS_mount(path, "/", 1)) {
		return true;
	}

	error("Couldn't find %s", MAELSTROM_DATA);
	return false;
}

/* ----------------------------------------------------------------- */
/* -- Blitter main program */
int main(int argc, char *argv[])
{
	/* Command line flags */
	Uint32 window_flags = 0;
	Uint32 render_flags = SDL_RENDERER_PRESENTVSYNC;

	/* Normal variables */
	SDL_Event event;

	if ( !InitFilesystem(argv[0]) ) {
		exit(1);
	}

	/* Seed the random number generator */
	SeedRandom(0L);
	/* Initialize the controls */
	LoadControls();

	/* Initialize game logic data structures */
	if ( InitLogicData() < 0 ) {
		exit(1);
	}

	/* Parse command line arguments */
#ifdef __MACOSX__
	//window_flags |= SDL_WINDOW_FULLSCREEN;
#endif
	for ( progname=argv[0]; --argc; ++argv ) {
		if ( strcmp(argv[1], "-fullscreen") == 0 ) {
			window_flags |= SDL_WINDOW_FULLSCREEN;
		} else
		if ( strcmp(argv[1], "-windowed") == 0 ) {
			window_flags &= ~SDL_WINDOW_FULLSCREEN;
		} else
		if ( strcmp(argv[1], "-gamma") == 0 ) {
			int gammacorrect;

			if ( ! argv[2] ) {  /* Print the current gamma */
				mesg("Current Gamma correction level: %d\n",
								gGammaCorrect);
				exit(0);
			}
			if ( (gammacorrect=atoi(argv[2])) < 0 || 
							gammacorrect > 8 ) {
				error(
	"Gamma correction value must be between 0 and 8. -- Exiting.\n");
				exit(1);
			}
			/* We need to update the gamma */
			gGammaCorrect = gammacorrect;
			SaveControls();

			++argv;
			--argc;
		}
		else if ( strcmp(argv[1], "-volume") == 0 ) {
			int volume;

			if ( ! argv[2] ) {  /* Print the current volume */
				mesg("Current volume level: %d\n",
								gSoundLevel);
				exit(0);
			}
			if ( (volume=atoi(argv[2])) < 0 || volume > 8 ) {
				error(
	"Volume must be a number between 0 and 8. -- Exiting.\n");
				exit(1);
			}
			/* We need to update the volume */
			gSoundLevel = volume;
			SaveControls();

			++argv;
			--argc;
		} else if ( LogicParseArgs(&argv, &argc) == 0 ) {
			/* LogicParseArgs() took care of everything */;
		} else if ( strcmp(argv[1], "-version") == 0 ) {
			error("%s", Version);
			exit(0);
		} else {
			PrintUsage();
		}
	}

	/* Make sure we have a valid player list (netlogic) */
	if ( InitLogic() < 0 )
		exit(1);

	/* Initialize everything. :) */
	if ( DoInitializations(window_flags, render_flags) < 0 ) {
		/* An error message was already printed */
		exit(1);
	}

	DropEvents();
	gRunning = true;
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	ui->ShowPanel(PANEL_MAIN);

	while ( gRunning ) {

		ui->Draw();

		/* -- In case we were faded out */
		screen->FadeIn();

		/* -- Get events */
		while ( screen->PollEvent(&event) ) {
			if ( ui->HandleEvent(event) )
				continue;

			/* -- Handle window close requests */
			if ( event.type == SDL_QUIT ) {
				RunQuitGame();
			}
		}
		Delay(FRAME_DELAY);
	}

	ui->HidePanel(PANEL_MAIN);
	exit(0);
}	/* -- main */


/* ----------------------------------------------------------------- */
/* -- Setup the main screen */

bool
MainPanelDelegate::OnLoad()
{
	UIElementLabel *label;
	UIElementButton *button;

	/* Set the version */
	label = m_panel->GetElement<UIElementLabel>("version");
	if (label) {
		label->SetText(VERSION_STRING);
	}

	/* Hook up the action click callbacks */
	button = m_panel->GetElement<UIElementButton>("PlayButton");
	if (button) {
		button->SetClickCallback(RunPlayGame);
	}
	button = m_panel->GetElement<UIElementButton>("ControlsButton");
	if (button) {
		button->SetButtonDelegate(new UIDialogLauncher(ui, DIALOG_CONTROLS));
	}
	button = m_panel->GetElement<UIElementButton>("ZapButton");
	if (button) {
		button->SetButtonDelegate(new UIDialogLauncher(ui, DIALOG_ZAP, NULL, ZapHighScores));
	}
	button = m_panel->GetElement<UIElementButton>("AboutButton");
	if (button) {
		button->SetClickCallback(RunDoAbout);
	}
	button = m_panel->GetElement<UIElementButton>("QuitButton");
	if (button) {
		button->SetClickCallback(RunQuitGame);
	}
	button = m_panel->GetElement<UIElementButton>("VolumeDownButton");
	if (button) {
		button->SetClickCallback(DecrementSound);
	}
	button = m_panel->GetElement<UIElementButton>("VolumeUpButton");
	if (button) {
		button->SetClickCallback(IncrementSound);
	}
	button = m_panel->GetElement<UIElementButton>("ToggleFullscreen");
	if (button) {
		button->SetClickCallback(RunToggleFullscreen);
	}
	button = m_panel->GetElement<UIElementButton>("Cheat");
	if (button) {
		button->SetButtonDelegate(new UIDialogLauncher(ui, DIALOG_CHEAT, CheatDialogInit, CheatDialogDone));
	}
	button = m_panel->GetElement<UIElementButton>("Special");
	if (button) {
		button->SetButtonDelegate(new UIDialogLauncher(ui, DIALOG_DAWN));
	}
	button = m_panel->GetElement<UIElementButton>("Screenshot");
	if (button) {
		button->SetClickCallback(RunScreenshot);
	}

	button = m_panel->GetElement<UIElementButton>("SetVolume0");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(0));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume1");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(1));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume2");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(2));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume3");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(3));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume4");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(4));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume5");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(5));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume6");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(6));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume7");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(7));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume8");
	if (button) {
		button->SetButtonDelegate(new SetVolumeDelegate(8));
	}

	return true;
}

void
MainPanelDelegate::OnShow()
{
	gUpdateBuffer = true;
}

void
MainPanelDelegate::OnTick()
{
	UIElementLabel *label;
	char name[32];
	char text[128];

	if (!gUpdateBuffer) {
		return;
	}
	gUpdateBuffer = false;

	for (int index = 0; index < 10; index++) {
		Uint8 R, G, B;

		if ( gLastHigh == index ) {
			R = 0xFF;
			G = 0xFF;
			B = 0xFF;
		} else {
			R = 30000>>8;
			G = 30000>>8;
			B = 30000>>8;
		}

		sprintf(name, "name_%d", index);
		label = m_panel->GetElement<UIElementLabel>(name);
		if (label) {
			label->SetTextColor(R, G, B);
			label->SetText(hScores[index].name);
		}

		sprintf(name, "score_%d", index);
		label = m_panel->GetElement<UIElementLabel>(name);
		if (label) {
			label->SetTextColor(R, G, B);
			sprintf(text, "%d", hScores[index].score);
			label->SetText(text);
		}

		sprintf(name, "wave_%d", index);
		label = m_panel->GetElement<UIElementLabel>(name);
		if (label) {
			label->SetTextColor(R, G, B);
			sprintf(text, "%d", hScores[index].wave);
			label->SetText(text);
		}
	}

	label = m_panel->GetElement<UIElementLabel>("last_score");
	if (label) {
		sprintf(text, "%d", GetScore());
		label->SetText(text);
	}

	label = m_panel->GetElement<UIElementLabel>("volume");
	if (label) {
		sprintf(text, "%d", gSoundLevel);
		label->SetText(text);
	}
}


void Message(const char *message)
{
// FIXME: This totally doesn't work anymore, but that may not matter if we're cutting network support.
	int x, y;
	SDL_Texture *textimage;
	if (!message) {
		return;
	}

	/* This was taken from the DrawMainScreen function */
	x = (SCREEN_WIDTH - 512) / 2;
	y = 25;
	textimage = fontserv->TextImage(message, fonts[NEWYORK_14], STYLE_BOLD, 0xCC, 0xCC, 0xCC);
	if ( textimage ) {
		screen->QueueBlit(x, y-screen->GetImageHeight(textimage)+2, textimage, NOCLIP);
		fontserv->FreeText(textimage);
	}
}
