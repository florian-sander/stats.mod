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

#include "../settings.h"

static void sensor_peak(struct stats_chan *chan);

static void sensor_text(struct stats_chan *chan, char *nick, char *text)
{
	int i, hour;
	char buf[511];
	struct stats_member *m;

	Assert(chan);
	Assert(chan->stats);
	if (nostats(chan->chan))
		return;
	strncpy(buf, text, 510);
	buf[510] = 0;
	text = buf;

	hour = gethour();
	chan->stats->activity[hour]++;
	add_chanlog(chan->stats, nick, text, SL_PRIVMSG);
	m = schan_members_find(&chan->members, nick);
	if (!m) {
		check_for_url(nick, chan->chan, text);
		return;
	}
	m->last = now;
	// increase spoken lines (needed for autoadd)
	// if there's no link to the stats, call initstats() which either
	// returns an existing stats struct, or initializes a new one
	m->spoken_lines++;
	if (!m->stats && m->user) {
		m->stats = initstats(chan->chan, m->user->user);
	}
	if (!m->stats || !m->user) {
		check_for_url(nick, chan->chan, text);
		return;
	}
	check_for_url(m->user->user, chan->chan, text);
	m->stats->lastspoke = now;
	nincrstats(m->stats, T_WORDS, countwords(text));
	nincrstats(m->stats, T_LETTERS, strlen(text));
	nincrstats(m->stats, T_LINES, 1);
	i = countsmileys(text);
	if (i)
		nincrstats(m->stats, T_SMILEYS, i);
	i = countquestions(text);
	if (i)
		nincrstats(m->stats, T_QUESTIONS, i);
	addquote(m->stats, text);
	/* always use calcwordstats() at the end, since
	 * it splits the string */
	calcwordstats(m->stats, text);
	return;
}

static void sensor_topic(char *nick, struct stats_chan *chan, char *topic)
{
	struct stats_member *m;

	Context;
	Assert(chan);
	if (nostats(chan->chan))
		return;
	m = schan_members_find(&chan->members, nick);
	if (m && m->stats)
		nincrstats(m->stats, T_TOPICS, 1);
	addtopic(chan->chan, topic, nick);
	return;
}

static void sensor_action(char *nick, struct stats_chan *chan, char *text)
{
	char *pbuf;
	struct stats_member *m;

	Assert(chan);
	if (nostats(chan->chan))
		return;
	m = schan_members_find(&chan->members, nick);
	if (!m)
		return;
	if (!m->stats)
		return;
	nincrstats(m->stats, T_ACTIONS, 1);
	pbuf = nmalloc(strlen(nick) + strlen(text) + 2);
	sprintf(pbuf, "%s %s", nick, text);
	sensor_text(chan, nick, pbuf);
	nfree(pbuf);
	return;
}

static void sensor_kick(char *nick, struct stats_chan *chan, char *victim, char *reason)
{
	struct stats_member *m;
	char *buf;

	Assert(chan);
	if (nostats(chan->chan))
		return;
	Assert(chan->stats);
	buf = nmalloc(strlen(victim) + strlen(nick) + strlen(reason) + 23);
	sprintf(buf, "*** %s was kicked by %s (%s)", victim, nick, reason);
	add_chanlog(chan->stats, nick, buf, SL_KICK);
	save_kick(chan->stats, buf);
	nfree(buf);
	m = schan_members_find(&chan->members, nick);
	if (!m)
		return;
	if (!m->stats)
		return;
	nincrstats(m->stats, T_KICKS, 1);
}

static void sensor_mode(char *nick, struct stats_chan *chan, char *mode, char *victim)
{
	struct stats_member *m;
	char *buf;

	Assert(chan);
	if (nostats(chan->chan))
		return;
	Assert(mode);
	Assert(chan->stats);
	if (mode[1] != 'k') {
		// log everything except key changes (you don't want your channel
		// key displayed on the webpages, do you?
		buf = nmalloc(strlen(nick) + strlen(mode) + strlen(victim) + 13);
		sprintf(buf, "%s sets mode %s %s", nick, mode, victim);
		add_chanlog(chan->stats, nick, buf, SL_MODE);
		nfree(buf);
	}
	m = schan_members_find(&chan->members, nick);
	if (!m)
		return;
	if (!m->stats)
		return;
	nincrstats(m->stats, T_MODES, 1);
	if ((mode[1] == 'b') && (mode[0] == '+'))
		nincrstats(m->stats, T_BANS, 1);
}

static void sensor_nick(char *nick, struct stats_chan *chan, char *newnick)
{
	struct stats_member *m;

	Assert(chan);
	if (nostats(chan->chan))
		return;
	Assert(chan->stats);
	add_chanlog(chan->stats, nick, newnick, SL_NICK);
	m = schan_members_find(&chan->members, nick);
	if (!m)
		return;
	if (!m->stats)
		return;
	nincrstats(m->stats, T_NICKS, 1);
}

static void sensor_join(char *nick, char *uhost, struct stats_chan *chan)
{
	struct stats_member *m;

	Assert(chan);
	if (nostats(chan->chan))
		return;
	Assert(chan->stats);
	add_chanlog(chan->stats, nick, "", SL_JOIN);
	sensor_peak(chan);
	m = schan_members_find(&chan->members, nick);
	if (!m)
		return;
	if (!m->stats)
		return;
	nincrstats(m->stats, T_JOINS, 1);
	addhost(uhost, chan->stats);
}

static void sensor_left(char *nick, struct stats_chan *chan, int type)
{
	Assert(chan);
	if (nostats(chan->chan))
		return;
	Assert(chan->stats);
	add_chanlog(chan->stats, nick, "", type);
}

/* sensor_minutely():
 * - increases the spent time for each registered user
 * - if the time already got increased (if the user has a clone
 *   in the channel, for example) then it won't be increased again
 *   (thanks to Zev for this idea)
 * - if the user is not registered, stats_autosadd() gets called to
 *   check if we might want to add him/her
 * - count how many users there are in the chan
 */
static void sensor_minutely()
{
	struct stats_chan *chan;
	struct stats_member *m;
	int nr, hour;
	globstats *gs;

	Context;
	for (chan = schan_getfirst(); chan; chan = schan_getnext()) {
		if (nostats(chan->chan))
			continue;
		nr = 0;
		gs = chan->stats;
		for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
			if (m->stats) {
				if (m->stats->flag)
					continue;
				if (m->user && suser_list(m->user))
					nr++;
				nincrstats(m->stats, T_MINUTES, 1);
				m->stats->flag = 1;
			} else
				stats_autosadd(m, chan);
		}
		for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members))
			if (m->stats)
				m->stats->flag = 0;
		hour = gethour();
		if (hour != lasthour) {
			gs->users[S_USERSUM][hour] = nr;
			gs->users[S_USERCOUNTS][hour] = 1;
			lasthour = hour;
		} else {
			gs->users[S_USERSUM][hour] += nr;
			if (gs->users[S_USERCOUNTS][hour] < 0)
				gs->users[S_USERCOUNTS][hour] = 1;
			else
				gs->users[S_USERCOUNTS][hour]++;
		}
	}
}

static void sensor_peak(struct stats_chan *chan)
{
	struct stats_member *m;
	int users = 0;
	globstats *gs;

	Assert(chan);
	Assert(chan->stats);
/*	if (nostats(chan->chan))
		return; */
	gs = chan->stats;
	for (m = schan_members_getfirst(&chan->members); m; m = schan_members_getnext(&chan->members)) {
		if (m->user && !suser_list(m->user))
			continue;
		users++;
	}
	if (users > gs->peak[S_TOTAL]) {
		gs->peak[S_TOTAL] = users;
		putlog(LOG_MISC, "*", "New user peak in %s: %d.", chan->chan, users);
	}
	if (users > gs->peak[S_TODAY])
		gs->peak[S_TODAY] = users;
	if (users > gs->peak[S_WEEKLY])
		gs->peak[S_WEEKLY] = users;
	if (users > gs->peak[S_MONTHLY])
		gs->peak[S_MONTHLY] = users;
}
