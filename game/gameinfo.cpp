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
GameInfo::SetSinglePlayer(Uint8 wave, Uint8 lives, Uint8 turbo)
{
	Reset();
	this->gameID = 1;
	this->seed = GetRandSeed();
	this->wave = wave;
	this->lives = lives;
	this->turbo = turbo;
	this->deathMatch = 0;
}

void
GameInfo::SetMultiplayerHost(Uint8 deathMatch, const char *name)
{
	this->gameID = localID;
	this->seed = GetRandSeed();
	this->wave = DEFAULT_START_WAVE;
	this->lives = DEFAULT_START_LIVES;
	this->turbo = DEFAULT_START_TURBO;
	this->deathMatch = deathMatch;

	// We are the host node
	nodes[HOST_NODE].nodeID = localID;

	// We are the first player
	GameInfoPlayer *player = GetPlayer(0);
	player->nodeID = localID;
	SDL_strlcpy(player->name, name ? name : "", sizeof(player->name));
	player->controlMask = (CONTROL_KEYBOARD|CONTROL_JOYSTICK1);
}

void
GameInfo::CopyFrom(const GameInfo &rhs)
{
	int i;

	gameID = rhs.gameID;
	seed = rhs.seed;
	wave = rhs.wave;
	lives = rhs.lives;
	turbo = rhs.turbo;
	deathMatch = rhs.deathMatch;

	for (i = 0; i < MAX_NODES; ++i) {
		nodes[i].nodeID = rhs.nodes[i].nodeID;
		if (nodes[i].address != rhs.nodes[i].address) {
			nodes[i].address = rhs.nodes[i].address;

			// Reset the ping info
			InitializePing(i);
		}
	}

	for (i = 0; i < MAX_PLAYERS; ++i) {
		players[i].nodeID = rhs.players[i].nodeID;
		SDL_strlcpy(players[i].name, rhs.players[i].name,
			sizeof(players[i].name));
	}

	UpdateUI();
}

bool
GameInfo::ReadFromPacket(DynamicPacket &packet)
{
	int i;

	if (!packet.Read(gameID)) {
		return false;
	}
	if (!packet.Read(seed)) {
		return false;
	}
	if (!packet.Read(wave)) {
		return false;
	}
	if (!packet.Read(lives)) {
		return false;
	}
	if (!packet.Read(turbo)) {
		return false;
	}
	if (!packet.Read(deathMatch)) {
		return false;
	}

	for (i = 0; i < MAX_NODES; ++i) {
		if (!packet.Read(nodes[i].nodeID)) {
			return false;
		}
		if (!packet.Read(nodes[i].address.host)) {
			return false;
		}
		if (!packet.Read(nodes[i].address.port)) {
			return false;
		}
	}

	for (i = 0; i < MAX_PLAYERS; ++i) {
		if (!packet.Read(players[i].nodeID)) {
			return false;
		}
		if (!packet.Read(players[i].name, sizeof(players[i].name))) {
			return false;
		}
	}

	// We want to get the public address of the server
	// If we already have one, we assume that's the fastest interface
	if (!nodes[HOST_NODE].address.host) {
		nodes[HOST_NODE].address = packet.address;
	}

	return true;
}

void
GameInfo::WriteToPacket(DynamicPacket &packet)
{
	int i;

	packet.Write(gameID);
	packet.Write(seed);
	packet.Write(wave);
	packet.Write(lives);
	packet.Write(turbo);
	packet.Write(deathMatch);

	for (i = 0; i < MAX_NODES; ++i) {
		packet.Write(nodes[i].nodeID);
		packet.Write(nodes[i].address.host);
		packet.Write(nodes[i].address.port);
	}

	for (i = 0; i < MAX_PLAYERS; ++i) {
		packet.Write(players[i].nodeID);
		packet.Write(players[i].name);
	}
}

bool
GameInfo::HasNode(Uint32 nodeID)
{
	for (int i = 0; i < MAX_NODES; ++i) {
		if (nodes[i].nodeID == nodeID) {
			return true;
		}
	}
	return false;
}

bool
GameInfo::HasNode(const IPaddress &address)
{
	for (int i = 0; i < MAX_NODES; ++i) {
		if (nodes[i].address == address) {
			return true;
		}
	}
	return false;
}

void
GameInfo::RemoveNode(Uint32 nodeID)
{
	int i;
	for (i = 0; i < MAX_NODES; ++i) {
		if (nodeID == nodes[i].nodeID) {
			SDL_zero(nodes[i]);
		}
	}
	for (i = 0; i < MAX_PLAYERS; ++i) {
		if (nodeID == players[i].nodeID) {
			SDL_zero(players[i]);
		}
	}
}

bool
GameInfo::IsNetworkNode(int index)
{
	if (!nodes[index].nodeID) {
		return false;
	}
	if (nodes[index].nodeID == localID) {
		return false;
	}
	return true;
}

bool
GameInfo::IsFull()
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!players[i].nodeID) {
			return false;
		}
	}
	return true;
}

void
GameInfo::BindPlayerToUI(int index, UIElement *element)
{
	char name[32];
	GameInfoPlayer *player = &players[index];

	if (player->UI.element == element) {
		return;
	}

	player->UI.element = element;
	player->UI.enabled = element->GetElement<UIElementCheckbox>("enabled");
	player->UI.name = element->GetElement<UIElement>("name");
	player->UI.host = element->GetElement<UIElement>("host");
	player->UI.control = element->GetElement<UIElementRadioGroup>("control");
	player->UI.keyboard = element->GetElement<UIElement>("keyboard");
	player->UI.joystick = element->GetElement<UIElement>("joystick");
	player->UI.network = element->GetElement<UIElement>("network");

	for (int i = 0; i < NUM_PING_STATES; ++i) {
		SDL_snprintf(name, sizeof(name), "ping%d", i);
		player->UI.ping_states[i] = element->GetElement<UIElement>(name);
	}

	UpdateUI(player);
}

void
GameInfo::UpdateUI()
{
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		UpdateUI(&players[i]);
	}
}

void
GameInfo::UpdateUI(GameInfoPlayer *player)
{
	if (player->UI.name) {
		if (player->name[0]) {
			player->UI.name->Show();
			player->UI.name->SetText(player->name);
		} else {
			player->UI.name->Hide();
		}
	}
	if (player->UI.host) {
		GameInfoNode *node = GetNodeByID(player->nodeID);
		if (node->nodeID == localID) {
			//player->UI.host->Show();
			//player->UI.host->SetText("localhost");
			player->UI.host->Hide();
		} else if (node->address.host) {
			player->UI.host->Show();
			player->UI.host->SetText(SDLNet_ResolveIP(&node->address));
		} else {
			player->UI.host->Hide();
		}
	}
	if (player->UI.control) {
		if (player->nodeID == localID) {
			player->UI.control->SetValue(CONTROL_KEYBOARD);
		} else {
			player->UI.control->SetValue(CONTROL_NETWORK);
		}
	}
	if (player->UI.keyboard) {
		if (player->nodeID == localID) {
			player->UI.keyboard->Show();
		} else {
			player->UI.keyboard->Hide();
		}
	}
	if (player->UI.joystick) {
		if (player->nodeID == localID) {
			player->UI.joystick->Show();
		} else {
			player->UI.joystick->Hide();
		}
	}
	if (player->UI.network) {
		if (player->nodeID != localID) {
			player->UI.network->Show();
		} else {
			player->UI.network->Hide();
		}
	}
	for (int i = 0; i < NUM_PING_STATES; ++i) {
		UIElement *element = player->UI.ping_states[i];
		if (element) {
			if (GetNodeByID(player->nodeID)->ping.status == i) {
				element->Show();
			} else {
				element->Hide();
			}
		}
	}
}

void
GameInfo::InitializePing()
{
	for (int i = 0; i < MAX_NODES; ++i) {
		InitializePing(i);
	}
}

void
GameInfo::InitializePing(int index)
{
	GameInfoNode *node = GetNode(index);

	if (node->nodeID != localID) {
		node->ping.lastPing = SDL_GetTicks();
		node->ping.roundTripTime = 0;
		node->ping.status = PING_GOOD;
	}
}

void
GameInfo::UpdatePingTime(int index, Uint32 timestamp)
{
	Uint32 now;
	Uint32 elapsed;
	GameInfoNode *node;

	now = SDL_GetTicks();
	elapsed = (now - timestamp);

	node = GetNode(index);
	node->ping.lastPing = now;
	if (!node->ping.roundTripTime) {
		node->ping.roundTripTime = elapsed;
	} else {
		// Use a weighted average 2/3 previous value, 1/3 new value
		node->ping.roundTripTime = (2*node->ping.roundTripTime + 1*elapsed) / 3;
	}
}

void
GameInfo::UpdatePingStatus()
{
	for (int i = 0; i < MAX_NODES; ++i) {
		UpdatePingStatus(i);
	}
}

void
GameInfo::UpdatePingStatus(int index)
{
	GameInfoNode *node = GetNode(index);

	if (!IsNetworkNode(index)) {
		node->ping.status = PING_LOCAL;
	} else {
		Uint32 sinceLastPing;

		sinceLastPing = int(SDL_GetTicks() - node->ping.lastPing);
		if (sinceLastPing < 2*PING_INTERVAL) {
			if (node->ping.roundTripTime <= 2*FRAME_DELAY_MS) {
#ifdef DEBUG_NETWORK
printf("Game 0x%8.8x: node 0x%8.8x round trip time %d (GOOD)\n",
	gameID, node->nodeID, node->ping.roundTripTime);
#endif
				node->ping.status = PING_GOOD;
			} else if (node->ping.roundTripTime <= 3*FRAME_DELAY_MS) {
#ifdef DEBUG_NETWORK
printf("Game 0x%8.8x: node 0x%8.8x round trip time %d (OKAY)\n",
	gameID, node->nodeID, node->ping.roundTripTime);
#endif
				node->ping.status = PING_OKAY;
			} else {
#ifdef DEBUG_NETWORK
printf("Game 0x%8.8x: node 0x%8.8x round trip time %d (BAD)\n",
	gameID, node->nodeID, node->ping.roundTripTime);
#endif
				node->ping.status = PING_BAD;
			}
		} else if (sinceLastPing < PING_TIMEOUT) {
#ifdef DEBUG_NETWORK
printf("Game 0x%8.8x: node 0x%8.8x since last ping %d (BAD)\n",
	gameID, node->nodeID, sinceLastPing);
#endif
			node->ping.status = PING_BAD;
		} else {
#ifdef DEBUG_NETWORK
printf("Game 0x%8.8x: node 0x%8.8x since last ping %d (TIMEDOUT)\n",
	gameID, node->nodeID, sinceLastPing);
#endif
			node->ping.status = PING_TIMEDOUT;
		}
	}

	// Update the UI for matching players
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (players[i].nodeID == node->nodeID) {
			UpdateUI(&players[i]);
		}
	}
}

PING_STATUS
GameInfo::GetPingStatus(int index)
{
	if (IsNetworkNode(index)) {
		return nodes[index].ping.status;
	} else {
		return PING_LOCAL;
	}
}
