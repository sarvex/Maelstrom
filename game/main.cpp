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
#include "main.h"

#include "../screenlib/UIDialog.h"
#include "../screenlib/UIElement.h"
#include "../screenlib/UIElementCheckbox.h"
#include "../screenlib/UIElementEditbox.h"

#define MAELSTROM_PREFS	"com.galaxygameworks.Maelstrom"
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
	gGameInfo.SetHost(prefs->GetString(PREFERENCES_HANDLE),
				DEFAULT_START_WAVE,
				DEFAULT_START_LIVES,
				DEFAULT_START_TURBO, 0);
	RunSinglePlayerGame();
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
		gGameInfo.SetHost(prefs->GetString(PREFERENCES_HANDLE),
					wave, lives, turbo, 0);
		RunSinglePlayerGame();
	}
}
static void RunScreenshot(void*)
{
	screen->ScreenDump("ScoreDump", 64, 48, 298, 384);
}

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
	char path[4096];

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
int MaelstromMain(int argc, char *argv[])
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

	while ( gRunning ) {
		// If we got a reply event, start it up!
		if (gReplayFile) {
			RunReplayGame(gReplayFile);
			SDL_free(gReplayFile);
			gReplayFile = NULL;
		}

		ui->Draw();

		/* -- In case we were faded out */
		screen->FadeIn();

		/* -- Get events */
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
		DelayFrame();
	}

	ui->HidePanel(PANEL_MAIN);
	CleanUp();
	exit(0);
}	/* -- main */


/* ----------------------------------------------------------------- */
/* -- Setup the main screen */

bool
MainPanelDelegate::OnLoad()
{
	UIElement *label;
	UIElementButton *button;

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
		button->SetClickCallback(new UIDialogLauncher(ui, DIALOG_LOBBY));
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

	button = m_panel->GetElement<UIElementButton>("SetVolume0");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(0));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume1");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(1));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume2");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(2));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume3");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(3));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume4");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(4));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume5");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(5));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume6");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(6));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume7");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(7));
	}
	button = m_panel->GetElement<UIElementButton>("SetVolume8");
	if (button) {
		button->SetClickCallback(new SetVolumeCallback(8));
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

		sprintf(name, "name_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			label->SetText(hScores[index].name);
		}

		sprintf(name, "score_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			sprintf(text, "%d", hScores[index].score);
			label->SetText(text);
		}

		sprintf(name, "wave_%d", index);
		label = m_panel->GetElement<UIElement>(name);
		if (label) {
			label->SetColor(R, G, B);
			sprintf(text, "%d", hScores[index].wave);
			label->SetText(text);
		}
	}

	label = m_panel->GetElement<UIElement>("last_score");
	if (label) {
		if (gReplay.Load(LAST_REPLAY, true)) {
			sprintf(text, "%d", gReplay.GetFinalScore());
			label->SetText(text);
		} else {
			label->SetText("0");
		}
	}

	label = m_panel->GetElement<UIElement>("volume");
	if (label) {
		sprintf(text, "%d", gSoundLevel.Value());
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

void DelayFrame(void)
{
	Uint32 ticks;

	while ( ((ticks=SDL_GetTicks())-gLastDrawn) < FRAME_DELAY_MS ) {
		ui->Poll();
		SDL_Delay(1);
	}
	gLastDrawn = ticks;
}
