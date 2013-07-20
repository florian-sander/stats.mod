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

//static struct stats_chanset *schans = NULL;

/*schanset.root = NULL;
schanset.compare = schan_compare;
schanset.expmem = schan_expmem;
schanset.free = schan_free;*/

static int schan_compare(void *data, void *key)
{
	if (!rfc_casecmp(((struct stats_chan *)data)->chan, (char *)key))
		return 1;
	else
		return 0;
}

static int schan_expmem(void *data)
{
	int size = 0;

	size += sizeof(struct stats_chan);
	size += strlen(((struct stats_chan *) data)->chan) + 1;
	size += llist_expmem(&(((struct stats_chan *)data)->members));
	return 0;
}

static void schan_free(void *data)
{
	struct stats_chan *p = (struct stats_chan *) data;

	llist_free(&(p->members));
	nfree(p->chan);
	nfree(p);
}

static struct stats_chan *schan_find(char *name)
{
	return (struct stats_chan *) llist_find(&schanset, (void *) name);
}


static struct stats_chan *schan_create(char *name)
{
	struct stats_chan *ch;

	ch = nmalloc(sizeof(struct stats_chan));
	ch->chan = nmalloc(strlen(name) + 1);
	strcpy(ch->chan, name);
	schan_members_llist_init(&ch->members);
	ch->stats = findglobstats(name);
	if (!ch->stats)
		ch->stats = globstats_create(name);
	return ch;
}

static struct stats_chan *schan_add(char *name)
{
	struct stats_chan *ch;

	ch = schan_find(name);
	if (ch)
		return ch;
	ch = schan_create(name);
	llist_append(&schanset, (void *) ch);
	return ch;
}

static void schan_remove(char *name)
{
	llist_delete(&schanset, (void *) name);
}

static struct stats_chan *schan_getfirst()
{
	return (struct stats_chan *)llist_getfirst(&schanset);
}

static struct stats_chan *schan_getnext()
{
	return (struct stats_chan *)llist_getnext(&schanset);
}

static void schan_join(struct stats_chan *chan, char *nick, char *uhost,
					   char *user)
{
	Assert(chan);
	schan_members_join(&chan->members, nick, uhost, user, chan->chan);
}

static void schan_leave(struct stats_chan *chan, char *nick)
{
	Assert(chan);
	schan_members_leave(&chan->members, nick);
}

