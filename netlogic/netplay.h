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

/* Functions in netplay.cc */
extern int   InitNetData(void);
extern void  HaltNetData(void);
extern int   AddPlayer(const char *playerstr);
extern int   SetServer(char *serverstr);
extern int   CheckPlayers(void);
extern void  QueueKey(unsigned char Op, unsigned char Type);
extern int   SyncNetwork(void);
extern int   GetSyncBuf(int index, unsigned char **bufptr);
extern int   Send_NewGame(int *Wave, int *Lives, int *Turbo);
extern int   Await_NewGame(int *Wave, int *Lives, int *Turbo);

/* Variables from netplay.cc */
extern int	gOurPlayer;
extern int	gNumPlayers;
extern int	gDeathMatch;

