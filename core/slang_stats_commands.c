
static void slang_send_nick()
{
  if (glob_nick)
    strncat(slang_text_buf, glob_nick, sizeof(slang_text_buf));
}

static void slang_send_bot()
{
  strncat(slang_text_buf, botnetnick, sizeof(slang_text_buf));
}

static void slang_send_topnr()
{
  char buf[10];

  snprintf(buf, sizeof(buf), "%d", webnr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_graphnr()
{
  char buf[10];

  snprintf(buf, sizeof(buf), "%d", graphnr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_peak()
{
  char buf[10];

  if (glob_globstats && (glob_timerange != T_ERROR)) {
    snprintf(buf, sizeof(buf), "%d", glob_globstats->peak[glob_timerange]);
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static void slang_send_totalusers()
{
  char buf[10];

  if (glob_globstats) {
    snprintf(buf, sizeof(buf), "%d", countstatmembers(glob_globstats));
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static void slang_send_chanstarted()
{
  time_t tt, ttbuf;
  char sbuf[61];

  if (glob_globstats) {
    ttbuf = now;
    tt = glob_globstats->started;
    strftime(sbuf, 60, "%d.%m. %Y  %H:%M", localtime(&tt));
    ctime(&ttbuf); /* workaround for eggdrop bug */
    strncat(slang_text_buf, sbuf, sizeof(slang_text_buf));
  }
}

static void slang_send_chan()
{
  if (glob_globstats)
    strncat(slang_text_buf, glob_globstats->chan, sizeof(slang_text_buf));
}

static void slang_send_user()
{
  if (glob_locstats)
    strncat(slang_text_buf, glob_locstats->user, sizeof(slang_text_buf));
  else if (glob_user)
    strncat(slang_text_buf, glob_user->user, sizeof(slang_text_buf));
}

static void slang_send_sorting()
{
  if (glob_sorting != T_ERROR)
    strncat(slang_text_buf, itotype(glob_sorting), sizeof(slang_text_buf));
}

static void slang_send_range()
{
  char buf[10];

  snprintf(buf, sizeof(buf), "%d", glob_range);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_active_users()
{
  char buf[10];

  if (glob_globstats && (glob_timerange != T_ERROR)) {
    snprintf(buf, sizeof(buf), "%d",
             countactivestatmembers(glob_globstats, 1, glob_timerange,
                 (glob_sorting >= 0) ? glob_sorting : T_LINES, 1)
             );
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static void slang_send_word()
{
  if (glob_word)
    strncat(slang_text_buf, glob_word, sizeof(slang_text_buf));
}

static void slang_send_place()
{
  char buf[10];

  snprintf(buf, sizeof(buf), "%d", glob_place);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_password()
{
  if (glob_user && glob_user->password)
    strncat(slang_text_buf, glob_user->password, sizeof(slang_text_buf));
}

static void slang_send_botnick()
{
  strncat(slang_text_buf, botname, sizeof(slang_text_buf));
}

static void slang_send_server_host()
{
  char s[121];
  char *p;
  
#ifndef NO_EGG
  s[0] = 0;
  gethostname(s, 120);
  if (!s[0]) {
    p = strchr(botuserhost, '@');
    if (p) {
      strncpy(s, p, sizeof(s));
      s[120] = 0;
    }
  }
  strncat(slang_text_buf, s, sizeof(slang_text_buf));
#endif
}

static void slang_send_server_port()
{
  int i;
  char buf[10];

#ifndef NO_EGG
  for (i = 0; i < dcc_total; i++) {
    if (dcc[i].type == &MHTTPD_CON_HTTPD) {
      snprintf(buf, sizeof(buf), "%d", dcc[i].port);
      strncat(slang_text_buf, buf, sizeof(slang_text_buf));
      return;
    }
  }
#endif
}

static void slang_send_topic_by()
{
  if (glob_topic) {
    Assert(glob_topic->by);
    strncat(slang_text_buf, glob_topic->by, sizeof(slang_text_buf));
  }
}

static void slang_send_topic_when()
{
  char buf[20];
  
  if (glob_topic) {
    strftime(buf, 19, "%H:%M", localtime(&glob_topic->when));
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static void slang_send_url_by()
{
  if (glob_url) {
    Assert(glob_url->by);
    strncat(slang_text_buf, glob_url->by, sizeof(slang_text_buf));
  }
}

static void slang_send_url_when()
{
  char buf[20];
  
  if (glob_url) {
    strftime(buf, 19, "%H:%M", localtime(&glob_url->when));
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static void slang_send_random_urls()
{
  char buf[3];
  static struct stats_url *url;
  int nr = 0;

  if (!glob_globstats)
    return;
  for (url = glob_globstats->urls; url; url = url->next)
    nr++;
  if (nr > log_urls)
    nr = log_urls;
  snprintf(buf, sizeof(buf), "%d", nr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_random_kicks()
{
  char buf[3];
  struct stats_kick *kick;
  int nr = 0;

  if (!glob_globstats)
    return;
  for (kick = glob_globstats->kicks; kick; kick = kick->next)
    nr++;
  if (nr > display_kicks)
    nr = display_kicks;
  snprintf(buf, sizeof(buf), "%d", nr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_chanwords()
{
  static wordstats *ws;
  char buf[6];
  int nr = 0;

  if (!glob_globstats)
    return;
  do_globwordstats(glob_globstats);
  for (ws = glob_globstats->words; ws; ws = ws->next)
    nr++;
  snprintf(buf, sizeof(buf), "%d", nr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_userwords()
{
  wordstats *ws;
  char buf[6];
  int nr = 0;

  if (!glob_locstats)
    return;
  for (ws = glob_locstats->words; ws; ws = ws->next)
    nr++;
  snprintf(buf, sizeof(buf), "%d", nr);
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_value()
{
  int i, nr;
  char buf[30];
  wordstats *ws;
  
  if (!glob_locstats || (glob_timerange == T_ERROR) || !glob_toptype)
    return;
  i = typetoi(glob_toptype);
  if (i == T_MINUTES)
    snprintf(buf, sizeof(buf), "%s", stats_duration(glob_locstats->values[glob_timerange][i] * 60, 2));
  else if (i >= 0)
    snprintf(buf, sizeof(buf), "%li", glob_locstats->values[glob_timerange][i]);
  else if (i == T_WPL) {
    if (glob_locstats->values[glob_timerange][T_LINES])
      snprintf(buf, sizeof(buf), "%.2f",
              ((float) glob_locstats->values[glob_timerange][T_WORDS])
                / ((float) glob_locstats->values[glob_timerange][T_LINES]));
  } else if (i == T_IDLE) {
    if (glob_locstats->values[glob_timerange][T_LINES])
      snprintf(buf, sizeof(buf), "%.2f",
              ((float) glob_locstats->values[glob_timerange][T_MINUTES])
                / ((float) glob_locstats->values[glob_timerange][T_LINES]));
  } else if (i == T_VOCABLES) {
    nr = 0;
    for (ws = glob_locstats->words; ws; ws = ws->next)
      nr++;
    snprintf(buf, sizeof(buf), "%d", nr);
  } else {
    debug1("invalid type: %s", glob_toptype);
    snprintf(buf, sizeof(buf), "ERROR: '%s'", glob_toptype);
  }
  strncat(slang_text_buf, buf, sizeof(slang_text_buf));
}

static void slang_send_lastspoke()
{
  char buf[20];

  if (glob_locstats) {
    debug1("%d", glob_locstats->lastspoke);
    snprintf(buf, sizeof(buf), "%s", stats_duration(now - glob_locstats->lastspoke, 2));
    strncat(slang_text_buf, buf, sizeof(slang_text_buf));
  }
}

static struct slang_text_commands slang_text_stats_command_table[] =
{
  {"nick", slang_send_nick},
  {"bot", slang_send_bot},
  {"topnr", slang_send_topnr},
  {"graphnr", slang_send_graphnr},
  {"peak", slang_send_peak},
  {"totalusers", slang_send_totalusers},
  {"chanstarted", slang_send_chanstarted},
  {"chan", slang_send_chan},
  {"user", slang_send_user},
  {"nick", slang_send_nick},
  {"sorting", slang_send_sorting},
  {"range", slang_send_range},
  {"active_users", slang_send_active_users},
  {"word", slang_send_word},
  {"place", slang_send_place},
  {"password", slang_send_password},
  {"botnick", slang_send_botnick},
  {"server_host", slang_send_server_host},
  {"server_port", slang_send_server_port},
  {"topic_by", slang_send_topic_by},
  {"topic_when", slang_send_topic_when},
  {"url_by", slang_send_url_by},
  {"url_when", slang_send_url_when},
  {"random_urls", slang_send_random_urls},
  {"random_kicks", slang_send_random_kicks},
  {"chanwords", slang_send_chanwords},
  {"userwords", slang_send_userwords},
  {"value", slang_send_value},
  {"lastspoke", slang_send_lastspoke},
  {0, 0}
};
