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

static void incrstats(char *user, char *chan, int type, int value, int set)
{
  globstats *gs, *gs2;
  locstats *ls, *ls2;
  int i, ii;

  if (type >= TOTAL_TYPES)
    return;
  if (!user) {
    debug0("Stats.mod: incrstats(..) Ups, user is NULL!");
    return;
  }
  if (!chan) {
    debug0("Stats.mod: incrstats(..) Ups, chan is NULL!");
    return;
  }
  for (gs = sdata; gs; gs = gs->next) {
    if (!strcasecmp(chan, gs->chan))
      break;
  }
  if (!gs) {
    gs2 = sdata;
    while (gs2 && gs2->next)
      gs2 = gs2->next;
    gs = nmalloc(sizeof(globstats));
    globstats_init(gs);
    gs->started = now;
    gs->chan = nmalloc(strlen(chan) + 1);
    strcpy(gs->chan, chan);
    if (gs2)
      gs2->next = gs;
    else
      sdata = gs;
  }
  for (ls = gs->local; ls; ls = ls->next) {
    if (!strcasecmp(ls->user, user))
      break;
  }
  if (type == T_GSTARTED) {
    gs->started = value;
    return;
  }
  if (type == T_PEAK) {
    gs->peak[set] = value;
    return;
  }
  if (!ls) {
    ls2 = gs->local;
    while (ls2 && ls2->next)
      ls2 = ls2->next;
    ls = nmalloc(sizeof(locstats));
    locstats_init(ls);
    ls->started = now;
    ls->user = nmalloc(strlen(user) + 1);
    strcpy(ls->user, user);
    if (ls2)
      ls2->next = ls;
    else
      gs->local = ls;
    for (i = 0; i < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); i++)
      ls->snext[S_TOTAL][i] = ls->snext[S_DAILY][i] = ls->snext[S_WEEKLY][i] = ls->snext[S_MONTHLY][i] = NULL;
    for (i = 0; i < 4; i++) {
      for (ii = 0; ii < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); ii++) {
        ls2 = gs->slocal[i][ii];
        while (ls2 && ls2->snext[i][ii])
          ls2 = ls2->snext[i][ii];
        if (ls2)
          ls2->snext[i][ii] = ls;
        else
          gs->slocal[i][ii] = ls;
      }
    }
  }
  if (type == T_LSTARTED)
    ls->started = value;
  else {
    if (set > 0)
      ls->values[set - 1][type] = value;
    else if (set < 0)
      ls->values[(set * (-1)) - 1][type] += value;
    else {
      ls->values[S_TOTAL][type] += value;
      ls->values[S_TODAY][type] += value;
      ls->values[S_WEEKLY][type] += value;
      ls->values[S_MONTHLY][type] += value;
    }
  }
}

static globstats *globstats_create(char *chan)
{
  globstats *gs, *gs2;

  for (gs = sdata; gs; gs = gs->next) {
    if (!rfc_casecmp(chan, gs->chan))
      return gs;
  }
  gs2 = sdata;
  while (gs2 && gs2->next)
    gs2 = gs2->next;
  gs = nmalloc(sizeof(globstats));
  globstats_init(gs);
  gs->started = now;
  gs->chan = nmalloc(strlen(chan) + 1);
  strcpy(gs->chan, chan);
  if (gs2)
    gs2->next = gs;
  else
    sdata = gs;
  return gs;
}

static void nincrstats(locstats *ls, int type, int value)
{
  ls->values[S_TOTAL][type] += value;
  ls->values[S_TODAY][type] += value;
  ls->values[S_WEEKLY][type] += value;
  ls->values[S_MONTHLY][type] += value;
}

/* initstats()
 * erzeugt einen Eintrag in den Statistiken eines Channels
 * für einen User.
 * FRAGE: _Muss_ ich den Rückgabewert irgendwo verwenden, oder
 *		  ist er nur optional?									*/
static locstats *initstats(char *chan, char *user)
{
  globstats *gs, *gs2;
  locstats *ls, *ls2;
  int i, ii;

  gs = sdata;
  while (gs) {
    if (!rfc_casecmp(gs->chan, chan))
      break;
    gs = gs->next;
  }
  if (!gs) {
    gs2 = sdata;
    while (gs2 && gs2->next)
      gs2 = gs2->next;
    gs = nmalloc(sizeof(globstats));
    globstats_init(gs);
    gs->started = now;
    gs->chan = nmalloc(strlen(chan) + 1);
    strcpy(gs->chan, chan);
    if (gs2)
      gs2->next = gs;
    else
      sdata = gs;
  }
  for (ls = gs->local; ls; ls = ls->next) {
    if (!rfc_casecmp(ls->user, user))
      return ls;
  }
  if (!ls) {
    ls2 = gs->local;
    while (ls2 && ls2->next)
      ls2 = ls2->next;
    ls = nmalloc(sizeof(locstats));
    locstats_init(ls);
    ls->started = now;
    ls->user = nmalloc(strlen(user) + 1);
    strcpy(ls->user, user);
    if (ls2)
      ls2->next = ls;
    else
      gs->local = ls;
    for (i = 0; i < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); i++)
      ls->snext[S_TOTAL][i] = ls->snext[S_DAILY][i] = ls->snext[S_WEEKLY][i] = ls->snext[S_MONTHLY][i] = NULL;
    for (i = 0; i < 4; i++) {
      for (ii = 0; ii < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); ii++) {
        ls2 = gs->slocal[i][ii];
        while (ls2 && ls2->snext[i][ii])
          ls2 = ls2->snext[i][ii];
        if (ls2)
          ls2->snext[i][ii] = ls;
        else
          gs->slocal[i][ii] = ls;
      }
    }
  }
  return ls;
}

static void locstats_init(locstats *ls)
{
  int i;

  Assert(ls);
  ls->started = now;
  ls->next = NULL;
  ls->words = NULL;
  ls->tree = NULL;
  ls->quotes = NULL;
  ls->quotefr = 0;
  ls->flag = 0;
  for (i = 0; i < TOTAL_TYPES; i++) {
    ls->values[S_TOTAL][i] = 0;
    ls->values[S_TODAY][i] = 0;
    ls->values[S_WEEKLY][i] = 0;
    ls->values[S_MONTHLY][i] = 0;
  }
  ls->user = NULL;
  ls->u = NULL;
  ls->lastspoke = 0;
  for (i = 0; i < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); i++)
    ls->snext[S_TOTAL][i] = ls->snext[S_DAILY][i] = ls->snext[S_WEEKLY][i] = ls->snext[S_MONTHLY][i] = NULL;
}

static void globstats_init(globstats *gs)
{
  int i;

  Assert(gs);
  gs->next = NULL;
  gs->chan = NULL;
  gs->started = now;
  gs->peak[S_TOTAL] = gs->peak[S_DAILY] = gs->peak[S_WEEKLY] = gs->peak[S_MONTHLY] = 0;
  for (i = 0; i < 24; i++) {
    gs->users[S_USERSUM][i] = 0;
    gs->users[S_USERCOUNTS][i] = -1;
  }
  for (i = 0; i < 24; i++)
    gs->activity[i] = 0;
  gs->local = NULL;
  for (i = 0; i < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); i++)
    gs->slocal[S_TOTAL][i] = gs->slocal[S_DAILY][i] = gs->slocal[S_WEEKLY][i] = gs->slocal[S_MONTHLY][i] = NULL;
  gs->words = NULL;
  gs->topics = NULL;
  gs->hosts = NULL;
  gs->urls = NULL;
  gs->log = gs->lastlog = NULL;
  gs->log_length = 0;
  gs->kicks = NULL;
}

static int getstats(char *user, char *chan, char *type, int today)
{
  struct stats_global *gs = sdata;
  struct stats_local *ls;
  int itype;

  while (gs) {
    ls = gs->local;
    if (!strcasecmp(gs->chan, chan)) {
      while (ls) {
        if (!strcasecmp(ls->user, user)) {
          itype = typetoi(type);
          if (itype >= 0)
            return ls->values[today][itype];
        }
        ls = ls->next;
      }
    }
    gs = gs->next;
  }
  return 0;
}

static void countvocables(globstats *gs)
{
  locstats *ls;
  wordstats *ws;

  for (ls = gs->local; ls; ls = ls->next) {
    ls->vocables = 0;
    for (ws = ls->words; ws; ws = ws->next)
      ls->vocables++;
  }
}

// typetoi(): returns the index of a stat-type
static int typetoi(char *type)
{
  if (!strcasecmp(type, "lstarted"))
    return T_LSTARTED;
  else if (!strcasecmp(type, "gstarted"))
    return T_GSTARTED;
  else if (!strcasecmp(type, "words"))
    return T_WORDS;
  else if (!strcasecmp(type, "letters"))
    return T_LETTERS;
  else if (!strcasecmp(type, "minutes"))
    return T_MINUTES;
  else if (!strcasecmp(type, "topics"))
    return T_TOPICS;
  else if (!strcasecmp(type, "lines"))
    return T_LINES;
  else if (!strcasecmp(type, "actions"))
    return T_ACTIONS;
  else if (!strcasecmp(type, "kicks"))
    return T_KICKS;
  else if (!strcasecmp(type, "modes"))
    return T_MODES;
  else if (!strcasecmp(type, "bans"))
    return T_BANS;
  else if (!strcasecmp(type, "nicks"))
    return T_NICKS;
  else if (!strcasecmp(type, "joins"))
    return T_JOINS;
  else if (!strcasecmp(type, "smileys"))
    return T_SMILEYS;
  else if (!strcasecmp(type, "questions"))
    return T_QUESTIONS;
  else if (!strcasecmp(type, "wpl"))
    return T_WPL;
  else if (!strcasecmp(type, "w/l"))
    return T_WPL;
  else if (!strcasecmp(type, "word"))
    return T_WORD;
  else if (!strcasecmp(type, "vocables"))
    return T_VOCABLES;
  else if (!strcasecmp(type, "started"))
    return T_LSTARTED;
  else if (!strcasecmp(type, "quote"))
    return T_QUOTE;
  else if (!strcasecmp(type, "idle"))
    return T_IDLE;
  else {
    debug1("Stats.mod: Unknown stat type: %s", type);
    return T_ERROR;
  }
}

/* itotype():
 * similar to typetoi(), but returns the string that describes
 * the given type
 */
static char *itotype(int type)
{
  switch (type) {
    case T_WORDS: return "words";
    case T_LETTERS: return "letters";
    case T_MINUTES: return "minutes";
    case T_TOPICS: return "topics";
    case T_LINES: return "lines";
    case T_ACTIONS: return "actions";
    case T_MODES: return "modes";
    case T_BANS: return "bans";
    case T_KICKS: return "kicks";
    case T_NICKS: return "nicks";
    case T_JOINS: return "joins";
    case T_SMILEYS: return "smileys";
    case T_QUESTIONS: return "questions";
    case T_WPL: return "w/l";
    case T_IDLE: return "idle";
    case T_VOCABLES: return "vocables";
  }
  return "!!!ERROR!!!";
}

static locstats *findlocstats(char *chan, char *user)
{
  globstats *gl;
  locstats *ll;

  for (gl = sdata; gl; gl = gl->next) {
    if (!rfc_casecmp(gl->chan, chan))
      break;
  }
  if (!gl)
    return NULL;
  for (ll = gl->local; ll; ll = ll->next) {
    if (!rfc_casecmp(ll->user, user))
      return ll;
  }
  return NULL;
}

static globstats *findglobstats(char *chan)
{
  globstats *gl;

  for (gl = sdata; gl; gl = gl->next) {
    if (!rfc_casecmp(gl->chan, chan))
      break;
  }
  return gl;
}

static void write_stats()
{
  char s[125];
  FILE *f;
  struct stats_global *gs;
  struct stats_local *ls;
  struct stats_userlist *u;
  struct stats_hostlist *h;
  int i;

  Context;
  if (!statsfile[0])
    return;
  sprintf(s, "%s~new", statsfile);
  f = fopen(s, "w");
  chmod(s, statsfilemode);
  if (f == NULL) {
    putlog(LOG_MISC, "*", "ERROR writing stats file.");
    return;
  }
  fprintf(f, "@ # Statistics from %s.\n", botnetnick);
  fprintf(f, "@ filever 3\n");
  fprintf(f, "@ month %d\n", getmonth());
  for (gs = sdata; gs; gs = gs->next) {
    fprintf(f, "%s ! %d\n", gs->chan, (int) gs->started);
    fprintf(f, "%s @ %d\n", gs->chan, gs->peak[S_TOTAL]);
    fprintf(f, "@ peaks %s %d %d %d %d\n", gs->chan, gs->peak[S_TOTAL],
            gs->peak[S_DAILY], gs->peak[S_WEEKLY], gs->peak[S_MONTHLY]);
    for (ls = gs->local; ls; ls = ls->next) {
      fprintf(f, "%s %s %d", gs->chan, ls->user, (int) ls->started);
      for (i = 0; i < TOTAL_TYPES; i++)
        fprintf(f, " %ld", ls->values[S_TOTAL][i]);
      fprintf(f, "\n");
      fprintf(f, "@ lastspoke %d\n", (int) ls->lastspoke);
      fprintf(f, "@ daily %s %s", gs->chan, ls->user);
      for (i = 0; i < TOTAL_TYPES; i++)
        fprintf(f, " %ld", ls->values[S_DAILY][i]);
      fprintf(f, "\n");
      fprintf(f, "@ weekly %s %s", gs->chan, ls->user);
      for (i = 0; i < TOTAL_TYPES; i++)
        fprintf(f, " %ld", ls->values[S_WEEKLY][i]);
      fprintf(f, "\n");
      fprintf(f, "@ monthly %s %s", gs->chan, ls->user);
      for (i = 0; i < TOTAL_TYPES; i++)
        fprintf(f, " %ld", ls->values[S_MONTHLY][i]);
      fprintf(f, "\n");
      i = 0;
    }
  }
  for (u = suserlist; u; u = u->next) {
    fprintf(f, "@ user %s %d %lu %lu", u->user, u->flags, u->created, u->laston);
    for (h = u->hosts; h; h = h->next) {
      fprintf(f, " %s %lu %lu", h->mask, h->lastused, h->created);
    }
    fprintf(f, "\n");
    if (u->password || u->email || u->homepage || u->icqnr) {
      fprintf(f, "@ uxtra %s", u->user);
      if (u->email)
        fprintf(f, " e %s", u->email);
      if (u->homepage)
        fprintf(f, " h %s", u->homepage);
      if (u->icqnr)
        fprintf(f, " i %d", u->icqnr);
      if (u->password)
        fprintf(f, " p %s", u->password);
      fprintf(f, "\n");
    }
  }
  fclose(f);
  unlink(statsfile);
  movefile(s, statsfile);
  Context;
  return;
}

static void read_stats()
{
  FILE *f;
  char buf[SAVESTATSLENGTH + 1];
  char *s, *chan, *user, *cmd, *host, *tmp;
  int i, version, range, month, list, addhosts, flags;
  struct stats_userlist *u;
  time_t lastused;
  locstats *ls;
  globstats *gs;
  time_t created, laston;

  Context;
  ls = NULL;
  gs = NULL;
  version = 0;
  month = 0;
  f = fopen(statsfile, "r");
  if (f == NULL) {
    putlog(LOG_MISC, "*", "ERROR reading stats file");
    return;
  }
  free_stats();
  while (!feof(f)) {
    buf[0] = 0;
    s = buf;
    fgets(s, SAVESTATSLENGTH - 1, f);
    s[SAVESTATSLENGTH - 1] = 0;
    if (buf[0] == 0)
      continue;
    if (s[strlen(s) - 1] == '\n')
      s[strlen(s) - 1] = 0;
    chan = newsplit(&s);
    if (!strcmp(chan, "@")) {
      cmd = newsplit(&s);
      if (!strcmp(cmd, "filever"))
	version = atoi(newsplit(&s));
      else if (!strcmp(cmd, "month"))
        month = atoi(newsplit(&s));
      else if (!strcmp(cmd, "peaks")) {
        chan = newsplit(&s);
        incrstats("*", chan, T_PEAK, atoi(newsplit(&s)), S_TOTAL);
        incrstats("*", chan, T_PEAK, atoi(newsplit(&s)), S_DAILY);
        incrstats("*", chan, T_PEAK, atoi(newsplit(&s)), S_WEEKLY);
        incrstats("*", chan, T_PEAK, atoi(newsplit(&s)), S_MONTHLY);
      } else if (!strcmp(cmd, "daily") || !strcmp(cmd, "weekly")
                 || !strcmp(cmd, "monthly")) {
        if (!strcmp(cmd, "daily"))
	  range = S_DAILY;
        else if (!strcmp(cmd, "weekly"))
	  range = S_WEEKLY;
        else if (!strcmp(cmd, "monthly"))
          range = S_MONTHLY;
        else {
          debug2("Error while reading statsfile: range uninitialized! (%s %s)", cmd, s);
          continue;
        }
        if ((range == S_MONTHLY) && (month != lastmonth))
          continue;
        chan = newsplit(&s);
        user = newsplit(&s);
        // Check if pointers still point to the correct data and
        // update them, if not.
        if ((gs && strcmp(gs->chan, chan)) || !gs) {
          gs = findglobstats(chan);
          ls = findlocstats(chan, user);
        } else {
          if ((ls && strcmp(ls->user, user)) || !ls)
            ls = findlocstats(chan, user);
        }
        if (!ls)
          ls = initstats(chan, user);
        for (i = 0; i < TOTAL_TYPES; i++)
          ls->values[range][i] = atoi(newsplit(&s));
      } else if (!strcmp(cmd, "lastspoke")) {
	if (ls) {
	  ls->lastspoke = atoi(newsplit(&s));
	} else {
	  putlog(LOG_MISC, "*", "ERROR: Can't load lastspoke info. No locstats.");
	}
      } else if (!strcmp(cmd, "user")) {
	user = newsplit(&s);
	flags = 0;
	if (version < 2) {
	  list = atoi(newsplit(&s));
	  addhosts = atoi(newsplit(&s));
	  if (list)
	    flags |= S_LIST;
	  if (addhosts)
	    flags |= S_ADDHOSTS;
	} else
	  flags = atoi(newsplit(&s));
	if (version >= 3) {
	  created = atoi(newsplit(&s));
	  laston = atoi(newsplit(&s));
	} else {
	  created = get_creation_time_from_locstats(user);
	  laston = get_laston_time_from_hosts(user);
	}
	u = addsuser(user, created, laston);
	u->flags = flags;
	while (s[0]) {
	  host = newsplit(&s);
	  lastused = (time_t) atoi(newsplit(&s));
	  if (version >= 3)
	    created = (time_t) atoi(newsplit(&s));
	  else
	    created = lastused;
	  saddhost(u, host, lastused, created);
	}
      } else if (!strcmp(cmd, "uxtra")) {
        user = newsplit(&s);
        u = findsuser_by_name(user);
        while (u && s[0]) {
          tmp = newsplit(&s);
          if (!strcmp(tmp, "e"))
            setemail(u, newsplit(&s));
          else if (!strcmp(tmp, "h"))
            sethomepage(u, newsplit(&s));
          else if (!strcmp(tmp, "i"))
            u->icqnr = atoi(newsplit(&s));
          else if (!strcmp(tmp, "p"))
            setpassword(u, newsplit(&s));
        }
      }
    } else {
      // old style data
      // left-over from v1.0. I should change it, but I don't want
      // to break compatibility
      user = newsplit(&s);
      if (!strcmp(user, "!"))
        incrstats(user, chan, T_GSTARTED, atoi(newsplit(&s)), 1);
      else if (!strcmp(user, "@"))
        incrstats(user, chan, T_PEAK, atoi(newsplit(&s)), S_TOTAL);
      else {
        incrstats(user, chan, T_LSTARTED, atoi(newsplit(&s)), 1);
        // initstats also returns the current 'ls' if it also exists,
        // so better don't even use findlocstats() before to save
        // some CPU-time
        ls = initstats(chan, user);
        for (i = 0; i < TOTAL_TYPES; i++)
          ls->values[S_TOTAL][i] = atoi(newsplit(&s));
      }
    }
  }
  fclose(f);
  Context;
  return;
}

static void reset_tstats()
{
  globstats *gs;
  locstats *ls;
  int i;

  Context;
  putlog(LOG_MISC, "*", "Stats.mod: Resetting today's statistics...");
  for (gs = sdata; gs; gs = gs->next) {
    gs->peak[S_TODAY] = 0;
    free_wordstats(gs->words);
    gs->words = NULL;
    free_topics(gs->topics);
    gs->topics = NULL;
    free_urls(gs->urls);
    gs->urls = NULL;
    free_hosts(gs->hosts);
    gs->hosts = NULL;
    free_kicks(gs->kicks);
    gs->kicks = NULL;
    for (ls = gs->local; ls; ls = ls->next) {
      free_wordstats(ls->words);
      ls->words = NULL;
      ls->tree = NULL;
      free_quotes(ls->quotes);
      ls->quotes = NULL;
      for (i = 0; i < TOTAL_TYPES; i++)
        ls->values[S_TODAY][i] = 0;
    }
  }
  Context;
}

static void reset_mwstats(int range)
{
  globstats *gs;
  locstats *ls;
  int i;

  Context;
  putlog(LOG_MISC, "*", "Stats.mod: Resetting %s statistics...", (range == S_WEEKLY) ? "weekly" : "monthly");
  for (gs = sdata; gs; gs = gs->next) {
    gs->peak[range] = 0;
    for (ls = gs->local; ls; ls = ls->next) {
      for (i = 0; i < TOTAL_TYPES; i++)
        ls->values[range][i] = 0;
    }
  }
  Context;
}

static void resetlocstats(locstats *ls)
{
  int i;

  if (!ls) {
    debug0("ERROR! resetlocstats called with NULL pointer!");
    return;
  }
  for (i = 0; i < TOTAL_TYPES; i++) {
    ls->values[S_TOTAL][i] = 0;
    ls->values[S_TODAY][i] = 0;
    ls->values[S_WEEKLY][i] = 0;
    ls->values[S_MONTHLY][i] = 0;
  }
  return;
}

static void calcwordstats(locstats *stats, char *text)
{
  char *word;
  int i;

  Context;
  Assert(stats);
  if (!log_wordstats)
    return;
  for (i = 0; i < strlen(text); i++)
    if (strchr("!?.,\"<>&\\", text[i]))
      text[i] = ' ';
  while (text[0]) {
    word = newsplit(&text);
    strlower(word);
    incrwordstats(stats, word, 1, 0);
  }
}

// add another entry to the tree
static void incrwordstats(locstats *ls, char *word, int value, int set)
{
  wordstats *ne, *te, *le;
  wordstats *ll;
  int cmp;

  Context;
  if ((word[0] == ' ') || !word[0])
    return;
  if (min_word_length && (strlen(word) < min_word_length))
    return;     /* only log words that are longer than min_word_length chars */
  if (!ls) {
    return;
  }
  // at first, check if it already exists and only needs to be updated
  te = ls->tree;
  le = NULL;
  while (te) {
    if (!(cmp = strcasecmp(te->word, word)))
      break;
    le = te;
    if (cmp < 0)
      te = te->left;
    else
      te = te->right;
  }
  if (!te) { // nothing to update, so let's append a new node
    ne = nmalloc(sizeof(struct stats_words));
    ne->word = nmalloc(strlen(word) + 1);
    strcpy(ne->word, word);
    ne->nr = 0;
    ne->left = ne->right = ne->next = NULL;
    if (!le)  // no last entry -> new entry is going to be the crown
      ls->tree = ne;
    else {
      if (strcasecmp(le->word, word) < 0)  // -1 -> left child
        le->left = ne;
      else                                  // 1 -> right child
        le->right = ne;
    }
    // now let's add it also to the linked list (needed for sorting)
    ll = ls->words;
    while (ll && ll->next)
      ll = ll->next;
    if (ll)
      ll->next = ne;
    else
      ls->words = ne;
    te = ne;
  }
  // now let's set the value
  if (set)
    te->nr = value;
  else
    te->nr += value;
}

static void nincrwordstats(globstats *gs, char *word, int value)
{
  wordstats *l, *ll;

  for (l = gs->words; l; l = l->next)
    if (!strcmp(word, l->word))
      break;
  if (!l) {
    l = gs->words;
    while (l && l->next)
      l = l->next;
    ll = nmalloc(sizeof(wordstats));
    ll->word = nmalloc(strlen(word) + 1);
    strcpy(ll->word, word);
    ll->nr = 0;
    ll->next = NULL;
    if (l)
      l->next = ll;
    else
      gs->words = ll;
    l = ll;
  }
  l->nr += value;
}

static time_t glob_lastglobwordstats;
static void do_globwordstats(globstats *gs)
{
  wordstats *l;
  locstats *ls;

  if (glob_lastglobwordstats == now)
    return; /* don't recalculate everything if we already did it in this second */
  debug0("calculating global wordstats");
  glob_lastglobwordstats = now;
  for (l = gs->words; l; l = l->next)
    l->nr = 0;
  for (ls = gs->local; ls; ls = ls->next)
    for (l = ls->words; l; l = l->next)
      nincrwordstats(gs, l->word, l->nr);
  sortwordstats(NULL, gs);
}

static void addquote(locstats *stats, char *quote)
{
  quotestr *l, *nl;

  Assert(quote);
  if (!quote_freq)
    return;
  Assert(stats);
  stats->quotefr--;
  if (stats->quotefr > 0)
    return;
  stats->quotefr = quote_freq;
  l = stats->quotes;
  while (l && l->next)
    l = l->next;
  nl = nmalloc(sizeof(quotestr));
  nl->next = NULL;
  nl->quote = nmalloc(strlen(quote) + 1);
  strcpy(nl->quote, quote);
  if (l)
    l->next = nl;
  else
    stats->quotes = nl;
}

static void addtopic(char *channel, char *topic, char *by)
{
  topicstr *e, *ne;
  globstats *gs;

  Context;
  gs = findglobstats(channel);
  if (!gs)
    return;
  for (e = gs->topics; e; e = e->next)
    if (!strcasecmp(topic, e->topic))
      return;
  e = gs->topics;
  while (e && e->next)
    e = e->next;
  ne = nmalloc(sizeof(topicstr));
  ne->topic = nmalloc(strlen(topic) + 1);
  strcpy(ne->topic, topic);
  ne->by = nmalloc(strlen(by) + 1);
  strcpy(ne->by, by);
  ne->when = now;
  ne->next = NULL;
  if (e)
    e->next = ne;
  else
    gs->topics = ne;
}

static void addhost(char *host, globstats *gs)
{
  hoststr *e, *ne;
  char *s;

  if (!gs || !host)
    return;
  s = strchr(host, '@');
  if (s)
    host = s + 1;
  if (strcmp(host, "[IP]"))
    strlower(host);
  for (e = gs->hosts; e; e = e->next) {
    if (!strcmp(host, e->host)) {
      e->nr++;
      return;
    }
  }
  e = gs->hosts;
  while (e && e->next)
    e = e->next;
  ne = nmalloc(sizeof(hoststr));
  ne->host = nmalloc(strlen(host) + 1);
  strcpy(ne->host, host);
  ne->nr = 1;
  ne->next = NULL;
  if (e)
    e->next = ne;
  else
    gs->hosts = ne;
  return;
}

static void setword(globstats *gs, char *word)
{
  locstats *l;
  wordstats *w;

  for (l = gs->local; l; l = l->next) {
    l->word = NULL;
    for (w = l->words; w; w = w->next) {
      if (!strcmp(w->word, word)) {
        l->word = w;
        break;
      }
    }
  }
}

static int track_stat_user(char *oldnick, char *newnick)
{
  globstats *gs;
  locstats *ls;
  struct stats_userlist *u;
  int found = 0;

  Context;
  for (gs = sdata; gs; gs = gs->next) {
    for (ls = gs->local; ls; ls = ls->next) {
      if (!rfc_casecmp(oldnick, ls->user) && strcmp(newnick, ls->user)) {
        nfree(ls->user);
        ls->user = nmalloc(strlen(newnick) + 1);
        strcpy(ls->user, newnick);
        // ls->u should still be valid...
        found = 1;
        debug3("Transferred stats from %s to %s in %s", oldnick, newnick, gs->chan);
      }
    }
  }
  for (u = suserlist; u; u = u->next) {
    if (!rfc_casecmp(oldnick, u->user) && strcmp(newnick, u->user)) {
      nfree(u->user);
      u->user = nmalloc(strlen(newnick) + 1);
      strcpy(u->user, newnick);
      found = 1;
      debug2("Changed user name from %s to %s in my local database.", oldnick, newnick);
    }
  }
  if (found)
    return 1;
  return 0;
}

static void check_for_url(char *user, char *chan, char *text)
{
  char *p, *url;
  char *tmp, *tmp2, *t;
  struct stats_url *e, *ne;
  globstats *gs;
  int weiter;

  if (log_urls < 1)
    return;
  gs = findglobstats(chan);
  if (!gs)
    return;
  url = p = tmp = tmp2 = t = NULL;
  if ((p = strstr(text, "http://")))
    url = newsplit(&p);
  else if ((p = strstr(text, "ftp://")))
    url = newsplit(&p);
  else if (strstr(text, "www.") || strstr(text, ".com") || strstr(text, "ftp.")) {
    tmp = nmalloc(strlen(text) + 1);
    strcpy(tmp, text);
    t = tmp;
    while (t[0]) {
      p = newsplit(&t);
      if (strstr(p, "www.") || strstr(p, ".com") || strstr(text, "ftp.")) {
        url = p;
        break;
      }
    }
  }
  if (!url)
    return;
  if (strchr(url, '@')) { /* probably an email address or something similar */
    if (tmp)
      nfree(tmp);
    return;
  }
  if (strncasecmp(url, "http://", 7) && strncasecmp(url, "ftp://", 6)) {
    if (!strncasecmp(url, "ftp.", 4)) {
      tmp2 = nmalloc(strlen(url) + 6 + 1);
      strcpy(tmp2, "ftp://");
      strcpy(tmp2 + 6, url);
    } else {
      tmp2 = nmalloc(strlen(url) + 7 + 1);
      strcpy(tmp2, "http://");
      strcpy(tmp2 + 7, url);
    }
    url = tmp2;
  }
  for (e = gs->urls; e; e = e->next) {
    if (!strcmp(e->url, url)) {
      nfree(e->by);
      e->by = nmalloc(strlen(user) + 1);
      strcpy(e->by, user);
      e->when = now;
      if (tmp)
        nfree(tmp);
      if (tmp2)
        nfree(tmp2);
      return;
    }
  }
  weiter = 1;
  for (p = url; (p != url) && weiter; p--) {
    if (strchr(".!?,#", p[0]))
      p[0] = 0;
    else
      weiter = 0;
  }
  for (e = gs->urls; e && e->next; e = e->next);
  ne = nmalloc(sizeof(struct stats_url));
  ne->url = nmalloc(strlen(url) + 1);
  strcpy(ne->url, url);
  ne->by = nmalloc(strlen(user) + 1);
  strcpy(ne->by, user);
  ne->when = now;
  ne->next = NULL;
  if (e)
    e->next = ne;
  else
    gs->urls = ne;
  if (tmp)
    nfree(tmp);
  if (tmp2)
    nfree(tmp2);
  debug2("Logged URL: \"%s\" mentioned by %s.", ne->url, ne->by);
}

static void add_chanlog(globstats *gs, char *nick, char *text, int type)
{
  char ts[20];
  time_t tt, ttbuf;
  quotestr *newlog, *l;

  if (!gs || (kick_context < 1))
    return;
  Assert(nick);
  Assert(text);
  ttbuf = tt = now;
  strftime(ts, 19, "[%H:%M:%S]", localtime(&tt));
  newlog = nmalloc(sizeof(quotestr));
  newlog->next = NULL;
  if (type == SL_PRIVMSG) {
    newlog->quote = nmalloc(strlen(nick) + strlen(ts) + strlen(text) + 5);
    sprintf(newlog->quote, "%s <%s> %s", ts, nick, text);
  } else if (type == SL_KICK) {
    newlog->quote = nmalloc(strlen(text) + strlen(ts) + 2);
    sprintf(newlog->quote, "%s %s", ts, text);
  } else if (type == SL_MODE) {
    newlog->quote = nmalloc(strlen(ts) + strlen(text) + 2);
    sprintf(newlog->quote, "%s %s", ts, text);
  } else if (type == SL_NICK) {
    newlog->quote = nmalloc(strlen(ts) + strlen(nick) + strlen(text) + 19);
    sprintf(newlog->quote, "%s %s changed nick to %s", ts, nick, text);
  } else if (type == SL_PART) {
    newlog->quote = nmalloc(strlen(ts) + strlen(nick) + strlen(gs->chan) + 12);
    sprintf(newlog->quote, "%s %s has left %s", ts, nick, gs->chan);
  } else if (type == SL_JOIN) {
    newlog->quote = nmalloc(strlen(ts) + strlen(nick) + strlen(gs->chan) + 14);
    sprintf(newlog->quote, "%s %s has joined %s", ts, nick, gs->chan);
  } else if (type == SL_QUIT) {
    newlog->quote = nmalloc(strlen(ts) + strlen(nick) + strlen(text) + 18);
    sprintf(newlog->quote, "%s %s has quit IRC (%s)", ts, nick, text);
  } else {
    debug1("Unknown log-type: %d !!!", type);
    newlog->quote = nmalloc(1);
    newlog->quote[0] = 0;
  }
  if (gs->lastlog)
    gs->lastlog->next = newlog;
  else
    gs->log = newlog;
  gs->lastlog = newlog;
  gs->log_length++;
  while ((gs->log_length > kick_context) && (gs->log_length > 0)) {
    l = gs->log->next;
    Assert(gs->log->quote);
    nfree(gs->log->quote);
    if (gs->lastlog == gs->log)
      gs->lastlog = NULL;
    nfree(gs->log);
    gs->log = l;
    gs->log_length--;
  }
  ctime(&ttbuf); /* workaround for a bug in older eggdrops */
}

static void save_kick(globstats *gs, char *kick)
{
  struct stats_kick *k, *nk;
  quotestr *log, *l, *nl;

  Context;
  if (!gs)
    return;
  for (k = gs->kicks; k && k->next; k = k->next);
  nk = nmalloc(sizeof(struct stats_kick));
  nk->next = NULL;
  nk->log = NULL;
  if (!gs->log || (kick_context < 1)) {
    nl = nmalloc(sizeof(quotestr));
    nl->quote = nmalloc(strlen(kick) + 1);
    strcpy(nl->quote, kick);
    nl->next = NULL;
    nk->log = nl;
  } else {
    for (log = gs->log; log; log = log->next) {
      nl = nmalloc(sizeof(quotestr));
      nl->quote = nmalloc(strlen(log->quote) + 1);
      strcpy(nl->quote, log->quote);
      nl->next = NULL;
      for (l = nk->log; l && l->next; l = l->next);
      if (l)
        l->next = nl;
      else
        nk->log = nl;
    }
  }
  if (k)
    k->next = nk;
  else
    gs->kicks = nk;
  debug1("Logged kick in %s.", gs->chan);
}

static int getplace(globstats *gs, int today, int itype, char *user)
{
  locstats *ls;
  int place = 0;

  // if itype is < 0, get the modified itype that we need for accessing the data
  if (itype < 0)
    itype = (TOTAL_TYPES - 1) + (itype * -1);
  for (ls = gs->slocal[today][itype]; ls; ls = ls->snext[today][itype]) {
    if (!listsuser(ls, gs->chan))
      continue;
    place++;
    if (!rfc_casecmp(ls->user, user))
      return place;
  }
  return 0;
}

static int countstatmembers(globstats *gs)
{
  int members = 0;
  locstats *ls;

  Context;
  for (ls = gs->local; ls; ls = ls->next) {
    if (listsuser(ls, gs->chan))
      members++;
  }
  return members;
}

static int countallstatmembers(globstats *gs)
{
  int members = 0;
  locstats *ls;

  Context;
  for (ls = gs->local; ls; ls = ls->next)
      members++;
  return members;
}

/* countactivestatmembers():
 * counts all active members in a chan (and skips dead entries)
 */
static int countactivestatmembers(globstats *gs, int listable, int today, int type, int min)
{
  int members = 0;
  locstats *ls;

  Context;
  for (ls = gs->local; ls; ls = ls->next) {
    if (ls->values[today][type] < min)
      continue;
    if (listable && !listsuser(ls, gs->chan))
      continue;
    members++;
  }
  return members;
}

static int gettotal(globstats *gs, int type, int today)
{
  int total = 0;
  locstats *ls;

  for (ls = gs->local;ls; ls = ls->next) {
    if (!ls->u)
      ls->u = findsuser_by_name(ls->user);
    if (ls->u && !suser_list(ls->u))
      continue;
    total += ls->values[today][type];
  }
  return total;
}

static void free_stats()
{
  struct stats_global *sl;

  Context;
  while (sdata) {
    sl = sdata->next;
    free_localstats(sdata->local);
    free_wordstats(sdata->words);
    free_topics(sdata->topics);
    free_urls(sdata->urls);
    free_quotes(sdata->log);
    free_hosts(sdata->hosts);
    free_kicks(sdata->kicks);
    nfree(sdata->chan);
    nfree(sdata);
    sdata = sl;
  }
  sdata = NULL;
  llist_free(&schanset);
//  schans = NULL;
  free_suserlist(suserlist);
  suserlist = NULL;
  slang_glob_free();
  Context;
  return;
}

static void free_localstats(struct stats_local *sl)
{
  struct stats_local *sll;

  Context;
  while (sl) {
    Context;
    sll = sl->next;
    free_wordstats(sl->words);
    free_quotes(sl->quotes);
    nfree(sl->user);
    nfree(sl);
    sl = sll;
    Context;
  }
  Context;
  return;
}

static void free_wordstats(wordstats *l)
{
  wordstats *ll;

  Context;
  while (l) {
    ll = l->next;
    nfree(l->word);
    nfree(l);
    l = ll;
  }
  return;
}

static void free_quotes(quotestr *l)
{
  quotestr *ll;

  Context;
  while (l) {
    ll = l->next;
    nfree(l->quote);
    nfree(l);
    l = ll;
  }
  return;
}

static void free_topics(topicstr *e)
{
  topicstr *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->topic);
    nfree(e->by);
    nfree(e);
    e = ee;
  }
  return;
}

static void free_urls(struct stats_url *e)
{
  struct stats_url *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->url);
    nfree(e->by);
    nfree(e);
    e = ee;
  }
  return;
}

static void free_kicks(struct stats_kick *e)
{
  struct stats_kick *ee;

  Context;
  while (e) {
    ee = e->next;
    free_quotes(e->log);
    nfree(e);
    e = ee;
  }
}

static void free_hosts(hoststr *e)
{
  hoststr *ee;

  Context;
  while (e) {
    ee = e->next;
    nfree(e->host);
    nfree(e);
    e = ee;
  }
  return;
}

static int userdata_merge(char *sTo, char *sFrom)
{
	globstats *gs;
	locstats *ls;
	int i, found = 0;

	for (gs = sdata; gs; gs = gs->next) {
		for (ls = gs->local; ls; ls = ls->next) {
			if (!strcasecmp(ls->user, sFrom)) {
				found = 1;
				for (i = 0; i < TOTAL_TYPES; i++) {
					incrstats(sTo, gs->chan, i, ls->values[S_TOTAL][i], (S_TOTAL + 1) * (-1));
					incrstats(sTo, gs->chan, i, ls->values[S_DAILY][i], (S_DAILY + 1) * (-1));
					incrstats(sTo, gs->chan, i, ls->values[S_WEEKLY][i], (S_WEEKLY + 1) * (-1));
					incrstats(sTo, gs->chan, i, ls->values[S_MONTHLY][i], (S_MONTHLY + 1) * (-1));
				}
				if (getstats(sTo, "", "gstarted", 0) < getstats(sFrom, "", "gstarted", 0))
					incrstats(sTo, "", T_LSTARTED, getstats(sFrom, "", "gstarted", 0), 1);
			}
		}
	}
	return found;
}
