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

static int stats_checkhand (char *oldnick, char *newnick)
{

  Context;
  if (track_stat_user(oldnick, newnick))
    putlog(LOG_MISC, "*", "Stats.mod: Transferred stats from %s to %s", oldnick, newnick);
  Context;
  return 1;
}

static cmd_t stats_nkch[] =
{
  {"*", "", (Function) stats_checkhand, "stat:nkch"},
  {0, 0, 0, 0}
};

static void stats_autoadd(memberlist *m, char *chan)
{
  struct userrec *u;
  char s[121], s2[121], host[121];
  struct xtra_key *xk;
  time_t tt;

  Context;
  if (!use_userfile)
    return;
  if (autoadd < 0)
    return;
  if (!m->nick[0])
    return;
  if ((now - m->joined) < (autoadd * 60))
    return;
  if (match_my_nick(m->nick))
    return;
  if ((strlen(m->nick) + strlen(m->userhost)) >= 120)
    return;
  sprintf(s, "%s!%s", m->nick, m->userhost);
  if (strchr(s, '*') || strchr(s, '?'))
    return;     /* don't add users with wildcards in their hostmask */
  maskhost(s, host);
  u = get_user_by_host(s);
  if (u)
    return;
  u = get_user_by_handle(userlist, m->nick);
  if (u) {
    if (!matchattr(u, badflags, chan)) {
      addhost_by_handle(m->nick, host);
      putlog(LOG_MISC, "*", "Stats.mod: Added hostmask %s to %s.", host, m->nick);
      return;
    }
  } else {
    userlist = adduser(userlist, m->nick, host, "-", USER_DEFAULT);
    u = get_user_by_handle(userlist, m->nick);
    xk = user_malloc(sizeof(struct xtra_key));
    xk->key = user_malloc(7);
    strcpy(xk->key, "AADDED");
    xk->data = user_malloc(10);
    sprintf(xk->data, "%09lu", now);
    set_user(&USERENTRY_XTRA, u, xk);
    tt = now;
    strftime(s2, 120, "Added by Stats.mod on %d.%m.%Y %H:%M.", localtime(&tt));
    /* sprintf(s2, "Added by Stats.mod on %s", ctime(&tt)); */
    set_user(&USERENTRY_COMMENT, u, (void *) s2);
    set_handle_laston(chan, u, now);
    putlog(LOG_MISC, "*", "Stats.mod: Added %s to userlist.", m->nick);
  }
  Context;
}

static void deloldstatusers()
{
  struct laston_info *li;
  struct xtra_key *xk;
  struct userrec *u, *lu;
  int changed, del;
  struct chanset_t *chan;
  struct stats_userlist *su, *lsu;
  struct stats_hostlist *h, *lh;

  if (stat_expire_user < 1)
    return;
  u = userlist;
  while (u) {
    changed = 0;
    del = 1;
    li = get_user(&USERENTRY_LASTON, u);
    if (li) {
      if ((now - li->laston) > (stat_expire_user * 86400)) {
        for (xk = get_user(&USERENTRY_XTRA, u); (xk && del); xk = xk->next) {
          if (!strcasecmp(xk->key, "AADDED")) {
            if(!get_user(&USERENTRY_PASS, u)) {
	      for (chan = chanset; chan; chan = chan->next) {
#if EGG_IS_MIN_VER(10500)
                if (matchattr(u, badflags, chan->dname)) {
#else
                if (matchattr(u, badflags, chan->name)) {
#endif
		  del = 0;
		  break;
		}
	      }
	      if (!del)
	        break;
              putlog(LOG_MISC, "*", "Stats.mod: %s wasn't seen for over %d days. Deleted from the eggdrop userfile.", u->handle, stat_expire_user);
              lu = u->next;
              deluser(u->handle);
              u = lu;
              changed = 1;
              break;
            }
          }
        }
      }
    }
    if (!changed)
      u = u->next;
  }
  su = suserlist;
  lsu = NULL;
  while (su) {
    h = su->hosts;
    lh = NULL;
    while (h) {
      if ((now - h->lastused) > (stat_expire_user * 86400)) {
	putlog(LOG_MISC, "*", "Stats.Mod: %s didn't use the hostmask %s during the last %d days. Removing from hostlist...",
	       su->user, h->mask, stat_expire_user);
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
    if (!su->hosts) {
      u = get_user_by_handle(userlist, su->user);
      if (u) {
        lsu = su;
        su = su->next;
        continue;
      }
      putlog(LOG_MISC, "*", "Stats.Mod: All of %s's hosts expired. Deleting stat user...", su->user);
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
  struct chanset_t *chan;

  Context;
  gs = sdata;
  gs2 = NULL;
  while (gs) {
#ifndef OLDBOT
    chan = findchan_by_dname(gs->chan);
#else
    chan = findchan(gs->chan);
#endif
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
	    if (use_userfile && u2) {
	      if (strcmp(ls->user, u2->handle)) {
	        debug2("Stats.mod: Transferred stats from %s to %s", ls->user, u2->handle);
                nfree(ls->user);
                ls->user = nmalloc(strlen(u2->handle) + 1);
                strcpy(ls->user, u2->handle);
	      }
	    } else if (!use_userfile && u) {
	      if (strcmp(ls->user, u->user)) {
	        debug2("Stats.mod: Transferred stats from %s to %s", ls->user, u->user);
                nfree(ls->user);
                ls->user = nmalloc(strlen(u->user) + 1);
                strcpy(ls->user, u->user);
	      }
	    }
          }
        }
        if (kill) {
          putlog(LOG_MISC, "*", "Stats.mod: Deleting stats for %s in %s(empty data or no such user)", ls->user, gs->chan);
          for (i = 0; i < 4; i++) {
            for (ii = 0; ii < (TOTAL_TYPES + TOTAL_SPECIAL_TYPES); ii++) {
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
                putlog(LOG_MISC, "*", "WARNING!!! %s not found in sorted list ([%d][%d])! Corrupted data?", ls->user, i, ii);
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
      putlog(LOG_MISC, "*", "Stats.mod: Deleting stats for %s. (empty data or no such channel)", gs->chan);
      free_one_chan(gs->chan);
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
