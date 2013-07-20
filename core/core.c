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

#ifndef assert
#include <assert.h>
#endif

#ifndef Assert
#define Assert assert
#endif

#ifdef NO_EGG
#include "core/compat/noegg.h"
#include "core/compat/noegg.c"
#endif

#ifndef NO_MEM_DEBUG
#ifndef DYNAMIC_MEM_DEBUG
#include "core/dynamic_mem_debug.c"
#endif
#endif

#ifndef DYNAMIC_MEM_DEBUG

#	ifndef nmalloc
#		define nmalloc(x) malloc(x)
#	endif

#	ifndef nfree
#		define nfree(x) free(x)
#	endif

#	ifndef nreallc
#		define nrealloc(p, i) realloc(p, i)
#	endif

#endif

#ifndef Context
#define Context
#endif

#include "core/generic_linked_list.c"
#include "core/llists.c"

#include "core/data.h"
#include "core/schan.h"
#include "core/schan_members.h"
#include "core/userrec.h"
#include "core/slang.h"
#include "core/mini_httpd.h"
#include "core/templates.h"
#include "core/misc.h"


static struct stats_global *sdata = NULL;
static struct stats_userlist *suserlist = NULL;
static struct slang_header *coreslangs = NULL;
static struct llist_header schanset = {NULL, NULL, 0, schan_compare, schan_expmem, schan_free};


#include "core/vars.c"
#include "core/global_vars.c"

#include "core/slang.c"

#include "core/datahandling.c"
#include "core/data_sorting.c"

#include "core/mini_httpd.c"

#include "core/slang_stats_commands.c"

#include "core/schan.c"
#include "core/schan_members.c"
#include "core/schan_interface.c"

#include "core/sensors.c"

#include "core/userrec.c"

#include "core/user.c"
#include "core/templates.c"
#include "core/templates_stats_commands.c"
#include "core/templates_httpd_commands.c"
#include "core/http_processing.c"
#include "core/misc.c"

static void stats_core_init()
{
	sdata = NULL;
	suserlist = NULL;
	coreslangs = NULL;
}

static void stats_core_unload()
{
	slang_free(coreslangs);
}
