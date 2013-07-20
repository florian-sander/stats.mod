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

static globstats *glob_globstats;
static locstats *glob_locstats;
static int glob_timerange, glob_sorting, glob_place, glob_cl_timerange;
static int glob_au_percent, glob_wordplace, glob_graph_total, glob_range;
static int glob_loginerror, glob_activity_timerange, glob_activity;
static int glob_activity_percent, glob_top_start, glob_top_end;
static char *glob_toptype, *glob_nick, *glob_word;
static struct stats_url *glob_url;
static float glob_au_users, glob_graph_percent, glob_graph_width_percent;
static hoststr *glob_isp, *glob_tld;
static topicstr *glob_topic;
static struct stats_kick *glob_kick;
static struct stats_quote *glob_kick_context;
static wordstats *glob_wordstats;
#ifndef NO_EGG
//static memberlist *glob_chanmember;
#endif
static struct stats_member *glob_statsmember;
static struct slang_header *glob_slang;
static struct template_skin *glob_skin;
static struct stats_userlist *glob_user;


static void init_global_vars()
{
//  slang_text_buffer = 0;
}

static void reset_global_vars()
{
  glob_globstats = NULL;
  glob_locstats = NULL;
  glob_toptype = NULL;
  glob_timerange = T_ERROR;
  glob_sorting = T_ERROR;
  glob_place = 0;
  glob_au_users = 0.0;
  glob_au_percent = 0;
  glob_url = NULL;
  glob_isp = glob_tld = NULL;
  glob_topic = NULL;
  glob_kick = NULL;
  glob_kick_context = NULL;
  glob_wordstats = NULL;
  glob_wordplace = 0;
#ifndef NO_EGG
//  glob_chanmember = NULL;
#endif
  glob_statsmember = NULL;
  glob_graph_percent = 0.0;
  glob_graph_width_percent = 0.0;
  glob_slang = NULL;
  glob_nick = NULL;
  glob_range = 0;
  glob_word = NULL;
  glob_loginerror = 0;
  glob_user = NULL;
  glob_activity_timerange = glob_activity = glob_activity_percent = 0;
  glob_cl_timerange = 0;
  glob_top_start = 1;
  glob_top_end = webnr;
}

static void free_global_vars()
{
  Context;
}
