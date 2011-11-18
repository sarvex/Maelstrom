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

#ifndef _gameinfo_h
#define _gameinfo_h

#include "protocol.h"
#include "packet.h"

class UIElement;
class UIElementCheckbox;
class UIElementRadioGroup;

enum PLAYER_CONTROL {
	CONTROL_KEYBOARD = 1,
	CONTROL_JOYSTICK,
	CONTROL_NETWORK,
};

enum PING_STATUS {
	PING_LOCAL,
	PING_GOOD,
	PING_OKAY,
	PING_BAD,
	PING_TIMEDOUT,
	NUM_PING_STATES
};

// This represents a physical machine (host/port combo) on the network
struct GameInfoNode
{
	Uint32 nodeID;
	IPaddress address;

	struct {
		Uint32 lastPing;
		Uint32 roundTripTime;
		PING_STATUS status;
	} ping;

};

// This represents a player in the game, on a particular network node
//
// The hosting node may have any number of players
// The other nodes may each only have one player, to simplify things
// like the join/leave/kick process.
//
struct GameInfoPlayer
{
	Uint32 nodeID;
	char name[MAX_NAMELEN+1];
	Uint8 controlMask;

	struct {
		UIElement *element;
		UIElementCheckbox *enabled;
		UIElement *name;
		UIElement *host;
		UIElementRadioGroup *control;
		UIElement *keyboard;
		UIElement *joystick;
		UIElement *network;
		UIElement *ping_states[NUM_PING_STATES];
	} UI;
};

class GameInfo
{
public:
	GameInfo() { Reset(); }

	// Equality operator for array operations
	bool operator ==(const GameInfo &rhs) {
		return gameID == rhs.gameID;
	}

	void Reset();

	void SetSinglePlayer(Uint8 wave, Uint8 lives, Uint8 turbo);

	void SetMultiplayerHost(Uint8 deathMatch, const char *name);

	void SetLocalID(Uint32 uniqueID) {
		localID = uniqueID;
	}

	void CopyFrom(const GameInfo &rhs);

	bool ReadFromPacket(DynamicPacket &packet);
	void WriteToPacket(DynamicPacket &packet);

	GameInfoNode *GetHost() {
		return GetNode(HOST_NODE);
	}
	GameInfoNode *GetNode(int index) {
		return &nodes[index];
	}
	GameInfoNode *GetNodeByID(Uint32 nodeID) {
		for (int i = 0; i < MAX_NODES; ++i) {
			if (nodeID == nodes[i].nodeID) {
				return &nodes[i];
			}
		}
	}
	GameInfoPlayer *GetPlayer(int index) {
		return &players[index];
	}

	bool HasNode(Uint32 nodeID);
	bool HasNode(const IPaddress &address);
	void RemoveNode(Uint32 nodeID);

	bool IsHosting() {
		return localID == gameID;
	}
	bool IsNetworkNode(int index);

	bool IsFull();

	void BindPlayerToUI(int index, UIElement *element);
	void UpdateUI();
	void UpdateUI(GameInfoPlayer *player);

	void InitializePing();
	void InitializePing(int index);
	void UpdatePingTime(int index, Uint32 timestamp);
	void UpdatePingStatus();
	void UpdatePingStatus(int index);
	PING_STATUS GetPingStatus(int index);

public:
	Uint32 gameID;
	Uint32 seed;
	Uint8 wave;
	Uint8 lives;
	Uint8 turbo;
	Uint8 deathMatch;
	GameInfoNode nodes[MAX_NODES];
	GameInfoPlayer players[MAX_PLAYERS];

	Uint32 localID;
};

#endif // _gameinfo_h
