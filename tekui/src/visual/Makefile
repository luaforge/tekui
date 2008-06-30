
BASEDIR ?= ../..
include $(BASEDIR)/config

###############################################################################

LIBS = \
	$(LIBDIR)/libvisual.a

$(OBJDIR)/visual_mod.lo: \
	visual_mod.c $(INCDIR)/tek/mod/visual.h
	$(CC) $(LIBCFLAGS) -o $@ -c visual_mod.c
$(OBJDIR)/visual_api.lo: \
	visual_api.c $(INCDIR)/tek/mod/visual.h
	$(CC) $(LIBCFLAGS) -o $@ -c visual_api.c
$(OBJDIR)/visual_hash.lo: \
	visual_hash.c $(INCDIR)/tek/mod/visual.h
	$(CC) $(LIBCFLAGS) -o $@ -c visual_hash.c

$(LIBDIR)/libvisual.a: \
	$(OBJDIR)/visual_mod.lo $(OBJDIR)/visual_api.lo $(OBJDIR)/visual_hash.lo
	$(AR) $@ $?

###############################################################################

libs: $(LIBDIR) $(OBJDIR) $(LIBS)

clean: FORCE
	-$(RM) $(MODS) $(TOOLS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
