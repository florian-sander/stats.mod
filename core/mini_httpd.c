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

/* mini_httpd.c
 *
 * minimalistic http server for use in eggdrop modules
 *
 * Usage:
 *
 * add init_httpd() to module_start()
 * add unload_httpd() to module_close()
 * add expmem_httpd() to module_expmem()
 *
 * call start_httpd(port) to start listening for incoming connections
 * on the specified port.
 *
 * create a function "static void process_get_request(idx);". This function
 * gets called when someone connects to your server and sends an GET request.
 * you can access the requested path with http_connection(idx)->path.
 *
 * Don't forget to send the http header with send_http_header(int idx, code)
 * before you start sending the output.
 *
 * cookies are stored in http_connection(idx)->cookies, parameters in ->params.
 * You can also access them via get_cookie_value(idx, cookiename) or
 * get_param_value(idx, paramname).
 *
 * Variables (which you might want to tcl-trace and add to your config file)
 *
 * char httpd_ip[21] = "";
 *      Defines on which vhost httpd will listen for connections.
 *      If this is set to "", it'll listen on all vhosts.
 *
 * static char httpd_log[121] = "";
 *      Logfile to which http access will be loged (CLF format)
 *
 * static char httpd_loglevel[20] = "1";
 *      Defines to which loglevel access will be logged.
 *
 * static char httpd_ignore_msg[256] = "<H1>You are on ignore.</H1>";
 *      reply which the recipient will see, if he/she is on ignore
 *
 * static int max_http_thr = 0;
 * static int max_http_time = 0;
 *      simple flood protection. Allows only thr connections in time seconds.
 *
 */

static char httpd_ip[21] = "";
static char httpd_loglevel[21] = "1";
static char httpd_ignore_msg[256] = "<H1>You are on ignore.</H1>";
static char httpd_log[121] = "";
static int max_http_thr = 0;
static int max_http_time = 0;
static int http_timeout = 5;
static int httpd_dcc_index = -1;

static char *httpd_text_buf = NULL;

static struct dcc_table MHTTPD_CON_HTTPD =
{
  "HTTPD",
  DCT_VALIDIDX,
  eof_http,
  httpd_accept,
  0,
  timeout_listen_httpd,
  display_httpd_accept,
  0,
  NULL,
  0
};

static struct dcc_table MHTTPD_CON_HTTP =
{
  "HTTP",
  DCT_VALIDIDX,
  eof_http,
  http_activity,
  &http_timeout,
  timeout_http,
  display_http,
  expmem_http,
  kill_http,
#ifdef OLDBOT
  out_http,
#else
  out_http,
  outdone_http
#endif
};

#define http_connection(i) ((struct http_connection_data *) dcc[(i)].u.other)

/* init_httpd()
 * initializes a few variables
 */
static void init_httpd()
{
  httpd_text_buf = NULL;
}

/* expmem_httpd()
 * expmem function
 */
/*
static int expmem_httpd()
{
  int size = 0;

  if (httpd_text_buf)
    size += strlen(httpd_text_buf) + 1;
  return size;
}
*/

/* unload_httpd():
 * frees all allocated memory, stops listening and kills all
 * existing connections.
 */
static void unload_httpd()
{
  stop_httpd();
  if (httpd_text_buf)
    nfree(httpd_text_buf);
}

/* start_httpd():
 * starts listening for http connections on the defined port.
 */
static void start_httpd(int port)
{
  int i, zz;
  char tmp[50];

  Context;
  // a little hack to make httpd listen on the defined vhost
  // (or on all vhosts, if none is defined)
  // Just set my-ip to the wanted vhost, since open_listen()
  // uses this var
  sprintf(tmp, "set my-ip \"%s\";", httpd_ip);
  do_tcl("httpd-hack-start",
      "set my-ip-httpd-backup ${my-ip};"
      "set my-hostname-httpd-backup ${my-hostname};"
      "set my-hostname \"\"");
  do_tcl("httpd-hack-setip", tmp);
  // now get a listening socket
  zz = open_listen(&port);
  // don't forget to restore my-ip when we're done ^_^
  do_tcl("httpd-hack-end",
      "set my-ip ${my-ip-httpd-backup};"
      "set my-hostname ${my-hostname-httpd-backup}");
  // ohoh, we didn't get a socket :(
  if (zz == (-1)) {
    putlog(LOG_MISC, "*", "ERROR! Cannot open listening socket for httpd!");
    return;
  }
  // now add this new socket to our dcc table and display a warning,
  // if the table is full
  if ((i = new_dcc(&MHTTPD_CON_HTTPD, 0)) == -1) {
    putlog(LOG_MISC, "*", "ERROR! Cannot open listening socket for httpd! DCC table is full!");
    // better kill the socket, before we get a "phantom-socket" ^_^
    killsock(zz);
    return;
  }
  // store the index in a global var, so we can access it easily later...
  httpd_dcc_index = i;
  // now fill the dcc-struct with information
  dcc[i].sock = zz;
  dcc[i].addr = (IP) (-559026163);
  dcc[i].port = port;
  strcpy(dcc[i].nick, "httpd");
  strcpy(dcc[i].host, "*");
  dcc[i].timeval = now;
  putlog(LOG_MISC, "*", "Now listening for http connections on port %d", port);
}

/* stop_httpd()
 * kills all httpd connections and listening sockets
 */
static void stop_httpd()
{
  int i;

  for (i = 0; i < dcc_total; i++) {
    if (dcc[i].type == &MHTTPD_CON_HTTPD) {
      putlog(LOG_MISC, "*",
      	     "no longer listening for http connections on port %d",
             dcc[i].port);
      killsock(dcc[i].sock);
      lostdcc(i);
    } else if (dcc[i].type == &MHTTPD_CON_HTTP) {
      putlog(LOG_MISC, "*", "killing http connection from %s", dcc[i].host);
      killsock(dcc[i].sock);
      lostdcc(i);
    }
  }
}

/* init_http_connection_data():
 * inits all variables in our http_connection_data struct
 */
static void init_http_connection_data(int idx)
{
  http_connection(idx)->traffic = 0;
  http_connection(idx)->code = -1;
  http_connection(idx)->browser = NULL;
  http_connection(idx)->referer = NULL;
  http_connection(idx)->path = NULL;
  http_connection(idx)->cmd = NULL;
  http_connection(idx)->postparams = NULL;
  http_connection(idx)->cookies = NULL;
  http_connection(idx)->params = NULL;
  http_connection(idx)->headers = NULL;
  http_connection(idx)->langs = NULL;
  http_connection(idx)->getpostparams = 0;
  http_connection(idx)->content_length = 0;
}

/* expmem_http()
 * expmem's all data allocated to store browser info, referer, cookies, etc...
 */
static int expmem_http(void *x)
{
  register struct http_connection_data *p = (struct http_connection_data *) x;
  int tot = 0;

  Context;
  if (!p) {
    putlog(LOG_MISC, "*", "Can't expmem clientinfo, no pointer. This should not happen!");
    return 0;
  }
  tot += sizeof(struct http_connection_data);
  if (p->browser)
    tot += strlen(p->browser) + 1;
  if (p->referer)
    tot += strlen(p->referer) + 1;
  if (p->path)
    tot += strlen(p->path) + 1;
  if (p->cmd)
    tot += strlen(p->cmd) + 1;
  if (p->postparams)
    tot += strlen(p->postparams) + 1;
  if (p->cookies)
    tot += llist_2string_expmem(p->cookies);
  if (p->params)
    tot += llist_2string_expmem(p->params);
  if (p->headers)
    tot += llist_1string_expmem(p->headers);
  if (p->langs)
    tot += llist_1string_expmem(p->langs);
  return tot;
}

/* free_http_connection_data():
 * frees all data of our http_connection_data struct
 */
static void free_http_connection_data(int idx)
{
  if (http_connection(idx)->browser)
    nfree(http_connection(idx)->browser);
  if (http_connection(idx)->referer)
    nfree(http_connection(idx)->referer);
  if (http_connection(idx)->path)
    nfree(http_connection(idx)->path);
  if (http_connection(idx)->cmd)
    nfree(http_connection(idx)->cmd);
  if (http_connection(idx)->postparams)
    nfree(http_connection(idx)->postparams);
  if (http_connection(idx)->cookies)
    llist_2string_free(http_connection(idx)->cookies);
  if (http_connection(idx)->params)
    llist_2string_free(http_connection(idx)->params);
  if (http_connection(idx)->headers)
    llist_1string_free(http_connection(idx)->headers);
  if (http_connection(idx)->langs)
    llist_1string_free(http_connection(idx)->langs);
  n_free(http_connection(idx), __FILE__, __LINE__);
}

/* http_activity():
 * handles all the data that the browser sends to us.
 */
static void http_activity(int idx, char *buf, int len)
{
  char *cmd, *path, *imask, *params;
#ifdef flush_inbuf
  int i;
#endif
  int lev, content_length;
  struct timeval t;
  double pre_time;

  debug2("%s: %s", dcc[idx].host, buf);

  // at first, check if the user is on ignore and therefore should
  // be ignored
  imask = nmalloc(strlen(dcc[idx].host) + 11);
  sprintf(imask, "http!*@%s", dcc[idx].host);
  if (match_ignore(imask)) {
    debug1("Ignoring http access from %s", dcc[idx].host);
    send_http_header(idx, 401);
    if (httpd_ignore_msg[0])
      dprintf(idx, "%s", httpd_ignore_msg);
    killsock(dcc[idx].sock);
    lostdcc(idx);
    nfree(imask);
    return;
  }
  nfree(imask);

  if ((http_connection(idx)->content_length > 0) && (http_connection(idx)->getpostparams)) {
    append_postparam_string(idx, buf);
    return;
  }

  // then check for recognized commands which we want to be logged
  // (only GET is supported, at the moment)
  if ((!strncasecmp(buf, "GET ", 4) || !strncasecmp(buf, "POST ", 5))
      && !http_connection(idx)->cmd) {
    http_connection(idx)->cmd = nmalloc(strlen(buf) + 1);
    strcpy(http_connection(idx)->cmd, buf);
  }
  // now check if we know the command and store all info that we need
  cmd = newsplit(&buf);
  // GET: request for a document
  if (!strcasecmp(cmd, "GET") || !strcasecmp(cmd, "POST")) {
    // at first, check if we're being flooded and kill the connection
    // if there were too many requests.
    if (http_flood()) {
      http_connection(idx)->code = 401;
      killsock(dcc[idx].sock);
      lostdcc(idx);
      return;
    }
//    if (!strcasecmp(cmd, "POST"))
//      http_connection(idx)->getpostparams = 1;
    // now log the access
    Assert(http_connection(idx)->cmd);
    lev = logmodes(httpd_loglevel);
    if (lev)
      putlog(lev, "*", "%s: %s", dcc[idx].host, http_connection(idx)->cmd);
    // and finally store the request, if there wasn't already one before.
    if (!http_connection(idx)->path) {
      params = newsplit(&buf);
      path = csplit(&params, '?');
      // cut the parameters off and store them
      add_params(idx, params);
      http_connection(idx)->path = nmalloc(strlen(path) + 1);
      strcpy(http_connection(idx)->path, path);
    }
  // user-agent: browser-information
  } else if (!strcasecmp(cmd, "User-Agent:")) {
    if (http_connection(idx)->browser)
      return;
    http_connection(idx)->browser = nmalloc(strlen(buf) + 1);
    strcpy(http_connection(idx)->browser, buf);
  } else if (!strcasecmp(cmd, "Referer:")) {
    if (http_connection(idx)->referer)
      return;
    http_connection(idx)->referer = nmalloc(strlen(buf) + 1);
    strcpy(http_connection(idx)->referer, buf);
  } else if (!strcasecmp(cmd, "Cookie:")) {
    add_cookies(idx, buf);
  } else if (!strcasecmp(cmd, "Content-Length:") && !http_connection(idx)->content_length) {
    content_length = atoi(buf);
    debug1("setting content length to %d", content_length);
    http_connection(idx)->content_length = content_length;
  } else if (!strcasecmp(cmd, "Accept-language:")) {
    add_language(idx, buf);
  } else if (!buf[0]) {
    if (http_connection(idx)->cmd && !(!strncasecmp(http_connection(idx)->cmd, "POST ", 5))) {
      debug0("now sending...");
      gettimeofday(&t, NULL);
      pre_time = (float) t.tv_sec + (((float) t.tv_usec) / 1000000);
      process_get_request(idx);
      gettimeofday(&t, NULL);
      debug1("Processing time: %.3f", ((float) t.tv_sec + (((float) t.tv_usec) / 1000000)) - pre_time);
      dcc[idx].status = 1;
#ifndef OLDBOT
      /* If there's no data in our socket, we immediately get rid of it.
       */
      if (!sock_has_data(SOCK_DATA_OUTGOING, dcc[idx].sock)) {
        killsock(dcc[idx].sock);
        lostdcc(idx);
      }
#endif
    } else {
      debug0("waiting for post params...");
      http_connection(idx)->getpostparams = 1;
#ifdef sockoptions
      i = sockoptions(dcc[idx].sock, EGG_OPTION_UNSET, SOCK_BUFFER);
      if (i)
        debug1("WARNING: sockoptions returned %d while trying to deativate "
               "buffering for POST parameters!", i);
#endif
#ifdef flush_inbuf
      flush_inbuf(idx);
#endif
    }
  }
}

/* add_cookies()
 * simple function to add one or more cookies to the cookie-list
 */
static void add_cookies(int idx, char *buf)
{
  char *cookie, *name, *value;

  while (buf[0]) {
    cookie = csplit(&buf, ';');
    while (cookie[0] == ' ')
      cookie++;
    name = csplit(&cookie, '=');
    value = cookie;
    http_connection(idx)->cookies
               = llist_2string_add(http_connection(idx)->cookies, name, value);
  }
}

static char *get_cookie_value(int idx, char *name)
{
  Assert(idx >= 0);
  return llist_2string_get_s2(http_connection(idx)->cookies, name);
}

/* add_params():
 * extracts all parameters from the URL and stores them
 * in a simple linked list
 */
static void add_params(int idx, char *buf)
{
  char *param, *name, *value;

  if (strchr(buf, '?')) {
    debug1("WARNING: '?' found in paramstring '%s'. This should have been "
           "already split!", buf);
    return;
  }

  while (buf[0]) {
    param = csplit(&buf, '&');
    name = csplit(&param, '=');
    value = decode_url(param);
    debug2("adding parameter: '%s'='%s'", name, value);
    http_connection(idx)->params
               = llist_2string_add(http_connection(idx)->params, name, value);
  }
}

static char *get_param_value(int idx, char *name)
{
  Assert(idx >= 0);
  return llist_2string_get_s2(http_connection(idx)->params, name);
}

static void add_language(int idx, char *buf)
{
  char *lang;

  if (buf)
    buf = csplit(&buf, ';'); /* strip "; q=1.5", whatever it means... */
  while (buf[0]) {
    lang = csplit(&buf, ',');
    lang = csplit(&lang, '-'); /* en-us => en */
    while (lang[0] == ' ')
      lang++;
    debug1("adding accepted language: '%s'", lang);
    http_connection(idx)->langs =
    	llist_1string_add(http_connection(idx)->langs, lang);
  }
}

#ifndef OLDBOT
static void outdone_http(int idx)
{
  if (dcc[idx].status) {
    killsock(dcc[idx].sock);
    lostdcc(idx);
  } else
    dcc[idx].status = 1;
}
#endif

static void display_http(int idx, char *buf)
{
  sprintf(buf, "http connection");
}

static void display_httpd_accept(int idx, char *buf)
{
  sprintf(buf, "httpd");
}

static void timeout_http(int idx)
{
#ifdef flush_inbuf
  if (http_connection(idx)->getpostparams && http_connection(idx)->path) {
    // If there's still something in the inbuffer, then we might still be receivng
    // POST parameters or something. Just let the connection live a bit longer.
    // (FIXME: DOSable by flooding with body)
    if (flush_inbuf(idx) > 0) {
      debug0("inbuf not empty on timeout. Flushed...");
      dcc[idx].timeval = now;
      return;
    }
  }
#endif
  send_http_header(idx, 408);
  dprintf(idx, "<html>\n<head><title>408 Request Time-out</title></head>\n"
               "<body>\n<H1>Request Time-out</H1><br>\n<p>Your browser didn't "
               "send enough information to process the request within %d "
               "seconds.</p>\n", http_timeout);
#ifndef flush_inbuf
  dprintf(idx, "<p>If your browser did send the information, then the problem "
  		"is probably that this server doesn't have the netstuff patch "
  		"installed. Please ask the admin to install it. This "
  		"patch is needed to receive login-information with browsers "
  		"as Netscape Navigator, Opera or some others. (That's not a "
  		"bug in the browser, but a missing network-related function "
  		"in eggdrop which gets added by the patch)</p>\n");
#endif
  dprintf(idx, "</body>\n</html>\n");
  killsock(dcc[idx].sock);
  lostdcc(idx);
}

/* kill_http():
 * This function called when connection is killed. It
 * logs the connection to the logfile, if one exists.
 */
static void kill_http(int idx, void *x)
{
  char ts[41], test[11];
  time_t tt;
  FILE *f;

  Context;
  tt = now;
  ctime(&tt);
  /* 05/Feb/2000:12:08:17 +0100 */
  strftime(test, 19, "%z", localtime(&tt));
  // if test contains 'z' then strftime() doesn't support
  // %z (timezone-offset) on this system, and we have to
  // use a config var instead
  if (test[0] != 'z')
    strftime(ts, 40, "%d/%b/%Y:%H:%M:%S %z", localtime(&tt));
  else
    strftime(ts, 40, "%d/%b/%Y:%H:%M:%S", localtime(&tt));
  if (httpd_log[0]) {
    f = fopen(httpd_log, "a");
    if (f == NULL)
      putlog(LOG_MISC, "*", "ERROR writing httpd log.");
    else {
      if (test[0] != 'z')
        fprintf(f,
         "%s - - [%s] \"%s\" %d %d \"%s\" \"%s\"\n", dcc[idx].host, ts,
         http_connection(idx)->cmd ? http_connection(idx)->cmd : "",
         http_connection(idx)->code, http_connection(idx)->traffic,
         http_connection(idx)->referer ? http_connection(idx)->referer : "-",
         http_connection(idx)->browser ? http_connection(idx)->browser : "");
      else
        fprintf(f,
         "%s - - [%s %+05d] \"%s\" %d %d \"%s\" \"%s\"\n",
         dcc[idx].host, ts, offset * (-1) * 100,
         http_connection(idx)->cmd ? http_connection(idx)->cmd : "",
         http_connection(idx)->code, http_connection(idx)->traffic,
         http_connection(idx)->referer ? http_connection(idx)->referer : "-",
         http_connection(idx)->browser ? http_connection(idx)->browser : "");
      fclose(f);
    }
  }
  // don't forget to free the data when we're done.
  free_http_connection_data(idx);
}

/* out_http():
 * Called when data is sent through the socket. Used to log the
 * sent traffic.
 */
static void out_http(int idx, char *buf, void *x)
{
  register struct http_connection_data *p = (struct http_connection_data *) x;

  if (!p) {
    putlog(LOG_MISC, "*", "No http_connection pointer. This should not happen!");
    return;
  }
  p->traffic += strlen(buf);
  tputs(dcc[idx].sock, buf, strlen(buf));
}

/* http_accept():
 * accepts an incoming http connection
 */
static void httpd_accept(int idx, char *buf, int len)
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
    putlog(LOG_MISC, "*", "HTTPd: Error accepting connection: %s", s);
    return;
  }
  if ((i = new_dcc(&MHTTPD_CON_HTTP, sizeof(struct http_connection_data))) == (-1)) {
    putlog(LOG_MISC, "*", "Error accepting http connection. DCC table is full.");
    killsock(sock);
    return;
  }
  dcc[i].sock = sock;
  dcc[i].addr = ip;
  dcc[i].port = port;
  strcpy(dcc[i].nick, "http");
#ifndef OLDBOT
  sprintf(s, "%s", iptostr(my_htonl(ip)));
#else
  sprintf(s, "%lu.%lu.%lu.%lu", (ip >> 24) & 0xff, (ip >> 16) & 0xff,
  	  (ip >> 8) & 0xff, ip & 0xff); /* dw */
#endif
  strcpy(dcc[i].host, s);
#endif
  dcc[i].timeval = now;
  dcc[i].status = 0;
  // init http_connection_data struct
  init_http_connection_data(i);
}


static void eof_http(int idx)
{
  debug0("eof http");
  killsock(dcc[idx].sock);
  lostdcc(idx);
}


static void set_cookie(int idx, char *name, char *value)
{
  char tbuf[40], *buf;
  time_t tt;
  int len;

  tt = now + 30 * 60 * 60 * 24;
  strftime(tbuf, sizeof(tbuf), "%a, %d-%b-%Y %H:%M:%S GMT", localtime(&tt));
  len = 34 + strlen(name) + strlen(value) + strlen(tbuf) + 1;
  buf = nmalloc(len);
  snprintf(buf, len, "Set-Cookie: %s=%s; expires=%s; path=/\n", name, value, tbuf);
  http_connection(idx)->headers = llist_1string_add(http_connection(idx)->headers, buf);
  nfree(buf);
}

/* append_postparam_string()
 * appends this chunk to the buffer that contains the POST parameters.
 * when the buffer is filled, processing gets started automatically.
 */
static void append_postparam_string(int idx, char *buf)
{
  if (!http_connection(idx)->getpostparams) {
    debug2("?!? Tried to append post param string '%s' to connection #%d, "
           "but this connection doesn't expect any params... probably a bug. :(",
           buf, idx);
    return;
  }
  if (!http_connection(idx)->postparams) {

    if (!(http_connection(idx)->content_length > 0)) {
      send_http_header(idx, 400);
      dprintf(idx, "<html><head><title>400 Bad Request</title></head>"
                   "<body><H1>Bad Request:</H1> invalid "
                   "content-length '%d'.</body></html>\n",
                   http_connection(idx)->content_length);
      killsock(dcc[idx].sock);
      lostdcc(idx);
      return;
    }

    http_connection(idx)->postparams
       = nmalloc(http_connection(idx)->content_length + 1);
    http_connection(idx)->postparams[0] = 0;
    debug1("allocated %d bytes for params", http_connection(idx)->content_length + 1);
  }

  debug1("appending content: '%s'", buf);
  debug1("old: '%s'", http_connection(idx)->postparams);

  strncat(http_connection(idx)->postparams,
          buf,
          http_connection(idx)->content_length);
  http_connection(idx)->postparams[http_connection(idx)->content_length] = 0;
  debug1("new: '%s'", http_connection(idx)->postparams);

  if ((http_connection(idx)->content_length > 0) &&
       http_connection(idx)->getpostparams &&
       http_connection(idx)->postparams)
  {
    if (strlen(http_connection(idx)->postparams) >= http_connection(idx)->content_length) {
      debug0("parsing params...");
      add_params(idx, http_connection(idx)->postparams);
      process_request(idx);
    }
  }
}

/* send_http_header()
 * sends the http header
 */
static void send_http_header(int idx, int code)
{
  struct llist_1string *h;

  if (code == 200)
    dprintf(idx, "HTTP/1.0 200 OK\n");
  else if (code == 401)
    dprintf(idx, "HTTP/1.0 401 Access Forbidden\n");
  else if (code == 404)
    dprintf(idx, "HTTP/1.1 404 Not Found\n");
  else if (code == 500)
    dprintf(idx, "HTTP/1.1 500 Internal Server Error\n");
  else
    dprintf(idx, "HTTP/1.0 %d %d\n", code, code);
  dprintf(idx, "Server: EggdropMiniHTTPd/%s\n", HTTPD_VERSION);
  dprintf(idx, "Content-Type: text/html\n");
  for (h = http_connection(idx)->headers; h; h = h->next) {
    debug1("Sending additional header: '%s'", h->s1);
    dprintf(idx, "%s", h->s1);
  }
  dprintf(idx, "\n");
  http_connection(idx)->code = code;
}

/* process_request():
 * calls the main processing function process_get_request(), takes the
 * processing time and tries to kill the socket if everything got already
 * sent.
 */
static void process_request(int idx)
{
  struct timeval t;
  double pre_time;

  Context;
  Assert(idx >= 0);
  debug0("now sending...");
  gettimeofday(&t, NULL);
  pre_time = (float) t.tv_sec + (((float) t.tv_usec) / 1000000);
  process_get_request(idx);
  gettimeofday(&t, NULL);
  debug1("Processing time: %.3f", ((float) t.tv_sec + (((float) t.tv_usec) / 1000000)) - pre_time);
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














/* http_flood()
 * simple check for floods
 */
static int mhttp_time = 0, mhttp_thr = 0;
static int http_flood()
{
  if (!max_http_thr || !max_http_time)
    return 0;
  if ((now - mhttp_time) > max_http_time) {
    mhttp_time = now;
    mhttp_thr = 0;
  }
  mhttp_thr++;
  if (mhttp_thr > max_http_thr)
    return 1;
  return 0;
}

/* csplit()
 * basically the same as nsplit, but allows you to define
 * the divider.
 */
static char *csplit(char **rest, char divider)
{
  register char *o, *r;

  if (!rest)
    return *rest = "";
  o = *rest;
  while (*o == divider)
    o++;
  r = o;
  while (*o && (*o != divider))
    o++;
  if (*o)
    *o++ = 0;
  *rest = o;
  return r;
}

/* text2html():
 * replaces all strange chars by html-unicode-codes and removes
 * stupid color codes */
static char *text2html(char *text)
{
  char *buf, ubuf[8];
  unsigned char c;

  if (httpd_text_buf)
  	httpd_text_buf = nrealloc(httpd_text_buf, (strlen(text) * sizeof(char) * 7) + 1);
  else
    httpd_text_buf = nmalloc((strlen(text) * sizeof(char) * 7) + 1);
  buf = httpd_text_buf;

  *buf = 0;
  while (text[0]) {
    c = text[0];
    if (((c >= 97) && (c <= 122)) || ((c >= 65) && (c <= 90))) {
      *buf = c;
      buf++;
    } else if (c == 3) {	/* filter $§%#&-mirc colors! */
      /* inspired by src/dcc.c */
      if (isdigit(text[1])) {
	text++;
	if (isdigit(text[1]))
	  text++;
	if (text[1] == ',') {
	  text++;
	  if (isdigit(text[1]))
	    text++;
	  if (isdigit(text[1]))
	    text++;
	}
      }

      if (!1) { /* DELETEME!!! */
      /* from src/dcc.c */
      if (isdigit(text[1])) {	/* Is the first char a number? */
	text++;		/* Skip over the ^C and the first digit */
	if (isdigit(text[1]))
	  text++;		/* Is this a double digit number? */
	if (text[1] == ',') {	/* Do we have a background color next? */
	  if (isdigit(text[2]))
	    text += 2;	/* Skip over the first background digit */
	  if (isdigit(text[1]))
	    text++;		/* Is it a double digit? */
	}
      }
      }


    } else if ((c == 2) || (c == 7) || (c == 0x16) || (c == 0x1f)) {
      /* do nothing, just ignore those $§%#&-codes! */
      debug0("deleteme (mini_httpd.c, text2html())");
    } else {
      snprintf(ubuf, sizeof(ubuf), "&#%d;", (unsigned int) c);
      strcpy(buf, ubuf);
      buf += strlen(ubuf);
    }
    text++;
  }
  *buf = 0;
  httpd_text_buf = nrealloc(httpd_text_buf, strlen(httpd_text_buf) + 1);
  return httpd_text_buf;
}

/* encode_url():
 * encodes all special characters in an url
 */
static char encoded_url_buf[128];
static char *eu_last_url;
static char *encode_url(char *url)
{
  char *buf;
  unsigned char c;

  Assert(url);
  // if we are going to re-encode the same URL again, then
  // save some CPU time and just return our buffer again
  // (I guess noone would mess with that buffer, so it _should_
  //  be save)
  if (url == eu_last_url)
    return encoded_url_buf;
  else
    eu_last_url = url;
  buf = encoded_url_buf;
  while (url[0] && (buf < (encoded_url_buf + 120))) {
    c = url[0];
    if (((c >= 97) && (c <= 122)) || ((c >= 65) && (c <= 90))) {
      buf[0] = c;
      buf++;
    } else {
      buf[0] = '%';
      buf++;
      snprintf(buf, 3, "%02x", c);
      buf += 2;
    }
    url++;
  }
  buf[0] = 0;
  return encoded_url_buf;
}

/* decode_url():
 * Decodes all special characters(%3F == '?', %21 == '!', etc)
 * and returns the decoded url
 */
static char *decode_url(char *paramurl)
{
  char *p, *buf, *url, c, hex[5];
  long int i;

  Context;
  // free url-buffer (global var)
  if (httpd_text_buf)
    nfree(httpd_text_buf);
  // initialize url-buffer
  httpd_text_buf = nmalloc(1);
  httpd_text_buf[0] = 0;
  // copy parameter into a buffer
  buf = nmalloc(strlen(paramurl) + 1);
  strcpy(buf, paramurl);
  url = buf;
  // now loop to get every encoded char
  while ((p = strchr(url, '%'))) {
    // '%' marks the beginning of an encoded char, so cut the string here.
    p[0] = 0;
    // append the first part of the string to our buffer
    httpd_text_buf = nrealloc(httpd_text_buf, strlen(httpd_text_buf) + strlen(url) + 1);
    strcat(httpd_text_buf, url);
    // set the pointer to the beginning of the next string
    url = p + 1;
    // first check if there are enough chars left to decode
    if (strlen(url) >= 2) {
      // the number behind '%' is a hex-number which is the ASCII code of
      // the char, so dump the hex into a string and scan it
      snprintf(hex, 5, "0x%c%c", p[1], p[2]);
      i = strtol(hex, NULL, 0);
      if (!i) {
        i = '?';
        debug0("MiniHTTPd: decode_url(): i is 0");
      }
      c = (char) i;
      // now append the decoded char to the url
      httpd_text_buf = nrealloc(httpd_text_buf, strlen(httpd_text_buf) + 1 + 1);
      sprintf(httpd_text_buf, "%s%c", httpd_text_buf, c);
      // increase the pointer to abandon the encoded char
      url += 2;
    } else {
      // just append the original '%' if there weren't enough chars to decode
      httpd_text_buf = nrealloc(httpd_text_buf, strlen(httpd_text_buf) + 1 + 1);
      strcat(httpd_text_buf, "%");
    }
  }
  // finally append the rest of the param to our buffer. There are no encoded
  // chars left.
  httpd_text_buf = nrealloc(httpd_text_buf, strlen(httpd_text_buf) + strlen(url) + 1);
  strcat(httpd_text_buf, url);
  // free the buffer
  nfree(buf);
  Context;
  return httpd_text_buf;
}

static void timeout_listen_httpd(int idx)
{
  debug0("timeout httpd listen");
  killsock(dcc[idx].sock);
  lostdcc(idx);
}
