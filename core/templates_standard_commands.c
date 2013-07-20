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

static void template_add_subcontent(struct template_content *content,
                                    struct llist_2string *params,
                                    char *included_text)
{
  Assert(!content->subcontent);
  if (included_text)
    content->subcontent = templates_content_parse(included_text);
}

/* template_send_module_version():
 * sends the module version (surprise!)
 */
static void template_send_module_version(int idx, struct template_content *tpc)
{
  dprintf(idx, "%s", MODULE_VERSION);
}

/* <?slang #?>
 * outputs a line from the slangfile
 */
static void template_send_slang(int idx, struct template_content *tpc)
{
  dprintf(idx, "%s", getslang(tpc->intpar1));
}

static void template_add_cmd_slang(struct template_content *content,
                                   struct llist_2string *params,
                                   char *included_text)
{
  for (; params; params = params->next)
    if (!strcasecmp(params->s1, "id"))
      content->intpar1 = atoi(params->s2);
}

static void template_add_cmd_template(struct template_content *content,
                                   struct llist_2string *params,
                                   char *included_text)
{
  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "name")) {
      content->charpar1 = nmalloc(strlen(params->s2) + 1);
      strcpy(content->charpar1, params->s2);
      return;
    }
  }
  putlog(LOG_MISC, "*", "ERROR: missing parameter for template tag!");
}

static void template_send_template(int idx, struct template_content *tpc)
{
  if (tpc->charpar1)
    template_send(glob_skin, tpc->charpar1, idx);
}

static void template_send_form_method(int idx, struct template_content *tpc)
{
  if (get_param_value(idx, "dontpost"))
    dprintf(idx, "GET");
  else
    dprintf(idx, "POST");
}

static void template_send_if_dontpost(int idx, struct template_content *tpc)
{
  if (get_param_value(idx, "dontpost"))
    templates_content_send(tpc->subcontent, idx);
}

static void template_send_botnick(int idx, struct template_content *tpc)
{
  dprintf(idx, "%s", botname);
}

struct template_commands templates_standard_commands[] =
{
  {"module_version", template_send_module_version, NULL},
  {"slang", template_send_slang, template_add_cmd_slang},
  {"template", template_send_template, template_add_cmd_template},
  {"form_method", template_send_form_method, NULL},
  {"if_dontpost", template_send_if_dontpost, template_add_subcontent},
  {"botnick", template_send_botnick, NULL},
  {0, 0, 0},
};
