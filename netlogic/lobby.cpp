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
	m_hosting = false;
	m_hostOrJoin = NULL;
	m_globalGame = NULL;
}

bool
LobbyDialogDelegate::OnLoad()
{
	// Get the addresses for this machine
	IPaddress addresses[32];
	int i, count;

	count = SDLNet_GetLocalAddresses(addresses, SDL_arraysize(addresses));
	m_addresses.clear();
	for (i = 0; i < count; ++i) {
		m_addresses.add(addresses[i]);
	}

	// Get the address of the global server
	if (SDLNet_ResolveHost(&m_globalServer, GLOBAL_SERVER_HOST, LOBBY_PORT) < 0) {
		fprintf(stderr, "Warning: Couldn't resolve global server host %s\n", GLOBAL_SERVER_HOST);
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

	return true;
}

void
LobbyDialogDelegate::OnShow()
{
	// Start up networking
	SetHostOrJoin(0, m_hostOrJoin->GetValue());
}

void
LobbyDialogDelegate::OnHide()
{
	if (m_dialog->GetDialogStatus() > 0) {
		NewGame();
	}

	// Shut down networking
	HaltNetData();
}

void
LobbyDialogDelegate::OnTick()
{
	if (m_hostOrJoin->GetValue() <= 0) {
		// Neither host nor join is checked
		return;
	}

	Uint32 now = SDL_GetTicks();
	if (!m_lastRefresh ||
	    (now - m_lastRefresh) > GLOBAL_CHECK_INTERVAL) {
		if (m_globalGame->IsChecked()) {
			if (m_hosting) {
				AdvertiseGame();
			} else {
				GetGameList();
			}
		}
		m_lastRefresh = now;
	}
}

void
LobbyDialogDelegate::SetHostOrJoin(void*, int value)
{
	// This is called when the lobby switches from hosting to joining
	HaltNetData();
	ClearGameList();

	if (value > 0) {
		m_hosting = (value == HOST_GAME);
		if (InitNetData(m_hosting) < 0) {
			m_hostOrJoin->SetValue(0);
		}
		m_lastRefresh = 0;
	}
}

void
LobbyDialogDelegate::GlobalGameChanged(void*)
{
	m_lastRefresh = 0;

	if (!m_globalGame->IsChecked()) {
		if (m_hosting) {
			RemoveGame();
		} else {
			ClearGameList();
		}
	}
}

void
LobbyDialogDelegate::AdvertiseGame()
{
	m_packet.Reset();
	m_packet.Write((Uint8)LOBBY_ANNOUNCE_GAME);
	PackAddresses(m_packet);
	m_packet.address = m_globalServer;

	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::RemoveGame()
{
	m_packet.Reset();
	m_packet.Write((Uint8)LOBBY_REMOVE_GAME);
	PackAddresses(m_packet);
	m_packet.address = m_globalServer;

	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::GetGameList()
{
	m_packet.Reset();
	m_packet.Write((Uint8)LOBBY_REQUEST_GAME_SERVERS);
	PackAddresses(m_packet);
	m_packet.address = m_globalServer;

	SDLNet_UDP_Send(gNetFD, -1, &m_packet);
}

void
LobbyDialogDelegate::ClearGameList()
{
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
