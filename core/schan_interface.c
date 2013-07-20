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

/*
static void ci_join(char *nick, char *uhost, char *user, char *channel)
{
  struct stats_chan *chan;

  chan = schan_find(channel);
  if (!chan) {
    chan = schan_add(channel);
  }
  schan_join(chan, nick, uhost, user);
}
*/

/*
static void ci_leave(char *nick, char *channel)
{
	struct stats_chan *chan;

	chan = schan_find(channel);
	if (chan)
	  schan_leave(chan, nick);
}
*/

static void ci_clearchan(struct stats_chan *chan)
{
	struct stats_member *m;

	Assert(chan);
	for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
		schan_members_leave(&chan->members, m->nick);
	}
}

static struct stats_member *getschanmember(char *nick, char *channel)
{
	struct stats_chan *chan;

	chan = schan_find(channel);
	if (chan)
		return schan_members_find(&chan->members, nick);
	else
		return NULL;
}

/* used when shosts/susers changed */
static void update_schannel_members()
{
	struct stats_chan *chan;
	struct stats_member *m;

	Context;
	for (chan = schan_getfirst(); chan; chan = schan_getnext())
		for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members))
			schan_members_update(m, chan->chan);
}
