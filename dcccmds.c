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
  struct stats_userlist *uu1, *uu2;
  char *user1, *user2;

  Context;
  user1 = newsplit(&par);
  user2 = par;
  uu1 = findsuser_by_name(user1);
  uu2 = findsuser_by_name(user2);
  if (!user1[0] || !user2[0]) {
    dprintf(idx, "Usage: .sumuser <user1> <user2>\n");
    return 0;
  }
  if (!uu1) {
    dprintf(idx, "%s isn't a valid user!\n", user1);
    return 0;
  }
  if (!uu2) {
    dprintf(idx, "%s isn't a valid user!\n", user2);
    return 0;
  }
  user_merge(user1, user2);
  dprintf(idx, "Transferred stats from %s to %s and deleted %s\n", user2,
  	  user1, user2);
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
  struct stats_chan *chan;
  struct stats_member *m;
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
  chan = schan_find(chname);
  if (!chan) {
    if (!findchan_by_dname(chname))
      dprintf(idx, "Invalid channel: %s\n", chname);
    else
      dprintf(idx, "Channel %s is inactive\n", chname);
    return 0;
  }
  dprintf(idx, "%d users on channel:\n", schan_members_count(&chan->members));
  sprintf(spaces, "                                                ");
  spaces[len - 4] = 0;
  dprintf(idx, "NICK%s", spaces);
  spaces[len - 4] = ' ';
  spaces[HANDLEN - 4] = 0;
  dprintf(idx, "  USER%s", spaces);
  dprintf(idx, "  UHOST\n");
  spaces[HANDLEN - 4] = ' ';
  for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
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
  glob_slang = slang_find(coreslangs, default_slang);
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
  dprintf(idx, "flags: %clist %caddhosts %cnostats\n",
  		suser_list(u) ? '+' : '-',
  		suser_addhosts(u) ? '+' : '-',
  		suser_nostats(u) ? '+' : '-');
  dprintf(idx, "Age: %s\n", stats_duration((now - u->created), 3));
  dprintf(idx, "Allowed inactivity: %s\n", stats_duration(TIMETOLIVE(u), 3));
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
  saddhost(u, host, now, now);
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
  u = addsuser(suser, now, now);
  if (host[0]) {
    saddhost(u, host, now, now);
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
  putlog(LOG_MISC, "Deleted stats user %s.", u->user);
  delsuser(suser);
  update_schannel_members();
  return 0;
}

static int cmd_schattr(struct userrec *user, int idx, char *par)
{
  struct stats_userlist *u;
  char *suser, *mode;

  Context;
  suser = newsplit(&par);
  if (!suser[0] || !par[0]) {
    dprintf(idx, "Usage: .schattr <statuser> <+/-list +/-addhosts +/-nostats>\n");
    return 0;
  }
  putlog(LOG_CMDS, "*", "#%s# schattr %s %s", dcc[idx].nick, suser, par);
  u = findsuser_by_name(suser);
  if (!u) {
    dprintf(idx, "No such user.\n");
    return 0;
  }
  while (par[0]) {
    mode = newsplit(&par);
    if (!user_changeflag(u, mode))
      dprintf(idx, "Unknown mode \"%s\" ignored.\n", mode);
  }
  dprintf(idx, "New settings for %s: %clist %caddhosts %cnostats\n", u->user,
          suser_list(u) ? '+' : '-', suser_addhosts(u) ? '+' : '-',
          suser_nostats(u) ? '+' : '-');
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
  int list, addhosts, nostats, match, matches, deb;
  struct stats_userlist *u;
  struct stats_hostlist *h;

  Context;
  mask = NULL;
  list = addhosts = nostats = -1;
  putlog(LOG_CMDS, "*", "#%s# smatch %s", dcc[idx].nick, par);
  while (par[0]) {
    tmp = newsplit(&par);
    if ((tmp[0] == '+') || (tmp[0] == '-')) {
      if (!strcasecmp(tmp + 1, "addhosts"))
        addhosts = (tmp[0] == '+');
      else if (!strcasecmp(tmp + 1, "list"))
        list = (tmp[0] == '+');
      else if (!strcasecmp(tmp + 1, "nostats"))
        nostats = (tmp[0] == '+');
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
  if (!mask && (list == -1) && (addhosts == -1) && (nostats == -1)) {
    dprintf(idx, "Usage: .smatch [hostmask|nickmask] [+|-addhosts] [+|-list]\n");
    return 0;
  }
  debug3("mask: %s, list: %d, addhosts: %d", mask, list, addhosts);
  matches = deb =  0;
  for (u = suserlist; u; u = u->next) {
    deb++;
    match = 0;
    if (list && (list != -1) && !suser_list(u))
      continue;
    if (!list && suser_list(u))
      continue;
    if (addhosts && (addhosts != -1) && !suser_addhosts(u))
      continue;
    if (!addhosts && suser_addhosts(u))
      continue;
    if (nostats && (nostats != -1) && !suser_nostats(u))
      continue;
    if (!nostats && suser_nostats(u))
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
