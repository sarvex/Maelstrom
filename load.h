
#ifndef _load_h
#define _load_h

#include <physfs.h>
#include "physfsrwops.h"

#include "SDL_FrameBuf.h"

/* Functions exported from load.cpp */
extern SDL_Surface *Load_Title(FrameBuf *screen, int title_id);
extern SDL_Surface *GetCIcon(FrameBuf *screen, short cicn_id);

#endif /* _load_h */
