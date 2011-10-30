/*
  maclib:  A companion library to SDL for working with Macintosh (tm) data
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

/* These are routines to parse a Macintosh Resource Fork file

	-Sam Lantinga			(slouken@devolution.com)

Note: Most of the info in this file came from "Inside Macintosh"
*/

#include <stdlib.h>
#include <string.h>

#include "SDL_types.h"
#include "bitesex.h"
#include "Mac_Resource.h"

/* The format for AppleDouble files -- in a header file */
#define APPLEDOUBLE_MAGIC	0x00051607
#include "applefile.h"

/* The format for MacBinary files -- in a header file */
#define MACBINARY_MASK		0xFCFF
#define MACBINARY_MAGIC		0x8081
#include "macbinary.h"

/* These are the data structures that make up the Macintosh Resource Fork */
struct Resource_Header {
	Uint32	res_offset;	/* Offset of resources in file */
	Uint32	map_offset;	/* Offset of resource map in file */
	Uint32	res_length;	/* Length of the resource data */
	Uint32	map_length;	/* Length of the resource map */
};

struct Resource_Data {
	Uint32	Data_length;	/* Length of the resources data */
#ifdef SHOW_VARLENGTH_FIELDS
	Uint8	Data[0];	/* The Resource Data */
#endif
};

struct Type_entry {
	char	Res_type[4];	/* Resource type */
	Uint16	Num_rez;	/* # this type resources in map - 1 */
	Uint16	Ref_offset;	/* Offset from type list, of reference
				   list for this type */
};

struct Ref_entry {
	Uint16	Res_id;		/* The ID for this resource */
	Uint16	Name_offset;	/* Offset in name list of resource
				   name, or -1 if no name */
	Uint8	Res_attrs;	/* Resource attributes */
	Uint8	Res_offset[3];	/* 3-byte offset from Resource data */
	Uint32	Reserved;	/* Reserved for use in-core */
};

struct Name_entry {
	Uint8	Name_len;	/* Length of the following name */
#ifdef SHOW_VARLENGTH_FIELDS
	Uint8	name[0];	/* Variable length resource name */
#endif
	};

struct Resource_Map {
	Uint8	Reserved[22];	/* Reserved for use in-core */
	Uint16	Map_attrs;	/* Map attributes */
	Uint16	types_offset;	/* Offset of resource type list */
	Uint16  names_offset;	/* Offset of resource name list */
	Uint16	num_types;	/* # of types in map - 1 */
#ifdef SHOW_VARLENGTH_FIELDS
	struct Type_entry  types[0];	 /* Variable length types list */
	struct Ref_entry   refs[0];	 /* Variable length reference list */
	struct Name_entry  names[0];	 /* Variable length name list */
#endif
	};

int Res_cmp(const void *A, const void *B)
{
	struct Mac_Resource::resource *a, *b;

	a=(struct Mac_Resource::resource *)A;
	b=(struct Mac_Resource::resource *)B;

	if ( a->id > b->id )
		return(1);
	else if ( a->id < b->id )
		return(-1);
	else /* They are equal?? */
		return(0);
}

/* Here's an iterator to find heuristically (I've always wanted to use that
   word :-) a macintosh resource fork from a general mac name.

   This function may be overkill, but I want to be able to find any Macintosh
   resource fork, darn it! :)
*/
static void CheckAppleFile(PHYSFS_File *resfile, Uint32 *resbase)
{
	ASHeader header;
	if (PHYSFS_readBytes(resfile,&header.magicNum,sizeof(header.magicNum)) == sizeof(header.magicNum) &&
		(bytesex32(header.magicNum) == APPLEDOUBLE_MAGIC) ) {
		PHYSFS_readBytes(resfile, &header.versionNum,
				sizeof(header.versionNum));
		bytesex32(header.versionNum);
		PHYSFS_readBytes(resfile, &header.filler,
				sizeof(header.filler));
		PHYSFS_readBytes(resfile, &header.numEntries,
				sizeof(header.numEntries));
		bytesex16(header.numEntries);
#ifdef APPLEDOUBLE_DEBUG
mesg("Header magic: 0x%.8x, version 0x%.8x\n",
			header.magicNum, header.versionNum);
#endif

		ASEntry entry;
#ifdef APPLEDOUBLE_DEBUG
mesg("Number of entries: %d, sizeof(entry) = %d\n",
			header.numEntries, sizeof(entry));
#endif
		for ( int i = 0; i<header.numEntries; ++ i ) {
			if (PHYSFS_readBytes(resfile, &entry, sizeof(entry)) != sizeof(entry))
				break;
			bytesex32(entry.entryID);
			bytesex32(entry.entryOffset);
			bytesex32(entry.entryLength);
#ifdef APPLEDOUBLE_DEBUG
mesg("Entry (%d): ID = 0x%.8x, Offset = %d, Length = %d\n",
	i+1, entry.entryID, entry.entryOffset, entry.entryLength);
#endif
			if ( entry.entryID == AS_RESOURCE ) {
				*resbase = entry.entryOffset;
				break;
			}
		}
	}
	PHYSFS_seek(resfile, 0);
}
static void CheckMacBinary(PHYSFS_File *resfile, Uint32 *resbase)
{
	MBHeader header;
	if ( PHYSFS_readBytes(resfile, header.data, sizeof(header.data)) == sizeof(header.data) &&
		((header.Version()&MACBINARY_MASK) == MACBINARY_MAGIC) ) {
		*resbase = sizeof(header.data) + header.DataLength();
	}
	PHYSFS_seek(resfile, 0);
}
static PHYSFS_File *Open_MacRes(char **original, Uint32 *resbase)
{
	char *filename, *basename, *ptr, *newname;
	const char *dirname;
	PHYSFS_File *resfile=NULL;

	/* Search and replace characters */
	const int N_SNRS = 2;
	struct searchnreplace {
		char search;
		char replace;
	} snr[N_SNRS] = {
		{ '\0',	'\0' },
		{ ' ',	'_' },
	};
	int iterations=0;

	/* Separate the Mac name from a UNIX path */
	filename = new char[strlen(*original)+1];
	strcpy(filename, *original);
	if ( (basename=strrchr(filename, '/')) != NULL ) {
		dirname = filename;
		*(basename++) = '\0';
	} else {
		dirname = "";
		basename = filename;
	}

	for ( iterations=0; iterations < N_SNRS; ++iterations ) {
		/* Translate ' ' into '_', etc */
		/* Note that this translation is irreversible */
		for ( ptr = basename; *ptr; ++ptr ) {
			if ( *ptr == snr[iterations].search )
				*ptr = snr[iterations].replace;
		}

		/* First look for Executor (tm) resource forks */
		newname = new char[strlen(dirname)+2+1+strlen(basename)+1];
		sprintf(newname, "%s%s%%%s", dirname, (*dirname ? "/" : ""),
								basename);
		if ( (resfile=PHYSFS_openRead(newname)) != NULL ) {
			break;
		}
		delete[] newname;

		/* Look for MacBinary files */
		newname = new char[strlen(dirname)+2+strlen(basename)+4+1];
		sprintf(newname, "%s%s%s.bin", dirname, (*dirname ? "/" : ""),
								basename);
		if ( (resfile=PHYSFS_openRead(newname)) != NULL ) {
			break;
		}
		delete[] newname;

		/* Look for raw resource fork.. */
		newname = new char[strlen(dirname)+2+strlen(basename)+1];
		sprintf(newname, "%s%s%s", dirname, (*dirname ? "/" : ""),
								basename);
		if ( (resfile=PHYSFS_openRead(newname)) != NULL ) {
			break;
		}
	}
	/* Did we find anything? */
	if ( iterations != N_SNRS ) {
		*original = newname;
		*resbase = 0;

		/* Look for AppleDouble format header */
		CheckAppleFile(resfile, resbase);

		/* Look for MacBinary format header */
		CheckMacBinary(resfile, resbase);
	}
#ifdef APPLEDOUBLE_DEBUG
mesg("Resfile base = %d\n", *resbase);
#endif
	delete[] filename;
	return(resfile);
}

Mac_Resource:: Mac_Resource(const char *file)
{
	char *filename = (char *)file;
	struct Resource_Header Header;
	struct Resource_Map    Map;
	struct Type_entry      type_ent;
	struct Ref_entry       ref_ent;
	Uint16                *ref_offsets;
	Uint8                  name_len;
	unsigned long          cur_offset;
	int i, n;

	/* Clear out any variables */
	Resources = NULL;

	/* Try to open the Macintosh resource fork */
	errstr = NULL;
	if ( (filep=Open_MacRes(&filename, &base)) == NULL ) {
		error("Couldn't open resource file '%s'", filename);
		return;
	} else {
		/* Open_MacRes() passes back the real name of resource file */
		delete[] filename;
	}
	PHYSFS_seek(filep, base);

	if ( PHYSFS_readBytes(filep, &Header, sizeof(Header)) != sizeof(Header) ) {
		error("Couldn't read resource info from '%s'", filename);
		return;
	}
	bytesex32(Header.res_length);
	bytesex32(Header.res_offset);
	res_offset = Header.res_offset;
	bytesex32(Header.map_length);
	bytesex32(Header.map_offset);

	PHYSFS_seek(filep, base+Header.map_offset);
	if ( PHYSFS_readBytes(filep, &Map, sizeof(Map)) != sizeof(Map) ) {
		error("Couldn't read resource info from '%s'", filename);
		return;
	}
	bytesex16(Map.types_offset);
	bytesex16(Map.names_offset);
	bytesex16(Map.num_types);
	Map.num_types += 1;	/* Value in fork is one short */

	/* Fill in our class members */
	num_types = Map.num_types;
	Resources = new struct resource_list[num_types];
	for ( i=0; i<num_types; ++i )
		Resources[i].list = NULL;

	ref_offsets = new Uint16[num_types];
	PHYSFS_seek(filep, base+Header.map_offset+Map.types_offset+2);
	for ( i=0; i<num_types; ++i ) {
		if ( PHYSFS_readBytes(filep, &type_ent, sizeof(type_ent)) != sizeof(type_ent) ) {
			error("Couldn't read resource info from '%s'",
								filename);
			delete[] ref_offsets;
			return;
		}
		bytesex16(type_ent.Num_rez);
		bytesex16(type_ent.Ref_offset);
		type_ent.Num_rez += 1;	/* Value in fork is one short */

		strncpy(Resources[i].type, type_ent.Res_type, 4);
		Resources[i].type[4] = '\0';
		Resources[i].count   = type_ent.Num_rez;
		Resources[i].list = new struct resource[Resources[i].count];
		for ( n=0; n<Resources[i].count; ++n ) {
			Resources[i].list[n].name = NULL;
			Resources[i].list[n].data = NULL;
		}
		ref_offsets[i] = type_ent.Ref_offset;
	}

	for ( i=0; i<num_types; ++i ) {
		PHYSFS_seek(filep, 
		base+Header.map_offset+Map.types_offset+ref_offsets[i]);
		for ( n=0; n<Resources[i].count; ++n ) {
			if ( PHYSFS_readBytes(filep, &ref_ent, sizeof(ref_ent)) != sizeof(ref_ent) ) {
				error("Couldn't read resource info from '%s'",
								filename);
				delete[] ref_offsets;
				return;
			}
			bytesex16(ref_ent.Res_id);
			bytesex16(ref_ent.Name_offset);
			Resources[i].list[n].offset = 
					((ref_ent.Res_offset[0]<<16) |
                                         (ref_ent.Res_offset[1]<<8) |
                                         (ref_ent.Res_offset[2]));
			Resources[i].list[n].id = ref_ent.Res_id;

			/* Grab the name, while we're here... */
			if ( ref_ent.Name_offset == 0xFFFF ) {
				Resources[i].list[n].name = new char[1];
				Resources[i].list[n].name[0] = '\0';
			} else {
				cur_offset = PHYSFS_tell(filep);
				PHYSFS_seek(filep, 
		base+Header.map_offset+Map.names_offset+ref_ent.Name_offset);
				PHYSFS_readBytes(filep, &name_len, 1);
				Resources[i].list[n].name=new char[name_len+1];
				PHYSFS_readBytes(filep, Resources[i].list[n].name, name_len);
				Resources[i].list[n].name[name_len] = '\0';
				PHYSFS_seek(filep, cur_offset);
			}
		}
#ifndef macintosh
		/* Sort the resources in ascending order. :) */
		qsort(Resources[i].list,Resources[i].count,
					sizeof(struct resource), Res_cmp);
#endif
	}
	delete[] ref_offsets;
}

Mac_Resource:: ~Mac_Resource()
{
	if ( filep )
		PHYSFS_close(filep);

	if ( ! Resources )
		return;
	for ( int i=0; i<num_types; ++i ) {
		if ( ! Resources[i].list )
			continue;
		for ( int n=0; n<Resources[i].count; ++n ) {
			if ( Resources[i].list[n].name )
				delete[] Resources[i].list[n].name;
			if ( Resources[i].list[n].data ) {
				delete[] Resources[i].list[n].data->data;
				delete Resources[i].list[n].data;
			}
		}
		delete[] Resources[i].list;
	}
	delete[] Resources;
}

char **
Mac_Resource:: Types(void)
{
	int i;
	char **types;

	types = new char *[num_types+1];
	for ( i=0; i<num_types; ++i )
		types[i] = Resources[i].type;
	types[i] = NULL;
	return(types);
}

Uint16
Mac_Resource:: NumResources(const char *res_type)
{
	int i;

	for ( i=0; i<num_types; ++i ) {
		if ( strncmp(res_type, Resources[i].type, 4) == 0 )
			return(Resources[i].count);
	}
	return(0);
}

Uint16 *
Mac_Resource:: ResourceIDs(const char *res_type)
{
	int i, n;
	Uint16 *ids;

	for ( i=0; i<num_types; ++i ) {
		if ( strncmp(res_type, Resources[i].type, 4) == 0 ) {
			ids = new Uint16[Resources[i].count+1];
			for ( n=0; n<Resources[i].count; ++n )
				ids[n] = Resources[i].list[n].id;
			ids[n] = 0xFFFF;
			return(ids);
		}
	}
	error("Couldn't find resources of type '%s'", res_type);
	return(NULL);
}

char *
Mac_Resource:: ResourceName(const char *res_type, Uint16 id)
{
	int i, n;

	for ( i=0; i<num_types; ++i ) {
		if ( strncmp(res_type, Resources[i].type, 4) == 0 ) {
			for ( n=0; n<Resources[i].count; ++n ) {
				if ( id == Resources[i].list[n].id )
					return(Resources[i].list[n].name);
			}
		}
	}
	error("Couldn't find resource of type '%s', id %hu", res_type, id);
	return(NULL);
}

Mac_ResData *
Mac_Resource:: Resource(const char *res_type, Uint16 id)
{
	int i, n;
	Mac_ResData *d;

	for ( i=0; i<num_types; ++i ) {
		if ( strncmp(res_type, Resources[i].type, 4) == 0 ) {
			for ( n=0; n<Resources[i].count; ++n ) {
				if ( id == Resources[i].list[n].id ) {
					/* Is it already loaded? */
					d = Resources[i].list[n].data;
					if ( d )
						return(d);

					/* Load it */
					d = new Mac_ResData;
					PHYSFS_seek(filep, base+res_offset+Resources[i].list[n].offset);
					PHYSFS_readBytes(filep, &d->length, 4);
					bytesex32(d->length);
					d->data = new Uint8[d->length];
					if (PHYSFS_readBytes(filep, d->data, d->length) != d->length) {
						delete[] d->data;
						error("Couldn't read %d bytes", d->length);
						delete d;
						return(NULL);
					}
					Resources[i].list[n].data = d;
					return(d);
				}
			}
		}
	}
	error("Couldn't find resource of type '%s', id %hu", res_type, id);
	return(NULL);
}

Mac_ResData *
Mac_Resource:: Resource(const char *res_type, const char *name)
{
	int i, n;
	Mac_ResData *d;

	for ( i=0; i<num_types; ++i ) {
		if ( strncmp(res_type, Resources[i].type, 4) == 0 ) {
			for ( n=0; n<Resources[i].count; ++n ) {
				if (strcmp(name, Resources[i].list[n].name)==0){
					/* Is it already loaded? */
					d = Resources[i].list[n].data;
					if ( d )
						return(d);

					/* Load it */
					d = new Mac_ResData;
					PHYSFS_seek(filep, base+res_offset+Resources[i].list[n].offset);
					PHYSFS_readBytes(filep, &d->length, 4);
					bytesex32(d->length);
					d->data = new Uint8[d->length];
					if (PHYSFS_readBytes(filep, d->data, d->length) != d->length) {
						delete[] d->data;
						error("Couldn't read %d bytes", d->length);
						delete d;
						return(NULL);
					}
					Resources[i].list[n].data = d;
					return(d);
				}
			}
		}
	}
	error("Couldn't find resource of type '%s', name %s", res_type, name);
	return(NULL);
}

