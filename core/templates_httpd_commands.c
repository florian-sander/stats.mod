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

static void template_send_requested_url(int idx, struct template_content *tpc)
{
  dprintf(idx, "%s", http_connection(idx)->path);
}

static void template_send_server_version(int idx, struct template_content *tpc)
{
  dprintf(idx, "EggdropMiniHTTPd/%s", HTTPD_VERSION);
}

static void template_send_server_port(int idx, struct template_content *tpc)
{
  int i;

  for (i = 0; i < dcc_total; i++) {
    if (dcc[i].type == &MHTTPD_CON_HTTPD) {
      dprintf(idx, "%d", dcc[i].port);
      return;
    }
  }
}

struct template_commands template_httpd_commands[] =
{
  {"requested_url", template_send_requested_url, NULL},
  {"server_version", template_send_server_version, NULL},
  {"server_port", template_send_server_port, NULL},
  {0, 0, 0},
};
