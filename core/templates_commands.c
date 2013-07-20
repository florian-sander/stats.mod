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

struct template_command_list *glob_tpl_cmd_list;

static struct template_command_list *templates_commands_list_add(struct template_command_list *, struct template_commands *);
//static int templates_commands_list_expmem(struct template_command_list *);
static void templates_commands_list_free(struct template_command_list *);
static struct template_content *templates_commands_addtocontent(struct template_content *, char *, struct llist_2string *, char *);

static struct template_command_list *templates_commands_list_add(struct template_command_list *where, struct template_commands *what)
{
  struct template_command_list *newcommandlist;

  newcommandlist = nmalloc(sizeof(struct template_command_list));
  newcommandlist->commands = what;
  newcommandlist->next = where;
  return newcommandlist;
}

/*
static int templates_commands_list_expmem(struct template_command_list *what)
{
  int size = 0;

  while (what) {
    size += sizeof(struct template_command_list);
    what = what->next;
  }
  return size;
}*/

static void templates_commands_list_free(struct template_command_list *what)
{
  struct template_command_list *next;

  while (what) {
    next = what->next;
    nfree(what);
    what = next;
  }
}

static struct template_content *templates_commands_addtocontent(struct template_content *where,
                                            char *command,
                                            struct llist_2string *params,
                                            char *included_text)
{
  struct template_command_list *clist;
  struct template_commands *cmd;
  struct template_content *newcontent;
  int len, i;

  newcontent = templates_content_create();
  where = templates_content_append(where, newcontent);
  for (clist = glob_tpl_cmd_list; clist; clist = clist->next) {
    cmd = clist->commands;
    for (i = 0; 1; i++) {
      if (!cmd[i].command)
        break;
      if (!strcasecmp(command, cmd[i].command)) {
        newcontent->command = cmd[i].targetfunc;
        if (cmd[i].addfunc)
          cmd[i].addfunc(newcontent, params, included_text);
        return where;
      }
    }
  }
  len = strlen(command) + 34 + 1;
  newcontent->html = nmalloc(len);
  snprintf(newcontent->html, len, "<H1>Unknown Tag: &quot;%s&quot;</H1>", command);
  putlog(LOG_MISC, "*", "ERROR loading template: Unknown command '%s'!", command);
  return where;
}
