/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "SDL.h"
#include "physfs.h"
#include "hashtable.h"

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
	const char *lastValue;

	if (!value) {
		value = "";
	}
	if (hash_find(m_values, key, (const void **)&lastValue)) {
		if (SDL_strcmp(lastValue, value) == 0) {
			return;
		}
		hash_remove(m_values, key);
	}
	hash_insert(m_values, SDL_strdup(key), SDL_strdup(value));
}

void
Prefs::SetNumber(const char *key, int value)
{
	char buf[32];

	SDL_snprintf(buf, sizeof(buf), "%d", value);
	SetString(key, buf);
}

void
Prefs::SetBool(const char *key, bool value)
{
	if (value) {
		SetString(key, "true");
	} else {
		SetString(key, "false");
	}
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

int
Prefs::GetNumber(const char *key, int defaultValue)
{
	const char *value;

	if (hash_find(m_values, key, (const void **)&value)) {
		return SDL_atoi(value);
	}
	return defaultValue;
}

bool
Prefs::GetBool(const char *key, bool defaultValue)
{
	const char *value;

	if (hash_find(m_values, key, (const void **)&value)) {
		if (*value == '1' || *value == 't' || *value == 'T') {
			return true;
		} else if (*value == '0' || *value == 'f' || *value == 'F') {
			return false;
		}
	}
	return defaultValue;
}
