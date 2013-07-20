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

#define GENERIC_LINKED_LIST

struct llist_entry
{
	struct llist_entry *next;
	struct llist_entry *last;
	void *data;
};

struct llist_header
{
	struct llist_entry *root;
	struct llist_entry *last;

	int size;
	int (*comparedata) (void *, void *);
	int (*expmemdata) (void *);
	void (*freedata) (void *);
};

static void llist_append(struct llist_header *head, void *data)
{
	struct llist_entry *p, *np;

	np = nmalloc(sizeof(struct llist_entry));

	np->data = NULL;
	np->next = NULL;
	np->last = NULL;

	np->data = data;
	for (p = head->root; p && p->next; p = p->next);
	if (p) {
		p->next = np;
		np->last = p;
	} else
		head->root = np;
	head->size++;
}

static void *llist_find(struct llist_header *head, void *key)
{
	struct llist_entry *p;

	for (p = head->root; p; p = p->next)
		if (head->comparedata(p->data, key))
			return p->data;
	return NULL;
}

static void llist_delete(struct llist_header *head, void *key)
{
	struct llist_entry *p, *last;

	p = head->root;
	last = NULL;
	while (p) {
		if (head->comparedata(p->data, key)) {
			if (last)
				last->next = p->next;
			else
				head->root = p->next;
			head->freedata(p->data);
			if (p->next)
				p->next->last = last;
			/* make sure that the getfirst/getnext-loop can
			 * continue if we are deleting the currently active
			 * item. If last is NULL, then getnext() will continue
			 * with the root which should be the correct successor
			 * of p in this case. */
			if (head->last == p)
				head->last = last;
			nfree(p);
			if (last)
				p = last->next;
			else
				p = head->root;
			head->size--;
		} else {
			last = p;
			p = p->next;
		}
	}
}

static int llist_expmem(struct llist_header *head)
{
	struct llist_entry *p;
	int size = 0;

	for (p = head->root; p; p = p->next) {
		size += sizeof(struct llist_entry);

		size += head->expmemdata(p->data);
	}
	return size;
}

static void llist_free(struct llist_header *head)
{
	struct llist_entry *p, *n;

	p = head->root;
	while (p) {
		n = p->next;
		head->freedata(p->data);
		nfree(p);
		p = n;
	}
	head->root = NULL;
	head->size = 0;
}

static struct llist_entry *llist_getfirst(struct llist_header *head)
{
	Assert(head);
	head->last = head->root;
	if (head->last)
		return head->last->data;
	else
		return NULL;
}

static struct llist_entry *llist_getnext(struct llist_header *head)
{
	/* if head->last exists, then we can just proceed to the next
	 * entry. If it does not exist, then it was the first entry in
	 * the chain and got deleted while we were looping, so we can
	 * simply proceed with the root which should be the next item. */
	if (head->last)
		head->last = head->last->next;
	else
		head->last = head->root;
	if (head->last)
		return head->last->data;
	else
		return NULL;
}
