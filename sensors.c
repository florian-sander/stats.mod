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

static int sensor_msgm(char *nick, char *uhost, char *hand, char *chan, char *rest)
{
  int i;
  char buf[511];
  struct stats_memberlist *m;
  globstats *gs;

  if (nostats(chan))
    return 0;
  strncpy(buf, rest, 510);
  buf[510] = 0;
  rest = buf;
  gs = findglobstats(chan);
  add_chanlog(gs, nick, rest, SL_PRIVMSG);
  m = nick2suser(nick, chan);
  if(!use_userfile) {
    if (!m) {
      putlog(LOG_LEV5, chan, "!m (%s)", nick);
      check_for_url(nick, chan, rest);
      return 0;
    }
    if (!m->user) {
      putlog(LOG_LEV5, chan, "!m->user (%s)", nick);
      check_for_url(nick, chan, rest);
      return 0;
    }
    check_for_url(m->user->user, chan, rest);
    // if there's no link to the stats, call initstats() which either
    // returns an existing stats struct, or initializes a new one
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_WORDS, countwords(rest));
    nincrstats(m->stats, T_LETTERS, strlen(rest));
    nincrstats(m->stats, T_LINES, 1);
    i = countsmileys(rest);
    if (i)
      nincrstats(m->stats, T_SMILEYS, i);
    i = countquestions(rest);
    if (i)
      nincrstats(m->stats, T_QUESTIONS, i);
    addquote(hand, gs, rest, m->stats);
    calcwordstats(hand, gs, rest, m->stats);
  } else {
    if (hand[0] == '*') {
      putlog(LOG_LEV5, chan, "hand[0] == '*' (%s)", nick);
      check_for_url(nick, chan, rest);
      return 0;
    }
    check_for_url(hand, chan, rest);
    incrstats(hand, chan, T_WORDS, countwords(rest), 0);
    incrstats(hand, chan, T_LETTERS, strlen(rest), 0);
    incrstats(hand, chan, T_LINES, 1, 0);
    i = countsmileys(rest);
    if (i)
      incrstats(hand, chan, T_SMILEYS, i, 0);
    i = countquestions(rest);
    if (i)
      incrstats(hand, chan, T_QUESTIONS, i, 0);
    addquote(hand, gs, rest, NULL);
    calcwordstats(hand, gs, rest, NULL);
  }
  return 0;
}

static void sensor_minutely()
{
  struct chanset_t *chan;
  memberlist *m;
  struct stats_chanset *ch;
  struct stats_memberlist *mm;

  Context;
  if (use_userfile) {
    for (chan = chanset; chan; chan = chan->next) {
#ifndef OLDBOT
      if (nostats(chan->dname))
#else
      if (nostats(chan->name))
#endif
        continue;
      if (chan->channel.members > 0) {
        for (m = chan->channel.member; m; m = m->next) {
#ifndef OLDBOT
          if (m->user != NULL)
            incrstats(m->user->handle, chan->dname, T_MINUTES, 1, 0);
          else
            stats_autoadd(m, chan->dname);
#else
          if (m->user != NULL)
            incrstats(m->user->handle, chan->name, T_MINUTES, 1, 0);
          else
            stats_autoadd(m, chan->name);
#endif
        }
      }
    }
  } else {
    for (ch = schans; ch; ch = ch->next) {
      if (nostats(ch->chan))
        continue;
      for (mm = ch->members; mm; mm = mm->next) {
        if (mm->user) {
          if (!mm->stats)
            mm->stats = initstats(ch->chan, mm->user->user);
          // now let's use a generic flag to check if we already increased this
          // user's minutes (if he's a clone, for example)
          // (thanks to Zev for this idea)
          if (mm->stats->flag)
            continue;
          nincrstats(mm->stats, T_MINUTES, 1);
          mm->stats->flag = 1;
        } else {
          stats_autosadd(mm, ch);
        }
      }
      // now reset the flag
      for (mm = ch->members; mm; mm = mm->next)
        if (mm->stats)
          mm->stats->flag = 0;
    }
  }
  Context;
}

static void sensor_countusers()
{
  struct chanset_t *chan;
  memberlist *m;
  struct stats_chanset *ch;
  struct stats_memberlist *mm;
  globstats *gs;
  int hour, nr;

  for (chan = chanset; chan; chan = chan->next) {
    if (chan->channel.members < 1)
      continue;
    nr = 0;
#if EGG_IS_MIN_VER(10500)
    if (nostats(chan->name))
      continue;
    gs = findglobstats(chan->dname);
#else
    if (nostats(chan->name))
      continue;
    gs = findglobstats(chan->name);
#endif
    if (!gs)
      continue;
    if (use_userfile) {
      for (m = chan->channel.member; m; m = m->next) {
        if (!m->nick[0])
          continue;
        if (m->user != NULL)
#if EGG_IS_MIN_VER(10500)
          if (matchattr(m->user, nostatsflags, chan->dname))
#else
          if (matchattr(m->user, nostatsflags, chan->name))
#endif
            continue;
        nr++;
      }
    } else {
#if EGG_IS_MIN_VER(10500)
      ch = findschan(chan->dname);
#else
      ch = findschan(chan->name);
#endif
      if (!ch)
        continue;
      for (mm = ch->members; mm; mm = mm->next) {
	if (mm->user && !mm->user->list)
	  continue;
	nr++;
      }
    }
    hour = gethour();
    if (hour != lasthour) {
      gs->users[S_USERSUM][hour] = nr;
      gs->users[S_USERCOUNTS][hour] = 1;
      lasthour = hour;
    } else {
      gs->users[S_USERSUM][hour] += nr;
      if (gs->users[S_USERCOUNTS][hour] < 0)
        gs->users[S_USERCOUNTS][hour] = 1;
      else
        gs->users[S_USERCOUNTS][hour]++;
    }
  }
}

static void sensor_peak(char *channel)
{
  struct chanset_t *chan;
  memberlist *m;
  struct stats_chanset *ch;
  struct stats_memberlist *mm;
  globstats *gs;
  int users = 0;

  if (nostats(channel))
    return;
  gs = findglobstats(channel);
  if (!gs)
    return;
#ifndef OLDBOT
  chan = findchan_by_dname(channel);
#else
  chan = findchan(channel);
#endif
  if (!chan)
    return;
  if (use_userfile) {
    if (chan->channel.members > 0) {
      for (m = chan->channel.member; m; m = m->next) {
        if (!m->nick[0])
          continue;
        if (m->user) {
          if (matchattr(m->user, nopeak, channel))
            continue;
        }
        users++;
      }
    }
  } else {
#if EGG_IS_MIN_VER(10500)
    ch = findschan(chan->dname);
#else
    ch = findschan(chan->name);
#endif
    if (!ch)
      return;
    for (mm = ch->members; mm; mm = mm->next) {
      if (mm->user && !mm->user->list)
	continue;
      users++;
    }
  }
  if (users > gs->peak[S_TOTAL]) {
    gs->peak[S_TOTAL] = users;
    putlog(LOG_MISC, "*", "New user peak in %s: %d.", channel, users);
  }
  if (users > gs->peak[S_TODAY])
    gs->peak[S_TODAY] = users;
  if (users > gs->peak[S_WEEKLY])
    gs->peak[S_WEEKLY] = users;
  if (users > gs->peak[S_MONTHLY])
    gs->peak[S_MONTHLY] = users;

}

static int sensor_topc(char *nick, char *uhost, char *hand, char *chan, char *topic)
{
  struct stats_memberlist *m;

  Context;
  if (nostats(chan))
    return 0;
  if (!use_userfile) {
    m = nick2suser(nick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_TOPICS, 1);
  } else if (hand[0] != '*')
    incrstats(hand, chan, T_TOPICS, 1, 0);
  addtopic(chan, topic, nick);
  return 0;
}

static int sensor_action(char *nick, char *uhost, char *hand, char *chan, char *key, char *rest)
{
  char *pbuf;
  struct stats_memberlist *m;

  if (!strchr(CHANMETA, chan[0]))
    return 0;
  if (nostats(chan))
    return 0;
  if (!use_userfile) {
    m = nick2suser(nick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_ACTIONS, 1);
  } else if (hand[0] != '*')
    incrstats(hand, chan, T_ACTIONS, 1, 0);
  pbuf = nmalloc(strlen(nick) + strlen(rest) + 2);
  sprintf(pbuf, "%s %s", nick, rest);
  sensor_msgm(nick, uhost, hand, chan, pbuf);
  nfree(pbuf);
  return 0;
}

static int sensor_kick(char *nick, char *uhost, char *hand, char *chan, char *victim, char *reason)
{
  struct stats_memberlist *m;
  char *buf;
  globstats *gs;

  if (nostats(chan))
    return 0;
  gs = findglobstats(chan);
  buf = nmalloc(strlen(victim) + strlen(nick) + strlen(reason) + 23);
  sprintf(buf, "*** %s was kicked by %s (%s)", victim, nick, reason);
  add_chanlog(gs, nick, buf, SL_KICK);
  save_kick(gs, buf);
  nfree(buf);
  skillmember(chan, victim);
  if (!use_userfile) {
    m = nick2suser(nick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_KICKS, 1);
  } else if (hand[0] != '*')
    incrstats(hand, chan, T_KICKS, 1, 0);
  return 0;
}

static int sensor_mode(char *nick, char *uhost, char *hand, char *chan, char *mode, char *victim)
{
  struct stats_memberlist *m;
  char *buf;

  if (nostats(chan))
    return 0;
  Assert(mode);
  if (mode[1] == 'k')
  	return 0;
  buf = nmalloc(strlen(nick) + strlen(mode) + strlen(victim) + 13);
  sprintf(buf, "%s sets mode %s %s", nick, mode, victim);
  add_chanlog(findglobstats(chan), nick, buf, SL_MODE);
  nfree(buf);
  if (!use_userfile) {
    m = nick2suser(nick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_MODES, 1);
    if ((mode[1] == 'b') && (mode[0] == '+'))
      nincrstats(m->stats, T_BANS, 1);
  } else if (hand[0] != '*') {
    incrstats(hand, chan, T_MODES, 1, 0);
    if ((mode[1] == 'b') && (mode[0] == '+'))
      incrstats(hand, chan, T_BANS, 1, 0);
  }
  return 0;
}

static int sensor_nick(char *nick, char *uhost, char *hand, char *chan, char *newnick)
{
  struct stats_memberlist *m;

  if (nostats(chan))
    return 0;
  add_chanlog(findglobstats(chan), nick, newnick, SL_NICK);
  strackmember(chan, nick, newnick);
  if (!use_userfile) {
    m = nick2suser(newnick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_NICKS, 1);
  } else if (hand[0] != '*')
    incrstats(hand, chan, T_NICKS, 1, 0);
  return 0;
}

static int sensor_join(char *nick, char *uhost, char *hand, char *chan)
{
  struct stats_memberlist *m;

  if (nostats(chan))
    return 0;
  add_chanlog(findglobstats(chan), nick, NULL, SL_JOIN);
  if (match_my_nick(nick))
    free_one_chan(chan);
  else
    saddmember(nick, uhost, hand, chan);
  sensor_peak(chan);
  if (!use_userfile) {
    m = nick2suser(nick, chan);
    if (!m)
      return 0;
    if (!m->user)
      return 0;
    if (!m->stats)
      m->stats = initstats(chan, m->user->user);
    nincrstats(m->stats, T_JOINS, 1);
  } else if (hand[0] != '*')
    incrstats(hand, chan, T_JOINS, 1, 0);
  addhost(uhost, findglobstats(chan));
  return 0;
}

static int sensor_part(char *nick, char *uhost, char *hand, char *chan)
{
  if (nostats(chan))
    return 0;
  add_chanlog(findglobstats(chan), nick, NULL, SL_PART);
  skillmember(chan, nick);
  return 0;
}

static int sensor_sign(char *nick, char *uhost, char *hand, char *chan, char *reason)
{
  if (nostats(chan))
    return 0;
  add_chanlog(findglobstats(chan), nick, reason, SL_QUIT);
  skillmember(chan, nick);
  return 0;
}

static cmd_t stats_pubm[] =
{
  {"*", "", (Function) sensor_msgm, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_topc[] =
{
  {"*", "", (Function) sensor_topc, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_ctcp[] =
{
  {"ACTION", "", (Function) sensor_action, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_kick[] =
{
  {"*", "", (Function) sensor_kick, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_mode[] =
{
  {"*", "", (Function) sensor_mode, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_nick[] =
{
  {"*", "", (Function) sensor_nick, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_join[] =
{
  {"*", "", (Function) sensor_join, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_part[] =
{
  {"*", "", (Function) sensor_part, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_sign[] =
{
  {"*", "", (Function) sensor_sign, "stat"},
  {0, 0, 0, 0}
};
