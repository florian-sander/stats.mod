/*
 * Copyright (C) 2001  Florian Sander
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

struct llist_2string {
  struct llist_2string *next;
  char *s1;
  char *s2;
};

static struct llist_2string *llist_2string_add(struct llist_2string *where, char *s1, char *s2)
{
  struct llist_2string *newitem;

  newitem = (struct llist_2string *) nmalloc(sizeof(struct llist_2string));
  newitem->next = NULL;
  newitem->s1 = (char *) nmalloc(strlen(s1) + 1);
  strcpy(newitem->s1, s1);
  newitem->s2 = (char *) nmalloc(strlen(s2) + 1);
  strcpy(newitem->s2, s2);
  newitem->next = where;
  return newitem;
}

static int llist_2string_expmem(struct llist_2string *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct llist_2string);
    size += strlen(what->s1) + 1;
    size += strlen(what->s2) + 1;
    what = what->next;
  }
  return size;
}

static void llist_2string_free(struct llist_2string *what)
{
  struct llist_2string *next;

  while (what) {
    next = what->next;
    nfree(what->s1);
    nfree(what->s2);
    nfree(what);
    what = next;
  }
}

static char *llist_2string_get_s2(struct llist_2string *where, char *s1)
{
  for (; where; where = where->next)
    if (!strcasecmp(where->s1, s1))
      return where->s2;
  return NULL;
}

/******************************/

struct llist_1string {
  struct llist_1string *next;
  char *s1;
};

static struct llist_1string *llist_1string_add(struct llist_1string *where, char *s1)
{
  struct llist_1string *newitem, *target;

  newitem = nmalloc(sizeof(struct llist_1string));
  newitem->s1 = nmalloc(strlen(s1) + 1);
  strcpy(newitem->s1, s1);
  newitem->next = NULL;
  target = where;
  while (target && target->next)
    target = target->next;
  if (target)
    target->next = newitem;
  else
    return newitem;
  return where;
}

static int llist_1string_expmem(struct llist_1string *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct llist_1string);
    size += strlen(what->s1) + 1;
    what = what->next;
  }
  return size;
}

static void llist_1string_free(struct llist_1string *what)
{
  struct llist_1string *next;

  while (what) {
    next = what->next;
    nfree(what->s1);
    nfree(what);
    what = next;
  }
}
