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

static int cmd_savestats(struct userrec *u, int idx, char *par)
{
  write_stats();
  putlog(LOG_CMDS, "*", "#%s# savestats", dcc[idx].nick);
  return 0;
}

static int cmd_loadstats(struct userrec *u, int idx, char *par)
{
  read_stats();
  putlog(LOG_CMDS, "*", "#%s# loadstats", dcc[idx].nick);
  return 0;
}

static int cmd_writewebstats(struct userrec *u, int idx, char *par)
{
  Context;
#if EGG_IS_MIN_VER(10500)
  write_new_webstats();
#else
  dprintf(idx, "Sorry, this function isn't available with eggdrop1.4.\n");
  dprintf(idx, "Either update your bot to eggdrop1.5+ or use livestats instead of static webfiles.\n");
  dprintf(idx, "(or even better: do both! <g>)\n");
#endif
  putlog(LOG_CMDS, "*", "#%s# writewebstats %s", dcc[idx].nick, par);
  Context;
  return 0;
}

static int cmd_purgestats(struct userrec *u, int idx, char *par)
{
  Context;
  deloldstatusers();
  Context;
  purgestats();
  Context;
  update_schannel_members();
  Context;
  putlog(LOG_CMDS, "*", "#%s# purgestats", dcc[idx].nick);
  Context;
  return 0;
}

static int cmd_sumuser(struct userrec *user, int idx, char *par)
{
  struct userrec *u1, *u2;
  struct stats_userlist *uu1, *uu2;
  struct stats_hostlist *h;
  char *user1, *user2, *hand1, *hand2;
  globstats *gs;
  locstats *ls;
  int i;
  struct list_type *q;

  Context;
  user1 = newsplit(&par);
  user2 = par;
  u1 = get_user_by_handle(userlist, user1);
  u2 = get_user_by_handle(userlist, user2);
  uu1 = findsuser_by_name(user1);
  uu2 = findsuser_by_name(user2);
  if (!user1[0] || !user2[0]) {
    dprintf(idx, "Usage: .sumuser <user1> <user2>\n");
    return 0;
  }
  if (use_userfile && !u1) {
    dprintf(idx, "%s isn't a valid user!\n", user1);
    return 0;
  }
  if (use_userfile && !u2) {
    dprintf(idx, "%s isn't a valid user!\n", user2);
    return 0;
  }
  if (!use_userfile && !uu1) {
    dprintf(idx, "%s isn't a valid user!\n", user1);
    return 0;
  }
  if (!use_userfile && !uu2) {
    dprintf(idx, "%s isn't a valid user!\n", user2);
    return 0;
  }
  if (use_userfile) {
    hand1 = u1->handle;
    hand2 = u2->handle;
  } else {
    hand1 = uu1->user;
    hand2 = uu2->user;
  }
  for (gs = sdata; gs; gs = gs->next) {
    for (ls = gs->local; ls; ls = ls->next) {
      if (!strcasecmp(ls->user, hand2)) {
        for (i = 0; i < TOTAL_TYPES; i++) {
          incrstats(hand1, gs->chan, i, ls->values[S_TOTAL][i], (S_TOTAL + 1) * (-1));
          incrstats(hand1, gs->chan, i, ls->values[S_DAILY][i], (S_DAILY + 1) * (-1));
          incrstats(hand1, gs->chan, i, ls->values[S_WEEKLY][i], (S_WEEKLY + 1) * (-1));
          incrstats(hand1, gs->chan, i, ls->values[S_MONTHLY][i], (S_MONTHLY + 1) * (-1));
        }
        if (getstats(hand1, "", "gstarted", 0)
            < getstats(hand2, "", "gstarted", 0))
          incrstats(hand1, "", T_LSTARTED, getstats(hand2, "", "gstarted", 0), 1);
      }
    }
  }
  if (use_userfile) {
    for (q = get_user(&USERENTRY_HOSTS, u2); q; q = q->next)
      addhost_by_handle(u1->handle, q->extra);
    deluser(u2->handle);
  } else {
    for (h = uu2->hosts; h; h = h->next)
      saddhost(uu1, h->mask, h->lastused);
    delsuser(uu2->user);
  }
  dprintf(idx, "Transferred stats from %s to %s and deleted %s\n", user2,
  	  hand1, user2);
  putlog(LOG_CMDS, "*", "#%s# sumuser %s %s", dcc[idx].nick, user1, user2);
  update_schannel_members();
  Context;
  return 0;
}

static int cmd_resetuser(struct userrec *u, int idx, char *par)
{
  char *user, *chan;
  locstats *ls;

  Context;
  user = newsplit(&par);
  chan = newsplit(&par);
  ls = findlocstats(chan, user);
  if (!ls) {
    dprintf(idx, "Couldn't find stats for %s in %s.\n", user, chan);
    return 0;
  }
  resetlocstats(ls);
  dprintf(idx, "%s's stats set to 0 on %s.\n", ls->user, chan);
  putlog(LOG_CMDS, "*", "#%s# resetuser %s %s", dcc[idx].nick, user, chan);
  Context;
  return 0;
}

static int cmd_schannel(struct userrec *u, int idx, char *par)
{
  char *chname, spaces[50], *user, ubuf[2];
  struct stats_chanset *chan;
  struct stats_memberlist *m;
  int len = 0;

  Context;
  ubuf[0] = '*';
  ubuf[1] = 0;
#if EGG_IS_MIN_VER(10500)
  len = nick_len;
#else
  len = NICKLEN;
#endif
  chname = newsplit(&par);
  putlog(LOG_CMDS, "*", "#%s# (%s) schannel %s", dcc[idx].nick,
	 dcc[idx].u.chat->con_chan, chname);
  if (!chname[0])
    chname = dcc[idx].u.chat->con_chan;
  chan = findschan(chname);
  if (!chan) {
    if (!findchan_by_dname(chname))
      dprintf(idx, "Invalid channel: %s\n", chname);
    else
      dprintf(idx, "Channel %s is inactive\n", chname);
    return 0;
  }
  dprintf(idx, "%d users on channel:\n", countmembers(chan->members));
  sprintf(spaces, "                                                ");
  spaces[len - 4] = 0;
  dprintf(idx, "NICK%s", spaces);
  spaces[len - 4] = ' ';
  spaces[HANDLEN - 4] = 0;
  dprintf(idx, "  USER%s", spaces);
  dprintf(idx, "  UHOST\n");
  spaces[HANDLEN - 4] = ' ';
  for (m = chan->members; m; m = m->next) {
    if (m->user)
      user = m->user->user;
    else
      user = ubuf;
    spaces[len - strlen(m->nick)] = 0;
    dprintf(idx, "%s%s", m->nick, spaces);
    spaces[len - strlen(m->nick)] = ' ';
    spaces[HANDLEN - strlen(user)] = 0;
    dprintf(idx, "  %s%s", user, spaces);
    spaces[HANDLEN - strlen(user)] = ' ';
    dprintf(idx, "  %s\n", m->uhost);
  }
  Context;
  return 0;
}

static int cmd_swhois(struct userrec *user, int idx, char *par)
{
  struct stats_userlist *u;

  Context;
  if (!par[0]) {
    dprintf(idx, "Usage: .swhois <stats-user>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# swhois %s", dcc[idx].nick, par);
  u = findsuser_by_name(par);
  if (!u) {
    dprintf(idx, "No such user %s.\n", par);
    return 0;
  }
  dump_suser(idx, u);
  return 0;
}

static void dump_suser(int idx, struct stats_userlist *u)
{
  struct stats_hostlist *h;
  struct userrec *uu;

  dprintf(idx, "------ %s ------\n", u->user);
  dprintf(idx, "flags: %clist %caddhosts\n", u->list ? '+' : '-', u->addhosts ? '+' : '-');
  if (u->hosts) {
    dprintf(idx, "Hosts: %s\n", u->hosts->mask);
    for (h = u->hosts->next; h; h = h->next)
      dprintf(idx, "       %s\n", h->mask);
  }
  uu = get_user_by_handle(userlist, u->user);
  if (uu)
    dprintf(idx, "This user also exists in the eggdrop-userfile. There might be additional hosts.\n");
}

static int cmd_pls_shost(struct userrec *user, int idx, char *par)
{
  char *suser, *host;
  struct stats_userlist *u;

  Context;
  suser = newsplit(&par);
  host = newsplit(&par);
  if (!suser[0] || !host[0]) {
    dprintf(idx, "Usage: .+shost <statuser> <host>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# +shost %s %s", dcc[idx].nick, suser, host);
  u = findsuser_by_name(suser);
  if (!u) {
    dprintf(idx, "No such user: %s\n", suser);
    return 0;
  }
  saddhost(u, host, now);
  dprintf(idx, "Added hostmask %s to %s.\n", host, u->user);
  update_schannel_members();
  return 0;
}

static int cmd_mns_shost(struct userrec *user, int idx, char *par)
{
  char *suser, *host;
  struct stats_userlist *u;

  Context;
  suser = newsplit(&par);
  host = newsplit(&par);
  if (!suser[0] || !host[0]) {
    dprintf(idx, "Usage: .-shost <statuser> <host>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# -shost %s %s", dcc[idx].nick, suser, host);
  u = findsuser_by_name(suser);
  if (!u) {
    dprintf(idx, "No such user: %s\n", suser);
    return 0;
  }
  if (sdelhost(u, host)) {
    dprintf(idx, "Removed hostmask %s from %s.\n", host, u->user);
    update_schannel_members();
  } else
    dprintf(idx, "Couldn't remove hostmask %s from %s: no such mask!\n", host, u->user);
  update_schannel_members();
  return 0;
}

static int cmd_pls_suser(struct userrec *user, int idx, char *par)
{
  struct stats_userlist *u;
  char *suser, *host;

  Context;
  suser = newsplit(&par);
  host = newsplit(&par);
  if (!suser[0]) {
    dprintf(idx, "Usage: .+suser <statuser> [host]\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# +suser %s %s", dcc[idx].nick, suser, host);
  u = findsuser_by_name(suser);
  if (u) {
    dprintf(idx, "That statuser already exists!\n");
    return 0;
  }
  u = addsuser(suser, 1, 1);
  if (host[0]) {
    saddhost(u, host, now);
    putlog(LOG_MISC, "Added stats user %s with hostmask %s.", u->user, host);
  } else
    putlog(LOG_MISC, "Added stats user %s without a hostmask.", u->user);
  update_schannel_members();
  return 0;
}

static int cmd_mns_suser(struct userrec *user, int idx, char *par)
{
  struct stats_userlist *u;
  char *suser;

  Context;
  suser = newsplit(&par);
  if (!suser[0]) {
    dprintf(idx, "Usage: .-suser <statuser>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# -suser %s", dcc[idx].nick, suser);
  u = findsuser_by_name(suser);
  if (!u) {
    dprintf(idx, "No such user.\n");
    return 0;
  }
  delsuser(suser);
  putlog(LOG_MISC, "Deleted stats user %s.", u->user);
  update_schannel_members();
  return 0;
}

static int cmd_schattr(struct userrec *user, int idx, char *par)
{
  struct stats_userlist *u;
  char *suser, *mode1, *mode2;

  Context;
  suser = newsplit(&par);
  mode1 = newsplit(&par);
  mode2 = newsplit(&par);
  if (!suser[0] || !mode1[0]) {
    dprintf(idx, "Usage: .schattr <statuser> <+/-list +/-addhosts>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# schattr %s %s %s", dcc[idx].nick, suser, mode1, mode2);
  u = findsuser_by_name(suser);
  if (!u) {
    dprintf(idx, "No such user.\n");
    return 0;
  }
  if (!strcasecmp(mode1, "+list"))
    u->list = 1;
  else if (!strcasecmp(mode1, "-list"))
    u->list = 0;
  if (!strcasecmp(mode1, "+addhosts"))
    u->addhosts = 1;
  else if (!strcasecmp(mode1, "-addhosts"))
    u->addhosts = 0;
  dprintf(idx, "New settings for %s: %clist %caddhosts\n", u->user,
          u->list ? '+' : '-', u->addhosts ? '+' : '-');
  return 0;
}

static int cmd_updateschans(struct userrec *user, int idx, char *par)
{
  Context;
  putlog(LOG_CMDS, "*", "#%s# updateschans", dcc[idx].nick);
  update_schannel_members();
  return 0;
}

static int cmd_smatch(struct userrec *user, int idx, char *par)
{
  char *tmp, *mask;
  int list, addhosts, match, matches, deb;
  struct stats_userlist *u;
  struct stats_hostlist *h;

  Context;
  mask = NULL;
  list = addhosts = -1;
  putlog(LOG_CMDS, "*", "#%s# smatch %s", dcc[idx].nick, par);
  while (par[0]) {
    tmp = newsplit(&par);
    if ((tmp[0] == '+') || (tmp[0] == '-')) {
      if (!strcasecmp(tmp + 1, "addhosts"))
        addhosts = (tmp[0] == '+');
      else if (!strcasecmp(tmp + 1, "list"))
        list = (tmp[0] == '+');
      else {
        dprintf(idx, "Unknown stats-user flag: %s\n", tmp);
        return 0;
      }
    } else {
      if (mask) {
	dprintf(idx, "Usage: .smatch [hostmask|nickmask] [+|-addhosts] [+|-list]\n");
	return 0;
      }
      mask = tmp;
    }
  }
  if (!mask && (list == -1) && (addhosts == -1)) {
    dprintf(idx, "Usage: .smatch [hostmask|nickmask] [+|-addhosts] [+|-list]\n");
    return 0;
  }
  debug3("mask: %s, list: %d, addhosts: %d", mask, list, addhosts);
  matches = deb =  0;
  for (u = suserlist; u; u = u->next) {
    deb++;
    match = 0;
    if ((list != (-1)) && (u->list != list))
      continue;
    if ((addhosts != (-1)) && (u->addhosts != addhosts))
      continue;
    if (mask) {
      if (wild_match(mask, u->user)) {
        match = 1;
      } else {
	for (h = u->hosts; h && !match; h = h->next) {
	  if (wild_match(mask, h->mask)) {
	    match = 1;
	  }
	}
      }
    } else
      match = 1;
    if (match) {
      matches++;
      if (matches > 20) {
        dprintf(idx, "More than 20 matches. Truncated.\n");
        break;
      } else
        dump_suser(idx, u);
    }
  }
  if (!matches)
    dprintf(idx, "No matches!\n");
  else if (matches <= 20)
    dprintf(idx, "===\n%d matches found.\n", matches);
  debug2("%d matches, %d cycles", matches, deb);
  return 0;
}

static int cmd_chsusername(struct userrec *user, int idx, char *par)
{
  char *oldnick, *newnick;
  struct stats_userlist *u;

  Context;
  putlog(LOG_CMDS, "*", "#%s# chsusername %s", dcc[idx].nick, par);
  oldnick = newsplit(&par);
  newnick = newsplit(&par);
  if (!newnick[0] || par[0]) {
    dprintf(idx, "Usage: .chsusername old_user_name new_user_name\n");
    return 0;
  }
  u = findsuser_by_name(oldnick);
  if (!u) {
    dprintf(idx, "No such user: %s\n", oldnick);
    return 0;
  }
  u = findsuser_by_name(newnick);
  if (u && strcasecmp(oldnick, newnick)) {
    dprintf(idx, "User %s already exists!\n", newnick);
    return 0;
  }
  if (get_user_by_handle(userlist, oldnick)) {
    dprintf(idx, "%s exists in the eggdrop userfile. Please use .chnick or "
            ".chhand instead to keep the stats synched.\n");
    return 0;
  }
  track_stat_user(oldnick, newnick);
  update_schannel_members();
  return 0;
}

static cmd_t mydcc[] =
{
  {"savestats", "m", cmd_savestats, NULL},
  {"loadstats", "m", cmd_loadstats, NULL},
  {"writewebstats", "m|-", cmd_writewebstats, NULL},
  {"purgestats", "m|-", cmd_purgestats, NULL},
  {"sumuser", "n|-", cmd_sumuser, NULL},
  {"resetuser", "m|-", cmd_resetuser, NULL},
  {"schannel", "o|o", cmd_schannel, NULL},
  {"swhois", "o|o", cmd_swhois, NULL},
  {"+shost", "m|-", cmd_pls_shost, NULL},
  {"-shost", "m|-", cmd_mns_shost, NULL},
  {"+suser", "m|-", cmd_pls_suser, NULL},
  {"-suser", "m|-", cmd_mns_suser, NULL},
  {"schattr", "m|-", cmd_schattr, NULL},
  {"updateschans", "-|-", cmd_updateschans, NULL},
  {"smatch", "m", cmd_smatch, NULL},
  {"chsusername", "m", cmd_chsusername, NULL},
  {0, 0, 0, 0}
};
