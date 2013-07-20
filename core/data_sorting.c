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

static void sortstats(struct stats_global *gs, int itype, int today)
{
  int again = 1;
  struct stats_local *last, *p, *c, *n;
  int a, b, pitype;

  Context;
  Assert(gs);
  again = 1;
  last = NULL;
  if (itype < 0) {
    // switch to the special sorting function
    switch (itype) {
      case T_WPL:
        sortstats_wpl(gs, today);
        break;
      case T_VOCABLES:
        sortstats_vocables(gs, today);
        break;
      case T_WORD:
        sortstats_word(gs, today);
        break;
      case T_IDLE:
        sortstats_idle(gs, today);
        break;
      default:
        debug1("Missing sorting algorithm for \"%d\"!!!", itype);
    }
    return;
  }

  // if (itype < 0) pitype = (TOTAL_TYPES - 1) + (itype * -1);
  // not needed here
  pitype = itype;
  while ((gs->slocal[today][pitype] != last) && (again)) {
    p = NULL;
    c = gs->slocal[today][pitype];
    n = c->snext[today][pitype];
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        a = c->values[today][itype];
        b = n->values[today][itype];
      }
      if (a < b) {
        again = 1;
        c->snext[today][pitype] = n->snext[today][pitype];
        n->snext[today][pitype] = c;
        if (p == NULL)
          gs->slocal[today][pitype] = n;
        else
          p->snext[today][pitype] = n;
      }
      p = c;
      c = n;
      n = n->snext[today][pitype];
    }
    last = c;
  }
  Context;
  return;
}

static void sortstats_wpl(struct stats_global *gs, int today)
{
  int again = 1;
  struct stats_local *last, *p, *c, *n;
  int a, b, pitype;

  Context;
  again = 1;
  last = NULL;
  pitype = (T_WPL * (-1)) + TOTAL_TYPES - 1;
  while ((gs->slocal[today][pitype] != last) && (again)) {
    p = NULL;
    c = gs->slocal[today][pitype];
    n = c->snext[today][pitype];
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        if (c->values[today][T_LINES] >= min_lines)
          a = (int) (((float) c->values[today][T_WORDS] / (float) c->values[today][T_LINES]) * 100.0);
        else
          a = 0;
        if (n->values[today][T_LINES] >= min_lines)
          b = (int) (((float) n->values[today][T_WORDS] / (float) n->values[today][T_LINES]) * 100.0);
        else
          b = 0;
      }
      if (a < b) {
        again = 1;
        c->snext[today][pitype] = n->snext[today][pitype];
        n->snext[today][pitype] = c;
        if (p == NULL)
          gs->slocal[today][pitype] = n;
        else
          p->snext[today][pitype] = n;
      }
      p = c;
      c = n;
      n = n->snext[today][pitype];
    }
    last = c;
  }
  Context;
  return;
}

static void sortstats_vocables(struct stats_global *gs, int today)
{
  int again = 1;
  struct stats_local *last, *p, *c, *n;
  int a, b, pitype;

  Context;
  again = 1;
  last = NULL;
  countvocables(gs);
  pitype = (T_VOCABLES * (-1)) + TOTAL_TYPES - 1;
  while ((gs->slocal[today][pitype] != last) && (again)) {
    p = NULL;
    c = gs->slocal[today][pitype];
    n = c->snext[today][pitype];
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        a = c->vocables;
        b = n->vocables;
      }
      if (a < b) {
        again = 1;
        c->snext[today][pitype] = n->snext[today][pitype];
        n->snext[today][pitype] = c;
        if (p == NULL)
          gs->slocal[today][pitype] = n;
        else
          p->snext[today][pitype] = n;
      }
      p = c;
      c = n;
      n = n->snext[today][pitype];
    }
    last = c;
  }
  Context;
  return;
}

static void sortstats_word(struct stats_global *gs, int today)
{
  int again = 1;
  struct stats_local *last, *p, *c, *n;
  int a, b, pitype;

  Context;
  debug1("sortstats_word: today == %d", today);
  again = 1;
  last = NULL;
  pitype = (T_WORD * (-1)) + TOTAL_TYPES - 1;
  debug1("pitype: %d", pitype);
  while ((gs->slocal[today][pitype] != last) && (again)) {
    p = NULL;
    c = gs->slocal[today][pitype];
    n = c->snext[today][pitype];
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        if (c->word)
          a = c->word->nr;
        else
          a = 0;
        if (n->word)
          b = n->word->nr;
        else
          b = 0;
      }
      if (a < b) {
        again = 1;
        c->snext[today][pitype] = n->snext[today][pitype];
        n->snext[today][pitype] = c;
        if (p == NULL)
          gs->slocal[today][pitype] = n;
        else
          p->snext[today][pitype] = n;
      }
      p = c;
      c = n;
      n = n->snext[today][pitype];
    }
    last = c;
  }
  Context;
  return;
}

// sort stats by idle-factor (minutes/lines)
static void sortstats_idle(struct stats_global *gs, int today)
{
  int again = 1;
  struct stats_local *last, *p, *c, *n;
  int a, b, pitype;

  Context;
  again = 1;
  last = NULL;
  pitype = (T_IDLE * (-1)) + TOTAL_TYPES - 1;
  while ((gs->slocal[today][pitype] != last) && (again)) {
    p = NULL;
    c = gs->slocal[today][pitype];
    n = c->snext[today][pitype];
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        if (c->values[today][T_LINES] >= min_lines)
          a = (int) (((float) c->values[today][T_MINUTES] / (float) c->values[today][T_LINES]) * 100.0);
        else
          a = 0;
        if (n->values[today][T_LINES] >= min_lines)
          b = (int) (((float) n->values[today][T_MINUTES] / (float) n->values[today][T_LINES]) * 100.0);
        else
          b = 0;
      }
      if (a < b) {
        again = 1;
        c->snext[today][pitype] = n->snext[today][pitype];
        n->snext[today][pitype] = c;
        if (p == NULL)
          gs->slocal[today][pitype] = n;
        else
          p->snext[today][pitype] = n;
      }
      p = c;
      c = n;
      n = n->snext[today][pitype];
    }
    last = c;
  }
  Context;
  return;
}

static void sortwordstats(locstats *ls, globstats *gs)
{
  int again = 1;
  wordstats *last, *p, *c, *n, *tmp;
  int a, b;

  Context;
  again = 1;
  last = NULL;
  if (ls)
    tmp = ls->words;
  else
    tmp = gs->words;
  while ((tmp != last) && (again)) {
    p = NULL;
    if (ls)
      c = ls->words;
    else
      c = gs->words;
    n = c->next;
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        a = c->nr;
        b = n->nr;
      }
      if (a < b) {
  again = 1;
  c->next = n->next;
  n->next = c;
  if (p == NULL) {
    if (ls)
      ls->words = n;
    else
      gs->words = n;
    tmp = n;
  } else
    p->next = n;
      }
      p = c;
      c = n;
      n = n->next;
    }
    last = c;
  }
  Context;
  return;
}

static void sorthosts(struct stats_global *gs)
{
  int again = 1;
  hoststr *last, *p, *c, *n;
  int a, b;

  Context;
  again = 1;
  last = NULL;
  while ((gs->hosts != last) && (again)) {
    p = NULL;
    c = gs->hosts;
    n = c->next;
    again = 0;
    while (n != last) {
      if (!c || !n)
        a = b = 0;
      else {
        a = c->nr;
        b = n->nr;
      }
      if (a < b) {
        again = 1;
        c->next = n->next;
        n->next = c;
        if (p == NULL)
          gs->hosts = n;
        else
          p->next = n;
      }
      p = c;
      c = n;
      n = n->next;
    }
    last = c;
  }
  Context;
  return;
}

static void sort_stats_alphabetically(globstats *gs)
{
  locstats *as, *bs, *l, *last;
  int a, b, again = 1;
  char *astr, *bstr, n[2];

  Context;
  n[0] = n[1] = 0;
  last = NULL;
  while ((gs->local != last) && again) {
    again = 0;
    l = NULL;
    as = gs->local;
    bs = gs->local->next;
    while(bs) {
      if (!as)
        astr = n;
      else
        astr = as->user;
      if (!bs)
        bstr = n;
      else
        bstr = bs->user;
      a = (int) tolower(astr[0]);
      b = (int) tolower(bstr[0]);
      while ((a == b) && a && b) {
	astr++;
	bstr++;
        a = (int) tolower(astr[0]);
        b = (int) tolower(bstr[0]);
      }
      if (a > b) {
        if (!l)
          gs->local = bs;
        else
          l->next = bs;
        as->next = bs->next;
        bs->next = as;
        again = 1;
        if (l == NULL)
          gs->local = bs;
        else
          l = bs;
      }
      l = as;
      as = bs;
      bs = bs->next;
    }
    last = as;
  }
  Context;
}
