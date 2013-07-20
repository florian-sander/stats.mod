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

static void egg_check_chan_desynch()
{
	struct stats_chan *chan;
	struct chanset_t *eggchan;
	memberlist *m;
	int eggmembers, statsmembers;

	for (eggchan = chanset; eggchan; eggchan = eggchan->next) {
		if (!schan_find(eggchan->dname))
			schan_add(eggchan->dname);
	}
	for (chan = schan_getfirst(); chan; chan = schan_getnext()) {
		eggchan = findchan_by_dname(chan->chan);
		if (!eggchan) {
			debug1("Eggdrop doesn't know '%s'. Removing from schanlist...", chan->chan);
			schan_remove(chan->chan);
			continue;
		}
		statsmembers = schan_members_count(&chan->members);
		eggmembers = eggchan->channel.members;
		if (statsmembers != eggmembers) {
			debug3("Channel '%s' desynched: %d eggmembers vs %d statsmembers. Resynching...",
					chan->chan, eggmembers, statsmembers);
			ci_clearchan(chan);
			for (m = eggchan->channel.member; m && m->nick && *m->nick; m = m->next) {
				if (m->userhost)
					schan_join(chan, m->nick, m->userhost, m->user ? m->user->handle : NULL);
			}
		}
	}
}

static int egg_chan_active(char *chan)
{
	if (!list_secret_chans && secretchan(chan))
		return 0;
	if (inactivechan(chan))
		return 0;
	return 1;
}
