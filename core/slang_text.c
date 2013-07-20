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

struct slang_text {
  struct slang_text *next;
  char *string;
  void (*command) ();
};

struct slang_text_commands {
  char *command;
  void (*targetfunc) ();
};

struct slang_command_list {
  struct slang_command_list *next;
  struct slang_text_commands *commands;
};

static struct slang_text *slang_text_parse(char *);
static struct slang_text *slang_text_create(struct slang_text *);
static void slang_text_add_string(struct slang_text *, char *);
static void slang_text_add_command(struct slang_text *, char *);
static void slang_text_free(struct slang_text *);
//static int slang_text_expmem(struct slang_text *);
static char *slang_text_get(struct slang_text *);
#ifndef SLANG_NOTYPES
static int slang_text_strcasecmp(struct slang_text *, char *);
#endif

static struct slang_text *slang_text_parse(char *text)
{
  char *cmdstart, *cmdend;
  struct slang_text *firstitem, *item;

  firstitem = slang_text_create(NULL);
  item = firstitem;
  while ((cmdstart = strstr(text, "<?"))) {
    cmdstart[0] = 0;
    slang_text_add_string(item, text);
    item = slang_text_create(item);
    text += 2;
    cmdstart += 2;
    cmdend = strstr(cmdstart, "/?>");
    if (!cmdend) {
      putlog(LOG_MISC, "*", "ERROR parsing slang text: unterminated command \"%s\"!", cmdstart);
      break;
    }
    cmdend[0] = 0;
    slang_text_add_command(item, cmdstart);
    item = slang_text_create(item);
    text = cmdend + 3;
  }
  slang_text_add_string(item, text);
  return firstitem;
}

static struct slang_text *slang_text_create(struct slang_text *where)
{
  struct slang_text *newpart;

  newpart = nmalloc(sizeof(struct slang_text));
  newpart->next = NULL;
  newpart->string = NULL;
  newpart->command = NULL;
  while (where && where->next)
    where = where->next;
  if (where)
    where->next = newpart;
  return newpart;
}

static void slang_text_add_string(struct slang_text *item, char *s)
{
  Assert(item);
  Assert(!item->string);
  item->string = nmalloc(strlen(s) + 1);
  strcpy(item->string, s);
}

static void slang_text_free(struct slang_text *item)
{
  if (!item)
    return;
  slang_text_free(item->next);
  if (item->string)
    nfree(item->string);
  nfree(item);
}

/*static int slang_text_expmem(struct slang_text *item)
{
  int size = 0;

  while (item) {
    size += sizeof(struct slang_text);
    if (item->string)
      size += strlen(item->string) + 1;
    item = item->next;
  }
  return size;
}*/

#ifndef SLANG_NOTYPES
static int slang_text_strcasecmp(struct slang_text *item, char *text)
{
  Assert(item);
  debug2("s_t_sc: '%s', '%s'", text, item->string);
  if (item->command || item->next)
    return 1;
  return strcasecmp(item->string, text);
}
#endif

static char slang_text_buf[500];
static char *slang_text_get(struct slang_text *item)
{
  slang_text_buf[0] = 0;
  while (item) {
    if (item->string)
      strncat(slang_text_buf, item->string, sizeof(slang_text_buf));
    else if (item->command)
      item->command();
    item = item->next;
  }
  return slang_text_buf;
}

/*****************************************************/


static struct slang_command_list *glob_slang_cmd_list;

static struct slang_command_list *slang_commands_list_add(struct slang_command_list *where, struct slang_text_commands *what)
{
  struct slang_command_list *newcommandlist;

  newcommandlist = nmalloc(sizeof(struct slang_command_list));
  newcommandlist->commands = what;
  newcommandlist->next = where;
  return newcommandlist;
}

/*
static int slang_commands_list_expmem(struct slang_command_list *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct slang_command_list);
    what = what->next;
  }
  return size;
}
*/

static void slang_commands_list_free(struct slang_command_list *what)
{
  struct slang_command_list *next;

  while (what) {
    next = what->next;
    nfree(what);
    what = next;
  }
}

static void slang_text_add_command(struct slang_text *item, char *s)
{
  struct slang_command_list *cmdlist;
  char *cmd;
  int i;

  cmd = newsplit(&s);
  i = 0;
  for (cmdlist = glob_slang_cmd_list; cmdlist; cmdlist = cmdlist->next) {
    for (i = 0; 1; i++) {
      if (!cmdlist->commands[i].command)
        break;
      if (!strcasecmp(cmdlist->commands[i].command, cmd)) {
        item->command = cmdlist->commands[i].targetfunc;
        return;
      }
    }
  }
  putlog(LOG_MISC, "*", "ERROR! Unknown slang-command: '%s'", cmd);
}
