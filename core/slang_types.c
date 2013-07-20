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

struct slang_type {
  struct slang_type *next;
  char *type;
  struct slang_multitext *mtext;
};

static struct slang_type *slang_type_add(struct slang_type *, char *, char *);
//static int slang_type_expmem(struct slang_type *);
static void slang_type_free(struct slang_type *);
static char *slang_type_get(struct slang_type *, char *);

static struct slang_type *slang_type_add(struct slang_type *where, char *type, char *text)
{
  struct slang_type *newitem;

  newitem = NULL;
  if (where) {
    for (newitem = where; newitem; newitem = newitem->next)
      if (!strcasecmp(newitem->type, type))
        break;
  }
  if (!newitem) {
    newitem = nmalloc(sizeof(struct slang_type));
    newitem->type = nmalloc(strlen(type) + 1);
    strcpy(newitem->type, type);
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

/*static int slang_type_expmem(struct slang_type *what)
{
  int size = 0;

  for (; what; what = what->next) {
    size += sizeof(struct slang_type);
    size += strlen(what->type) + 1;
    size += slang_multitext_expmem(what->mtext);
  }
  return size;
}*/

static void slang_type_free(struct slang_type *what)
{
  struct slang_type *next;

  while (what) {
    next = what->next;
    slang_multitext_free(what->mtext);
    nfree(what->type);
    nfree(what);
    what = next;
  }
}

static char *slang_type_get(struct slang_type *where, char *type)
{
  while (where) {
    if (!strcasecmp(where->type, type))
      return slang_multitext_getrandomtext(where->mtext);
    where = where->next;
  }
  return NULL;
}

static char *slang_type_slang2type(struct slang_type *where, char *slang)
{
  Assert(slang);
  while (where) {
    if (slang_multitext_find(where->mtext, slang))
      return where->type;
  }
  return NULL;
}
