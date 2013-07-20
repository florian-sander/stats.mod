# Generated automatically from Makefile.in by configure.
# Makefile for src/compat/
# $Id: Makefile,v 1.1 2005/04/08 13:48:26 Administrator Exp $

SHELL = /bin/sh
top_srcdir = ../..
srcdir = .


INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}

CC = gcc -pipe -mwin32
LD = gcc -pipe -mwin32
STRIP = strip
CFLAGS = -g -O2 -I../.. -I$(top_srcdir) -I$(top_srcdir)/src -DHAVE_CONFIG_H $(CFLGS)
CPPFLAGS = 

OBJS = inet_aton.o snprintf.o memset.o memcpy.o strcasecmp.o strftime.o

doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../.. && $(MAKE)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM $(srcdir)/*.c > .depend

clean:
	@rm -f .depend *.o *~

compat: $(OBJS)

.SUFFIXES:
.SUFFIXES: .c .o .h

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

#safety hash
gnu_strftime.o: ./gnu_strftime.c
inet_aton.o: ./inet_aton.c ../../src/main.h ../../config.h \
 ../../src/lang.h ../../src/eggdrop.h ../../src/flags.h \
 ../../src/proto.h ../../lush.h ../../src/misc_file.h ../../src/cmdt.h \
 ../../src/tclegg.h ../../src/tclhash.h ../../src/chan.h \
 ../../src/users.h ../../src/compat/compat.h \
 ../../src/compat/inet_aton.h ../../src/compat/snprintf.h \
 ../../src/compat/memset.h ../../src/compat/memcpy.h \
 ../../src/compat/strcasecmp.h ../../src/compat/strftime.h inet_aton.h
memcpy.o: ./memcpy.c ../../src/main.h ../../config.h ../../src/lang.h \
 ../../src/eggdrop.h ../../src/flags.h ../../src/proto.h ../../lush.h \
 ../../src/misc_file.h ../../src/cmdt.h ../../src/tclegg.h \
 ../../src/tclhash.h ../../src/chan.h ../../src/users.h \
 ../../src/compat/compat.h ../../src/compat/inet_aton.h \
 ../../src/compat/snprintf.h ../../src/compat/memset.h \
 ../../src/compat/memcpy.h ../../src/compat/strcasecmp.h \
 ../../src/compat/strftime.h memcpy.h
memset.o: ./memset.c ../../src/main.h ../../config.h ../../src/lang.h \
 ../../src/eggdrop.h ../../src/flags.h ../../src/proto.h ../../lush.h \
 ../../src/misc_file.h ../../src/cmdt.h ../../src/tclegg.h \
 ../../src/tclhash.h ../../src/chan.h ../../src/users.h \
 ../../src/compat/compat.h ../../src/compat/inet_aton.h \
 ../../src/compat/snprintf.h ../../src/compat/memset.h \
 ../../src/compat/memcpy.h ../../src/compat/strcasecmp.h \
 ../../src/compat/strftime.h memset.h
snprintf.o: ./snprintf.c ../../src/main.h ../../config.h \
 ../../src/lang.h ../../src/eggdrop.h ../../src/flags.h \
 ../../src/proto.h ../../lush.h ../../src/misc_file.h ../../src/cmdt.h \
 ../../src/tclegg.h ../../src/tclhash.h ../../src/chan.h \
 ../../src/users.h ../../src/compat/compat.h \
 ../../src/compat/inet_aton.h ../../src/compat/snprintf.h \
 ../../src/compat/memset.h ../../src/compat/memcpy.h \
 ../../src/compat/strcasecmp.h ../../src/compat/strftime.h snprintf.h
strcasecmp.o: ./strcasecmp.c ../../src/main.h ../../config.h \
 ../../src/lang.h ../../src/eggdrop.h ../../src/flags.h \
 ../../src/proto.h ../../lush.h ../../src/misc_file.h ../../src/cmdt.h \
 ../../src/tclegg.h ../../src/tclhash.h ../../src/chan.h \
 ../../src/users.h ../../src/compat/compat.h \
 ../../src/compat/inet_aton.h ../../src/compat/snprintf.h \
 ../../src/compat/memset.h ../../src/compat/memcpy.h \
 ../../src/compat/strcasecmp.h ../../src/compat/strftime.h memcpy.h
strftime.o: ./strftime.c ../../src/main.h ../../config.h \
 ../../src/lang.h ../../src/eggdrop.h ../../src/flags.h \
 ../../src/proto.h ../../lush.h ../../src/misc_file.h ../../src/cmdt.h \
 ../../src/tclegg.h ../../src/tclhash.h ../../src/chan.h \
 ../../src/users.h ../../src/compat/compat.h \
 ../../src/compat/inet_aton.h ../../src/compat/snprintf.h \
 ../../src/compat/memset.h ../../src/compat/memcpy.h \
 ../../src/compat/strcasecmp.h ../../src/compat/strftime.h strftime.h
