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

#include "templates_commands.c"
#include "templates_content.c"
#include "templates_standard_commands.c"
#include "templates_skin.c"
#include "templates_template.c"

static struct template_skin *skins;

/* init_templates()
 * initializes some global variables
 */
static void init_templates()
{
  skins = NULL;
  glob_tpl_cmd_list = NULL;
  glob_tpl_cmd_list = templates_commands_list_add(glob_tpl_cmd_list, templates_standard_commands);
}

/* unload_templates()
 * removes every template-related stuff from memory
 */
static void unload_templates()
{
  Context;
  templates_skin_free(skins);
  templates_commands_list_free(glob_tpl_cmd_list);
  skins = NULL;
  Context;
}

/* expmem_templates():
 * returns the memory usage of the template-system
 */
/*
static int expmem_templates()
{
  int size = 0;

  Context;
  size += templates_skin_expmem(skins);
  size += templates_commands_list_expmem(glob_tpl_cmd_list);
  Context;
  return size;
}
*/

static int loadskin(char *parbuf)
{
  FILE *f;
  char *buf, *s, *cmd, *str_skin, *name, *filename, *shortname, *longname;
  char *conffile, *path, *filebuf;
  struct template_skin *skin;
  struct template_content *content;
  struct slang_header *slang;
  int len;

  f = fopen(parbuf, "r");
  if (!f) {
    putlog(LOG_MISC, "*", "ERROR loading skin! Couldn't open config "
           "file '%s'!", parbuf);
    return 0;
  }
  debug1("parbuf: '%s'", parbuf);
  conffile = inverted_csplit(&parbuf, '/');
  path = parbuf;
  debug2("path: '%s', conffile: '%s'", path, conffile);
  if (!path[0]) {
    path = ".";
    debug1("empty path, new path: %s", path);
  }
  skin = NULL;
  buf = nmalloc(2000);
  while (!feof(f)) {
    s = buf;
    if (fgets(s, 2000, f)) {
      // at first, kill those stupid line feeds and carriage returns...
      if (s[strlen(s) - 1] == '\n')
        s[strlen(s) - 1] = 0;
      if (s[strlen(s) - 1] == '\r')
        s[strlen(s) - 1] = 0;
      if (!s[0])
        continue;
      cmd = newsplit(&s);
      if (!strcasecmp(cmd, "skin")) {
        str_skin = newsplit(&s);
        debug2("adding skin '%s' (%s)", str_skin, s);
        skins = templates_skin_add(skins, str_skin, s);
        skin = templates_skin_find(skins, str_skin);
        if (!skin) {
          putlog(LOG_MISC, "*", "ERROR loading skin: unknown error creating skin structure!");
          fclose(f);
          nfree(buf);
          return 0;
        }
      } else if (!strcasecmp(cmd, "template")) {
        name = newsplit(&s);
        filename = newsplit(&s);
        if (!name[0] || !filename[0]) {
          putlog(LOG_MISC, "*", "ERROR loading template: Too few parameters!");
          continue;
        }
        len = strlen(path) + 1 + strlen(filename) + 1;
        filebuf = nmalloc(len);
        snprintf(filebuf, len, "%s/%s", path, filename);
        putlog(LOG_MISC, "*", "Loading template '%s' from '%s'...", name, filebuf);
        content = templates_content_load(filebuf);
        nfree(filebuf);
        if (!content) {
          putlog(LOG_MISC, "*", "ERROR loading template from '%s'!", filename);
          continue;
        }
        skin->templates = templates_template_add_parsedcontent(skin->templates,
                                                                name, content);
      } else if (!strcasecmp(cmd, "slang")) {
        filename = newsplit(&s);
        shortname = newsplit(&s);
        longname = s;
        if (!shortname[0] || !longname[0] || !filename[0]) {
          putlog(LOG_MISC, "*", "ERROR loading slang for skin '%s': too few "
                 "parameters!", skin->name);
          continue;
        }
        skin->slang = slang_create(skin->slang, shortname, longname);
        slang = slang_find(skin->slang, shortname);
        len = strlen(path) + 1 + strlen(filename) + 1;
        filebuf = nmalloc(len);
        snprintf(filebuf, len, "%s/%s", path, filename);
        if (!slang_load(slang, filebuf)) {
          putlog(LOG_MISC, "*", "ERROR loading slang for skin '%s'",
                 skin->name);
          nfree(filebuf);
          continue;
        }
        nfree(filebuf);
      }
    }
  }
  nfree(buf);
  return 1;
}

static void template_send(struct template_skin *skin, char *name, int idx)
{
  struct templates_template *tpl;

  Assert(skin);
  tpl = templates_template_find(skin->templates, name);
  if (!tpl) {
    dprintf(idx, "<H1>Template not found: %s</H1>", name);
    return;
  }
  templates_content_send(tpl->contents, idx);
}
