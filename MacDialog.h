
#ifndef _MacDialog_h
#define _MacDialog_h

#include "screenlib/UIDialog.h"

class MacDialog : public UIDialog
{
public:
	MacDialog(UIManager *ui, const char *name);

	virtual bool IsA(UIElementType type) {
		return UIDialog::IsA(type) || type == GetType();
	}

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

protected:
	static UIElementType s_elementType;

public:
	static UIElementType GetType() {
		if (!s_elementType) {
			s_elementType = GenerateType();
		}
		return s_elementType;
	}
};

#endif // _MacDialog_h
