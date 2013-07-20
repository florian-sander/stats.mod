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

static char *inverted_csplit(char **rest, char divider);
static char *stats_duration(int seconds, int details);
static int countsmileys(char *text);
static int countsmileys(char *text);
static int countwords(char *buf);
static int countquestions(char *buf);
static void strlower(char *text);
static int gethour();
static int getmonth();
static int ismonday();
static void maskstricthost(const char *s, char *nw);
static int get_timerange(char *text);
static int email_send(char *to, char *subject, char *body);
