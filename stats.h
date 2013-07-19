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

#define SAVESTATSLENGTH 5000
#define MAXSLANGLENGTH 2000


#define TYPES "words letters started minutes topics lines actions modes bans kicks nicks joins smileys questions"
#define T_GSTARTED -1
#define T_LSTARTED -2
#define T_PEAK -3
#define T_WPL -4
#define T_WORD -5
#define T_VOCABLES -6
#define T_QUOTE -7
#define T_IDLE -8
#define T_ERROR -999
#define T_WORDS 0
#define T_LETTERS 1
#define T_MINUTES 2
#define T_TOPICS 3
#define T_LINES 4
#define T_ACTIONS 5
#define T_MODES 6
#define T_BANS 7
#define T_KICKS 8
#define T_NICKS 9
#define T_JOINS 10
#define T_SMILEYS 11
#define T_QUESTIONS 12
#define TOTAL_TYPES 13
#define TOTAL_SPECIAL_TYPES 8

#define S_TOTAL 0
#define S_TODAY 1
#define S_DAILY 1
#define S_WEEKLY 2
#define S_MONTHLY 3

#define RANGESTR today ? ((today == S_DAILY) ? SLDAILY : ((today == S_WEEKLY) ? SLWEEKLY : SLMONTHLY)) : SLTOTAL
#define RANGESTR_LONG today ? ((today == S_DAILY) ? SLLTODAY : ((today == S_WEEKLY) ? SLLWEEKLY : SLLMONTHLY)) : SLLTOTAL
#define ISLINK(x) x ? "" : "!"
#define ISTEXT(x, y) x ? y : "&nbsp;"

#define S_USERSUM 0
#define S_USERCOUNTS 1

#define SL_PRIVMSG 0
#define SL_KICK 1
#define SL_MODE 2
#define SL_NICK 3
#define SL_PART 4
#define SL_QUIT 5
#define SL_JOIN 6


typedef struct stats_hosts {
  struct stats_hosts *next;
  char *host;
  int nr;
} hoststr;

typedef struct stats_words {
  struct stats_words *next;
  struct stats_words *left;
  struct stats_words *right;
  char *word;
  int nr;
} wordstats;

typedef struct stats_quote {
  struct stats_quote *next;
  char *quote;
} quotestr;

typedef struct stats_topic {
  struct stats_topic *next;
  char *topic;
  char *by;
  time_t when;
} topicstr;

struct stats_url {
  struct stats_url *next;
  char *url;
  char *by;
  int shown;
  time_t when;
};

struct stats_kick {
  struct stats_kick *next;
  quotestr *log;
  int shown;
};

typedef struct stats_local {
  struct stats_local *next;
  struct stats_local *snext[4][TOTAL_TYPES + TOTAL_SPECIAL_TYPES];
  char *user;
  struct stats_userlist *u;
  time_t started;
  long int values[4][TOTAL_TYPES];
  wordstats *words;
  wordstats *tree;
  wordstats *word;
  int vocables;
  quotestr *quotes;
  int quotefr;
  int flag;
} locstats;

typedef struct stats_global {
  struct stats_global *next;
  char *chan;
  time_t started;
  int peak[4];
  int users[2][24];
  struct stats_local *local;
  struct stats_local *slocal[4][TOTAL_TYPES + TOTAL_SPECIAL_TYPES];
  wordstats *words;
  topicstr *topics;
  hoststr *hosts;
  struct stats_url *urls;
  quotestr *log;
  quotestr *lastlog;
  int log_length;
  struct stats_kick *kicks;
} globstats;

struct stats_clientinfo {
  int traffic;
  int code;
  char *browser;
  char *referer;
  char *cmd;
};

#define stats_info_access(i) ((struct stats_clientinfo *) dcc[(i)].u.other)

static void sortstats(struct stats_global *, int, int);
static void countvocables(globstats *);
static int localstats_expmem(struct stats_local *);
static int wordstats_expmem(wordstats *);
static void free_stats();
static void free_localstats(struct stats_local *);
static void free_wordstats(wordstats *);
static void reset_tstats();
static void free_urls(struct stats_url *);
static void tell_top(char *, char *, char *, char *, int, int, int);
static void tell_place(char *, char *, char *, char *, char *, int);
static void tell_stat(char *, char *, char *, char *, char *, int);
static int typetoi(char *);
static int slangtypetoi(char *);
static void incrstats(char *, char *, int, int, int);
static int getstats(char *, char *, char *, int);
static locstats *findlocstats(char *, char *);
static globstats *findglobstats(char *);
static int countwords(char *);
static void write_stats();
static void read_stats();
static int countsmileys(char *);
static int countstatmembers(globstats *);
static int gettotal(globstats *, int, int);
static void stats_autoadd(memberlist *, char *);
static int matchattr(struct userrec *, char *, char *);
static void deloldstatusers();
static void purgestats();
static void sensor_minutely();
static int validchan(char *);
static void start_listen_livestats(int);
static void stop_listen_livestats();
static void display_livestats_accept(int, char *);
static void timeout_livestats(int);
#ifndef OLDBOT
static void outdone_livestats(int);
#endif
static void timeout_listen_livestats(int);
static void livestats_accept(int, char *, int);
static void eof_livestats(int);
static void display_livestats(int, char *);
static void livestats_activity(int, char *, int);
static void send_livestats(int, char *);
static char *splitpath(char **);
static void sort_stats_alphabetically(globstats *);
static void sensor_peak(char *);
static int stat_flood();
static void resetlocstats(locstats *);
static int expmem_livestats(void *);
static void kill_livestats(int, void *);
static void out_livestats(int, char *, void *);
static void calcwordstats(char *, globstats *, char *, locstats *ls);
static void incrwordstats(locstats *, char *, int, int);
static void strlower(char *);
static void sortwordstats(locstats *, globstats *);
static void filt(char *);
static char *filt2(char *);
static char *filtbrackets(char *);
static void tell_wordstats(char *, char *, char *, char *, char *);
static void tell_topwords(char *, char *, char *, char *);
static void nincrwordstats(globstats *, char *, int);
static void addquote(char *, globstats *, char *, locstats *);
static void free_quotes(quotestr *);
static int quotes_expmem(quotestr *);
static void do_globwordstats(globstats *);
static int topics_expmem(topicstr *);
static int urls_expmem(struct stats_url *);
static int hosts_expmem(hoststr *);
static void free_topics(topicstr *);
static void free_hosts(hoststr *);
static void addtopic(char *, char *, char *);
static void do_graphs(int, int, globstats *, char *);
static void do_toptalkers(int, int, globstats *, char *, char *);
static void do_miscstats(int, char *);
static int countquestions(char *);
static int inactivechan(char *);
static int gethour();
static int getmonth();
static void sensor_countusers();
static void reset_mwstats(int);
static int secretchan(char *);
static int nostats(char *);
static void sorthosts(struct stats_global *);
static void addhost(char *, globstats *);
static void tell_top_word(char *, char *, char *, char *, int, int, globstats *);
static int quietstats(char *);
static int track_stat_user(char *, char *);
static void check_for_url(char *, char *, char *);
static void free_kicks(struct stats_kick *);
static int kicks_expmem(struct stats_kick *);
#if EGG_IS_MIN_VER(10500)
static void write_new_webstats();
void stats_setsock(int, int);
#endif
static char *stats_duration(int);
static int livestats_flood();
static void display_users_on_chan(int, char *, struct chanset_t *);

/* eggdrop-userfile-independent user management */
struct stats_hostlist {
  struct stats_hostlist *next;
  char *mask;
  time_t lastused;
};

struct stats_userlist {
  struct stats_userlist *next;
  char *user;
  int list;
  int addhosts;
  struct stats_hostlist *hosts;
  char *email;
  char *homepage;
};

struct stats_memberlist {
  struct stats_memberlist *next;
  char *nick;
  char *uhost;
  time_t joined;
  struct stats_userlist *user;
  locstats *stats;
};

struct stats_chanset {
  struct stats_chanset *next;
  char *chan;
  struct stats_memberlist *members;
};

static int suserlist_expmem(struct stats_userlist *);
static int hostlist_expmem(struct stats_hostlist *);
static int chanlist_expmem(struct stats_chanset *);
static int memberlist_expmem(struct stats_memberlist *);
static void free_suserlist(struct stats_userlist *);
static void free_hostlist(struct stats_hostlist *);
static void free_chanlist(struct stats_chanset *);
static void free_memberlist(struct stats_memberlist *);
static void free_one_chan(char *);

static struct stats_memberlist *nick2suser(char *, char *);
static struct stats_chanset *findschan(char *);
static struct stats_chanset *initchan(char *);
static void saddmember(char *, char *, char *, char *);
static void strackmember(char *, char *, char *);
static void skillmember(char *, char *);
static struct stats_userlist *findsuser(char *);
static struct stats_userlist *findsuser_by_name(char *);
static struct stats_userlist *addsuser(char *, int, int);
static void delsuser(char *user);
static void saddhost(struct stats_userlist *, char *, time_t);
static int sdelhost(struct stats_userlist *, char *);
static void stats_autosadd(struct stats_memberlist *, struct stats_chanset *);
static void nincrstats(locstats *, int, int);
static locstats *initstats(char *, char *);
static int listsuser(locstats *, char *);
static int countsusers();
static int counthosts();
static void weed_userlink_from_chanset(struct stats_userlist *);
static void weed_statlink_from_chanset(locstats *);
static void weed_userlink_from_locstats(struct stats_userlist *);
static int countmembers(struct stats_memberlist *);
static void check_desynch();
static void update_schannel_members();
static void dump_suser(int, struct stats_userlist *);
static void setemail(struct stats_userlist *, char *);
static void sethomepage(struct stats_userlist *, char *);
static void long_dprintf(int, char *);

/* language system */

struct slang_texts {
  struct slang_texts *next;
  char *text;
  int dynamic;
};

struct slang_ids {
  struct slang_ids *next;
  int id;
  int entries;
  struct slang_texts *texts;
};

struct slang_types {
  struct slang_types *next;
  char *type;
  int entries;
  struct slang_texts *texts;
};

struct slang_lang {
  struct slang_lang *next;
  char *lang;
  struct slang_ids *ids;
  struct slang_types *types;
  struct slang_bntypes *bignumbers;
  char *durs[10];
};

struct slang_chan {
  struct slang_chan *next;
  char *chan;
  char *lang;
};

struct slang_bnplaces {
  struct slang_bnplaces *next;
  int place;
  int entries;
  struct slang_texts *texts;
};

struct slang_bntypes {
  struct slang_bntypes *next;
  char *type;
  struct slang_bnplaces *places;
};

static int slang_expmem();
static void free_slang();
static int loadslang(char *, char *);
static void addslangitem(struct slang_lang *, int, char *);
static void addslangtype(struct slang_lang *, char *);
static void addslangbn(struct slang_lang *, char *);
static char *getslang(int idnr);
static char *getslangtype(char *);
static char *dynamicslang(char *);
static int isdynamicslang(char *);
static char *dynamicslang(char *);
static char *chanlang(char *);
static void setchanlang(char *, char *);
static void setslglobs(char *, int, int, time_t);
static void setslnick(char *);
static char *getdur(int);

static void sortstats_wpl(struct stats_global *, int);
static void sortstats_vocables(struct stats_global *, int);
static void sortstats_word(struct stats_global *, int);
static void sortstats_idle(struct stats_global *, int);
static void maskstricthost(const char *, char *);
