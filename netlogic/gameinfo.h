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

enum {
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

struct GameInfoPlayer
{
	Uint32 playerID;
	IPaddress address;
	char name[MAX_NAMELEN+1];

	struct {
		Uint32 lastPing;
		Uint32 roundTripTime;
		PING_STATUS status;
	} ping;

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

	void SetLocalID(Uint32 playerID) {
		localID = playerID;
	}

	void SetHostInfo(Uint32 gameID, const char *name);

	void CopyFrom(const GameInfo &rhs);

	bool ReadFromPacket(DynamicPacket &packet);
	void WriteToPacket(DynamicPacket &packet);

	GameInfoPlayer *GetHost() {
		return GetPlayer(HOST_PLAYER);
	}
	GameInfoPlayer *GetPlayer(int index) {
		return &players[index];
	}
	GameInfoPlayer *GetPlayerByID(Uint32 playerID) {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (players[i].playerID == playerID) {
				return &players[i];
			}
		}
		return NULL;
	}

	bool HasPlayer(Uint32 playerID) {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (players[i].playerID == playerID) {
				return true;
			}
		}
		return false;
	}
	bool HasPlayer(const IPaddress &address) {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (players[i].address == address) {
				return true;
			}
		}
		return false;
	}

	bool IsNetworkPlayer(int index) {
		if (!players[index].playerID) {
			return false;
		}
		if (players[index].playerID == localID) {
			return false;
		}
		return true;
	}

	bool IsFull() {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (!players[i].playerID) {
				return false;
			}
		}
		return true;
	}

	void BindPlayerToUI(int index, UIElement *element);
	void UpdateUI();
	void UpdateUI(GameInfoPlayer *player);

	void InitializePing() {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			InitializePing(i);
		}
	}
	void InitializePing(int index);
	void UpdatePingTime(int index, Uint32 timestamp);
	void UpdatePingStatus();

	PING_STATUS GetPingStatus(int index) {
		if (IsNetworkPlayer(index)) {
			return players[index].ping.status;
		} else {
			return PING_LOCAL;
		}
	}

public:
	Uint32 gameID;
	Uint8 deathMatch;
	GameInfoPlayer players[MAX_PLAYERS];

	Uint32 localID;
};

#endif // _gameinfo_h
