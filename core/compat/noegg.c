#ifndef __stdlib_h__
#include <stdlib.h>
#endif

// #include "compat/snprintf.c"

static void nputlog(char *s, ...)
{

}

char *newsplit(char **rest)
{
  register char *o, *r;

  if (!rest)
    return *rest = "";
  o = *rest;
  while (*o == ' ')
    o++;
  r = o;
  while (*o && (*o != ' '))
    o++;
  if (*o)
    *o++ = 0;
  *rest = o;
  return r;
}

static void fatal(char *s, int x)
{
	putlog(LOG_MISC, "*", "%s", s);
	exit(EXIT_FAILURE);
}