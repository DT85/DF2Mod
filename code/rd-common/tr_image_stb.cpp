/*
===========================================================================
Copyright (C) 1999 - 2005, Id Software, Inc.
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2005 - 2015, ioquake3 contributors
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "../server/exe_headers.h"

#include "tr_common.h"

static void* R_LocalMalloc(size_t size)
{
	return R_Malloc(size, TAG_TEMP_WORKSPACE);
}

static void* R_LocalReallocSized(void *ptr, size_t old_size, size_t new_size)
{
	void *mem = R_Malloc(new_size, TAG_TEMP_WORKSPACE, qfalse);
	if (ptr)
	{
		memcpy(mem, ptr, old_size);
		R_Free(ptr);
	}
	return mem;
}
static void R_LocalFree(void *ptr)
{
	if (ptr)
		R_Free(ptr);
}

#define STBI_MALLOC R_LocalMalloc
#define STBI_REALLOC_SIZED R_LocalReallocSized
#define STBI_FREE R_LocalFree

#define STB_IMAGE_IMPLEMENTATION

#define STBI_TEMP_ON_STACK
#define STBI_ONLY_HDR
#include "rd-common/stb_image.h"

#define IMG_BYTE 0
#define IMG_FLOAT 1

// Loads a HDR image from file.
void LoadHDR ( const char *filename, byte **data, int *width, int *height, int *depth)
{
	byte *buf = NULL;
	int x, y, n;
	int len = ri.FS_ReadFile (filename, (void **)&buf);
	if ( len < 0 || buf == NULL )
	{
		return;
	}
	stbi_set_flip_vertically_on_load(0);
	*data = (byte *)stbi_loadf_from_memory(buf, len, &x, &y, &n, 3);

	ri.FS_FreeFile(buf);

	if (!data)
		ri.Printf(PRINT_DEVELOPER, "R_LoadHDR(%s) failed: %s\n", filename, stbi_failure_reason());

	if (width)
		*width = x;
	if (height)
		*height = y;
	if (depth)
		*depth = 32;
}
