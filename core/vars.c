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

static char statsfile[121] = "statsmod.dat";
static char webstats[510] = "words letters lines actions smileys joins kicks modes topics minutes";
static char graphstats[510] = "words letters lines actions smileys joins kicks modes topics minutes";
static char stat_reply[128] = "words letters smileys minutes";
static char graphgif[128] = "";
static char graphcolor[20] = "blue";
static char webdir[256] = "../public_html";
static char smileys[128] = ":-) :) ;) ;-) ^_^ :-D :-P :P =) ;D";
static char badflags[20] = "ofvb|ofv";
static char nostatsflags[20] = "b|-";
static char nopeak[20] = "b|-";
static char network[41] = "unknown-net";
static char default_slang[21] = "en";
static char default_skin[21] = "classic";
static char binary_url[121] = "";
static int statsfilemode = 0600;
static int webnr = 15;
static int graphnr = 15;
static int stats_save_time = 10;
static int autoadd = 5;
// static int stat_expire_user = 30;
// static int stat_expire_delay = 30;
static int write_today = 1;
static int maxstat_thr = 0;
static int maxstat_time = 0;
static int maxlivestats_thr = 0;
static int maxlivestats_time = 0;
static int mstat_thr = 0;
static time_t mstat_time = 0;
static int livestats_timeout;
static int offset = 0;
static int topwords_limit = 5;
static int quote_freq = 10;
static int log_wordstats = 0;
static int lasthour = 0;
static int lastmonth = 0;
static int min_word_length = 0;
static int table_color = 0x3850B8;
static int fade_table_to = 0x000000;
static int table_border = 0;
static int log_urls = 1;
static int kick_context = 0;
static int display_kicks = 0;
static int display_average_users = 1;
static int show_userlist = 1;
static int show_usersonchan = 1;
static int list_secret_chans = 1;
static int autoadd_min_lines = 0;
static int min_lines = 0;
static int expire_base = 7;	/* Minimum time before user gets deleted */
static int expire_factor = 25;	/* percent value which defines how much of
				   a user's "age" he can stay away without
				   being expired */
