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

static int inactivechan(char *chan)
{
  struct chanset_t *ch;

  ch = findchan_by_dname(chan);
  if (!ch)
    return 0;
  if (ch->status & CHAN_INACTIVE)
    return 1;
  return 0;
}

static int nostats(char *chan)
{
#if EGG_IS_MIN_VER(10503)
  if (ngetudef("nostats", chan))
    return 1;
#endif
  return 0;
}
