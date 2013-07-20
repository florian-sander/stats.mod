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

/*
static int templates_content_expmem(struct template_content *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct template_content);
    if (what->html)
      size += strlen(what->html) + 1;
    if (what->charpar1)
      size += strlen(what->charpar1) + 1;
    size += templates_content_expmem(what->subcontent);
    what = what->next;
  }
  return size;
}
*/

static void templates_content_free(struct template_content *what)
{
  struct template_content *next;

  while (what) {
    next = what->next;
    if (what->html)
      nfree(what->html);
    if (what->charpar1)
      nfree(what->charpar1);
    templates_content_free(what->subcontent);
    nfree(what);
    what = next;
  }
}

#define TEMPLATE_LINE_LENGTH 1024
static struct template_content *templates_content_load(char *filename)
{
  FILE *f;
  char buf[TEMPLATE_LINE_LENGTH + 1], *contentstring;
  struct template_content *content;

  Context;
  // at first, load the whole file into a buffer
  f = fopen(filename, "r");
  if (f == NULL) {
    putlog(LOG_MISC, "*", "Couldn't open template from %s!", filename);
    return NULL;
  }
  contentstring = nmalloc(1);
  contentstring[0] = 0;
  while (!feof(f)) {
    if (fgets(buf, TEMPLATE_LINE_LENGTH, f)) {
      buf[TEMPLATE_LINE_LENGTH] = 0;
      contentstring = nrealloc(contentstring, strlen(contentstring) + strlen(buf) + 1);
      strcat(contentstring, buf);
    }
  }
  fclose(f);
  // now process the content
  content = templates_content_parse(contentstring);
  nfree(contentstring);
  return content;
}

static struct template_content *templates_content_create()
{
  struct template_content *newcontent;

  newcontent = nmalloc(sizeof(struct template_content));
  newcontent->next = NULL;
  newcontent->html = NULL;
  newcontent->command = NULL;
  newcontent->what = 0;
  newcontent->charpar1 = NULL;
  newcontent->floatpar1 = 0.0;
  newcontent->floatpar2 = 0.0;
  newcontent->intpar1 = 0;
  newcontent->subcontent = NULL;
  return newcontent;
}

static struct template_content *templates_content_append(struct template_content *where, struct template_content *what)
{
  struct template_content *c;

  if (what->next)
    debug0("WARNING in templates_content_append(): what->next does exist!");
  for (c = where; c && c->next; c = c->next);
  if (c)
    c->next = what;
  else {
    Assert(!where);
    where = what;
  }
  return where;
}

/* template_parse_content():
 * parse the content and return a pointer to the filled content struct
 */
static struct template_content *templates_content_parse(char *buf)
{
  char *s, *cmdstart, *cmdend, *cmd, *included_text, *end_tag;
  char tag_buf[100];
  struct llist_2string *params;
  int need_end_tag;
  struct template_content *content;

  Context;
  content = NULL;
  while ((s = strstr(buf, "<?"))) {
    // initialize variables
    need_end_tag = 1;
    included_text = cmdstart = cmd = cmdend = end_tag = NULL;
    params = NULL;

    // cut the tag from the leading text
    s[0] = 0;
    s += 2;

    content = templates_content_addhtml(content, buf);

    cmdstart = buf = s;

    // and find the end of the tag
    cmdend = strstr(cmdstart, "?>");
    if (!cmdend) {
      putlog(LOG_MISC, "*", "ERROR parsing template: tag not terminated! (%s)", cmdstart);
      continue;
    }
    cmdend[0] = 0;
    s = buf = cmdend + 2;

    // if the command isn't really a command, but a comment, then
    // just skip it.
    if (!strncmp(cmdstart, "--", 2))
      continue;

    // check if we need a seperate end-tag, or if the tag is already terminated
    // (following XML-style)
    if (cmdstart[strlen(cmdstart) - 1] == '/') {
      need_end_tag = 0;
      cmdstart[strlen(cmdstart) - 1] = 0;
    }

    // now get the name of the command.
    cmd = newsplit(&cmdstart);

    // find the ending tag if needed...
    if (need_end_tag) {
      included_text = s;
      snprintf(tag_buf, sizeof(tag_buf), "<?/%s?>", (cmd[0] == '!') ? cmd + 1 : cmd);
      end_tag = strstr(s, tag_buf);
      if (!end_tag) {
        putlog(LOG_MISC, "*", "ERROR parsing template: end-tag (%s) not found!", tag_buf);
        continue;
      }
      end_tag[0] = 0;
      s = buf = end_tag + strlen(tag_buf);
    }

    // if this is just a comment or an disabled tag, then don't parse or store it all all.
    if (!strcmp(cmd, "comment") || (cmd[0] == '!'))
      continue;

    // parse the parameters
    params = templates_content_parseparams(cmdstart);

    // and finally add the tag with parameters and included text to
    // our template
    content = templates_commands_addtocontent(content, cmd, params, included_text);

    // now free the params again...
    llist_2string_free(params);
  }

  // append all remaining html code
  content = templates_content_addhtml(content, buf);

  return content;
}

static struct template_content *templates_content_addhtml(struct template_content *where, char *html)
{
  struct template_content *newcontent;

  Assert(html);
  newcontent = templates_content_create();
  newcontent->html = nmalloc(strlen(html) + 1);
  strcpy(newcontent->html, html);
  where = templates_content_append(where, newcontent);
  return where;
}

static struct llist_2string *templates_content_parseparams(char *buf)
{
  struct llist_2string *params;
  char *name, *value, *s;

  Assert(buf);
  params = NULL;

  while (buf[0]) {
    while (buf[0] == ' ')
      buf++;
    s = buf;
    name = csplit(&buf, '=');
    if (buf[0] != '"') {
      putlog(LOG_MISC, "*", "ERROR parsing parameters: missing '\"'! (%s)", name);
      continue;
    }
    buf++;
    value = buf;
    while (buf[0]) {
      if (buf[0] == '"')
        break;
      buf++;
    }
    if (buf[0] != '"') {
      putlog(LOG_MISC, "*", "ERROR parsing parameters: missing '\"'! (%s)", name);
      continue;
    }
    buf[0] = 0;
    buf++;
    params = llist_2string_add(params, name, value);
  }
  return params;
}

static void templates_content_send(struct template_content *tpc, int idx)
{
  for (;tpc; tpc = tpc->next) {
    if (tpc->html)
      dprintf(idx, "%s", tpc->html);
    else if (tpc->command)
      tpc->command(idx, tpc);
    else
      dprintf(idx, "<H1>ERROR: No content!</H1>");
  }
}
