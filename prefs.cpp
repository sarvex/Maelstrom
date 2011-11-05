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

#include "physfs.h"
#include "utils/hashtable.h"

#include "prefs.h"


static void
hash_nuke_strings(const void *key, const void *value, void *data)
{
	SDL_free((char*)key);
	SDL_free((char*)value);
}

Prefs::Prefs(const char *file)
{
	m_file = SDL_strdup(file);
	m_values = hash_create(NULL, hash_hash_string, hash_keymatch_string, hash_nuke_strings);
}

Prefs::~Prefs()
{
	SDL_free(m_file);
	hash_destroy(m_values);
}

bool
Prefs::Load()
{
	PHYSFS_File *fp;
	PHYSFS_sint64 size;
	char *data;
	char *key, *value, *next;

	fp = PHYSFS_openRead(m_file);
	if (!fp) {
		/* This is fine, we just haven't written them yet */
		return false;
	}

	size = PHYSFS_fileLength(fp);
	data = new char[size+1];
	if (PHYSFS_readBytes(fp, data, size) != size) {
		fprintf(stderr, "Warning: Couldn't read from %s: %s\n",
					m_file, PHYSFS_getLastError());
		PHYSFS_close(fp);
		delete[] data;
		return false;
	}
	data[size] = '\0';
	PHYSFS_close(fp);

	key = data;
	while (*key) {
		value = SDL_strchr(key, '=');
		if (!value) {
			break;
		}
		*value++ = '\0';

		next = value;
		while (*next && *next != '\r' && *next != '\n') {
			++next;
		}
		if (*next) {
			*next++ = '\0';
		}

		SetString(key, value);

		key = next;
		while (*key && (*key == '\r' || *key == '\n')) {
			++key;
		}
	}
	delete[] data;

	return true;
}

static __inline__ bool
writeString(PHYSFS_File *fp, const char *string)
{
	size_t len = SDL_strlen(string);
	return ((size_t)PHYSFS_writeBytes(fp, string, len) == len);
}

bool
Prefs::Save()
{
	PHYSFS_File *fp;
	const char *key, *value;
	void *iter;

	fp = PHYSFS_openWrite(m_file);
	if (!fp) {
		fprintf(stderr, "Warning: Couldn't open %s: %s\n",
					m_file, PHYSFS_getLastError());
		return false;
	}

	iter = NULL;
	while (hash_iter(m_values, (const void **)&key, (const void **)&value, &iter)) {
		if (!writeString(fp, key) ||
		    !writeString(fp, "=") ||
		    !writeString(fp, value) ||
		    !writeString(fp, "\r\n")) {
			fprintf(stderr, "Warning: Couldn't write to %s: %s\n",
						m_file, PHYSFS_getLastError());
			PHYSFS_close(fp);
			return false;
		}
	}
	PHYSFS_close(fp);

	return true;
}

void
Prefs::SetString(const char *key, const char *value)
{
	hash_remove(m_values, key);
	hash_insert(m_values, SDL_strdup(key), SDL_strdup(value));
}

void
Prefs::SetNumber(const char *key, Uint32 value)
{
	char buf[32];

	SDL_snprintf(buf, sizeof(buf), "%u", value);
	SetString(key, buf);
}

const char *
Prefs::GetString(const char *key, const char *defaultValue)
{
	const char *value;

	if (hash_find(m_values, key, (const void **)&value)) {
		return value;
	}
	return defaultValue;
}

Uint32
Prefs::GetNumber(const char *key, Uint32 defaultValue)
{
	const char *value;

	if (hash_find(m_values, key, (const void **)&value)) {
		return SDL_atoi(value);
	}
	return defaultValue;
}
