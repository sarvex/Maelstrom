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

/* This file handles the controls configuration and updating the keystrokes 
*/

#include <string.h>
#include <ctype.h>

#include "Maelstrom_Globals.h"
#include "load.h"
#include "screenlib/UIDialog.h"
#include "screenlib/UIElementLabel.h"
#include "screenlib/UIElementRadio.h"

#define MAELSTROM_DATA	".Maelstrom-data"

/* Savable and configurable controls/data */

/* Pause        Shield     Thrust  TurnR      TurnL     Fire     Quit  */
#ifdef FAITHFUL_SPECS
Controls controls =
{ SDLK_CAPSLOCK,SDLK_SPACE,SDLK_UP,SDLK_RIGHT,SDLK_LEFT,SDLK_TAB,SDLK_ESCAPE };
#else
Controls controls =
   { SDLK_PAUSE,SDLK_SPACE,SDLK_UP,SDLK_RIGHT,SDLK_LEFT,SDLK_TAB,SDLK_ESCAPE };
#endif

#ifdef MOVIE_SUPPORT
int	gMovie = 0;
#endif
Uint8 gSoundLevel = 4;
Uint8 gGammaCorrect = 3;


/* Map a keycode to a key name */
void KeyName(SDL_Keycode keycode, char *namebuf)
{
	const char *name;
	char ch;
	int starting;

	/* Get the name of the key */
	name = SDL_GetKeyName(keycode);

	/* Add "arrow" to the arrow keys */
	if ( strcmp(name, "up") == 0 ) {
		name = "up arrow";
	} else
	if ( strcmp(name, "down") == 0 ) {
		name = "down arrow";
	} else
	if ( strcmp(name, "right") == 0 ) {
		name = "right arrow";
	} else
	if ( strcmp(name, "left") == 0 ) {
		name = "left arrow";
	}
	/* Make the key names uppercased */
	for ( starting = 1; *name; ++name ) {
		ch = *name;
		if ( starting ) {
			if ( islower(ch) )
				ch = toupper(ch);
			starting = 0;
		} else {
			if ( ch == ' ' )
				starting = 1;
		}
		*namebuf++ = ch;
	}
	*namebuf = '\0';
}

static FILE *OpenData(const char *mode, char **fname)
{
	static char datafile[BUFSIZ];
	FILE *data;

	if ( fname ) {
		*fname = datafile;
	}
	sprintf(datafile,  "%s/%s", PHYSFS_getWriteDir(), MAELSTROM_DATA);
	if ( (data=fopen(datafile, mode)) == NULL )
		return(NULL);
	return(data);
}

void LoadControls(void)
{
	char  buffer[BUFSIZ], *datafile;
	FILE *data;

	/* Open our control data file */
	data = OpenData("r", &datafile);
	if ( data == NULL ) {
		return;
	}

	/* Read the controls */
	if ( (fread(buffer, 1, 5, data) != 5) || strncmp(buffer, "MAEL3", 5) ) {
		error(
		"Warning: Data file '%s' is corrupt! (will fix)\n", datafile);
		fclose(data);
		return;
	}
	fread(&gSoundLevel, sizeof(gSoundLevel), 1, data);
	fread(&controls, sizeof(controls), 1, data);
	fread(&gGammaCorrect, sizeof(gGammaCorrect), 1, data);
	fclose(data);
}

void SaveControls(void)
{
	char  *datafile;
	const char *newmode;
	FILE *data;

	/* Don't clobber existing joystick data */
	if ( (data=OpenData("r", NULL)) != NULL ) {
		newmode = "r+";
		fclose(data);
	} else
		newmode = "w";

	if ( (data=OpenData(newmode, &datafile)) == NULL ) {
		error("Warning: Couldn't save controls to %s\n", datafile);
		return;
	}

	fwrite("MAEL3", 1, 5, data);
	fwrite(&gSoundLevel, sizeof(gSoundLevel), 1, data);
	fwrite(&controls, sizeof(controls), 1, data);
	fwrite(&gGammaCorrect, sizeof(gGammaCorrect), 1, data);
	fclose(data);
}

bool
ControlsDialogDelegate::OnLoad()
{
	char name[32];

	for (int i = 0; i < SDL_arraysize(m_controlKeys); ++i) {
		sprintf(name, "control%d", 1+i);
		m_controlKeys[i] = m_panel->GetElement<UIElementLabel>(name);
		if (!m_controlKeys[i]) {
			fprintf(stderr, "Warning: Couldn't find control key label '%s'\n", name);
			return false;
		}
	}

	m_radioGroup = m_panel->GetElement<UIElementRadioGroup>("controlsRadioGroup");
	if (!m_radioGroup) {
		fprintf(stderr, "Warning: Couldn't find 'controlsRadioGroup'\n");
		return false;
	}

	return true;
}

void
ControlsDialogDelegate::OnShow()
{
	UIElementRadioButton *button;

	button = m_radioGroup->GetRadioButton(1);
	if (button) {
		button->SetChecked(true);
	}

	for (int i = 0; i < SDL_arraysize(m_keyinuseTimers); ++i) {
		m_keyinuseTimers[i] = 0;
	}

	m_controls = controls;

	ShowKeyLabels();
}

void
ControlsDialogDelegate::OnHide()
{
	if (m_panel->IsA(UIDialog::GetType()) &&
	    static_cast<UIDialog*>(m_panel)->GetDialogStatus() > 0) {
		controls = m_controls;
		SaveControls();
	}
}

void
ControlsDialogDelegate::OnTick()
{
	for (int i = 0; i < SDL_arraysize(m_keyinuseTimers); ++i) {
		if (m_keyinuseTimers[i] && (SDL_GetTicks() - m_keyinuseTimers[i]) > 1000) {
			m_keyinuseTimers[i] = 0;
			ShowKeyLabel(i);
		}
	}
}

bool
ControlsDialogDelegate::HandleEvent(const SDL_Event &event)
{
	if (event.type == SDL_KEYDOWN) {
		return true;
	}
	if (event.type == SDL_KEYUP) {
		int index;
		SDL_Keycode key = event.key.keysym.sym;

		index = m_radioGroup->GetValue() - 1;

		/* See if this key is in use */
		m_keyinuseTimers[index] = 0;
		for (int i = 0; i < NUM_CTLS; ++i) {
			if (i != index && key == GetKeycode(i)) {
				m_keyinuseTimers[index] = SDL_GetTicks();
				break;
			}
		}
		if (!m_keyinuseTimers[index]) {
			SetKeycode(index, key);
		}
		ShowKeyLabel(index);

		return true;
	}
	return false;
}

void
ControlsDialogDelegate::ShowKeyLabel(int index)
{
	const char *text;

	if (m_keyinuseTimers[index]) {
		text = "That key is in use!";
	} else {
		text = SDL_GetKeyName(GetKeycode(index));
	}
	m_controlKeys[index]->SetText(text);
}

SDL_Keycode
ControlsDialogDelegate::GetKeycode(int index)
{
	switch (index) {
		case FIRE_CTL:
			return m_controls.gFireControl;
		case THRUST_CTL:
			return m_controls.gThrustControl;
		case SHIELD_CTL:
			return m_controls.gShieldControl;
		case TURNR_CTL:
			return m_controls.gTurnRControl;
		case TURNL_CTL:
			return m_controls.gTurnLControl;
		case PAUSE_CTL:
			return m_controls.gPauseControl;
		case QUIT_CTL:
			return m_controls.gQuitControl;
		default:
			return SDLK_UNKNOWN;
	}
}

void
ControlsDialogDelegate::SetKeycode(int index, SDL_Keycode keycode)
{
	switch (index) {
		case FIRE_CTL:
			m_controls.gFireControl = keycode;
			break;
		case THRUST_CTL:
			m_controls.gThrustControl = keycode;
			break;
		case SHIELD_CTL:
			m_controls.gShieldControl = keycode;
			break;
		case TURNR_CTL:
			m_controls.gTurnRControl = keycode;
			break;
		case TURNL_CTL:
			m_controls.gTurnLControl = keycode;
			break;
		case PAUSE_CTL:
			m_controls.gPauseControl = keycode;
			break;
		case QUIT_CTL:
			m_controls.gQuitControl = keycode;
			break;
		default:
			break;
	}
}

static void HandleEvent(SDL_Event *event)
{
	SDL_Keycode key;

	switch (event->type) {
#ifdef SDL_INIT_JOYSTICK
		/* -- Handle joystick axis motion */
		case SDL_JOYAXISMOTION:
			/* X-Axis - rotate right/left */
			if ( event->jaxis.axis == 0 ) {
				if ( event->jaxis.value < -8000 ) {
					SetControl(LEFT_KEY, 1);
					SetControl(RIGHT_KEY, 0);
				} else
				if ( event->jaxis.value > 8000 ) {
					SetControl(RIGHT_KEY, 1);
					SetControl(LEFT_KEY, 0);
				} else {
					SetControl(LEFT_KEY, 0);
					SetControl(RIGHT_KEY, 0);
				}
			} else
			/* Y-Axis - accelerate */
			if ( event->jaxis.axis == 1 ) {
				if ( event->jaxis.value < -8000 ) {
					SetControl(THRUST_KEY, 1);
				} else {
					SetControl(THRUST_KEY, 0);
				}
			}
			break;

		/* -- Handle joystick button presses/releases */
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if ( event->jbutton.state == SDL_PRESSED ) {
				if ( event->jbutton.button == 0 ) {
					SetControl(FIRE_KEY, 1);
				} else
				if ( event->jbutton.button == 1 ) {
					SetControl(SHIELD_KEY, 1);
				}
			} else {
				if ( event->jbutton.button == 0 ) {
					SetControl(FIRE_KEY, 0);
				} else
				if ( event->jbutton.button == 1 ) {
					SetControl(SHIELD_KEY, 0);
				}
			}
			break;
#endif

		/* -- Handle key presses/releases */
		case SDL_KEYDOWN:
			/* -- Handle ALT-ENTER hotkey */
	                if ( (event->key.keysym.sym == SDLK_RETURN) &&
			     (event->key.keysym.mod & KMOD_ALT) ) {
				screen->ToggleFullScreen();
				break;
			}
		case SDL_KEYUP:
			/* -- Handle normal key bindings */
			key = event->key.keysym.sym;
			if ( event->key.state == SDL_PRESSED ) {
				/* Check for various control keys */
				if ( key == controls.gFireControl )
					SetControl(FIRE_KEY, 1);
				else if ( key == controls.gTurnRControl )
					SetControl(RIGHT_KEY, 1);
				else if ( key == controls.gTurnLControl )
					SetControl(LEFT_KEY, 1);
				else if ( key == controls.gShieldControl )
					SetControl(SHIELD_KEY, 1);
				else if ( key == controls.gThrustControl )
					SetControl(THRUST_KEY, 1);
				else if ( key == controls.gPauseControl )
					SetControl(PAUSE_KEY, 1);
				else if ( key == controls.gQuitControl )
					SetControl(ABORT_KEY, 1);
				else if ( SpecialKey(event->key.keysym.sym) == 0 )
					/* The key has been handled */;
				else if ( key == SDLK_F3 ) {
					/* Special key --
						Do a screen dump here.
					 */
					screen->ScreenDump("ScreenShot",
								0, 0, 0, 0);
#ifdef MOVIE_SUPPORT
				} else if ( key == XK_F5 ) {
					/* Special key --
						Toggle movie function.
					 */
					extern int SelectMovieRect(void);
					if ( ! gMovie )
						gMovie = SelectMovieRect();
					else
						gMovie = 0;
mesg("Movie is %s...\n", gMovie ? "started" : "stopped");
#endif
				}
			} else {
				/* Update control key status */
				if ( key == controls.gFireControl )
					SetControl(FIRE_KEY, 0);
				else if ( key == controls.gTurnRControl )
					SetControl(RIGHT_KEY, 0);
				else if ( key == controls.gTurnLControl )
					SetControl(LEFT_KEY, 0);
				else if ( key == controls.gShieldControl )
					SetControl(SHIELD_KEY, 0);
				else if ( key == controls.gThrustControl )
					SetControl(THRUST_KEY, 0);
			}
			break;

		case SDL_QUIT:
			SetControl(ABORT_KEY, 1);
			break;
	}
}


/* This function gives a good way to delay a specified amount of time
   while handling keyboard/joystick events, or just to poll for events.
*/
void HandleEvents(int timeout)
{
	SDL_Event event;

	do { 
		while ( SDL_PollEvent(&event) ) {
			HandleEvent(&event);
		}
		if ( timeout ) {
			/* Delay 1/60 of a second... */
			Delay(1);
		}
	} while ( timeout-- );
}

int DropEvents(void)
{
	SDL_Event event;
	int keys = 0;

	while ( SDL_PollEvent(&event) ) {
		if ( event.type == SDL_KEYDOWN ) {
			++keys;
		}
	}
	return(keys);
}

