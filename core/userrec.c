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

/* Find the stats-user that belongs to a hostmask
*/
static struct stats_userlist *findsuser(char *host)
{
  struct stats_userlist *user, *u;
  struct stats_hostlist *h, *h2;
  int len = 0;

  Context;
  u = NULL;
  h2 = NULL;
  for (user = suserlist; user; user = user->next) {
    for (h = user->hosts; h; h = h->next) {
      /* the longest hostmask gives the best match */
      if (!len || (strlen(h->mask) > len)) {
        if (wild_match(h->mask, host)) {
          u = user;
          h2 = h;
          len = strlen(h->mask);
        }
      }
    }
  }
  if (u) {
    h2->lastused = now;
    return u;
  }
  return NULL;
}

static struct stats_userlist *findsuser_by_name(char *user)
{
  struct stats_userlist *u;

  Context;
  for (u = suserlist; u; u = u->next)
    if (!rfc_casecmp(u->user, user))
      return u;
  return NULL;
}

static struct stats_userlist *addsuser(char *user, time_t created, time_t laston)
{
  struct stats_userlist *u, *nu;

  Context;
  for (u = suserlist; u; u = u->next)
    if (!rfc_casecmp(u->user, user))
      return u;
  u = suserlist;
  while (u && u->next)
    u = u->next;
  nu = stats_userlist_create_entry(user);
  nu->user = nmalloc(strlen(user) + 1);
  strcpy(nu->user, user);
  nu->created = created;
  nu->laston = laston;
  if (u)
    u->next = nu;
  else
    suserlist = nu;
  return nu;
}

static void delsuser(char *user)
{
  struct stats_userlist *u, *lu;

  Context;
  debug1("Deleting %s...", user);
  u = suserlist;
  lu = NULL;
  while (u) {
    if (!rfc_casecmp(u->user, user)) {
      if (lu)
        lu->next = u->next;
      else
        suserlist = u->next;
      stats_userlist_free_entry(u);
      if (lu)
        u = lu->next;
      else
        u = suserlist;
    } else {
      lu = u;
      u = u->next;
    }
  }
}

static struct stats_userlist *stats_userlist_create_entry(char *user)
{
  struct stats_userlist *newentry;

  newentry = nmalloc(sizeof(struct stats_userlist));
  newentry->next = NULL;
  newentry->user = NULL;
  newentry->password = NULL;
  newentry->email = NULL;
  newentry->homepage = NULL;
  newentry->flags = 0;
  newentry->icqnr = 0;
  newentry->hosts = NULL;
  newentry->created = 0;
  newentry->laston = 0;
  suser_setflag(newentry, S_LIST);
  suser_setflag(newentry, S_ADDHOSTS);

  return newentry;
}

/* static int stats_userlist_expmem_entry(struct stats_userlist *what)
{
  int size = 0;

  Assert(what);
  Assert(what->user);
  size += sizeof(struct stats_userlist);
  size += strlen(what->user) + 1;
  if (what->password)
    size += strlen(what->password) + 1;
  if (what->email)
    size += strlen(what->email) + 1;
  if (what->homepage)
    size += strlen(what->homepage) + 1;
  size += hostlist_expmem(what->hosts);
  return size;
} */

static void stats_userlist_free_entry(struct stats_userlist *what)
{
  Assert(what);
  Assert(what->user);
  free_hostlist(what->hosts);
  nfree(what->user);
  if (what->email)
    nfree(what->email);
  if (what->homepage)
    nfree(what->homepage);
  if (what->password)
    nfree(what->password);
  weed_userlink_from_chanset(what);
  weed_userlink_from_locstats(what);
  nfree(what);
}

static void saddhost(struct stats_userlist *u, char *host, time_t lastused, time_t created)
{
  struct stats_hostlist *h, *nh;

  Context;
  for (h = u->hosts; h; h = h->next)
    if (!rfc_casecmp(h->mask, host))
      return;
  h = u->hosts;
  while (h && h->next)
    h = h->next;
  nh = nmalloc(sizeof(struct stats_hostlist));
  nh->mask = nmalloc(strlen(host) + 1);
  strcpy(nh->mask, host);
  nh->lastused = lastused;
  nh->created = created;
  nh->next = NULL;
  if (h)
    h->next = nh;
  else
    u->hosts = nh;
}

static int sdelhost(struct stats_userlist *u, char *host)
{
  struct stats_hostlist *h, *lh;

  Context;
  h = u->hosts;
  lh = NULL;
  while (h) {
    if (!rfc_casecmp(h->mask, host)) {
      nfree(h->mask);
      if (lh)
        lh->next = h->next;
      else
        u->hosts = h->next;
      nfree(h);
      return 1;
    }
    lh = h;
    h = h->next;
  }
  return 0;
}

static void stats_autosadd(struct stats_member *m, struct stats_chan *chan)
{
  struct stats_userlist *u;
  struct userrec *uu;
  char *mhost, *host;

  Context;
  if (autoadd < 0)
    return;
  if (m->spoken_lines < autoadd_min_lines)
    return;
  if ((now - m->joined) < (autoadd * 60))
    return;
  if (m->user) {
    debug3("Stats.Mod: stats_autosadd called for %s in %s, but m->user already belongs to %s",
           m->nick, chan->chan, m->user->user);
    return;
  }
  u = findsuser_by_name(m->nick);
  host = nmalloc(strlen(m->uhost) + strlen(m->nick) + 2);
  sprintf(host, "%s!%s", m->nick, m->uhost);
  mhost = nmalloc(strlen(host) + 10); /* better a few bytes too much than too little */
  // I use maskstricthost() here, because stats.mod shouldn't strip
  // a host anywhere at all. (strict-hosts 0 sucks...)
  maskstricthost(host, mhost);
  // mhost = nrealloc(mhost, strlen(mhost) + strlen(nick) + 1);sprintf(mhost, "%s%s", m->nick, mhost + 1);
  if (u) {
    if (suser_addhosts(u)) {
      saddhost(u, mhost, now, now);
      m->user = u;
      putlog(LOG_MISC, "*", "Stats.Mod: Added stats-hostmask %s to %s.", mhost, u->user);
    }
  } else {
#ifndef NO_EGG
    uu = get_user_by_host(host);
    if (!uu && (autoadd == 0)) {
      nfree(mhost);
      nfree(host);
      return;
    }
    if (uu)
      u = addsuser(uu->handle, now, now);
    else
#endif
      u = addsuser(m->nick, now, now);
    saddhost(u, mhost, now, now);
#ifndef NO_EGG
    if (uu)
      putlog(LOG_MISC, "*", "Stats.Mod: %s matched %s(in the \"common\" userfile), added %s to userbase.", host, uu->handle, u->user);
    else
#endif
      putlog(LOG_MISC, "*", "Stats.Mod: Added %s(%s) to userbase.", u->user, mhost);
    m->user = u;
    // send a welcome message to our new user
    welcome_suser(m->nick, u, chan->chan);
  }
  if (m->user) {
    m->stats = findlocstats(chan->chan, m->user->user);
    if (!m->stats)
      m->stats = initstats(chan->chan, m->user->user);
  } else
    m->stats = NULL;
  nfree(mhost);
  nfree(host);
}

static void welcome_suser(char *nick, struct stats_userlist *u, char *chan)
{
  char *text;

  reset_global_vars();
  glob_user = u;
  glob_nick = nick;
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));
  if ((text = getslang_first(500))) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, text);
    while ((text = getslang_next()))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, text);
  }
}

static int listsuser(locstats *ls, char *chan)
{
  if (!ls->u)
    ls->u = findsuser_by_name(ls->user);
  if (ls->u && !suser_list(ls->u))
    return 0;
  return 1;
}

static int countsusers()
{
  static struct stats_userlist *u;
  int users = 0;

  Context;
  for (u = suserlist; u; u = u->next)
    users++;
  return users;
}

static int counthosts()
{
  static struct stats_userlist *u;
  static struct stats_hostlist *h;
  int hosts = 0;

  Context;
  for (u = suserlist; u; u = u->next)
    for (h = u->hosts; h; h = h->next)
      hosts++;
  return hosts;
}

static void weed_userlink_from_chanset(struct stats_userlist *u)
{
  struct stats_chan *chan;
  struct stats_member *m;

  Context;
  for (chan = schan_getfirst(); chan; chan = schan_getnext()) {
    for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
      if (m->user == u) {
        m->user = NULL;
        m->stats = NULL;
      }
    }
  }
}

static void weed_statlink_from_chanset(locstats *ls)
{
  struct stats_chan *chan;
  struct stats_member *m;

  Context;
  for (chan = schan_getfirst(); chan; chan = schan_getnext()) {
    for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
      if (m->stats == ls) {
        m->stats = NULL;
      }
    }
  }
}

/* weed_userlink_from_locstats():
 * removes all references to a userstruct from the stat-structs
 * (mostly used if the user got deleted)
 */
static void weed_userlink_from_locstats(struct stats_userlist *u)
{
  globstats *gs;
  locstats *ls;

  Context;
  for (gs = sdata; gs; gs = gs->next)
    for (ls = gs->local; ls; ls = ls->next)
      if (ls->u == u)
        ls->u = NULL;
  Context;
}


static void setemail(struct stats_userlist *u, char *email)
{
  if (!u) {
    putlog(LOG_MISC, "*", "ERROR! Tried to set email for NULL!");
    return;
  }
  if (u->email) {
    debug0("email exists... deleting");
    nfree(u->email);
    u->email = NULL;
  }
  while (email[0] == ' ')
    email++;
  if (email[0]) {
    u->email = nmalloc(strlen(email) + 1);
    strcpy(u->email, email);
    debug1("newemail: '%s'", u->email);
  }
}

static void sethomepage(struct stats_userlist *u, char *homepage)
{
  int len;

  if (!u) {
    putlog(LOG_MISC, "*", "ERROR! Tried to set homepage for NULL!");
    return;
  }
  if (u->homepage) {
    nfree(u->homepage);
    u->homepage = NULL;
  }
  while (homepage[0] == ' ')
    homepage++;
  if (homepage[0]) {
    if (!strncasecmp(homepage, "http://", 7)) {
      u->homepage = nmalloc(strlen(homepage) + 1);
      strcpy(u->homepage, homepage);
    } else {
      len = strlen(homepage) + 7 + 1;
      u->homepage = nmalloc(len);
      snprintf(u->homepage, len, "http://%s", homepage);
    }
  }
}

static void setpassword(struct stats_userlist *u, char *password)
{
  if (!u) {
    putlog(LOG_MISC, "*", "ERROR! Tried to set password for NULL!");
    return;
  }
  if (u->password) {
    nfree(u->password);
    u->password = NULL;
  }
  while (password[0] == ' ')
    password++;
  if (password[0]) {
    u->password = nmalloc(strlen(password) + 1);
    strcpy(u->password, password);
  }
}

static time_t get_creation_time_from_locstats(char *user)
{
  struct stats_chan *chan;
  locstats *ls;
  time_t creation = now;

  for (chan = schan_getfirst(); chan; chan = schan_getnext()) {
    ls = findlocstats(chan->chan, user);
    if (ls) {
      if (ls->started < creation)
        creation = ls->started;
    } else
      debug2("no ls: %s@%s", user, chan->chan);
  }
  debug2("creation of %s: %lu", user, creation);
  if (creation == now)
    debug0("creation == now!");
  return creation;
}

static time_t get_laston_time_from_hosts(char *user)
{
  struct stats_userlist *u;
  struct stats_hostlist *h;
  time_t laston = now;

  u = findsuser_by_name(user);
  if (u) {
    for (h = u->hosts; h; h = h->next)
      if (h->lastused > laston)
        laston = h->lastused;
  }
  debug2("laston of %s: %lu", user, laston);
  return laston;
}

static int user_changeflag(struct stats_userlist *u, char *mode)
{
  Assert(u);
  if (!strcasecmp(mode, "+list"))
    suser_setflag(u, S_LIST);
  else if (!strcasecmp(mode, "-list"))
    suser_delflag(u, S_LIST);
  if (!strcasecmp(mode, "+addhosts"))
    suser_setflag(u, S_ADDHOSTS);
  else if (!strcasecmp(mode, "-addhosts"))
    suser_delflag(u, S_ADDHOSTS);
  else if (!strcasecmp(mode, "+nostats")) {
    suser_setflag(u, S_NOSTATS);
    suser_delflag(u, S_LIST);
  } else if (!strcasecmp(mode, "-nostats"))
    suser_delflag(u, S_NOSTATS);
  else
    return 0;
  return 1;
}

static void free_suserlist(struct stats_userlist *e)
{
  struct stats_userlist *ee;

  Context;
  while (e) {
    ee = e->next;
    stats_userlist_free_entry(e);
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

static int user_email_password(struct stats_userlist *user)
{
	char *p, *text = NULL;
	int len = 0, newlen = 0, ret;

	if (!user->password)
		return U_NOPASSWORD;
	if (!user->email)
		return U_NOEMAIL;
	text = nmalloc(1);
	*text = 0;
	for (p = getslang_first(1510); p; p = getslang_next()) {
		newlen = strlen(p);
		text = nrealloc(text, len + newlen + 1 + 1);
		len += newlen + 1;
		strcat(text, p);
		strcat(text, "\n");
	}

    ret = email_send(user->email, getslang(1500), text);
    nfree(text);
    return ret;
}

static int user_merge(char *sTo, char *sFrom)
{
	struct stats_userlist *uTo, *uFrom;
	struct stats_hostlist *h;

	uTo = findsuser_by_name(sTo);
	uFrom = findsuser_by_name(sFrom);
	if (!uTo || !uFrom)
		return 0;
	if (!userdata_merge(sTo, sFrom))
		return 0;
	for (h = uFrom->hosts; h; h = h->next)
		saddhost(uTo, h->mask, h->lastused, h->created);
	delsuser(sFrom);
	return 1;
}
