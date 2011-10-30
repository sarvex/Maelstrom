/*
  snd2wav:  A program to convert Macintosh (tm) sound resources
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

/* A Macintosh sound resource converter */

#include <stdlib.h>
#include <string.h>

#include "Mac_Wave.h"

static Wave wave;

int main(int argc, char *argv[])
{
	Mac_Resource *macx;
	Mac_ResData  *snd;
	char wavname[128];
	Uint16 *ids, rate;
	int i;

	rate = 0;
	if ( (argc >= 3) && (strcmp(argv[1], "-rate") == 0) ) {
		int i;
		rate = (Uint16)atoi(argv[2]);
		for ( i=3; argv[i]; ++i ) {
			argv[i-2] = argv[i];
		}
		argv[i-2] = NULL;
		argc -= 2;
	}
	if ( argv[1] == NULL ) {
		fprintf(stderr,
		"Usage: %s [-rate <rate>] <snd_fork> [soundnum]\n", argv[0]);
		exit(1);
	}

	macx = new Mac_Resource(argv[1]);
	if ( macx->Error() ) {
		fprintf(stderr, "%s\n", macx->Error());
		delete macx;
		exit(255);
	}
	if ( macx->NumResources("snd ") == 0 ) {
		fprintf(stderr, "No sound resources in '%s'\n", argv[1]);
		delete macx;
		exit(1);
	}

	/* If a specific resource is requested, save it alone */
	if ( argv[2] ) {
		ids = new Uint16[2];
		ids[0] = atoi(argv[2]);
		ids[1] = 0xFFFF;
	} else
		ids = macx->ResourceIDs("snd ");

	for ( i=0; ids[i] != 0xFFFF; ++i ) {
		snd = macx->Resource("snd ", ids[i]);
		if ( snd == NULL ) {
			fprintf(stderr, "Warning: %s\n", macx->Error());
			continue;
		}
		wave.Load(snd, rate);
		sprintf(wavname, "snd_%d.wav", ids[i]);
		wave.Save(wavname);
	}
	delete macx;
	exit(0);
}
