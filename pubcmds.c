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

static int nopubstats(char *chan)
{
  if (!chan) {
    debug0("WARNING: nopubstats() called with NULL pointer");
    return 1;
  }
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("nopubstats", chan))
    return 1;
#endif
  return 0;
}

static int quietstats(char *chan)
{
  if (!chan) {
    debug0("WARNING: quietstats() called with NULL pointer");
    return 1;
  }
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("quietstats", chan))
    return 1;
#endif
  return 0;
}

static int pub_top10(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! top10 %s", nick, hand, text);
  tell_top(channel, channel, nick, text, 1, 10, 0);
  return 1;
}

static int pub_top20(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! top20 %s", nick, hand, text);
  tell_top(channel, channel, nick, text, 11, 20, 0);
  return 1;
}

static int pub_ttop10(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! ttop10 %s", nick, hand, text);
  tell_top(channel, channel, nick, text, 1, 10, 1);
  return 1;
}

static int pub_ttop20(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! ttop20 %s", nick, hand, text);
  tell_top(channel, channel, nick, text, 11, 20, 1);
  return 1;
}

static void tell_top(char *channel, char *dest, char *nick, char *type, int from, int to, int today)
{
  char t[50], *result, *tosend, *slang, *slangtype;
  int i, ti, pi;
  struct stats_global *l;
  struct stats_local *ll;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif

  Context;
  filt(type);
  if (stat_flood())
    return;
#if EGG_IS_MIN_VER(10500)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  l = findglobstats(channel);
  if (!l) {
    debug1("StatMod: Can't do topx for %s, no such entry in linked list", channel);
    return;
  }
  if (!strncasecmp(type, "word ", 5)) {
    tell_top_word(channel, dest, nick, type, from, to, l);
    return;
  }
  if (!type[0] || (strlen(type) > 45))
    strcpy(t, "words");
  else
    strcpy(t, type);
  ti = slangtypetoi(t);
  setslglobs(l->chan, l->peak[today], 0, l->started);
  if ((ti < 0) && (ti != T_WPL) && (ti != T_IDLE)) {
    slang = SLNOSUCHTYPE;
    if (quietstats(dest)) { /* if dest != chan, we want a PRIVMSG anyway */
      tosend = nmalloc(15 + strlen(nick) + strlen(slang) + 1);
      sprintf(tosend, "NOTICE %s :%s\n", nick, slang);
    } else {
      tosend = nmalloc(16 + strlen(dest) + strlen(slang) + 1);
      sprintf(tosend, "PRIVMSG %s :%s\n", dest, slang);
    }
    dprintf(DP_HELP, "%s", tosend);
    nfree(tosend);
    return;
  }
  sortstats(l, ti, today);
  slglobint = to;
  slang = getslang(1001 + today);
  slangtype = getslangtype(t);
  result = nmalloc(strlen(slang) + strlen(slangtype) + 1);
  sprintf(result, slang, slangtype);
  i = 1;
//  pi = (T_WPL * (-1)) + TOTAL_TYPES - 1;  // THIS APPEARS WRONG... DOUBLE-CHECK LATER!!!
  if (ti < 0)
    pi = (ti * -1) + (TOTAL_TYPES - 1);
  else
    pi = ti;
  for (ll = l->slocal[today][pi]; ll; ll = ll->snext[today][pi]) {
    if (!listsuser(ll, channel))
      continue;
    if ((ti >= 0) && !ll->values[today][ti])
      break;
    if ((i >= from) && (i <= to)) {
      if (ti >= 0) {
        // how much should I alloc for an integer? *sigh*
        // I'll just use 10... should be enough
        result = nrealloc(result, strlen(result) + 10 + 5 + strlen(ll->user) + 10 + 1);
        sprintf(result, "%s %d. %s(%ld)", result, i, ll->user, ll->values[today][ti]);
      } else if (ti == T_WPL) {
        result = nrealloc(result, strlen(result) + 10 + 5 + strlen(ll->user) + 10 + 1);
        if (ll->values[today][T_LINES])
          sprintf(result, "%s %d. %s(%.2f)", result, i, ll->user, (float) ll->values[today][T_WORDS] / (float) ll->values[today][T_LINES]);
        else
          sprintf(result, "%s %d. %s(0)", result, i, ll->user);
      } else if (ti == T_IDLE) {
        result = nrealloc(result, strlen(result) + 10 + 5 + strlen(ll->user) + 10 + 1);
        if (ll->values[today][T_LINES])
          sprintf(result, "%s %d. %s(%.2f)", result, i, ll->user, (float) ll->values[today][T_MINUTES] / (float) ll->values[today][T_LINES]);
        else
          sprintf(result, "%s %d. %s(0)", result, i, ll->user);
      } else {
        result = nrealloc(result, strlen(result) + 11 + 10 + 1);
        sprintf(result, "%s ERROR (%d)", result, ti);
      }
    }
    i++;
  }
  if (quietstats(dest)) {
    tosend = nmalloc(12 + strlen(nick) + strlen(result) + 1);
    sprintf(tosend, "NOTICE %s :%s\n", nick, result);
  } else {
    tosend = nmalloc(13 + strlen(dest) + strlen(result) + 1);
    sprintf(tosend, "PRIVMSG %s :%s\n", dest, result);
  }
  dprintf(DP_HELP, "%s", tosend);
  nfree(tosend);
  nfree(result);
  Context;
}

static void tell_top_word(char *channel, char *dest, char *nick, char *type, int from, int to, globstats *gs)
{
  locstats *e;
  char *result, *tosend, *slang;
  int i = 1, itype;

  Context;
  newsplit(&type);
  strlower(type);
  setword(gs, type);
  sortstats(gs, T_WORDS, 1);
  slglobint = to;
  setslglobs(gs->chan, gs->peak[S_TODAY], 0, gs->started);
  slang = SLTOPWORD;
  result = nmalloc(strlen(slang) + strlen(type) + 1);
  sprintf(result, slang, type);
  itype = (T_WORDS * (-1)) + TOTAL_TYPES - 1;
  for (e = gs->slocal[S_TODAY][itype]; e; e = e->snext[S_TODAY][itype]) {
    if (!listsuser(e, channel))
      continue;
    if ((i > to) || !e->word)
      break;
    if (i >= from) {
      result = nrealloc(result, strlen(result) + 13 + 10 + strlen(e->user) + 10 + 1);
      sprintf(result, "%s %d. %s(%d)", result, i, e->user, e->word->nr);
    }
    i++;
  }
  if (quietstats(dest)) {
    tosend = nmalloc(15 + strlen(nick) + strlen(result) + 1);
    sprintf(tosend, "NOTICE %s :%s\n", nick, result);
  } else {
    tosend = nmalloc(16 + strlen(dest) + strlen(result) + 1);
    sprintf(tosend, "PRIVMSG %s :%s\n", dest, result);
  }
  dprintf(DP_HELP, "%s", tosend);
  nfree(result);
  nfree(tosend);
  Context;
}

static int pub_place(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! place %s", nick, hand, text);
  tell_place(nick, channel, hand, channel, text, S_TOTAL);
  Context;
  return 1;
}

static int pub_tplace(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! tplace %s", nick, hand, text);
  tell_place(nick, channel, hand, channel, text, S_TODAY);
  Context;
  return 1;
}

static void tell_place(char *nick, char *dest, char *hand, char *channel, char *text, int today)
{
  globstats *gs;
  locstats *ls;
  struct stats_memberlist *m;
  struct stats_userlist *su;
  struct userrec *u;
  char *who, *slang;
  int place = 0;
  int itype;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif

  Context;
  // at first, check for flood...
  if (stat_flood())
    return;
  // .. now init vars ...
  m = NULL;
  u = NULL;
  su = NULL;
  who = slang = NULL;
  // ... get the global stat-struct for the channel ...
  gs = findglobstats(channel);
  // .. and init langsystem (stuff like peak and staring time
  // shouldn't be needed)
  setslglobs(channel, 0, countstatmembers(gs), 0);
#if EGG_IS_MIN_VER(10500)
  // make sure that we're going to send to the right chan (!sa23hCHAN is WRONG)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  // if there is no parameter, just default to "words"
  if (!text[0]) {
    itype = T_WORDS;
  // else try resolving the type
  } else {
    itype = slangtypetoi(text);
  }
  // if we have a valid type now, then the parameter does not
  // specify a username, so let's use the triggering user as target.
  if (itype != T_ERROR) {
    // use_userfile is true, so grab the handle
    if (use_userfile) {
      who = nmalloc(strlen(hand) + 1);
      strcpy(who, hand);
    } else {
      // try grabbing the correct username from the userdatabase...
      m = nick2suser(nick, channel);
      if (m && m->user) {
        who = nmalloc(strlen(m->user->user) + 1);
        strcpy(who, m->user->user);
      // if the user is not found, use "*", which identifies
      // a non-existant user
      } else {
        who = nmalloc(2);
        strcpy(who, "*");
      }
    }
    // if who is "*", then the user isn't in any database. Serve him an error.
    if (who[0] == '*') {
      setslnick(nick);
      if (quietstats(dest)) {
        dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTYOU);
      } else {
        dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTYOU);
      }
      if (who)
        nfree(who);
      return;
    }
  } else { // itype == T_ERROR, parameter must be a username
    itype = T_WORDS;
    if (use_userfile) {   // grab user from egg-userfile
      u = get_user_by_handle(userlist, text);
      if (u) {
        who = nmalloc(strlen(u->handle) + 1);
        strcpy(who, u->handle);
      } else {
        // oops, user not found!
        setslnick(text);
        if (quietstats(dest)) {
          dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
        } else {
          dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
        }
        // no need to free anything, we didn't allocate anything, yet
        Assert(!who);
        return;
      }
    } else {  // grab user from stats-userbase
      // first check if nick is on chan (faster)
      m = nick2suser(text, channel);
      if (m && m->user) {
        who = nmalloc(strlen(m->user->user) + 1);
        strcpy(who, m->user->user);
      } else {
        // no success, yet? ok, then search through the complete database...
        su = findsuser_by_name(text);
        if (su) {
          who = nmalloc(strlen(su->user) + 1);
          strcpy(who, su->user);
        } else {
          // still no success? Poor user... serve him an error.
          setslnick(text);
          if (quietstats(dest)) {
            dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
          } else {
            dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
          }
          Assert(!who);
          return;
        }
      }
    }
  }
  if (!gs) {
    Assert(who);
    nfree(who);
    debug1("Stats.mod: Couldn't exec !place, I don't have any statistics in %s.", channel);
    return;
  }
  // sort the stats
  sortstats(gs, itype, today);
  // if itype is < 0, get the modified itype that we need for accessing the data
  if (itype < 0)
    itype = (TOTAL_TYPES - 1) + (itype * -1);
  // now calculate the place
  for (ls = gs->slocal[today][itype]; ls; ls = ls->snext[today][itype]) {
    if (!listsuser(ls, channel))
      continue;
    place++;
    if (!rfc_casecmp(who, ls->user))
      break;
  }
  // if ls is NULL now, then the user doesn't have any stats in this chan
  if (!ls) {
    setslnick(who);
    if (quietstats(dest)) {
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
    } else {
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
    }
  } else {
    // stats found, so let's output them
    setslnick(who);
    slglobint = place;
    // there are 4 slang-entries for today, this week, etc
    slang = getslang(1012 + today);
    if (quietstats(dest)) {
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, slang);
    } else {
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, slang);
    }
  }
  Assert(who);
  nfree(who);
  Context;
}

static int pub_stat(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! stat %s", nick, hand, text);
  tell_stat(nick, channel, hand, channel, text, S_TOTAL);
  Context;
  return 1;
}

static int pub_tstat(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! tstat %s", nick, hand, text);
  tell_stat(nick, channel, hand, channel, text, S_TODAY);
  Context;
  return 1;
}

static void tell_stat(char *nick, char *dest, char *hand, char *channel, char *text, int today)
{
  locstats *ls;
  char *who, *tosend, what[128], *pwhat, *type, *dur, *stmp;
  int itype, first;
  struct stats_memberlist *m;
  struct userrec *u;
  struct stats_userlist *su;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif

  Context;
  if (stat_flood())
    return;
  who = tosend = type = dur = NULL;
  setslglobs(channel, 0, 0, 0);
#if EGG_IS_MIN_VER(10500)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  if (text[0] == 0) {
    if (use_userfile) {
      who = nmalloc(strlen(hand) + 1);
      strcpy(who, hand);
    } else {
      m = nick2suser(nick, channel);
      if (m && m->user) {
        who = nmalloc(strlen(m->user->user) + 1);
        strcpy(who, m->user->user);
      } else {
        who = nmalloc(2);
        strcpy(who, "*");
      }
    }
  } else {
    if (use_userfile) {   // grab user from egg-userfile
      u = get_user_by_handle(userlist, text);
      if (u) {
        who = nmalloc(strlen(u->handle) + 1);
        strcpy(who, u->handle);
      } else {
        // oops, user not found!
        setslnick(text);
        if (quietstats(dest)) {
          dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
        } else {
          dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
        }
        // no need to free anything, we didn't allocate anything, yet
        Assert(!who);
        return;
      }
    } else {  // grab user from stats-userbase
      // first check if nick is on chan (faster)
      m = nick2suser(text, channel);
      if (m && m->user) {
        who = nmalloc(strlen(m->user->user) + 1);
        strcpy(who, m->user->user);
      } else {
        // no success, yet? ok, then search through the complete database...
        su = findsuser_by_name(text);
        if (su) {
          who = nmalloc(strlen(su->user) + 1);
          strcpy(who, su->user);
        } else {
          // still no success? Poor user... serve him an error.
          setslnick(text);
          if (quietstats(dest)) {
            dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
          } else {
            dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
          }
          Assert(!who);
          return;
        }
      }
    }
  }
  setslglobs(channel, 0, 0, 0);
  if (who[0] == '*') {
    setslnick(nick);
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTYOU);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTYOU);
    nfree(who);
    return;
  }
  ls = findlocstats(channel, who);
  if (!ls) {
    setslnick(who);
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
  } else {
    if (quietstats(dest)) {
      tosend = nmalloc(14 + strlen(nick) + strlen(who) + 1);
      sprintf(tosend, "NOTICE %s :%s:", nick, who);
    } else {
      tosend = nmalloc(15 + strlen(dest) + strlen(who) + 1);
      sprintf(tosend, "PRIVMSG %s :%s:", dest, who);
    }
    what[0] = 0;
    pwhat = what;
    strncpy(pwhat, stat_reply, 127);
    pwhat[127] = 0;
    first = 1;
    while (strlen(pwhat) > 0) {
      type = newsplit(&pwhat);
      itype = typetoi(type);
      if ((itype < 0) && (itype != T_WPL) && (itype != T_IDLE))
        continue;
      if (!first) {
        // if this isn't the first run, attach a "," to the string to
        // seperate the values.
        tosend = nrealloc(tosend, strlen(tosend) + 1 + 1);
        strcat(tosend, ",");
      } else {
        first = 0;
      }
      if (itype == T_MINUTES) {
        dur = stats_duration(ls->values[today][T_MINUTES] * 60);
        stmp = getslangtype("minutes");
        tosend = nrealloc(tosend, strlen(tosend) + 3 + strlen(stmp) + strlen(dur) + 1);
        sprintf(tosend, "%s %s: %s", tosend, stmp, dur);
      } else if (itype >= 0) {
        // same as usual: use 10 bytes for the integer-string
        stmp = getslangtype(type);
        tosend = nrealloc(tosend, 8 + strlen(tosend) + 10 + strlen(stmp));
        sprintf(tosend, "%s %ld %s", tosend, ls->values[today][itype], stmp);
      } else if ((itype == T_WPL) && (ls->values[today][T_LINES] != 0)) {
        stmp = getslangtype("wpl");
        tosend = nrealloc(tosend, strlen(tosend) + 2 + 10 + strlen(stmp) + 1);
        sprintf(tosend, "%s %.2f %s", tosend, (float) ls->values[today][T_WORDS] / (float) ls->values[today][T_LINES], stmp);
      } else if ((itype == T_IDLE) && (ls->values[today][T_LINES] != 0)) {
        stmp = getslangtype("idle");
        tosend = nrealloc(tosend, strlen(tosend) + 2 + 10 + strlen(stmp) + 1);
        sprintf(tosend, "%s %.2f %s", tosend, (float) ls->values[today][T_MINUTES] / (float) ls->values[today][T_LINES], stmp);
      }
    }
    dprintf(DP_HELP, "%s.\n", tosend);
    nfree(tosend);
  }
  nfree(who);
  Context;
}

static int pub_wordstats(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! wordstats %s", nick, hand, text);
  tell_wordstats(nick, channel, hand, channel, text);
  Context;
  return 0;
}

static void tell_wordstats(char *nick, char *dest, char *hand, char *channel, char *text)
{
  locstats *ls;
  wordstats *ws;
  char *who, *tosend, *slang;
  int i;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif


  Context;
  filt(text);
  if (stat_flood())
    return;
#if EGG_IS_MIN_VER(10500)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  if (text[0] == 0) {
    who = nmalloc(strlen(hand) + 1);
    strcpy(who, hand);
  } else {
    who = nmalloc(strlen(text) + 1);
    strcpy(who, text);
  }
  setslglobs(channel, 0, 0, 0);
  if (who[0] == '*') {
    setslnick(nick);
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTYOU);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTYOU);
    nfree(who);
    return;
  }
  ls = findlocstats(channel, who);
  if (!ls) {
    setslnick(who);
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
  } else {
    slgloblocstats = ls;
    if (!ls->words) {
      setslnick(who);
      if (quietstats(dest))
        dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOWORDSTATS);
      else
        dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOWORDSTATS);
    } else {
      slang = SLUSERSMOSTUSEDWORDS;
      if (quietstats(dest)) {
        tosend = nmalloc(13 + strlen(nick) + strlen(slang) + 1);
        sprintf(tosend, "NOTICE %s :%s", nick, slang);
      } else {
        tosend = nmalloc(14 + strlen(dest) + strlen(slang) + 1);
        sprintf(tosend, "PRIVMSG %s :%s", dest, slang);
      }
      i = 0;
      sortwordstats(ls, NULL);
      ws = ls->words;
      while (ws && (i < 10)) {
        i++;
        tosend = nrealloc(tosend, 14 + strlen(tosend) + 5 + strlen(ws->word) + 10 + 1);
        sprintf(tosend, "%s %d. %s (%d)", tosend, i, ws->word, ws->nr);
        ws = ws->next;
      }
      dprintf(DP_HELP, "%s\n", tosend);
      nfree(tosend);
    }
  }
  nfree(who);
  Context;
}

static int pub_topwords(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  Context;
  if (nopubstats(channel))
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! topwords %s", nick, hand, text);
  tell_topwords(nick, channel, hand, channel);
  Context;
  return 0;
}

static void tell_topwords(char *nick, char *dest, char *hand, char *channel)
{
  globstats *gs;
  wordstats *ws;
  char *tosend, *slang;
  int i;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif

  Context;
  if (stat_flood())
    return;
#if EGG_IS_MIN_VER(10500)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  gs = findglobstats(channel);
  if (!gs) {
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :I don't have any stats in %s\n", nick, channel);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s, I don't have any stats in %s\n", dest, nick, channel);
    return;
  }
  do_globwordstats(gs);
  ws = gs->words;
  if (!ws) {
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :I don't have any wordstats in %s\n", nick, channel);
    else
      dprintf(DP_HELP, "PRIVMSG %s :I don't have any wordstats in %s\n", dest, channel);
    return;
  }
  setslglobs(gs->chan, gs->peak[S_TODAY], 0, gs->started);
  setslnick(nick);
  slang = SLCHANSMOSTUSEDWORDS;
  if (quietstats(dest)) {
    tosend = nmalloc(13 + strlen(nick) + strlen(slang) + 1);
    sprintf(tosend, "NOTICE %s :%s", nick, slang);
  } else {
    tosend = nmalloc(14 + strlen(dest) + strlen(slang) + 1);
    sprintf(tosend, "PRIVMSG %s :%s", dest, slang);
  }
  i = 0;
  while (ws && (i < 10)) {
    i++;
    tosend = nrealloc(tosend, 14 + strlen(tosend) + 5 + strlen(ws->word) + 10);
    sprintf(tosend, "%s %d. %s (%d)", tosend, i, ws->word, ws->nr);
    ws = ws->next;
  }
  dprintf(DP_HELP, "%s\n", tosend);
  nfree(tosend);
  Context;
}

static int stat_flood()
{
  if (!maxstat_thr || !maxstat_time)
    return 0;
  if ((now - mstat_time) > maxstat_time) {
    mstat_time = now;
    mstat_thr = 0;
  }
  mstat_thr++;
  if (mstat_thr > maxstat_thr)
    return 1;
  return 0;
}

static cmd_t stats_pub[] =
{
  {"!top10", "", pub_top10, 0},
  {"!ttop10", "", pub_ttop10, 0},
  {"!top20", "", pub_top20, 0},
  {"!ttop20", "", pub_ttop20, 0},
  {"!place", "", pub_place, 0},
  {"!tplace", "", pub_tplace, 0},
  {"!stat", "", pub_stat, 0},
  {"!tstat", "", pub_tstat, 0},
  {"!wordstats", "", pub_wordstats, 0},
  {"!topwords", "", pub_topwords, 0},
  {0, 0, 0, 0}
};
