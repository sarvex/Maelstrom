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

#include <physfs.h>

#include "SDL_FrameBuf.h"
#include "UIPanel.h"
#include "UIElement.h"

UIElementFactory UIPanel::s_elementFactory;

UIPanel::UIPanel(FrameBuf *screen, const char *name) : UIArea()
{
	m_screen = screen;
	m_name = new char[strlen(name)+1];
	strcpy(m_name, name);

	m_rect.w = screen->Width();
	m_rect.h = screen->Height();
}

UIPanel::~UIPanel()
{
	delete[] m_name;

	for (unsigned i = 0; i < m_elements.length(); ++i) {
		delete m_elements[i];
	}
}

bool
UIPanel::Load(const char *file)
{
	PHYSFS_File *fp;
	PHYSFS_sint64 size;
	char *buffer;

	ClearError();

	if (!s_elementFactory) {
		SetError("No panel element factory set");
		return false;
	}

	fp = PHYSFS_openRead(file);
	if (!fp) {
		SetError("Couldn't open %s: %s", file, PHYSFS_getLastError());
		return false;
	}

	size = PHYSFS_fileLength(fp);
	buffer = new char[size+1];
	if (PHYSFS_readBytes(fp, buffer, size) != size) {
		SetError("Couldn't read from %s: %s", file, PHYSFS_getLastError());
		PHYSFS_close(fp);
		delete[] buffer;
		return false;
	}
	buffer[size] = '\0';
	PHYSFS_close(fp);

	rapidxml::xml_document<> doc;
	try {
		doc.parse<0>(buffer);
	} catch (rapidxml::parse_error e) {
		SetError("Parse error: %s", e.what());
		delete[] buffer;
		return false;
	}

	rapidxml::xml_node<> *node = doc.first_node();
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;
	if (strcmp(node->name(), "UIPanel") != 0) {
		SetError("Parse error: UIPanel root element expected");
		delete[] buffer;
		return false;
	}
	attr = node->first_attribute("name", 0, false);;
	if (attr) {
		const char *name = node->value();
		delete[] m_name;
		m_name = new char[strlen(name)+1];
		strcpy(m_name, name);
	}
	if (!UIArea::Load(node)) {
		delete[] buffer;
		return false;
	}
	child = node->first_node("elements", 0, false);
	if (child) {
		if (!LoadElements(child)) {
			delete[] buffer;
			return false;
		}
	}
	delete[] buffer;
	return true;
}

bool
UIPanel::LoadElements(rapidxml::xml_node<> *node)
{
	for (node = node->first_node(); node; node = node->next_sibling()) {
		UIElement *element = s_elementFactory(this, node->name());
		if (!element) {
			SetError("Couldn't find handler for element %s", node->name());
			return false;
		}
		if (!element->Load(node)) {
			SetError("Couldn't load element %s: %s", node->name(), element->Error());
			delete element;
			return false;
		}
		AddElement(element);
	}
	return true;
}

UIArea *
UIPanel::GetAnchorElement(const char *name)
{
	if (!name) {
		return m_screen;
	}
	return NULL;
}

UIElement *
UIPanel::GetElement(const char *name)
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (strcmp(name, m_elements[i]->GetName()) == 0) {
			return m_elements[i];
		}
	}
	return NULL;
}

void
UIPanel::Draw()
{
	for (unsigned i = 0; i < m_elements.length(); ++i) {
		if (m_elements[i]->IsShown()) {
			m_elements[i]->Draw();
		}
	}
}

bool
UIPanel::HandleEvent(const SDL_Event &event)
{
	for (unsigned i = m_elements.length(); i--; ) {
		if (m_elements[i]->HandleEvent(event)) {
			return true;
		}
	}
	return false;
}
