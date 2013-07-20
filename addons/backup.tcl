# Simple script to make a daily backup of certain files

set files-to-backup "statsmod.dat"
set backup-dir "backup/"

bind time - "00 00 * * *" time:backup
proc time:backup {a b c d e} {
  global files-to-backup backup-dir

  putlog "Backing up..."
  if {[string index ${backup-dir} [expr [string length ${backup-dir}] - 1]] != "/"} {
    append ${backup-dir} "/"
  }
  foreach datei ${files-to-backup} {
    file copy $datei ${backup-dir}${datei}.[strftime "%Y%m%d" [unixtime]]
  }
}