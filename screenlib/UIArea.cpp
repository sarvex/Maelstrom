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

UIArea::UIArea() : ErrorBase()
{
	m_rect.x = 0;
	m_rect.y = 0;
	m_rect.w = 0;
	m_rect.h = 0;
	m_shown = true;
}

bool
UIArea::Load(rapidxml::xml_node<> *node)
{
	rapidxml::xml_node<> *child;
	rapidxml::xml_attribute<> *attr;

	attr = node->first_attribute("shown", 0, false);
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
			m_rect.w = atoi(attr->value());
		}
		attr = child->first_attribute("h", 0, false);
		if (attr) {
			m_rect.h = atoi(attr->value());
		}
	}

	child = node->first_node("anchor", 0, false);
	if (child) {
		UIArea *anchorElement;
		AnchorLocation anchorFrom = TOPLEFT;
		AnchorLocation anchorTo = TOPLEFT;
		int x, y;

		attr = child->first_attribute("anchorElement", 0, false);
		anchorElement = GetAnchorElement(attr ? attr->value() : NULL);
		if (!anchorElement) {
			SetError("Element 'anchor' couldn't find anchor element %s",
				attr ? attr->value() : "NULL");
			return false;
		}

		attr = child->first_attribute("anchorFrom", 0, false);
		if (attr) {
			anchorFrom = ParseAnchorLocation(attr->value());
		}
		attr = child->first_attribute("anchorTo", 0, false);
		if (attr) {
			anchorTo = ParseAnchorLocation(attr->value());
		}
		anchorElement->GetAnchorLocation(anchorTo, &x, &y);

		switch (anchorFrom & X_MASK) {
			case X_CENTER:
				x -= Width() / 2;
				break;
			case X_RIGHT:
				x -= Width();
				break;
			default:
				break;
		}
		switch (anchorFrom & Y_MASK) {
			case Y_CENTER:
				y -= Height() / 2;
				break;
			case Y_BOTTOM:
				y -= Height();
				break;
			default:
				break;
		}

		attr = child->first_attribute("x", 0, false);
		if (attr) {
			x += atoi(attr->value());
		}
		attr = child->first_attribute("y", 0, false);
		if (attr) {
			y += atoi(attr->value());
		}

		m_rect.x = x;
		m_rect.y = y;
	}

	return true;
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
