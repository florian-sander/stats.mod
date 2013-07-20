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

static int eggbnd_pubm(char *nick, char *uhost, char *hand, char *channel, char *rest)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan)
		sensor_text(chan, nick, rest);
	return 0;
}

static int eggbnd_topc(char *nick, char *uhost, char *hand, char *channel, char *topic)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan)
		sensor_topic(nick, chan, topic);
	return 0;
}

static int eggbnd_action(char *nick, char *uhost, char *hand, char *channel,
						 char *key, char *rest)
{
	struct stats_chan *chan;
	
	if (strchr(CHANMETA, channel[0])) {
		chan = schan_find(channel);
		if (chan)
			sensor_action(nick, chan, rest);
	}
	return 0;
}

static int eggbnd_kick(char *nick, char *uhost, char *hand, char *channel,
					   char *victim, char *reason)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan) {
		sensor_kick(nick, chan, victim, reason);
		schan_leave(chan, victim);
	}
	return 0;
}

static int eggbnd_mode(char *nick, char *uhost, char *hand, char *channel,
					   char *mode, char *victim)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan)
		sensor_mode(nick, chan, mode, victim);
	return 0;
}

static int eggbnd_nick(char *nick, char *uhost, char *hand, char *channel,
					   char *newnick)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan) {		
		sensor_nick(nick, chan, newnick);
		schan_members_rename(&chan->members, nick, newnick);
	}
	return 0;
}

static int eggbnd_join(char *nick, char *uhost, char *hand, char *channel)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (!chan)
		chan = schan_add(channel);
	schan_join(chan, nick, uhost, *hand != '*' ? hand : NULL);
	sensor_join(nick, uhost, chan);
	return 0;
}

static int eggbnd_part(char *nick, char *uhost, char *hand, char *channel)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan) {
		sensor_left(nick, chan, SL_PART);
		schan_leave(chan, nick);
	}
	return 0;
}

static int eggbnd_sign(char *nick, char *uhost, char *hand, char *channel,
					   char *reason)
{
	struct stats_chan *chan;
	
	chan = schan_find(channel);
	if (chan) {
		sensor_left(nick, chan, SL_QUIT);
		schan_leave(chan, nick);
	}
	return 0;
}

static int eggbnd_minutely()
{
	sensor_minutely();
	egg_check_chan_desynch();
}

static cmd_t stats_pubm[] =
{
  {"*", "", (Function) eggbnd_pubm, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_topc[] =
{
  {"*", "", (Function) eggbnd_topc, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_ctcp[] =
{
  {"ACTION", "", (Function) eggbnd_action, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_kick[] =
{
  {"*", "", (Function) eggbnd_kick, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_mode[] =
{
  {"*", "", (Function) eggbnd_mode, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_nick[] =
{
  {"*", "", (Function) eggbnd_nick, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_join[] =
{
  {"*", "", (Function) eggbnd_join, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_part[] =
{
  {"*", "", (Function) eggbnd_part, "stat"},
  {0, 0, 0, 0}
};

static cmd_t stats_sign[] =
{
  {"*", "", (Function) eggbnd_sign, "stat"},
  {0, 0, 0, 0}
};
