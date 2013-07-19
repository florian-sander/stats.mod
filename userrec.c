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


/* nick2suser():
 * searches the channel list for a nick and returns a pointer
 * to his/her user-struct, if found.
 */
static struct stats_memberlist *nick2suser(char *nick, char *channel)
{
  struct stats_chanset *chan;
  struct stats_memberlist *m, *lm;

  Context;
  chan = findschan(channel);
  if (!chan)
    chan = initchan(channel);
  if (chan) {
    lm = NULL;
    m = chan->members;
    while (m) {
      if (!rfc_casecmp(m->nick, nick)) {
        if (lm) {
          // move this entry to the top of the list
          // since chatting means writing several sentences,
          // we'll probably need this item again pretty soon.
          // => should be faster
          lm->next = m->next;
          m->next = chan->members;
          chan->members = m;
        }
        if (!m->user)
          return NULL;
        else
          return m;
      }
      lm = m;
      m = m->next;
    }


  }
  return NULL;
}

static struct stats_chanset *findschan(char *channel)
{
  struct stats_chanset *chan;

  for (chan = schans; chan; chan = chan->next)
    if (!strcasecmp(chan->chan, channel))
      return chan;
  return NULL;
}

static struct stats_chanset *initchan(char *channel)
{
  struct chanset_t *ch;
  memberlist *m;
  struct stats_chanset *chan, *nchan;
  char *host;

  Context;
  debug1("Stats.mod: Initing chanset for %s", channel);
  ch = findchan_by_dname(channel);
  if (!ch) {
    debug1("initchan: no such chan %s", channel);
    return NULL;
  }
  if (ch->channel.members < 1) {
    debug2("initchan: %s members == %d", channel, ch->channel.members);
    return NULL;
  }
  chan = schans;
  while (chan && chan->next)
    chan = chan->next;
  nchan = nmalloc(sizeof(struct stats_chanset));
  nchan->chan = nmalloc(strlen(channel) + 1);
  strcpy(nchan->chan, channel);
  nchan->next = NULL;
  nchan->members = NULL;
  if (chan)
    chan->next = nchan;
  else
    schans = nchan;
  for (m = ch->channel.member; m; m = m->next) {
    if (!m->userhost) {
      debug2("Stats.Mod: No host for %s in %s available, yet.",
             m->nick, channel);
      continue;
    }
    if (m->user == NULL) {
      host = nmalloc(strlen(m->nick) + 1 + strlen(m->userhost) + 1);
      sprintf(host, "%s!%s", m->nick, m->userhost);
      m->user = get_user_by_host(host);
      nfree(host);
    }
    saddmember(m->nick, m->userhost, m->user ? m->user->handle : "*", channel);
  }
  return nchan;
}

static void saddmember(char *nick, char *uhost, char *hand, char *channel)
{
  struct stats_chanset *chan;
  struct stats_memberlist *m, *nm;
  char *host;

  Context;
  if (!nick) {
    debug0("kein nick");
    return;
  }
  Context;
  if (!nick[0]) {
    debug0("kein nick0");
    return;
  }
  debug3("saddmember(%s, %s, %s)", channel, nick, uhost);
  chan = findschan(channel);
  if (!chan)
    return;
  m = chan->members;
  while (m && m->next)
    m = m->next;
  nm = nmalloc(sizeof(struct stats_memberlist));
  nm->nick = nmalloc(strlen(nick) + 1);
  strcpy(nm->nick, nick);
  nm->uhost = nmalloc(strlen(uhost) + 1);
  strcpy(nm->uhost, uhost);
  if (hand[0] != '*') {
    nm->user = findsuser_by_name(hand);
    if (!nm->user) {
      nm->user = addsuser(hand, 1, 1);
      debug1("Stats.Mod: Created suserrec for %s.", hand);
    }
  } else {
    host = nmalloc(strlen(nick) + 1 + strlen(uhost) + 1);
    sprintf(host, "%s!%s", nick, uhost);
    nm->user = findsuser(host);
    nfree(host);
  }
  if (nm->user)
    nm->stats = findlocstats(channel, nm->user->user);
  else
    nm->stats = NULL;
  nm->next = NULL;
  nm->joined = now;
  if (m)
    m->next = nm;
  else
    chan->members = nm;
}

static void strackmember(char *channel, char *oldnick, char *newnick)
{
  struct stats_chanset *chan;
  struct stats_memberlist *m;

  Context;
  chan = findschan(channel);
  if (!chan)
    return;
  for (m = chan->members; m; m = m->next) {
    if (!rfc_casecmp(m->nick, oldnick)) {
      nfree(m->nick);
      m->nick = nmalloc(strlen(newnick) + 1);
      strcpy(m->nick, newnick);
      return;
    }
  }
}

static void skillmember(char *channel, char *nick)
{
  struct stats_chanset *chan;
  struct stats_memberlist *m, *mm;

  Context;
  chan = findschan(channel);
  if (!chan)
    return;
  m = chan->members;
  mm = NULL;
  while (m) {
    if (!rfc_casecmp(m->nick, nick)) {
      nfree(m->nick);
      nfree(m->uhost);
      if (mm)
        mm->next = m->next;
      else
        chan->members = m->next;
      nfree(m);
      return;
    }
    mm = m;
    m = m->next;
  }
  putlog(LOG_MISC, "*", "Stats.mod: skillmember(%s, %s) failed!!!", channel, nick);
}


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

static struct stats_userlist *addsuser(char *user, int list, int addhosts)
{
  struct stats_userlist *u, *nu;

  Context;
  for (u = suserlist; u; u = u->next)
    if (!rfc_casecmp(u->user, user))
      return u;
  u = suserlist;
  while (u && u->next)
    u = u->next;
  nu = nmalloc(sizeof(struct stats_userlist));
  nu->user = nmalloc(strlen(user) + 1);
  strcpy(nu->user, user);
  nu->list = list;
  nu->addhosts = addhosts;
  nu->next = NULL;
  nu->hosts = NULL;
  nu->email = NULL;
  nu->homepage = NULL;
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
      free_hostlist(u->hosts);
      if (u->email)
        nfree(u->email);
      if (u->homepage)
        nfree(u->homepage);
      nfree(u->user);
      weed_userlink_from_chanset(u);
      weed_userlink_from_locstats(u);
      nfree(u);
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

static void saddhost(struct stats_userlist *u, char *host, time_t lastused)
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

static void stats_autosadd(struct stats_memberlist *m, struct stats_chanset *chan)
{
  struct stats_userlist *u;
  struct userrec *uu;
  char *mhost, *host, *egghost;
  struct chanset_t *eggchan;
  memberlist *eggmember;

  Context;
  if (autoadd < 0)
    return;
  if ((now - m->joined) < (autoadd * 60))
    return;
  if (m->user) {
    debug3("Stats.Mod: stats_autosadd called for %s in %s, but m->user already belongs to %s",
           m->nick, chan->chan, m->user->user);
    return;
  }
#ifndef OLDBOT
  eggchan = findchan_by_dname(chan->chan);
#else
  eggchan = findchan(chan->chan);
#endif
  if (!eggchan) {
    putlog(LOG_MISC, "*", "Stats.Mod: Couldn't find eggdrop channel data while autoadding in %s.", chan->chan);
    return;
  }
  eggmember = ismember(eggchan, m->nick);
  if (!eggmember) {
    putlog(LOG_MISC, "*", "Stats.Mod: Couldn't find eggdrop member data while autoadding %s in %s.", m->nick, chan->chan);
    return;
  }
  egghost = eggmember->userhost;
  if (!egghost) {
    debug2("Stats.Mod: Couldn't autoadd %s in %s because there isn't an validated host, yet.", m->nick, chan->chan);
    return;
  }
  u = findsuser_by_name(m->nick);
  host = nmalloc(strlen(egghost) + strlen(m->nick) + 2);
  sprintf(host, "%s!%s", m->nick, egghost);
  mhost = nmalloc(strlen(host) + 10); /* better a few bytes too much than too little */
  // I use maskstricthost() here, because stats.mod shouldn't strip
  // a host anywhere at all. (strict-hosts 0 sucks...)
  maskstricthost(host, mhost);
//  mhost = nrealloc(mhost, strlen(mhost) + strlen(nick) + 1);sprintf(mhost, "%s%s", m->nick, mhost + 1);
  if (u) {
    if (u->addhosts) {
      saddhost(u, mhost, now);
      m->user = u;
      putlog(LOG_MISC, "*", "Stats.Mod: Added stats-hostmask %s to %s.", mhost, u->user);
    }
  } else {
    uu = get_user_by_host(host);
    if (!uu && (autoadd == 0)) {
      nfree(mhost);
      nfree(host);
      return;
    }
    if (uu)
      u = addsuser(uu->handle, 1, 1);
    else
      u = addsuser(m->nick, 1, 1);
    saddhost(u, mhost, now);
    if (uu)
      putlog(LOG_MISC, "*", "Stats.Mod: %s matched %s(in the \"common\" userfile), added %s to userbase.", host, uu->handle, u->user);
    else
      putlog(LOG_MISC, "*", "Stats.Mod: Added %s(%s) to userbase.", u->user, mhost);
    m->user = u;
  }
  if (m->user)
    m->stats = findlocstats(chan->chan, m->user->user);
  else
    m->stats = NULL;
  nfree(mhost);
  nfree(host);
}

static int listsuser(locstats *ls, char *chan)
{
  struct userrec *ou;

  if (!use_userfile) {
    if (!ls->u)
      ls->u = findsuser_by_name(ls->user);
    if (ls->u && !ls->u->list)
      return 0;
  } else {
    ou = get_user_by_handle(userlist, ls->user);
    if (matchattr(ou, nostatsflags, chan))
      return 0;
  }
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
  struct stats_chanset *chan;
  struct stats_memberlist *m;

  Context;
  for (chan = schans; chan; chan = chan->next) {
    for (m = chan->members; m; m = m->next) {
      if (m->user == u) {
        m->user = NULL;
        m->stats = NULL;
      }
    }
  }
}

static void weed_statlink_from_chanset(locstats *ls)
{
  struct stats_chanset *chan;
  struct stats_memberlist *m;

  Context;
  for (chan = schans; chan; chan = chan->next) {
    for (m = chan->members; m; m = m->next) {
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

static int countmembers(struct stats_memberlist *m)
{
  int nr = 0;

  Context;
  while (m) {
    nr++;
    m = m->next;
  }
  return nr;
}

static void check_desynch()
{
  struct stats_chanset *chan;
  struct chanset_t *ch;

  Context;
  for (ch = chanset; ch; ch = ch->next) {
    if (ch->status & CHAN_INACTIVE)
      continue;
#if EGG_IS_MIN_VER(10500)
    chan = findschan(ch->dname);
#else
    chan = findschan(ch->name);
#endif
    if (chan)
      if (ch->channel.members == countmembers(chan->members))
        continue;
#if EGG_IS_MIN_VER(10500)
    putlog(LOG_DEBUG, "*", "Stats.Mod: Channel list for %s desynched!!! Resetting...", ch->dname);
#else
    putlog(LOG_DEBUG, "*", "Stats.Mod: Channel list for %s desynched!!! Resetting...", ch->name);
#endif
    if (chan)
      free_one_chan(chan->chan);
#if EGG_IS_MIN_VER(10500)
    initchan(ch->dname);
#else
    initchan(ch->name);
#endif
  }
}

/* used when shosts/susers changed */
static void update_schannel_members()
{
  struct stats_chanset *chan;
  struct stats_memberlist *m;
  struct userrec *u;
  char *host;

  Context;
  for (chan = schans; chan; chan = chan->next) {
    for (m = chan->members; m; m = m->next) {
      m->user = NULL;
      host = nmalloc(strlen(m->nick) + 1 + strlen(m->uhost) + 1);
      sprintf(host, "%s!%s", m->nick, m->uhost);
      u = get_user_by_host(host);
      if (u) {
        m->user = findsuser_by_name(u->handle);
        if (!m->user) {
          m->user = addsuser(u->handle, 1, 0);
          debug1("Stats.Mod: Created suserrec for %s.", u->handle);
        }
      } else
        m->user = findsuser(host);
      nfree(host);
      m->stats = NULL;
    }
  }
  debug0("update_schannel_members()");
}

static void setemail(struct stats_userlist *u, char *email)
{
  if (!u) {
    putlog(LOG_MISC, "*", "ERROR! Tried to set email for NULL!");
    return;
  }
  if (u->email)
    nfree(u->email);
  u->email = nmalloc(strlen(email) + 1);
  strcpy(u->email, email);
}

static void sethomepage(struct stats_userlist *u, char *homepage)
{
  if (!u) {
    putlog(LOG_MISC, "*", "ERROR! Tried to set homepage for NULL!");
    return;
  }
  if (u->homepage)
    nfree(u->homepage);
  u->homepage = nmalloc(strlen(homepage) + 1);
  strcpy(u->homepage, homepage);
}
