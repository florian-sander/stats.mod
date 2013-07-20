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

struct stats_member {
  char *nick;
  char *uhost;
  time_t joined;
  time_t last;
  struct stats_userlist *user;
  locstats *stats;
  int spoken_lines;
  memberlist *eggmember;
};

static void schan_members_llist_init(struct llist_header *head);
static void schan_members_join(struct llist_header *head, char *nick, char *uhost, char *user, char *chan);
static void schan_members_leave(struct llist_header *head, char *nick);
