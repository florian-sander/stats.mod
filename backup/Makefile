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

../stats.o: ../module.h ../modvals.h ../../eggdrop.h datahandling.c \
 stats.c sensors.c dcccmds.c misc.c pubcmds.c msgcmds.c \
 user.c userrec.c tclstats.c slang.c slang.h mini_httpd.c \
 templates.c http_processing.c slang_text.c slang_chanlang.c \
 slang_multitext.c slang_ids.c slang_types.c slang_facts.c \
 slang_facts_places.c slang_duration.c templates_commands.c \
 templates_skin.c templates_stats_commands.c templates_content.c \
 slang_stats_commands.c dynamic_mem_debug.c stats.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -DMAKING_MODS -c stats.c
	rm -f ../stats.o
	mv stats.o ../

../../../stats.$(MOD_EXT): ../stats.o
	$(LD) -o ../../../stats.$(MOD_EXT) ../stats.o $(XLIBS)

#safety hash
