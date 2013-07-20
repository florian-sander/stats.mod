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

static struct template_skin *templates_skin_add(struct template_skin *, char *, char *);
//static int templates_skin_expmem(struct template_skin *);
static void templates_skin_free(struct template_skin *);
static struct template_skin *templates_skin_find(struct template_skin *list, char *name);

static struct template_skin *templates_skin_add(struct template_skin *where, char *name, char *desc)
{
  struct template_skin *newskin;

  for (newskin = where; newskin; newskin = newskin->next)
    if (!strcasecmp(newskin->name, name))
      break;
  if (!newskin) {
    newskin = nmalloc(sizeof(struct template_skin));
    newskin->next = NULL;
    newskin->name = NULL;
    newskin->desc = NULL;
    newskin->slang = NULL;
    newskin->templates = NULL;
    newskin->name = nmalloc(strlen(name) + 1);
    strcpy(newskin->name, name);
    if (desc) {
      newskin->desc = nmalloc(strlen(desc) + 1);
      strcpy(newskin->desc, desc);
    }
    newskin->next = where;
    where = newskin;
  } else {
    // update description
    if (newskin->desc) {
      nfree(newskin->desc);
      newskin->desc = NULL;
    }
    if (desc) {
      newskin->desc = nmalloc(strlen(desc) + 1);
      strcpy(newskin->desc, desc);
    }
  }
  return where;
}

/*
static int templates_skin_expmem(struct template_skin *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct template_skin);
    Assert(what->name);
    size += strlen(what->name) + 1;
    if (what->desc)
      size += strlen(what->desc) + 1;
    size += slang_expmem(what->slang);
    size += templates_template_expmem(what->templates);
    what = what->next;
  }
  return size;
}
*/

static void templates_skin_free(struct template_skin *what)
{
  struct template_skin *next;

  while (what) {
    next = what->next;
    templates_template_free(what->templates);
    slang_free(what->slang);
    Assert(what->name);
    nfree(what->name);
    if (what->desc)
      nfree(what->desc);
    nfree(what);
    what = next;
  }
}

static struct template_skin *templates_skin_find(struct template_skin *list, char *name)
{
  while (list) {
    if (!strcasecmp(list->name, name))
      return list;
    list = list->next;
  }
  return NULL;
}
