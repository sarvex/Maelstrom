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

#include "../screenlib/UIElement.h"
#include "../screenlib/UIElementCheckbox.h"
#include "../screenlib/UIElementRadio.h"

#include "gameinfo.h"


void
GameInfo::Reset()
{
	SDL_zero(*this);
}

void
GameInfo::SetHostInfo(Uint32 gameID, const char *name)
{
	this->gameID = gameID;
	players[0].playerID = gameID;
	SDL_strlcpy(players[0].name, name, sizeof(players[0].name));
}

void
GameInfo::CopyFrom(const GameInfo &rhs)
{
	gameID = rhs.gameID;
	deathMatch = rhs.deathMatch;

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (players[i].address != rhs.players[i].address) {
			// FIXME: reset the ping info
		}
		players[i] = rhs.players[i];
	}
	UpdateUI();
}

bool
GameInfo::ReadFromPacket(DynamicPacket &packet)
{
	if (!packet.Read(gameID)) {
		return false;
	}
	if (!packet.Read(deathMatch)) {
		return false;
	}

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!packet.Read(players[i].playerID)) {
			return false;
		}
		if (!packet.Read(players[i].address.host)) {
			return false;
		}
		if (!packet.Read(players[i].address.port)) {
			return false;
		}
		if (!packet.Read(players[i].name, sizeof(players[i].name))) {
			return false;
		}
	}

	// We want to get the public address of the server
	players[0].address = packet.address;

	return true;
}

void
GameInfo::WriteToPacket(DynamicPacket &packet)
{
	packet.Write(gameID);
	packet.Write(deathMatch);

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		packet.Write(players[i].playerID);
		packet.Write(players[i].address.host);
		packet.Write(players[i].address.port);
		packet.Write(players[i].name);
	}
}

void
GameInfo::BindPlayerToUI(int index, UIElement *element)
{
	GameInfoPlayer *info = &players[index];

	info->UI.enabled = element->GetElement<UIElementCheckbox>("enabled");
	info->UI.name = element->GetElement<UIElement>("name");
	info->UI.host = element->GetElement<UIElement>("host");
	info->UI.ping = element->GetElement<UIElement>("ping");
	info->UI.control = element->GetElement<UIElementRadioGroup>("control");
	info->UI.keyboard = element->GetElement<UIElement>("keyboard");
	info->UI.joystick = element->GetElement<UIElement>("joystick");
	info->UI.network = element->GetElement<UIElement>("network");

	UpdateUI(info);
}

void
GameInfo::UpdateUI()
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		UpdateUI(&players[i]);
	}
}

void
GameInfo::UpdateUI(GameInfoPlayer *info)
{
	if (info->UI.name) {
		if (info->name[0]) {
			info->UI.name->Show();
			info->UI.name->SetText(info->name);
		} else {
			info->UI.name->Hide();
		}
	}
	if (info->UI.host) {
		if (info->address.host) {
			info->UI.host->Show();
			info->UI.host->SetText(SDLNet_ResolveIP(&info->address));
		} else {
			info->UI.host->Hide();
		}
	}
	if (info->UI.ping) {
		// FIXME: not yet implemented
		info->UI.ping->Hide();
	}
	if (info->UI.control) {
		if (info->playerID == localID) {
			info->UI.control->SetValue(CONTROL_KEYBOARD);
		} else {
			info->UI.control->SetValue(CONTROL_NETWORK);
		}
	}
	if (info->UI.keyboard) {
		if (info->playerID == localID) {
			info->UI.keyboard->Show();
		} else {
			info->UI.keyboard->Hide();
		}
	}
	if (info->UI.joystick) {
		if (info->playerID == localID) {
			info->UI.joystick->Show();
		} else {
			info->UI.joystick->Hide();
		}
	}
	if (info->UI.network) {
		if (info->playerID != localID) {
			info->UI.network->Show();
		} else {
			info->UI.network->Hide();
		}
	}
}
