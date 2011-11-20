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

#include "Maelstrom_Globals.h"
#include "netplay.h"
#include "replay.h"


Replay::Replay()
{
	m_mode = REPLAY_IDLE;
}

void
Replay::SetMode(REPLAY_MODE mode)
{
	m_mode = mode;
}

bool
Replay::Load(const char *file)
{
	assert(!"Not yet implemented");
	return false;
}

bool
Replay::Save(const char *file)
{
	assert(!"Not yet implemented");
	return false;
}

void
Replay::HandleNewGame()
{
	if (m_mode == REPLAY_RECORDING) {
		m_game.CopyFrom(gGameInfo);
		m_game.PrepareForReplay();
	} else if (m_mode == REPLAY_PLAYBACK) {
		gGameInfo.CopyFrom(m_game);
		gGameInfo.PrepareForReplay();
	}
	m_data.Seek(0);
}

bool
Replay::HandlePlayback()
{
	if (m_mode != REPLAY_PLAYBACK) {
		return true;
	}

	// Check to make sure we haven't gotten a consistency error
	Uint32 seed;
	if (!m_data.Read(seed)) {
		// That's it, end of recording
		return false;
	}
	if (seed != GetRandSeed()) {
		error("Error!! \a Frame consistency problem, aborting!!\r\n");
		return false;
	}

	// Add the input for this frame
	Uint16 size;
	Uint8 value;
	if (!m_data.Read(size) || (m_data.Size() < (int)size)) {
		error("Error in replay, missing data\r\n");
		return false;
	}
	while (size--) {
		m_data.Read(value);
		QueueInput(value);
	}
	return true;
}

void
Replay::HandleRecording()
{
	if (m_mode != REPLAY_RECORDING) {
		return;
	}

	// Get the input for this frame
	Uint8 *data;
	int len = GetSyncBuf(&data);

	// Add it to our data buffer
	m_data.Write(GetRandSeed());
	assert(len <= 0xFFFF);
	m_data.Write((Uint16)len);
	m_data.Write(data, len);
}
