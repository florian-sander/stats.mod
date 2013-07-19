# Makefile for src/mod/stats.mod/

doofus:
	@echo ""
	@echo "Let's try this from the right directory..."
	@echo ""
	@cd ../../../; make

clean:
	@rm -f *.o *.so *~

static: ../stats.o

modules: ../../../stats.so

../stats.o: ../module.h ../modvals.h ../../eggdrop.h datahandling.c \
 stats.c sensors.c dcccmds.c misc.c pubcmds.c msgcmds.c webfiles.c \
 user.c livestats.c userrec.c tclstats.c slang.c slang.h stats.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c stats.c
	rm -f ../stats.o
	mv stats.o ../

../../../stats.so: ../stats.o
	$(LD) -o ../../../stats.so ../stats.o
	$(STRIP) ../../../stats.so

#safety hash
