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

/* Network protocol for synchronization and keystrokes */

#define SYNC_MSG	0x00			/* Sent during game */
#define NEW_GAME	0x01			/* Sent by players at start */
#define NET_ABORT	0x04			/* Used with address server */
#define KEY_PRESS	0x80			/* Sent during game */
#define KEY_RELEASE	0xF0			/* Sent during game */

/* * * * * * * *
	This stuff is shared between netplay.cc and netplayd
*/
/* The default port for Maelstrom games.  What is 0xAEAE?? *shrug* :) */
#define NETPLAY_PORT	0xAEAE			/* port 44718 */

/* The minimum length of a new packet buffer */
#define NEW_PACKETLEN	(3+3*4)

/* Note: if you change MAX_PLAYERS, you need to modify the gPlayerColors
   array in player.cc
*/
#define MAX_PLAYERS		3		/* No more than 255!! */

