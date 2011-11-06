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

	for (int i = 1; i < MAX_PLAYERS; ++i) {
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
