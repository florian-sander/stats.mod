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

struct slang_facts {
  struct slang_facts *next;
  int sorting;
  struct slang_facts_place *places;
};

static struct slang_facts *slang_facts_add(struct slang_facts *, int, int, char *);
//static int slang_facts_expmem(struct slang_facts *);
static void slang_facts_free(struct slang_facts *);
static int slang_facts_selectfirst(struct slang_facts *);
static int slang_facts_selectnext();
//static char *slang_facts_getfirst();
//static char *slang_facts_getnext();
static char *slang_facts_get(int);

static struct slang_facts *slang_facts_add(struct slang_facts *where, int sorting, int place, char *text)
{
  struct slang_facts *newitem, *target;

  newitem = NULL;
  if (where) {
    for (newitem = where; newitem; newitem = newitem->next)
      if (newitem->sorting == sorting)
        break;
  }
  if (!newitem) {
    newitem = nmalloc(sizeof(struct slang_facts));
    newitem->next = NULL;
    newitem->sorting = sorting;
    newitem->places = NULL;
    for (target = where; target && target->next; target = target->next);
    if (target)
      target->next = newitem;
    else
      where = newitem;
  }
  newitem->places = slang_facts_place_add(newitem->places, place, text);
  return where;
}

/*static int slang_facts_expmem(struct slang_facts *what)
{
  int size = 0;

  for (; what; what = what->next) {
    size += sizeof(struct slang_facts);
    size += slang_facts_place_expmem(what->places);
  }
  return size;
}*/

static void slang_facts_free(struct slang_facts *what)
{
  struct slang_facts *next;

  while (what) {
    next = what->next;
    slang_facts_place_free(what->places);
    nfree(what);
    what = next;
  }
}

static struct slang_facts *glob_fact;
static int slang_facts_selectfirst(struct slang_facts *what)
{
  int itype, pitype;
  locstats *ls;

  if (!glob_globstats)
    return 0;
  if (!glob_globstats->local)
    return 0;
  for (glob_fact = what; glob_fact; glob_fact = glob_fact->next) {
    sortstats(glob_globstats, glob_fact->sorting, S_DAILY);
    itype = glob_fact->sorting;
    glob_sorting = itype;
    if (itype >= 0) {
      if (!glob_globstats->slocal[S_TODAY][itype]->values[S_TODAY][itype])
        continue;
    } else {
      pitype = (itype * -1) + (TOTAL_TYPES - 1);
      ls = glob_globstats->slocal[S_TODAY][pitype];
      if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS]
		|| !ls->values[S_DAILY][T_LINES]))
	continue;
      else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES]
      		|| !ls->values[S_DAILY][T_LINES]))
	continue;
      else if ((itype == T_VOCABLES) && !ls->vocables)
	continue;
    }
    return 1;
  }
  return 0;
}

static int slang_facts_selectnext()
{
  int itype, pitype;
  locstats *ls;

  if (!glob_fact)
    return 0;
  for (glob_fact = glob_fact->next; glob_fact; glob_fact = glob_fact->next) {
    sortstats(glob_globstats, glob_fact->sorting, S_DAILY);
    itype = glob_fact->sorting;
    glob_sorting = itype;
    if (itype >= 0) {
      if (!glob_globstats->slocal[S_TODAY][itype]->values[S_TODAY][itype])
        continue;
    } else {
      pitype = (itype * -1) + (TOTAL_TYPES - 1);
      ls = glob_globstats->slocal[S_TODAY][pitype];
      if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS]
		|| !ls->values[S_DAILY][T_LINES]))
	continue;
      else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES]
      		|| !ls->values[S_DAILY][T_LINES]))
	continue;
      else if ((itype == T_VOCABLES) && !ls->vocables)
	continue;
    }
    return 1;
  }
  return 0;
}

/*
static char *slang_facts_getfirst()
{
  Assert(glob_fact);
  return slang_facts_place_getfirst(glob_fact->places);
}

static char *slang_facts_getnext()
{
  return slang_facts_place_getnext();
}
*/

static char *slang_facts_get(int place)
{
  return slang_facts_place_get(glob_fact->places, glob_fact->sorting, place);
}
