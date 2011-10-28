
#ifndef _UIDialog_h
#define _UIDialog_h

#include "screenlib/UIPanel.h"


class UIDialog : public UIPanel
{
public:
	UIDialog(UIManager *ui, const char *name);
	virtual ~UIDialog();

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Show();
	virtual void Draw();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	enum {
		COLOR_BLACK,
		COLOR_DARK,
		COLOR_MEDIUM,
		COLOR_LIGHT,
		COLOR_WHITE,
		NUM_COLORS
	};
	Uint32 m_colors[NUM_COLORS];
	bool m_expand;
	int m_step;
};

#endif // _UIDialog_h
