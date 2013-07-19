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

static char *dynslang = NULL;
static char *filtbracketsstr = NULL;
static char *slglobchan = NULL;
static char *slgloblang = NULL;
static char *slglobtype = NULL;
static char *slglobnick = NULL;
static int slglobpeak, slglobtotalusers, slglobint;
static time_t slglobgstarted;
static locstats *slgloblocstats;

static int slang_expmem()
{
  int size = 0;
  struct slang_lang *l;
  struct slang_ids *i;
  struct slang_types *ty;
  struct slang_texts *t;
  struct slang_chan *ch;
  struct slang_bntypes *bt;
  struct slang_bnplaces *bp;

  Context;
  for (l = slangs; l; l = l->next) {
    size += sizeof(struct slang_lang);
    if (l->lang)
      size += strlen(l->lang) + 1;
    for (i = l->ids; i; i = i->next) {
      size += sizeof(struct slang_ids);
      for (t = i->texts; t; t = t->next) {
	size += sizeof(struct slang_texts);
	size += strlen(t->text) + 1;
      }
    }
    for (ty = l->types; ty; ty = ty->next) {
      size += sizeof(struct slang_types);
      size += strlen(ty->type) + 1;
      for (t = ty->texts; t; t = t->next) {
	size += sizeof(struct slang_texts);
	size += strlen(t->text) + 1;
      }
    }
    Context;
    for (bt = l->bignumbers; bt; bt = bt->next) {
      size += sizeof(struct slang_bntypes);
      size += strlen(bt->type) + 1;
      for (bp = bt->places; bp; bp = bp->next) {
	size += sizeof(struct slang_bnplaces);
        for (t = bp->texts; t; t = t->next) {
	  size += sizeof(struct slang_texts);
	  size += strlen(t->text) + 1;
        }
      }
    }
  }
  Context;
  for (ch = slangchans; ch; ch = ch->next) {
    size += sizeof(struct slang_chan);
    size += strlen(ch->chan) + 1;
    if (ch->lang)
      size += strlen(ch->lang) + 1;
  }
  if (dynslang)
    size += strlen(dynslang) + 1;
  if (slglobchan)
    size += strlen(slglobchan) + 1;
  if (slgloblang)
    size += strlen(slgloblang) + 1;
  if (slglobnick)
    size += strlen(slglobnick) + 1;
  if (filtbracketsstr)
      size += strlen(filtbracketsstr) + 1;
  return size;
}

static void free_slang()
{
  struct slang_lang *l, *ll;
  struct slang_ids *i, *ii;
  struct slang_texts *t, *tt;
  struct slang_types *ty, *tty;
  struct slang_chan *ch, *cch;
  struct slang_bntypes *bt, *btt;
  struct slang_bnplaces *bp, *bpp;

  Context;
  l = slangs;
  while (l) {
    l->durs[0] = l->durs[1] = l->durs[2] = l->durs[3] = l->durs[4] = NULL;
    l->durs[5] = l->durs[6] = l->durs[7] = l->durs[8] = l->durs[9] = NULL;
    i = l->ids;
    while (i) {
      t = i->texts;
      while (t) {
        tt = t->next;
        nfree(t->text);
        nfree(t);
        t = tt;
      }
      ii = i->next;
      nfree(i);
      i = ii;
    }
    ty = l->types;
    while (ty) {
      t = ty->texts;
      while (t) {
        tt = t->next;
        nfree(t->text);
        nfree(t);
        t = tt;
      }
      tty = ty->next;
      nfree(ty->type);
      nfree(ty);
      ty = tty;
    }
    Context;
    bt = l->bignumbers;
    while (bt) {
      bp = bt->places;
      while (bp) {
        t = bp->texts;
        while (t) {
          tt = t->next;
          nfree(t->text);
          nfree(t);
          t = tt;
        }
        bpp = bp->next;
        nfree(bp);
        bp = bpp;
      }
      btt = bt->next;
      nfree(bt->type);
      nfree(bt);
      bt = btt;
    }
    Context;
    ll = l->next;
    if (l->lang)
      nfree(l->lang);
    nfree(l);
    l = ll;
  }
  Context;
  ch = slangchans;
  while (ch) {
    cch = ch->next;
    nfree(ch->chan);
    if (ch->lang)
      nfree(ch->lang);
    nfree(ch);
    ch = cch;
  }
  if (slglobchan)
    nfree(slglobchan);
  if (slgloblang)
    nfree(slgloblang);
  if (dynslang)
    nfree(dynslang);
  if (slglobnick)
    nfree(slglobnick);
  if (filtbracketsstr)
      nfree(filtbracketsstr);
  slglobchan = slgloblang = dynslang = slglobnick = filtbracketsstr = NULL;
  Context;
}

static int loadslang(char *lang, char *file)
{
  FILE *f;
  char *s, buf[MAXSLANGLENGTH], *id;
  struct slang_lang *l, *nl;

  Context;
  if (lang)
    putlog(LOG_MISC, "*", "Loading slangfile for %s: %s", lang, file);
  else
    putlog(LOG_MISC, "*", "Loading default slangfile: %s", file);
  f = fopen(file, "r");
  if (f == NULL) {
    putlog(LOG_MISC, "*", "ERROR reading slangfile");
    return 0;
  }
  l = slangs;
  while (l) {
    if (!lang && !l->lang)
      break;
    if (lang && l->lang)
      if (!strcasecmp(lang, l->lang))
        break;
    l = l->next;
  }
  if (!l) {
    l = slangs;
    while (l && l->next)
      l = l->next;
    nl = nmalloc(sizeof(struct slang_lang));
    if (!lang) {
      nl->lang = NULL;
    } else {
      nl->lang = nmalloc(strlen(lang) + 1);
      strcpy(nl->lang, lang);
    }
    nl->next = NULL;
    nl->ids = NULL;
    nl->types = NULL;
    nl->bignumbers = NULL;
    nl->durs[0] = nl->durs[1] = nl->durs[2] = nl->durs[3] = nl->durs[4] = NULL;
    nl->durs[5] = nl->durs[6] = nl->durs[7] = nl->durs[8] = nl->durs[9] = NULL;
    if (l)
      l->next = nl;
    else
      slangs = nl;
    l = nl;
  }
  while (!feof(f)) {
    buf[0] = 0;
    s = buf;
    fgets(s, MAXSLANGLENGTH - 1, f);
    s[MAXSLANGLENGTH - 1] = 0;
    if (!s[0])
      continue;
    if ((s[strlen(s) - 1] == '\n') || (s[strlen(s) - 1] == '\r'))
      s[strlen(s) - 1] = 0;
    if ((s[strlen(s) - 1] == '\n') || (s[strlen(s) - 1] == '\r'))
      s[strlen(s) - 1] = 0;
    id = newsplit(&s);
    if (!strcasecmp(id, "T"))
      addslangtype(l, s);
    else if (!strcasecmp(id, "BN"))
      addslangbn(l, s);
    else
      addslangitem(l, atoi(id), s);
  }
  fclose(f);
  Context;
  return 1;
}

static void addslangitem(struct slang_lang *lang, int idnr, char *text)
{
  struct slang_ids *id, *nid;
  struct slang_texts *t, *nt;

  Context;
  if (idnr == 0)
    return;
  for (id = lang->ids; id; id = id->next)
    if (id->id == idnr)
      break;
  if (!id) {
    id = lang->ids;
    while (id && id->next)
      id = id->next;
    nid = nmalloc(sizeof(struct slang_ids));
    nid->id = idnr;
    nid->entries = 0;
    nid->texts = NULL;
    nid->next = NULL;
    if (id)
      id->next = nid;
    else
      lang->ids = nid;
    id = nid;
  }
  t = id->texts;
  while (t && t->next)
    t = t->next;
  nt = nmalloc(sizeof(struct slang_texts));
  nt->text = nmalloc(strlen(text) + 1);
  strcpy(nt->text, text);
  nt->dynamic = isdynamicslang(text);
  nt->next = NULL;
  if (t)
    t->next = nt;
  else
    id->texts = nt;
  id->entries++;
  if ((idnr >= 250) && (idnr < 260))
    lang->durs[idnr - 250] = nt->text;
}

static void addslangtype(struct slang_lang *lang, char *text)
{
  char *type;
  struct slang_types *ty, *nty;
  struct slang_texts *t, *nt;

  Context;
  type = newsplit(&text);
  strlower(type);
  if (!text[0])
    return;
  for (ty = lang->types; ty; ty = ty->next)
    if (!strcmp(ty->type, type))
      break;
  if (!ty) {
    for (ty = lang->types; ty && ty->next; ty = ty->next);
    nty = nmalloc(sizeof(struct slang_types));
    nty->type = nmalloc(strlen(type) + 1);
    strcpy(nty->type, type);
    nty->entries = 0;
    nty->next = NULL;
    nty->texts = NULL;
    if (ty)
      ty->next = nty;
    else
      lang->types = nty;
    ty = nty;
  }
  for (t = ty->texts; t && t->next; t = t->next);
  nt = nmalloc(sizeof(struct slang_texts));
  nt->text = nmalloc(strlen(text) + 1);
  strcpy(nt->text, text);
  nt->dynamic = isdynamicslang(text);
  nt->next = NULL;
  if (t)
    t->next = nt;
  else
    ty->texts = nt;
  ty->entries++;
}

static void addslangbn(struct slang_lang *lang, char *text)
{
  char *type, *splace;
  struct slang_bntypes *ty, *nty;
  struct slang_bnplaces *p, *np;
  struct slang_texts *t, *nt;
  int place;

  Context;
  type = newsplit(&text);
  strlower(type);
  splace = newsplit(&text);
  place = atoi(splace);
  if (!place) {
    debug2("place for %s is %d", type, place);
    return;
  }
  if (!text[0])
    return;
  for (ty = lang->bignumbers; ty; ty = ty->next)
    if (!strcmp(ty->type, type))
      break;
  if (!ty) {
    for (ty = lang->bignumbers; ty && ty->next; ty = ty->next);
    nty = nmalloc(sizeof(struct slang_bntypes));
    nty->type = nmalloc(strlen(type) + 1);
    strcpy(nty->type, type);
    nty->next = NULL;
    nty->places = NULL;
    if (ty)
      ty->next = nty;
    else
      lang->bignumbers = nty;
    ty = nty;
  }
  for (p = ty->places; p; p = p->next)
    if (p->place == place)
      return;
  if (!p) {
    for (p = ty->places; p && p->next; p = p->next);
    np = nmalloc(sizeof(struct slang_bnplaces));
    np->next = NULL;
    np->place = place;
    np->entries = 0;
    np->texts = NULL;
    if (p)
      p->next = np;
    else
      ty->places = np;
    p = np;
  }
  for (t = p->texts; t && t->next; t = t->next);
  nt = nmalloc(sizeof(struct slang_texts));
  nt->text = nmalloc(strlen(text) + 1);
  strcpy(nt->text, text);
  nt->dynamic = isdynamicslang(text);
  nt->next = NULL;
  if (t)
    t->next = nt;
  else
    p->texts = nt;
  p->entries++;
  Context;
}

char badslang[15];
static char *getslang(int idnr)
{
  struct slang_lang *l;
  struct slang_ids *id;
  struct slang_texts *t;
  char *lang;
  unsigned long x;

  Context;
  lang = slgloblang;
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !lang) || (l->lang && lang && !strcasecmp(l->lang, lang))) {
      for (id = l->ids; id; id = id->next) {
	if (id->id == idnr) {
	  x = random() % id->entries;
	  t = id->texts;
	  while (t) {
	    if (!x) {
	      if (t->dynamic)
	        return dynamicslang(t->text);
	      else
	        return t->text;
	    }
	    x--;
	    t = t->next;
	  }
	}
      }
    }
  }
  sprintf(badslang, "SLANG%d", idnr);
  return badslang;
}

char baddurs[15];
static char *getdur(int id)
{
  struct slang_lang *l;
  char *lang;

  lang = slgloblang;
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !lang) || (l->lang && lang && !strcasecmp(l->lang, lang))) {
      if (!l->durs[id]) {
        sprintf(baddurs, "DURNS%d", id);
        return baddurs;
      } else {
        return l->durs[id];
      }
    }
  }
  sprintf(baddurs, "DURNF%d", id);
  return baddurs;
}

static char *getslangtype(char *type)
{
  struct slang_lang *l;
  struct slang_types *ty;
  struct slang_texts *t;
  unsigned long x;
  char *lang;

  Context;
  lang = slgloblang;
  // I originally used strlower here, but it seems that it causes
  // minor problems (like crashes ^_^), so I had to remove it and use
  // strcasecmp() instead. I hope it doesn't cost too much cpu time.
  // strlower(type);
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !lang) || (l->lang && lang && !strcasecmp(l->lang, lang))) {
      for (ty = l->types; ty; ty = ty->next) {
	if (!strcmp(ty->type, type)) {
	  x = random() % ty->entries;
	  t = ty->texts;
	  while (t) {
	    if (!x)
	      return t->text;
	    x--;
	    t = t->next;
	  }
	}
      }
    }
  }
  return type;
}

/* slangtypetoi():
 * find the index of a "slanged" stat-type
 * if it isn't slanged, typetoi() is automatically called
 */
static int slangtypetoi(char *type)
{
  struct slang_lang *l;
  struct slang_types *ty;
  struct slang_texts *t;
  char *lang;

  Context;
  lang = slgloblang;
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !lang) || (l->lang && lang && !strcasecmp(l->lang, lang))) {
      for (ty = l->types; ty; ty = ty->next) {
        for (t = ty->texts; t; t = t->next) {
          if (!strcasecmp(t->text, type)) {
            return typetoi(ty->type);
          }
        }
      }
    }
  }
  return typetoi(type);
}

static int isdynamicslang(char *text)
{
  Context;
  if (strstr(text, "[bot]"))
    return 1;
  else if (strstr(text, "[topnr]"))
    return 1;
  else if (strstr(text, "[graphnr]"))
    return 1;
  else if (strstr(text, "[int]"))
    return 1;
  else if (strstr(text, "[chan]"))
    return 1;
  else if (strstr(text, "[user]"))
    return 1;
  else if (strstr(text, "[value]"))
    return 1;
  else if (strstr(text, "[peak]"))
    return 1;
  else if (strstr(text, "[totalusers]"))
    return 1;
  else if (strstr(text, "[chanstarted]"))
    return 1;
  else if (strstr(text, "[nick]"))
    return 1;
  else if (strstr(text, "[source "))
    return 1;
  return 0;
}

static char *dynamicslang(char *text)
{
  char *p, *tmp, *s, nr[61], *ende, *fbuf;
  time_t tt, ttbuf;
  int len, itype;
  int changed;
  FILE *f;

  if (dynslang)
    nfree(dynslang);
  tmp = nmalloc(strlen(text) + 1);
  strcpy(tmp, text);
  changed = 1;
  while (changed) {

  changed = 0;
  if ((p = strstr(tmp, "[bot]"))) {
    len = strlen(tmp) + strlen(botnetnick);
    p[0] = 0;
    p += 5;
    s = nmalloc(len + 1 - 5);
    sprintf(s, "%s%s%s", tmp, filtbrackets(botnetnick), p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[topnr]"))) {
    sprintf(nr, "%d", webnr);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 7;
    s = nmalloc(len + 1 - 7);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[graphnr]"))) {
    sprintf(nr, "%d", graphnr);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 9;
    s = nmalloc(len + 1 - 9);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[peak]"))) {
    sprintf(nr, "%d", slglobpeak);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 6;
    s = nmalloc(len + 1 - 6);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[totalusers]"))) {
    sprintf(nr, "%d", slglobtotalusers);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 12;
    s = nmalloc(len + 1 - 12);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[int]"))) {
    sprintf(nr, "%d", slglobint);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 5;
    s = nmalloc(len + 1 - 5);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[chanstarted]"))) {
    ttbuf = now;
    tt = slglobgstarted;
    strftime(nr, 60, "%d.%m. %Y  %H:%M", localtime(&tt));
    ctime(&ttbuf); /* workaround for eggdrop bug */
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 13;
    s = nmalloc(len + 1 - 13);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[value]"))) {
    itype = typetoi(slglobtype);
    if (itype >= 0)
      sprintf(nr, "%ld", slgloblocstats->values[S_DAILY][itype]);
    else if (itype == T_WPL)
      sprintf(nr, "%.2f", (float) slgloblocstats->values[S_TODAY][T_WORDS] / (float) slgloblocstats->values[S_TODAY][T_LINES]);
    else if (itype == T_IDLE)
      sprintf(nr, "%.2f", (float) slgloblocstats->values[S_TODAY][T_MINUTES] / (float) slgloblocstats->values[S_TODAY][T_LINES]);
    else if (itype == T_VOCABLES)
      sprintf(nr, "%d", slgloblocstats->vocables);
    else
      sprintf(nr, "UNKNOWN iTYPE: %d", itype);
    len = strlen(tmp) + strlen(nr);
    p[0] = 0;
    p += 7;
    s = nmalloc(len + 1 - 7);
    sprintf(s, "%s%s%s", tmp, nr, p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if (slglobchan && (p = strstr(tmp, "[chan]"))) {
    len = strlen(tmp) + strlen(slglobchan);
    p[0] = 0;
    p += 6;
    s = nmalloc(len + 1 - 6);
    sprintf(s, "%s%s%s", tmp, filtbrackets(slglobchan), p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if (slgloblocstats && (p = strstr(tmp, "[user]"))) {
    len = strlen(tmp) + strlen(slgloblocstats->user);
    p[0] = 0;
    p += 6;
    s = nmalloc(len + 1 - 6);
    sprintf(s, "%s%s%s", tmp, filtbrackets(slgloblocstats->user), p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if (slglobnick && (p = strstr(tmp, "[nick]"))) {
    len = strlen(tmp) + strlen(slglobnick);
    p[0] = 0;
    p += 6;
    s = nmalloc(len + 1 - 6);
    sprintf(s, "%s%s%s", tmp, filtbrackets(slglobnick), p);
    nfree(tmp);
    tmp = s;
    changed = 1;
  }
  if ((p = strstr(tmp, "[source "))) {
    len = strlen(tmp);
    p[0] = 0;
    p += 8;
    ende = strchr(p, ']');
    ende[0] = 0;
    fbuf = NULL;
    f = fopen(p, "r");
    if (f == NULL) {
      fbuf = nmalloc(16 + strlen(p) + 1);
      sprintf(fbuf, "File not found: %s", p);
    } else {
      s = nmalloc(512);
      while (!feof(f)) {
        s[0] = 0;
        fgets(s, 512 - 1, f);
        if (!s[0])
          continue;
        if (fbuf) {
          fbuf = nrealloc(fbuf, strlen(fbuf) + strlen(s) + 1);
          strcat(fbuf, s);
        } else {
          fbuf = nmalloc(strlen(s) + 1);
          strcpy(fbuf, s);
        }
      }
      fclose(f);
      nfree(s);
    }
    p = ende + 1;
    len += strlen(fbuf);
    s = nmalloc(len + 1 - 8);
    sprintf(s, "%s%s%s", tmp, filtbrackets(fbuf), p);
    nfree(tmp);
    nfree(fbuf);
    tmp = s;
    changed = 1;
  }

  }
  dynslang = tmp;
  return dynslang;
}

static char *filtbrackets(char *text)
{
  char *p;

  if (filtbracketsstr) {
    nfree(filtbracketsstr);
    filtbracketsstr = NULL;
  }
  if (strchr(text, '[') || strchr(text, ']')) {
    filtbracketsstr = nmalloc(strlen(text) + 1);
    strcpy(filtbracketsstr, text);
    for (p = filtbracketsstr; p[0]; p++) {
      if (p[0] == '[')
        p[0] = '{';
      else if (p[0] == ']')
        p[0] = '}';
    }
    return filtbracketsstr;
  } else {
    return text;
  }
}

static char *chanlang(char *channel)
{
  struct slang_chan *chan;

  Context;
  for (chan = slangchans; chan; chan = chan->next)
    if (!rfc_casecmp(chan->chan, channel))
      return chan->lang;
  return NULL;
}

static void setchanlang(char *channel, char *lang)
{
  struct slang_chan *chan, *nchan;

  Context;
  if (!strcasecmp(lang, "default"))
    lang = NULL;
  for (chan = slangchans; chan; chan = chan->next)
    if (!rfc_casecmp(chan->chan, channel))
      break;
  if (!chan) {
    for (chan = slangchans; chan && chan->next; chan = chan->next);
    nchan = nmalloc(sizeof(struct slang_chan));
    nchan->chan = nmalloc(strlen(channel) + 1);
    strcpy(nchan->chan, channel);
    nchan->lang = NULL;
    nchan->next = NULL;
    if (chan)
      chan->next = nchan;
    else
      slangchans = nchan;
    chan = nchan;
  }
  if (chan->lang) {
    nfree(chan->lang);
    chan->lang = NULL;
  }
  if (lang) {
    chan->lang = nmalloc(strlen(lang) + 1);
    strcpy(chan->lang, lang);
  }
}

static void setslglobs(char *chan, int peak, int totalusers, time_t globstarted)
{
  char *lang = NULL;

  Context;
  if (slglobchan)
    nfree(slglobchan);
  if (slgloblang)
    nfree(slgloblang);
  slgloblang = NULL;
  slglobchan = NULL;
  if (chan) {
    slglobchan = nmalloc(strlen(chan) + 1);
    strcpy(slglobchan, chan);
    lang = chanlang(chan);
  }
  if (lang) {
    slgloblang = nmalloc(strlen(lang) + 1);
    strcpy(slgloblang, lang);
  }
  slglobpeak = peak;
  slglobtotalusers = totalusers;
  slglobgstarted = globstarted;
}

static void setslnick(char *nick)
{
  if (slglobnick)
    nfree(slglobnick);
  slglobnick = nmalloc(strlen(nick) + 1);
  strcpy(slglobnick, nick);
}
