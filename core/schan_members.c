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

static int schan_members_compare(void *data, void *key)
{
	if (!rfc_casecmp(((struct stats_member *)data)->nick, (char *)key))
		return 1;
	return 0;
}

static int schan_members_expmem(void *data)
{
	struct stats_member *p = (struct stats_member *)data;
	int size = 0;

	size += sizeof(struct stats_member);
	size += strlen(p->nick) + 1;
	size += strlen(p->uhost) + 1;
	return size;
}

static void schan_members_free(void *data)
{
	struct stats_member *p = (struct stats_member *)data;

	nfree(p->nick);
	nfree(p->uhost);
	nfree(p);
}

static void schan_members_llist_init(struct llist_header *head)
{
	head->root = NULL;
	head->size = 0;
	head->comparedata = schan_members_compare;
	head->expmemdata = schan_members_expmem;
	head->freedata = schan_members_free;
}

static struct stats_member *schan_members_create()
{
	struct stats_member *nm;

	nm = nmalloc(sizeof(struct stats_member));
	nm->nick = NULL;
	nm->uhost = NULL;
	nm->joined = 0;
	nm->last = now;
	nm->user = NULL;
	nm->stats = NULL;
	nm->spoken_lines = 0;
	return nm;
}

static void schan_members_join(struct llist_header *head, char *nick, char *uhost, char *user, char *chan)
{
	struct stats_member *m;
	char *host;
#ifndef NO_EGG
	struct chanset_t *eggchan;
#endif

	m = schan_members_create();
	m->nick = nmalloc(strlen(nick) + 1);
	strcpy(m->nick, nick);
	m->uhost = nmalloc(strlen(uhost) + 1);
	strcpy(m->uhost, uhost);
	m->joined = now;
	if (user) {
		m->user = findsuser_by_name(user);
		if (!m->user) {
			m->user = addsuser(user, now, now);
			debug1("Stats.Mod: Created suserrec for %s.", user);
		}
	} else {
		host = nmalloc(strlen(nick) + 1 + strlen(uhost) + 1);
		sprintf(host, "%s!%s", nick, uhost);
		m->user = findsuser(host);
		nfree(host);
	}
	if (m->user) {
		m->user->laston = now;
		m->stats = findlocstats(chan, m->user->user);
		if (!m->stats)
			m->stats = initstats(chan, m->user->user);
	}
#ifndef NO_EGG
	eggchan = findchan_by_dname(chan);
	if (chan)
		m->eggmember = ismember(eggchan, nick);
	if (!m->eggmember)
		debug2("Warning[stats.mod]: Couldn't find eggmember for %s in %s.", nick, chan);
#endif
	llist_append(head, (void *) m);
}

static void schan_members_update(struct stats_member *m, char *chan)
{
	char *host;
#ifndef NO_EGG
	struct userrec *u;
#endif

	m->user = NULL;
	host = nmalloc(strlen(m->nick) + 1 + strlen(m->uhost) + 1);
	sprintf(host, "%s!%s", m->nick, m->uhost);
#ifndef NO_EGG
	u = get_user_by_host(host);
	if (u) {
		m->user = findsuser_by_name(u->handle);
		if (!m->user) {
	  		m->user = addsuser(u->handle, now, now);
	  		debug1("Stats.Mod: Created suserrec for %s.", u->handle);
		}
	} else
#endif
		m->user = findsuser(host);
	nfree(host);
	if (m->user) {
		m->stats = findlocstats(chan, m->user->user);
		if (!m->stats)
			m->stats = initstats(chan, m->user->user);
	}
}

static void schan_members_leave(struct llist_header *head, char *nick)
{
	Assert(head);
	llist_delete(head, (void *)nick);
}

static void schan_members_rename(struct llist_header *head, char *oldnick, char *newnick)
{
	struct stats_member *m;

	m = llist_find(head, (void *)oldnick);
	if (!m)
		return;
	Assert(newnick);
	m->nick = nrealloc(m->nick, strlen(newnick) + 1);
	strcpy(m->nick, newnick);
}

static struct stats_member *schan_members_find(struct llist_header *head, char *nick)
{
	return llist_find(head, (void *)nick);
}

static struct stats_member *schan_members_getfirst(struct llist_header *head)
{
	return (struct stats_member *)llist_getfirst(head);
}

static struct stats_member *schan_members_getnext(struct llist_header *head)
{
	return (struct stats_member *)llist_getnext(head);
}

static int schan_members_count(struct llist_header *head)
{
	return head->size;
}
