/*
 * Copyright (C) 2001  Florian Sander
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

static float glob_r, glob_g, glob_b, glob_rstep, glob_gstep, glob_bstep;

#define CF_TOP -1
#define CF_TOPICS -2
#define CF_MISCFACTS -3
#define CF_USERS -4
#define CF_ONCHAN -5


/* template_init_colorfade()
 * see template_add_cmd_init_colorfade for description
 */
static void template_init_colorfade(int idx, struct template_content *htpc)
{
  int wert, steps;
  float r2, b2, g2;
  memberlist *m;
  struct chanset_t *chan;
  topicstr *topic;
  int ret;

  // find out how many steps the color fade will have
  steps = htpc->intpar1;
  // if it has 0 steps, then the number of steps wasn't specified as parameter,
  // so we'll just use the number of results
  if (steps == CF_TOP)
    steps = webnr;
  else if (steps == CF_MISCFACTS) {
    steps = 0;
    for (ret = selectfirstfact(); ret; ret = selectnextfact())
      steps++;
  } else if ((steps == CF_USERS) && glob_globstats)
    steps = countallstatmembers(glob_globstats);
  else if ((steps == CF_ONCHAN) && glob_globstats) {
    steps = 0;
    chan = findchan_by_dname(glob_globstats->chan);
    if (chan)
      for (m = chan->channel.member; m && m->nick[0]; m = m->next)
        steps++;
  } else if ((steps == CF_TOPICS) && glob_globstats) {
    steps = 0;
    for (topic = glob_globstats->topics; topic; topic = topic->next)
      steps++;
  } else if (steps < 1)
    steps = 1;
  // split our r/g/b values of the starting color (stored in intpar1)
  wert = htpc->floatpar1;
  glob_b = wert & 0xff; glob_g = (wert & 0xff00) >> 8; glob_r = (wert & 0xff0000) >> 16;
  // now do the same with the target color (intpar2)
  wert = htpc->floatpar2;
  b2 = wert & 0xff; g2 = (wert & 0xff00) >> 8; r2 = (wert & 0xff0000) >> 16;
  // finally, determine the "length" of a step between colors
  glob_rstep = (r2 - glob_r) / steps;
  glob_gstep = (g2 - glob_g) / steps;
  glob_bstep = (b2 - glob_b) / steps;
  // all global variables are now initialized and can be used.
}

/* template_add_cmd_init_colorfade():
 * Parameters: <startcolor> <endcolor> [steps]
 * initialiazes a color-fade from <startcolor> to <endcolor>
 * in [steps] steps. (if steps isn't defined, numresults is used)
 */
static void template_add_cmd_init_colorfade(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  char *startcolor, *endcolor, *steps;
  float istartcolor, iendcolor, isteps;

  Context;
  startcolor = endcolor = steps = "";
  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "startcolor"))
      startcolor = params->s2;
    else if (!strcasecmp(params->s1, "endcolor"))
      endcolor = params->s2;
    else if (!strcasecmp(params->s1, "steps"))
      steps = params->s2;
    else
      putlog(LOG_MISC, "*", "ERROR parsing templates: Unknown parameter '%s' "
             "for command init_color_fade.", params->s1);
  }
  istartcolor = strtol(startcolor, NULL, 0);
  iendcolor = strtol(endcolor, NULL, 0);
  if (!istartcolor)
    istartcolor = table_color;
  if (!iendcolor)
    iendcolor = fade_table_to;
  if (!strcasecmp(steps, "toplist"))
    isteps = CF_TOP;
  else if (!strcasecmp(steps, "topics"))
    isteps = CF_TOPICS;
  else if (!strcasecmp(steps, "miscfacts"))
    isteps = CF_MISCFACTS;
  else if (!strcasecmp(steps, "users"))
    isteps = CF_USERS;
  else if (!strcasecmp(steps, "onchan"))
    isteps = CF_ONCHAN;
  else
    isteps = strtol(steps, NULL, 0);
  // now write a pointer to the executing command and all parameters into our
  // content structure
  h_tpc->command = template_init_colorfade;
  h_tpc->floatpar1 = istartcolor;
  h_tpc->floatpar2 = iendcolor;
  h_tpc->intpar1 = isteps;
}

/* template_send_fcolor():
 * outputs the current color-code
 */
static void template_send_fcolor(int idx, struct template_content *htpc)
{
  dprintf(idx, "#%02x%02x%02x", (int) glob_r, (int)  glob_g, (int) glob_b);
}

/* template_fade_color():
 * fades the color one step further
 */
static void template_fade_color(int idx, struct template_content *htpc)
{
  glob_r += glob_rstep;
  glob_g += glob_gstep;
  glob_b += glob_bstep;
}

/* <?chanlist ...?>
 * outputs the list of channels
 */
static void template_send_chanlist(int idx, struct template_content *h_tpc)
{
  globstats *gs;

  Context;
  for (gs = sdata; gs; gs = gs->next) {
	if (!egg_chan_active(gs->chan))
	  continue;
/*    if (!list_secret_chans && secretchan(gs->chan))
      continue; */
/*    if (inactivechan(gs->chan))
      continue;
      */
    glob_globstats = gs;
    debug0("lang für skins nicht vergessen!");
    templates_content_send(h_tpc->subcontent, idx);
  }
}

/* <?chan?>
 * outputs the current chan
 */

static void template_send_chan(int idx, struct template_content *tpc)
{
  char *chan;

  if (glob_globstats) {
    if (tpc->floatpar1 && glob_globstats->chan[0] == '#')
      chan = glob_globstats->chan + 1;
    else
      chan = glob_globstats->chan;
    dprintf(idx, "%s", tpc->intpar1 ? encode_url(chan) : chan);
  }
}

/* <?encoded_chan?>
 * outputs the current channel, but in an encoded form, so that
 * it can be used in URLs
 */
/*
static void template_send_encoded_chan(int idx, struct template_content *tpc)
{
  if (glob_globstats) {
    if (glob_globstats->chan[0] == '#')
      dprintf(idx, "%s", encode_url(glob_globstats->chan + 1));
    else
      dprintf(idx, "%s", encode_url(glob_globstats->chan));
  }
}
*/

/* <?topnr?>
 * outputs how many users get listed in the topX
 */
static void template_send_topnr(int idx, struct template_content *tpc)
{
  dprintf(idx, "%d", webnr);
}

/* <?current_topic?>
 * outputs the topic of the current chan (if there is any)
 */
static void template_send_current_topic(int idx, struct template_content *tpc)
{
  struct chanset_t *chan;

  if (glob_globstats) {
    chan = findchan_by_dname(glob_globstats->chan);
    if (chan && chan->channel.topic)
      dprintf(idx, "%s", text2html(chan->channel.topic));
  }
}

/* <?if_total ...?>
 * <?if_daily ...?>
 * <?if_weekly ...?>
 * <?if_monthly ...?>
 * outputs its subcontent if the current timerange is
 * TOTAL/DAILY/WEEKLY/MONTHLY
 */
static void template_send_if_total(int idx, struct template_content *h_tpc)
{
  if (glob_timerange == S_TOTAL)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_daily(int idx, struct template_content *h_tpc)
{
  if (glob_timerange == S_DAILY)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_weekly(int idx, struct template_content *h_tpc)
{
  if (glob_timerange == S_WEEKLY)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_monthly(int idx, struct template_content *h_tpc)
{
  if (glob_timerange == S_MONTHLY)
    templates_content_send(h_tpc->subcontent, idx);
}

/* <?topstats ...?>
 * sends the list of stats which should be shown in the topX
 */
static void template_send_topstats(int idx, struct template_content *h_tpc)
{
  char buf[512], *pbuf;

  Context;
  strncpy(buf, webstats, sizeof(buf));
  pbuf = buf;
  while (pbuf[0]) {
    glob_toptype = newsplit(&pbuf);
    templates_content_send(h_tpc->subcontent, idx);
  }
}

/* <?toplist ...?>
 * sends the top X users
 */
static void template_send_toplist(int idx, struct template_content *h_tpc)
{
  locstats *ls;
  int sort;

  Context;
  Assert((glob_timerange >= 0) && (glob_timerange <= 3));
  if (glob_sorting == T_ERROR) {
    dprintf(idx, "<H1>ERROR! Invalid sorting!</H1>");
    return;
  }
  if (glob_sorting < 0)
    sort = (glob_sorting * -1) + (TOTAL_TYPES - 1);
  else
    sort = glob_sorting;
  debug0("überprüfen, ob toplist rekursions-fähig ist");
  glob_place = 0;
  for (ls = glob_globstats->slocal[glob_timerange][sort]; ls; ls = ls->snext[glob_timerange][sort]) {
    if (glob_place > glob_top_end)
      break;
    // skip users who shouldn't be listed (.schattr user -list)
    if (!listsuser(ls, glob_globstats->chan))
      continue;
    // break if the value is 0, because we probably reached the end
    // of the sorted list (who want's to see the stats of 1000 users who
    // don't have any stats in the sorted value?)
    if ((glob_sorting >= 0) && !ls->values[glob_timerange][glob_sorting])
      break;
    glob_place++;
    if (glob_place < glob_top_start)
      continue;
    glob_locstats = ls;
    if (!ls->u)
      ls->u = findsuser_by_name(ls->user);
    glob_user = ls->u;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_type(int idx, struct template_content *h_tpc)
{
  if (h_tpc->intpar1)
    dprintf(idx, "%s", getslangtype(glob_toptype));
  else
    dprintf(idx, "%s", glob_toptype);
}

static void template_add_cmd_type(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  Context;
  for (; params; params = params->next)
    if (!strcasecmp(params->s1, "slang") && !strcasecmp(params->s2, "yes"))
      h_tpc->intpar1 = 1;
}

static void template_send_place(int idx, struct template_content *h_tpc)
{
  dprintf(idx, "%d", glob_place);
}

static void template_add_cmd_chan_user(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  Context;
  for (; params; params = params->next)
    if (!strcasecmp(params->s1, "encode") && !strcasecmp(params->s2, "yes"))
      h_tpc->intpar1 = 1;
    else if (!strcasecmp(params->s1, "short") && !strcasecmp(params->s2, "yes"))
      h_tpc->floatpar1 = 1.0;
}

static void template_send_user(int idx, struct template_content *h_tpc)
{
  if (glob_locstats)
    dprintf(idx, "%s", h_tpc->intpar1 ? encode_url(glob_locstats->user) : glob_locstats->user);
  else if (glob_user)
    dprintf(idx, "%s", h_tpc->intpar1 ? encode_url(glob_user->user) : glob_user->user);
}

static void template_send_value(int idx, struct template_content *h_tpc)
{
  int i;
  if (!glob_locstats || (glob_timerange == T_ERROR) || !glob_toptype) {
    if (!glob_locstats)
      debug0("no locstats");
    if (!glob_timerange == T_ERROR)
      debug0("no timerange");
    if (!glob_toptype)
      debug0("no toptype");
    return;
  }
  i = typetoi(glob_toptype);
  if (i == T_MINUTES)
    dprintf(idx, "%s", stats_duration(glob_locstats->values[glob_timerange][i] * 60, 2));
  else if (i >= 0)
    dprintf(idx, "%d", glob_locstats->values[glob_timerange][i]);
  else if (i == T_WPL) {
    if (glob_locstats->values[glob_timerange][T_LINES])
      dprintf(idx, "%.2f",
              ((float) glob_locstats->values[glob_timerange][T_WORDS])
                / ((float) glob_locstats->values[glob_timerange][T_LINES]));
  } else if (i == T_IDLE) {
    if (glob_locstats->values[glob_timerange][T_LINES])
      dprintf(idx, "%.2f",
              ((float) glob_locstats->values[glob_timerange][T_MINUTES])
                / ((float) glob_locstats->values[glob_timerange][T_LINES]));
  } else
    debug1("invalid type: %s", glob_toptype);
}

static void template_send_channel_load(int idx, struct template_content *h_tpc)
{
  float umax, uonep, aonep, f;
  int i, amax;

  Context;
  if (!glob_globstats)
    return;
  umax = 0.0;
  amax = 0;
  for (i = 0; i < 24; i++) {
    if (glob_globstats->users[S_USERCOUNTS][i] > 0)
      if ((((float) glob_globstats->users[S_USERSUM][i]) / ((float) glob_globstats->users[S_USERCOUNTS][i])) > umax)
        umax = ((float) glob_globstats->users[S_USERSUM][i]) / ((float) glob_globstats->users[S_USERCOUNTS][i]);
    if (glob_globstats->activity[i] > amax)
      amax = glob_globstats->activity[i];
  }
  uonep = umax / 100.0;
  aonep = amax / 100.0;
  for (i = 0; i < 24; i++) {
    glob_cl_timerange = i;
    glob_activity = glob_globstats->activity[i];
    if (glob_globstats->users[S_USERCOUNTS][i] > 0) {
      f = ((float) glob_globstats->users[S_USERSUM][i]) / ((float) glob_globstats->users[S_USERCOUNTS][i]);
      glob_au_users = f;
      glob_au_percent = (int) (f / uonep);
    } else {
      glob_au_users = -1.0;
      glob_au_percent = -1;
    }
    if (glob_activity >= 0)
      glob_activity_percent = (int) (glob_globstats->activity[i] / aonep);
    else
      glob_activity_percent = 0;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_au_users(int idx, struct template_content *h_tpc)
{
  if (glob_au_percent >= 0)
    dprintf(idx, "%.1f", glob_au_users);
}

static void template_send_if_cl_logged(int idx, struct template_content *h_tpc)
{
  if (glob_au_percent != -1)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_au_value(int idx, struct template_content *h_tpc)
{
  if (!h_tpc->intpar1)
    return;
  dprintf(idx, "%d", (int) (((float) (glob_au_percent / 100.0)) * ((float) h_tpc->intpar1)));
}

static void template_add_cmd_au_value(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  int max = 100;

  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "max"))
      max = atoi(params->s2);
  }
  if (!max)
    max = 100;
  h_tpc->intpar1 = max;
}

static void template_send_activity_value(int idx, struct template_content *h_tpc)
{
  if (!h_tpc->intpar1)
    return;
  dprintf(idx, "%d", (int) (((float) (glob_activity_percent / 100.0)) * ((float) h_tpc->intpar1)));
}

static void template_send_activity(int idx, struct template_content *h_tpc)
{
  dprintf(idx, "%d", glob_activity);
}

static void template_send_if_urls(int idx, struct template_content *h_tpc)
{
  if (log_urls)
    if (glob_globstats)
      if (glob_globstats->urls)
        templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_random_urls(int idx, struct template_content *h_tpc)
{
  int urls, nr, i;
  unsigned long x;

  if (!glob_globstats || !log_urls)
    return;
  urls = 0;
  for (glob_url = glob_globstats->urls; glob_url; glob_url = glob_url->next) {
    urls++;
    glob_url->shown = 0;
  }
  if (!urls)
    return;
  Assert(glob_globstats->urls);
  if (urls > log_urls)
    nr = log_urls;
  else
    nr = urls;
  while (nr > 0) {
    x = random() % nr;
    i = 0;
    for (glob_url = glob_globstats->urls; glob_url; glob_url = glob_url->next) {
      if (glob_url->shown)
        continue;
      if (i == x) {
        glob_url->shown = 1;
        templates_content_send(h_tpc->subcontent, idx);
        urls--;
      }
      i++;
    }
    nr--;
  }
}

static void template_send_url(int idx, struct template_content *h_tpc)
{
  if (glob_url)
    dprintf(idx, "%s", glob_url->url);
}

static void template_send_if_hosts(int idx, struct template_content *h_tpc)
{
  if (glob_globstats)
    if (glob_globstats->hosts)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_hosts(int idx, struct template_content *h_tpc)
{
  globstats *tlds, *isps;
  hoststr *h;
  char *host;
  char *s;
  int i;

  if (!glob_globstats)
    return;
  if (!glob_globstats->hosts)
    return;
  tlds = nmalloc(sizeof(globstats));
  tlds->hosts = NULL;
  isps = nmalloc(sizeof(globstats));
  isps->hosts = NULL;
  for (h = glob_globstats->hosts; h; h = h->next) {
	Assert(h->host);
    // skip IPv6 hosts
    if (strchr(h->host, ':'))
      continue;
    host = strrchr(h->host, '.') + 1;
    // skip other unusable hosts
    if (!((host - 1) && host)) {
      debug1("[stats.mod] Skipping host-stats for '%s'.", h->host);
      continue;
    }
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
  i = 0;
  glob_isp = isps->hosts;
  glob_tld = tlds->hosts;
  while ((i <= 5) && (glob_isp || glob_tld)) {
    i++;
    templates_content_send(h_tpc->subcontent, idx);
    if (glob_isp)
      glob_isp = glob_isp->next;
    if (glob_tld)
      glob_tld = glob_tld->next;
  }
  free_hosts(isps->hosts);
  free_hosts(tlds->hosts);
  nfree(isps);
  nfree(tlds);
}

static void template_send_isp(int idx, struct template_content *h_tpc)
{
  if (glob_isp)
    dprintf(idx, "%s", glob_isp->host);
}

static void template_send_ispnr(int idx, struct template_content *h_tpc)
{
  if (glob_isp)
    dprintf(idx, "%d", glob_isp->nr);
}

static void template_send_tld(int idx, struct template_content *h_tpc)
{
  if (glob_tld)
    dprintf(idx, "%s", glob_tld->host);
}

static void template_send_tldnr(int idx, struct template_content *h_tpc)
{
  if (glob_tld)
    dprintf(idx, "%d", glob_tld->nr);
}

static void template_send_if_kicks(int idx, struct template_content *h_tpc)
{
  if (glob_globstats)
    if (glob_globstats->kicks)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_random_kicks(int idx, struct template_content *h_tpc)
{
  int nr, i, kicks;
  struct stats_kick *kick;
  unsigned long x;

  if (!glob_globstats)
    return;
  if (!glob_globstats->kicks)
    return;
  nr = 0;
  for (kick = glob_globstats->kicks; kick; kick = kick->next) {
    nr++;
    kick->shown = 0;
  }
  if (nr > display_kicks)
    kicks = display_kicks;
  else
    kicks = nr;
  while (kicks > 0) {
    x = random() % nr;
    i = 0;
    for (glob_kick = glob_globstats->kicks; glob_kick; glob_kick = glob_kick->next) {
      if (glob_kick->shown)
        continue;
      if (i == x) {
        glob_kick->shown = 1;
        templates_content_send(h_tpc->subcontent, idx);
        break;
      }
      i++;
    }
    nr--;
    kicks--;
  }
}

/* <?kick_contexts ..?>
 * sends the last few loglines before a kick occured.
 * the log of the actual kick is _not_ sent with this command
 */
static void template_send_kick_contexts(int idx, struct template_content *h_tpc)
{
  if (!glob_kick)
    return;
  for (glob_kick_context = glob_kick->log; glob_kick_context && glob_kick_context->next; glob_kick_context = glob_kick_context->next)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_kick_context(int idx, struct template_content *h_tpc)
{
  if (glob_kick_context)
    dprintf(idx, "%s", text2html(glob_kick_context->quote));
}

static void template_send_kick(int idx, struct template_content *h_tpc)
{
  struct stats_quote *k;

  if (!glob_kick)
    return;
  for (k = glob_kick->log; k && k->next; k = k->next);
  dprintf(idx, "%s", text2html(k->quote));
}

static void template_send_if_topics(int idx, struct template_content *h_tpc)
{
  if (glob_globstats)
    if (glob_globstats->topics)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_topiclist(int idx, struct template_content *h_tpc)
{
  if (glob_globstats)
    for (glob_topic = glob_globstats->topics; glob_topic; glob_topic = glob_topic->next)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_topic(int idx, struct template_content *h_tpc)
{
  if (glob_topic)
    dprintf(idx, "%s", text2html(glob_topic->topic));
}

static void template_send_topic_set_by(int idx, struct template_content *h_tpc)
{
  if (glob_topic)
    dprintf(idx, "%s", glob_topic->by);
}

static void template_send_topic_set_when(int idx, struct template_content *h_tpc)
{
  char ts[21];

  if (glob_topic) {
    strftime(ts, 20, "%H:%M", localtime(&glob_topic->when));
    dprintf(idx, "%s", ts);
  }
}

static void template_send_if_chan_topwords(int idx, struct template_content *h_tpc)
{
  locstats *ls;

  if (glob_globstats) {
    for (ls = glob_globstats->local; ls; ls = ls->next) {
      if (ls->words) {
        templates_content_send(h_tpc->subcontent, idx);
        return;
      }
    }
  }
}

static void template_send_chan_topwords(int idx, struct template_content *h_tpc)
{
  if (!glob_globstats)
    return;
  do_globwordstats(glob_globstats);
  glob_wordplace = 0;
  for (glob_wordstats = glob_globstats->words;
       glob_wordstats && (glob_wordplace < 10);
       glob_wordstats = glob_wordstats->next) {
    glob_wordplace++;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_if_user_topwords(int idx, struct template_content *h_tpc)
{
  if (glob_locstats)
    if (glob_locstats->words)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_user_topwords(int idx, struct template_content *h_tpc)
{
  if (!glob_locstats)
    return;
  sortwordstats(glob_locstats, NULL);
  glob_wordplace = 0;
  for (glob_wordstats = glob_locstats->words;
       glob_wordstats && (glob_wordplace < 10);
       glob_wordstats = glob_wordstats->next) {
    glob_wordplace++;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_wordplace(int idx, struct template_content *h_tpc)
{
  dprintf(idx, "%d", glob_wordplace);
}

static void template_send_word(int idx, struct template_content *h_tpc)
{
  if (glob_wordstats)
    dprintf(idx, "%s", text2html(glob_wordstats->word));
}

static void template_send_wordnr(int idx, struct template_content *h_tpc)
{
  if (glob_wordstats)
    dprintf(idx, "%d", glob_wordstats->nr);
}

static void template_send_miscfacts(int idx, struct template_content *h_tpc)
{
/*  struct slang_lang *l;
  int itype, pitype;
  locstats *ls;

  if (!glob_globstats)
    return;
  // at first, find the current language
  for (l = slangs; l; l = l->next) {
    if ((!l->lang && !slgloblang) || (l->lang && slgloblang && !strcasecmp(l->lang, slgloblang))) {
      // now cycle through all fact-types
      for (glob_fact = l->bignumbers; glob_fact; glob_fact = glob_fact->next) {
        itype = typetoi(glob_fact->type);
        if (itype < 0)
          pitype = (itype * -1) + (TOTAL_TYPES - 1);
        else
          pitype = itype;
        sortstats(glob_globstats, itype, S_DAILY);
        ls = glob_globstats->slocal[S_DAILY][pitype];
        if (!ls)
          continue;
        else if ((itype >= 0) && !ls->values[S_DAILY][itype])
          continue;
        else if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS] || !ls->values[S_DAILY][T_LINES]))
          continue;
        else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES] || !ls->values[S_DAILY][T_LINES]))
          continue;
        else if ((itype == T_VOCABLES) && !ls->vocables)
          continue;
        templates_content_send(h_tpc->subcontent, idx);
      }
    }
  }*/
  int ret;

  for (ret = selectfirstfact(); ret; ret = selectnextfact())
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_fact(int idx, struct template_content *h_tpc)
{
/*  unsigned long x;
  int itype, pitype, iplace;
  locstats *ls;
  struct slang_texts *txt;
  struct slang_bnplaces *place;

  if (!glob_fact || !glob_globstats)
    return;
  itype = typetoi(glob_fact->type);
  sortstats(glob_globstats, itype, S_DAILY);
  // now find the desired place
  for (place = glob_fact->places; place; place = place->next)
    if (place->place == h_tpc->intpar1)
      break;
  if (!place)
    return;
  if (itype < 0)
    pitype = (itype * -1) + (TOTAL_TYPES - 1);
  else
    pitype = itype;
  ls = glob_globstats->slocal[S_DAILY][pitype];
  // now find the user who who is on the specified place
  iplace = 1;
  while (ls && (iplace < place->place)) {
    iplace++;
    ls = ls->snext[S_DAILY][pitype];
  }
  // just don't output anything if there's no useful data
  if (!ls)
    return;
  else if ((itype >= 0) && !ls->values[S_DAILY][itype])
    return;
  else if ((itype == T_WPL) && (!ls->values[S_DAILY][T_WORDS] || !ls->values[S_DAILY][T_LINES]))
    return;
  else if ((itype == T_IDLE) && (!ls->values[S_DAILY][T_MINUTES] || !ls->values[S_DAILY][T_LINES]))
    return;
  else if ((itype == T_VOCABLES) && !ls->vocables)
    return;
  slgloblocstats = ls;
  slglobtype = glob_fact->type;
  x = random() % place->entries;
  txt = place->texts;
  while (txt) {
    if (!x)
      dprintf(idx, "%s\n", dynamicslang(txt->text));
    x--;
    txt = txt->next;
  }
*/
  int place;
  char *fact;

  place = h_tpc->intpar1;
  if (!place)
    return;
  fact = getfact(place);
  if (fact)
    dprintf(idx, "%s", fact);
}


/* template_add_cmd_fact():
 * Parameters: <place>
 */
static void template_add_cmd_fact(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  int place;

  Context;
  place = 0;
  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "place"))
      place = atoi(params->s2);
  }
  if (!place) {
    putlog(LOG_MISC, "*", "Error parsing template: Invalid parameter for \"<?fact <place>?>\"!");
    place = 1;
  }
  h_tpc->command = template_send_fact;
  h_tpc->intpar1 = place;
}

static void template_send_userlist(int idx, struct template_content *h_tpc)
{
  if (!glob_globstats)
    return;
  sort_stats_alphabetically(glob_globstats);
  for (glob_locstats = glob_globstats->local; glob_locstats; glob_locstats = glob_locstats->next)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_timeranges(int idx, struct template_content *h_tpc)
{
  for (glob_timerange = 0; glob_timerange <= 3; glob_timerange++)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_timerange(int idx, struct template_content *h_tpc)
{
  dprintf(idx, "%s", RANGESTR_LONG(glob_timerange));
}

static void template_send_tplace(int idx, struct template_content *h_tpc)
{
  locstats *ls;
  int place = 0;

  if (!glob_globstats || !glob_locstats)
    return;
  sortstats(glob_globstats, T_WORDS, glob_timerange);
  for (ls = glob_globstats->slocal[glob_timerange][T_WORDS]; ls; ls = ls->snext[glob_timerange][T_WORDS]) {
    if (!listsuser(ls, glob_globstats->chan))
      continue;
    place++;
    if (ls == glob_locstats)
      break;
  }
  if (ls)
    dprintf(idx, "%d", place);
  else
    dprintf(idx, "-");
}

static void template_send_if_quote(int idx, struct template_content *h_tpc)
{
  if (glob_locstats)
    if (glob_locstats->quotes)
      templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_random_quote(int idx, struct template_content *h_tpc)
{
  int nr, i;
  quotestr *qs;
  unsigned long x;

  if (!glob_locstats)
    return;
  if (!glob_locstats->quotes)
    return;
  nr = 0;
  for (qs = glob_locstats->quotes; qs; qs = qs->next)
    nr++;
  x = random() % nr;
  i = 0;
  qs = glob_locstats->quotes;
  while (qs) {
    if (i == x) {
      dprintf(idx, "%s", text2html(qs->quote));
      return;
    }
    qs = qs->next;
    i++;
  }
}

static void template_send_onchanlist(int idx, struct template_content *h_tpc)
{
  struct stats_member *m;
  struct stats_chan *chan;

  if (!show_usersonchan || !glob_globstats)
    return;
  chan = schan_find(glob_globstats->chan);
  if (!chan)
    return;
  for (m = schan_members_getfirst(&chan->members); m ; m = schan_members_getnext(&chan->members)) {
    glob_statsmember = m;
    glob_user = m->user;
    if (m->stats)
      glob_locstats = m->stats;
    else
      glob_locstats = NULL;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_usermode(int idx, struct template_content *h_tpc)
{
  if (glob_statsmember && glob_statsmember->eggmember) {
#ifndef NO_EGG
    if (chan_hasop(glob_statsmember->eggmember))
      dprintf(idx, "@");
    if (chan_hasvoice(glob_statsmember->eggmember))
      dprintf(idx, "+");
#endif
  }
}

static void template_send_if_user(int idx, struct template_content *h_tpc)
{
  if (glob_locstats)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_nouser(int idx, struct template_content *h_tpc)
{
  if (!glob_locstats)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_idletime(int idx, struct template_content *h_tpc)
{
  if (!glob_statsmember)
    return;
#ifndef NO_EGG
  if (glob_statsmember->eggmember && chan_issplit(glob_statsmember->eggmember))
    return;
#endif
  dprintf(idx, "%s", stats_duration(now - glob_statsmember->last, 2));
}

static void template_send_if_netsplitted(int idx, struct template_content *h_tpc)
{
#ifndef NO_EGG
  if (!glob_statsmember || !glob_statsmember->eggmember)
    return;
  if (chan_issplit(glob_statsmember->eggmember))
    templates_content_send(h_tpc->subcontent, idx);
#endif
}

static void template_send_nick(int idx, struct template_content *h_tpc)
{
  if (glob_statsmember)
    dprintf(idx, "%s", glob_statsmember->nick);
}

static void template_send_graphstats(int idx, struct template_content *h_tpc)
{
  char buf[512], *pbuf;
  locstats ls;

  Context;
  if (!glob_globstats || (glob_timerange == T_ERROR))
    return;
  locstats_init(&ls);
  strncpy(buf, graphstats, sizeof(buf));
  pbuf = buf;
  while (pbuf[0]) {
    glob_toptype = newsplit(&pbuf);
    glob_sorting = slangtypetoi(glob_toptype);
    if (glob_sorting < 0)
      continue;
    sortstats(glob_globstats, glob_sorting, glob_timerange);
    glob_graph_total = gettotal(glob_globstats, glob_sorting, glob_timerange);
    ls.values[glob_timerange][glob_sorting] = glob_graph_total;
    glob_locstats = &ls;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

static void template_send_graphs(int idx, struct template_content *h_tpc)
{
  float percent, wpercent, onep;
  locstats *ls;
  int max;

  if (!glob_globstats || (glob_sorting == T_ERROR) ||
      (glob_timerange == T_ERROR) || (glob_sorting < 0))
    return;
  max = 0;
  for (ls = glob_globstats->slocal[glob_timerange][glob_sorting];
       ls;
       ls = ls->snext[glob_timerange][glob_sorting]) {
    if (listsuser(ls, glob_globstats->chan)) {
      max = ls->values[glob_timerange][glob_sorting];
      break;
    }
  }
  if (!glob_graph_total || !max)
    return;
  onep = (float) glob_graph_total / 100.0;
  glob_place = 0;
  for (ls = glob_globstats->slocal[glob_timerange][glob_sorting];
       ls;
       ls = ls->snext[glob_timerange][glob_sorting]) {
    if (!listsuser(ls, glob_globstats->chan))
      continue;
    if (!ls->values[glob_timerange][glob_sorting])
      break;
    glob_place++;
    if (glob_place > graphnr)
      break;
    percent = (float) ls->values[glob_timerange][glob_sorting] / onep;
    wpercent = (float) ls->values[glob_timerange][glob_sorting] / ((float) max / 100.0);
    glob_graph_percent = percent;
    glob_graph_width_percent = wpercent;
    glob_locstats = ls;
    templates_content_send(h_tpc->subcontent, idx);
  }
}

/* static void template_send_graph_filled_percent(int idx, struct template_content *h_tpc)
 *{
 *  dprintf(idx, "%d%%", (int) (glob_graph_width_percent * 0.8));
 *}
 */

static void template_send_graph_percent(int idx, struct template_content *h_tpc)
{
  if (!h_tpc->intpar1)
    dprintf(idx, "%.2f", glob_graph_percent);
  else
    dprintf(idx, "%d", (int) (((float) (glob_graph_width_percent / 100.0)) * ((float) h_tpc->intpar1)));
}

static void template_add_cmd_graph_percent(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  int max = 100, stretch = 0;

  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "max"))
      max = atoi(params->s2);
    if (!strcasecmp(params->s1, "stretch") && !strcasecmp(params->s2, "true"))
      stretch = 1;
  }
  if (!max)
    max = 100;
  if (stretch)
    h_tpc->intpar1 = max;
  else
    h_tpc->intpar1 = 0;
}

/*static void template_send_graph_label_percent(int idx, struct template_content *h_tpc)
 *{
 *  dprintf(idx, "%d%%", 100 - ((int) (glob_graph_width_percent * 0.8)));
 *}
 *
 *static void template_send_graph_exact_percent(int idx, struct template_content *h_tpc)
 *{
 *  dprintf(idx, "%.02f%%", glob_graph_percent);
 *}
 */

static void template_send_password(int idx, struct template_content *h_tpc)
{
  if (glob_user && glob_user->password)
    dprintf(idx, "%s", glob_user->password);
}

static void template_send_email(int idx, struct template_content *h_tpc)
{
  if (glob_user && glob_user->email)
    dprintf(idx, "%s", glob_user->email);
}

static void template_send_homepage(int idx, struct template_content *h_tpc)
{
  if (glob_user && glob_user->homepage)
    dprintf(idx, "%s", glob_user->homepage);
}

static void template_send_icqnr(int idx, struct template_content *h_tpc)
{
  if (glob_user && glob_user->icqnr)
    dprintf(idx, "%d", glob_user->icqnr);
}

static void template_send_if_icqnr(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_icqnr)");
    return;
  }
  if (glob_user->icqnr)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_email(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_email)");
    return;
  }
  if (glob_user->email)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_homepage(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_homepage)");
    return;
  }
  if (glob_user->homepage)
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_listuser(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_listuser)");
    return;
  }
  if (suser_list(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_addhosts(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_addhosts)");
    return;
  }
  if (suser_addhosts(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_not_listuser(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_listuser)");
    return;
  }
  if (!suser_list(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_not_addhosts(int idx, struct template_content *h_tpc)
{
  if (!glob_user) {
    debug0("WARNING: No globuser (if_addhosts)");
    return;
  }
  if (!suser_addhosts(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_nostats(int idx, struct template_content *h_tpc)
{
  if (glob_user && suser_nostats(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_not_nostats(int idx, struct template_content *h_tpc)
{
  if (glob_user && !suser_nostats(glob_user))
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_login_error(int idx, struct template_content *h_tpc)
{
  if (h_tpc->intpar1)
    dprintf(idx, "%s", getslang(h_tpc->intpar1 + glob_loginerror));
}

static void template_add_cmd_login_error(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  char *s_baseid;

  s_baseid = llist_2string_get_s2(params, "baseid");
  if (s_baseid)
    h_tpc->intpar1 = atoi(s_baseid);
}

static void template_send_binary_url(int idx, struct template_content *h_tpc)
{
  dprintf(idx, "%s", binary_url);
}

static void template_send_if_binary(int idx, struct template_content *h_tpc)
{
  if (binary_url[0])
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_if_nobinary(int idx, struct template_content *h_tpc)
{
  if (!binary_url[0])
    templates_content_send(h_tpc->subcontent, idx);
}

static void template_send_memberage(int idx, struct template_content *h_tpc)
{
  int max, age, maxage;
  float onep, percent;

  if (glob_locstats && !h_tpc->intpar1)
    dprintf(idx, "%s", stats_duration(now - glob_locstats->started, 3));
  else if (glob_locstats && glob_globstats) {
    max = h_tpc->intpar1;
    maxage = (int) h_tpc->floatpar1 * 60 * 60 * 24;
    if ((now - glob_globstats->started) < maxage)
      maxage = (now - glob_globstats->started);
    age = now - glob_locstats->started;

    onep = (float) maxage / (float) max;
    percent = (float) age / onep;

    dprintf(idx, "%d", (int) percent);
  }
}

static void template_add_cmd_memberage(struct template_content *h_tpc,
                                            struct llist_2string *params,
                                            char *included_text)
{
  int relative = 0, max = 100, maxage = 0;

  for (; params; params = params->next) {
    if (!strcasecmp(params->s1, "max"))
      max = atoi(params->s2);
    if (!strcasecmp(params->s1, "relative") &&
        (!strcasecmp(params->s2, "true") || !strcasecmp(params->s2, "yes")))
      relative = 1;
    if (!strcasecmp(params->s1, "maxage"))
      maxage = atoi(params->s2);
  }
  if (!max)
    max = 100;
  if (relative)
    h_tpc->intpar1 = max;
  else
    h_tpc->intpar1 = 0;
  h_tpc->floatpar1 = (float) maxage;
}

static void template_send_types(int idx, struct template_content *h_tpc)
{
  char *buffer, *buf, *oldtype;

  oldtype = glob_toptype; /* backup glob_toptype and pray that we don't
				the old value during the cycle */
  buffer = nmalloc(strlen(TYPES) + 1);
  strcpy(buffer, TYPES);
  buf = buffer;
  for (glob_toptype = newsplit(&buf); glob_toptype[0];
  	glob_toptype = newsplit(&buf))
    templates_content_send(h_tpc->subcontent, idx);
  nfree(buffer);
  buffer = nmalloc(strlen(SPECIAL_TYPES) + 1);
  strcpy(buffer, SPECIAL_TYPES);
  buf = buffer;
  for (glob_toptype = newsplit(&buf); glob_toptype[0];
  	glob_toptype = newsplit(&buf))
    templates_content_send(h_tpc->subcontent, idx);
  nfree(buffer);
  glob_toptype = oldtype;
}

static void template_send_sorting(int idx, struct template_content *h_tpc)
{
  if (glob_sorting != T_ERROR)
    dprintf(idx, "%s", itotype(glob_sorting));
}

static void template_send_userage(int idx, struct template_content *h_tpc)
{
  if (glob_user)
    dprintf(idx, "%s", stats_duration((now - glob_user->created), 3));
}

static void template_send_timetolive(int idx, struct template_content *h_tpc)
{
  if (glob_user)
    dprintf(idx, "%s", stats_duration(TIMETOLIVE(glob_user), 2));
}

struct template_commands stats_template_commands[] =
{
  {"init_colorfade", template_init_colorfade, template_add_cmd_init_colorfade},
  {"fcolor", template_send_fcolor, NULL},
  {"fade_color", template_fade_color, NULL},
  {"chanlist", template_send_chanlist, template_add_subcontent},
  {"chan", template_send_chan, template_add_cmd_chan_user},
//  {"encoded_chan", template_send_encoded_chan, NULL},
  {"current_topic", template_send_current_topic, NULL},
  {"topnr", template_send_topnr, NULL},
  {"if_total", template_send_if_total, template_add_subcontent},
  {"if_daily", template_send_if_daily, template_add_subcontent},
  {"if_weekly", template_send_if_weekly, template_add_subcontent},
  {"if_monthly", template_send_if_monthly, template_add_subcontent},
  {"topstats", template_send_topstats, template_add_subcontent},
  {"toplist", template_send_toplist, template_add_subcontent},
  {"type", template_send_type, template_add_cmd_type},
  {"types", template_send_types, template_add_subcontent},
  {"place", template_send_place, NULL},
  {"user", template_send_user, template_add_cmd_chan_user},
  {"value", template_send_value, NULL},
  {"if_urls", template_send_if_urls, template_add_subcontent},
  {"random_urls", template_send_random_urls, template_add_subcontent},
  {"url", template_send_url, NULL},
  {"if_hosts", template_send_if_hosts, template_add_subcontent},
  {"hosts", template_send_hosts, template_add_subcontent},
  {"isp", template_send_isp, NULL},
  {"ispnr", template_send_ispnr, NULL},
  {"tld", template_send_tld, NULL},
  {"tldnr", template_send_tldnr, NULL},
  {"if_kicks", template_send_if_kicks, template_add_subcontent},
  {"random_kicks", template_send_random_kicks, template_add_subcontent},
  {"kick_contexts", template_send_kick_contexts, template_add_subcontent},
  {"kick_context", template_send_kick_context, NULL},
  {"kick", template_send_kick, NULL},
  {"if_topics", template_send_if_topics, template_add_subcontent},
  {"topiclist", template_send_topiclist, template_add_subcontent},
  {"topic", template_send_topic, NULL},
  {"topic_set_by", template_send_topic_set_by, NULL},
  {"topic_set_when", template_send_topic_set_when, NULL},
  {"if_chan_topwords", template_send_if_chan_topwords, template_add_subcontent},
  {"chan_topwords", template_send_chan_topwords, template_add_subcontent},
  {"if_user_topwords", template_send_if_user_topwords, template_add_subcontent},
  {"user_topwords", template_send_user_topwords, template_add_subcontent},
  {"wordplace", template_send_wordplace, NULL},
  {"word", template_send_word, NULL},
  {"wordnr", template_send_wordnr, NULL},
  {"userlist", template_send_userlist, template_add_subcontent},
  {"timeranges", template_send_timeranges, template_add_subcontent},
  {"timerange", template_send_timerange, NULL},
  {"tplace", template_send_tplace, NULL},
  {"if_quote", template_send_if_quote, template_add_subcontent},
  {"random_quote", template_send_random_quote, NULL},
  {"onchanlist", template_send_onchanlist, template_add_subcontent},
  {"usermode", template_send_usermode, NULL},
  {"if_user", template_send_if_user, template_add_subcontent},
  {"if_nouser", template_send_if_nouser, template_add_subcontent},
  {"idletime", template_send_idletime, NULL},
  {"if_netsplitted", template_send_if_netsplitted, template_add_subcontent},
  {"nick", template_send_nick, NULL},
  {"graphstats", template_send_graphstats, template_add_subcontent},
  {"graphs", template_send_graphs, template_add_subcontent},
  {"graph_percent", template_send_graph_percent, template_add_cmd_graph_percent},
  {"password", template_send_password, NULL},
  {"email", template_send_email, NULL},
  {"homepage", template_send_homepage, NULL},
  {"icqnr", template_send_icqnr, NULL},
  {"if_icqnr", template_send_if_icqnr, template_add_subcontent},
  {"if_homepage", template_send_if_homepage, template_add_subcontent},
  {"if_email", template_send_if_email, template_add_subcontent},
  {"if_listuser", template_send_if_listuser, template_add_subcontent},
  {"if_addhosts", template_send_if_addhosts, template_add_subcontent},
  {"if_not_listuser", template_send_if_not_listuser, template_add_subcontent},
  {"if_not_addhosts", template_send_if_not_addhosts, template_add_subcontent},
  {"if_nostats", template_send_if_nostats, template_add_subcontent},
  {"if_not_nostats", template_send_if_not_nostats, template_add_subcontent},
  {"login_error", template_send_login_error, template_add_cmd_login_error},
  {"if_binary", template_send_if_binary, template_add_subcontent},
  {"if_nobinary", template_send_if_nobinary, template_add_subcontent},
  {"binary_url", template_send_binary_url, NULL},
  {"miscfacts", template_send_miscfacts, template_add_subcontent},
  {"fact", template_send_fact, template_add_cmd_fact},
  {"activity_value", template_send_activity_value, template_add_cmd_au_value},
  {"channel_load", template_send_channel_load, template_add_subcontent},
  {"au_users", template_send_au_users, NULL},
  {"activity", template_send_activity, NULL},
  {"au_value", template_send_au_value, template_add_cmd_au_value},
  {"if_cl_logged", template_send_if_cl_logged, template_add_subcontent},
  {"memberage", template_send_memberage, template_add_cmd_memberage},
  {"sorting", template_send_sorting, NULL},
  {"userage", template_send_userage, NULL},
  {"timetolive", template_send_timetolive, NULL},
  {0, 0, 0},
};
