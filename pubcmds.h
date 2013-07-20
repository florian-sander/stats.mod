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

static char *tell_ntop(char *chan, char *params, int last);
static char *tell_top_word(char *chan, char *word, int range, globstats *gs);
static char *tell_place(char *nick, char *hand, char *channel, char *text);
static char *tell_stat(char *nick, char *channel, char *text);
static void tell_wordstats(char *nick, char *dest, char *hand, char *channel, char *text);
static void tell_topwords(char *nick, char *dest, char *hand, char *channel);
