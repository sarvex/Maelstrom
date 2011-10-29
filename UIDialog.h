
#ifndef _UIDialog_h
#define _UIDialog_h

#include "screenlib/UIPanel.h"
#include "screenlib/UIElementButton.h"

class UIDialog;

/* This function gets called when the dialog is hidden.
   The status defaults to 0, but can be changed by dialog buttons.
 */
typedef void (*UIDialogHandler)(UIDialog *dialog, int status);

class UIDialog : public UIPanel
{
public:
	UIDialog(UIManager *ui, const char *name);
	virtual ~UIDialog();

	virtual bool IsA(UIElementType type) {
		return UIPanel::IsA(type) || type == GetType();
	}

	/* Set a function that's called when the dialog is hidden */
	void SetDialogHandler(UIDialogHandler handler) {
		m_handler = handler;
	}
	void SetDialogStatus(int status) {
		m_status = status;
	}

	virtual bool Load(rapidxml::xml_node<> *node, const UITemplates *templates);

	virtual void Show();
	virtual void Hide();
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
	int m_status;
	UIDialogHandler m_handler;

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

//
// A class to make it easy to launch a dialog from a button
//
class UIDialogLauncher : public UIButtonDelegate
{
public:
	UIDialogLauncher(UIManager *ui, const char *name, UIDialogHandler handler = NULL);

	virtual void OnClick();

protected:
	UIManager *m_ui;
	const char *m_name;
	UIDialogHandler m_handler;
};

#endif // _UIDialog_h
