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

#ifndef _UIArea_h
#define _UIArea_h

#include "SDL_rect.h"
#include "../utils/array.h"
#include "../utils/rapidxml.h"
#include "ErrorBase.h"

enum {
	X_LEFT = 0x01,
	X_CENTER = 0x02,
	X_RIGHT = 0x4,
	X_MASK = (X_LEFT|X_CENTER|X_RIGHT),
	Y_TOP = 0x10,
	Y_CENTER = 0x20,
	Y_BOTTOM = 0x40,
	Y_MASK = (Y_TOP|Y_CENTER|Y_BOTTOM)
};
enum AnchorLocation {
	TOPLEFT = (Y_TOP|X_LEFT),
	TOP = (Y_TOP|X_CENTER),
	TOPRIGHT = (Y_TOP|X_RIGHT),
	LEFT = (Y_CENTER|X_LEFT),
	CENTER = (Y_CENTER|X_CENTER),
	RIGHT = (Y_CENTER|X_RIGHT),
	BOTTOMLEFT = (Y_BOTTOM|X_LEFT),
	BOTTOM = (Y_BOTTOM|X_CENTER),
	BOTTOMRIGHT = (Y_BOTTOM|X_RIGHT)
};

class FrameBuf;

class UIArea : public ErrorBase
{
public:
	UIArea(FrameBuf *screen, UIArea *anchor = NULL, int w = 0, int h = 0);

	bool Load(rapidxml::xml_node<> *node);

	// This function returns anchor areas by name
	virtual UIArea *GetAnchorElement(const char *name) {
		return NULL;
	}

	void SetPosition(int x, int y);
	void SetSize(int w, int h);
	void SetWidth(int w);
	void SetHeight(int h);
	void SetAnchor(AnchorLocation from, AnchorLocation to, UIArea *anchor,
					int offsetX = 0, int offsetY = 0);

	bool ContainsPoint(int x, int y) const {
		return (x >= m_rect.x && x < m_rect.x+m_rect.w &&
		        y >= m_rect.y && y < m_rect.y+m_rect.h);
	}

	FrameBuf *GetScreen() const {
		return m_screen;
	}
	const SDL_Rect &GetRect() const {
		return m_rect;
	}
	int X() const {
		return m_rect.x;
	}
	int Y() const {
		return m_rect.y;
	}
	int Width() const {
		return m_rect.w;
	}
	int Height() const {
		return m_rect.h;
	}

	virtual void Show() {
		m_shown = true;
	}
	virtual void Hide() {
		m_shown = false;
	}
	bool IsShown() const {
		return m_shown;
	}

	void AddAnchoredArea(UIArea *area) {
		m_anchoredAreas.add(area);
	}
	void DelAnchoredArea(UIArea *area) {
		m_anchoredAreas.remove(area);
	}

protected:
	bool LoadBool(rapidxml::xml_node<> *node, const char *name, bool &value);
	bool LoadNumber(rapidxml::xml_node<> *node, const char *name, int &value);
	bool LoadString(rapidxml::xml_node<> *node, const char *name, char *&value);
	bool LoadAnchorLocation(rapidxml::xml_node<> *node, const char *name, AnchorLocation &value);
	bool LoadColor(rapidxml::xml_node<> *node, const char *name, Uint32 &value);

	void GetAnchorLocation(AnchorLocation spot, int *x, int *y) const;
	void CalculateAnchor(bool triggerRectChanged = true);
	virtual void OnRectChanged();

protected:
	FrameBuf *m_screen;
	bool m_shown;

private:
	/* This is private so updates can trigger OnRectChanged() */
	SDL_Rect m_rect;

	struct {
		UIArea *element;
		AnchorLocation anchorFrom;
		AnchorLocation anchorTo;
		int offsetX, offsetY;
	} m_anchor;

	array<UIArea *> m_anchoredAreas;
};

#endif // _UIArea_h
