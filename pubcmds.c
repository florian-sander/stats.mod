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
  Assert(chan);
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("nopubstats", chan) || ngetudef("nostats", chan))
    return 1;
#endif
  return 0;
}

static int quietstats(char *chan)
{
  Assert(chan);
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("quietstats", chan))
    return 1;
#endif
  return 0;
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

static int pub_top(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  char *toptext;

  if (nopubstats(channel))
    return 1;
  // check for flood
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
  putlog(LOG_CMDS, channel, "<<%s>> !%s! top %s", nick, hand, text);
  toptext = tell_ntop(channel, text, 0);
  if (!toptext)
    return 1;
  if (quietstats(channel))
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, toptext);
  else
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", channel, toptext);
  return 1;
}

static int pub_last(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  char *toptext;

  if (nopubstats(channel))
    return 1;
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
  putlog(LOG_CMDS, channel, "<<%s>> !%s! top10 %s", nick, hand, text);
  toptext = tell_ntop(channel, text, 1);
  if (!toptext)
    return 1;
  if (quietstats(channel))
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, toptext);
  else
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", channel, toptext);
  return 1;
}

static char *stats_pubcmd_reply;
static char *tell_ntop(char *chan, char *params, int last)
{
  char *par, *slang, *type, *word;
  int timerange, range, tmp, sorting, itype, place, start, len, replylen;
  int active_users;
  globstats *gs;
  locstats *ls;

  if (stats_pubcmd_reply) {
    nfree(stats_pubcmd_reply);
    stats_pubcmd_reply = NULL;
  }
  par = slang = type = word = NULL;
  ls = NULL;
  gs = NULL;
  timerange = S_TOTAL;
  type = "words";
  range = 10;
  itype = T_WORDS;
  // get pointer to the stats struct
  if (!(gs = findglobstats(chan))) {
    debug1("no globstats for %s", chan);
    return NULL;
  }
  glob_globstats = gs;
  // pars through params
  while (params[0]) {
    par = newsplit(&params);
    // check if param is a timerange
    if ((tmp = get_timerange(par)) != T_ERROR)
      timerange = tmp;
    // still no match? Then lets check if the param
    // is a range (top10, top20, etc...)
    else if ((tmp = atoi(par)))
      range = tmp;
    // still no match? Uhm... ok, maybe this is a
    else if ((tmp = slangtypetoi(par)) != T_ERROR) {
      itype = tmp;
      if (itype == T_WORD) {
        word = newsplit(&params);
      }
      type = par;
    } else
      debug1("Unknown parameter: %s", par);
  }
  // init language stuff
  glob_sorting = itype;
  glob_range = range;
  glob_timerange = timerange;

  // use special function if user requested sorting by a special word
  if (glob_sorting == T_WORD)
    return tell_top_word(chan, word, range, gs);

  // now sort the stats
  sortstats(gs, itype, timerange);
  active_users = countactivestatmembers(gs, 0, timerange,
                        (itype >= 0) ? itype : T_LINES, 1);
  // now get the "entry phrase" for the topX
  if (!last)
    slang = getslang(100 + timerange);
  else
    slang = getslang(110 + timerange);
  // now allocate memory and sprintf the start of the repy into it
  stats_pubcmd_reply = nmalloc(strlen(slang) + 1);
  strcpy(stats_pubcmd_reply, slang);
  // if we sorted by a special type, transform the sorting index to
  // a usable index to access the stat-array
  if (itype < 0)
    sorting = (itype * -1) + (TOTAL_TYPES - 1);
  else
    sorting = itype;
  // if we want to show the last users, then adjust the range
  if (last)
    range = active_users - range + 10;
  start = range - 10;
  if (start < 1)
    start = 1;
  place = 0;
  for (ls = gs->slocal[timerange][sorting]; ls; ls = ls->snext[timerange][sorting]) {
    place++;
    if (place > range)
      break;
    if (itype >= 0)
      if (!ls->values[timerange][itype])
        break;
    if (place >= start) {
      replylen = strlen(stats_pubcmd_reply);
      len = 3 + strlen(ls->user) + 10 + 6 + 1;
      stats_pubcmd_reply = nrealloc(stats_pubcmd_reply, replylen + len);
      if (itype >= 0)
        snprintf(stats_pubcmd_reply + replylen, len, " %d. %s (%ld)", place, ls->user,
                 ls->values[timerange][itype]);
      else if (itype == T_WPL)
        snprintf(stats_pubcmd_reply + replylen, len, " %d. %s (%.2f)", place, ls->user,
                 ls->values[timerange][T_WORDS] ?
                 (ls->values[timerange][T_WORDS] / ls->values[timerange][T_LINES]) :
                 0.0);
      else if (itype == T_IDLE)
        snprintf(stats_pubcmd_reply + replylen, len, " %d. %s (%.2f)", place, ls->user,
                 ls->values[timerange][T_LINES] ?
                 (ls->values[timerange][T_MINUTES] / ls->values[timerange][T_LINES]) :
                 0.0);
      else
        snprintf(stats_pubcmd_reply + replylen, len, " %d. %s (ERROR)", place, ls->user);
    }
  }
  stats_pubcmd_reply = nrealloc(stats_pubcmd_reply, strlen(stats_pubcmd_reply) + 1);
  return stats_pubcmd_reply;
}

static char *tell_top_word(char *chan, char *word, int range, globstats *gs)
{
  locstats *ls;
  char buf[100], *slang;
  int num, itype;

  Context;
  Assert(!stats_pubcmd_reply);

  word = newsplit(&word);
  strlower(word);
  setword(gs, word);
  glob_word = word;

  sortstats(gs, T_WORD, 1);

  if (range > 0)
    slang = getslang(120); // "Top <?range?>("<?word?>"):"
  else
    slang = getslang(130); // Last <?range?>("<?word?>"):
  stats_pubcmd_reply = nmalloc(strlen(slang) + 1);
  strcpy(stats_pubcmd_reply, slang);
  itype = (T_WORD * (-1)) + TOTAL_TYPES - 1;

  if (range < 0) {
    num = countactivestatmembers_by_word(gs, 1, 1);
    range = num - (range * (-1));
    if (range < 0)
      range = num;
  }

  glob_place = 0;
  for (ls = gs->slocal[S_TODAY][itype]; ls; ls = ls->snext[S_TODAY][itype]) {
    if (!listsuser(ls, chan))
      continue;
    glob_place++;
    if ((glob_place > range) || !ls->word) {
      break;
    }
    if (glob_place >= range - 10) {
      snprintf(buf, sizeof(buf), " %d. %s(%d)", glob_place, ls->user, ls->word->nr);
      stats_pubcmd_reply = nrealloc(stats_pubcmd_reply, strlen(stats_pubcmd_reply) + strlen(buf) + 1);
      strcat(stats_pubcmd_reply, buf);
    }
  }
  Context;
  return stats_pubcmd_reply;
}

static int pub_place(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  char *reply;

  Context;
  if (nopubstats(channel))
    return 1;
  if (stat_flood())
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! place %s", nick, hand, text);
  reply = tell_place(nick, hand, channel, text);
  if (quietstats(channel)) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, reply);
  } else {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", channel, reply);
  }


  Context;
  return 1;
}

static char *tell_place(char *nick, char *hand, char *channel, char *text)
{
  globstats *gs;
  struct stats_member *m;
  struct stats_userlist *su;
  struct userrec *u;
  char *who, *slang, *par, *type;
  int place = 0;
  int itype;
  int itmp;
  int today;

  Context;
  // at first, check for flood...
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
  // ... get the global stat-struct for the channel ...
  gs = findglobstats(channel);
  if (!gs) {
    debug1("Stats.mod: Couldn't exec !place, I don't have any statistics in %s.", channel);
    return "NOSTATS!";
  }
  glob_globstats = gs;
  // .. now init vars ...
  m = NULL;
  u = NULL;
  su = NULL;
  who = slang = type = NULL;
  itype = T_ERROR;
  today = S_TOTAL;
  // parse params...
  while (text[0]) {
    par = newsplit(&text);
    // maybe par is a stat-type?
    if ((itype == T_ERROR) && ((itmp = slangtypetoi(par)) != T_ERROR))
      itype = itmp;
    // or an user in the stats-userbase?
    else if (!who && (su = findsuser_by_name(par)))
      who = su->user;
    // or at least a user in the eggdrop userbase?
    else if (!who && (u = get_user_by_handle(userlist, par)))
      who = u->handle;
    else if ((itmp = get_timerange(par)) != T_ERROR)
      today = itmp;
    // oh.. what now? We can't know if it's a wrong type or a non-existant user,
    // so let's assume it is a user and return an error
    else {
      glob_nick = par;
      return getslang(220); // "I don't have any stats about <?nick?>."
    }
  }
  // if no type was specified, just default to "words"
  if (itype == T_ERROR)
    itype = T_WORDS;
  glob_sorting = itype;
  glob_timerange = today;
  // if we still don't know whose place we should show, then just asume
  // that the triggering user wants to know his own place (how selfish.. ^_^)
  if (!who) {
    // at first, check if we already know this user by handle
    if (!(!strcasecmp(hand, "*"))) {
      who = hand;
    } else {
      // now try to get the user from the stats-userbase
      // maybe the user is on the channel and we already found
      // the matching username?
      m = getschanmember(nick, channel);
      if (m && m->user)
        who = m->user->user;
      // not? Ok, then this user got a problem...
      // we _could_ check for the user's host here and try to resolve his
      // susername this way, but I think that's not worth the effort
    }
    // still no match? Ouch! Then we probably don't have any stats about this
    // poor user...
    if (!who) {
      glob_nick = nick;
      return getslang(221); // "I don't have any stats about you."
    }
  }
  Assert(who);
  Assert(itype != T_ERROR);
  Assert(gs);
  // sort the stats
  sortstats(gs, itype, today);
  // and get the place of the user
  place = getplace(gs, today, itype, who);
  // if ls is NULL now, then the user doesn't have any stats in this chan
  if (!place) {
    glob_nick = who;
    return getslang(220); // "I don't have any stats about <?nick?>."
  } else {
    // stats found, so let's output them
    glob_nick = who;
    glob_place = place;
    // there are 4 slang-entries for today, this week, etc
    return getslang(200 + today);
  }
  Context;
}

static int pub_stat(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  char *reply;

  Context;
  if (stat_flood())
    return 0;
  if (nopubstats(channel))
    return 0;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! stat %s", nick, hand, text);
  reply = tell_stat(nick, channel, text);
  Assert(reply);
  if (quietstats(channel))
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, reply);
  else
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", channel, reply);
  Context;
  return 1;
}

static char *tell_stat(char *nick, char *channel, char *text)
{
/*
  char *who, *tosend, , , ;
  int itype, first;
  struct stats_memberlist *m;
  struct stats_userlist *su;
#if EGG_IS_MIN_VER(10500)
  struct chanset_t *chan;
#endif
*/
  char *par, *tosend, buf[50], *stmp, what[128], *pwhat, *type, *dur;
  int i, timerange, first, itype;
  struct stats_userlist *utmp, *user;
  struct stats_member *member;
  locstats *ls;

  Context;
  if (stats_pubcmd_reply) {
    nfree(stats_pubcmd_reply);
    stats_pubcmd_reply = NULL;
  }
  reset_global_vars();
  timerange = S_TOTAL;
  tosend = nmalloc(500);
  tosend[0] = 0;
  user = NULL;
  type = dur = NULL;

  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
  glob_globstats = findglobstats(channel);
  while (text[0]) {
    par = newsplit(&text);
    if ((i = get_timerange(par)) != T_ERROR)
      timerange = i;
    else if ((utmp = findsuser_by_name(par)))
      user = utmp;
    else {
      member = getschanmember(par, channel);
      if (member && member->user && !suser_nostats(member->user))
        user = member->user;
      else {
        glob_nick = par;
        nfree(tosend);
        return SLNOSTATSABOUTSOMEONE;
      }
    }
  }

  if (!user) {
    member = getschanmember(nick, channel);
    if (member && member->user && !suser_nostats(member->user))
      user = member->user;
    if (!user) {
      nfree(tosend);
      glob_nick = nick;
      return SLNOSTATSABOUTYOU;
    }
  }


  ls = findlocstats(channel, user->user);
  glob_locstats = ls;
  if (!ls) {
    glob_nick = user->user;
    nfree(tosend);
    return SLNOSTATSABOUTSOMEONE;
  } else {
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
        strncat(tosend, ",", 500);
      } else {
        first = 0;
      }
      if (itype == T_MINUTES) {
        dur = stats_duration(ls->values[timerange][T_MINUTES] * 60, 6);
        stmp = getslangtype("minutes");
        snprintf(buf, sizeof(buf), " %s: %s", stmp, dur);
        strncat(tosend, buf, 500);
      } else if (itype >= 0) {
        // same as usual: use 10 bytes for the integer-string
        stmp = getslangtype(type);
        snprintf(buf, sizeof(buf), " %ld %s", ls->values[timerange][itype], stmp);
        strncat(tosend, buf, 500);
      } else if ((itype == T_WPL) && (ls->values[timerange][T_LINES] != 0)) {
        stmp = getslangtype("wpl");
        // Thanks to Algirdas 'QQ' Kepezinskas for tracking down
        // the bug which was placed in this line.
        snprintf(buf, sizeof(buf), " %.2f %s", (float) ls->values[timerange][T_WORDS] / (float) ls->values[timerange][T_LINES], stmp);
        strncat(tosend, buf, 500);
      } else if ((itype == T_IDLE) && (ls->values[timerange][T_LINES] != 0)) {
        stmp = getslangtype("idle");
        snprintf(buf, sizeof(buf), " %.2f %s", (float) ls->values[timerange][T_MINUTES] / (float) ls->values[timerange][T_LINES], stmp);
        strncat(tosend, buf, 500);
      }
    }
  }
  i = strlen(user->user) + 2 + strlen(tosend) + 1;
  stats_pubcmd_reply= nmalloc(i);
  snprintf(stats_pubcmd_reply, i, "%s: %s", user->user, tosend);
  nfree(tosend);
  return stats_pubcmd_reply;
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
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
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
  if (who[0] == '*') {
    glob_nick = nick;
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTYOU);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTYOU);
    nfree(who);
    return;
  }
  ls = findlocstats(channel, who);
  glob_locstats = ls;
  if (!ls) {
    glob_nick = who;
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLNOSTATSABOUTSOMEONE);
    else
      dprintf(DP_HELP, "PRIVMSG %s :%s\n", dest, SLNOSTATSABOUTSOMEONE);
  } else {
    if (!ls->words) {
      glob_nick = who;
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
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, channel));
#if EGG_IS_MIN_VER(10500)
  if (!strcasecmp(channel, dest)) {
    chan = findchan_by_dname(dest);
    if (chan)
      dest = chan->name;
  }
#endif
  gs = findglobstats(channel);
  glob_globstats = gs;
  if (!gs) {
    return;
  }
  do_globwordstats(gs);
  ws = gs->words;
  if (!ws) {
    if (quietstats(dest))
      dprintf(DP_HELP, "NOTICE %s :I don't have any wordstats in %s\n", nick, SLNOCHANWORDSTATS);
    else
      dprintf(DP_HELP, "PRIVMSG %s :I don't have any wordstats in %s\n", dest, SLNOCHANWORDSTATS);
    return;
  }
  glob_nick = nick;
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

static char *cmd_lastspoke(char *chan, char *text)
{
  char *user;
  globstats gs;

  if (stats_pubcmd_reply) {
    nfree(stats_pubcmd_reply);
    stats_pubcmd_reply = NULL;
  }
  reset_global_vars();
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));

  user = newsplit(&text);
  glob_nick = user;
  glob_globstats = findglobstats(chan);
  glob_locstats = findlocstats(chan, user);
  if (!glob_locstats)  {
    if (!glob_globstats) {
      globstats_init(&gs);
      gs.chan = chan;
    }
    return getslang(220);
  }
  if (!glob_locstats->lastspoke)
    return getslang(400);
  return getslang(405);
}

static int pub_lastspoke(char *nick, char *host, char *hand,
        char *channel, char *text)
{
  char *reply;

  Context;
  if (nopubstats(channel))
    return 1;
  if (stat_flood())
    return 1;
  putlog(LOG_CMDS, channel, "<<%s>> !%s! lastspoke %s", nick, hand, text);
  reply = cmd_lastspoke(channel, text);
  if (quietstats(channel)) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, reply);
  } else {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", channel, reply);
  }
  Context;
  return 1;
}

static cmd_t stats_pub[] =
{
  {"!place", "", pub_place, 0},
  {"!stat", "", pub_stat, 0},
  {"!wordstats", "", pub_wordstats, 0},
  {"!topwords", "", pub_topwords, 0},
  {"!top", "", pub_top, 0},
  {"!last", "", pub_last, 0},
  {"!lastspoke", "", pub_lastspoke, 0},
  {0, 0, 0, 0}
};
