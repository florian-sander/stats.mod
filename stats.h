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


#define MAXSLANGLENGTH 2000

// #define RANGESTR today ? ((today == S_DAILY) ? SLDAILY : ((today == S_WEEKLY) ? SLWEEKLY : SLMONTHLY)) : SLTOTAL

#define ISLINK(x) x ? "" : "!"
#define ISTEXT(x, y) x ? y : "&nbsp;"






struct stats_clientinfo {
  int traffic;
  int code;
  char *browser;
  char *referer;
  char *cmd;
};

#define stats_info_access(i) ((struct stats_clientinfo *) dcc[(i)].u.other)

/*
static void countvocables(globstats *);
static int localstats_expmem(struct stats_local *);
static int wordstats_expmem(wordstats *);
static void free_stats();
static void free_localstats(struct stats_local *);
static void free_wordstats(wordstats *);
static void reset_tstats();
static void free_urls(struct stats_url *);
static char *tell_ntop(char *, char *, int);
static char *tell_place(char *, char *, char *, char *);
static char *tell_stat(char *, char *, char *);


static void incrstats(char *, char *, int, int, int);
static int getstats(char *, char *, char *, int);
static locstats *findlocstats(char *, char *);
static globstats *findglobstats(char *);
static int countwords(char *);
static void write_stats();
static void read_stats();
static int countsmileys(char *);
static int countstatmembers(globstats *);
static int countallstatmembers(globstats *);
static int countactivestatmembers(globstats *, int, int, int, int);
static int gettotal(globstats *, int, int);
static int matchattr(struct userrec *, char *, char *);
static void deloldstatusers();
static void purgestats();
static void sensor_minutely();
static int validchan(char *);

static int stat_flood();
static void resetlocstats(locstats *);
static void calcwordstats(locstats *stats, char *text);

static void strlower(char *);

static void tell_wordstats(char *, char *, char *, char *, char *);
static void tell_topwords(char *, char *, char *, char *);
static void nincrwordstats(globstats *, char *, int);
static void addquote(locstats *stats, char *quote);
static int stats_globstats_expmem(struct stats_global *);
static void free_quotes(quotestr *);
static int quotes_expmem(quotestr *);
static void do_globwordstats(globstats *);
static int topics_expmem(topicstr *);
static int urls_expmem(struct stats_url *);
static int hosts_expmem(hoststr *);
static void free_topics(topicstr *);
static void free_hosts(hoststr *);
static void addtopic(char *, char *, char *);
static int countquestions(char *);
static int inactivechan(char *);
static int gethour();
static int getmonth();
static void reset_mwstats(int);
static int secretchan(char *);
static int nostats(char *);
static void sorthosts(struct stats_global *);
static void addhost(char *, globstats *);
static char *tell_top_word(char *, char *, int, globstats *);
static int quietstats(char *);
static int track_stat_user(char *, char *);
static void check_for_url(char *, char *, char *);
static void free_kicks(struct stats_kick *);
static int kicks_expmem(struct stats_kick *);
static char *stats_duration(int, int);
*/

/* eggdrop-userfile-independent user management */

static int suserlist_expmem(struct stats_userlist *);
static int hostlist_expmem(struct stats_hostlist *);
static void free_suserlist(struct stats_userlist *);
static void free_hostlist(struct stats_hostlist *);
//static void free_one_chan(char *);


static struct stats_userlist *stats_userlist_create_entry(char *);
static void stats_userlist_free_entry(struct stats_userlist *);
static int stats_userlist_expmem_entry(struct stats_userlist *);
static void delsuser(char *);
static void saddhost(struct stats_userlist *, char *, time_t, time_t);
static int sdelhost(struct stats_userlist *, char *);
static void welcome_suser(char *, struct stats_userlist *, char *);
static void nincrstats(locstats *, int, int);
static locstats *initstats(char *, char *);
static int listsuser(locstats *, char *);
static int countsusers();
static int counthosts();
static void weed_userlink_from_chanset(struct stats_userlist *);
static void weed_statlink_from_chanset(locstats *);
static void weed_userlink_from_locstats(struct stats_userlist *);
static void dump_suser(int, struct stats_userlist *);
static void setemail(struct stats_userlist *, char *);
static void sethomepage(struct stats_userlist *, char *);
static void setpassword(struct stats_userlist *, char *);

static void maskstricthost(const char *, char *);

static int getplace(globstats *, int, int, char *);

static char *inverted_csplit(char **, char);
static int get_timerange(char *);

static time_t get_laston_time_from_hosts(char *user);
static time_t get_creation_time_from_locstats(char *user);

static int secretchan(char *chan);