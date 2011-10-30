/*
  macres:  A program to extract data from Macintosh (tm) resource forks
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

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

/* Test program to list and extract Macintosh resources from a resource fork */

#include <stdlib.h>

#include "SDL.h"
#include "Mac_Resource.h"

int main(int argc, char *argv[])
{
	Mac_Resource *res;
	char      **types;
	Uint16     *ids;
	int         i, j;

	if ( ! argv[1] ) {
		fprintf(stderr, "Usage: %s <Mac Resource File>\n", argv[0]);
		exit(1);
	}

	res = new Mac_Resource(argv[1]);
	if ( res->Error() ) {
		fprintf(stderr, "Mac_Resource: %s\n", res->Error());
		delete res;
		exit(2);
	}
	
	types = res->Types();
	for ( i=0; types[i]; ++i ) {
		ids = res->ResourceIDs(types[i]);
		printf("Resource set: type = '%s', contains %hd resources\n",
					types[i], res->NumResources(types[i]));
		for ( j=0; ids[j] < 0xFFFF; ++j ) {
			printf("\tResource %hu (ID = %d): \"%s\"\n", j+1,
				ids[j], res->ResourceName(types[i], ids[j]));
			if ( argv[2] ) {
				char path[23];
				sprintf(path,"%s/%s:%hu", argv[2],
							types[i], ids[j]);
				FILE *output;
				Mac_ResData *D;
            			if ( (output=fopen(path, "w")) != NULL ) {
					D = res->Resource(types[i], ids[j]);
					fwrite(D->data, D->length, 1,  output);
					fclose(output);               
            			}
			}
		}
		delete[]  ids;
	}
	delete[] types;
	delete res;
	exit(0);
}
