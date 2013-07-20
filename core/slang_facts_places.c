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

struct slang_facts_place {
  struct slang_facts_place *next;
  int place;
  struct slang_multitext *mtext;
};

static struct slang_facts_place *slang_facts_place_add(struct slang_facts_place *, int, char *);
//static int slang_facts_place_expmem(struct slang_facts_place *);
static void slang_facts_place_free(struct slang_facts_place *);
//static char *slang_facts_place_getfirst(struct slang_facts_place *);
//static char *slang_facts_place_getnext();
static char *slang_facts_place_get(struct slang_facts_place *, int, int);

static struct slang_facts_place *slang_facts_place_add(struct slang_facts_place *where, int place, char *text)
{
  struct slang_facts_place *newitem, *target;

  newitem = NULL;
  if (where) {
    for (newitem = where; newitem; newitem = newitem->next)
      if (newitem->place == place)
        break;
  }
  if (!newitem) {
    newitem = nmalloc(sizeof(struct slang_facts_place));
    newitem->place = place;
    newitem->mtext = NULL;
    newitem->next = NULL;
    for (target = where; target && target->next; target = target->next);
    if (target)
      target->next = newitem;
    else
      where = newitem;
  }
  newitem->mtext = slang_mtext_add(newitem->mtext, text);
  return where;
}

/*static int slang_facts_place_expmem(struct slang_facts_place *what)
{
  int size = 0;

  for (; what; what = what->next) {
    size += sizeof(struct slang_facts_place);
    size += slang_multitext_expmem(what->mtext);
  }
  return size;
}*/

static void slang_facts_place_free(struct slang_facts_place *what)
{
  struct slang_facts_place *next;

  while (what) {
    next = what->next;
    slang_multitext_free(what->mtext);
    nfree(what);
    what = next;
  }
}

/*
static struct slang_facts_place *glob_fact_place;
static char *slang_facts_place_getfirst(struct slang_facts_place *where)
{
  int itype, pitype;
  locstats *ls;

  if (!glob_globstats || !glob_fact)
    return 0;
  if (!glob_globstats->local)
    return 0;
  itype = glob_fact->sorting;
  pitype = (itype * -1) + (TOTAL_TYPES - 1);
  glob_sorting = itype;


  for (glob_fact_place = what; glob_fact_place; glob_fact_place = glob_fact_place->next) {
    glob_place = 0;

    for (ls = glob_globstats->slocal[S_TODAY][pitype]; ls; ls = ls->snext[S_TODAY][pitype]) {

      // skip this fact if the value seems to be 0
      if (itype >= 0) {
	if (!glob_globstats->slocal[S_TODAY][itype]->values[S_TODAY][itype])
	  break;
      } else {
	 if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS]
		  || !ls->values[S_DAILY][T_LINES]))
	  break;
	else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES]
		  || !ls->values[S_DAILY][T_LINES]))
	  break;
	else if ((itype == T_VOCABLES) && !ls->vocables)
	  break;
      }

      glob_place++;
      if (glob_place == glob_facts_place->place) {
	glob_locstats = ls;
	return slang_multitext_get(glob_facts_place->mtext);
      }
    }
  }

  return NULL;
}

static char *slang_facts_place_getnext()
{
  int itype, pitype;
  locstats *ls;

  if (!glob_globstats || !glob_fact)
    return 0;
  if (!glob_globstats->local)
    return 0;
  itype = glob_fact->sorting;
  pitype = (itype * -1) + (TOTAL_TYPES - 1);
  glob_sorting = itype;


  for (; glob_fact_place; glob_fact_place = glob_fact_place->next) {
    glob_place = 0;

    for (ls = glob_globstats->slocal[S_TODAY][pitype]; ls; ls = ls->snext[S_TODAY][pitype]) {

      // skip this fact if the value seems to be 0
      if (itype >= 0) {
	if (!glob_globstats->slocal[S_TODAY][itype]->values[S_TODAY][itype])
	  break;
      } else {
	 if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS]
		  || !ls->values[S_DAILY][T_LINES]))
	  break;
	else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES]
		  || !ls->values[S_DAILY][T_LINES]))
	  break;
	else if ((itype == T_VOCABLES) && !ls->vocables)
	  break;
      }

      glob_place++;
      if (glob_place == glob_facts_place->place) {
	glob_locstats = ls;
	return slang_multitext_get(glob_facts_place->mtext);
      }
    }
  }

  return NULL;
}
*/

static char *slang_facts_place_get(struct slang_facts_place *where, int itype, int place)
{
  struct slang_facts_place *fp;
  locstats *ls;
  int pitype;

  if (!glob_globstats || !place)
    return NULL;
  if (itype < 0)
    pitype = (itype * -1) + (TOTAL_TYPES - 1);
  else
    pitype = itype;
  glob_sorting = itype;
  for (fp = where; fp; fp = fp->next) {
    if (fp->place == place) {
      glob_place = 0;
      for (ls = glob_globstats->slocal[S_TODAY][pitype]; ls; ls = ls->snext[S_TODAY][pitype]) {
	if (itype >= 0) {
	  if (!glob_globstats->slocal[S_TODAY][itype]->values[S_TODAY][itype])
	    return NULL;
	} else {
	   if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS]
		    || !ls->values[S_DAILY][T_LINES]))
	    return NULL;
	  else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES]
		    || !ls->values[S_DAILY][T_LINES]))
	    return NULL;
	  else if ((itype == T_VOCABLES) && !ls->vocables)
	    return NULL;
	}
	glob_place++;
	if (glob_place == fp->place) {
	  glob_locstats = ls;
	  glob_timerange = S_TODAY;
	  glob_toptype = itotype(itype);
	  return slang_multitext_getrandomtext(fp->mtext);
	}
      }
    }
  }
  return NULL;
}
