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

// Define this to get extremely verbose debug printing
//#define DEBUG_REPLAY

#define DELTA_SIZEMASK	0x7F
#define DELTA_SEED	0x80


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
		m_data.Reset();
		m_pausedInput.Reset();
	} else if (m_mode == REPLAY_PLAYBACK) {
		gGameInfo.CopyFrom(m_game);
		gGameInfo.PrepareForReplay();
		m_data.Seek(0);
	}
	m_seed = m_game.seed;
}

bool
Replay::HandlePlayback()
{
	if (m_mode != REPLAY_PLAYBACK) {
		return true;
	}

	if (gPaused) {
		return true;
	}

	Uint8 delta;
	if (!m_data.Read(delta)) {
		// That's it, end of recording
#ifdef DEBUG_REPLAY
printf("Replay complete!\n");
#endif
		return false;
	}

	// Check to make sure we haven't gotten a consistency error
	if (delta & DELTA_SEED) {
		if (!m_data.Read(m_seed)) {
			error("Error in replay, missing data\r\n");
			return false;
		}
	}
	if (m_seed != GetRandSeed()) {
		error("Error!! \a consistency problem expecting seed %8.8x, got seed %8.8x, aborting!!\r\n", m_seed, GetRandSeed());
		return false;
	}

	// Add the input for this frame
	int size = (delta & DELTA_SIZEMASK);
	Uint8 value;
	if (m_data.Size() < size) {
		error("Error in replay, missing data\r\n");
		return false;
	}
#ifdef DEBUG_REPLAY
printf("Read %d bytes for frame %d", size, NextFrame);
#endif
	while (size--) {
		m_data.Read(value);
		QueueInput(value);
	}
#ifdef DEBUG_REPLAY
printf(", pos = %d, seed = %8.8x\n", m_data.Tell(), m_seed);
#endif
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
	int size = GetSyncBuf(&data);

	// If we're paused, save this data for the next unpaused frame
	if (gPaused) {
		m_pausedInput.Write(data, size);
		return;
	}
	assert(size+m_pausedInput.Size() < DELTA_SIZEMASK);

	Uint8 delta;
	delta = (Uint8)size+m_pausedInput.Size();

	// Add it to our data buffer
	Uint32 seed = GetRandSeed();
	if (seed != m_seed) {
		delta |= DELTA_SEED;
		m_data.Write(delta);
		m_data.Write(seed);
		m_seed = seed;
	} else {
		m_data.Write(delta);
	}

	if (m_pausedInput.Size() > 0) {
		m_pausedInput.Seek(0);
		m_data.Write(m_pausedInput);
		m_pausedInput.Reset();
	}
	m_data.Write(data, size);
#ifdef DEBUG_REPLAY
printf("Wrote %d bytes for frame %d, size = %d, seed = %8.8x\n", size, NextFrame, m_data.Size(), m_seed);
#endif
}
