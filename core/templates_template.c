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

static struct templates_template *templates_template_add_parsedcontent(struct templates_template *where, char *name, struct template_content *content)
{
  struct templates_template *newtemplate;

  Assert(name);
  Assert(content);
  for (newtemplate = where; newtemplate; newtemplate = newtemplate->next)
    if (!strcmp(newtemplate->name, name))
      break;
  if (!newtemplate) {
    newtemplate = nmalloc(sizeof(struct templates_template));
    newtemplate->next = NULL;
    newtemplate->name = NULL;
    newtemplate->contents = NULL;
    newtemplate->name = nmalloc(strlen(name) + 1);
    strcpy(newtemplate->name, name);
    newtemplate->next = where;
    where = newtemplate;
  } else
    templates_content_free(newtemplate->contents);
  newtemplate->contents = content;
  return where;
}

/*
static int templates_template_expmem(struct templates_template *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct templates_template);
    Assert(what->name);
    size += strlen(what->name) + 1;
    size += templates_content_expmem(what->contents);
    what = what->next;
  }
  return size;
}
*/

static void templates_template_free(struct templates_template *what)
{
  struct templates_template *next;

  while (what) {
    next = what->next;
    Assert(what->name);
    nfree(what->name);
    templates_content_free(what->contents);
    nfree(what);
    what = next;
  }
}

static struct templates_template *templates_template_find(struct templates_template *where, char *name)
{
  Assert(name);
  while (where) {
    if (!strcasecmp(where->name, name))
      return where;
    where = where->next;
  }
  return NULL;
}
