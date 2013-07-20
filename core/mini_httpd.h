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

#define HTTPD_VERSION "1.1.0"

static void process_get_request(int);

struct http_connection_data {
  int traffic;
  int code;
  char *browser;
  char *referer;
  char *path;
  char *cmd;
  char *postparams;
  int getpostparams;
  int content_length;
  struct llist_2string *cookies;
  struct llist_2string *params;
  struct llist_1string *headers;
  struct llist_1string *langs;
};

static void init_httpd();
//static int expmem_httpd();
static void unload_httpd();
static void start_httpd(int);
static void stop_httpd();
static void init_http_connection_data(int);
static void free_http_connection_data(int);
static void http_activity(int, char *, int);
static void send_http_header(int, int);
static void add_cookies(int, char *);
static char *get_cookie_value(int, char *);
static void add_params(int, char *);
static char *get_param_value(int, char *);
#ifndef OLDBOT
static void outdone_http(int);
#endif
static void display_http(int, char *);
static void display_httpd_accept(int, char *);
static void timeout_http(int);
static void timeout_listen_httpd(int);
static void kill_http(int, void *);
static int expmem_http(void *);
static void out_http(int, char *, void *);
static void httpd_accept(int, char *, int);
static int http_flood();
static void eof_http(int);
static char *decode_url(char *);
static char *encode_url(char *);
static char *csplit(char **, char);
static void append_postparam_string(int, char *);
static void process_request(int);
static void add_language(int, char *);
static char *text2html(char *);
