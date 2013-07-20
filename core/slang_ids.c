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

struct slang_id {
  struct slang_id *next;
  int id;
  struct slang_multitext *mtext;
};

static struct slang_id* slang_id_add(struct slang_id *, int, char *);
//static int slang_id_expmem(struct slang_id *);
static void slang_id_free(struct slang_id *);
static char *slang_id_get(struct slang_id *, int);

static struct slang_id* slang_id_add(struct slang_id *where, int id, char *text)
{
  struct slang_id *newitem;

  newitem = NULL;
  if (where) {
    for (newitem = where; newitem; newitem = newitem->next)
      if (newitem->id == id)
        break;
  }
  if (!newitem) {
    newitem = nmalloc(sizeof(struct slang_id));
    newitem->next = NULL;
    newitem->id = id;
    newitem->mtext = NULL;
    if (where)
      newitem->next = where;
    else
      newitem->next = NULL;
    where = newitem;
  }
  newitem->mtext = slang_mtext_add(newitem->mtext, text);
  return where;
}

/*
static int slang_id_expmem(struct slang_id *what)
{
  int size = 0;

  for (; what; what = what->next) {
    size += sizeof(struct slang_id);
    size += slang_multitext_expmem(what->mtext);
  }
  return size;
}
*/

static void slang_id_free(struct slang_id *what)
{
  struct slang_id *next;

  while (what) {
    next = what->next;
    slang_multitext_free(what->mtext);
    nfree(what);
    what = next;
  }
}

static char *slang_id_get(struct slang_id *where, int i)
{
  while (where) {
    if (where->id == i)
      return slang_multitext_getrandomtext(where->mtext);
    where = where->next;
  }
  return NULL;
}

#ifndef SLANG_NOGETALL
static char *slang_id_get_first(struct slang_id *where, int id)
{
  while (where) {
    if (where->id == id) {
      return slang_multitext_get_first(where->mtext);
    }
    where = where->next;
  }
  return NULL;
}

static char *slang_id_get_next()
{
  return slang_multitext_get_next();
}
#endif
