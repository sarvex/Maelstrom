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

	bool IsFull() {
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			if (!players[i].playerID) {
				return false;
			}
		}
		return true;
	}

	void BindPlayerToUI(int index, UIElement *element);

public:
	Uint32 gameID;
	Uint8 deathMatch;

	struct GameInfoPlayer {
		Uint32 playerID;
		IPaddress address;
		char name[MAX_NAMELEN+1];

		struct {
			UIElementCheckbox *enabled;
			UIElement *name;
			UIElement *host;
			UIElement *ping;
			UIElementRadioGroup *control;
			UIElement *keyboard;
			UIElement *joystick;
			UIElement *network;
		} UI;
	};
	GameInfoPlayer players[MAX_PLAYERS];

	Uint32 localID;

protected:
	void UpdateUI();
	void UpdateUI(GameInfoPlayer *player);
};

#endif // _gameinfo_h
