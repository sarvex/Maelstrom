/*
  screenlib:  A simple window and UI library based on the SDL library
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

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

#include "SDL_FrameBuf.h"
#include "UIArea.h"


UIArea::UIArea(FrameBuf *screen, UIArea *anchor, int w, int h) : ErrorBase()
{
	m_screen = screen;
	m_shown = true;
	m_rect.x = 0;
	m_rect.y = 0;
	m_rect.w = w;
	m_rect.h = h;
	m_anchor.element = anchor;
	m_anchor.anchorFrom = CENTER;
	m_anchor.anchorTo = CENTER;
	m_anchor.offsetX = 0;
	m_anchor.offsetY = 0;
	if (anchor) {
		anchor->AddAnchoredArea(this);
	}
}

bool
UIArea::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;
	SDL_Rect rect = m_rect;

	LoadBool(node, "show", m_shown);

	child = node->first_node("size", 0, false);
	if (child) {
		LoadNumber(child, "w", m_rect.w);
		LoadNumber(child, "h", m_rect.h);
	}

	child = node->first_node("anchor", 0, false);
	if (child) {
		int x, y;

		attr = child->first_attribute("anchor", 0, false);
		if (m_anchor.element) {
			m_anchor.element->DelAnchoredArea(this);
		}
		m_anchor.element = GetAnchorElement(attr ? attr->value() : NULL);
		if (m_anchor.element) {
			m_anchor.element->AddAnchoredArea(this);
		} else {
			SetError("Element 'anchor' couldn't find anchor element %s",
				attr ? attr->value() : "NULL");
			return false;
		}

		LoadAnchorLocation(child, "anchorFrom", m_anchor.anchorFrom);
		LoadAnchorLocation(child, "anchorTo", m_anchor.anchorTo);

		LoadNumber(child, "x", m_anchor.offsetX);
		LoadNumber(child, "y", m_anchor.offsetY);
	}

	CalculateAnchor(false);
	if (m_rect.x != rect.x || m_rect.y != rect.y ||
	    m_rect.w != rect.w || m_rect.h != rect.h) {
		OnRectChanged();
	}

	return true;
}

void
UIArea::SetPosition(int x, int y) {
	/* Setting the position breaks the anchoring */
	m_anchor.element = NULL;

	if (x != m_rect.x || y != m_rect.y) {
		m_rect.x = x;
		m_rect.y = y;
		OnRectChanged();
	}
}

void
UIArea::SetSize(int w, int h)
{
	if (w != m_rect.w || h != m_rect.h) {
		m_rect.w = w;
		m_rect.h = h;
		CalculateAnchor(false);
		OnRectChanged();
	}
}

void
UIArea::SetWidth(int w)
{
	if (w != m_rect.w) {
		m_rect.w = w;
		CalculateAnchor(false);
		OnRectChanged();
	}
}

void
UIArea::SetHeight(int h)
{
	if (h != m_rect.h) {
		m_rect.h = h;
		CalculateAnchor(false);
		OnRectChanged();
	}
}

void
UIArea::SetAnchor(AnchorLocation from, AnchorLocation to, UIArea *anchor,
						int offsetX, int offsetY)
{
	m_anchor.element = anchor;
	m_anchor.anchorFrom = from;
	m_anchor.anchorTo = to;
	m_anchor.offsetX = offsetX;
	m_anchor.offsetY = offsetY;
	CalculateAnchor();
}

bool
UIArea::LoadBool(rapidxml::xml_node<> *node, const char *name, bool &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		const char *text = attr->value();
		if (*text == '\0' || *text == '0' ||
		    *text == 'f' || *text == 'F') {
			value = false;
		} else {
			value = true;
		}
		return true;
	}
	return false;
}

bool
UIArea::LoadNumber(rapidxml::xml_node<> *node, const char *name, int &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		value = (int)strtol(attr->value(), NULL, 0);
		return true;
	}
	return false;
}

bool
UIArea::LoadString(rapidxml::xml_node<> *node, const char *name, char *&value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		if (value) {
			SDL_free(value);
		}
		value = SDL_strdup(attr->value());
		return true;
	}
	return false;
}

bool
UIArea::LoadAnchorLocation(rapidxml::xml_node<> *node, const char *name, AnchorLocation &value)
{
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute(name, 0, false);
	if (attr) {
		const char *text = attr->value();

		if (strcasecmp(text, "TOPLEFT") == 0) {
			value = TOPLEFT;
		} else if (strcasecmp(text, "TOP") == 0) {
			value = TOP;
		} else if (strcasecmp(text, "TOPRIGHT") == 0) {
			value = TOPRIGHT;
		} else if (strcasecmp(text, "LEFT") == 0) {
			value = LEFT;
		} else if (strcasecmp(text, "CENTER") == 0) {
			value = CENTER;
		} else if (strcasecmp(text, "RIGHT") == 0) {
			value = RIGHT;
		} else if (strcasecmp(text, "BOTTOMLEFT") == 0) {
			value = BOTTOMLEFT;
		} else if (strcasecmp(text, "BOTTOM") == 0) {
			value = BOTTOM;
		} else if (strcasecmp(text, "BOTTOMRIGHT") == 0) {
			value = BOTTOMRIGHT;
		} else {
			/* Failed to parse */
			return false;
		}
		return true;
	}
	return false;
}

bool
UIArea::LoadColor(rapidxml::xml_node<> *node, const char *name, Uint32 &value)
{
	rapidxml::xml_node<> *child;

	child = node->first_node("color", 0, false);
	if (child) {
		rapidxml::xml_attribute<> *attr;
		int r = 0xFF, g = 0xFF, b = 0xFF;

		LoadNumber(child, "r", r);
		LoadNumber(child, "g", g);
		LoadNumber(child, "b", b);
		value = m_screen->MapRGB(r, g, b);
		return true;
	}
	return false;
}
void
UIArea::GetAnchorLocation(AnchorLocation spot, int *x, int *y) const
{
	switch (spot & X_MASK) {
		case X_LEFT:
			*x = m_rect.x;
			break;
		case X_CENTER:
			*x = m_rect.x + m_rect.w/2;
			break;
		case X_RIGHT:
			*x = m_rect.x + m_rect.w;
			break;
		default:
			assert(0);
	}
	switch (spot & Y_MASK) {
		case Y_TOP:
			*y = m_rect.y;
			break;
		case Y_CENTER:
			*y = m_rect.y + m_rect.h/2;
			break;
		case Y_BOTTOM:
			*y = m_rect.y + m_rect.h;
			break;
		default:
			assert(0);
	}
}

void
UIArea::CalculateAnchor(bool triggerRectChanged)
{
	int x, y;

	if (!m_anchor.element) {
		return;
	}
	m_anchor.element->GetAnchorLocation(m_anchor.anchorTo, &x, &y);

	switch (m_anchor.anchorFrom & X_MASK) {
		case X_CENTER:
			x -= Width() / 2;
			break;
		case X_RIGHT:
			x -= Width();
			break;
		default:
			break;
	}
	switch (m_anchor.anchorFrom & Y_MASK) {
		case Y_CENTER:
			y -= Height() / 2;
			break;
		case Y_BOTTOM:
			y -= Height();
			break;
		default:
			break;
	}

	x += m_anchor.offsetX;
	y += m_anchor.offsetY;
	if (x != m_rect.x || y != m_rect.y) {
		m_rect.x = x;
		m_rect.y = y;

		if (triggerRectChanged) {
			OnRectChanged();
		}
	}
}

void
UIArea::OnRectChanged()
{
	for (unsigned i = 0; i < m_anchoredAreas.length(); ++i) {
		m_anchoredAreas[i]->CalculateAnchor(true);
	}
}
