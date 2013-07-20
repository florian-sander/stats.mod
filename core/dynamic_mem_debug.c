/*
 * Copyright (C) 2000,2001  Florian Sander
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define DYNAMIC_MEM_DEBUG 1

#ifndef __stdio_h__
#include <stdio.h>
#endif

#ifndef __string_h__
#include <string.h>
#endif

#include <errno.h>

#ifdef malloc
#undef malloc
#endif

#ifdef free
#undef free
#endif

#ifdef nmalloc
#undef nmalloc
#endif

#define nmalloc(x)	dmd_malloc((x),__FILE__,__LINE__)

#ifdef nrealloc
#undef nrealloc
#endif

#define nrealloc(x,y)	dmd_realloc((x),(y),__FILE__,__LINE__)

#ifdef nfree
#undef nfree
#endif

#define nfree(x)	dmd_free((x),__FILE__,__LINE__)

#ifndef GENERIC_BINARY_TREE
#include "generic_binary_tree.c"
#endif

#define DMD_FILE_SIZE 20

struct dmd_memblock {
	void *ptr;
	int size;
	int line;
	char file[DMD_FILE_SIZE + 1];
	int expmem;
};

static int dmd_compare(void *data1, void *data2);
static void dmd_freeblock(void *data);
static void dmd_checkblock(void *data);

static struct generic_binary_tree dmd_tree = {NULL, dmd_compare, NULL, dmd_freeblock};
static void (*dmd_expmem_func) () = NULL;

static struct dmd_memblock *dmd_create(void *ptr, int size, int line, const char *file)
{
	struct dmd_memblock *nb;
	char *p;

	nb = malloc(sizeof(struct dmd_memblock));
	nb->ptr = ptr;
	nb->size = size;
	nb->line = line;
	p = strrchr(file, '/');
	strncpy(nb->file, p ? p + 1 : file, DMD_FILE_SIZE);
	nb->file[DMD_FILE_SIZE] = 0;
	return nb;
}

static void dmd_freeblock(void *data)
{
	free((struct dmd_memblock *)data);
}

static int dmd_compare(void *data1, void *data2)
{
	struct dmd_memblock *b1 = (struct dmd_memblock *) data1, *b2 = (struct dmd_memblock *) data2;
	unsigned long v1, v2;

	v1 = (unsigned long) b1->ptr;
	v2 = (unsigned long) b2->ptr;
	if (v1 > v2)
		return 1;
	else if (v1 < v2)
		return -1;
	else
		return 0;
}

static void *dmd_malloc(int size, const char *file, int line)
{
	struct dmd_memblock *mb;
	void *ptr;

//	debug2("malloc %s:%d", file, line);
	ptr = malloc(size);
//	debug1("ptr: %u", (unsigned long) ptr);
	if (!ptr) {
		putlog(LOG_MISC, "*", "*** DMD: FAILED MALLOC %s (%d) (%d): %s", file, line, size, strerror(errno));
		fatal("Memory allocation failed", 0);
	}
	mb = dmd_create(ptr, size, line, file);
	if (btree_get(&dmd_tree, mb))
		putlog(LOG_MISC, "*", "*** DMD: DOUBLED POINTER?!?!?");
	btree_add(&dmd_tree, (void *) mb);
	return mb->ptr;
}

static void *dmd_realloc(void *old, int size, const char *file, int line)
{
	struct dmd_memblock *mb, sb;
	void *p;

	// debug4("realloc %s:%d (%ul -> %i)", file, line,  (unsigned long) old, size);
	sb.ptr = old;
	mb = btree_get(&dmd_tree, &sb);
	if (!mb) {
		putlog(LOG_MISC, "*", "*** DMD: FAILED REALLOC %s:%d (%d). Old pointer not found. This is probably fatal!", file, line, size);
		return NULL;
	}
	p = mb->ptr;
	p = realloc(p, size);
	if (!p) {
		putlog(LOG_MISC, "*", "*** DMD: FAILED REALLOC %s:%d (%d). realloc() returned NULL", file, line, size);
//		fatal("Memory re-allocation failed", 0);
	}
	if (((unsigned long) p) != ((unsigned long) old)) {
//		debug0("newpointer");
		btree_remove(&dmd_tree, mb);
		mb = dmd_create(p, size, line, file);
		btree_add(&dmd_tree, (void *) mb);
	} else {
//		debug0("oldpointer");
		mb->size = size;
/*		p = strrchr(file, '/');
		strncpy(mb->file, p ? p + 1 : file, DMD_FILE_SIZE);
		mb->file[DMD_FILE_SIZE] = 0;
		mb->line = line;*/
	}
	return p;
}

static void dmd_free(void *p, const char *file, int line)
{
	struct dmd_memblock *mb, sb;

//	debug2("free %s:%d", file, line);
	sb.ptr = p;
	mb = btree_get(&dmd_tree, &sb);
	if (!mb) {
		putlog(LOG_MISC, "*", "*** DMD: ATTEMPTING TO FREE NON-MALLOC'D PTR: %s:%d",
				file, line);
		return;
	}
	btree_remove(&dmd_tree, mb);
	free(p);
}

static void dmd_reset(void *data)
{
	struct dmd_memblock *b = (struct dmd_memblock *)data;

	b->expmem = -1;
}

static void dmd_init()
{
	btree_freetree(&dmd_tree);
}

static FILE *dmd_filepointer;
static int dmd_founderrors;
static void dmd_expmem()
{
	dmd_founderrors = 0;
	btree_getall(&dmd_tree, dmd_reset);
	if (dmd_expmem_func)
		dmd_expmem_func();
	dmd_filepointer = fopen("MEMDEBUG", "w");
	btree_getall(&dmd_tree, dmd_checkblock);
	fclose(dmd_filepointer);
	if (dmd_founderrors)
		putlog(LOG_MISC, "*", "*** DMD: %d EXPMEM ERRORS!", dmd_founderrors);
}

static void dmd_checkblock(void *data)
{
	struct dmd_memblock *b = (struct dmd_memblock *)data;

	if (b->expmem == -1) {
		fprintf(dmd_filepointer, "LEAK      : %d bytes in %s:%d\n", b->size, b->file, b->line);
		dmd_founderrors++;
	} else if (b->expmem != b->size) {
		fprintf(dmd_filepointer, "wrong size: %d bytes allocated, %d bytes expected in %s:%d\n",
				b->size, b->expmem, b->file, b->line);
		dmd_founderrors++;
	}
}

static void dmd_unload()
{
	dmd_expmem();
	btree_freetree(&dmd_tree);
}
