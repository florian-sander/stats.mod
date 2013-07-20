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

struct slang_chanlang {
  struct slang_chanlang *next;
  char *chan;
  char *lang;
};

static struct slang_chanlang *chanlangs = NULL;

static struct slang_chanlang *slang_chanlang_add(struct slang_chanlang *, char *, char *);
//static int slang_chanlang_expmem(struct slang_chanlang *);
static void slang_chanlang_free(struct slang_chanlang *);
static char *slang_chanlang_get(struct slang_chanlang *, char *);

static struct slang_chanlang *slang_chanlang_add(struct slang_chanlang *where, char *chan, char *lang)
{
  struct slang_chanlang *item;

  for (item = where; item; item = item->next)
    if (!rfc_casecmp(item->chan, chan))
      break;
  if (!item) {
    item = nmalloc(sizeof(struct slang_chanlang));
    item->chan = nmalloc(strlen(chan) + 1);
    strcpy(item->chan, chan);
    item->lang = nmalloc(strlen(lang) + 1);
    strcpy(item->lang, lang);
    item->next = where;
    where = item;
  } else {
    Assert(item->lang);
    item->lang = nrealloc(item->lang, strlen(lang) + 1);
    strcpy(item->lang, lang);
  }
  return where;
}

/*
static int slang_chanlang_expmem(struct slang_chanlang *what)
{
  int size = 0;

  while (what) {
    Assert(what);
    Assert(what->chan);
    Assert(what->lang);
    size += sizeof(struct slang_chanlang);
    size += strlen(what->chan) + 1;
    size += strlen(what->lang) + 1;
    what = what->next;
  }
  return size;
}
*/

static void slang_chanlang_free(struct slang_chanlang *what)
{
  struct slang_chanlang *next;

  while (what) {
    Assert(what);
    Assert(what->chan);
    Assert(what->lang);
    next = what->next;
    nfree(what->chan);
    nfree(what->lang);
    nfree(what);
    what = next;
  }
}

static char *slang_chanlang_get(struct slang_chanlang *where, char *chan)
{
  while (where) {
    if (!rfc_casecmp(where->chan, chan))
      return where->lang;
    where = where->next;
  }
  return default_slang;
}

/* slang_getbynick():
 * tries to find an appropriate language for nick by searching
 * him on a channel and using the language of this channel.
 */
static struct slang_header *slang_getbynick(struct slang_header *where, char *nick)
{
  struct chanset_t *chan;

#ifndef NO_EGG
  for (chan = chanset; chan; chan = chan->next)
    if (ismember(chan, nick))
#if EGG_IS_MIN_VER(10500)
      return slang_find(where, slang_chanlang_get(chanlangs, chan->dname));
#else
      return slang_find(where, slang_chanlang_get(chanlangs, chan->name));
#endif
#endif
  return slang_find(where, default_slang);
}
