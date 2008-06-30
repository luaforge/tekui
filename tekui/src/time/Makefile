
BASEDIR ?= ../..
include $(BASEDIR)/config

###############################################################################

LIBS = \
	$(LIBDIR)/libtime.a

$(OBJDIR)/time_mod.lo: \
	time_mod.c $(INCDIR)/tek/mod/time.h
	$(CC) $(LIBCFLAGS) -o $@ -c time_mod.c

$(LIBDIR)/libtime.a: \
	$(OBJDIR)/time_mod.lo
	$(AR) $@ $?

###############################################################################

libs: $(LIBDIR) $(OBJDIR) $(LIBS)

clean: FORCE
	-$(RM) $(MODS) $(TOOLS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
