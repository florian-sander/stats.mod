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

/* Don't know a better place for these defines... */
#define SLE_USERNOTFOUND 1
#define SLE_NOUSERPASS 2
#define SLE_WRONGPASS 3

/* send_webseen():
 * take the URL string, split the parameters off,
 * calculate seen-results if necessary, and finally
 * send a template to the client
 */
static void process_get_request(int idx)
{
  char *url, urlbuf[512], *newurl, *s_timerange, *s_sorting;
  char *chan, *cmd, *user, *pass, *lchan, *lang, *str_skin;
  char *email, *homepage, *icqnr, *newpass, *addhosts, *list;
  char *newpass_confirmation, *nostats, *s_start, *s_end;
  struct stats_userlist *u;
  struct llist_1string *langlist;

  Context;
  // init all global vars
  reset_global_vars();
  if (!http_connection(idx)->path) {
    debug1("%s: no request. Dropping connection.", dcc[idx].host);
    return;
  }
  // copy the url into a buffer, so we can work on it without messing it up
  strncpy(urlbuf, http_connection(idx)->path, 512);
  urlbuf[511] = 0;
  url = urlbuf;
  // make sure there is a '/' at the end of the URL, or most links will
  // be broken.
  if (url[strlen(url) - 1] != '/') {
    newurl = nmalloc(strlen(url) + 1 + 1);
    strcpy(newurl, url);
    strcat(newurl, "/");
    dprintf(idx, "HTTP/1.1 301 Moved Permanently\nServer: EggdropMiniHTTPd/%s\n", HTTPD_VERSION);
    dprintf(idx, "Location: %s\nConnection: close\nContent-Type: text/html\n\n", newurl);
    dprintf(idx, "<HTML><body>The concluding \"/\" is important!<br><center>");
    dprintf(idx, "<a href=\"%s\">%s</a></center><br>", newurl, newurl);
    http_connection(idx)->code = 301;
    nfree(newurl);
    return;
  }

  // try to get skin and lang settings from the parameter list
  // If the parameter is specified, write it into a cookie. If it
  // is not specified, try to get it from a cookie first, and use the default
  // if it isn't even defined in a cookie
  if ((str_skin = get_param_value(idx, "skin")))
    set_cookie(idx, "skin", str_skin);
  else if (!(str_skin = get_cookie_value(idx, "skin")))
    str_skin = default_skin;
  if (!(glob_skin = templates_skin_find(skins, str_skin))) {
    if (!(glob_skin = templates_skin_find(skins, default_skin))) {
      send_http_header(idx, 500);
      dprintf(idx, "<HTML><BODY><H1>Internal Server Error: No skin found!</H1></BODY></HTML>");
      return;
    }
  }

  if ((lang = get_param_value(idx, "lang")))
    set_cookie(idx, "lang", lang);
  else if (!(lang = get_cookie_value(idx, "lang"))) {
    langlist = http_connection(idx)->langs;
    while (langlist) {
      if (slang_valid(glob_skin->slang, langlist->s1)) {
	lang = langlist->s1;
	break;
      }
      langlist = langlist->next;
    }
    if (!lang)
      lang = default_slang;
  }
  if (!(glob_slang = slang_find(glob_skin->slang, lang))) {
    if (!(glob_slang = slang_find(glob_skin->slang, default_slang))) {
      send_http_header(idx, 500);
      dprintf(idx, "<HTML><BODY><H1>Internal Server Error: No language found!</H1></BODY></HTML>");
      return;
    }
  }

  // now it's time to choose what to do
  if (!strcmp(url, "/")) {
    // user accessed the server root? ok, send the root template...
    send_http_header(idx, 200);
    template_send(glob_skin, "root", idx);
    return;
  } else if (!strcasecmp(url, "/cgi-bin/usersettings/")) {
    user = get_param_value(idx, "username");
    if (!user) {
      send_http_header(idx, 200);
      template_send(glob_skin, "userlogin", idx);
      return;
    }
    u = findsuser_by_name(user);
    if (!u) {
      glob_loginerror = SLE_USERNOTFOUND;
      send_http_header(idx, 200);
      template_send(glob_skin, "login_error", idx);
      return;
    }
    glob_user = u;
    if (get_param_value(idx, "sendpass")) {
      user_email_password(u);
      send_http_header(idx, 200);
      template_send(glob_skin, "password_emailed", idx);
      return;
	}
    pass = get_param_value(idx, "password");
    if (!pass) {
      send_http_header(idx, 200);
      template_send(glob_skin, "userlogin", idx);
      return;
    }
    if (!u->password) {
      glob_loginerror = SLE_NOUSERPASS;
      send_http_header(idx, 200);
      template_send(glob_skin, "login_error", idx);
      return;
    }
    if (!(!strcmp(u->password, pass))) {
      glob_loginerror = SLE_WRONGPASS;
      send_http_header(idx, 200);
      template_send(glob_skin, "login_error", idx);
      return;
    }
    icqnr = get_param_value(idx, "icqnr");
    if (icqnr)
      u->icqnr = atoi(icqnr);
    email = get_param_value(idx, "email");
    if (email) {
      setemail(u, email);
    }
    homepage = get_param_value(idx, "homepage");
    if (homepage) {
      sethomepage(u, homepage);
    }
    newpass = get_param_value(idx, "newpassword");
    newpass_confirmation = get_param_value(idx, "newpass_confirmation");
    if (newpass && newpass[0] && (newpass[0] != ' ')
        && newpass_confirmation && !strcmp(newpass_confirmation, newpass)) {
      u->password = nrealloc(u->password, strlen(newpass) + 1);
      strcpy(u->password, newpass);
    }
    list = get_param_value(idx, "list");
    if (list) {
      if (atoi(list))
        suser_setflag(u, S_LIST);
      else
        suser_delflag(u, S_LIST);
    }
    addhosts = get_param_value(idx, "addhosts");
    if (addhosts) {
      if (atoi(addhosts))
        suser_setflag(u, S_ADDHOSTS);
      else
        suser_delflag(u, S_ADDHOSTS);
    }
    nostats = get_param_value(idx, "nostats");
    if (nostats) {
      if (atoi(nostats)) {
	suser_setflag(u, S_NOSTATS);
	suser_delflag(u, S_LIST);
      } else
        suser_delflag(u, S_NOSTATS);
    }
    send_http_header(idx, 200);
    template_send(glob_skin, "usersettings", idx);
    return;
  } else {
    // strip the leading '/'
    url++;
    // and split the channel from the URL
    chan = decode_url(csplit(&url, '/'));
    glob_globstats = findglobstats(chan);
    if (!glob_globstats) {
      lchan = nmalloc(strlen(chan) + 1 + 1);
      lchan[0] = '#';
      strcpy(lchan + 1, chan);
      glob_globstats = findglobstats(lchan);
      nfree(lchan);
      if (!glob_globstats) {
        send_http_header(idx, 404);
        template_send(glob_skin, "404", idx);
        return;
      }
    }
    cmd = csplit(&url, '/');
    if (!strcasecmp(cmd, "")) {
      send_http_header(idx, 200);
      template_send(glob_skin, "chan", idx);
      return;
    } else if (!strcasecmp(cmd, "misc")) {
      send_http_header(idx, 200);
      template_send(glob_skin, "misc", idx);
      return;
    } else if (!strcasecmp(cmd, "top")) {
      s_timerange = csplit(&url, '/');
      s_sorting = csplit(&url, '/');
      if (!s_sorting[0] && !strcasecmp(s_timerange, "custom")) {
	// custom top talker list
	s_timerange = get_param_value(idx, "timerange");
	s_sorting = get_param_value(idx, "sorting");
	s_start = get_param_value(idx, "start");
	s_end = get_param_value(idx, "end");
	if (s_timerange)
		glob_timerange = get_timerange(s_timerange);
	else
		glob_timerange = S_TOTAL;
	if (s_sorting)
		glob_sorting = typetoi(s_sorting);
	else
		glob_sorting = T_WORDS;
	if (s_start)
		glob_top_start = atoi(s_start);
	if (s_end)
		glob_top_end = atoi(s_end);
	if (!glob_top_start)
		glob_top_start = 1;
	if (glob_top_end <= glob_top_start)
		glob_top_end = glob_top_start + webnr;
	if (glob_sorting == T_ERROR) {
	  	debug1("Invalid sorting '%s'. Defaulting to 'words'.", s_sorting);
		glob_sorting = T_WORDS;
	      }
	if (glob_timerange == T_ERROR)
		glob_sorting = S_TOTAL;
	glob_toptype = itotype(glob_sorting);
	sortstats(glob_globstats, glob_sorting, glob_timerange);
	debug2("sorting: %s (%d)", s_sorting, glob_sorting);
	send_http_header(idx, 200);
	template_send(glob_skin, "custom_top", idx);
	return;
      }
      if (!s_sorting[0] || !s_timerange[0]) {
        // redirect client to full URL if it skipped anything
        chan = encode_url(glob_globstats->chan);
        newurl = nmalloc(strlen(chan) + 18 + 1);
        sprintf(newurl, "/%s/top/total/words/", chan);
        dprintf(idx, "HTTP/1.1 301 Moved Permanently\nServer: EggdropMiniHTTPd/%s\n", HTTPD_VERSION);
        dprintf(idx, "Location: %s\nConnection: close\nContent-Type: text/html\n\n", newurl);
        dprintf(idx, "<HTML><body>The concluding \"/\" is important!<br><center>");
        dprintf(idx, "<a href=\"%s\">%s</a></center><br>", newurl, newurl);
        http_connection(idx)->code = 301;
        nfree(newurl);
        return;
      }
      if (!strcasecmp(s_timerange, "total"))
        glob_timerange = S_TOTAL;
      else if (!strcasecmp(s_timerange, "today"))
        glob_timerange = S_TODAY;
      else if (!strcasecmp(s_timerange, "weekly"))
        glob_timerange = S_WEEKLY;
      else if (!strcasecmp(s_timerange, "monthly"))
        glob_timerange = S_MONTHLY;
      else if (!strcasecmp(s_timerange, "daily"))
        glob_timerange = S_DAILY;
      else {
        send_http_header(idx, 404);
        template_send(glob_skin, "404", idx);
        return;
      }
      Assert(glob_globstats);
      if (!strcasecmp(s_sorting, "graphs")) {
        send_http_header(idx, 200);
        template_send(glob_skin, "graphs", idx);
        return;
      }
      glob_sorting = slangtypetoi(s_sorting);
      if ((glob_timerange == T_ERROR) || (glob_sorting == T_ERROR)) {
        debug2("invalid top-parameter \"%s\" or \"%s\"", s_sorting, s_timerange);
        send_http_header(idx, 404);
        template_send(glob_skin, "404", idx);
        return;
      }
      glob_top_start = 1;
      glob_top_end = webnr;
      sortstats(glob_globstats, glob_sorting, glob_timerange);
      send_http_header(idx, 200);
      template_send(glob_skin, "top", idx);
      return;
    } else if (!strcasecmp(cmd, "users")) {
      user = decode_url(csplit(&url, '/'));
      if (!user[0]) {
        send_http_header(idx, 200);
        template_send(glob_skin, "userlist", idx);
        return;
      }
      glob_locstats = findlocstats(glob_globstats->chan, user);
      if (!glob_locstats) {
        send_http_header(idx, 404);
        template_send(glob_skin, "404", idx);
        return;
      }
      if (!glob_locstats->u)
        glob_locstats->u = findsuser_by_name(glob_locstats->user);
      glob_user = glob_locstats->u;
      if (glob_user && suser_nostats(glob_user)) {
	// don't let anyone access "private" stats
	send_http_header(idx, 404);
	template_send(glob_skin, "404", idx);
	return;
      }
      send_http_header(idx, 200);
      template_send(glob_skin, "user", idx);
      return;
    } else if (!strcasecmp(cmd, "onchan")) {
      send_http_header(idx, 200);
      template_send(glob_skin, "onchan", idx);
      return;
    }
  }
  send_http_header(idx, 404);
  template_send(glob_skin, "404", idx);
}
