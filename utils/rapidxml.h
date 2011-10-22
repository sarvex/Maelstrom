#ifndef _rapidxml_h
#define _rapidxml_h

#define RAPIDXML_NO_STDLIB

#include <assert.h>

namespace std
{
	typedef ::size_t size_t;
}

extern inline void * operator new (size_t, void * p) throw() { return p ; }

#include "rapidxml.hpp"

#endif // _rapidxml_h
