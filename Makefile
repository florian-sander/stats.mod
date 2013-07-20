# Makefile for src/mod/stats.mod/

doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../; make

clean:
	@rm -f *.o *.$(MOD_EXT) *~

static: ../stats.o

modules: ../../../stats.$(MOD_EXT)

../stats.o: ../module.h ../modvals.h ../../eggdrop.h \
 egg_chancontrol.c pubcmds.c \
 core/sensors.c core/datahandling.c core/global_vars.c core/schan_members.c \
 stats.c tclstats.c core/misc.c core/dynamic_mem_debug.c core/userrec.c stats.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c stats.c
	rm -f ../stats.o
	mv stats.o ../

../../../stats.$(MOD_EXT): ../stats.o
	$(LD) -o ../../../stats.$(MOD_EXT) ../stats.o $(XLIBS)

core: core.o

core.o: core/core.c
	gcc -pipe -g -O2 -I. -g3 -DNO_EGG core/core.c

#safety hash
