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
#include "player.h"
#include "../screenlib/UIDialog.h"
#include "../screenlib/UIElement.h"
#include "../screenlib/UIElementRadio.h"

#define MAELSTROM_DATA	".Maelstrom-data"

/* Savable and configurable controls/data */

Controls::Controls() :
	gPauseControl("Controls.PauseControl", SDLK_PAUSE),
	gShieldControl("Controls.ShieldControl", SDLK_SPACE),
	gThrustControl("Controls.ThrustControl", SDLK_UP),
	gTurnRControl("Controls.TurnRControl", SDLK_RIGHT),
	gTurnLControl("Controls.TurnLControl", SDLK_LEFT),
	gFireControl("Controls.FireControl", SDLK_TAB),
	gQuitControl("Controls.QuitControl", SDLK_ESCAPE)
{
}

void
Controls::Bind(Prefs *prefs)
{
	gPauseControl.Bind(prefs);
	gShieldControl.Bind(prefs);
	gThrustControl.Bind(prefs);
	gTurnRControl.Bind(prefs);
	gTurnLControl.Bind(prefs);
	gFireControl.Bind(prefs);
	gQuitControl.Bind(prefs);
}

Controls controls;
PrefsVariable<int> gSoundLevel("SoundLevel", 4);
PrefsVariable<int> gGammaCorrect("GammaCorrect", 3);


void LoadControls(void)
{
	gSoundLevel.Bind(prefs);
	gGammaCorrect.Bind(prefs);
	controls.Bind(prefs);
}

void SaveControls(void)
{
	prefs->Save();
}

bool
ControlsDialogDelegate::OnLoad()
{
	char name[32];

	for (int i = 0; (unsigned)i < SDL_arraysize(m_controlKeys); ++i) {
		sprintf(name, "control%d", 1+i);
		m_controlKeys[i] = m_panel->GetElement<UIElement>(name);
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

	for (int i = 0; (unsigned)i < SDL_arraysize(m_keyinuseTimers); ++i) {
		m_keyinuseTimers[i] = 0;
	}

	m_controls = controls;

	ShowKeyLabels();
}

void
ControlsDialogDelegate::OnHide()
{
	if (m_dialog->GetDialogStatus() > 0) {
		controls = m_controls;
		SaveControls();
	}
}

void
ControlsDialogDelegate::OnTick()
{
	for (int i = 0; (unsigned)i < SDL_arraysize(m_keyinuseTimers); ++i) {
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

static Player *GetLocalPlayer()
{
	return GetControlPlayer(CONTROL_LOCAL);
}

static Player *GetKeyboardPlayer()
{
	return GetControlPlayer(CONTROL_KEYBOARD);
}

static Player *GetJoystickPlayer(Uint8 which)
{
	Uint8 joystickControl = (CONTROL_JOYSTICK1 << which);
	return GetControlPlayer(joystickControl);
}

static void HandleEvent(SDL_Event *event)
{
	Player *player;
	SDL_Keycode key;

	if (ui->HandleEvent(*event)) {
		return;
	}

	switch (event->type) {
#ifdef SDL_INIT_JOYSTICK
		/* -- Handle joystick axis motion */
		case SDL_JOYAXISMOTION:
			/* X-Axis - rotate right/left */
			player = GetJoystickPlayer(event->jaxis.which);
			if (!player) {
				break;
			}
			if ( event->jaxis.axis == 0 ) {
				if ( event->jaxis.value < -8000 ) {
					player->SetControl(LEFT_KEY, 1);
					player->SetControl(RIGHT_KEY, 0);
				} else
				if ( event->jaxis.value > 8000 ) {
					player->SetControl(RIGHT_KEY, 1);
					player->SetControl(LEFT_KEY, 0);
				} else {
					player->SetControl(LEFT_KEY, 0);
					player->SetControl(RIGHT_KEY, 0);
				}
			} else
			/* Y-Axis - accelerate */
			if ( event->jaxis.axis == 1 ) {
				if ( event->jaxis.value < -8000 ) {
					player->SetControl(THRUST_KEY, 1);
				} else {
					player->SetControl(THRUST_KEY, 0);
				}
			}
			break;

		/* -- Handle joystick button presses/releases */
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			player = GetJoystickPlayer(event->jbutton.which);
			if (!player) {
				break;
			}
			if ( event->jbutton.state == SDL_PRESSED ) {
				if ( event->jbutton.button == 0 ) {
					player->SetControl(FIRE_KEY, 1);
				} else
				if ( event->jbutton.button == 1 ) {
					player->SetControl(SHIELD_KEY, 1);
				}
			} else {
				if ( event->jbutton.button == 0 ) {
					player->SetControl(FIRE_KEY, 0);
				} else
				if ( event->jbutton.button == 1 ) {
					player->SetControl(SHIELD_KEY, 0);
				}
			}
			break;
#endif

		/* -- Handle key presses/releases */
		case SDL_KEYDOWN:
			/* -- Handle ALT-ENTER hotkey */
			player = GetKeyboardPlayer();
			if (!player) {
				break;
			}
	                if ( (event->key.keysym.sym == SDLK_RETURN) &&
			     (event->key.keysym.mod & KMOD_ALT) ) {
				screen->ToggleFullScreen();
				break;
			}
		case SDL_KEYUP:
			/* -- Handle normal key bindings */
			player = GetKeyboardPlayer();
			if (!player) {
				break;
			}
			key = event->key.keysym.sym;
			if ( event->key.state == SDL_PRESSED ) {
				/* Check for various control keys */
				if ( key == controls.gFireControl )
					player->SetControl(FIRE_KEY, 1);
				else if ( key == controls.gTurnRControl )
					player->SetControl(RIGHT_KEY, 1);
				else if ( key == controls.gTurnLControl )
					player->SetControl(LEFT_KEY, 1);
				else if ( key == controls.gShieldControl )
					player->SetControl(SHIELD_KEY, 1);
				else if ( key == controls.gThrustControl )
					player->SetControl(THRUST_KEY, 1);
				else if ( key == controls.gPauseControl )
					player->SetControl(PAUSE_KEY, 1);
				else if ( key == controls.gQuitControl )
					player->SetControl(ABORT_KEY, 1);
			} else {
				/* Update control key status */
				if ( key == controls.gFireControl )
					player->SetControl(FIRE_KEY, 0);
				else if ( key == controls.gTurnRControl )
					player->SetControl(RIGHT_KEY, 0);
				else if ( key == controls.gTurnLControl )
					player->SetControl(LEFT_KEY, 0);
				else if ( key == controls.gShieldControl )
					player->SetControl(SHIELD_KEY, 0);
				else if ( key == controls.gThrustControl )
					player->SetControl(THRUST_KEY, 0);
				else if ( key == SDLK_F1 ) {
					/* Special key --
						Switch displayed player
					 */
					RotatePlayerView();
				} else if ( key == SDLK_F3 ) {
					/* Special key --
						Do a screen dump here.
					 */
					screen->ScreenDump("ScreenShot",
								0, 0, 0, 0);
				}
			}
			break;

		case SDL_WINDOWEVENT:
			player = GetLocalPlayer();
			if (!player) {
				break;
			}
			if (event->window.event == SDL_WINDOWEVENT_MINIMIZED) {
				player->SetControl(MINIMIZE_KEY, 1);
			} if (event->window.event == SDL_WINDOWEVENT_RESTORED) {
				player->SetControl(MINIMIZE_KEY, 0);
			}
			break;

		case SDL_QUIT:
			player = GetLocalPlayer();
			if (!player) {
				break;
			}
			player->SetControl(ABORT_KEY, 1);
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
		while ( screen->PollEvent(&event) ) {
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

	while ( screen->PollEvent(&event) ) {
		if ( event.type == SDL_KEYDOWN ) {
			++keys;
		}
	}
	return(keys);
}

