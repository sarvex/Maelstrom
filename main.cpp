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
#include "fastrand.h"
#include "checksum.h"
#include "about.h"
#include "main.h"

#include "UIElementLabel.h"
#include "UIElementKeyButton.h"

/* External functions used in this file */
extern int DoInitializations(Uint32 window_flags, Uint32 render_flags);	/* init.cpp */
extern void CleanUp(void);

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
static void RunConfigureControls(void)
{
	ConfigureControls();
}
static void RunPlayGame(void)
{
	gStartLives = 3;
	gStartLevel = 1;
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
static void RunZapScores(void)
{
	if ( ZapHighScores() ) {
		/* Fade the screen and redisplay scores */
		screen->Fade();
		Delay(SOUND_DELAY);
		sound->PlaySound(gExplosionSound, 5);
		gUpdateBuffer = true;
	}
}
static void RunToggleFullscreen(void)
{
	screen->ToggleFullScreen();
}
static void RunCheat(void)
{
	gStartLevel = GetStartLevel();
	if ( gStartLevel > 0 ) {
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
/* -- Run a graphics speed test.                                     */
static void RunSpeedTest(void)
{
	const int test_reps = 100;	/* How many full cycles to run */

	Uint32 then, now;
	int i, frame, x=((640/2)-16), y=((480/2)-16);

	then = SDL_GetTicks();
	for ( i=0; i<test_reps; ++i ) {
		for ( frame=0; frame<SHIP_FRAMES; ++frame ) {
			screen->Clear();
			screen->QueueBlit(x, y, gPlayerShip->sprite[frame]);
			screen->Update();
		}
	}
	now = SDL_GetTicks();
	mesg("Graphics speed test took %d milliseconds per cycle.\r\n",
						((now-then)/test_reps));
}

/* ----------------------------------------------------------------- */
/* -- Print a Usage message and quit.
      In several places we depend on this function exiting.
 */
static char *progname;
void PrintUsage(void)
{
	error("\nUsage: %s [-netscores] -printscores\n", progname);
	error("or\n");
	error("Usage: %s <options>\n\n", progname);
	error("Where <options> can be any of:\n\n"
"	-fullscreen		# Run Maelstrom in full-screen mode\n"
"	-windowed		# Run Maelstrom in windowed mode\n"
"	-gamma [0-8]		# Set the gamma correction\n"
"	-volume [0-8]		# Set the sound volume\n"
"	-netscores		# Use the world-wide network score server\n"
	);
	LogicUsage();
	error("\n");
	exit(1);
}

/* ----------------------------------------------------------------- */
/* -- Blitter main program */
int main(int argc, char *argv[])
{
	/* Command line flags */
	int doprinthigh = 0;
	int speedtest = 0;
	Uint32 window_flags = 0;
	Uint32 render_flags = SDL_RENDERER_PRESENTVSYNC;

	/* Normal variables */
	SDL_Event event;

	if ( !PHYSFS_init(argv[0]) ) {
		error("Couldn't initialize PHYSFS: %s\n", PHYSFS_getLastError());
		exit(1);
	}
	if ( !PHYSFS_setSaneConfig("galaxygameworks", "Maelstrom", NULL, 0, 0) ) {
		error("Couldn't set PHYSFS config: %s\n", PHYSFS_getLastError());
		exit(1);
	}
	if ( !PHYSFS_mount(argv[0], "/", 1) &&
	     !PHYSFS_mount("Maelstrom_Data.zip", "/", 1) ) {
		error("Can't find Maelstrom_Data.zip\n");
		exit(1);
	}

#ifndef __WIN95__
	/* The first thing we do is calculate our checksum */
	(void) checksum();
#endif /* ! Win95 */

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
		}
#define CHECKSUM_DEBUG
#ifdef CHECKSUM_DEBUG
		else if ( strcmp(argv[1], "-checksum") == 0 ) {
			mesg("Checksum = %s\n", get_checksum(NULL, 0));
			exit(0);
		}
#endif /* CHECKSUM_DEBUG */
		else if ( strcmp(argv[1], "-printscores") == 0 )
			doprinthigh = 1;
		else if ( strcmp(argv[1], "-netscores") == 0 )
			gNetScores = 1;
		else if ( strcmp(argv[1], "-speedtest") == 0 ) {
			speedtest = 1;
			render_flags &= ~SDL_RENDERER_PRESENTVSYNC;
		} else if ( LogicParseArgs(&argv, &argc) == 0 ) {
			/* LogicParseArgs() took care of everything */;
		} else if ( strcmp(argv[1], "-version") == 0 ) {
			error("%s", Version);
			exit(0);
		} else {
			PrintUsage();
		}
	}

	/* Do we just want the high scores? */
	if ( doprinthigh ) {
		PrintHighScores();
		exit(0);
	}

	/* Make sure we have a valid player list (netlogic) */
	if ( InitLogic() < 0 )
		exit(1);

	/* Initialize everything. :) */
	if ( DoInitializations(window_flags, render_flags) < 0 ) {
		/* An error message was already printed */
		exit(1);
	}

	if ( speedtest ) {
		RunSpeedTest();
		CleanUp();
		exit(0);
	}

	DropEvents();
	gRunning = true;
//	while ( sound->Playing() )
//		Delay(SOUND_DELAY);
	ui->ShowPanel(PANEL_MAIN);

	while ( gRunning ) {

		ui->Draw();

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
	return(0);
}	/* -- main */


int DrawText(int x, int y, const char *text, MFont *font, Uint8 style,
					Uint8 R, Uint8 G, Uint8 B)
{
	SDL_Texture *textimage;
	int width;

	textimage = fontserv->TextImage(text, font, style, R, G, B);
	if ( textimage == NULL ) {
		width = 0;
	} else {
#ifdef UI_DEBUG
printf("DrawText: %d,%d '%s'\n", x, y-screen->GetImageHeight(textimage)+2, text);
#endif
		screen->QueueBlit(x, y-screen->GetImageHeight(textimage)+2, textimage, NOCLIP);
		width = screen->GetImageWidth(textimage);
		fontserv->FreeText(textimage);
	}
	return(width);
}


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
		button->SetClickCallback(RunConfigureControls);
	}
	button = m_panel->GetElement<UIElementButton>("ZapButton");
	if (button) {
		button->SetClickCallback(RunZapScores);
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
		button->SetClickCallback(RunCheat);
	}
	button = m_panel->GetElement<UIElementButton>("Special");
	if (button) {
		button->SetClickCallback(ShowDawn);
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
	int xOff;

	if (!message) {
		return;
	}

	/* This was taken from the DrawMainScreen function */
	xOff = (SCREEN_WIDTH - 512) / 2;
	DrawText(xOff, 25, message, fonts[NEWYORK_14], STYLE_BOLD,
						0xCC, 0xCC, 0xCC);
}
