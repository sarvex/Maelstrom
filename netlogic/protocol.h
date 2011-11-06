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

#ifndef _protocol_h
#define _protocol_h

/* Architecture
 *
 * The lobby server simply acts as an address broker, keeping a list
 * of IP addresses of game servers, and serving them to players
 * wanting to find games.
 *
 * The joining game and hosting games then communicate with each other
 * to create a list of games and join/leave them.
 */

/* Protocol messages */
enum LobbyProtocol {
	LOBBY_NONE,

	/**********/
	/* Messages between the hosting game and the lobby server */

	LOBBY_ANNOUNCE_GAME = 1,
	/* Sent by the hosting game to the lobby server
	   This is sent periodically to keep the entry refreshed, since
	   the server will automatically age out entries after 30 seconds.

		Uint8 numaddresses
		{
			Uint32 host;
			Uint16 port;
		} addresses[]
	 */

	LOBBY_REMOVE_GAME,
	/* Sent by the hosting game to the lobby server
	   This is sent when the game is no longer available to join.

		Uint8 numaddresses
		{
			Uint32 host;
			Uint16 port;
		} addresses[]
	 */

	LOBBY_ANNOUNCE_PLAYER,
	/* Sent by the lobby server when a player requests the game list.
	   This allows the hosting game to send a packet to the player
	   requesting to join, opening the firewall for them.

		Uint8 numaddresses
		{
			Uint32 host;
			Uint16 port;
		} addresses[]
	*/

	/**********/
	/* Messages between the joining game and the lobby server */

	LOBBY_REQUEST_GAME_SERVERS = 10,
	/* Sent by the joining game, containing a list of it's addresses

		Uint8 numaddresses
		{
			Uint32 host;
			Uint16 port;
		} addresses[]
	 */

	LOBBY_GAME_SERVERS,
	/* Sent by the lobby server containing all the current game addresses

		Uint8 numaddresses
		{
			Uint32 host;
			Uint16 port;
		} addresses[]
	*/

	/**********/
	/* Messages between the joining game and the hosting game */

	LOBBY_OPEN_FIREWALL = 20,
	/* Sent by the hosting game in response to lobby server messages
	   to open the firewall for communication
	 */

	LOBBY_REQUEST_GAME_INFO,
	/* Sent by the joining game to get info for the game list
	 */

	LOBBY_GAME_INFO,
	/* Sent by the hosting game, if there are slots open

		Uint32 gameID
		Uint8 namelen
		char name[]
	 */

	LOBBY_PING,
	/* Sent by the joining game to determine latency and as a keepalive
	   check to make sure the player and game are still valid.

		Uint32 gameID
		Uint32 playerID
		Uint32 sequence
	 */

	LOBBY_PONG,
	/* Echoed by the hosting game in response to LOBBY_PING

		Uint32 gameID
		Uint32 playerID
		Uint32 sequence
	 */

	LOBBY_REQUEST_JOIN,
	/* Sent by the joining game

		Uint32 gameID
		Uint32 playerID
		Uint8 namelen
		char name[]
	*/

	LOBBY_JOINED,
	/* Sent by the hosting game

		Uint32 gameID
		Uint32 playerID
		Uint8 playerSlot
	*/

	/* You can't add any more packets past here, look above for space! */
	LOBBY_PACKET_MAX = 256
};

/* Network protocol for synchronization and keystrokes */

#define LOBBY_MSG	0x00			/* Sent before game */
#define NEW_GAME	0x01			/* Sent by players at start */
#define SYNC_MSG	0x02			/* Sent during game */
#define KEY_PRESS	0x04			/* Sent during game */
#define KEY_RELEASE	0x08			/* Sent during game */

/* The default port for Maelstrom games */
#define LOBBY_PORT	0xAE00			/* port 44544 */
#define NETPLAY_PORT	0xAF00			/* port 44800 */

/* The minimum length of a new packet buffer */
#define NEW_PACKETLEN	(3+3*4)

/* Note: if you change MAX_PLAYERS, you need to modify the gPlayerColors
   array in player.cpp
*/
#define MAX_PLAYERS		3		/* No more than 255!! */

#endif /* _protocol_h */
