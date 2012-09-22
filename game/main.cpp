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
#include "about.h"
#include "game.h"
#include "netplay.h"
#include "features.h"
#include "main.h"

#include "../screenlib/UIDialog.h"
#include "../screenlib/UIElement.h"
#include "../screenlib/UIElementCheckbox.h"
#include "../screenlib/UIElementEditbox.h"

#if __IPHONEOS__
#include "../Xcode-iOS/Maelstrom_GameKit.h"
#endif

#define MAELSTROM_ORGANIZATION	"AmbrosiaSW"
#define MAELSTROM_NAME		"Maelstrom"
#define MAELSTROM_DATA	"Maelstrom_Data.zip"

static const char *Version =
"Maelstrom v1.4.3 (GPL version 4.0.0) -- 10/08/2011 by Sam Lantinga\n";

// Global variables set in this file...
Bool	gUpdateBuffer;
Bool	gRunning;


// Main Menu actions:
static void RunDoAbout(void*)
{
	ui->ShowPanel(PANEL_ABOUT);
}
static void RunSinglePlayerGame()
{
	if (InitNetData(false) < 0) {
		return;
	}
	NewGame();
	HaltNetData();
}
static void RunPlayGame(void*)
{
	gGameInfo.SetHost(DEFAULT_START_WAVE, DEFAULT_START_LIVES, DEFAULT_START_TURBO, 0, prefs->GetBool(PREFERENCES_KIDMODE));
	gGameInfo.SetPlayerSlot(0, prefs->GetString(PREFERENCES_HANDLE), CONTROL_LOCAL);
	RunSinglePlayerGame();
}
static void RunMultiplayerGame(void*)
{
	UIDialog *dialog;

	if (!HasFeature(FEATURE_NETWORK)) {
		ShowFeature(FEATURE_NETWORK);
		return;
	}

	dialog = ui->GetPanel<UIDialog>(DIALOG_LOBBY);
	if (dialog) {
		ui->ShowPanel(dialog);
	}
}
static void RunReplayGame(const char *file)
{
	if (!gReplay.Load(file)) {
		return;
	}

	gReplay.SetMode(REPLAY_PLAYBACK);
	RunSinglePlayerGame();
}
static void RunLastReplay(void*)
{
	RunReplayGame(LAST_REPLAY);
}
static void RunQuitGame(void*)
{
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	gRunning = false;
}
static void IncrementSound(void*)
{
	if ( gSoundLevel < 8 ) {
		sound->Volume(++gSoundLevel);
		sound->PlaySound(gNewLife, 5);

		/* -- Draw the new sound level */
		gUpdateBuffer = true;
	}
}
static void DecrementSound(void*)
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
static void RunToggleFullscreen(void*)
{
	screen->ToggleFullScreen();
}
static void CheatDialogInit(void*, UIDialog *dialog)
{
	UIElementEditbox *editbox;

	editbox = dialog->GetElement<UIElementEditbox>("level");
	if (editbox) {
		editbox->SetFocus(true);
	}
}
static void CheatDialogDone(void*, UIDialog *dialog, int status)
{
	UIElementEditbox *editbox;
	UIElementCheckbox *checkbox;
	Uint8 wave = DEFAULT_START_WAVE;
	Uint8 lives = DEFAULT_START_LIVES;
	Uint8 turbo = DEFAULT_START_TURBO;

	if (status > 0) {
		editbox = dialog->GetElement<UIElementEditbox>("level");
		if (editbox) {
			wave = editbox->GetNumber();
			if (wave < 1 || wave > 40) {
				wave = DEFAULT_START_WAVE;
			}
		}

		editbox = dialog->GetElement<UIElementEditbox>("lives");
		if (editbox) {
			lives = editbox->GetNumber();
			if (lives < 1 || lives > 40) {
				lives = DEFAULT_START_LIVES;
			}
		}

		checkbox = dialog->GetElement<UIElementCheckbox>("turbofunk");
		if (checkbox) {
			turbo = checkbox->IsChecked();
		}

		Delay(SOUND_DELAY);
		sound->PlaySound(gNewLife, 5);
		Delay(SOUND_DELAY);
		gGameInfo.SetHost(wave, lives, turbo, 0, prefs->GetBool(PREFERENCES_KIDMODE));
		gGameInfo.SetPlayerSlot(0, prefs->GetString(PREFERENCES_HANDLE), CONTROL_LOCAL);
		RunSinglePlayerGame();
	}
}
static void RunScreenshot(void*)
{
	screen->ScreenDump("ScoreDump", 64, 48, 298, 384);
}
static void UpdateKidMode(void *param)
{
	UIElementCheckbox *checkbox = (UIElementCheckbox*)param;

	if (checkbox->IsChecked()) {
		if (!HasFeature(FEATURE_KIDMODE)) {
			ShowFeature(FEATURE_KIDMODE);
			checkbox->SetChecked(false);
		}
	}

	checkbox->SaveData(prefs);
}

class RunReplayCallback : public UIClickCallback
{
public:
	RunReplayCallback(int index) : m_index(index) { }

	virtual void operator()() {
		const char *file = hScores[m_index].file;
		if (file) {
			RunReplayGame(file);
		}
	}
private:
	int m_index;
};

class SetVolumeCallback : public UIClickCallback
{
public:
	SetVolumeCallback(int volume) : m_volume(volume) { }

	virtual void operator()() {
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
	);
	error("\n");
	exit(1);
}

/* ----------------------------------------------------------------- */
/* -- Initialize PHYSFS and mount the data archive */
static bool
InitFilesystem(const char *argv0)
{
	const char *prefspath;

	if (!PHYSFS_init(argv0)) {
		error("Couldn't initialize PHYSFS: %s\n", PHYSFS_getLastError());
		return false;
	}

	// Set up the write directory for this platform
	prefspath = PHYSFS_getPrefDir(MAELSTROM_ORGANIZATION, MAELSTROM_NAME);
	if (!prefspath) {
		error("Couldn't get preferences path for this platform\n");
		return false;
	}
	if (!PHYSFS_setWriteDir(prefspath)) {
		error("Couldn't set write directory to %s: %s\n", prefspath, PHYSFS_getLastError());
		return false;
	}

	/* Put the write directory first in the search path */
	PHYSFS_mount(prefspath, NULL, 0);

	/* Then add the base directory to the search path */
	PHYSFS_mount(PHYSFS_getBaseDir(), NULL, 0);

	/* Then add the data file, which could be appended to the executable */
	if (PHYSFS_mount(argv0, "/", 1)) {
		return true;
	}

	/* ... or not */
	char path[4096];
	SDL_snprintf(path, SDL_arraysize(path), "%s%s", PHYSFS_getBaseDir(), MAELSTROM_DATA);
	if (PHYSFS_mount(path, "/", 1)) {
		return true;
	}

	error("Couldn't find %s", MAELSTROM_DATA);
	return false;
}

/* ----------------------------------------------------------------- */
extern "C" void ShowFrame(void*);
extern "C"
void ShowFrame(void*)
{
	ui->Draw();

	if (!gGameOn) {
		// If we got a reply event, start it up!
		if (gReplayFile) {
			RunReplayGame(gReplayFile);
			SDL_free(gReplayFile);
			gReplayFile = NULL;
		}

		/* -- Get events */
		SDL_Event event;
		while ( screen->PollEvent(&event) ) {
			if ( ui->HandleEvent(event) )
				continue;

			/* -- Handle file drop requests */
			if ( event.type == SDL_DROPFILE ) {
				gReplayFile = event.drop.file;
			}

			/* -- Handle window close requests */
			if ( event.type == SDL_QUIT ) {
				RunQuitGame(0);
			}
		}
	}
}

/* ----------------------------------------------------------------- */
/* -- Blitter main program */
int MaelstromMain(int argc, char *argv[])
{
	/* Command line flags */
	Uint32 window_flags = 0;
	Uint32 render_flags = SDL_RENDERER_PRESENTVSYNC;

	if ( !InitFilesystem(argv[0]) ) {
		exit(1);
	}

	/* Seed the random number generator */
	SeedRandom(0L);

	/* Parse command line arguments */
	window_flags |= SDL_WINDOW_FULLSCREEN;
	for ( progname=argv[0]; --argc; ++argv ) {
		if ( strcmp(argv[1], "-fullscreen") == 0 ) {
			window_flags |= SDL_WINDOW_FULLSCREEN;
		} else
		if ( strcmp(argv[1], "-windowed") == 0 ) {
			window_flags &= ~SDL_WINDOW_FULLSCREEN;
		} else if ( strcmp(argv[1], "-version") == 0 ) {
			error("%s", Version);
			exit(0);
		} else if ( strcmp(argv[1], "-help") == 0 ) {
			PrintUsage();
		}
	}

	/* Initialize everything. :) */
	if ( DoInitializations(window_flags, render_flags) < 0 ) {
		/* An error message was already printed */
		CleanUp();
		exit(1);
	}

	DropEvents();
	gRunning = true;
#ifndef FAST_ITERATION
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
#endif

	ui->ShowPanel(PANEL_MAIN);

#if __IPHONEOS__
	// Initialize the Game Center for scoring and matchmaking
	InitGameCenter();

	// Set up the game to run in the window animation callback on iOS
	// so that Game Center and so forth works correctly.
	SDL_iPhoneSetAnimationCallback(screen->GetWindow(), 2, ShowFrame, 0);
#else
	while ( gRunning ) {
		ShowFrame(0);

		if (!gGameOn || !gGameInfo.turbo) {
			DelayFrame();
		}
	}
	CleanUp();
#endif
	return 0;

}	/* -- main */


/* ----------------------------------------------------------------- */
/* -- Setup the main screen */

bool
MainPanelDelegate::OnLoad()
{
	char name[128];
	int i;
	UIElement *label;
	UIElementButton *button;
	UIElementCheckbox *checkbox;

	/* Set the version */
	label = m_panel->GetElement<UIElement>("version");
	if (label) {
		label->SetText(VERSION_STRING);
	}

	/* Hook up the action click callbacks */
	button = m_panel->GetElement<UIElementButton>("PlayButton");
	if (button) {
		button->SetClickCallback(RunPlayGame);
	}
	button = m_panel->GetElement<UIElementButton>("MultiplayerButton");
	if (button) {
		button->SetClickCallback(RunMultiplayerGame);
	}
	button = m_panel->GetElement<UIElementButton>("ControlsButton");
	if (button) {
		button->SetClickCallback(new UIDialogLauncher(ui, DIALOG_CONTROLS));
	}
	button = m_panel->GetElement<UIElementButton>("ZapButton");
	if (button) {
		button->SetClickCallback(new UIDialogLauncher(ui, DIALOG_ZAP, NULL, ZapHighScores));
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
		button->SetClickCallback(new UIDialogLauncher(ui, DIALOG_CHEAT, CheatDialogInit, CheatDialogDone));
	}
	button = m_panel->GetElement<UIElementButton>("Special");
	if (button) {
		button->SetClickCallback(new UIDialogLauncher(ui, DIALOG_DAWN));
	}
	button = m_panel->GetElement<UIElementButton>("Screenshot");
	if (button) {
		button->SetClickCallback(RunScreenshot);
	}
	checkbox = m_panel->GetElement<UIElementCheckbox>("KidMode");
	if (checkbox) {
		checkbox->SetClickCallback(UpdateKidMode, checkbox);
	}

	for (i = 0; i < 9; ++i) {
		SDL_snprintf(name, sizeof(name), "SetVolume%d", i);
		button = m_panel->GetElement<UIElementButton>(name);
		if (button) {
			button->SetClickCallback(new SetVolumeCallback(i));
		}
	}

	for (i = 0; i < NUM_SCORES; ++i) {
		SDL_snprintf(name, sizeof(name), "play_%d", i);
		button = m_panel->GetElement<UIElementButton>(name);
		if (button) {
			button->SetClickCallback(new RunReplayCallback(i));
		}
	}
	button = m_panel->GetElement<UIElementButton>("play_last");
	if (button) {
		button->SetClickCallback(RunLastReplay);
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
	UIElement *label;
	char name[32];
	char text[128];

	if (!gUpdateBuffer) {
		return;
	}
	gUpdateBuffer = false;

	for (int index = 0; index < NUM_SCORES; index++) {
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

		SDL_snprintf(name, sizeof(name), "name_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			label->SetText(hScores[index].name);
		}

		SDL_snprintf(name, sizeof(name), "score_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			SDL_snprintf(text, sizeof(text), "%d", hScores[index].score);
			label->SetText(text);
		}

		SDL_snprintf(name, sizeof(name), "wave_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			SDL_snprintf(text, sizeof(text), "%d", hScores[index].wave);
			label->SetText(text);
		}
	}

	label = m_panel->GetElement<UIElement>("last_score");
	if (label) {
		if (gReplay.Load(LAST_REPLAY, true)) {
			SDL_snprintf(text, sizeof(text), "%d", gReplay.GetFinalScore());
			label->SetText(text);
		} else {
			label->SetText("0");
		}
	}

	label = m_panel->GetElement<UIElement>("volume");
	if (label) {
		SDL_snprintf(text, sizeof(text), "%d", gSoundLevel.Value());
		label->SetText(text);
	}
}


void Message(const char *message)
{
	// FIXME: This totally doesn't work anymore
}

void DelayFrame(void)
{
	Uint32 ticks;

	while ( ((ticks=SDL_GetTicks())-gLastDrawn) < FRAME_DELAY_MS ) {
		ui->Poll();
		SDL_Delay(1);
	}
	gLastDrawn = ticks;
}
