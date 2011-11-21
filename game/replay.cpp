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

#include <zlib.h>
#include "physfs.h"

#include "Maelstrom_Globals.h"
#include "netplay.h"
#include "player.h"
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

/* The file format is as follows:

	Uint8 version
	Uint32 frameCount
	Uint8 finalPlayer
	Uint8 finalWave
	{
		Uint32 score;
		Uint8  frags;
	} finalScore[MAX_PLAYERS]

	Uint32 gameSize
	GameInfo game

	Uint32 dataSize
	Uint32 compressed_size
	Uint8 compressed_data[]
*/

// This is a temporary hack
static PHYSFS_File *
CopyTempReplay(const char *file)
{
	FILE *rfp;
	PHYSFS_File *wfp;
	const char *base;
	char path[1024];
	char data[1024];
	size_t size;

	base = SDL_max(SDL_strrchr(file, '/'), SDL_strrchr(file, '\\'));
	if (base) {
		base = base+1;
	} else {
		base = file;
	}

	PHYSFS_mkdir("tmp");
	SDL_snprintf(path, sizeof(path), "tmp/%s", base);

	rfp = fopen(file, "rb");
	if (!rfp) {
		fprintf(stderr, "Couldn't open %s\n", file);
		return NULL;
	}

	wfp = PHYSFS_openWrite(path);
	if (!wfp) {
		fprintf(stderr, "Couldn't write to %s: %s\n", path, PHYSFS_getLastError());
		fclose(rfp);
		return NULL;
	}

	while ((size = fread(data, 1, sizeof(data), rfp)) > 0) {
		if (!PHYSFS_writeBytes(wfp, data, size)) {
			goto physfs_write_error;
		}
	}
	if (!PHYSFS_close(wfp)) {
		goto physfs_write_error;
	}
	fclose(rfp);

	return PHYSFS_openRead(path);

physfs_write_error:
	fprintf(stderr, "Error writing to %s: %s\n", path, PHYSFS_getLastError());
	fclose(rfp);
	PHYSFS_close(wfp);
	PHYSFS_delete(path);
	return NULL;
}

bool
Replay::Load(const char *file, bool headerOnly)
{
	char path[1024];
	PHYSFS_File *fp;
	DynamicPacket data;
	uLongf destLen;
	Uint32 size;
	Uint32 compressedSize;

	// Open the file
	if (SDL_strchr(file, '/') == NULL) {
		SDL_snprintf(path, sizeof(path), "%s/%s.%s", REPLAY_DIRECTORY, file, REPLAY_FILETYPE);
		file = path;
	}
	fp = PHYSFS_openRead(file);
	if (!fp) {
		// If the file is outside our sandbox, try to copy it in
		fp = CopyTempReplay(file);
	}
	if (!fp) {
		fprintf(stderr, "Couldn't read %s: %s\n", file, PHYSFS_getLastError());
		return false;
	}

	Uint8 version;
	if (!PHYSFS_readBytes(fp, &version, 1)) {
		goto physfs_read_error;
	}
	if (version != REPLAY_VERSION) {
		fprintf(stderr, "Unsupported version %d, expected %d\n", version, REPLAY_VERSION);
		goto error_return;
	}
	if (!PHYSFS_readULE32(fp, &m_frameCount)) {
		goto physfs_read_error;
	}
	if (!PHYSFS_readBytes(fp, &m_finalPlayer, 1)) {
		goto physfs_read_error;
	}
	if (!PHYSFS_readBytes(fp, &m_finalWave, 1)) {
		goto physfs_read_error;
	}
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!PHYSFS_readULE32(fp, &m_finalScore[i].Score)) {
			goto physfs_read_error;
		}
		if (!PHYSFS_readBytes(fp, &m_finalScore[i].Frags, 1)) {
			goto physfs_read_error;
		}
	}

	if (!headerOnly) {
		if (!PHYSFS_readULE32(fp, &size)) {
			goto physfs_read_error;
		}
		data.Reset();
		data.Grow(size);
		if (!PHYSFS_readBytes(fp, data.data, size)) {
			goto physfs_read_error;
		}
		data.len = size;
		if (!m_game.ReadFromPacket(data)) {
			fprintf(stderr, "Couldn't read game information from %s", file);
			goto error_return;
		}

		if (!PHYSFS_readULE32(fp, &size)) {
			goto physfs_read_error;
		}
		m_data.Reset();
		m_data.Grow(size);

		if (!PHYSFS_readULE32(fp, &compressedSize)) {
			goto physfs_read_error;
		}
		data.Reset();
		data.Grow(compressedSize);

		if (!PHYSFS_readBytes(fp, data.data, compressedSize)) {
			goto physfs_read_error;
		}
		destLen = size;
		if (uncompress(m_data.Data(), &destLen, data.Data(), compressedSize) != Z_OK) {
			fprintf(stderr, "Error uncompressing replay data\n");
			goto error_return;
		}
		m_data.len = size;
	}

	// We're done!
	PHYSFS_close(fp);
	return true;

physfs_read_error:
	fprintf(stderr, "Error reading from %s: %s\n", file, PHYSFS_getLastError());
error_return:
	PHYSFS_close(fp);
	return false;
}

bool
Replay::Save(const char *file)
{
	char path[1024];
	PHYSFS_File *fp;
	DynamicPacket data;
	uLongf destLen;

	// Create the directory if needed
	PHYSFS_mkdir(REPLAY_DIRECTORY);

	// Open the file
	if (SDL_strchr(file, '/') == NULL) {
		SDL_snprintf(path, sizeof(path), "%s/%s.%s", REPLAY_DIRECTORY, file, REPLAY_FILETYPE);
		file = path;
	}
	fp = PHYSFS_openWrite(file);
	if (!fp) {
		fprintf(stderr, "Couldn't write to %s: %s\n", file, PHYSFS_getLastError());
		return false;
	}

	Uint8 version = REPLAY_VERSION;
	if (!PHYSFS_writeBytes(fp, &version, 1)) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeULE32(fp, m_frameCount)) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeBytes(fp, &m_finalPlayer, 1)) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeBytes(fp, &m_finalWave, 1)) {
		goto physfs_write_error;
	}
	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (!PHYSFS_writeULE32(fp, m_finalScore[i].Score)) {
			goto physfs_write_error;
		}
		if (!PHYSFS_writeBytes(fp, &m_finalScore[i].Frags, 1)) {
			goto physfs_write_error;
		}
	}

	data.Reset();
	m_game.WriteToPacket(data);
	if (!PHYSFS_writeULE32(fp, data.Size())) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeBytes(fp, data.Data(), data.Size())) {
		goto physfs_write_error;
	}

	destLen = compressBound(m_data.Size());
	data.Reset();
	data.Grow(destLen);
	if (compress(data.Data(), &destLen, m_data.Data(), m_data.Size()) != Z_OK) {
		fprintf(stderr, "Error compressing replay data\n");
		goto error_return;
	}
	if (!PHYSFS_writeULE32(fp, m_data.Size())) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeULE32(fp, data.Size())) {
		goto physfs_write_error;
	}
	if (!PHYSFS_writeBytes(fp, data.Data(), data.Size())) {
		goto physfs_write_error;
	}

	// We're done!
	if (!PHYSFS_close(fp)) {
		goto physfs_write_error;
	}
	return true;

physfs_write_error:
	fprintf(stderr, "Error writing to %s: %s\n", file, PHYSFS_getLastError());
error_return:
	PHYSFS_close(fp);
	PHYSFS_delete(path);
	return false;
}

void
Replay::HandleNewGame()
{
	if (m_mode == REPLAY_IDLE) {
		m_mode = REPLAY_RECORDING;
	}

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
	m_frameCount = 0;
	m_finalPlayer = 0;
	m_finalWave = 0;
	SDL_zero(m_finalScore);
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
	for (int i = 0; i < size; ++i) {
		m_data.Read(value);
		QueueInput(value);
	}

	++m_frameCount;

#ifdef DEBUG_REPLAY
printf("Read %d bytes for frame %d, pos = %d, seed = %8.8x\n", size, m_frameCount, m_data.Tell(), m_seed);
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

	++m_frameCount;

#ifdef DEBUG_REPLAY
printf("Wrote %d bytes for frame %d, size = %d, seed = %8.8x\n", size, m_frameCount, m_data.Size(), m_seed);
#endif
}

void
Replay::HandleGameOver()
{
	if (m_mode != REPLAY_RECORDING) {
		return;
	}

	m_finalPlayer = gDisplayed;
	m_finalWave = gWave;

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		m_finalScore[i].Score = gPlayers[i]->GetScore();
		m_finalScore[i].Frags = gPlayers[i]->GetFrags();
	}

	// Save this as the last score
	Save("LastScore");
}
