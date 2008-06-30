
BASEDIR ?= ../..
include $(BASEDIR)/config

EXTRADEFS += $(DFB_DEFS) -DFNT_DEFDIR=\"$(LUA_SHARE)/tek/ui/font/\" \
			-DCUR_DEFFILE=\"$(LUA_SHARE)/tek/ui/cursor/cursor-green.png\"

###############################################################################

LIBS = \
	$(LIBDIR)/libdisplay_dfb.a

$(OBJDIR)/display_dfb_mod.lo: \
	display_dfb_mod.c display_dfb_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_dfb_mod.c
$(OBJDIR)/display_dfb_api.lo: \
	display_dfb_api.c display_dfb_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_dfb_api.c
$(OBJDIR)/display_dfb_font.lo: \
	display_dfb_font.c display_dfb_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_dfb_font.c
$(OBJDIR)/display_dfb_utf8.lo: \
	display_dfb_utf8.c display_dfb_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_dfb_utf8.c

$(LIBDIR)/libdisplay_dfb.a: \
	$(OBJDIR)/display_dfb_mod.lo $(OBJDIR)/display_dfb_api.lo \
	$(OBJDIR)/display_dfb_font.lo $(OBJDIR)/display_dfb_utf8.lo
	$(AR) $@ $?

###############################################################################

libs: $(LIBDIR) $(OBJDIR) $(LIBS)

clean: FORCE
	-$(RM) $(MODS) $(TOOLS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
