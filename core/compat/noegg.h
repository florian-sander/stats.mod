// #include <assert.h>
// #include <stdio.h>

// #include "compat/snprintf.h"

static void putlog(int, char *, char *s, ...);
char *newsplit(char **rest);

#define strcasecmp(s1, s2)	strcmpi(s1, s2)
#define strncasecmp(s1, s2, n)	strnicmp(s1, s2, n)

#define rfc_casecmp(s1, s2)	strcmpi(s1, s2)

#define random rand

#define LOG_MISC	0
#define LOG_DEBUG	1

#define debug0(s)		putlog(LOG_DEBUG, "*", s)
#define debug1(s1, s2)		putlog(LOG_DEBUG, "*", s1, s2)
#define debug2(s1, s2, s3)	putlog(LOG_DEBUG, "*", s1, s2, s3)
#define debug3(s1, s2, s3, s4)	putlog(LOG_DEBUG, "*", s1, s2, s3, s4)

#define now time(NULL)

#define chmod(x1, x2)	assert(1)

#define botnetnick "Stats.dll"

#define botname "Stats.dll"

#define movefile(f1, f2) rename(f1, f2)

static void fatal(char *s, int x);

// #define findchan_by_dname(x) NULL