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

#ifndef _ErrorBase_h
#define _ErrorBase_h

#include <string.h>
#include <stdarg.h>

#define ERRORBASE_ERRSIZ	1024

class ErrorBase {

public:
	ErrorBase() : errstr(NULL) { }
	virtual ~ErrorBase() {
		ClearError();
	}

	/* Error message routine */
	char *Error(void) {
		return(errstr);
	}

	void ClearError() {
		if (errstr) {
			delete[] errstr;
		}
		errstr = NULL;
	}

protected:
	/* Error message */
	void SetError(const char *fmt, ...) {
		va_list ap;

		va_start(ap, fmt);
		if (!errstr) {
			errstr = new char[ERRORBASE_ERRSIZ];
		}
		vsnprintf(errstr, ERRORBASE_ERRSIZ, fmt, ap);
		va_end(ap);
        }
	char *errstr;
};

#endif /* _ErrorBase_h */
