/*
*   Statistics module for eggdrop 1.4+
*     by G`Quann
*/

/*
 * stats.c   - this is never gonna work... (Oct 99)
*/

/*
*  oops... seems that it works :) (Dec 99)
*/

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

#define MAKING_STATS
#define MODULE_NAME "stats"
#define MODULE_VERSION "1.5.1"
#ifndef NO_EGG
#include "../module.h"
#include "../irc.mod/irc.h"
#include "../server.mod/server.h"
#include "../channels.mod/channels.h"
#else
#include "compat/noegg.h"
#include "compat/noegg.c"
#endif
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h> /* for time_t */
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>

#undef global

#ifndef NO_EGG
static Function *global = NULL, *irc_funcs = NULL, *server_funcs = NULL, *channels_funcs = NULL;
#endif

#ifndef EGG_IS_MIN_VER
#define EGG_IS_MIN_VER(ver)             ((ver) <= 10400)
#endif

#if !EGG_IS_MIN_VER(10500)
#define OLDBOT 1
#endif

#ifndef Context
#define Context context
#endif

#ifndef findchan_by_dname
#define findchan_by_dname findchan
#endif

#ifndef my_htonl
#define my_htonl htonl
#endif


// #include "stats.h"

#ifndef NO_MEM_DEBUG
#ifndef DYNAMIC_MEM_DEBUG
#include "core/dynamic_mem_debug.c"
#endif
#endif



/* static struct slang_lang *slangs = NULL;
static struct slang_chan *slangchans = NULL;
 */

#include "egg_chancontrol.h"
#include "core/core.c"

#include "settings.c"

#include "misc.c"
#include "pubcmds.h"
#include "pubcmds.c"
#include "msgcmds.c"
#include "dcccmds.h"
#include "dcccmds.c"
#include "tclstats.c"

#include "egg_chancontrol.c"
#include "egg_bindings.c"

static int stats_save_temp = 1;

static int stats_expmem()
{
	return 0;
}

/*
static int stats_expmem()
{
  int size = 0;

  Context;
#ifdef DYNAMIC_MEM_DEBUG
  return 0;
#endif
  size += stats_globstats_expmem(sdata);
  Context;
  size += suserlist_expmem(suserlist);
  size += llist_expmem(&schanset);
  size += expmem_templates();
  size += expmem_httpd();
  size += expmem_global_vars();
  size += slang_glob_expmem();
  size += slang_expmem(coreslangs);
  size += slang_chanlang_expmem(chanlangs);
  if (stats_pubcmd_reply)
    size += strlen(stats_pubcmd_reply) + 1;
  return size;
}

static int stats_globstats_expmem(struct stats_global *gs)
{
  int size = 0;

  while (gs) {
    size += sizeof(struct stats_global);
    size += strlen(gs->chan) + 1;
    size += localstats_expmem(gs->local);
    size += wordstats_expmem(gs->words);
    size += topics_expmem(gs->topics);
    size += urls_expmem(gs->urls);
    size += hosts_expmem(gs->hosts);
    size += quotes_expmem(gs->log);
    size += kicks_expmem(gs->kicks);
    gs = gs->next;
  }
  return size;
}

static int suserlist_expmem(struct stats_userlist *e)
{
  int size = 0;

  Context;
  while (e) {
    size += stats_userlist_expmem_entry(e);
    e = e->next;
  }
  return size;
}

static int hostlist_expmem(struct stats_hostlist *e)
{
  int size = 0;

  Context;
  while (e) {
    size += sizeof(struct stats_hostlist);
    size += strlen(e->mask) + 1;
    e = e->next;
  }
  return size;
}

static int localstats_expmem(struct stats_local *sl)
{
  int size = 0;

  Context;
  while (sl) {
    size += sizeof(struct stats_local);
    size += strlen(sl->user) + 1;
    size += wordstats_expmem(sl->words);
    size += quotes_expmem(sl->quotes);
    sl = sl->next;
  }
  Context;
  return size;
}

static int wordstats_expmem(wordstats *l)
{
  int size = 0;

  Context;
  while (l) {
    size += strlen(l->word) + 1;
    size += sizeof(wordstats);
    l = l->next;
  }
  return size;
}

static int quotes_expmem(quotestr *l)
{
  int size = 0;

  Context;
  while (l) {
    size += strlen(l->quote) + 1;
    size += sizeof(quotestr);
    l = l->next;
  }
  return size;
}

static int topics_expmem(topicstr *e)
{
  int size = 0;

  Context;
  while (e) {
    size += strlen(e->topic) + 1;
    size += strlen(e->by) + 1;
    size += sizeof(topicstr);
    e = e->next;
  }
  return size;
}

static int urls_expmem(struct stats_url *e)
{
  int size = 0;

  Context;
  while (e) {
    size += strlen(e->url) + 1;
    size += strlen(e->by) + 1;
    size += sizeof(struct stats_url);
    e = e->next;
  }
  return size;
}

static int kicks_expmem(struct stats_kick *e)
{
  int size = 0;

  Context;
  while (e) {
    size += sizeof(struct stats_kick);
    size += quotes_expmem(e->log);
    e = e->next;
  }
  return size;
}

static int hosts_expmem(hoststr *e)
{
  int size = 0;

  Context;
  while (e) {
    size += strlen(e->host) + 1;
    size += sizeof(hoststr);
    e = e->next;
  }
  return size;
}
*/






/* a report on the module status */
static void stats_report(int idx, int details)
{
//  int memtotal, memusers, memchans, memtemplates, memslang, memdata;
  int users, hosts;

  Context;
  users = countsusers();
  hosts = counthosts();
  if (!details)
    dprintf(idx, "    Stats-userbase contains %d users and %d hosts\n", users, hosts);
  else {
/*    memtotal = stats_expmem();
    memdata = stats_globstats_expmem(sdata);
    memusers = suserlist_expmem(suserlist);
    memchans = llist_expmem(&schanset);
    memtemplates = expmem_templates();
    memslang = slang_glob_expmem() + slang_expmem(coreslangs) +
    		slang_chanlang_expmem(chanlangs);
    dprintf(idx, "    Stats-userbase contains %d users and %d hosts using "
		"%d bytes.\n", users, hosts, memusers);
    dprintf(idx, "    Statistic data itself using %d bytes.\n", memdata);
    dprintf(idx, "    Channels and channel members using %d bytes.\n", memchans);
    dprintf(idx, "    Templates using %d bytes.\n", memtemplates);
    dprintf(idx, "    Total memory usage: %d bytes.\n", memtotal); */
  }
}

static void stats_minutely ()
{
  eggbnd_minutely();
  if (stats_save_temp >= stats_save_time) {
    write_stats();
    stats_save_temp = 1;
  } else
    stats_save_temp++;
}

static void stats_daily ()
{
  Context;
  deloldstatusers();
  purgestats();
  reset_tstats();
  if (lastmonth != getmonth()) {
    reset_mwstats(S_MONTHLY);
    lastmonth = getmonth();
  }
  if (ismonday())
    reset_mwstats(S_WEEKLY);
  update_schannel_members();
  Context;
}

static tcl_ints my_tcl_ints[] =
{
  {"save-stats", &stats_save_time, 0},
  {"webnr", &webnr, 0},
  {"topnr", &webnr, 0},
  {"graphnr", &graphnr, 0},
  {"autoadd", &autoadd, 0},
  {"write-today", &write_today, 0},
  {"stats-file-mode", &statsfilemode, 0},
  {"livestats-timeout", &livestats_timeout, 0},
  {"offset", &offset, 0},
  {"topwords-limit", &topwords_limit, 0},
  {"quote-frequency", &quote_freq, 0},
  {"log-wordstats", &log_wordstats, 0},
  {"min-word-length", &min_word_length, 0},
  {"table-color", &table_color, 0},
  {"fade-table-to", &fade_table_to, 0},
  {"table-border", &table_border, 0},
  {"display-urls", &log_urls, 0},
  {"kick-context", &kick_context, 0},
  {"display-kicks", &display_kicks, 0},
  {"display-average-users", &display_average_users, 0},
  {"show-userlist", &show_userlist, 0},
  {"show-usersonchan", &show_usersonchan, 0},
  {"list-secret-chans", &list_secret_chans, 0},
  {"autoadd-min-lines", &autoadd_min_lines, 0},
  {"min-lines", &min_lines, 0},
  {"expire-base", &expire_base, 0},
  {"expire-factor", &expire_factor, 0},
  {0, 0, 0}
};

static tcl_strings my_tcl_strings[] =
{
  {"statsfile", statsfile, 121, 0},
  {"webstats", webstats, 510, 0},
  {"topstats", webstats, 510, 0},
  {"webdir", webdir, 256, 0},
  {"smileys", smileys, 128, 0},
  {"smilies", smileys, 128, 0},
  {"graphstats", graphstats, 510, 0},
  {"graphgif", graphgif, 128, 0},
  {"graphcolor", graphcolor, 20, 0},
  {"anti-autoadd-flags", badflags, 20, 0},
  {"anti-stats-flag", nostatsflags, 20, 0},
  {"anti-peak-flag", nopeak, 20, 0},
  {"network", network, 40, 0},
  {"livestats-loglevel", httpd_loglevel, 20, 0},
  {"livestats-log", httpd_log, 121, 0},
  {"stat-reply", stat_reply, 128, 0},
  {"livestats-ignore-msg", httpd_ignore_msg, 255, 0},
  {"livestats-ip", httpd_ip, 20, 0},
  {"default-slang", default_slang, 20, 0},
  {"default-skin", default_skin, 20, 0},
  {"binary-url", binary_url, 120, 0},
  {0, 0, 0, 0}
};

static tcl_coups my_tcl_coups[] =
{
  {"max-stat-cmds", &maxstat_thr, &maxstat_time},
  {"max-livestats-access", &maxlivestats_thr, &maxlivestats_time},
  {0, 0, 0},
};

static char *stats_close()
{
  Context;
  stats_core_unload();
  unload_httpd();
  unload_templates();
  write_stats();
  free_stats();
  free_global_vars();
  slang_chanlang_free(chanlangs);
  if (stats_pubcmd_reply)
    nfree(stats_pubcmd_reply);
  rem_builtins(H_dcc, mydcc);
  rem_builtins(H_pubm, stats_pubm);
  rem_builtins(H_pub, stats_pub);
  rem_builtins(H_msg, stats_msg);
  rem_builtins(H_topc, stats_topc);
  rem_builtins(H_ctcp, stats_ctcp);
  rem_builtins(H_kick, stats_kick);
  rem_builtins(H_mode, stats_mode);
  rem_builtins(H_nick, stats_nick);
  rem_builtins(H_join, stats_join);
  rem_builtins(H_nkch, stats_nkch);
  rem_builtins(H_sign, stats_sign);
  rem_builtins(H_part, stats_part);
  rem_tcl_ints(my_tcl_ints);
  rem_tcl_coups(my_tcl_coups);
  rem_tcl_strings(my_tcl_strings);
  rem_tcl_commands(mytcls);
  rem_help_reference("stats.help");
  del_hook(HOOK_MINUTELY, (Function) stats_minutely);
  del_hook(HOOK_DAILY, (Function) stats_daily);
  module_undepend(MODULE_NAME);
#ifndef NO_MEM_DEBUG
  dmd_unload();
#endif
  return NULL;
}

char *stats_start();

static Function stats_table[] =
{
  (Function) stats_start,
  (Function) stats_close,
  (Function) stats_expmem,
  (Function) stats_report,
};

char *stats_start(Function * global_funcs)
{
  global = global_funcs;
  Context;
#ifndef NO_MEM_DEBUG
  dmd_init();
#endif
  stats_core_init();
//  schans = NULL;
  suserlist = NULL;
//  slangs = NULL;
//  slangchans = NULL;
  stats_pubcmd_reply = NULL;

  chanlangs = NULL;
  slang_glob_init();
  Context;
  module_register(MODULE_NAME, stats_table, 1, 5);
  if (!(irc_funcs = module_depend(MODULE_NAME, "irc", 1, 0)))
    return "You need the irc module to use the stats module.";
  if (!(server_funcs = module_depend(MODULE_NAME, "server", 1, 0)))
    return "You need the server module to use the stats module.";
  if (!(channels_funcs = module_depend(MODULE_NAME, "channels", 1, 0)))
    return "You need the channels module to use the stats module.";
  if (!module_depend(MODULE_NAME, "eggdrop", 108, 0)) {
    module_undepend(MODULE_NAME);
    return "Sorry, stats.mod requires Eggdrop 1.8.0 or later.";
  }
#ifndef OLDBOT
  livestats_timeout = 10;
#else
  livestats_timeout = 1;
#endif
  add_builtins(H_dcc, mydcc);
  add_builtins(H_pubm, stats_pubm);
  add_builtins(H_pub, stats_pub);
  add_builtins(H_msg, stats_msg);
  add_builtins(H_topc, stats_topc);
  add_builtins(H_ctcp, stats_ctcp);
  add_builtins(H_kick, stats_kick);
  add_builtins(H_mode, stats_mode);
  add_builtins(H_nick, stats_nick);
  add_builtins(H_join, stats_join);
  add_builtins(H_nkch, stats_nkch);
  add_builtins(H_sign, stats_sign);
  add_builtins(H_part, stats_part);
  add_tcl_ints(my_tcl_ints);
  add_tcl_coups(my_tcl_coups);
  add_tcl_strings(my_tcl_strings);
  add_tcl_commands(mytcls);
  add_hook(HOOK_MINUTELY, (Function) stats_minutely);
  add_hook(HOOK_DAILY, (Function) stats_daily);
  add_help_reference("stats.help");
  lastmonth = getmonth();
  read_stats();
#if EGG_IS_MIN_VER(10503)
  initudef(1, "nopubstats", 1);
  initudef(1, "quietstats", 1);
  initudef(1, "nostats", 1);
#endif
  init_httpd();
  init_templates();
  init_global_vars();
  glob_tpl_cmd_list = templates_commands_list_add(glob_tpl_cmd_list, stats_template_commands);
  glob_tpl_cmd_list = templates_commands_list_add(glob_tpl_cmd_list, template_httpd_commands);
  glob_slang_cmd_list = slang_commands_list_add(glob_slang_cmd_list, slang_text_stats_command_table);
  putlog(LOG_MISC, "*", "Stats.mod v%s loaded.", MODULE_VERSION);
  return NULL;
}
