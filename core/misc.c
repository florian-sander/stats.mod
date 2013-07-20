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

static char *inverted_csplit(char **rest, char divider)
{
  char *p;

  if (!rest)
    return *rest = "";
  p = *rest + strlen(*rest) - 1;
  while ((p[0] != divider) && (p != *rest))
    p--;
  p[0] = 0;
  return p + 1;
}

/* stolen from tcl_duration in tclmisc.c */
static char duration_temp[256];
static char *stats_duration(int seconds, int details)
{
  char s[256];
  time_t sec;
  int details_shown = 0;

  sec = seconds;
  s[0] = 0;
  if (sec < 1) {
    snprintf(duration_temp, sizeof(duration_temp), "%s", SLSOMETIME);
    return duration_temp;
  }
  if (sec >= 31536000) {
    sprintf(s, "%d %s ", (int) (sec / 31536000),
            ((int) (sec / 31536000) > 1) ? SLYEARS : SLYEAR);
    sec -= (((int) (sec / 31536000)) * 31536000);
    details_shown++;
  }
  if ((sec >= 604800) && (details_shown < details)) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 604800),
            ((int) (sec / 604800) > 1) ? SLWEEKS : SLWEEK);
    sec -= (((int) (sec / 604800)) * 604800);
    details_shown++;
  }
  if ((sec >= 86400) && (details_shown < details)) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 86400),
            ((int) (sec / 86400) > 1) ? SLDAYS : SLDAY);
    sec -= (((int) (sec / 86400)) * 86400);
    details_shown++;
  }
  if ((sec >= 3600) && (details_shown < details)) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 3600),
            ((int) (sec / 3600) > 1) ? SLHOURS : SLHOUR);
    sec -= (((int) (sec / 3600)) * 3600);
    details_shown++;
  }
  if ((sec >= 60) && (details_shown < details)) {
    sprintf(&s[strlen(s)], "%d %s ", (int) (sec / 60),
            ((int) (sec / 60) > 1) ? SLMINUTES : SLMINUTE);
    sec -= (((int) (sec / 60)) * 60);
    details_shown++;
  }
  if ((sec > 0) && (details_shown < details)) {
    sprintf(&s[strlen(s)], "%d %s", (int) (sec / 1),
            ((int) (sec / 1) > 1) ? SLSECONDS : SLSECOND);
  }
  if (s[strlen(s) - 1] == ' ')
  	s[strlen(s) - 1]='\0';
  snprintf(duration_temp, sizeof(duration_temp), "%s", s);
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

// void lower(char*p){for(;*p=tolower(*p);p++);}

static void strlower(char *text)
{
  int i;

  for (i = 0; i < strlen(text); i++)
    text[i] = tolower(text[i]);
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

/* maskstricthost():
 * basically the same as maskhost() from src/misc.c, but _never_ stripts
 * "~+-^=" off the host
 * maskhost() version: * $Id: misc.c,v 1.1 2005/04/08 13:48:30 Administrator Exp $
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

/* get_timerange():
 *
 */
static int get_timerange(char *text)
{
  if (!strcasecmp(text, "today"))
    return S_TODAY;
  else if (!strcasecmp(text, "daily"))
    return S_TODAY;
  else if (!strcasecmp(text, "weekly"))
    return S_WEEKLY;
  else if (!strcasecmp(text, "monthly"))
    return S_MONTHLY;
  else if (!strcasecmp(text, "total"))
    return S_TOTAL;
  else
    return T_ERROR;
  // FIXME: Check for slanged timeranges!
}

#define SENDMAIL_ERROR 1

static int email_send(char *to, char *subject, char *body)
{
	FILE *f;

	f = popen("/usr/sbin/sendmail -t", "w");
	if (!f)
		return SENDMAIL_ERROR;
	fprintf(f, "To: %s\n", to);
	fprintf(f, "From: %s\n", botnetnick);
	fprintf(f, "Subject: %s\n", subject);
	fprintf(f, "\n");
	fprintf(f, "%s", body);
	fprintf(f, "\n.\n");
	debug0("rückmeldung von pclose testen");
	if (pclose(f) == -1)
		return SENDMAIL_ERROR;
	else
		return 0;
}
