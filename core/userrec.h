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

#define S_LIST		1
#define S_ADDHOSTS	2
#define S_NOSTATS	4

#define U_NOPASSWORD	1
#define U_NOEMAIL		2

static void stats_autosadd(struct stats_member *m, struct stats_chan *chan);

struct stats_hostlist {
  struct stats_hostlist *next;
  char *mask;
  time_t lastused;
  time_t created;
};

struct stats_userlist {
  struct stats_userlist *next;
  char *user;
  char *password;
  char *email;
  char *homepage;
  int flags;
  int icqnr;
  time_t created;
  time_t laston;
  struct stats_hostlist *hosts;
};

#define suser_list(u)		(u->flags & S_LIST)
#define suser_addhosts(u)	(u->flags & S_ADDHOSTS)
#define suser_nostats(u)	(u->flags & S_NOSTATS)

#define suser_setflag(u, flag)	(u->flags |= flag)
#define suser_delflag(u, flag)	(u->flags &= ~flag)

#define TIMETOLIVE(x) (((now - x->created) * (expire_factor / 100)) + (expire_base * 86400))

static struct stats_userlist *addsuser(char *, time_t, time_t);
static struct stats_userlist *findsuser(char *);
static struct stats_userlist *findsuser_by_name(char *);
static struct stats_userlist *stats_userlist_create_entry(char *);
static void stats_userlist_free_entry(struct stats_userlist *);
static void saddhost(struct stats_userlist *u, char *host, time_t lastused, time_t created);
static void welcome_suser(char *nick, struct stats_userlist *u, char *chan);
static int listsuser(locstats *ls, char *chan);
static void weed_userlink_from_chanset(struct stats_userlist *u);
static void weed_statlink_from_chanset(locstats *ls);
static void weed_userlink_from_locstats(struct stats_userlist *u);
static void setemail(struct stats_userlist *u, char *email);
static void sethomepage(struct stats_userlist *u, char *homepage);
static void setpassword(struct stats_userlist *u, char *password);
static time_t get_creation_time_from_locstats(char *user);
static time_t get_laston_time_from_hosts(char *user);
static void free_suserlist(struct stats_userlist *e);
static void free_hostlist(struct stats_hostlist *e);
