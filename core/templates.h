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

// template content struct. Stores the content (html and command pointers)
// of an template
struct template_content {
  struct template_content *next;
  char *html;
  void (*command) (int, struct template_content *);
  int what;
  char *charpar1;
  float floatpar1;
  float floatpar2;
  int intpar1;
  struct template_content *subcontent;
};

struct template_commands {
  char *command;
  void (*targetfunc) (int, struct template_content *);
  void (*addfunc) (struct template_content *header, struct llist_2string *params, char *included_text);
};

struct template_command_list {
  struct template_command_list *next;
  struct template_commands *commands;
};

// template header struct. Stores the name and a pointer to the content
// of an template.
struct templates_template {
  struct templates_template *next;
  char *name;
  struct template_content *contents;
};

// template skin struct
// contains the name of the skin and a pointer to the language-list
struct template_skin {
  struct template_skin *next;
  char *name;
  char *desc;
  struct slang_header *slang;
  struct templates_template *templates;
};

static struct templates_template *templates_template_add_parsedcontent(struct templates_template *, char *, struct template_content *);
//static int templates_template_expmem(struct templates_template *);
static void templates_template_free(struct templates_template *);

//static int templates_content_expmem(struct template_content *);
static void templates_content_free(struct template_content *);
static struct template_content *templates_content_load(char *);
static struct template_content *templates_content_create();
static struct template_content *templates_content_append(struct template_content *, struct template_content *);
static struct template_content *templates_content_parse(char *);
static struct template_content *templates_content_addhtml(struct template_content *, char *);
static void templates_content_send(struct template_content *, int);
static struct llist_2string *templates_content_parseparams(char *);

static void template_send(struct template_skin *, char *, int);
