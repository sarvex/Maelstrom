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

#include <stdlib.h>
#include <time.h>

#include "SDL_net.h"
#include "../Maelstrom_Globals.h"
#include "../screenlib/UIElementCheckbox.h"
#include "../screenlib/UIElementRadio.h"
#include "lobby.h"
#include "protocol.h"
#include "netplay.h"

// Update the game list every 3 seconds
//#define GLOBAL_SERVER_HOST	"obelix.dreamhost.com"
#define GLOBAL_SERVER_HOST	"localhost"
#define GLOBAL_CHECK_INTERVAL	3000


LobbyDialogDelegate::LobbyDialogDelegate(UIPanel *panel) :
	UIDialogDelegate(panel)
{
	m_state = STATE_NONE;
	m_uniqueID = 0;
	m_lastRefresh = 0;
	m_requestSequence = 1;
}

bool
LobbyDialogDelegate::OnLoad()
{
	int i, count;
	IPaddress addresses[32];
	char name[32];

	// Get the address of the global server
	if (SDLNet_ResolveHost(&m_globalServer, GLOBAL_SERVER_HOST, LOBBY_PORT) < 0) {
		fprintf(stderr, "Warning: Couldn't resolve global server host %s\n", GLOBAL_SERVER_HOST);
	}

	// Get the addresses for this machine
	count = SDLNet_GetLocalAddresses(addresses, SDL_arraysize(addresses));
	m_addresses.clear();
	for (i = 0; i < count; ++i) {
		m_addresses.add(addresses[i]);
	}

	m_hostOrJoin = m_dialog->GetElement<UIElementRadioGroup>("hostOrJoin");
	if (!m_hostOrJoin) {
		fprintf(stderr, "Warning: Couldn't find radio group 'hostOrJoin'\n");
		return false;
	}
	m_hostOrJoin->SetValueCallback(this, &LobbyDialogDelegate::SetHostOrJoin);

	m_globalGame = m_dialog->GetElement<UIElementCheckbox>("globalGame");
	if (!m_globalGame) {
		fprintf(stderr, "Warning: Couldn't find checkbox 'globalGame'\n");
		return false;
	}
	if (m_globalServer.host == INADDR_NONE) {
		m_globalGame->SetChecked(false);
		m_globalGame->SetDisabled(true);
	}
	m_globalGame->SetClickCallback(this, &LobbyDialogDelegate::GlobalGameChanged);

	if (!GetElement("gamelist", m_gameListArea)) {
		return false;
	}
	if (!GetElement("gameinfo", m_gameInfoArea)) {
		return false;
	}
	if (!GetElement("playButton", m_playButton)) {
		return false;
	}

	count = SDL_arraysize(m_gameListElements);
	for (i = 0; i < count; ++i) {
		SDL_snprintf(name, sizeof(name), "game%d", i+1);
		if (!GetElement(name, m_gameListElements[i])) {
			return false;
		}

		UIElement *button = m_gameListElements[i]->GetElement<UIElement>("join");
		if (button) {
			button->SetClickCallback(this, &LobbyDialogDelegate::JoinGameClicked, m_gameListElements[i]);
		}
	}

	count = SDL_arraysize(m_gameInfoPlayers);
	for (i = 0; i < count; ++i) {
		SDL_snprintf(name, sizeof(name), "player%d", i+1);
		if (!GetElement(name, m_gameInfoPlayers[i])) {
			return false;
		}
	}

	return true;
}

bool
LobbyDialogDelegate::GetElement(const char *name, UIElement *&element)
{
	element = m_dialog->GetElement<UIElement>(name);
	if (!element) {
		fprintf(stderr, "Warning: Couldn't find element '%s'\n", name);
		return false;
	}
	return true;
}

void
LobbyDialogDelegate::OnShow()
{
	// Seed the random number generator for our unique ID
	srand(time(NULL)+SDL_GetTicks());

	// Start up networking
	SetHostOrJoin(0, m_hostOrJoin->GetValue());
}

void
LobbyDialogDelegate::OnHide()
{
	// Start the game!
	if (m_dialog->GetDialogStatus() > 0) {
		NewGame();
	}

	// Shut down networking
	HaltNetData();
}

void
LobbyDialogDelegate::OnTick()
{
	if (m_state == STATE_NONE) {
		// Neither host nor join is checked
		return;
	}

	Uint32 now = SDL_GetTicks();
	if (!m_lastRefresh ||
	    (now - m_lastRefresh) > GLOBAL_CHECK_INTERVAL) {
		if (m_state == STATE_HOSTING) {
			AdvertiseGame();
		} else if (m_state == STATE_LISTING) {
			GetGameList();
		} else {
			GetGameInfo();
		}
		m_lastRefresh = now;
	}

	// See if there are any packets on the network
	m_packet.Reset();
	while (SDLNet_UDP_Recv(gNetFD, &m_packet)) {
		ProcessPacket(m_packet);
		m_packet.Reset();
	}
}

void
LobbyDialogDelegate::SetHostOrJoin(void*, int value)
{
	// This is called when the lobby switches from hosting to joining
	HaltNetData();
	ClearGameInfo();
	ClearGameList();

	if (value > 0) {
		if (InitNetData(value == HOST_GAME) < 0) {
			m_hostOrJoin->SetValue(2);
			return;
		}

		if (value == HOST_GAME) {
			m_state = STATE_HOSTING;
		} else {
			m_state = STATE_LISTING;
		}
		m_uniqueID = rand();
		m_lastRefresh = 0;

		if (m_state == STATE_HOSTING) {
			m_game.SetHostInfo(m_uniqueID, prefs->GetString(PREFERENCES_HANDLE));
		}
		m_game.SetLocalID(m_uniqueID);
	} else {
		m_state = STATE_NONE;
	}

	UpdateUI();
}

void
LobbyDialogDelegate::GlobalGameChanged(void*)
{
	m_lastRefresh = 0;

	if (!m_globalGame->IsChecked()) {
		if (m_state == STATE_HOSTING) {
			RemoveGame();
		} else {
			ClearGameList();
		}
	}
}

void
LobbyDialogDelegate::JoinGameClicked(void *_element)
{
	UIElement *element = (UIElement *)_element;
	for (int i = 0; (unsigned)i < SDL_arraysize(m_gameListElements); ++i) {
		if (element == m_gameListElements[i]) {
			// We found the one that was clicked!
			JoinGame(m_gameList[i]);
			break;
		}
	}
}

void
LobbyDialogDelegate::UpdateUI()
{
	if (m_state == STATE_NONE) {
		m_gameListArea->Hide();
		m_gameInfoArea->Hide();
	} else if (m_state == STATE_LISTING) {
		m_gameListArea->Show();
		m_gameInfoArea->Hide();
		for (int i = 0; (unsigned)i < SDL_arraysize(m_gameListElements); ++i) {
			if (i < m_gameList.length()) {
				m_gameListElements[i]->Show();
				m_gameList[i].BindPlayerToUI(0, m_gameListElements[i]);
			} else {
				m_gameListElements[i]->Hide();
			}
		}
	} else {
		m_gameInfoArea->Show();
		m_gameListArea->Hide();
		for (int i = 0; i < MAX_PLAYERS; ++i) {
			m_game.BindPlayerToUI(i, m_gameInfoPlayers[i]);
		}
	}
	if (m_state == STATE_HOSTING) {
		m_playButton->SetDisabled(false);
	} else {
		m_playButton->SetDisabled(true);
	}
}

void
LobbyDialogDelegate::AdvertiseGame()
{
	if (m_globalGame->IsChecked()) {
		m_packet.StartLobbyMessage(LOBBY_ANNOUNCE_GAME);
		PackAddresses(m_packet);
		m_packet.address = m_globalServer;

		SDLNet_UDP_Send(gNetFD, -1, &m_packet);
	}
}

void
LobbyDialogDelegate::RemoveGame()
{
	m_packet.StartLobbyMessage(LOBBY_REMOVE_GAME);
	PackAddresses(m_packet);
	m_packet.address = m_globalServer;

	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::GetGameList()
{
	if (m_globalGame->IsChecked()) {
		m_packet.StartLobbyMessage(LOBBY_REQUEST_GAME_SERVERS);
		PackAddresses(m_packet);
		m_packet.address = m_globalServer;

		SDLNet_UDP_Send(gNetFD, -1, &m_packet);
	}

	// Get game info for local games
	m_packet.StartLobbyMessage(LOBBY_REQUEST_GAME_INFO);
	m_packet.address.host = INADDR_BROADCAST;
	m_packet.address.port = SDL_SwapBE16(NETPLAY_PORT);
	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::GetGameInfo()
{
	m_packet.StartLobbyMessage(LOBBY_REQUEST_GAME_INFO);
	m_packet.address = m_game.players[0].address;
	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::JoinGame(GameInfo &game)
{
	m_packet.StartLobbyMessage(LOBBY_REQUEST_JOIN);
	m_packet.Write(game.gameID);
	m_packet.Write(m_uniqueID);
	m_packet.Write(prefs->GetString(PREFERENCES_HANDLE));
	m_packet.address = game.players[0].address;;

	SDLNet_UDP_Send(gNetFD, -1, &m_packet);

	m_game.CopyFrom(game);
	m_state = STATE_JOINING;
	UpdateUI();
}

void
LobbyDialogDelegate::ClearGameInfo()
{
	m_game.Reset();
}

void
LobbyDialogDelegate::ClearGameList()
{
	m_gameList.clear();
}

void
LobbyDialogDelegate::PackAddresses(DynamicPacket &packet)
{
	Uint16 port;

	port = SDLNet_UDP_GetPeerAddress(gNetFD, -1)->port;

	m_packet.Write((Uint8)m_addresses.length());
	for (int i = 0; i < m_addresses.length(); ++i) {
		m_packet.Write(m_addresses[i].host);
		m_packet.Write(port);
	}
}

void
LobbyDialogDelegate::ProcessPacket(DynamicPacket &packet)
{
	Uint8 cmd;

	if (!m_packet.Read(cmd)) {
		return;
	}
	if (cmd != LOBBY_MSG) {
		return;
	}
	if (!m_packet.Read(cmd)) {
		return;
	}

	if (m_state == STATE_HOSTING) {
		if (cmd == LOBBY_ANNOUNCE_PLAYER) {
			if (m_globalGame->IsChecked()) {
				ProcessAnnouncePlayer(packet);
			}
			return;
		}

		if (m_game.IsFull() && !m_game.HasPlayer(packet.address)) {
			return;
		}

		if (cmd == LOBBY_PING) {
			//ProcessPing(packet);
		} else if (cmd == LOBBY_REQUEST_GAME_INFO) {
			ProcessRequestGameInfo(packet);
		} else if (cmd == LOBBY_REQUEST_JOIN) {
			ProcessRequestJoin(packet);
		} else if (cmd == LOBBY_REQUEST_LEAVE) {
			//ProcessRequestLeave(packet);
		}
		return;

	}

	if (m_state == STATE_LISTING) {
		if (cmd == LOBBY_GAME_SERVERS) {
			if (m_globalGame->IsChecked()) {
				ProcessGameServerList(packet);
			}
			return;
		}

	}

	// These packets we handle in all the join states
	if (cmd == LOBBY_PING) {
		// Somebody thinks we're still in a game lobby
		//RejectPing(packet);
	} else if (cmd == LOBBY_GAME_INFO) {
		ProcessGameInfo(packet);
	}
}

void
LobbyDialogDelegate::ProcessAnnouncePlayer(DynamicPacket &packet)
{
	Uint8 count;
	IPaddress address;

	// Open the firewall so this player can contact us.
	m_reply.StartLobbyMessage(LOBBY_OPEN_FIREWALL);

	if (!packet.Read(count)) {
		return;
	}
	for (Uint8 i = 0; i < count; ++i) {
		if (!packet.Read(address.host) ||
		    !packet.Read(address.port)) {
			return;
		}
		m_reply.address = address;
		
		SDLNet_UDP_Send(gNetFD, -1, &m_reply);
	}
}

void
LobbyDialogDelegate::ProcessRequestGameInfo(DynamicPacket &packet)
{
	m_reply.StartLobbyMessage(LOBBY_GAME_INFO);
	m_game.WriteToPacket(m_reply);
	m_reply.address = packet.address;

	SDLNet_UDP_Send(gNetFD, -1, &m_reply);
}

void
LobbyDialogDelegate::ProcessRequestJoin(DynamicPacket &packet)
{
	Uint32 gameID;
	Uint32 playerID;
	char name[MAX_NAMELEN+1];

	if (!packet.Read(gameID) || gameID != m_game.gameID) {
		return;
	}
	if (!packet.Read(playerID)) {
		return;
	}
	if (!packet.Read(name, sizeof(name))) {
		return;
	}

	// Find an empty slot
	int slot;
	for (slot = 0; slot < MAX_PLAYERS; ++slot) {
		if (playerID == m_game.players[slot].playerID) {
			// We already have this player, just update it.
			break;
		}
	}
	if (slot == MAX_PLAYERS) {
		for (slot = 0; slot < MAX_PLAYERS; ++slot) {
			if (!m_game.players[slot].playerID) {
				break;
			}
		}
	}
	assert(slot < MAX_PLAYERS);

	// Fill in the data
	GameInfo::GameInfoPlayer *player = &m_game.players[slot];
	player->playerID = playerID;
	player->address = packet.address;
	SDL_strlcpy(player->name, name, sizeof(player->name));

	// Let everybody know!
	m_reply.StartLobbyMessage(LOBBY_GAME_INFO);
	m_game.WriteToPacket(m_reply);
	for (slot = 0; slot < MAX_PLAYERS; ++slot) {
		Uint32 playerID = m_game.players[slot].playerID;
		if (playerID && playerID != m_uniqueID) {
			m_reply.address = m_game.players[slot].address;
			SDLNet_UDP_Send(gNetFD, -1, &m_reply);
		}
	}

	// Update our own UI
	UpdateUI();
}

void
LobbyDialogDelegate::ProcessGameInfo(DynamicPacket &packet)
{
	GameInfo game;

	if (!game.ReadFromPacket(packet)) {
		return;
	}

	if (m_state != STATE_LISTING) {
		if (game.gameID != m_game.gameID) {
			// Probably an old packet...
			return;
		}

		m_game.CopyFrom(game);

		if (m_state == STATE_JOINING) {
			if (m_game.HasPlayer(m_uniqueID)) {
				// We successfully joined the game
				m_state = STATE_JOINED;
			}
		} else {
			if (!m_game.HasPlayer(m_uniqueID)) {
				// We were kicked from the game
				m_state = STATE_LISTING;
			}
		}
	}

	if (m_state == STATE_LISTING) {
		// Add or update the game list
		int i;
		for (int i = 0; i < m_gameList.length(); ++i) {
			if (game.gameID == m_gameList[i].gameID) {
				m_gameList[i].CopyFrom(game);
				break;
			}
		}
		if (i == m_gameList.length()) {
			m_gameList.add(game);
		}
	}

	UpdateUI();
}

void
LobbyDialogDelegate::ProcessGameServerList(DynamicPacket &packet)
{
	Uint8 serverCount, count;
	IPaddress address;

	// Request game information from the servers
	m_reply.StartLobbyMessage(LOBBY_REQUEST_GAME_INFO);

	if (!packet.Read(serverCount)) {
		return;
	}
	for (Uint8 i = 0; i < serverCount; ++i) {
		if (!packet.Read(count)) {
			return;
		}
		for (Uint8 j = 0; j < count; ++j) {
			if (!packet.Read(address.host) ||
			    !packet.Read(address.port)) {
				return;
			}
			m_reply.address = address;
			
			SDLNet_UDP_Send(gNetFD, -1, &m_reply);
		}
	}
}
