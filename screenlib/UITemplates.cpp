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

#include <stdio.h>
#include <strings.h>
#include <physfs.h>

#include "UITemplates.h"


UITemplates::UITemplates()
{
	m_data = NULL;
	m_hashTable = NULL;
}

UITemplates::~UITemplates()
{
	if (m_hashTable) {
		hash_destroy(m_hashTable);
	}
	if (m_data) {
		delete[] m_data;
	}
}

bool
UITemplates::Load(const char *file)
{
	PHYSFS_File *fp;
	PHYSFS_sint64 size;
	rapidxml::xml_node<> *node;
	rapidxml::xml_attribute<> *attr;

	fp = PHYSFS_openRead(file);
	if (!fp) {
		fprintf(stderr, "Warning: Couldn't open %s: %s\n",
					file, PHYSFS_getLastError());
		return false;
	}

	size = PHYSFS_fileLength(fp);
	m_data = new char[size+1];
	if (PHYSFS_readBytes(fp, m_data, size) != size) {
		fprintf(stderr, "Warning: Couldn't read from %s: %s\n",
					file, PHYSFS_getLastError());
		PHYSFS_close(fp);
		delete[] m_data;
		m_data = NULL;
		return false;
	}
	m_data[size] = '\0';
	PHYSFS_close(fp);

	try {
		m_doc.parse<0>(m_data);
	} catch (rapidxml::parse_error e) {
		fprintf(stderr, "Warning: Couldn't parse %s: error: %s\n",
					file, e.what());
		delete[] m_data;
		m_data = NULL;
		return false;
	}


	m_hashTable = hash_create(NULL, HashTable_Hash,
					HashTable_KeyMatch,
					HashTable_Nuke);

	node = m_doc.first_node();
	for (node = node->first_node(); node; node = node->next_sibling()) {
		attr = node->first_attribute("templateName", 0, false);
		if (attr) {
			HashKey *key = new HashKey;
			key->type = node->name();
			key->name = attr->value();
			hash_insert(m_hashTable, key, node);
		} else {
			fprintf(stderr, "Warning: UITemplate %s missing 'templateName'\n", node->name());
		}
	}
	return true;
}

rapidxml::xml_node<> *
UITemplates::GetTemplateFor(rapidxml::xml_node<> *node) const
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute("template", 0, false);
	if (!attr) {
		return NULL;
	}
	return GetTemplate(node->name(), attr->value());
}

rapidxml::xml_node<> *
UITemplates::GetTemplate(const char *type, const char *name) const
{
	HashKey key;
	rapidxml::xml_node<> *templateNode;

	if (!m_hashTable) {
		return NULL;
	}

	key.type = type;
	key.name = name;
	if (hash_find(m_hashTable, &key, (const void **)&templateNode)) {
		return templateNode;
	}
	return NULL;
}

// this is djb's xor hashing function.
unsigned
UITemplates::HashTable_Hash(const void *_key, void *data)
{
	const HashKey *key = static_cast<const HashKey *>(_key);
	const char *p;
	register unsigned hash = 5381;

	p = key->type;
	while (*p) {
		hash = ((hash << 5) + hash) ^ *(p++);
	}
	p = key->name;
	while (*p) {
		hash = ((hash << 5) + hash) ^ *(p++);
	}
	return hash;
}

int
UITemplates::HashTable_KeyMatch(const void *_a, const void *_b, void *data)
{
	const HashKey *a = static_cast<const HashKey *>(_a);
	const HashKey *b = static_cast<const HashKey *>(_a);

	return strcmp(a->type, b->type) == 0 && strcmp(a->name, b->name) == 0;
}

void
UITemplates::HashTable_Nuke(const void *_key, const void *value, void *data)
{
	HashKey *key = (HashKey *)_key;
	delete key;
}
