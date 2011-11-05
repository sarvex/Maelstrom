
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
		len = 0;
		maxlen = minSize;
		data = (Uint8*)SDL_malloc(minSize);
	}
	~DynamicPacket() {
		SDL_free(data);
	}

	void Reset() {
		len = 0;
	}

	DynamicPacket& operator <<(Uint8 value) {
		CheckSize(sizeof(value));
		data[len++] = value;
		return *this;
	}
	DynamicPacket& operator <<(Uint16 value) {
		CheckSize(sizeof(value));
		SDLNet_Write16(value, &data[len]);
		len += sizeof(value);
		return *this;
	}
	DynamicPacket& operator <<(Uint32 value) {
		CheckSize(sizeof(value));
		SDLNet_Write32(value, &data[len]);
		len += sizeof(value);
		return *this;
	}

protected:
	void CheckSize(size_t additionalSize) {
		if (len+additionalSize > (size_t)maxlen) {
			maxlen *= 2;
			data = (Uint8*)SDL_realloc(data, maxlen);
		}
	}
};

#endif // _packet_h
