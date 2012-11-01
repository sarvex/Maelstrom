/*
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/* XML resource loading */

#include <stdio.h>
#include "../physfs/physfs.h"
#include "loadxml.h"

#ifdef RAPIDXML_NO_EXCEPTIONS
const char *gLoadXMLError = NULL;
#endif

bool
LoadXML(const char *file, char *&buffer, rapidxml::xml_document<> &doc)
{
	PHYSFS_File *fp;
	PHYSFS_sint64 size;

	fp = PHYSFS_openRead(file);
	if (!fp) {
		/*
		fprintf(stderr, "Warning: Couldn't open %s: %s\n",
					file, PHYSFS_getLastError());
		*/
		return false;
	}

	size = PHYSFS_fileLength(fp);
	buffer = new char[size+1];
	if (PHYSFS_readBytes(fp, buffer, size) != size) {
		/*
		fprintf(stderr, "Warning: Couldn't read from %s: %s\n",
					file, PHYSFS_getLastError());
		*/
		PHYSFS_close(fp);
		delete[] buffer;
		return false;
	}
	buffer[size] = '\0';
	PHYSFS_close(fp);

#ifdef RAPIDXML_NO_EXCEPTIONS
	gLoadXMLError = NULL;
	doc.parse<0>(buffer);
	if (gLoadXMLError) {
		/*
		fprintf(stderr, "Warning: Couldn't parse %s: error: %s\n",
					file, gLoadXMLError);
		*/
		delete[] buffer;
		return false;
	}
#else
	try {
		doc.parse<0>(buffer);
	} catch (rapidxml::parse_error e) {
		/*
		fprintf(stderr, "Warning: Couldn't parse %s: error: %s\n",
					file, e.what());
		*/
		delete[] buffer;
		return false;
	}
#endif // RAPIDXML_NO_EXCEPTIONS

	return true;
}

#ifdef RAPIDXML_NO_EXCEPTIONS
void rapidxml::parse_error_handler(const char *what, void *where)
{
	gLoadXMLError = what;
}
#endif // RAPIDXML_NO_EXCEPTIONS
