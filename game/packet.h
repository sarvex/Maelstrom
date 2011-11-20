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

#ifndef _packet_h
#define _packet_h

#include "SDL_net.h"
#include "protocol.h"

// Utility functions to compare IP addresses 

extern inline bool operator==(const IPaddress &lhs, const IPaddress &rhs) {
	return lhs.host == rhs.host && lhs.port == rhs.port;
}
extern inline bool operator!=(const IPaddress &lhs, const IPaddress &rhs) {
	return !operator==(lhs, rhs);
}

// A dynamic packet class that takes care of allocating memory and packing data

class DynamicPacket : public UDPpacket
{
public:
	DynamicPacket(int minSize = 1024) {
		SDL_zero(*this);
		maxlen = minSize;
		data = (Uint8*)SDL_malloc(minSize);
	}
	~DynamicPacket() {
		SDL_free(data);
	}

	void StartLobbyMessage(int msg) {
		Reset();
		Write((Uint8)LOBBY_MSG);
		Write((Uint8)msg);
	}

	void Reset() {
		len = 0;
		pos = 0;
	}

	void Seek(int offset) {
		if (offset < len) {
			pos = offset;
		}
	}

	int Tell() {
		return pos;
	}

	void Write(Uint8 value) {
		Grow(sizeof(value));
		data[pos++] = value;
	}
	void Write(Uint16 value) {
		Grow(sizeof(value));
		SDLNet_Write16(value, &data[pos]);
		pos += sizeof(value);
	}
	void Write(Uint32 value) {
		Grow(sizeof(value));
		SDLNet_Write32(value, &data[pos]);
		pos += sizeof(value);
	}
	void Write(const char *value) {
		if (!value) {
			value = "";
		}

		size_t amount = SDL_strlen(value);
		if (amount > 255) {
			amount = 255;
		}
		Write((Uint8)amount);
		Grow(amount);
		SDL_memcpy(&data[pos], value, amount);
		pos += amount;
	}
	void Write(DynamicPacket &packet) {
		size_t amount = packet.len - packet.pos;
		Grow(amount);
		SDL_memcpy(&data[pos], &packet.data[packet.pos], amount);
		packet.pos += amount;
		pos += amount;
	}

	bool Read(Uint8 &value) {
		if (pos+sizeof(value) > (size_t)len) {
			return false;
		}
		value = data[pos++];
		return true;
	}
	bool Read(Uint16 &value) {
		if (pos+sizeof(value) > (size_t)len) {
			return false;
		}
		value = SDLNet_Read16(&data[pos]);
		pos += sizeof(value);
		return true;
	}
	bool Read(Uint32 &value) {
		if (pos+sizeof(value) > (size_t)len) {
			return false;
		}
		value = SDLNet_Read32(&data[pos]);
		pos += sizeof(value);
		return true;
	}
	bool Read(char *value, size_t maxlen) {
		Uint8 amount;
		if (!Read(amount)) {
			return false;
		}
		if (amount > maxlen-1) {
			return false;
		}
		SDL_memcpy(value, &data[pos], amount);
		value[amount] = '\0';
		pos += amount;
		return true;
	}

protected:
	void Grow(size_t additionalSize) {
		if (len+additionalSize > (size_t)maxlen) {
			while (len+additionalSize > (size_t)maxlen) {
				maxlen *= 2;
			}
			data = (Uint8*)SDL_realloc(data, maxlen);
		}
		len += additionalSize;
	}

protected:
	int pos;
};

#endif // _packet_h
