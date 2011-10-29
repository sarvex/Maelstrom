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

#include "UIArea.h"


static AnchorLocation ParseAnchorLocation(const char *text)
{
	AnchorLocation value = TOPLEFT;

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
	}
	return value;

}

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

	attr = node->first_attribute("show", 0, false);
	if (attr) {
		const char *value = attr->value();
		if (*value == '0' || *value == 'f' || *value == 'F') {
			m_shown = false;
		} else {
			m_shown = true;
		}
	}

	child = node->first_node("size", 0, false);
	if (child) {
		attr = child->first_attribute("w", 0, false);
		if (attr) {
			m_rect.w = SDL_atoi(attr->value());
		}
		attr = child->first_attribute("h", 0, false);
		if (attr) {
			m_rect.h = SDL_atoi(attr->value());
		}
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

		attr = child->first_attribute("anchorFrom", 0, false);
		if (attr) {
			m_anchor.anchorFrom = ParseAnchorLocation(attr->value());
		}
		attr = child->first_attribute("anchorTo", 0, false);
		if (attr) {
			m_anchor.anchorTo = ParseAnchorLocation(attr->value());
		}

		attr = child->first_attribute("x", 0, false);
		if (attr) {
			m_anchor.offsetX = SDL_atoi(attr->value());
		}
		attr = child->first_attribute("y", 0, false);
		if (attr) {
			m_anchor.offsetY = SDL_atoi(attr->value());
		}
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
