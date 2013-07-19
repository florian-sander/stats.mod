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

/* stolen from tcl_duration in tclmisc.c */
char duration_temp[256];
static char *stats_duration(int seconds)
{
  char s[256];
  time_t sec;

  sec = seconds;
  s[0] = 0;
  if (sec < 1) {
    sprintf(duration_temp, "0 seconds");
    return duration_temp;
  }
  if (sec >= 31536000) {
    sprintf(s, "%d %s ", (int) (sec / 31536000),
            ((int) (sec / 31536000) > 1) ? SLYEARS : SLYEAR);
    sec -= (((int) (sec / 31536000)) * 31536000);
  }
  if (sec >= 604800) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 604800),
            ((int) (sec / 604800) > 1) ? SLWEEKS : SLWEEK);
    sec -= (((int) (sec / 604800)) * 604800);
  }
  if (sec >= 86400) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 86400),
            ((int) (sec / 86400) > 1) ? SLDAYS : SLDAY);
    sec -= (((int) (sec / 86400)) * 86400);
  }
  if (sec >= 3600) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 3600),
            ((int) (sec / 3600) > 1) ? SLHOURS : SLHOUR);
    sec -= (((int) (sec / 3600)) * 3600);
  }
  if (sec >= 60) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 60),
            ((int) (sec / 60) > 1) ? SLMINUTES : SLMINUTE);
    sec -= (((int) (sec / 60)) * 60);
  }
/*  is there any place where seconds might be displayed??? I think not. */
/*
  if (sec > 0) {
    sprintf(&s[strlen(s)], "%d %s", (int) (sec / 1),
            ((int) (sec / 1) > 1) ? SLSECONDS : SLSECOND);
  }
*/
  sprintf(duration_temp, "%s", s);
  return duration_temp;
}

static int countsmileys(char *text)
{
  char buf[512], *pbuf, *smiley, *p;
  int ismileys = 0;

  sprintf(buf, "%s", smileys);
  pbuf = buf;
  while (strlen(pbuf) > 0) {
    smiley = newsplit(&pbuf);
    p = strstr(text, smiley);
    while (p) {
      ismileys++;
      p += strlen(smiley);
      p = strstr(p, smiley);
    }
  }
  return ismileys;
}

static int countstatmembers(globstats *gs)
{
  int members = 0;
  struct userrec *u;
  locstats *ls;

  Context;
  if (!gs)
    return 0;
  for (ls = gs->local; ls; ls = ls->next) {
    u = get_user_by_handle(userlist, ls->user);
    if (matchattr(u, nostatsflags, gs->chan))
      continue;
    members++;
  }
  return members;
}

static int gettotal(globstats *gs, int type, int today)
{
  int total = 0;
  struct userrec *u;
  struct stats_userlist *su;
  locstats *ls;

  for (ls = gs->local;ls; ls = ls->next) {
    if (use_userfile) {
      u = get_user_by_handle(userlist, ls->user);
      if (matchattr(u, nostatsflags, gs->chan))
        continue;
    } else {
      su = findsuser_by_name(ls->user);
      if (su && !su->list)
        continue;
    }
    total += ls->values[today][type];
  }
  return total;
}

/* stolen from tcl_matchattr */
static int matchattr (struct userrec *u, char *flags, char *chan) {
  struct flag_record plus, minus, user;
  int ok = 0, f;

#ifndef OLDBOT
  if (u && (!chan || findchan_by_dname(chan))) {
#else
  if (u && (!chan || findchan(chan))) {
#endif
    user.match = FR_GLOBAL | (chan ? FR_CHAN : 0) | FR_BOT;
    get_user_flagrec(u, &user, chan);
    plus.match = user.match;
    break_down_flags(flags, &plus, &minus);
    f = (minus.global || minus.udef_global || minus.chan ||
   minus.udef_chan || minus.bot);
    if (flagrec_eq(&plus, &user)) {
      if (!f)
  ok = 1;
      else {
  minus.match = plus.match ^ (FR_AND | FR_OR);
  if (!flagrec_eq(&minus, &user))
    ok = 1;
      }
    }
  }
  return ok;
}

static int countwords(char *buf)
{
  int i, words = 1;

  for (i = 0; i < strlen(buf); i++) {
    if ((buf[i] == ' ') && (buf[i+1] != ' '))
      words++;
  }
  return words;
}

static int countquestions(char *buf)
{
  int i, questions = 0;

  for (i = 0; i < strlen(buf); i++) {
    if ((buf[i] == '?') && (buf[i+1] != '?'))
      questions++;
  }
  return questions;
}

static char *splitpath(char **rest)
{
  register char *o, *r;

  if (!rest)
    return *rest = "";
  o = *rest;
  while (*o == ' ')
    o++;
  r = o;
  while (*o && (*o != '/'))
    o++;
  if (*o)
    *o++ = 0;
  *rest = o;
  return r;
}

static void strlower(char *text)
{
  int i;

  for (i = 0; i < strlen(text); i++)
    text[i] = tolower(text[i]);
}

static void filt(char *text)
{
  int i;

  for (i = 0; i < strlen(text); i++)
    if (text[i] == '%')
      text[i] = ' ';
}

char filt2_buf[512];
static char *filt2(char *text)
{
  char *buf;
  int i = 0;

  buf = filt2_buf;
  while ((i < 500) && text[0]) {
    if (text[0] == '<') {
      buf[0] = '&';
      buf[1] = 'l';
      buf[2] = 't';
      buf[3] = ';';
      buf += 4;
      i += 4;
    } else if (text[0] == '>') {
      buf[0] = '&';
      buf[1] = 'g';
      buf[2] = 't';
      buf[3] = ';';
      buf += 4;
      i += 4;
    } else {
      buf[0] = text[0];
      buf++;
      i++;
    }
    text++;
  }
  buf[0] = 0;
  return filt2_buf;
}

static int gethour()
{
  char ts[10];
  time_t tt;

  tt = now;
  strftime(ts, 9, "%H", localtime(&tt));
  ts[9] = 0;
  return atoi(ts);
}

static int getmonth()
{
  char ts[10];
  time_t tt;

  tt = now;
  strftime(ts, 9, "%m", localtime(&tt));
  ts[9] = 0;
  return atoi(ts);
}

static int ismonday()
{
  char ts[10];
  time_t tt;

  tt = now;
  strftime(ts, 9, "%a", localtime(&tt));
  ts[9] = 0;
  return (!strcasecmp(ts, "mon"));
}

static int secretchan(char *chan)
{
  struct chanset_t *ch;

  if (list_secret_chans)
    return 0;
  ch = findchan_by_dname(chan);
  if (!ch)
    return 0;
  if (ch->status & CHAN_SECRET)
    return 1;
  return 0;
}

static void long_dprintf(int idx, char *text)
{
  char buf[501];

  if (strlen(text) < 500)
    dprintf(idx, "%s", text);
  else {
    while (text[0]) {
      strncpy(buf, text, 500);
      buf[500] = 0;
      dprintf(idx, "%s", buf);
      text += strlen(buf);
    }
  }
}

/* maskstricthost():
 * basically the same as maskhost() from src/misc.c, but _never_ stripts
 * "~+-^=" off the host
 * maskhost() version: * $Id: misc.c,v 1.30 2000/10/27 19:27:32 fabian Exp $
 */
static void maskstricthost(const char *s, char *nw)
{
  register const char *p, *q, *e, *f;
  int i;

  *nw++ = '*';
  *nw++ = '!';
  p = (q = strchr(s, '!')) ? q + 1 : s;
  /* Strip of any nick, if a username is found, use last 8 chars */
  if ((q = strchr(p, '@'))) {
    int fl = 0;

    if ((q - p) > 9) {
      nw[0] = '*';
      p = q - 7;
      i = 1;
    } else
      i = 0;
    while (*p != '@') {
      if (!fl && strchr("~+-^=", *p)) {
//        if (strict_host)
      nw[i] = '?';
//    else
//      i--;
      } else
    nw[i] = *p;
      fl++;
      p++;
      i++;
    }
    nw[i++] = '@';
    q++;
  } else {
    nw[0] = '*';
    nw[1] = '@';
    i = 2;
    q = s;
  }
  nw += i;
  e = NULL;
  /* Now q points to the hostname, i point to where to put the mask */
  if ((!(p = strchr(q, '.')) || !(e = strchr(p + 1, '.'))) && !strchr(q, ':'))
    /* TLD or 2 part host */
    strcpy(nw, q);
  else {
    if (e == NULL) {        /* IPv6 address?        */
      const char *mask_str;

      f = strrchr(q, ':');
      if (strchr(f, '.')) { /* IPv4 wrapped in an IPv6? */
    f = strrchr(f, '.');
    mask_str = ".*";
      } else            /* ... no, true IPv6.       */
    mask_str = ":*";
      strncpy(nw, q, f - q);
      /* No need to nw[f-q] = 0 here, as the strcpy below will
       * terminate the string for us.
       */
      nw += (f - q);
      strcpy(nw, mask_str);
    } else {
      for (f = e; *f; f++);
      f--;
      if (*f >= '0' && *f <= '9') { /* Numeric IP address */
    while (*f != '.')
      f--;
    strncpy(nw, q, f - q);
    /* No need to nw[f-q] = 0 here, as the strcpy below will
     * terminate the string for us.
     */
    nw += (f - q);
    strcpy(nw, ".*");
      } else {              /* Normal host >= 3 parts */
    /*    a.b.c  -> *.b.c
     *    a.b.c.d ->  *.b.c.d if tld is a country (2 chars)
     *             OR   *.c.d if tld is com/edu/etc (3 chars)
     *    a.b.c.d.e -> *.c.d.e   etc
     */
    const char *x = strchr(e + 1, '.');

    if (!x)
      x = p;
    else if (strchr(x + 1, '.'))
      x = e;
    else if (strlen(x) == 3)
      x = p;
    else
      x = e;
    sprintf(nw, "*%s", x);
      }
    }
  }
}

