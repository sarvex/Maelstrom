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


int   gNumPlayers;
int   gOurPlayer;
UDPsocket gNetFD;

static IPaddress      PlayAddr[MAX_PLAYERS];
static Uint32         NextFrame;
UDPpacket            *OutBound[2];
static int            CurrOut;
/* This is the data offset of a SYNC packet */
#define PDATA_OFFSET	(1+1+sizeof(Uint32)+sizeof(Uint32))

/* We keep one packet backlogged for retransmission */
#define OutBuf		OutBound[CurrOut]->data
#define OutLen		OutBound[CurrOut]->len
#define LastBuf		OutBound[!CurrOut]->data
#define LastLen		OutBound[!CurrOut]->len

static unsigned char *SyncPtrs[2][MAX_PLAYERS];
static unsigned char  SyncBufs[2][MAX_PLAYERS][BUFSIZ];
static int            SyncLens[2][MAX_PLAYERS];
static int            ThisSyncs[2];
static int            CurrIn;
static SDLNet_SocketSet SocketSet;

/* We cache one packet if the other player is ahead of us */
#define SyncPtr		SyncPtrs[CurrIn]
#define SyncBuf		SyncBufs[CurrIn]
#define SyncLen		SyncLens[CurrIn]
#define ThisSync	ThisSyncs[CurrIn]
#define NextPtr		SyncPtrs[!CurrIn]
#define NextBuf		SyncBufs[!CurrIn]
#define NextLen		SyncLens[!CurrIn]
#define NextSync	ThisSyncs[!CurrIn]

#define TOGGLE(var)	var = !var


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

	/* Create the outbound packets */
	for ( i=0; i<2; ++i ) {
		OutBound[i] = SDLNet_AllocPacket(BUFSIZ);
		if ( OutBound[i] == NULL ) {
			error("Out of memory (creating network buffers)\n");
			return(-1);
		}
	}

	/* Initialize network game variables */
	gOurPlayer  = -1;
	NextFrame = 0;
	for ( i=0; i<MAX_PLAYERS; ++i ) {
		SyncPtrs[0][i] = NULL;
		SyncPtrs[1][i] = NULL;
	}
	OutBound[0]->data[0] = SYNC_MSG;
	OutBound[1]->data[0] = SYNC_MSG;
	/* Type field, frame sequence, current random seed */
	OutBound[0]->len = PDATA_OFFSET;
	OutBound[1]->len = PDATA_OFFSET;
	CurrOut = 0;

	ThisSyncs[0] = 0;
	ThisSyncs[1] = 0;
	CurrIn = 0;
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

/* This MUST be called after command line options have been processed. */
int CheckPlayers(void)
{
	int i;

	if (gGameInfo.GetNumPlayers() == 0) {
		error("No players specified!\r\n");
		return(-1);
	}

	gNumPlayers = gGameInfo.GetNumPlayers();
	gOurPlayer  = -1;
	for ( i=0; i<MAX_PLAYERS; ++i ) {
		if (gGameInfo.IsValidPlayer(i)) {
			if (gGameInfo.IsLocalPlayer(i)) {
				if (gOurPlayer < 0) {
					gOurPlayer = i;
				}
			} else {
				PlayAddr[i] = gGameInfo.GetPlayerAddress(i);
			}
		}
	}
	if (gOurPlayer < 0) {
		error("Which player are you?\r\n");
		return(-1);
	}

	/* Now, so we can send to ourselves... */
	PlayAddr[gOurPlayer] = *SDLNet_UDP_GetPeerAddress(gNetFD, -1);
	if ( ! PlayAddr[gOurPlayer].host ) {
		SDLNet_ResolveHost(&PlayAddr[gOurPlayer], "127.0.0.1", SDL_SwapBE16(PlayAddr[gOurPlayer].port));
	}

	/* Bind all of our players to the channels */
	for ( i=0; i<gNumPlayers; ++i ) {
		SDLNet_UDP_Bind(gNetFD, 0, &PlayAddr[i]);
		SDLNet_UDP_Bind(gNetFD, i+1, &PlayAddr[i]);
	}
	return(0);
}

void QueueKey(unsigned char Op, unsigned char Type)
{
	/* Drop keys on a full buffer (assumed never to really happen) */
	if ( OutLen >= (BUFSIZ-2) )
		return;

//error("Queued key 0x%.2x for frame %d\r\n", Type, NextFrame);
	OutBuf[OutLen++] = Op;
	OutBuf[OutLen++] = Type;
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
	UDPpacket sent;
	Uint32 seed, frame;
	unsigned char buf[BUFSIZ];
	int index, nleft;
	int timeout;

	/* Set the next inbound packet buffer */
	TOGGLE(CurrIn);

	/* Set the frame number */
	frame = NextFrame;
	SDLNet_Write32(frame, &OutBuf[1]);
	seed = GetRandSeed();
	SDLNet_Write32(seed, &OutBuf[1+sizeof(frame)]);

	/* Send the packet to all the players */
	SDLNet_UDP_Send(gNetFD, 0, OutBound[CurrOut]);
	for ( nleft=0, index=0; index<gNumPlayers; ++index ) {
		if ( SyncPtr[index] == NULL ) {
			++nleft;
		}
	}
	NextSync = 0;

	/* Get the inbound packet ready for data */
	sent.data = buf;
	sent.maxlen = sizeof(buf);

	/* Wait for Ack's */
	timeout = 0;
	while ( nleft ) {
		int ready = SDLNet_CheckSockets(SocketSet, 100);
		if ( ready < 0 ) {
			error("Network error: SDLNet_CheckSockets()");
			return(-1);
		}
		if ( ready == 0 ) {
			error("Timed out waiting for frame %ld\r\n", NextFrame);

			/* Timeout, resend the sync packet */
			for ( index=0; index<gNumPlayers; ++index ) {
				if ( SyncPtr[index] == NULL ) {
					SDLNet_UDP_Send(gNetFD, index+1, OutBound[CurrOut]);
				}
			}

			/* Don't wait forever */
			++timeout;
			if ( timeout == (PING_TIMEOUT/100) ) {
				// Reset our current input state
				TOGGLE(CurrIn);
				return(-1);
			}
		}
		if ( ready <= 0 ) {
			continue;
		}

		/* We are guaranteed that there is data here */
		if ( SDLNet_UDP_Recv(gNetFD, &sent) <= 0 ) {
			error("Network error: SDLNet_UDP_Recv()");
			return(-1);
		}
//error("Received packet!\r\n");

		/* We have a packet! */
		if ( buf[0] == LOBBY_MSG ) {
			continue;
		}
		if ( buf[0] == NEW_GAME ) {
			/* FIXME: Convert this to the DynamicPacket */
			buf[0] = NEW_GAME_ACK;
			SDLNet_Write32(gGameInfo.gameID, &buf[1]);
			SDLNet_Write32(gGameInfo.localID, &buf[4]);
			sent.len = 9;
			SDLNet_UDP_Send(gNetFD, -1, &sent);
//error("NEW_GAME packet!\r\n");
			continue;
		}
		if ( buf[0] != SYNC_MSG ) {
			error("Unknown packet: 0x%x\n", buf[0]);
			continue;
		}
		if ( sent.channel <= 0 ) {
			error("Packet from unknown source\n");
			continue;
		}
		index = sent.channel - 1;

		/* Ignore it if it is a duplicate packet */
		if ( SyncPtr[index] != NULL ) {
			continue;
		}

		/* Check the frame number */
		frame = SDLNet_Read32(&buf[1]);
#ifdef DEBUG_NETWORK
//error("Received a packet of frame %lu from player %d\r\n", frame, index+1);
#endif
		if ( frame != NextFrame ) {
			/* We kept the last frame cached, so send it */
			if ( frame == (NextFrame-1) ) {
#ifdef DEBUG_NETWORK
error("Transmitting packet for old frame (%lu)\r\n", frame);
#endif
				SDLNet_UDP_Send(gNetFD, sent.channel, OutBound[!CurrOut]);
			} else if ( frame == (NextFrame+1) ) {
#ifdef DEBUG_NETWORK
error("Received packet for next frame! (%lu, current = %lu)\r\n",
						frame, NextFrame);
#endif
				/* Send this player our current frame */
				SDLNet_UDP_Send(gNetFD, sent.channel, OutBound[CurrOut]);
				/* Cache this frame for next round,
				   skip consistency check, for now */
				memcpy(NextBuf[NextSync], &buf[PDATA_OFFSET], sent.len-PDATA_OFFSET);
				NextPtr[index] = NextBuf[NextSync];
				NextLen[index] = sent.len-PDATA_OFFSET;
				++NextSync;
			}
#ifdef DEBUG_NETWORK
else
error("Warning! Received packet for really old frame! (%lu, current = %lu)\r\n",
							frame, NextFrame);
#endif
			/* Go to select, reset timeout */
			continue;
		}

		/* Do a consistency check!! */
		Uint32 newseed = SDLNet_Read32(&buf[1+sizeof(frame)]);
		if ( newseed != seed ) {
			/* We're hosed, to correct this we would have to sync the complete game state */
			error(
"Error!! \a Frame consistency error with player %d!!\r\n", index+1);
			return(-1);
		}

		/* Okay, we finally have a valid timely packet */
		memcpy(SyncBuf[ThisSync], &buf[PDATA_OFFSET], sent.len-PDATA_OFFSET);
		SyncPtr[index] = SyncBuf[ThisSync];
		SyncLen[index] = sent.len-PDATA_OFFSET;
		++ThisSync;
		--nleft;
	}

	/* Set the next outbound packet buffer */
	++NextFrame;
	TOGGLE(CurrOut);
	OutLen = PDATA_OFFSET;

	return(0);
}

/* This function retrieves a particular player's network buffer */
int GetSyncBuf(int index, unsigned char **bufptr)
{
	int retlen;

	*bufptr = SyncPtr[index];
	SyncPtr[index] = NULL;
	retlen = SyncLen[index];
	SyncLen[index] = 0;
#ifdef SERIOUS_DEBUG
if ( retlen > 0 ) {
	for ( int i=1; i<retlen; i+=2 ) {
		error(
"Keystroke (key = 0x%.2x) for player %d on frame %d!\r\n",
					(*bufptr)[i], index+1, NextFrame);
	}
}
#endif
	return(retlen);
}

inline void SuckPackets(void)
{
	UDPpacket sent;
	unsigned char buf[BUFSIZ];

	sent.data = buf;
	sent.maxlen = sizeof(buf);
	while ( SDLNet_UDP_Recv(gNetFD, &sent) ) {
		/* Keep sucking */ ;
	}
}

/* Flash an error up on the screen and pause for 3 seconds */
static void ErrorMessage(const char *message)
{
	/* Display the error message */
	Message(message);

	/* Wait exactly (almost) 3 seconds */
	SDL_Delay(3000);
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
	DynamicPacket newgame, packet;

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

		if ( SDLNet_CheckSockets(SocketSet, 100) <= 0 ) {
			HandleEvents(0);
			/* Peek at key buffer for Quit key */
			for ( i=(PDATA_OFFSET+1); i<OutLen; i += 2 ) {
				if ( OutBuf[i] == ABORT_KEY ) {
					OutLen = PDATA_OFFSET;
					return(-1);
				}
			}
			OutLen = PDATA_OFFSET;

			for (i = 0; i < MAX_NODES; ++i) {
				if ( waiting[i] ) {
					SDLNet_UDP_Send(gNetFD, i+1, &newgame);
				}
			}
			continue;
		}

		/* We are guaranteed that there is data here */
		packet.Reset();
		if ( SDLNet_UDP_Recv(gNetFD, &packet) <= 0 ) {
			ErrorMessage("Network error receiving packets");
			return(-1);
		}

		/* We have a packet! */
		Uint8 cmd;
		Uint32 gameID;
		Uint32 nodeID;
		if (!packet.Read(cmd) || cmd != NEW_GAME_ACK) {
			/* Continue waiting */
			continue;
		}
		if (!packet.Read(gameID) || !packet.Read(nodeID)) {
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
