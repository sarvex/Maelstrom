
#ifndef _load_h
#define _load_h

#include <physfs.h>
#include "utils/physfsrwops.h"

#include "screenlib/SDL_FrameBuf.h"

/* Functions exported from load.cpp */
extern SDL_Texture *Load_Title(FrameBuf *screen, int title_id);
extern SDL_Texture *GetCIcon(FrameBuf *screen, short cicn_id);

#endif /* _load_h */
