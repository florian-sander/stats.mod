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



static int countactivestatmembers_by_word(globstats *gs, int listable, int min)
{
  int members = 0;
  locstats *ls;

  Context;
  for (ls = gs->local; ls; ls = ls->next) {
    if (listable && !listsuser(ls, gs->chan))
      continue;
    if (!ls->word)
      continue;
    if (ls->word->nr < min)
      continue;
    members++;
  }
  return members;
}


/* stolen from tcl_matchattr */
/*
static int matchattr (struct userrec *u, char *flags, char *chan) {
  struct flag_record plus, minus, user;
  int ok = 0, f;

#ifndef OLDBOT
  if (u && (!chan || findchan_by_dname(chan))) {
#else
  if (u && (!chan || findchan(chan))) {
#endif
    user.match = FR_GLOBAL | (chan ? FR_CHAN : 0) | FR_BOT;
    get_user_flagrec(u, &user, chan);
    plus.match = user.match;
    break_down_flags(flags, &plus, &minus);
    f = (minus.global || minus.udef_global || minus.chan ||
   minus.udef_chan || minus.bot);
    if (flagrec_eq(&plus, &user)) {
      if (!f)
  ok = 1;
      else {
  minus.match = plus.match ^ (FR_AND | FR_OR);
  if (!flagrec_eq(&minus, &user))
    ok = 1;
      }
    }
  }
  return ok;
}
*/

static int secretchan(char *chan)
{
  struct chanset_t *ch;

  if (list_secret_chans)
    return 0;
  ch = findchan_by_dname(chan);
  if (!ch)
    return 0;
  if (ch->status & CHAN_SECRET)
    return 1;
  return 0;
}



