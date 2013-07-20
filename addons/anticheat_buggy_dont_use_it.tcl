# AntiStatCheat v1.2.0

# this script is intended for use with stats.mod
# (http://www.visions-of-fantasy.de/stats.mod/)

# This script should prevent any cheating attempts
# Depending on the configuration, it either doesn't increase
# the stats during the cheat attempt, subtracts the words
# or does nothing at all and calculates the stats as usual.

# Cheating attempts get logged to a certain loglevel (2 by default)
# set your console to +2 and the channel you want to monitor to
# watch the bots reaction on cheating attempts.

### CONFIGURATION ###

# if a user talks more than x lines in a row without another
# user interfering, then he or she is probably trying to cheat
set acset(monologue) 7

# the following settings control the way how the bot should react
# on different cheating attempts.
# 0: do nothing
# 1: don't increase the stats
# 2: subtract words

# Repeating: This is usually a real cheating attempt, so the default
#            treatment is subtracting the words.
set acset(repeatpunish) 2
# Monologue: This is also a strong indication of a cheating attempt.
set acset(monologuepunish) 2
# Colors: Users who use colors aren't necessarly cheating, so just
#         don't increase the stats and don't punish them.
set acset(colorpunish) 1

# if the average wordlength in a line is smaller than this value, then
# someone probably tried to cheat ("a a a a a a")
set acset(wlen) 2.0
# 
set acset(wlenpunish) 1

# the loglevel where the bot logs the cheating attempts to
set acset(loglev) 2

# the time in seconds for that the bot remembers what the user has spoken
# to detect repeating.
set acset(tracktime) 240

### END OF CONFIGURATION ###

catch "unbind pubm - * *pubm:stat"
bind pubm - * pubm:stat
proc pubm:stat {nick uhost hand chan rest} {
  if {![stat:cheated $nick $hand $chan $rest words]} {
    *pubm:stat $nick $uhost $hand $chan $rest
  }
}

catch "unbind ctcp - ACTION *ctcp:stat"
bind ctcp - ACTION ctcp:stat
proc ctcp:stat {nick uhost hand chan key rest} {
  if {![stat:cheated $nick $hand $chan $rest action]} {
    *ctcp:stat $nick $uhost $hand $chan $key $rest
  }
}

proc stat:cheated {nick hand chan rest type} {
  global acset
  stat:resetstuff
  set hand [nick2suser $nick $chan]
  if {$hand == "*"} {
    stat:monologue $hand $chan
    return 1
  }
  if {$acset(repeatpunish) != 0} {
    if {[stat:repeated $hand $chan $rest]} {
      if {$acset(repeatpunish) == 2} {
      	putloglev $acset(loglev) $chan "StatCheat: $hand is repeating. Subtracting words instead of adding."
	incrstats $hand $chan words -[llength $rest]
        incrstats $hand $chan lines -1
	if {$type == "action"} {
	  incrstats $hand $chan actions -1
	}
      } else {
      	putloglev $acset(loglev) $chan "StatCheat: $hand is repeating. Ignoring..."
      }
      return 1
    }
  }
  if {$acset(monologuepunish) != 0} {
    if {[stat:monologue $hand $chan]} {
      if {$acset(monologuepunish) == 2} {
      	putloglev $acset(loglev) $chan "StatCheat: $hand is making a monologue since more than $acset(monologue) lines. Subtracting words instead of adding."
	incrstats $hand $chan words -[llength $rest]
        incrstats $hand $chan lines -1
	if {$type == "action"} {
	  incrstats $hand $chan actions -1
	}
      } else {
      	putloglev $acset(loglev) $chan "StatCheat: $hand is holding a monologue. Ignoring..."
      }
      return 1
    }
  }
  if {[string match "*\003*" $rest]} {
    if {$acset(colorpunish) == 2} {
      putloglev $acset(loglev) $chan "StatCheat: $hand is using colors. That's probably a script. Subtracting words instead of adding."
      incrstats $hand $chan words -[llength $rest]
      incrstats $hand $chan lines -1
      if {$type == "action"} {
	incrstats $hand $chan actions -1
      }
    } else {
      putloglev $acset(loglev) $chan "StatCheat: $hand is using colors. That's probably a script. Ignoring."
    }
    return 1
  }
  return 0
}

proc stat:repeated {hand chan text} {
  global statrepeat

  set idx ""
  lappend idx $hand
  lappend idx $chan
  if {![info exists statrepeat($idx)]} {
    set statrepeat($idx) ""
  }
  if {[lsearch -exact $statrepeat($idx) $text] > -1} {
    return 1
  } else {
    lappend statrepeat($idx) $text
    return 0
  }
}

proc stat:monologue {hand chan} {
  global statmonologue acset

  if {![info exists statmonologue($chan)]} {
    set statmonologue($chan) "foo 0"
  }
  set lasthand [lindex $statmonologue($chan) 0]
  set times [lindex $statmonologue($chan) 1]
  if {$lasthand != $hand} {
    set newdat ""
    lappend newdat $hand
    lappend newdat 1
    set statmonologue($chan) $newdat
    return 0
  } else {
    incr times
    set newdat ""
    lappend newdat $hand
    lappend newdat $times
    set statmonologue($chan) $newdat
  }
  if {$times >= $acset(monologue)} {
    return 1
  } else {
    return 0
  }
}

proc stat:resetstuff {} {
  global statrepeat statreset acset

  if {![info exists statreset]} {
    set statreset 0
  }
  if {[expr [unixtime] - $statreset] < $acset(tracktime)} {
    return
  }
  set statreset [unixtime]
  foreach item [array names statrepeat] {
    unset statrepeat($item)
  }
  catch "unbind ctcp - ACTION *ctcp:stat"
  catch "unbind pubm - * *pubm:stat"
}

putlog "AntiCheat v1.2.0 loaded."