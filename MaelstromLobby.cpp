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

#include "netlogic/protocol.h"

#define MAX_PACKET_SIZE	1024

void ProcessPacket(UDPpacket *packet)
{
	printf("Received packet from %s\n", SDLNet_ResolveIP(&packet->address));
}

int main(int argc, char *argv[])
{
	UDPsocket sock;
	UDPpacket *packet;

	sock = SDLNet_UDP_Open(LOBBY_PORT);
	if (!sock) {
		fprintf(stderr, "Couldn't create socket on port %d: %s\n",
			LOBBY_PORT, SDL_GetError());
		exit(1);
	}

	packet = SDLNet_AllocPacket(MAX_PACKET_SIZE);
	if (!packet) {
		fprintf(stderr, "Couldn't allocate packet of size %d: %s\n",
			MAX_PACKET_SIZE, SDL_GetError());
		SDLNet_UDP_Close(sock);
		exit(1);
	}

	for ( ; ; ) {
		while (SDLNet_UDP_Recv(sock, packet)) {
			ProcessPacket(packet);
		}

		SDL_Delay(100);
	}
	SDLNet_UDP_Close(sock);
}
