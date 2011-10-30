
#ifndef _MacDialog_h
#define _MacDialog_h

#include "screenlib/UIDialog.h"

class MacDialog : public UIDialog
{
DECLARE_TYPESAFE_CLASS(UIDialog)
public:
	MacDialog(UIManager *ui, const char *name);

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Show();
	virtual void Draw();

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

#endif // _MacDialog_h
