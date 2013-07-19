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

static int tcl_incrstats STDVAR
{
  int set, type;

  Context;
  BADARGS(5, 6, " user chan type value ?set?");
  if (argc == 6)
    set = atoi(argv[5]);
  else
    set = 0;
  type = typetoi(argv[3]);
  if (type == -3) {
    Tcl_AppendResult(irp, "invalid type", NULL);
    return TCL_ERROR;
  }
  incrstats(argv[1], argv[2], type, atoi(argv[4]), set);
  Context;
  return TCL_OK;
}

static int tcl_getstats STDVAR
{
  char s[30];
  int today = 0;

  Context;
  BADARGS(4, 5, " user chan type ?today?");
  if (typetoi(argv[3]) < 0) {
    Tcl_AppendResult(irp, "invalid type", NULL);
    return TCL_ERROR;
  }
  if (argc == 5)
    today = atoi(argv[4]);
  if ((today != 1) && (today != 0)) {
    Tcl_AppendResult(irp, "invalid today parameter. Must be 0 or 1.", NULL);
    return TCL_ERROR;
  }
  sprintf(s, "%d", getstats(argv[1], argv[2], argv[3], today));
  Tcl_AppendResult(irp, s, NULL);
  Context;
  return TCL_OK;
}

static int tcl_livestats STDVAR
{
  int port;

  Context;
  BADARGS(2, 2, " port");
  if (!strcasecmp(argv[1], "off") || !strcasecmp(argv[1], "0")) {
    stop_listen_livestats();
    return TCL_OK;
  }
  port = atoi(argv[1]);
  if (port < 1) {
    Tcl_AppendResult(irp, "invalid port", NULL);
    return TCL_ERROR;
  }
  start_listen_livestats(port);
  return TCL_OK;
}

static int tcl_resetuser STDVAR
{
  char *user, *chan;
  locstats *ls;

  Context;
  BADARGS(3, 3, " user channel");
  user = argv[1];
  chan = argv[2];
  ls = findlocstats(chan, user);
  if (!ls) {
    Tcl_AppendResult(irp, "couldn't find stats for user", NULL);
    return TCL_ERROR;
  }
  resetlocstats(ls);
  return TCL_OK;
}

static int tcl_loadslang STDVAR
{
  int ret;

  Context;
  BADARGS(2, 3, " [lang] slangfile");
  if (argc == 3)
    ret = loadslang(argv[1], argv[2]);
  else
    ret = loadslang(NULL, argv[1]);
  if (!ret) {
    Tcl_AppendResult(irp, "Couldn't open slang file!!!", NULL);
    return TCL_ERROR;
  }
  return TCL_OK;
}

static int tcl_resetslang STDVAR
{
  Context;
  free_slang();
  slangs = NULL;
  slangchans = NULL;
  return TCL_OK;
}

static int tcl_getslang STDVAR
{
  Context;
  BADARGS(2, 2, " ID");
  Tcl_AppendResult(irp, getslang(atoi(argv[1])), NULL);
  return TCL_OK;
}

static int tcl_nick2suser STDVAR
{
  struct stats_memberlist *m;
  Context;
  BADARGS(3, 3, " nick channel");
  m = nick2suser(argv[1], argv[2]);
  if (m && m->user)
    Tcl_AppendResult(irp, m->user->user, NULL);
  else
    Tcl_AppendResult(irp, "*", NULL);
  return TCL_OK;
}

static int tcl_setchanslang STDVAR
{
  Context;
  BADARGS(3, 3, " channel language");
  setchanlang(argv[1], argv[2]);
  return TCL_OK;
}

static int tcl_findsuser STDVAR
{
  struct stats_userlist *u;
  struct userrec *ou;

  Context;
  BADARGS(2, 2, " nick!user@host");
  ou = get_user_by_host(argv[1]);
  if (ou)
	Tcl_AppendResult(irp, ou->handle, NULL);
  else {
    u = findsuser(argv[1]);
    if (u)
      Tcl_AppendResult(irp, u->user, NULL);
    else
      Tcl_AppendResult(irp, "*", NULL);
  }
  return TCL_OK;
}

static tcl_cmds mytcls[] =
{
  {"incrstats", tcl_incrstats},
  {"getstats", tcl_getstats},
  {"livestats", tcl_livestats},
  {"resetuser", tcl_resetuser},
  {"nick2suser", tcl_nick2suser},
  {"loadslang", tcl_loadslang},
  {"resetslang", tcl_resetslang},
  {"getslang", tcl_getslang},
  {"setchanslang", tcl_setchanslang},
  {"findsuser", tcl_findsuser},
  {0, 0}
};
