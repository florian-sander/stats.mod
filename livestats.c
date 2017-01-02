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

static struct dcc_table LIVESTATS_LISTEN =
{
  "LIVESTATS_LISTEN",
  DCT_VALIDIDX,
  eof_livestats,
  livestats_accept,
  0,
  timeout_listen_livestats,
  display_livestats_accept,
  0,
  NULL,
  0
};

static struct dcc_table LIVESTATS =
{
  "LIVESTATS",
  DCT_VALIDIDX,
  eof_livestats,
  livestats_activity,
  &livestats_timeout,
  timeout_livestats,
  display_livestats,
  expmem_livestats,
  kill_livestats,
#ifdef OLDBOT
  out_livestats,
#else
  out_livestats,
  outdone_livestats
#endif
};

static int inactivechan(char *chan)
{
  struct chanset_t *ch;

  ch = findchan_by_dname(chan);
  if (!ch)
    return 1;
  if (ch->status & CHAN_INACTIVE)
    return 1;
  return 0;
}

static int nostats(char *chan)
{
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("nostats", chan))
    return 1;
#endif
  return 0;
}

static void start_listen_livestats(int port)
{
  int i, zz;
  char tmp[50];

  stop_listen_livestats();
  sprintf(tmp, "set my-ip \"%s\";", livestats_ip);
  do_tcl("livestats-hack-start",
      "set my-ip-livestats-backup ${my-ip};"
      "set my-hostname-livestats-backup ${my-hostname};"
      "set my-hostname \"\"");
  do_tcl("livestats-hack-setip", tmp);
  zz = open_listen(&port);
  do_tcl("livestats-hack-end",
      "set my-ip ${my-ip-livestats-backup};"
      "set my-hostname ${my-hostname-livestats-backup}");
  if (zz == (-1)) {
    putlog(LOG_MISC, "*", "ERROR! Cannot open listening socket for livestats!");
    return;
  }
  if ((i = new_dcc(&LIVESTATS_LISTEN, 0)) == -1) {
    putlog(LOG_MISC, "*", "ERROR! Cannot open listening socket for livestats! DCC table is full!");
    return;
  }
  dcc[i].sock = zz;
  dcc[i].addr = (IP) (-559026163);
  dcc[i].port = port;
  strcpy(dcc[i].nick, "livestats");
  strcpy(dcc[i].host, "*");
  dcc[i].timeval = now;
  putlog(LOG_MISC, "*", "Now listening for livestats connections on port %d", port);
}

static void stop_listen_livestats()
{
  int i;

  for (i = 0; i < dcc_total; i++) {
    if (dcc[i].type == &LIVESTATS_LISTEN) {
      putlog(LOG_MISC, "*",
      	     "no longer listening for livestats connections on port %d",
             dcc[i].port);
      killsock(dcc[i].sock);
      lostdcc(i);
    } else if (dcc[i].type == &LIVESTATS) {
      putlog(LOG_MISC, "*", "killing livestats connection from %s", dcc[i].host);
      killsock(dcc[i].sock);
      lostdcc(i);
    }
  }
}

static void livestats_activity(int idx, char *buf, int len)
{
  char *cmd, *path, *newpath, *imask;
  int lev;

  imask = nmalloc(strlen(dcc[idx].host) + 13);
  sprintf(imask, "livestats!*@%s", dcc[idx].host);
  if (match_ignore(imask)) {
    debug1("Ignoring livestats access from %s", dcc[idx].host);
    stats_info_access(idx)->code = 401;
    if (livestats_ignore_msg[0])
    dprintf(idx, "HTTP/1.0 401 Access Forbidden\nContent-Type: text/html\n\n%s", livestats_ignore_msg);
    killsock(dcc[idx].sock);
    lostdcc(idx);
    nfree(imask);
    return;
  }
  nfree(imask);
  if (!strncasecmp(buf, "GET ", 4)) {
    if (!stats_info_access(idx)->cmd) {
      stats_info_access(idx)->cmd = nmalloc(strlen(buf) + 1);
      strcpy(stats_info_access(idx)->cmd, buf);
      stats_info_access(idx)->code = 200;
    }
  }
  cmd = newsplit(&buf);
  if (!strcmp(cmd, "GET")) {
    if (livestats_flood()) {
      stats_info_access(idx)->code = 401;
      killsock(dcc[idx].sock);
      lostdcc(idx);
      return;
    }
    lev = logmodes(stats_loglevel);
    if (lev)
      putlog(lev, "*", "%s: GET %s", dcc[idx].host, buf);
    path = newsplit(&buf);
    if (!strncasecmp(path, "/robots.txt", 11)) {
      dprintf(idx, "HTTP/1.1 404 Not Found\nServer: stats.mod/%s\nConnection: close", MODULE_VERSION);
      dprintf(idx, "Content-Type: text/html; charset=iso-8859-1\n\n");
      dprintf(idx, "<HTML><HEAD><TITLE>404 Not Found</TITLE></HEAD><BODY><H1>Not Found</H1>");
      dprintf(idx, "The requested document was not found on this server.</BODY></HTML>");
      stats_info_access(idx)->code = 404;
      return;
    }
    if (path[strlen(path) - 1] != '/') {
      newpath = nmalloc(strlen(path) + 2);
      strcpy(newpath, path);
      newpath[strlen(path)] = '/';
      newpath[strlen(path) + 1] = 0;
      dprintf(idx, "HTTP/1.1 301 Moved Permanently\nServer: stats.mod/%s\n", MODULE_VERSION);
      dprintf(idx, "Location: %s\nConnection: close\nContent-Type: text/html\n\n", newpath);
      dprintf(idx, "<HTML><body>The concluding \"/\" is important!<br><center>");
      dprintf(idx, "<a href=\"%s\">%s</a></center><br>", newpath, newpath);
      stats_info_access(idx)->code = 301;
      nfree(newpath);
      return;
    }
    dprintf(idx, "HTTP/1.0 200 OK\nServer: stats.mod/%s\nContent-Type: text/html\n\n", MODULE_VERSION);
    send_livestats(idx, path);
  } else if (!strcasecmp(cmd, "User-Agent:")) {
    if (stats_info_access(idx)->browser)
      return;
    stats_info_access(idx)->browser = nmalloc(strlen(buf) + 1);
    strcpy(stats_info_access(idx)->browser, buf);
  } else if (!strcasecmp(cmd, "Referer:")) {
    if (stats_info_access(idx)->referer)
      return;
    stats_info_access(idx)->referer = nmalloc(strlen(buf) + 1);
    strcpy(stats_info_access(idx)->referer, buf);
  } else if (!buf[0]) {
    dcc[idx].status = 1;
#ifndef OLDBOT
    /* If there's no data in our socket, we immediately get rid of it.
     */
    if (!sock_has_data(SOCK_DATA_OUTGOING, dcc[idx].sock)) {
      killsock(dcc[idx].sock);
      lostdcc(idx);
    }
#endif
  }
}

#ifndef OLDBOT
static void outdone_livestats(int idx)
{
  if (dcc[idx].status) {
    killsock(dcc[idx].sock);
    lostdcc(idx);
  } else
    dcc[idx].status = 1;
}
#endif

static void display_livestats(int idx, char *buf)
{
  sprintf(buf, "livestats");
}

static void display_livestats_accept(int idx, char *buf)
{
  sprintf(buf, "lstn port");
}

static void timeout_livestats(int idx)
{
  killsock(dcc[idx].sock);
  lostdcc(idx);
}

static void timeout_listen_livestats(int idx)
{
  debug0("timeout listen");
  killsock(dcc[idx].sock);
  lostdcc(idx);
}

static void kill_livestats(int idx, void *x)
{
  register struct stats_clientinfo *p = (struct stats_clientinfo *) x;
  char ts[41], test[11];
  time_t tt;
  FILE *f;

  Context;
  tt = now;
  if (!p) {
    putlog(LOG_MISC, "*", "Can't kill clientinfo, no pointer. This should not happen!");
    return;
  }
  ctime(&tt);
  /* 05/Feb/2000:12:08:17 +0100 */
  strftime(test, 19, "%z", localtime(&tt));
  if (test[0] != 'z')
    strftime(ts, 40, "%d/%b/%Y:%H:%M:%S %z", localtime(&tt));
  else
    strftime(ts, 40, "%d/%b/%Y:%H:%M:%S", localtime(&tt));
  if (livestats_log[0]) {
    f = fopen(livestats_log, "a");
    if (f == NULL)
      putlog(LOG_MISC, "*", "ERROR writing livestats log.");
    else {
      if (test[0] != 'z')
        fprintf(f,
    	 "%s - - [%s] \"%s\" %d %d \"%s\" \"%s\"\n", dcc[idx].host, ts,
	 p->cmd ? p->cmd : "", p->code, p->traffic,
	 p->referer ? p->referer : "-", p->browser ?  p->browser : "");
      else
        fprintf(f,
    	 "%s - - [%s %+05d] \"%s\" %d %d \"%s\" \"%s\"\n", dcc[idx].host,
	 ts, offset * (-1) * 100, p->cmd   ? p->cmd : "", p->code ,p->traffic,
	 p->referer ? p->referer : "-", p->browser ?  p->browser : "");
      fclose(f);
    }
  }
  if (p->browser)
    nfree(p->browser);
  if (p->referer)
    nfree(p->referer);
  if (p->cmd)
    nfree(p->cmd);
  nfree(p);
}

static int expmem_livestats(void *x)
{
  register struct stats_clientinfo *p = (struct stats_clientinfo *) x;
  int tot = sizeof(struct stats_clientinfo);

  Context;
  if (!p) {
    putlog(LOG_MISC, "*", "Can't expmem clientinfo, no pointer. This should not happen!");
    return 0;
  }
  if (p->browser)
    tot += strlen(p->browser) + 1;
  if (p->referer)
    tot += strlen(p->referer) + 1;
  if (p->cmd)
    tot += strlen(p->cmd) + 1;
  return tot;
}

static void out_livestats(int idx, char *buf, void *x)
{
  register struct stats_clientinfo *p = (struct stats_clientinfo *) x;

  if (!p) {
    putlog(LOG_MISC, "*", "No stats_clientinfo pointer. This should not happen!");
    return;
  }
  p->traffic += strlen(buf);
  tputs(dcc[idx].sock, buf, strlen(buf));
}

static void livestats_accept(int idx, char *buf, int len)
{
#if EGG_IS_MIN_VER(10800)
  int i, j = 0;
  sockname_t name;
  unsigned short port;

  Context;
  if (dcc_total + 1 >= max_dcc) {
    j = answer(dcc[idx].sock, &name, &port, 0);
    if (j != -1) {
      dprintf(-j, "Sorry, too many connections already.\r\n");
      killsock(j);
    }
    return;
  }
  if ((i = new_dcc(&LIVESTATS, sizeof(struct stats_clientinfo))) == (-1)) {
    putlog(LOG_MISC, "*", "Error accepting livestats connection. DCC table is full.");
    return;
  }
  dcc[i].sock = answer(dcc[idx].sock, &dcc[i].sockname,
                       (short unsigned *) &dcc[i].port, 0);
  if (dcc[i].sock < 0) {
    putlog(LOG_MISC, "*", "Stats.mod: Error accepting livestats connection: %s", strerror(errno));
    lostdcc(i);
    return;
  }
  strcpy(dcc[i].nick, "httpstats");
  strcpy(dcc[i].host, iptostr(&dcc[idx].sockname.addr.sa));
  dcc[i].timeval = now;
  dcc[i].status = 0;
  ((struct stats_clientinfo *) dcc[i].u.other)->traffic = 0;
  ((struct stats_clientinfo *) dcc[i].u.other)->code = 200;
  ((struct stats_clientinfo *) dcc[i].u.other)->browser = NULL;
  ((struct stats_clientinfo *) dcc[i].u.other)->referer = NULL;
  ((struct stats_clientinfo *) dcc[i].u.other)->cmd = NULL;
#else
  unsigned long ip;
  unsigned short port;
  int j = 0, sock, i;
  char s[UHOSTLEN];

  Context;
  if (dcc_total + 1 >= max_dcc) {
    j = answer(dcc[idx].sock, s, &ip, &port, 0);
    if (j != -1) {
      dprintf(-j, "Sorry, too many connections already.\r\n");
      killsock(j);
    }
    return;
  }
  sock = answer(dcc[idx].sock, s, &ip, &port, 0);
  if (sock < 0) {
    neterror(s);
    putlog(LOG_MISC, "*", "Stats.mod: Error accepting livestats connection: %s", s);
    return;
  }
  if ((i = new_dcc(&LIVESTATS, sizeof(struct stats_clientinfo))) == (-1)) {
    putlog(LOG_MISC, "*", "Error accepting livestats connection. DCC table is full.");
    killsock(sock);
    return;
  }
  dcc[i].sock = sock;
  dcc[i].addr = ip;
  dcc[i].port = port;
  strcpy(dcc[i].nick, "httpstats");
#ifndef OLDBOT
  sprintf(s, "%s", iptostr(my_htonl(ip)));
#else
  sprintf(s, "%lu.%lu.%lu.%lu", (ip >> 24) & 0xff, (ip >> 16) & 0xff,
  	  (ip >> 8) & 0xff, ip & 0xff); /* dw */
#endif
  strcpy(dcc[i].host, s);
  dcc[i].timeval = now;
  dcc[i].status = 0;
  ((struct stats_clientinfo *) dcc[i].u.other)->traffic = 0;
  ((struct stats_clientinfo *) dcc[i].u.other)->code = 200;
  ((struct stats_clientinfo *) dcc[i].u.other)->browser = NULL;
  ((struct stats_clientinfo *) dcc[i].u.other)->referer = NULL;
  ((struct stats_clientinfo *) dcc[i].u.other)->cmd = NULL;
#endif
}

static int mlstat_time = 0, mlstat_thr = 0;
static int livestats_flood()
{
  if (!maxlivestats_thr || !maxlivestats_time)
    return 0;
  if ((now - mlstat_time) > maxlivestats_time) {
    mlstat_time = now;
    mlstat_thr = 0;
  }
  mlstat_thr++;
  if (mlstat_thr > maxlivestats_thr)
    return 1;
  return 0;
}

static void eof_livestats(int idx)
{
  debug0("eof accept");
  killsock(dcc[idx].sock);
  lostdcc(idx);
}

static void send_livestats(int idx, char *buf)
{
  char *channel, *command;
  char what[512], *pwhat, *type, *stoday, *user, prefix[4];
  struct chanset_t *chan;
  int today = 0;
  globstats *gs;
  locstats *ls, *ls2;
  time_t tt, ttbuf;
  struct userrec *u;
  int itype, nr, i, ii;
  wordstats *ws;
  quotestr *qs;
  unsigned long x;
  int wert;
  float r, g, b;
  float r2, g2, b2;
  float rstep, gstep, bstep;
  struct stats_userlist *suser;

  ttbuf = now;
  tt = now;
  if (!strcmp(buf, "/")) {
    setslglobs(NULL, 0, 0, 0);
    dprintf(idx, "<html><head><title>%s</title>\n%s\n</head>\n", ROOTTITLE, SLCSS);
    dprintf(idx, "%s\n", SLBODYTAG);
    long_dprintf(idx, SLHEADER);
    dprintf(idx, "<br><br><br><table border=1 width=100%%>\n");
    for (gs = sdata; gs; gs = gs->next) {
      if (!inactivechan(gs->chan) && !secretchan(gs->chan) && !nostats(gs->chan)) {
	if (gs->chan[0] == '!')
	  strcpy(prefix, "!");
	else if (gs->chan[0] == '+')
	  strcpy(prefix, "+");
	else if (gs->chan[0] == '&')
	  strcpy(prefix, "&");
	else
	  prefix[0] = 0;
#ifndef OLDBOT
        chan = findchan_by_dname(gs->chan);
#else
        chan = findchan(gs->chan);
#endif
        i = 7;
        if (!show_userlist)
          i--;
        if (!show_usersonchan)
          i--;
        dprintf(idx, "<tr><td rowspan=%d align=center width=50%%>", i);
        if (chan && chan->channel.topic)
          dprintf(idx, "<a href=\"%s%s/\">%s</a><br><font size=-1>(%s)</font></td>",
                  prefix, gs->chan + 1, gs->chan, filt2(chan->channel.topic));
        else
          dprintf(idx, "<a href=\"%s%s/\">%s</a></td>",
                  prefix, gs->chan + 1, gs->chan);
        dprintf(idx, "<td rowspan=4 align=center>%s</td>", SLTOP);
        setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
        dprintf(idx, "<td align=center><a href=\"%s%s/top/total/words/\">%s</a></td>",
        	prefix, gs->chan + 1, SLTOTAL);
        dprintf(idx, "</tr>\n<tr>");
        dprintf(idx, "<td align=center><a href=\"%s%s/top/daily/words/\">%s</a></td>",
        	prefix, gs->chan + 1, SLDAILY);
        dprintf(idx, "</tr>\n<tr>");
        dprintf(idx, "<td align=center><a href=\"%s%s/top/weekly/words/\">%s</a></td>",
        	prefix, gs->chan + 1, SLWEEKLY);
        dprintf(idx, "</tr>\n<tr>");
        dprintf(idx, "<td align=center><a href=\"%s%s/top/monthly/words/\">%s</a></td>",
        	prefix, gs->chan + 1, SLMONTHLY);
        dprintf(idx, "</tr>\n");
        if (show_userlist)
          dprintf(idx, "<tr><td align=center colspan=2><a href=\"%s%s/users/\">%s</a></td></tr>",
        	  prefix, gs->chan + 1, SLUSERS);
        if (show_usersonchan)
          dprintf(idx, "<tr><td align=center colspan=2><a href=\"%s%s/onchan/\">%s</a></td></tr>",
                  prefix, gs->chan + 1, SLONCHAN);
        dprintf(idx, "<tr><td align=center colspan=2><a href=\"%s%s/misc/\">%s</a></td>",
        	prefix, gs->chan + 1, SLMISCSTATS);
        dprintf(idx, "</tr><tr><td colspan=3 height=0><font size=-5>&nbsp;</font></td></tr>\n");
      }
    }
    dprintf(idx, "<tr><th colspan=3 align=center>");
    dprintf(idx, "<a href=\"http://www.kreativrauschen.de/stats.mod/\">");
    dprintf(idx, "Stats.mod v%s</a></th></tr></table>\n", MODULE_VERSION);
    long_dprintf(idx, SLFOOTER);
    dprintf(idx, "</body></html>\n");
    return;
  }
  if (buf[0] == '/') {
    if (!strncasecmp(buf, "/e/", 3)) {
      buf += 2;
      buf[0] = '!';
    } else if (!strncasecmp(buf, "/p/", 3)) {
      buf += 2;
      buf[0] = '+';
    } else if (!strncasecmp(buf, "/a/", 3)) {
      buf += 2;
      buf[0] = '&';
    } else if (buf[1] && strchr("+&!", buf[1]))
      buf++;
    else
      buf[0] = '#';
  }
  if (buf[strlen(buf) - 1] == '/')
    buf[strlen(buf) - 1] = 0;
  channel = splitpath(&buf);
#ifndef OLDBOT
  chan = findchan_by_dname(channel);
#else
  chan = findchan(channel);
#endif
  if (!chan) {
    dprintf(idx, "no such channel %s", channel);
    if (idx >= 0)
      stats_info_access(idx)->code = 404;
    return;
  }
  gs = findglobstats(channel);
  if (gs)
    setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
  else
    setslglobs(channel, 0, 0, 0);
  if (!strcmp(buf, "/") || !buf[0]) {
    dprintf(idx, "<html><head><title>%s</title>\n%s\n</head>%s", SLINDEXTITEL, SLCSS, SLBODYTAG);
    long_dprintf(idx, SLHEADER);
    dprintf(idx, "<br><br><br><center>\n");
    dprintf(idx, "<table border=1 width=50%%>\n");
    dprintf(idx, "<tr><th colspan=2><font size=+2>%s</font></th></tr>", channel);
    dprintf(idx, "<tr><td rowspan=4 align=center>%s</td>", SLTOP);
    dprintf(idx, "<td align=center><a href=\"top/total/words/\">%s</a></td>", SLTOTAL);
    dprintf(idx, "</tr>\n<tr>");
    dprintf(idx, "<td align=center><a href=\"top/daily/words/\">%s</a></td>", SLDAILY);
    dprintf(idx, "</tr>\n<tr>");
    dprintf(idx, "<td align=center><a href=\"top/weekly/words/\">%s</a></td>", SLWEEKLY);
    dprintf(idx, "</tr>\n<tr>");
    dprintf(idx, "<td align=center><a href=\"top/monthly/words/\">%s</a></td>", SLMONTHLY);
    dprintf(idx, "</tr>\n");
    if (show_userlist)
      dprintf(idx, "<tr><td align=center colspan=2><a href=\"users/\">%s</a></td></tr>", SLUSERS);
    if (show_usersonchan)
      dprintf(idx, "<tr><td align=center colspan=2><a href=\"onchan/\">%s</a></td></tr>", SLONCHAN);
    dprintf(idx, "<tr><td align=center colspan=2><a href=\"misc/\">%s</a></td></tr>", SLMISCSTATS);
    dprintf(idx, "<tr><td colspan=2 height=0><font size=-5>&nbsp;</font></td></tr>\n");
    dprintf(idx, "<tr><th colspan=2 align=center><font size=-2>");
    dprintf(idx, "<a href=\"http://www.kreativrauschen.de/stats.mod/\">");
    dprintf(idx, "Stats.mod v%s</a></font></th></tr></table>\n", MODULE_VERSION);
    dprintf(idx, "</center>\n");
    long_dprintf(idx, SLFOOTER);
    dprintf(idx, "</body></html>\n");
    return;
  }
  command = splitpath(&buf);
  if (!strcasecmp(command, "top")) {
    if (!strcmp(buf, "/") || !buf[0]) {
      dprintf(idx, "<html><body><table>");
      dprintf(idx, "<tr><td><a href=\"total/\">total</a></td></tr>");
      dprintf(idx, "<tr><td><a href=\"today/\">today</a></td></tr>");
      dprintf(idx, "</table></body></html>");
      return;
    }
    stoday = splitpath(&buf);
    if (!strcasecmp(stoday, "today") || !strcasecmp(stoday, "daily"))
      today = S_DAILY;
    else if (!strcasecmp(stoday, "weekly"))
      today = S_WEEKLY;
    else if (!strcasecmp(stoday, "monthly"))
      today = S_MONTHLY;
    else if (!strcasecmp(stoday, "total"))
      today = 0;
    else {
      dprintf(idx, "<html><body>Error, invalid value %s<table>", stoday);
      dprintf(idx, "<tr><td><a href=\"../total/\">total</a></td></tr>");
      dprintf(idx, "<tr><td><a href=\"../daily/\">daily</a></td></tr>");
      dprintf(idx, "<tr><td><a href=\"../weekly/\">weekly</a></td></tr>");
      dprintf(idx, "<tr><td><a href=\"../monthly/\">monthly</a></td></tr>");
      dprintf(idx, "</table></body></html>");
      Assert(idx >= 0);
      stats_info_access(idx)->code = 404;
      return;
    }
    if (!strcmp(buf, "/") || !buf[0]) {
      dprintf(idx, "<html><body><table>");
      sprintf(what, "%s", webstats);
      pwhat = what;
      while (strlen(pwhat) > 0) {
        type = newsplit(&pwhat);
        itype = typetoi(type);
        if (itype >= 0) {
          dprintf(idx, "<tr><td><a href=\"%s/\">%s</a></td></tr>", type, type);
        }
      }
      dprintf(idx, "</table></body></html>");
      return;
    }
    gs = findglobstats(channel);
    if (!gs) {
      debug1("Error! Can't find global stats for %s", channel);
      dprintf(idx, "<html><body>Error! Can't find global stats for %s</body></html>", channel);
      if (idx >= 0)
        stats_info_access(idx)->code = 404;
      return;
    }
    if (!strncasecmp(buf, "graph", 5)) {
      do_graphs(idx, today, gs, channel);
      return;
    }
    do_toptalkers(idx, today, gs, channel, buf);
    return;
  } else if (!strcasecmp(command, "onchan")) {
    display_users_on_chan(idx, channel, chan);
  } else if (!strcasecmp(command, "users")) {
    gs = findglobstats(channel);
    if (!gs) {
      debug1("Error! Can't find global stats for %s", channel);
      dprintf(idx, "<html><body>Error! Can't find global stats for %s</body></html>", channel);
      return;
    }
    setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
    if (!strcmp(buf, "/") || !buf[0]) {
      if (!show_userlist)
        return;
      sort_stats_alphabetically(gs);
      dprintf(idx, "<html><head><title>%s</title>\n%s\n</head>%s",
              SLUSERSTITLE, SLCSS, SLBODYTAG);
      long_dprintf(idx, SLHEADER);
      dprintf(idx, "<center>\n");
      dprintf(idx, "<table border=%d>\n<tr><th>User</th><th>Info</th></tr>\n", table_border);
      i = 0;
      for (ls = gs->local; ls; ls = ls->next)
        i++;
      wert = table_color;
      b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
      wert = fade_table_to;
      b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
      rstep = (r2 - r) / i;
      gstep = (g2 - g) / i;
      bstep = (b2 - b) / i;
      for (ls = gs->local; ls; ls = ls->next) {
        u = get_user_by_handle(userlist, ls->user);
        what[0] = 0;
        get_handle_chaninfo(ls->user, channel, what);
        pwhat = what;
        if (!what[0])
          pwhat = get_user(&USERENTRY_INFO, u);
        if (!pwhat) {
          sprintf(what, "&nbsp;");
          pwhat = what;
        }
        dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td><a href=\"%s/\">%s</a></td><td>%s</td></tr>\n",
                (int) r, (int) g, (int) b, ls->user, ls->user, filt2(pwhat));
        r += rstep;
        g += gstep;
        b += bstep;
      }
/*
*     dprintf(idx, "<tr><td colspan=2 align=center><a href=\"http://www.kreativrauschen.de/stats.mod/\">");
*     dprintf(idx, "Stats.mod v%s</a></td></tr></table>\n", MODULE_VERSION);
*/
      dprintf(idx, "</table>\n");
      dprintf(idx, "<br><br><hr>\n");
      dprintf(idx, "<table width=100%% border=0>\n");
      dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
      dprintf(idx, "<td width=25%% align=center><font size=-1><%sa "
              "href=\"../onchan/\">%s</a></font></td>\n",
              ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLONCHAN));
      dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../top/total/words/\">top%d</a></font></td>\n", webnr);
      dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../misc/\">%s</a></font></td>\n", SLMISCSTATS);
      dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../../\">%s</a></font></td>\n", SLOTHERCHANS);
      dprintf(idx, "</tr></table></td></tr>\n");
      dprintf(idx, "</table>\n");
      dprintf(idx, "<center>Created by <a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod v%s</a></center></body></html>", MODULE_VERSION);
      dprintf(idx, "</center>\n");
      long_dprintf(idx, SLFOOTER);
      dprintf(idx, "</body></html>");
      return;
    }
    user = buf;
    ls = findlocstats(channel, user);
    if (!ls) {
      debug2("Error! Can't find local stats for %s in %s", user, channel);
      dprintf(idx, "<html><body>I don't have any stats for %s in %s, sorry</body></html>", user, channel);
      if (idx >= 0)
        stats_info_access(idx)->code = 404;
      return;
    }
    i = countwords(webstats) + 2;
    suser = findsuser_by_name(user);
    dprintf(idx, "<html><head><META http-equiv=\"Pragma\" content=\"no-cache\"><META http-equiv=\"Expires\" content=\"now\">\n");
    slgloblocstats = ls;
    dprintf(idx, "<title>%s</title>\n%s\n</head>%s\n", SLUSERTITLE, SLCSS, SLBODYTAG);
    long_dprintf(idx, SLHEADER);
    dprintf(idx, "<center>\n");
    dprintf(idx, "%s\n", SLUSERHEAD);
    if (suser) {
      dprintf(idx, "<center>");
      if (suser->email)
        dprintf(idx, "<a href=\"mailto:%s\" target=\"_new\">%s</a><br>", suser->email, SLEMAIL);
      if (suser->homepage)
        dprintf(idx, "<a href=\"%s\" target=\"_new\">%s</a><br>", suser->homepage, SLHOMEPAGE);
      dprintf(idx, "</center><br>");
    }
    dprintf(idx, "<table border=%d><tr><td></td><td>%s</td>", table_border, SLPLACE);
    sprintf(what, "%s", webstats);
    pwhat = what;
    while (strlen(pwhat) > 0) {
      dprintf(idx, "<td>%s</td>", getslangtype(newsplit(&pwhat)));
    }
    dprintf(idx, "</tr>\n");
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / 4;
    gstep = (g2 - g) / 4;
    bstep = (b2 - b) / 4;
    for (today = 0; today < 4; today++) {
      dprintf(idx, "<tr><td align=center>%s</td>", RANGESTR_LONG);
      nr = 0;
      if (ls->values[today][T_WORDS] > 0) {
        sortstats(gs, T_WORDS, today);
        for (ls2 = gs->slocal[today][T_WORDS]; ls2; ls2 = ls2->snext[today][T_WORDS]) {
          nr++;
          if (!strcasecmp(ls2->user, user))
            break;
        }
      }
      dprintf(idx, "<td bgcolor=#%02x%02x%02x align=\"right\">\n", (int) r, (int) g, (int) b);
      if (nr == 0)
        dprintf(idx, "-</td>");
      else
        dprintf(idx, "%d</td>", nr);
      sprintf(what, "%s", webstats);
      pwhat = what;
      while (strlen(pwhat) > 0) {
        itype = typetoi(newsplit(&pwhat));
        dprintf(idx, "<td bgcolor=#%02x%02x%02x align=\"right\">\n", (int) r, (int) g, (int) b);
        if (itype == T_MINUTES)
          dprintf(idx, "%s</td>", stats_duration(ls->values[today][itype] * 60));
        else if (itype >= 0)
          dprintf(idx, "%d</td>", ls->values[today][itype]);
        else if (itype == T_WPL) {
	    if (ls->values[today][T_LINES])
            dprintf(idx, "%.2f</td>", (float) ls->values[today][T_WORDS] / (float) ls->values[today][T_LINES]);
          else
            dprintf(idx, "0</td>");
        } else if (itype == T_IDLE) {
          if (ls->values[today][T_LINES])
            dprintf(idx, "%.2f</td>", (float) ls->values[today][T_MINUTES] / (float) ls->values[today][T_LINES]);
          else
            dprintf(idx, "0</td>");
        } else
          dprintf(idx, "ERR!</td>");
      }
      dprintf(idx, "</tr>\n");
      r += rstep;
      g += gstep;
      b += bstep;
    }
    if (ls->quotes && quote_freq) {
      nr = 0;
      for (qs = ls->quotes; qs; qs = qs->next)
        nr++;
      x = random() % nr;
      ii = 0;
      qs = ls->quotes;
      while (qs) {
        if (ii == x) {
          dprintf(idx, "<tr><td colspan=%d align=center>", i);
          dprintf(idx, SLRANDQUOTE, filt2(qs->quote));
          dprintf(idx, "</td></tr>\n");
          break;
        }
        qs = qs->next;
        ii++;
      }
    }
    dprintf(idx, "</table></center>\n");
    if (ls->words) {
      if (ls->words) {
        nr = 0;
        for (ws = ls->words; ws; ws = ws->next)
          nr++;
        slglobint = nr;
        dprintf(idx, "<center><br><br>%s<br>\n", SLUWORDSTATS);
        sortwordstats(ls, NULL);
        dprintf(idx, "<table>\n");
        nr = 0;
        ws = ls->words;
        while (ws && (nr < 10)) {
          nr++;
          dprintf(idx, "<tr><td>%d.</td><td>%s</td><td>(%d)</td></tr>\n", nr, filt2(ws->word), ws->nr);
          ws = ws->next;
        }
        dprintf(idx, "</table><br><br></center>\n");
      }
    }
    dprintf(idx, "<br><br><hr>\n");
    dprintf(idx, "<table width=100%% border=0>\n");
    dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
    dprintf(idx, "<td width=20%% align=center><font size=-1><%sa href=\"../\">%s</a></font></td>\n",
            ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLOTHERUSERS));
    dprintf(idx, "<td width=20%% align=center><font size=-1><%sa href=\"../../onchan\">%s</a></font></td>\n",
            ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLONCHAN));
    dprintf(idx, "<td width=20%% align=center><font size=-1><a href=\"../../top/total/words/\">%s</a></font></td>\n", SLTOP);
    dprintf(idx, "<td width=20%% align=center><font size=-1><a href=\"../../misc/\">%s</a></font></td>\n", SLMISCSTATS);
    dprintf(idx, "<td width=20%% align=center><font size=-1><a href=\"../../../\">%s</a></font></td>\n", SLOTHERCHANS);
    dprintf(idx, "</tr></table></td></tr>\n");
    dprintf(idx, "</table>\n");
    dprintf(idx, "<center>Created by <a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod v%s</a></center></body></html>", MODULE_VERSION);
    long_dprintf(idx, SLFOOTER);
    dprintf(idx, "</body></html>");
    return;
  } else if (!strcasecmp(command, "misc")) {
    do_miscstats(idx, channel);
    return;
  } else {
    dprintf(idx, "<html><body>Error! unknown command %s</body></html>", command);
    if (idx >= 0)
      stats_info_access(idx)->code = 404;
    return;
  }
}

static void do_graphs(int idx, int today, globstats *gs, char *channel)
{
  int itype, nr;
  int total, rest, width, max;
  float onep, percent, wpercent;
  char bground[140];
  locstats *ls;
  char what[512], *pwhat, *type;


  Context;
  setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
  if (graphgif[0] == 0)
    bground[0] = 0;
  else
    sprintf(bground, "background=\"%s\"", graphgif);
  dprintf(idx, "<!-- Created by Stats.mod v%s-->\n", MODULE_VERSION);
  dprintf(idx, "<html><head><META http-equiv=\"Pragma\" content=\"no-cache\"><META http-equiv=\"Expires\" content=\"now\">\n");
  if (today == 0)
    dprintf(idx, "<title>%s</title>\n", SLGRTTITLE);
  else if (today == 1)
    dprintf(idx, "<title>%s</title>\n", SLGRDTITLE);
  else if (today == 2)
    dprintf(idx, "<title>%s</title>\n", SLGRWTITLE);
  else if (today == 3)
    dprintf(idx, "<title>%s</title>\n", SLGRMTITLE);
  dprintf(idx, "%s\n</head>%s\n", SLCSS, SLBODYTAG);
  long_dprintf(idx, SLHEADER);
  if (today == 0)
    dprintf(idx, "%s", SLGRTHEAD);
  else if (today == 1)
    dprintf(idx, "%s", SLGRDHEAD);
  else if (today == 2)
    dprintf(idx, "%s", SLGRWHEAD);
  else if (today == 3)
    dprintf(idx, "%s", SLGRMHEAD);
  sprintf(what, "%s", graphstats);
  pwhat = what;
  while (strlen(pwhat) > 0) {
    type = newsplit(&pwhat);
    itype = typetoi(type);
    if (itype < 0) {
      putlog(LOG_MISC, "*", "Stats.mod: Error serving livestats. Unsupported type %s. Skipping.", type);
      continue;
    }
    sortstats(gs, itype, today);
    max = 0;
    for (ls = gs->slocal[today][itype]; ls; ls = ls->snext[today][itype]) {
      if (listsuser(ls, channel)) {
        max = ls->values[today][itype];
        break;
      }
    }
    if (max == 0)
      continue;
    total = gettotal(gs, itype, today);
    rest = total;
    dprintf(idx, "<br><br><br><table width=100%% border=1><tr><th width=100%% align=center>");
    dprintf(idx, SLGRORDEREDBY, getslangtype(type));
    dprintf(idx, "</td></tr></table>\n");
    slglobint = total;
    dprintf(idx, SLGRTOTAL, getslangtype(type));
    dprintf(idx, "<br>\n");
    dprintf(idx, "<table width=100%%>\n");
    if (!total || !max)
      continue;
    onep = (float) total / 100.0;
    nr = 0;
    for (ls = gs->slocal[today][itype]; ls; ls = ls->snext[today][itype]) {
      if (!listsuser(ls, channel))
        continue;
      if (!ls->values[today][itype])
        break;
      nr++;
      if (nr > graphnr)
        break;
      dprintf(idx, "<tr>\n");
      percent = (float) ls->values[today][itype] / onep;
      wpercent = (float) ls->values[today][itype] / ((float) max / 100.0);
      width = (int) wpercent * 0.8;
      dprintf(idx, "  <td align=right width=10%%><a href=\"../../../users/%s/\">%s</a></td>\n", ls->user, ls->user);
      dprintf(idx, "  <td align=left width=90%%>\n");
      dprintf(idx, "    <table width=100%%><tr>\n");
      dprintf(idx, "      <td width=%d%% bgcolor=\"%s\" %s>&nbsp;</td>\n", width, graphcolor, bground);
      dprintf(idx, "      <td align=left width=%d%%>%.2f%% <font size=-2>[%d]</font></td>\n", 100 - width, percent, ls->values[today][itype]);
      dprintf(idx, "    </tr></table>\n");
      dprintf(idx, "  </td>\n");
      dprintf(idx, "</tr>\n");
      rest -= ls->values[today][itype];
    }
    dprintf(idx, "<tr>\n");
    percent = (float) rest / ((float) total / 100.0);
    wpercent = (float) rest / ((float) max / 100.0);
    width = (int) wpercent * 0.8;
    dprintf(idx, "  <td align=right width=10%%>%s</td>\n", SLGROTHERS);
    dprintf(idx, "  <td align=left width=90%%>\n");
    dprintf(idx, "    <table width=100%%><tr>\n");
    dprintf(idx, "      <td width=%d%% bgcolor=\"%s\" %s>&nbsp;</td>\n", width, graphcolor, bground);
    dprintf(idx, "      <td align=left width=%d%%>%.2f%% <font size=-2>[%d]</font></td>\n", 100 - width, percent, rest);
    dprintf(idx, "    </tr></table>\n");
    dprintf(idx, "  </td>\n");
    dprintf(idx, "</tr></table>\n");
  }
  dprintf(idx, "</table>\n");
  dprintf(idx, "<br><hr><br>\n");
  dprintf(idx, "<table width=100%% border=0>\n");
  dprintf(idx, "<tr><td width=25%% align=center><a href=\"../words/\">%s</a></td></tr>\n", SLGRTABLE);
  dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
  dprintf(idx, "<td width=25%% align=center><a href=\"../../total/graphs/\">%s</a></td>\n", SLTOTAL);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../daily/graphs/\">%s</a></td>\n", SLDAILY);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../weekly/graphs/\">%s</a></td>\n", SLWEEKLY);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../monthly/graphs/\">%s</a></td>\n", SLMONTHLY);
  dprintf(idx, "</tr></table></td></tr>\n");
  dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
  dprintf(idx, "<td width=25%% align=center><a href=\"../../../misc/\">%s</a></td>\n", SLMISCSTATS);
  dprintf(idx, "<td width=25%% align=center><%sa href=\"../../../users/\">%s</a></td>\n",
          ISLINK(show_userlist), ISTEXT(show_userlist, SLUSERS));
  dprintf(idx, "<td width=25%% align=center><%sa href=\"../../../onchan/\">%s</a></td>\n",
          ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLONCHAN));
  dprintf(idx, "<td width=25%% align=center><a href=\"../../../../\">%s</a></td>\n", SLOTHERCHANS);
  dprintf(idx, "</tr></table></td></tr>\n");
  dprintf(idx, "</table>\n");
  dprintf(idx, "<br><center>Created by "
          "<a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod "
          "v%s</a></center>\n", MODULE_VERSION);
  long_dprintf(idx, SLFOOTER);
  dprintf(idx, "</body></html>");
  return;
}

static void do_toptalkers(int idx, int today, globstats *gs, char *channel, char *buf)
{
  char what2[512], *pwhat2, *type, *type2;
  locstats *ls;
  int itype, itype2, pitype, nr, i;
  int wert;
  float r, g, b;
  float r2, g2, b2;
  float rstep, gstep, bstep;
  quotestr *q;
  unsigned long x;

  Context;
  setslglobs(channel, gs->peak[today], countstatmembers(gs), gs->started);
  type = buf;
  itype = typetoi(type);
  if (itype == T_ERROR) {
    dprintf(idx, "<html><body>Error! No such type %s</body></html>", type);
    if (idx >= 0)
      stats_info_access(idx)->code = 404;
    return;
  }
  if (itype < 0)
    pitype = (itype * -1) + (TOTAL_TYPES - 1);
  else
    pitype = itype;
  dprintf(idx, "<html><head><META http-equiv=\"Pragma\" content=\"no-cache\"><META http-equiv=\"Expires\" content=\"now\">\n");
  if (today == 0)
    dprintf(idx, "<title>%s</title>\n", SLTTOPTITLE);
  else if (today == 1)
    dprintf(idx, "<title>%s</title>\n", SLDTOPTITLE);
  else if (today == 2)
    dprintf(idx, "<title>%s</title>\n", SLWTOPTITLE);
  else if (today == 3)
    dprintf(idx, "<title>%s</title>\n", SLMTOPTITLE);
  dprintf(idx, "<META Name=\"Stats.mod\" Content=\"%s\">\n", MODULE_VERSION);
  dprintf(idx, "<META Name=\"channel\" Content=\"%s\">\n", gs->chan);
  dprintf(idx, "<META Name=\"network\" Content=\"%s\">\n", network);
  dprintf(idx, "<META Name=\"written\" Content=\"%lu\">\n", now);
  dprintf(idx, "%s\n</head>\n", SLCSS);
  dprintf(idx, "%s", SLBODYTAG);
  long_dprintf(idx, SLHEADER);
  if (today == 0)
    dprintf(idx, "%s\n", SLTTOPHEAD);
  else if (today == 1)
    dprintf(idx, "%s\n", SLDTOPHEAD);
  else if (today == 2)
    dprintf(idx, "%s\n", SLWTOPHEAD);
  else if (today == 3)
    dprintf(idx, "%s\n", SLMTOPHEAD);

  dprintf(idx, SLORDEREDBY, getslangtype(type));
  dprintf(idx, "<BR>\n");
  dprintf(idx, SLPEAK);
  dprintf(idx, "<BR>\n");
  dprintf(idx, "<P><center><table border=%d cellpadding=1><tr align=right>", table_border);
  dprintf(idx, "<th>%s</th>", SLTNR);
  dprintf(idx, "<th align=center>Nick</th>");
  sprintf(what2, "%s", webstats);
  pwhat2 = what2;
  while (strlen(pwhat2) > 0) {
    type2 = newsplit(&pwhat2);
    dprintf(idx, "<th><a href=\"../%s/\">%s</a></th>\n", type2, getslangtype(type2));
  }
  sortstats(gs, itype, today);
  nr = 0;
  wert = table_color;
  b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
  wert = fade_table_to;
  b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
  rstep = (r2 - r) / webnr;
  gstep = (g2 - g) / webnr;
  bstep = (b2 - b) / webnr;
  for (ls = gs->slocal[today][pitype]; ls; ls = ls->snext[today][pitype]) {
    if (!listsuser(ls, channel))
      continue;
    if ((itype >= 0) && !ls->values[today][itype])
      break;
    nr++;
    if (nr > webnr)
      break;
    dprintf(idx, "<tr align=\"right\" bgcolor=#%02x%02x%02x><td>%d</td><td><a href=\"../../../users/%s/\">%s</a></td>",
            (int) r, (int) g, (int) b, nr, ls->user, ls->user);
    r += rstep;
    g += gstep;
    b += bstep;
    sprintf(what2, "%s", webstats);
    pwhat2 = what2;
    while (strlen(pwhat2) > 0) {
      type2 = newsplit(&pwhat2);
      itype2 = typetoi(type2);
      dprintf(idx, "<td>");
      // mark the sorted value bold
      if (itype == itype2)
        dprintf(idx, "<b>");
      // now output the values
      // T_MINUTES gets a special handling, because it's kinda hard to read
      // something like "2348 Minutes" *g*
      if (itype2 == T_MINUTES)
        dprintf(idx, "%s", stats_duration(ls->values[today][itype2] * 60));
      // no positive type needs any special handling, so lt's just output it
      else if (itype2 >= 0)
        dprintf(idx, "%d", ls->values[today][itype2]);
      // words per line need to be calculated
      else if (itype2 == T_WPL) {
        if (ls->values[today][T_LINES])
          dprintf(idx, "%.2f", ((float) ls->values[today][T_WORDS]) / ((float) ls->values[today][T_LINES]));
        else
          dprintf(idx, "0");
      // idle-factor also needs to be calculated
      } else if (itype2 == T_IDLE) {
        if (ls->values[today][T_LINES])
          dprintf(idx, "%.2f", ((float) ls->values[today][T_MINUTES]) / ((float) ls->values[today][T_LINES]));
        else
          dprintf(idx, "0");
      } else if (itype2 == T_QUOTE) {
        if (!ls->quotes)
          dprintf(idx, "&nbsp;");
        else {
          nr = 0;
          for (q = ls->quotes; q; q = q->next)
            nr++;
          x = random() % nr;
          i = 0;
          q = ls->quotes;
          while (q) {
            if (i == x) {
              dprintf(idx, "%s", filt2(q->quote));
              break;
            }
            q = q->next;
            i++;
          }
	}
      } else  // output an error, if we missed something
        dprintf(idx, "ERROR! itype2: %d", itype2);
      if (itype == itype2)
        dprintf(idx, "</b>");
      dprintf(idx, "</td>");
    }
    dprintf(idx, "</tr>\n");
  }
  dprintf(idx, "<tr>");
  dprintf(idx, "<td colspan=%d align=center><b>", countwords(webstats) + 2);
  dprintf(idx, SLTOTALUSERS);
  dprintf(idx, "</B></td></tr>\n");
  dprintf(idx, "<tr><td colspan=%d align=center>", countwords(webstats) + 2);
  dprintf(idx, "<table width=100%% border=0>\n");
  dprintf(idx, "<tr><td width=100%% align=center colspan=4><a href=\"../graphs/\">%s</a></td></tr>\n", SLGRAPHS);
  dprintf(idx, "<tr><td width=25%% align=center><a href=\"../../total/%s/\">%s</a></td>\n", type, SLTOTAL);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../daily/%s/\">%s</a></td>\n", type, SLDAILY);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../weekly/%s/\">%s</a></td>\n", type, SLWEEKLY);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../monthly/%s/\">%s</a></td>\n", type, SLMONTHLY);
  dprintf(idx, "</tr></table></td></tr>\n");
  dprintf(idx, "<tr><td colspan=%d align=center><table width=100%% border=0><tr>\n", countwords(webstats) + 2);
  dprintf(idx, "<td width=25%% align=center><a href=\"../../../misc/\">%s</a></td>\n", SLMISCSTATS);
  dprintf(idx, "<td width=25%% align=center><%sa href=\"../../../users/\">%s</a></td>\n",
          ISLINK(show_userlist), ISTEXT(show_userlist, SLUSERS));
  dprintf(idx, "<td width=25%% align=center><%sa href=\"../../../onchan/\">%s</a></td>\n",
          ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLONCHAN));
  dprintf(idx, "<td width=25%% align=center><a href=\"../../../../\">%s</a></td>\n", SLOTHERCHANS);
  dprintf(idx, "</tr>\n</table></td></tr>\n");
  dprintf(idx, "</table><br>");
  if (!today)
    dprintf(idx, SLGSTARTED);
  dprintf(idx, "<br><a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod v%s</a>.", MODULE_VERSION);
  dprintf(idx, "</CENTER>\n");
  long_dprintf(idx, SLFOOTER);
  dprintf(idx, "</body></html>");
  return;
}

static void do_miscstats(int idx, char *channel)
{
  globstats *gs, *tlds, *isps;
  wordstats *ws;
  int nr, tr, i, wert, wieoft, pitype;
  locstats *ls;
  char *host, *s;
  topicstr *t;
  char ts[20];
  float f, max, onep;
  hoststr *h, *isp, *tld;
  struct slang_lang *l;
  struct slang_bntypes *ty;
  struct slang_bnplaces *p;
  struct slang_texts *txt;
  unsigned long x;
  float r, g, b;
  float r2, g2, b2;
  float rstep, gstep, bstep;
  struct stats_url *url;
  struct stats_kick *kick;
  struct stats_quote *log;

  Context;
  gs = findglobstats(channel);
  if (!gs) {
    dprintf(idx, "<html><body>ERROR! Can't find stats for %s!</body></html>\n", channel);
    return;
  }
  setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
  dprintf(idx, "<html><head><META http-equiv=\"Pragma\" content=\"no-cache\">"
          "<META http-equiv=\"Expires\" content=\"now\"><title>%s</title>\n",
          SLMISCTITLE);
  long_dprintf(idx, SLCSS);
  dprintf(idx, "</head>\n");
  dprintf(idx, "%s\n", SLBODYTAG);
  long_dprintf(idx, SLHEADER);
  dprintf(idx, "%s<br>\n", SLMISCHEAD);
  do_globwordstats(gs);
  max = 0.0;
  wert = nr = 0; /* I'm recycling these vars, so ignore the strange names */
  for (i = 0; i < 24; i++) {
    if (i < 12) {
      if (gs->users[S_USERCOUNTS][i] > 0)
        wert = 1;
    } else {
      if (gs->users[S_USERCOUNTS][i] > 0)
        nr = 1;
    }
    if (gs->users[S_USERCOUNTS][i] > 0)
      if ((((float) gs->users[S_USERSUM][i]) / ((float) gs->users[S_USERCOUNTS][i])) > max)
        max = ((float) gs->users[S_USERSUM][i]) / ((float) gs->users[S_USERCOUNTS][i]);
  }
  if (display_average_users && (max > 0.0)) {
    dprintf(idx, "%s<br><font size=\"-5\">\n", SLMAUSERS);
    if (wert && nr)
      dprintf(idx, "<table border=0 width=\"75%%\"><tr><td width=\"50%%\" align=\"center\" valign=\"bottom\">\n");
    else
      dprintf(idx, "<table border=0 width=\"37%%\"><tr><td width=\"100%%\" align=\"center\" valign=\"bottom\">\n");
    dprintf(idx, "<table border=%d width=\"100%%\">", table_border);
    onep = max / 100.0;
    for (i = 0; i < 24; i++) {
      if ((i == 12) && (wert && nr))
        dprintf(idx, "</table></td><td width=\"50%%\" align=\"center\" valign=\"bottom\"><table border=%d width=\"100%%\">", table_border);
      if (gs->users[S_USERCOUNTS][i] > 0) {
        f = ((float) gs->users[S_USERSUM][i]) / ((float) gs->users[S_USERCOUNTS][i]);
        dprintf(idx, "<tr><td width=\"1%%\"><font size=\"-5\">%d.00-%d.59</font></td><td width=\"99%%\"><table width=\"100%%\"><tr>", i, i);
        if (((int) (f / onep)) > 0)
          dprintf(idx, "<td width=\"%d%%\" bgcolor=\"%s\"><font size=\"-5\">&nbsp;</font></td><td><font size=\"-5\">%.2f</font></td></tr></table></td>", (int) (f / onep), graphcolor, f);
        else if ((f / onep) > 0.0)
          dprintf(idx, "<td width=\"100%%\"><font size=\"-5\">%.2f</font></td></tr></table></td>", f);
        else
          dprintf(idx, "<td width=\"100%%\"><font size=\"-5\">%.2f</font></td></tr></table></td>", f);
        dprintf(idx, "</tr>\n");
      }
    }
    dprintf(idx, "</table>");
    dprintf(idx, "</td></tr></table>");
    dprintf(idx, "</font>\n");
  }
  if (gs->topics) {
    dprintf(idx, "<br><br>%s<br>\n<table border=%d>", SLMTOPICS, table_border);
    i = 0;
    for (t = gs->topics; t; t = t->next)
      i++;
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / i;
    gstep = (g2 - g) / i;
    bstep = (b2 - b) / i;
    for (t = gs->topics; t; t = t->next) {
      strftime(ts, 20, "%H:%M", localtime(&t->when));
      setslnick(t->by);
      dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td>\"%s\"</td><td>",
              (int) r, (int) g, (int) b, filt2(t->topic));
      dprintf(idx, SLMTOPICBY, ts);
      dprintf(idx, "</td></tr>\n");
      r += rstep;
      g += gstep;
      b += bstep;
    }
    dprintf(idx, "</table><br><br>\n");
  }
  if (gs->urls && log_urls) {
    nr = 0;
    for (url = gs->urls; url; url = url->next) {
      nr++;
      url->shown = 0;
    }
    if (nr > log_urls)
      wieoft = log_urls;
    else
      wieoft = nr;
    slglobint = wieoft;
    dprintf(idx, "<br><br><table border=%d><caption>%s</caption>", table_border, SLMURLS);
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / wieoft;
    gstep = (g2 - g) / wieoft;
    bstep = (b2 - b) / wieoft;
    while (wieoft > 0) {
      x = random() % nr;
      i = 0;
      url = gs->urls;
      while (url) {
        if (url->shown) {
          url = url->next;
          continue;
        }
        if (i == x) {
          dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td>\"<a href=\"%s\">%s</a>\"</td><td>",
                  (int) r, (int) g, (int) b, url->url, url->url);
          strftime(ts, 20, "%H:%M", localtime(&url->when));
          setslnick(url->by);
          dprintf(idx, SLMURLBY, ts);
          dprintf(idx, "</td></tr>\n");
          url->shown = 1;
          nr--;
          r += rstep;
          g += gstep;
          b += bstep;
          break;
        }
        url = url->next;
        i++;
      }
      wieoft--;
    }
    dprintf(idx, "</table>\n");
  }
  if (gs->hosts) {
    tlds = nmalloc(sizeof(globstats));
    tlds->hosts = NULL;
    isps = nmalloc(sizeof(globstats));
    isps->hosts = NULL;
    for (h = gs->hosts; h; h = h->next) {
      // skip IPv6 hosts
      if (strchr(h->host, ':'))
        continue;
      host = strrchr(h->host, '.') + 1;
      if (!atoi(host) && (host[0] != '0')) {
        addhost(host, tlds);
        host = h->host;
        while ((s = strchr(host, '.')) && strchr(s + 1, '.'))
          host = s + 1;
        addhost(host, isps);
      } else {
        addhost("[IP]", tlds);
        addhost("[IP]", isps);
      }
    }
    sorthosts(isps);
    sorthosts(tlds);
    dprintf(idx, "<br><br><table border=%d><tr><th colspan=2>%s</th></tr><tr><th>%s</th><th>%s</th></tr>\n",
            table_border, SLMMOSTUSED, SLMISPS, SLMTLDS);
    i = 0;
    isp = isps->hosts;
    tld = tlds->hosts;
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / 5;
    gstep = (g2 - g) / 5;
    bstep = (b2 - b) / 5;
    while ((i <= 5) && (isp || tld)) {
      i++;
      dprintf(idx, "<tr bgcolor=#%02x%02x%02x>\n", (int) r, (int) g, (int) b);
      r += rstep;
      g += gstep;
      b += bstep;
      if (isp)
        dprintf(idx, "<td>%s (%d)</td>", isp->host, isp->nr);
      else
        dprintf(idx, "<td>-</td>");
      if (tld)
        dprintf(idx, "<td>%s (%d)</td>", tld->host, tld->nr);
      else
        dprintf(idx, "<td>-</td>");
      dprintf(idx, "</tr>\n");
      if (isp)
        isp = isp->next;
      if (tld)
        tld = tld->next;
    }
    dprintf(idx, "</table>\n");
    free_hosts(isps->hosts);
    free_hosts(tlds->hosts);
    nfree(isps);
    nfree(tlds);
  }
  if (gs->kicks && (display_kicks > 0)) {
    nr = 0;
    for (kick = gs->kicks; kick; kick = kick->next) {
      nr++;
      kick->shown = 0;
    }
    if (nr > display_kicks)
      wieoft = display_kicks;
    else
      wieoft = nr;
    slglobint = wieoft;
    dprintf(idx, "<br><br><table border=%d><caption>%s</caption>", table_border, SLMKICKS);
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / wieoft;
    gstep = (g2 - g) / wieoft;
    bstep = (b2 - b) / wieoft;
    while (wieoft > 0) {
      x = random() % nr;
      i = 0;
      kick = gs->kicks;
      while (kick) {
        if (kick->shown) {
          kick = kick->next;
          continue;
        }
        if (i == x) {
          dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td><cite><font size=-2>", (int) r, (int) g, (int) b);
          for (log = kick->log; log; log = log->next) {
	    if (log->next)
              dprintf(idx, "%s<br>", filt2(log->quote));
            else
              dprintf(idx, "</font>%s</b><br>", filt2(log->quote));
	  }
          dprintf(idx, "</cite></td></tr>\n");
          kick->shown = 1;
          nr--;
          r += rstep;
          g += gstep;
          b += bstep;
          break;
        }
        kick = kick->next;
        i++;
      }
      wieoft--;
    }
    dprintf(idx, "</table>\n");
  }
  if (gs->words) {
    nr = 0;
    for (ws = gs->words; ws; ws = ws->next)
      nr++;
    dprintf(idx, "<br><br>"); dprintf(idx, SLMCWORDSTATS, nr); dprintf(idx, "<br>\n");
    dprintf(idx, "%s<br><table border=%d>\n", SLMMOSTUSEDWORDS, table_border);
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / 10;
    gstep = (g2 - g) / 10;
    bstep = (b2 - b) / 10;
    ws = gs->words;
    nr = 0;
    while (ws && (nr < 10)) {
      nr++;
      dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td>%d.</td><td>%s</td><td>(%d)</td></tr>\n",
              (int) r, (int) g, (int) b, nr, filt2(ws->word), ws->nr);
      r += rstep;
      g += gstep;
      b += bstep;
      ws = ws->next;
    }
    dprintf(idx, "</table>\n");
  }
  dprintf(idx, "<br><br><table border=%d>\n", table_border);
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !slgloblang) || (l->lang && slgloblang && !strcasecmp(l->lang, slgloblang))) {
      i = 0;
      for (ty = l->bignumbers; ty; ty = ty->next)
        i++;
      wert = table_color;
      b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
      wert = fade_table_to;
      b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
      rstep = (r2 - r) / i;
      gstep = (g2 - g) / i;
      bstep = (b2 - b) / i;
      for (ty = l->bignumbers; ty; ty = ty->next) {
        i = typetoi(ty->type);
        sortstats(gs, i, S_DAILY);
        tr = 0;
        for (p = ty->places; p; p = p->next) {
          nr = 1;
          if (i < 0)
            pitype = (i * -1) + (TOTAL_TYPES - 1);
          else
            pitype = i;
          ls = gs->slocal[S_DAILY][pitype];
          while (ls && (nr < p->place)) {
            nr++;
            ls = ls->snext[S_DAILY][pitype];
          }
          // just skip this entry if any vital information is missing.
          if (!ls)
            continue;
          else if ((i >= 0) && !ls->values[S_DAILY][i])
            continue;
          else if ((i == T_WPL) && (!ls->values[S_DAILY][T_WORDS] || !ls->values[S_DAILY][T_LINES]))
            continue;
          else if ((i == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES] || !ls->values[S_DAILY][T_LINES]))
            continue;
          else if ((i == T_VOCABLES) && !ls->vocables)
            continue;
	  if (!tr) {
	    dprintf(idx, "<tr bgcolor=#%02x%02x%02x><td>\n", (int) r, (int) g, (int) b);
            r += rstep;
            g += gstep;
            b += bstep;
	    tr = 1;
	  }
          slgloblocstats = ls;
          slglobtype = ty->type;
          x = random() % p->entries;
          txt = p->texts;
          while (txt) {
            if (!x)
              dprintf(idx, "%s\n", dynamicslang(txt->text));
            x--;
            txt = txt->next;
	  }
	}
	if (tr)
	  dprintf(idx, "</td></tr>\n");
      }
    }
  }
  dprintf(idx, "</table>\n");
  dprintf(idx, "<br><br><hr>\n");
  dprintf(idx, "<table width=100%% border=0>\n");
  dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
  dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../top/total/words/\">top%d</a></font></td>\n", webnr);
  dprintf(idx, "<td width=25%% align=center><font size=-1><%sa href=\"../users/\">%s</a></font></td>\n",
          ISLINK(show_userlist), ISTEXT(show_userlist, SLUSERS));
  dprintf(idx, "<td width=25%% align=center><font size=-1><%sa href=\"../onchan/\">%s</a></font></td>\n",
          ISLINK(show_usersonchan), ISTEXT(show_usersonchan, SLONCHAN));
  dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../../\">%s</a></font></td>\n", SLOTHERCHANS);
  dprintf(idx, "</tr></table></td></tr>\n");
  dprintf(idx, "</table>\n");
  dprintf(idx, "<center>Created by <a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod v%s</a></center></body></html>", MODULE_VERSION);
  long_dprintf(idx, SLFOOTER);
  dprintf(idx, "</body></html>");
}

static void display_users_on_chan(int idx, char *channel, struct chanset_t *chan)
{
  memberlist *m;
  struct stats_memberlist *mm;
  int wert;
  float r, g, b;
  float r2, g2, b2;
  float rstep, gstep, bstep;
  int i;
  char s1[121];

  Context;
  dprintf(idx, "<html><head><title>%s</title>\n%s\n</head>\n", SLUSERSONCHANTITLE, SLCSS);
  dprintf(idx, "%s\n", SLBODYTAG);
  long_dprintf(idx, SLHEADER);
  if (show_usersonchan && chan && (chan->channel.members > 0)) {
    dprintf(idx, "<br><br><center><table border=%d>\n<tr><th colspan=3>%s</th></tr>\n", table_border, SLNOWONCHAN);
    dprintf(idx, "<tr><th align=\"center\">nick</th><th align=\"center\">user</th><th align=\"center\">%s</th></tr>\n", SLIDLETIME);
    i = 0;
    for (m = chan->channel.member; m && m->nick[0]; m = m->next)
      i++;
    wert = table_color;
    b = wert & 0xff; g = (wert & 0xff00) >> 8; r = (wert & 0xff0000) >> 16;
    wert = fade_table_to;
    b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
    rstep = (r2 - r) / i;
    gstep = (g2 - g) / i;
    bstep = (b2 - b) / i;
    for (m = chan->channel.member; m && m->nick[0]; m = m->next) {
      dprintf(idx, "<tr bgcolor=#%02x%02x%02x>", (int) r, (int) g, (int) b);
      dprintf(idx, "<td align=\"left\">%s%s</td>",
              chan_hasop(m) ? "@" : (chan_hasvoice(m) ? "+" : "&nbsp;"),
              m->nick);
      r += rstep;
      g += gstep;
      b += bstep;
      if (!m->user && ((strlen(m->nick) + strlen(m->userhost) + 1) < 120)) {
        sprintf(s1, "%s!%s", m->nick, m->userhost);
        m->user = get_user_by_host(s1);
      }
      if (!use_userfile) {
        mm = nick2suser(m->nick, channel);
        if (mm && mm->user)
          dprintf(idx, "<td align=\"center\"><a href=\"../users/%s/\">%s</a></td>", mm->user->user, mm->user->user);
        else
          dprintf(idx, "<td align=\"center\">-</td>");
      } else {
        if (m->user)
          dprintf(idx, "<td align=\"center\"><a href=\"../users/%s/\">%s</a></td>", m->user->handle, m->user->handle);
        else
          dprintf(idx, "<td align=\"center\">-</td>");
      }
      if (chan_issplit(m))
        dprintf(idx, "<td align=\"right\">%s</td>", SLNETSPLITTED);
      else if (!rfc_casecmp(m->nick, botname))
        dprintf(idx, "<td align=\"right\">%s</td>", SLITSME);
      else
        dprintf(idx, "<td align=\"right\">&nbsp;%s</td>", stats_duration(now - m->last));
      dprintf(idx, "</tr>\n");
    }
    dprintf(idx, "</table></center>\n<br><br>");
  }
  dprintf(idx, "<table width=100%% border=0>\n");
  dprintf(idx, "<tr><td align=center><table width=100%% border=0><tr>\n");
  dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../top/total/words/\">top%d</a></font></td>\n", webnr);
  dprintf(idx, "<td width=25%% align=center><font size=-1><%sa href=\"../users/\">%s</a></font></td>\n",
          ISLINK(show_userlist), ISTEXT(show_userlist, SLUSERS));
  dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../misc/\">%s</a></font></td>\n", SLMISCSTATS);
  dprintf(idx, "<td width=25%% align=center><font size=-1><a href=\"../../\">%s</a></font></td>\n", SLOTHERCHANS);
  dprintf(idx, "</tr></table></td></tr>\n");
  dprintf(idx, "</table>\n");
  dprintf(idx, "<center>Created by <a href=\"http://www.kreativrauschen.de/stats.mod/\">Stats.mod v%s</a></center></body></html>", MODULE_VERSION);
  long_dprintf(idx, SLFOOTER);
  dprintf(idx, "</body></html>");
}
