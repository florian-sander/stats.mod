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

static int stats_checkhand(char *oldnick, char *newnick)
{

	Context;
	if (findsuser_by_name(newnick)) {
		putlog(LOG_MISC, "*", "Stats.mod: %s changed handle to %s which already "
				"existed in the database. The datasets have been merged.",
				oldnick, newnick);
		if (!user_merge(newnick, oldnick))
			putlog(LOG_MISC, "*", ".. failed!");
	} else {
		if (track_stat_user(oldnick, newnick))
			putlog(LOG_MISC, "*", "Stats.mod: Transferred stats from %s to %s",
				   oldnick, newnick);
		else
			putlog(LOG_MISC, "*", "Stats.mod: Transfer from %s to %s failed!", oldnick, newnick);
	}
	Context;
	return 1;
}

static cmd_t stats_nkch[] = {
	{"*", "", (Function) stats_checkhand, "stat:nkch"},
	{0, 0, 0, 0}
};

static void deloldstatusers()
{
	struct userrec *u;
	struct stats_userlist *su, *lsu;
	struct stats_hostlist *h, *lh;

	if (expire_base < 1)
		return;
	su = suserlist;
	lsu = NULL;
	while (su) {
		h = su->hosts;
		lh = NULL;
		while (h) {
			if ((now - h->lastused) > TIMETOLIVE(h)) {
				putlog(LOG_MISC, "*",
					   "Stats.Mod: %s didn't use the hostmask %s during the last %d days. Removing from hostlist...",
					   su->user, h->mask, ((now - h->lastused) / 86400));
				nfree(h->mask);
				if (lh)
					lh->next = h->next;
				else
					su->hosts = h->next;
				nfree(h);
				if (lh)
					h = lh->next;
				else
					h = su->hosts;
			} else {
				lh = h;
				h = h->next;
			}
		}
		if (!su->hosts && (TIMETOLIVE(su) < (now - su->laston))) {
			u = get_user_by_handle(userlist, su->user);
			if (u) {
				lsu = su;
				su = su->next;
				continue;
			}
			putlog(LOG_MISC, "*",
				   "Stats.Mod: %s wasn't online since %d days. "
				   "Deleting stat user...", su->user,
				   (now - su->laston) / 86400);
			nfree(su->user);
			if (lsu)
				lsu->next = su->next;
			else
				suserlist = su->next;
			weed_userlink_from_chanset(su);
			weed_userlink_from_locstats(su);
			nfree(su);
			if (lsu)
				su = lsu->next;
			else
				su = suserlist;
		} else {
			lsu = su;
			su = su->next;
		}
	}
}

static void purgestats()
{
	globstats *gs, *gs2;
	locstats *ls, *ls2;
	locstats *sl, *sl2;
	int i, ii, kill;
	struct stats_userlist *u;
	struct userrec *u2;
	struct stats_chan *chan;

	Context;
	gs = sdata;
	gs2 = NULL;
	while (gs) {
		chan = schan_find(gs->chan);
		if (chan && gs->local) {
			ls = gs->local;
			ls2 = NULL;
			while (ls) {
				kill = 1;
				u2 = get_user_by_handle(userlist, ls->user);
				u = findsuser_by_name(ls->user);
				if (u2 || u) {
					for (i = 0; i < TOTAL_TYPES; i++) {
						if (ls->values[S_TOTAL][i] != 0) {
							kill = 0;
							break;
						}
					}
					if (!kill) {
						if (strcmp(ls->user, u->user)) {
							debug2
							   ("Stats.mod: Transferred stats from %s to %s",
								ls->user, u->user);
							nfree(ls->user);
							ls->user = nmalloc(strlen(u->user) + 1);
							strcpy(ls->user, u->user);
						}
					}
				}
				if (kill) {
					putlog(LOG_MISC, "*",
						   "Stats.mod: Deleting stats for %s in %s(empty data or no such user)",
						   ls->user, gs->chan);
					for (i = 0; i < 4; i++) {
						for (ii = 0; ii < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES);
							 ii++) {
							sl = gs->slocal[i][ii];
							sl2 = NULL;
							while (sl) {
								if (!rfc_casecmp(sl->user, ls->user))
									break;
								sl2 = sl;
								sl = sl->snext[i][ii];
							}
							if (sl) {
								if (sl2)
									sl2->snext[i][ii] = sl->snext[i][ii];
								else
									gs->slocal[i][ii] = sl->snext[i][ii];
							} else
								putlog(LOG_MISC, "*",
									   "WARNING!!! %s not found in sorted list ([%d][%d])! Corrupted data?",
									   ls->user, i, ii);
						}
					}
					if (ls2)
						ls2->next = ls->next;
					else
						gs->local = ls->next;
					nfree(ls->user);
					free_wordstats(ls->words);
					free_quotes(ls->quotes);
					weed_statlink_from_chanset(ls);
					nfree(ls);
					if (ls2)
						ls = ls2->next;
					else
						ls = gs->local;
				} else {
					ls2 = ls;
					ls = ls->next;
				}
			}
			gs2 = gs;
			gs = gs->next;
		} else {
			putlog(LOG_MISC, "*",
				   "Stats.mod: Deleting stats for %s. (no such channel)", gs->chan);
			if (gs2)
				gs2->next = gs->next;
			else
				sdata = gs->next;
			free_localstats(gs->local);
			free_wordstats(gs->words);
			free_topics(gs->topics);
			free_urls(gs->urls);
			free_quotes(gs->log);
			free_hosts(gs->hosts);
			free_kicks(gs->kicks);
			nfree(gs->chan);
			nfree(gs);
			if (gs2)
				gs = gs2->next;
			else
				gs = sdata;
		}
	}
	Context;
}
