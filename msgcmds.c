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

static int msg_place(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan, *reply;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_nick = nick;
  chan = newsplit(&rest);
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! place %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2034));
    return 1;
  }
  reply = tell_place(nick, u ? u->handle : "*", chan, rest);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, reply);
  return 1;
}

static int msg_stat(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_nick = nick;
  chan = newsplit(&rest);
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2036));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! stat %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, tell_stat(nick, chan, rest));
  return 1;
}

static int msg_top(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan, *toptext;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_nick = nick;
  chan = newsplit(&rest);
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2036));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! top %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  toptext = tell_ntop(chan, rest, 0);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, toptext ? toptext : "ERROR");
  return 1;
}

static int msg_last(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan, *toptext;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  reset_global_vars();
  glob_nick = nick;
  chan = newsplit(&rest);
  glob_slang = slang_find(coreslangs, slang_chanlang_get(chanlangs, chan));
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2036));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! last %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  toptext = tell_ntop(chan, rest, 1);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, toptext ? toptext : "ERROR");
  return 1;
}

static int msg_wordstats(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  glob_nick = nick;
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
  // check for flood
  if (stat_flood())
    return 1;
  chan = newsplit(&rest);
  if (!validchan(chan)) {
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(2039));
    return 1;
  }
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! topwords %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  tell_topwords(nick, nick, u ? u->handle : "*", chan);
  return 1;
}

static int msg_statspass(char *nick, char *uhost, struct userrec *user, char *rest)
{
  struct stats_userlist *u;
  char *pass, *host;

  putlog(LOG_CMDS, "*", "(%s!%s) !%s! STATSPASS %s", nick, uhost, user ? user->handle : "*", rest);
  reset_global_vars();
  glob_slang = slang_getbynick(coreslangs, nick);
  glob_nick = nick;
  if (user)
    u = findsuser_by_name(user->handle);
  else {
    host = nmalloc(strlen(nick) + 1 + strlen(uhost) + 1);
    sprintf(host, "%s!%s", nick, uhost);
    u = findsuser(host);
    nfree(host);
  }
  if (!u) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLDONTRECOGNIZE);
    return 1;
  }
  glob_user = u;
  if (u->password) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLPASSALREADYSET);
    return 1;
  }
  pass = newsplit(&rest);
  if (!pass[0]) {
    dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLPASSUSAGE);
    return 1;
  }
  setpassword(u, pass);
  dprintf(DP_HELP, "NOTICE %s :%s\n", nick, SLPASSSET);
  return 1;
}

static int msg_lastspoke(char *nick, char *uhost, struct userrec *u,
        char *rest)
{
  char *chan, *reply;
  globstats gs;

  Context;
  // check for flood
  if (stat_flood())
    return 1;
  chan = newsplit(&rest);
  putlog(LOG_CMDS, "*", "(%s!%s) !%s! lastspoke %s %s", nick, uhost, u ? u->handle : "*", chan, rest);
  if (!validchan(chan)) {
    globstats_init(&gs);
    gs.chan = chan;
    dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, getslang(222));
    return 1;
  }
  reply = cmd_lastspoke(chan, rest);
  dprintf(DP_HELP, "PRIVMSG %s :%s\n", nick, reply);
  return 1;
}

static cmd_t stats_msg[] =
{
  {"place", "", msg_place, 0},
  {"stat", "", msg_stat, 0},
  {"wordstats", "", msg_wordstats, 0},
  {"topwords", "", msg_topwords, 0},
  {"statspass", "", msg_statspass, 0},
  {"statpass", "", msg_statspass, 0},
  {"lastspoke", "", msg_lastspoke, 0},
  {"top", "", msg_top, 0},
  {"last", "", msg_last, 0},
  {0, 0, 0, 0}
};
