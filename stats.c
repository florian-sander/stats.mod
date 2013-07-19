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
#define MODULE_VERSION "1.3.3dev3"
#include "../module.h"
#include "../irc.mod/irc.h"
#include "../server.mod/server.h"
#include "../channels.mod/channels.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h> /* for time_t */
#include <ctype.h>
#include <sys/types.h>
#include <fcntl.h>

#undef global
static Function *global = NULL, *irc_funcs = NULL, *server_funcs = NULL, *channels_funcs = NULL;

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

#include "stats.h"
#include "slang.h"

static struct stats_global *sdata = NULL;
static struct stats_chanset *schans = NULL;
static struct stats_userlist *suserlist = NULL;
static struct slang_lang *slangs = NULL;
static struct slang_chan *slangchans = NULL;

static char statsfile[121] = "statsmod.dat";
static char webstats[510] = "words letters lines actions smileys joins kicks modes topics minutes";
static char graphstats[510] = "words letters lines actions smileys joins kicks modes topics minutes";
static char stat_reply[128] = "words letters smileys minutes";
static char graphgif[128] = "";
static char graphcolor[20] = "blue";
static char webdir[256] = "../public_html";
static char webfiles[256] = "";
static char webfile_suffix[20] = ".html";
static char bodytag[512] = "<BODY BGCOLOR=#000000 TEXT=#1A9DFF LINK=#00D993 VLINK=#71C2FF ALINK=#2FFFBB>";
static char smileys[128] = ":-) :) ;) ;-) ^_^ :-D :-P :P =) ;D";
static char badflags[20] = "ofvb|ofv";
static char nostatsflags[20] = "b|-";
static char nopeak[20] = "b|-";
static char network[41] = "unknown-net";
static char stats_loglevel[20] = "1";
static char livestats_log[121];
static char livestats_ignore_msg[256] = "<H1>You are on ignore.</H1>";
static char livestats_ip[21] = "";
static int statsfilemode = 0600;
static int webupdate = 0;
static int webnr = 15;
static int graphnr = 15;
static int stats_save_time = 10;
static int autoadd = 5;
static int stat_expire_user = 0;
static int write_today = 1;
static int maxstat_thr = 0;
static int maxstat_time = 0;
static int maxlivestats_thr = 0;
static int maxlivestats_time = 0;
static int mstat_thr = 0;
static time_t mstat_time = 0;
static int livestats_timeout;
static int offset = 0;
static int max_words = 20;
static int topwords_limit = 5;
static int quote_freq = 10;
static int log_wordstats = 0;
static int lasthour = 0;
static int lastmonth = 0;
static int min_word_length = 0;
static int use_userfile = 0;
static int table_color = 0x3850B8;
static int fade_table_to = 0x000000;
static int table_border = 0;
static int log_urls = 1;
static int kick_context = 0;
static int display_kicks = 0;
static int display_average_users = 1;
static int show_userlist = 1;
static int show_usersonchan = 1;
static int list_secret_chans = 1;

#include "datahandling.c"
#include "slang.c"
#include "sensors.c"
#include "userrec.c"
#include "misc.c"
#include "pubcmds.c"
#include "msgcmds.c"
#include "dcccmds.c"
#include "tclstats.c"
#if EGG_IS_MIN_VER(10500)
#include "webfiles.c"
#endif
#include "user.c"
#include "livestats.c"

static int stats_save_temp = 1;
static int webupdate_temp = 1;

static int stats_expmem()
{
  int size = 0;
  struct stats_global *sl = sdata;

  Context;
  while (sl) {
    size += sizeof(struct stats_global);
    size += strlen(sl->chan) + 1;
    size += localstats_expmem(sl->local);
    size += wordstats_expmem(sl->words);
    size += topics_expmem(sl->topics);
    size += urls_expmem(sl->urls);
    size += hosts_expmem(sl->hosts);
    size += quotes_expmem(sl->log);
    size += kicks_expmem(sl->kicks);
    sl = sl->next;
  }
  Context;
  size += suserlist_expmem(suserlist);
  size += chanlist_expmem(schans);
  size += slang_expmem();
  return size;
}

static int suserlist_expmem(struct stats_userlist *e)
{
  int size = 0;

  Context;
  while (e) {
    size += sizeof(struct stats_userlist);
    size += strlen(e->user) + 1;
    if (e->email)
      size += strlen(e->email) + 1;
    if (e->homepage)
      size += strlen(e->homepage) + 1;
    size += hostlist_expmem(e->hosts);
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

static int chanlist_expmem(struct stats_chanset *e)
{
  int size = 0;

  Context;
  while (e) {
    size += sizeof(struct stats_chanset);
    size += strlen(e->chan) + 1;
    size += memberlist_expmem(e->members);
    e = e->next;
  }
  return size;
}

static int memberlist_expmem(struct stats_memberlist *e)
{
  int size = 0;

  Context;
  while (e) {
    size += sizeof(struct stats_memberlist);
    size += strlen(e->nick) + 1;
    size += strlen(e->uhost) + 1;
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

static void free_stats()
{
  struct stats_global *sl;

  Context;
  while (sdata) {
    sl = sdata->next;
    free_localstats(sdata->local);
    free_wordstats(sdata->words);
    free_topics(sdata->topics);
    free_urls(sdata->urls);
    free_quotes(sdata->log);
    free_hosts(sdata->hosts);
    free_kicks(sdata->kicks);
    nfree(sdata->chan);
    nfree(sdata);
    sdata = sl;
  }
  free_chanlist(schans);
  free_suserlist(suserlist);
  free_slang();
  Context;
  return;
}

static void free_suserlist(struct stats_userlist *e)
{
  struct stats_userlist *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->user);
    if (e->email)
      nfree(e->email);
    if (e->homepage)
      nfree(e->homepage);
    free_hostlist(e->hosts);
    nfree(e);
    e = ee;
  }
}

static void free_hostlist(struct stats_hostlist *e)
{
  struct stats_hostlist *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->mask);
    nfree(e);
    e = ee;
  }
}

static void free_chanlist(struct stats_chanset *e)
{
  struct stats_chanset *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->chan);
    free_memberlist(e->members);
    nfree(e);
    e = ee;
  }
}

static void free_one_chan(char *channel)
{
  struct stats_chanset *e, *ee;

  Context;
  e = schans;
  ee = NULL;
  while (e) {
    if (!rfc_casecmp(e->chan, channel)) {
      nfree(e->chan);
      free_memberlist(e->members);
      if (ee)
	ee->next = e->next;
      else
        schans = e->next;
      nfree(e);
      return;
    }
    ee = e;
    e = e->next;
  }
}

static void free_memberlist(struct stats_memberlist *e)
{
  struct stats_memberlist *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->nick);
    nfree(e->uhost);
    nfree(e);
    e = ee;
  }
}

static void free_localstats(struct stats_local *sl)
{
  struct stats_local *sll;

  Context;
  while (sl) {
    Context;
    sll = sl->next;
    free_wordstats(sl->words);
    free_quotes(sl->quotes);
    nfree(sl->user);
    nfree(sl);
    sl = sll;
    Context;
  }
  Context;
  return;
}

static void free_wordstats(wordstats *l)
{
  wordstats *ll;

  Context;
  while (l) {
    ll = l->next;
    nfree(l->word);
    nfree(l);
    l = ll;
  }
  return;
}

static void free_quotes(quotestr *l)
{
  quotestr *ll;

  Context;
  while (l) {
    ll = l->next;
    nfree(l->quote);
    nfree(l);
    l = ll;
  }
  return;
}

static void free_topics(topicstr *e)
{
  topicstr *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->topic);
    nfree(e->by);
    nfree(e);
    e = ee;
  }
  return;
}

static void free_urls(struct stats_url *e)
{
  struct stats_url *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->url);
    nfree(e->by);
    nfree(e);
    e = ee;
  }
  return;
}

static void free_kicks(struct stats_kick *e)
{
  struct stats_kick *ee;

  Context;
  while (e) {
    ee = e->next;
    free_quotes(e->log);
    nfree(e);
    e = ee;
  }
}

static void free_hosts(hoststr *e)
{
  hoststr *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->host);
    nfree(e);
    e = ee;
  }
  return;
}

/* a report on the module status */
static void stats_report(int idx, int details)
{
  int size, users, hosts;

  Context;
  size = stats_expmem();
  users = countsusers();
  hosts = counthosts();
  dprintf(idx, "    Stats-userbase contains %d users and %d hosts\n", users, hosts);
  if (details)
    dprintf(idx, "    using %d bytes\n", size);
}

static void stats_minutely ()
{
  sensor_minutely();
  if (stats_save_temp >= stats_save_time) {
    write_stats();
    stats_save_temp = 1;
  } else
    stats_save_temp++;
  if ((webupdate_temp >= webupdate) && (webupdate > 0)) {
    webupdate_temp = 1;
#if EGG_IS_MIN_VER(10500)
    write_new_webstats();
#else
    putlog(LOG_MISC, "*", "Couldn't write static webstats! Feature unsupported on eggdrop1.4!");
#endif
  } else
    webupdate_temp++;
  check_desynch();
}

static void stats_5minutely ()
{
  Context;
  sensor_countusers();
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
  {"webupdate", &webupdate, 0},
  {"webnr", &webnr, 0},
  {"topnr", &webnr, 0},
  {"graphnr", &graphnr, 0},
  {"autoadd", &autoadd, 0},
  {"expire-users", &stat_expire_user, 0},
  {"write-today", &write_today, 0},
  {"stats-file-mode", &statsfilemode, 0},
  {"livestats-timeout", &livestats_timeout, 0},
  {"offset", &offset, 0},
  {"max-words", &max_words, 0},
  {"topwords-limit", &topwords_limit, 0},
  {"quote-frequency", &quote_freq, 0},
  {"log-wordstats", &log_wordstats, 0},
  {"min-word-length", &min_word_length, 0},
  {"use-eggdrop-userfile", &use_userfile, 0},
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
  {0, 0, 0}
};

static tcl_strings my_tcl_strings[] =
{
  {"statsfile", statsfile, 121, 0},
  {"webstats", webstats, 510, 0},
  {"topstats", webstats, 510, 0},
  {"webdir", webdir, 256, 0},
  {"webfiles", webfiles, 256, 0},
  {"webfile-suffix", webfile_suffix, 20, 0},
  {"bodytag", bodytag, 512, 0},
  {"smileys", smileys, 128, 0},
  {"smilies", smileys, 128, 0},
  {"graphstats", graphstats, 510, 0},
  {"graphgif", graphgif, 128, 0},
  {"graphcolor", graphcolor, 20, 0},
  {"anti-autoadd-flags", badflags, 20, 0},
  {"anti-stats-flag", nostatsflags, 20, 0},
  {"anti-peak-flag", nopeak, 20, 0},
  {"network", network, 40, 0},
  {"livestats-loglevel", stats_loglevel, 20, 0},
  {"livestats-log", livestats_log, 121, 0},
  {"stat-reply", stat_reply, 128, 0},
  {"livestats-ignore-msg", livestats_ignore_msg, 128, 0},
  {"livestats-ip", livestats_ip, 20, 0},
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
  stop_listen_livestats();
  write_stats();
  free_stats();
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
  del_hook(HOOK_5MINUTELY, (Function) stats_5minutely);
  module_undepend(MODULE_NAME);
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
  sdata = NULL;
  schans = NULL;
  suserlist = NULL;
  slangs = NULL;
  slangchans = NULL;
  Context;
  module_register(MODULE_NAME, stats_table, 1, 3);
  if (!(irc_funcs = module_depend(MODULE_NAME, "irc", 1, 0)))
    return "You need the irc module to use the stats module.";
  if (!(server_funcs = module_depend(MODULE_NAME, "server", 1, 0)))
    return "You need the server module to use the stats module.";
  if (!(channels_funcs = module_depend(MODULE_NAME, "channels", 1, 0)))
    return "You need the channels module to use the stats module.";
  if (!module_depend(MODULE_NAME, "eggdrop", 107, 0)) {
    if (!module_depend(MODULE_NAME, "eggdrop", 106, 0)) {
      if (!module_depend(MODULE_NAME, "eggdrop", 105, 0)) {
        if (!module_depend(MODULE_NAME, "eggdrop", 104, 0)) {
          module_undepend(MODULE_NAME);
          return "Sorry, stats.mod doesn't work with this eggdrop version.";
        }
      }
    }
  }
#ifndef OLDBOT
  livestats_timeout = 10;
#else
  livestats_timeout = 1;
#endif
  livestats_log[0] = 0;
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
  add_hook(HOOK_5MINUTELY, (Function) stats_5minutely);
  add_help_reference("stats.help");
  lastmonth = getmonth();
  read_stats();
#if EGG_IS_MIN_VER(10503)
  initudef(1, "nopubstats", 1);
  initudef(1, "quietstats", 1);
  initudef(1, "nostats", 1);
#endif
  putlog(LOG_MISC, "*", "Stats.mod v%s loaded.", MODULE_VERSION);
  return NULL;
}
