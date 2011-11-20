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

/* This contains the network play functions and data */

#include <stdlib.h>

#include "SDL_net.h"
#include "SDL_endian.h"

#include "Maelstrom_Globals.h"
#include "netplay.h"
#include "protocol.h"

// Set this to 1 for normal debug info, and 2 for verbose packet logging
#define DEBUG_NETWORK 1

// Define this to simulate packet loss
//#define DEBUG_PACKETLOSS 10


UDPsocket gNetFD;

static SDLNet_SocketSet SocketSet;
static Uint32           NextFrame;

/* We keep one packet backlogged for retransmission */
static DynamicPacket OutBound[2];
static int           CurrOut;
#define CurrPacket	OutBound[CurrOut]
#define LastPacket	OutBound[!CurrOut]

/* We cache one packet if the other player is ahead of us */
static DynamicPacket Packet;
static DynamicPacket CachedPacket[MAX_NODES];

/* When we're done we have our input for the frame */
static DynamicPacket QueuedInput;
static DynamicPacket FrameInput;


int InitNetData(bool hosting)
{
	int i;
	int port;

	/* Initialize the networking subsystem */
	if ( SDLNet_Init() < 0 ) {
		error("Couldn't initialize networking: %s\n", SDLNet_GetError());
		return(-1);
	}

	/* Oh heck, create the UDP socket here... */
	if (hosting) {
		port = NETPLAY_PORT;
	} else {
		port = 0;
	}
	gNetFD = SDLNet_UDP_Open(port);
	if ( gNetFD == NULL ) {
		error("Couldn't create socket bound to port %d: %s\n", port, SDLNet_GetError());
		return(-1);
	}
	SocketSet = SDLNet_AllocSocketSet(1);
	if ( SocketSet == NULL ) {
		error("Couldn't create socket watch set\n");
		return(-1);
	}
	SDLNet_UDP_AddSocket(SocketSet, gNetFD);

#ifdef DEBUG_PACKETLOSS
	SDLNet_UDP_SetPacketLoss(gNetFD, DEBUG_PACKETLOSS);
#endif

	/* Initialize network game variables */
	NextFrame = 0;
	OutBound[0].Reset();
	OutBound[1].Reset();
	CurrOut = 0;
	for (i = 0; i < MAX_NODES; ++i) {
		CachedPacket[i].Reset();
	}
	QueuedInput.Reset();
	FrameInput.Reset();

	return(0);
}

void HaltNetData(void)
{
	if (SocketSet) {
		SDLNet_FreeSocketSet(SocketSet);
		SocketSet = NULL;
	}

	if (gNetFD) {
		SDLNet_UDP_Close(gNetFD);
		gNetFD = NULL;
	}

	SDLNet_Quit();
}

int CheckPlayers(void)
{
	int i;

	if (gGameInfo.GetNumPlayers() == 0) {
		error("No players specified!\r\n");
		return(-1);
	}

	bool foundLocalPlayer = false;
	for (i = 0; i < MAX_PLAYERS; ++i) {
		if (gGameInfo.IsLocalPlayer(i)) {
			foundLocalPlayer = true;
			break;
		}
	}
	if (!foundLocalPlayer) {
		error("Which player are you?\r\n");
		return(-1);
	}

	/* Bind all of our network nodes to the broadcast channel */
	for (i = 0; i < MAX_NODES; ++i) {
		if (gGameInfo.IsNetworkNode(i)) {
			const GameInfoNode *node = gGameInfo.GetNode(i);
			SDLNet_UDP_Bind(gNetFD, 0, &node->address);
			SDLNet_UDP_Bind(gNetFD, 1+i, &node->address);
		}
	}

	return(0);
}

void QueueInput(Uint8 value)
{
	QueuedInput.Write(value);
}

static bool ProcessSync(int index, DynamicPacket &packet)
{
	Uint32 seed;
	Uint8 state;

	if (!packet.Read(seed) || !packet.Read(state)) {
		error("Received short packet\r\n");
		return false;
	}

	if (seed != GetRandSeed()) {
		/* We're hosed, to correct this we would have to sync the complete game state */
		error("Error!! \a Frame consistency problem, aborting!!\r\n");
		return false;
	}

	gGameInfo.SetNodeState(index, state);

	// Should we validate that the input is for players from this node?
	//        ... nah... :)
	FrameInput.Write(packet);

	return true;
}

static int AwaitSync(int nleft, Uint32 waiting[MAX_NODES])
{
	int i;
	int timeout;
	Uint32 frame;

	// See if we have cached network packets
	for (i = 0; i < MAX_NODES; ++i) {
		if (CachedPacket[i].len > 0 && waiting[i]) {
			if (!ProcessSync(i, CachedPacket[i])) {
				return -1;
			}
			waiting[i] = 0;
			--nleft;
		}
		CachedPacket[i].Reset();
	}

	/* Wait for Ack's */
	timeout = 0;
	while ( nleft ) {
		int ready = SDLNet_CheckSockets(SocketSet, 100);
		if (ready < 0) {
			error("Network error: SDLNet_CheckSockets()\r\n");
			return(-1);
		}
		if (ready == 0) {
#if DEBUG_NETWORK >= 1
error("Timed out waiting for frame %ld\r\n", NextFrame);
#endif
			/* Timeout, resend the sync packet */
			for (i = 0; i < MAX_NODES; ++i) {
				if (waiting[i]) {
					SDLNet_UDP_Send(gNetFD, i+1, &CurrPacket);
				}
			}

			/* Don't wait forever */
			++timeout;
			if ( timeout == (PING_TIMEOUT/100) ) {
				return(-1);
			}
		}
		if ( ready <= 0 ) {
			continue;
		}

		/* We are guaranteed that there is data here */
		Packet.Reset();
		if ( SDLNet_UDP_Recv(gNetFD, &Packet) <= 0 ) {
			error("Network error: SDLNet_UDP_Recv()\r\n");
			return(-1);
		}

		/* We have a packet! */
		Uint8 cmd;
		if (!Packet.Read(cmd)) {
			error("Received short packet\r\n");
			continue;
		}
		if (cmd == LOBBY_MSG ) {
#if DEBUG_NETWORK >= 2
error("LOBBY_MSG packet!\r\n");
#endif
			continue;
		}
		if (cmd == NEW_GAME ) {
#if DEBUG_NETWORK >= 1
error("NEW_GAME packet!\r\n");
#endif
			Packet.Reset();
			Packet.Write((Uint8)NEW_GAME_ACK);
			Packet.Write(gGameInfo.gameID);
			Packet.Write(gGameInfo.localID);
			SDLNet_UDP_Send(gNetFD, -1, &Packet);
			continue;
		}
		if (cmd != SYNC_MSG) {
			error("Unknown packet: 0x%x\r\n", cmd);
			continue;
		}

		Uint32 gameID;
		Uint32 nodeID;
		if (!Packet.Read(gameID) || !Packet.Read(nodeID)) {
			error("Received short packet\r\n");
			continue;
		}
		if (gameID != gGameInfo.gameID) {
			/* This must be for a different game */
			continue;
		}
		int index = gGameInfo.GetNodeIndex(nodeID);
		if (index < 0) {
			error("Warning: packet from unknown source\r\n");
			continue;
		}

		/* Check the frame number */
		if (!Packet.Read(frame)) {
			error("Received short packet\r\n");
			continue;
		}
#if DEBUG_NETWORK >= 2
error("Received a packet of frame %lu from player %d\r\n", frame, index+1);
#endif
		if (frame == NextFrame) {
			/* Ignore it if it is a duplicate packet */
			if (!waiting[index]) {
#if DEBUG_NETWORK >= 1
error("Ignoring duplicate packet for frame %lu from player %d\r\n", frame, index+1);
#endif
				continue;
			}

			/* Do a consistency check!! */
			if (!ProcessSync(index, Packet)) {
				return -1;
			}
			waiting[index] = 0;
			--nleft;
		} else if (frame == (NextFrame-1)) {
			/* We kept the last frame cached, so send it */
#if DEBUG_NETWORK >= 1
error("Transmitting packet for old frame (%lu)\r\n", frame);
#endif
			LastPacket.address = Packet.address;
			SDLNet_UDP_Send(gNetFD, -1, &LastPacket);
		} else if (frame == (NextFrame+1)) {
#if DEBUG_NETWORK >= 1
error("Received packet for next frame! (%lu, current = %lu)\r\n",
					frame, NextFrame);
#endif
			/* Send this player our current frame */
			CurrPacket.address = Packet.address;
			SDLNet_UDP_Send(gNetFD, -1, &CurrPacket);

			/* Cache this frame for next round */
			CachedPacket[index].Reset();
			CachedPacket[index].Write(Packet);
			CachedPacket[index].Seek(0);
		}
#if DEBUG_NETWORK >= 1
else
error("Received packet for really old frame! (%lu, current = %lu)\r\n",
							frame, NextFrame);
#endif
	}
	return 0;
}

/* This function is called every frame, and is used to flush the network
   buffers, sending sync and keystroke packets.
   It is called AFTER the keyboard is polled, and BEFORE GetSyncBuf() is
   called by the player objects.

   Note:  We assume that FastRand() isn't called by an interrupt routine,
          otherwise we lose consistency.
*/
	
int SyncNetwork(void)
{
	int i;
	int nleft;
	Uint32 waiting[MAX_NODES];

	// Get the queued input
	FrameInput.Reset();
	QueuedInput.Seek(0);
	FrameInput.Write(QueuedInput);

	// See if we need to do network synchronization
	nleft = 0;
	for (i = 0; i < gGameInfo.GetNumNodes(); ++i) {
		if (gGameInfo.IsNetworkNode(i)) {
			++nleft;
			waiting[i] = gGameInfo.GetNode(i)->nodeID;
		} else {
			waiting[i] = 0;
		}
	}
	if (nleft > 0) {
		// Create the sync packet
		CurrPacket.Reset();
		CurrPacket.Write((Uint8)SYNC_MSG);
		CurrPacket.Write(gGameInfo.gameID);
		CurrPacket.Write(gGameInfo.localID);
		CurrPacket.Write(NextFrame);
		CurrPacket.Write(GetRandSeed());
		CurrPacket.Write(gGameInfo.GetLocalState());
		QueuedInput.Seek(0);
		CurrPacket.Write(QueuedInput);

		// Send the packet to all the players
		SDLNet_UDP_Send(gNetFD, 0, &CurrPacket);

		// Wait for sync packets from them
		if (AwaitSync(nleft, waiting) < 0) {
			return -1;
		}

		CurrOut = !CurrOut;
	}

	QueuedInput.Reset();
	++NextFrame;

	return(0);
}

/* This function retrieves the input for the frame */
int GetSyncBuf(Uint8 **bufptr)
{
	*bufptr = FrameInput.data;
	return FrameInput.len;
}

/* This function sends a NEW_GAME packet, and waits for all other players
   to respond with NEW_GAME_ACK
*/
int Send_NewGame()
{
	char message[BUFSIZ];
	int  nleft;
	Uint32 waiting[MAX_NODES];
	int  i, j;
	DynamicPacket newgame;

	/* Send all the packets */
	newgame.Write((Uint8)NEW_GAME);
	gGameInfo.WriteToPacket(newgame);
	SDLNet_UDP_Send(gNetFD, 0, &newgame);

	/* Get ready for responses */
	nleft = 0;
	for (i = 0; i < gGameInfo.GetNumNodes(); ++i) {
		if (gGameInfo.IsNetworkNode(i)) {
			++nleft;
			waiting[i] = gGameInfo.GetNode(i)->nodeID;
		} else {
			waiting[i] = 0;
		}
	}

	/* Wait for Ack's */
	while (nleft > 0) {
		/* Show a status */
		strcpy(message, "Waiting for players:");
		for (i = 0; i < MAX_PLAYERS; ++i) {
			const GameInfoPlayer *player = gGameInfo.GetPlayer(i);
			for (j = 0; j < MAX_NODES; ++j) {
				if (player->nodeID == waiting[j]) {
					sprintf(&message[strlen(message)], " %d", i+1);
					break;
				}
			}
		}
		Message(message);

		int ready = SDLNet_CheckSockets(SocketSet, 100);
		if (ready < 0) {
			error("Network error: SDLNet_CheckSockets()\r\n");
			return(-1);
		}
		if (ready == 0) {
			for (i = 0; i < MAX_NODES; ++i) {
				if (waiting[i]) {
					SDLNet_UDP_Send(gNetFD, i+1, &newgame);
				}
			}
			continue;
		}

		/* We are guaranteed that there is data here */
		Packet.Reset();
		if ( SDLNet_UDP_Recv(gNetFD, &Packet) <= 0 ) {
			error("Network error: SDLNet_UDP_Recv()\r\n");
			return(-1);
		}

		/* We have a packet! */
		Uint8 cmd;
		Uint32 gameID;
		Uint32 nodeID;
		if (!Packet.Read(cmd) || cmd != NEW_GAME_ACK) {
			/* Continue waiting */
			continue;
		}
		if (!Packet.Read(gameID) || !Packet.Read(nodeID)) {
			continue;
		}
		if (gameID != gGameInfo.gameID) {
			/* This must be for a different game */
			continue;
		}
		if (!nodeID) {
			continue;
		}
		for (i = 0; i < MAX_NODES; ++i) {
			if (nodeID == waiting[i]) {
				waiting[i] = 0;
				--nleft;
				break;
			}
		}
	}
	return(0);
}
