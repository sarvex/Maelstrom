/*
    SCREENLIB:  A framebuffer library based on the SDL library
    Copyright (C) 1997  Sam Lantinga

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

#ifndef _UITemplates_h
#define _UITemplates_h

#include "../utils/hashtable.h"
#include "../utils/rapidxml.h"

class UITemplates
{
public:
	UITemplates();
	~UITemplates();

	bool Load(const char *file);

	rapidxml::xml_node<> *GetTemplateFor(rapidxml::xml_node<> *node) const;
	rapidxml::xml_node<> *GetTemplate(const char *type, const char *name) const;

protected:
	char *m_data;
	rapidxml::xml_document<> m_doc;
	struct HashKey {
		const char *type;
		const char *name;
	};
	HashTable *m_hashTable;

protected:
	static unsigned HashTable_Hash(const void *key, void *data);
	static int HashTable_KeyMatch(const void *a, const void *b, void *data);
	static void HashTable_Nuke(const void *key, const void *value, void *data);
};

#endif // _UITemplates_h
