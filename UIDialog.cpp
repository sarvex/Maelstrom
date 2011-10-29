
#include "screenlib/SDL_FrameBuf.h"
#include "screenlib/UIManager.h"

#include "UIDialog.h"

#define EXPAND_STEPS 30


UIDialog::UIDialog(UIManager *ui, const char *name) :
	UIPanel(ui, name)
{
	m_fullscreen = false;

	m_colors[COLOR_BLACK] = m_screen->MapRGB(0x00, 0x00, 0x00);
	m_colors[COLOR_DARK] = m_screen->MapRGB(0x66, 0x66, 0x99);
	m_colors[COLOR_MEDIUM] = m_screen->MapRGB(0xBB, 0xBB, 0xBB);
	m_colors[COLOR_LIGHT] = m_screen->MapRGB(0xCC, 0xCC, 0xFF);
	m_colors[COLOR_WHITE] = m_screen->MapRGB(0xFF, 0xFF, 0xFF);
	m_expand = true;
	m_step = 0;
}

UIDialog::~UIDialog()
{
}

bool
UIDialog::Load(rapidxml::xml_node<> *node, const UITemplates *templates)
{
	rapidxml::xml_attribute<> *attr;

	if (!UIPanel::Load(node, templates)) {
		return false;
	}

	attr = node->first_attribute("expand", 0, false);
	if (attr) {
		const char *value = attr->value();
		if (*value == '0' || *value == 'f' || *value == 'F') {
			m_expand = false;
		} else {
			m_expand = true;
		}
	}

	return true;
}

void
UIDialog::Show()
{
	if (m_expand) {
		m_step = 0;
	} else {
		m_step = EXPAND_STEPS;
	}
}

void
UIDialog::Draw()
{
	int x, y, w, h;
	int maxx, maxy;

	if (m_step < EXPAND_STEPS) {
		w = (Width()*m_step)/EXPAND_STEPS;
		h = (Height()*m_step)/EXPAND_STEPS;
		x = X() + Width()/2 - (w/2);
		y = Y() + Height()/2 - (h/2);
	} else {
		w = Width();
		h = Height();
		x = X();
		y = Y();
	}

	/* The border is 4 pixels around the area of the dialog */
	w += 8;
	h += 8;
	x -= 4;
	y -= 4;
	maxx = x+w-1;
	maxy = y+h-1;

	/* Draw the dialog border and background color */
	m_screen->DrawLine(x, y, maxx, y, m_colors[COLOR_LIGHT]);
	m_screen->DrawLine(x, y, x, maxy, m_colors[COLOR_LIGHT]);
	m_screen->DrawLine(x, maxy, maxx, maxy, m_colors[COLOR_DARK]);
	m_screen->DrawLine(maxx, y, maxx, maxy, m_colors[COLOR_DARK]);
	m_screen->DrawRect(x+1, y+1, w-2, h-2, m_colors[COLOR_MEDIUM]);
	m_screen->DrawLine(x+2, y+2, maxx-2, y+2, m_colors[COLOR_DARK]);
	m_screen->DrawLine(x+2, y+2, x+2, maxy-2, m_colors[COLOR_DARK]);
	m_screen->DrawLine(x+3, maxy-2, maxx-2, maxy-2, m_colors[COLOR_LIGHT]);
	m_screen->DrawLine(maxx-2, y+3, maxx-2, maxy-2, m_colors[COLOR_LIGHT]);
	m_screen->DrawRect(x+3, y+3, w-6, h-6, m_colors[COLOR_BLACK]);
	m_screen->FillRect(x+4, y+4, w-8, h-8, m_colors[COLOR_WHITE]);

	/* Don't draw the controls until we've finished expanding */
	if (m_step < EXPAND_STEPS) {
		++m_step;
		return;
	}

	UIPanel::Draw();
}

bool
UIDialog::HandleEvent(const SDL_Event &event)
{
	UIPanel::HandleEvent(event);

	if (event.type != SDL_QUIT) {
		/* Press escape to cancel out of dialogs */
		if (event.type == SDL_KEYUP &&
		    event.key.keysym.sym == SDLK_ESCAPE) {
			GetUI()->HidePanel(GetUI()->GetCurrentPanel());
		}
		return true;
	}
	return false;
}
