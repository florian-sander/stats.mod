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

void stats_setsock(int sock, int options) /* fabian's code */
{
  (int) allocsock(sock, options);
  (int) fcntl(sock, F_SETFL, O_NONBLOCK);
}

static void write_new_webstats()
{
  char *url, *file, *dir, *dir2, *buf, *buf2, *type, *today;
  char *bak, *bak2;
  globstats *gs;
  locstats *ls;
  FILE *f;
  int fd;

  Context;
  putlog(LOG_MISC, "*", "Writing static webfiles to %s...", webdir);
  file = nmalloc(strlen(webdir) + strlen("/index.html") + 1);
  sprintf(file, "%s/index.html", webdir);
  fd = creat(file, 0644);
  if (fd == -1) {
    putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
    nfree(file);
    return;
  }
  nfree(file);
  stats_setsock(fd, SOCK_NONSOCK);
  send_livestats(-fd, "/");
  killsock(fd);
  for (gs = sdata; gs; gs = gs->next) {
    putlog(LOG_MISC, "*", "... writing stats for %s ...", gs->chan);
    if (strchr(gs->chan, '[') || strchr(gs->chan, ']') || strchr(gs->chan, '^')) {
      putlog(LOG_MISC, "*", "... illegal character ('[', ']', or '^') in channame, skipping...");
      continue;
    }
    dir = nmalloc(strlen(webdir) + strlen(gs->chan) + 1 + 1);
    sprintf(dir, "%s/%s", webdir, (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    mkdir(dir, 0755);
    file = nmalloc(strlen(dir) + 11 + 1);
    sprintf(file, "%s/index.html", dir);
    fd = creat(file, 0644);
    if (fd == -1) {
      putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
      nfree(dir);
      nfree(file);
      return;
    }
    nfree(file);
    url = nmalloc(strlen(gs->chan) + 2 + 1);
    sprintf(url, "/%s/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    stats_setsock(fd, SOCK_NONSOCK);
    send_livestats(-fd, url);
    nfree(url);
    killsock(fd);
    dir2 = nmalloc(strlen(dir) + 5 + 1);
    sprintf(dir2, "%s/misc", dir);
    mkdir(dir2, 0755);
    file = nmalloc(strlen(dir2) + 11 + 1);
    sprintf(file, "%s/index.html", dir2);
    fd = creat(file, 0644);
    if (fd == -1) {
      putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
      nfree(dir);
      nfree(file);
      nfree(dir2);
      return;
    }
    nfree(file);
    url = nmalloc(9 + strlen(gs->chan) + 1);
    sprintf(url, "/%s/misc/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    stats_setsock(fd, SOCK_NONSOCK);
    send_livestats(-fd, url);
    nfree(url);
    killsock(fd);
    nfree(dir2);
    dir2 = nmalloc(6 + strlen(dir) + 1);
    sprintf(dir2, "%s/top", dir);
    mkdir(dir2, 0755);
    file = nmalloc(strlen(dir2) + 11 + 1);
    sprintf(file, "%s/index.html", dir2);
    fd = creat(file, 0644);
    if (fd == -1) {
      putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
      nfree(dir);
      nfree(file);
      nfree(dir2);
      return;
    }
    nfree(file);
    url = nmalloc(8 + strlen(gs->chan) + 1);
    sprintf(url, "/%s/top/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    stats_setsock(fd, SOCK_NONSOCK);
    send_livestats(-fd, url);
    nfree(url);
    killsock(fd);
    nfree(dir2);
    buf = nmalloc(26 + 1);
    bak = buf;
    strcpy(buf, "total daily weekly monthly");
    while (buf[0]) {
      today = newsplit(&buf);
      dir2 = nmalloc(9 + strlen(dir) + strlen(today) + 1);
      sprintf(dir2, "%s/top/%s", dir, today);
      mkdir(dir2, 0755);
      nfree(dir2);
      buf2 = nmalloc(9 + strlen(webstats) + 1);
      sprintf(buf2, "%s graphs", webstats);
      bak2 = buf2;
      while (buf2[0]) {
        type = newsplit(&buf2);
        dir2 = nmalloc(12 + strlen(dir) + strlen(today) + strlen(type) + 1);
        sprintf(dir2, "%s/top/%s/%s", dir, today, type);
        mkdir(dir2, 0755);
        file = nmalloc(13 + strlen(dir2) + 1);
        sprintf(file, "%s/index.html", dir2);
        fd = creat(file, 0644);
        if (fd == -1) {
          putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
          nfree(dir);
          nfree(file);
          nfree(dir2);
          nfree(bak);
          nfree(bak2);
          return;
        }
        nfree(file);
        url = nmalloc(14 + strlen(gs->chan) + strlen(today) + strlen(type) + 1);
        sprintf(url, "/%s/top/%s/%s/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan, today, type);
        stats_setsock(fd, SOCK_NONSOCK);
        send_livestats(-fd, url);
        killsock(fd);
        nfree(url);
        nfree(dir2);
      }
      nfree(bak2);
    }
    nfree(bak);
    dir2 = nmalloc(8 + strlen(dir) + 1);
    sprintf(dir2, "%s/users", dir);
    mkdir(dir2, 0755);
    file = nmalloc(13 + strlen(dir2) + 1);
    sprintf(file, "%s/index.html", dir2);
    fd = creat(file, 0644);
    if (fd == -1) {
      putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
      nfree(dir);
      nfree(file);
      nfree(dir2);
      return;
    }
    nfree(file);
    url = nmalloc(10 + strlen(gs->chan) + 1 );
    sprintf(url, "/%s/users/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    stats_setsock(fd, SOCK_NONSOCK);
    send_livestats(-fd, url);
    killsock(fd);
    nfree(url);
    nfree(dir2);

    dir2 = nmalloc(9 + strlen(dir) + 1);
    sprintf(dir2, "%s/onchan", dir);
    mkdir(dir2, 0755);
    file = nmalloc(13 + strlen(dir2) + 1);
    sprintf(file, "%s/index.html", dir2);
    fd = creat(file, 0644);
    if (fd == -1) {
      putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
      nfree(dir);
      nfree(file);
      nfree(dir2);
      return;
    }
    nfree(file);
    url = nmalloc(11 + strlen(gs->chan) + 1 );
    sprintf(url, "/%s/onchan/", (gs->chan[0] == '#') ? gs->chan + 1 : gs->chan);
    stats_setsock(fd, SOCK_NONSOCK);
    send_livestats(-fd, url);
    killsock(fd);
    nfree(url);
    nfree(dir2);

    setslglobs(gs->chan, gs->peak[S_TOTAL], countstatmembers(gs), gs->started);
    for (ls = gs->local; ls; ls = ls->next) { 	/* WARNING! DANGEROUS LOOP! If the sorting changes, you can get trapped in an endless loop! */
      slgloblocstats = ls;
      dir2 = nmalloc(11 + strlen(dir) + strlen(ls->user) + 1);
      sprintf(dir2, "%s/users/%s", dir, ls->user);
      mkdir(dir2, 0755);
      file = nmalloc(13 + strlen(dir2) + 1);
      sprintf(file, "%s/index.html", dir2);
      f = fopen(file, "w");
      chmod(file, 0644);
      if (!f) {
        putlog(LOG_MISC, "*", "ERROR writing webfiles! (%s)", file);
        nfree(dir);
        nfree(file);
        nfree(dir2);
        return;
      }
      nfree(file);
      fprintf(f, "<html>\n<head>\n<title>%s</title>\n%s\n</head>\n%s\n", SLSTATICTITLE, SLCSS, bodytag);
      fprintf(f, "%s\n", SLSTATICBODY);
      fprintf(f, "<br><br><hr>\n");
      fprintf(f, "<table width=100%% border=0>\n");
      fprintf(f, "<tr><td align=center><table width=100%% border=0><tr>\n");
      fprintf(f, "<td width=25%% align=center><font size=-1><a href=\"../\">%s</a></font></td>\n", SLOTHERUSERS);
      fprintf(f, "<td width=25%% align=center><font size=-1><a href=\"../../top/total/words/\">%s</a></font></td>\n", SLTOP);
      fprintf(f, "<td width=25%% align=center><font size=-1><a href=\"../../misc/\">%s</a></font></td>\n", SLMISCSTATS);
      fprintf(f, "<td width=25%% align=center><font size=-1><a href=\"../../../\">%s</a></font></td>\n", SLOTHERCHANS);
      fprintf(f, "</tr></table></td></tr>\n");
      nfree(dir2);
      fclose(f);
    }
    nfree(dir);
  }
  putlog(LOG_MISC, "*", "... done.");
}
