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

#ifndef _prefs_h
#define _prefs_h

#include "SDL.h"

class HashTable;

class Prefs
{
public:
	Prefs(const char *file);
	virtual ~Prefs();

	bool Load();
	bool Save();

	void SetString(const char *key, const char *value);
	void SetNumber(const char *key, Uint32 value);
	void Set(const char *key, const char *value) {
		SetString(key, value);
	}
	void Set(const char *key, int value) {
		SetNumber(key, value);
	}
	void Set(const char *key, Uint32 value) {
		SetNumber(key, value);
	}
	void Set(const char *key, Uint8 value) {
		SetNumber(key, value);
	}

	const char *GetString(const char *key, const char *defaultValue = NULL);
	Uint32 GetNumber(const char *key, Uint32 defaultValue = 0);
	void Get(const char *key, const char *&value, const char *defaultValue) {
		value = GetString(key, defaultValue);
	}
	void Get(const char *key, int &value, int defaultValue) {
		value = GetNumber(key, defaultValue);
	}
	void Get(const char *key, Uint32 &value, Uint32 defaultValue) {
		value = GetNumber(key, defaultValue);
	}
	void Get(const char *key, Uint8 &value, Uint8 defaultValue) {
		value = GetNumber(key, defaultValue);
	}

protected:
	char *m_file;
	HashTable *m_values;
};

template <typename T>
class PrefsVariable
{
public:
	PrefsVariable(const char *name, const T &rhs) {
		m_prefs = NULL;
		m_name = name;
		m_defaultValue = m_value = rhs;
	}

	PrefsVariable& operator =(const T &rhs) {
		m_value = rhs;
		if (m_prefs) {
			m_prefs->Set(m_name, m_value);
		}
		return *this;
	}

	PrefsVariable& operator =(const PrefsVariable<T> &rhs) {
		return *this = rhs.m_value;
	}

	operator T() {
		return m_value;
	}

	T& operator++() {
		*this = m_value + 1;
		return m_value;
	}
	T operator++(int) {
		T value = m_value;
		*this = m_value + 1;
		return value;
	}
	T& operator--() {
		*this = m_value - 1;
		return m_value;
	}
	T operator--(int) {
		T value = m_value;
		*this = m_value - 1;
		return value;
	}

	void Register(Prefs *prefs) {
		m_prefs = prefs;
		m_prefs->Get(m_name, m_value, m_defaultValue);
	}

	T Value() {
		return m_value;
	}

	void Reset() {
		*this = m_defaultValue;
	}

protected:
	Prefs *m_prefs;
	const char *m_name;
	T m_value;
	T m_defaultValue;
};

#endif /* _prefs_h */
