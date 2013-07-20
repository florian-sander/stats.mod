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

struct slang_mt_content {
  struct slang_mt_content *next;
  struct slang_text *text;
};

struct slang_multitext {
  int nr;
  struct slang_mt_content *contents;
};

static struct slang_multitext *slang_mtext_add(struct slang_multitext *, char *);
//static int slang_multitext_expmem(struct slang_multitext *);
static void slang_multitext_free(struct slang_multitext *);
static char *slang_multitext_getrandomtext(struct slang_multitext *);
#ifndef SLANG_NOTYPES
static struct slang_text *slang_multitext_find(struct slang_multitext *, char *);
#endif
#ifndef SLANG_NOGETALL
static char *slang_multitext_get_first(struct slang_multitext *);
static char *slang_multitext_get_next();
#endif

static struct slang_multitext *slang_mtext_add(struct slang_multitext *where, char *text)
{
  struct slang_mt_content *oc, *nc;

  if (!where) {
    where = nmalloc(sizeof(struct slang_multitext));
    where->nr = 0;
    where->contents = NULL;
  }
  nc = nmalloc(sizeof(struct slang_mt_content));
  nc->next = NULL;
  nc->text = slang_text_parse(text);
  for (oc = where->contents; oc && oc->next; oc = oc->next);
  if (oc) {
    Assert(!oc->next);
    oc->next = nc;
  } else
    where->contents = nc;
  where->nr++;
  return where;
}

/*static int slang_multitext_expmem(struct slang_multitext *what)
{
  struct slang_mt_content *content;
  int size = 0;

  if (!what) {
    debug0("WARNING! slang_multitext_expmem() called with NULL pointer!");
    return 0;
  }
  size += sizeof(struct slang_multitext);
  for (content = what->contents; content; content = content->next) {
    size += sizeof(struct slang_mt_content);
    size += slang_text_expmem(content->text);
  }
  return size;
}*/

static void slang_multitext_free(struct slang_multitext *what)
{
  struct slang_mt_content *content, *next;

  if (!what) {
    debug0("WARNING! slang_multitext_free() called with NULL pointer!");
    return;
  }
  content = what->contents;
  while (content) {
    next = content->next;
    slang_text_free(content->text);
    nfree(content);
    content = next;
  }
  nfree(what);
}

static char *slang_multitext_getrandomtext(struct slang_multitext *where)
{
  struct slang_mt_content *content;
  unsigned long x;

  if (!where)
    return NULL;
  x = random() % where->nr;
  for (content = where->contents; content; content = content->next)
    if (!x)
      return slang_text_get(content->text);
    else
      x--;
  // we should never reach this part
  debug0("warning: getrandomtext didn't find anything!");
  return NULL;
}

#ifndef SLANG_NOTYPES
static struct slang_text *slang_multitext_find(struct slang_multitext *where, char *what)
{
  struct slang_mt_content *content;

  Assert(where);
  for (content = where->contents; content; content = content->next) {
    Assert(content->text);
    if (!slang_text_strcasecmp(content->text, what))
      return content->text;
  }
  return NULL;
}
#endif

#ifndef SLANG_NOGETALL
static struct slang_mt_content *glob_mtext_content;
static char *slang_multitext_get_first(struct slang_multitext *where)
{
  Assert(where);
  glob_mtext_content = where->contents;
  if (glob_mtext_content)
    return slang_text_get(glob_mtext_content->text);
  else
    return NULL;
}

static char *slang_multitext_get_next()
{
  glob_mtext_content = glob_mtext_content->next;
  if (glob_mtext_content)
    return slang_text_get(glob_mtext_content->text);
  else
    return NULL;
}
#endif
