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

#include "SDL_FrameBuf.h"
#include "UIManager.h"
#include "UIDialog.h"

UIElementType UIDialog::s_elementType;


UIDialog::UIDialog(UIManager *ui, const char *name) :
	UIPanel(ui, name)
{
	m_fullscreen = false;
	m_status = 0;
	m_handleInit = NULL;
	m_handleDone = NULL;
}

void
UIDialog::Show()
{
	m_status = 0;

	if (m_handleInit) {
		m_handleInit(this);
	}

	UIPanel::Show();
}

void
UIDialog::Hide()
{
	UIPanel::Hide();

	if (m_handleDone) {
		m_handleDone(this, m_status);
	}
}

bool
UIDialog::HandleEvent(const SDL_Event &event)
{
	if (UIPanel::HandleEvent(event)) {
		return true;
	}

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

UIDialogLauncher::UIDialogLauncher(UIManager *ui, const char *name, UIDialogInitHandler handleInit, UIDialogDoneHandler handleDone)
{
	m_ui = ui;
	m_name = name;
	m_handleInit = handleInit;
	m_handleDone = handleDone;
}

void
UIDialogLauncher::OnClick()
{
	UIDialog *dialog;

	dialog = m_ui->GetPanel<UIDialog>(m_name);
	if (dialog) {
		dialog->SetDialogHandlers(m_handleInit, m_handleDone);

		m_ui->ShowPanel(dialog);
	}
}
