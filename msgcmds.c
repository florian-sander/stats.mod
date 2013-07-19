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

static int validchan(char *chan)
{
  struct chanset_t *ch;

#ifndef OLDBOT
  ch = findchan_by_dname(chan);
#else
  ch = findchan(chan);
#endif
  if (ch)
    return 1;
  return 0;
}

static int msg_top10 (char *nick, char *uhost, struct userrec *u, char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2030));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! top10 %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_top(chan, nick, nick, rest, 1, 10, 0);
  return 1;
}

static int msg_ttop10 (char *nick, char *uhost, struct userrec *u, char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2031));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! ttop10 %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_top(chan, nick, nick, rest, 1, 10, 1);
  return 1;
}

static int msg_top20 (char *nick, char *uhost, struct userrec *u, char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2032));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! top20 %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_top(chan, nick, nick, rest, 11, 20, 0);
  return 1;
}

static int msg_ttop20 (char *nick, char *uhost, struct userrec *u, char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2033));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! ttop20 %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_top(chan, nick, nick, rest, 11, 20, 1);
  return 1;
}

static int msg_place(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2034));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! place %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_place(nick, nick, u ? u->handle : "*", chan, rest, S_TOTAL);
  return 1;
}

static int msg_tplace(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2035));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! tplace %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_place(nick, nick, u ? u->handle : "*", chan, rest, S_TODAY);
  return 1;
}

static int msg_stat(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2036));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! stat %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_stat(nick, nick, u ? u->handle : "*", chan, rest, S_TOTAL);
  return 1;
}

static int msg_tstat(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2037));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! tstat %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_stat(nick, nick, u ? u->handle : "*", chan, rest, S_TODAY);
  return 1;
}

static int msg_wordstats(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2038));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! wordstats %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_wordstats(nick, nick, u ? u->handle : "*", chan, rest);
  return 1;
}

static int msg_topwords(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2039));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! topwords %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_topwords(nick, nick, u ? u->handle : "*", chan);
  return 1;
}

static int msg_setemail(char *nick, char *uhost, struct userrec *user, char *rest)
{
  struct stats_userlist *u;
  char *host;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  if (user)
    u = findsuser_by_name(user->handle);
  else {
    host = nmalloc(strlen(nick) + 1 + strlen(uhost) + 1);
    sprintf(host, "%s!%s", nick, uhost);
    u = findsuser(host);
    nfree(host);
  }
  if (!u) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLDONTRECOGNIZE);
    return 1;
  }
  if (strchr(rest, ' ')) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLEMAILUSAGE);
    return 1;
  }
  setemail(u, rest);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLSETEMAIL);
  return 1;
}

static int msg_sethomepage(char *nick, char *uhost, struct userrec *user, char *rest)
{
  struct stats_userlist *u;
  char *host, *url;

  Context;
  setslglobs("", 0, 0, 0);
  setslnick(nick);
  if (user)
    u = findsuser_by_name(user->handle);
  else {
    host = nmalloc(strlen(nick) + 1 + strlen(uhost) + 1);
    sprintf(host, "%s!%s", nick, uhost);
    u = findsuser(host);
    nfree(host);
  }
  if (!u) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLDONTRECOGNIZE);
    return 1;
  }
  if (strchr(rest, ' ')) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLHPUSAGE);
    return 1;
  }
  if (strncasecmp(rest, "http://", 7)) {
    url = nmalloc(7 + strlen(rest) + 1);
    sprintf(url, "http://%s", rest);
  } else {
    url = nmalloc(strlen(rest) + 1);
    strcpy(url, rest);
  }
  sethomepage(u, url);
  nfree(url);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, SLSETHOMEPAGE);
  return 1;
}

static cmd_t stats_msg[] =
{
  {"top10", "", msg_top10, 0},
  {"ttop10", "", msg_ttop10, 0},
  {"top20", "", msg_top20, 0},
  {"ttop20", "", msg_ttop20, 0},
  {"place", "", msg_place, 0},
  {"tplace", "", msg_tplace, 0},
  {"stat", "", msg_stat, 0},
  {"tstat", "", msg_tstat, 0},
  {"wordstats", "", msg_wordstats, 0},
  {"topwords", "", msg_topwords, 0},
  {"setemail", "", msg_setemail, 0},
  {"sethomepage", "", msg_sethomepage, 0},
  {0, 0, 0, 0}
};
