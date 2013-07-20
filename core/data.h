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

#define TYPES "words letters started minutes topics lines actions modes bans kicks nicks joins smileys questions"
#define SPECIAL_TYPES "age wpl vocables idle"
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

#define RANGESTR_LONG(x) x ? ((x == S_DAILY) ? SLLTODAY : ((x == S_WEEKLY) ? SLLWEEKLY : SLLMONTHLY)) : SLLTOTAL

#define S_USERSUM 0
#define S_USERCOUNTS 1

#define SL_PRIVMSG 0
#define SL_KICK 1
#define SL_MODE 2
#define SL_NICK 3
#define SL_PART 4
#define SL_QUIT 5
#define SL_JOIN 6


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

typedef struct stats_hosts {
  struct stats_hosts *next;
  char *host;
  int nr;
} hoststr;

typedef struct stats_local {
  struct stats_local *next;
  struct stats_local *snext[4][TOTAL_TYPES + TOTAL_SPECIAL_TYPES];
  char *user;
  struct stats_userlist *u;
  time_t started;
  time_t lastspoke;
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
  int activity[24];
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

static void locstats_init(locstats *);
static void globstats_init(globstats *);

static char *itotype(int);
static int typetoi(char *);

static void incrwordstats(locstats *, char *, int, int);

static void sortstats(struct stats_global *, int, int);
static void sort_stats_alphabetically(globstats *);
static void sortstats_wpl(struct stats_global *, int);
static void sortstats_vocables(struct stats_global *, int);
static void sortstats_word(struct stats_global *, int);
static void sortstats_idle(struct stats_global *, int);
static void sortwordstats(locstats *, globstats *);
static void free_stats();
static void free_localstats(struct stats_local *sl);
static void free_wordstats(wordstats *l);
static void free_quotes(quotestr *l);
static void free_topics(topicstr *e);
static void free_urls(struct stats_url *e);
static void free_kicks(struct stats_kick *e);
static void free_hosts(hoststr *e);
