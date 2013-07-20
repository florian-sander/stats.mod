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

#define DURATIONS 13

struct slang_duration {
  char *durs[DURATIONS];
};

static struct slang_duration *slang_duration_add(struct slang_duration *where, int idx, char *text)
{
  int i;

  if ((idx < 0) || (idx >= DURATIONS)) {
    putlog(LOG_MISC, "*", "Warning: Invalid duration index \"%d\".", idx);
    return where;
  }
  debug2("Adding duration[%d]: %s", idx, text);
  if (!where) {
    where = nmalloc(sizeof(struct slang_duration));
    for (i = 0; i < DURATIONS; i++)
      where->durs[i] = NULL;
  }
  if (where->durs[idx])
    nfree(where->durs[idx]);
  where->durs[idx] = nmalloc(strlen(text) + 1);
  strcpy(where->durs[idx], text);
  return where;
}

/*static int slang_duration_expmem(struct slang_duration *what)
{
  int i, size = 0;

  if (!what)
    return 0;
  size += sizeof(struct slang_duration);
  for (i = 0; i < DURATIONS; i++)
    if (what->durs[i])
      size += strlen(what->durs[i]) + 1;
  return size;
}*/

static void slang_duration_free(struct slang_duration *what)
{
  int i;

  if (what) {
    for (i = 0; i < DURATIONS; i++)
      if (what->durs[i])
        nfree(what->durs[i]);
    nfree(what);
  }
}

static char *slang_duration_get(struct slang_duration *where, int idx)
{
  if (!where) {
    debug0("no where");
    return NULL;
  }
  if ((idx < 0) || (idx >= DURATIONS)) {
    debug1("invalid duration index: %d", idx);
    return NULL;
  }
  return where->durs[idx];
}
