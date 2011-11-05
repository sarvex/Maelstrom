
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

// A dynamic packet class that takes care of allocating memory and packing data

class DynamicPacket : public UDPpacket
{
public:
	DynamicPacket(int minSize = 32) {
		SDL_zero(*this);
		maxlen = minSize;
		data = (Uint8*)SDL_malloc(minSize);
	}
	~DynamicPacket() {
		SDL_free(data);
	}

	void Expand(size_t size) {
		CheckSize(size - len);
	}

	void Reset() {
		len = 0;
		pos = 0;
	}

	void Write(Uint8 value) {
		CheckSize(sizeof(value));
		data[len++] = value;
	}
	void Write(Uint16 value) {
		CheckSize(sizeof(value));
		SDLNet_Write16(value, &data[len]);
		len += sizeof(value);
	}
	void Write(Uint32 value) {
		CheckSize(sizeof(value));
		SDLNet_Write32(value, &data[len]);
		len += sizeof(value);
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

protected:
	void CheckSize(size_t additionalSize) {
		if (len+additionalSize > (size_t)maxlen) {
			while (len+additionalSize > (size_t)maxlen) {
				maxlen *= 2;
			}
			data = (Uint8*)SDL_realloc(data, maxlen);
		}
	}

protected:
	int pos;
};

#endif // _packet_h
