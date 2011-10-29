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

#ifndef _UIDialog_h
#define _UIDialog_h

#include "UIPanel.h"
#include "UIElementButton.h"

class UIDialog;

/* This function gets called when the dialog is shown.
*/
typedef void (*UIDialogInitHandler)(UIDialog *dialog);

/* This function gets called when the dialog is hidden.
   The status defaults to 0, but can be changed by dialog buttons.
 */
typedef void (*UIDialogDoneHandler)(UIDialog *dialog, int status);

class UIDialog : public UIPanel
{
public:
	UIDialog(UIManager *ui, const char *name);

	virtual bool IsA(UIElementType type) {
		return UIPanel::IsA(type) || type == GetType();
	}

	/* Set a function that's called when the dialog is hidden */
	void SetDialogHandlers(UIDialogInitHandler handleInit,
				UIDialogDoneHandler handleDone) {
		m_handleInit = handleInit;
		m_handleDone = handleDone;
	}
	void SetDialogStatus(int status) {
		m_status = status;
	}

	virtual void Show();
	virtual void Hide();
	virtual bool HandleEvent(const SDL_Event &event);

protected:
	int m_status;
	UIDialogInitHandler m_handleInit;
	UIDialogDoneHandler m_handleDone;

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
	UIDialogLauncher(UIManager *ui, const char *name, UIDialogInitHandler = NULL, UIDialogDoneHandler handleDone = NULL);

	virtual void OnClick();

protected:
	UIManager *m_ui;
	const char *m_name;
	UIDialogInitHandler m_handleInit;
	UIDialogDoneHandler m_handleDone;
};

#endif // _UIDialog_h
