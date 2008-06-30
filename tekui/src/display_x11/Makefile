
BASEDIR ?= ../..
include $(BASEDIR)/config

EXTRADEFS += $(X11_DEFS)

###############################################################################

LIBS = \
	$(LIBDIR)/libdisplay_x11.a

$(OBJDIR)/display_x11_mod.lo: \
	display_x11_mod.c display_x11_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_x11_mod.c
$(OBJDIR)/display_x11_api.lo: \
	display_x11_api.c display_x11_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_x11_api.c
$(OBJDIR)/display_x11_inst.lo: \
	display_x11_inst.c display_x11_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_x11_inst.c
$(OBJDIR)/display_x11_font.lo: \
	display_x11_font.c display_x11_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_x11_font.c
$(OBJDIR)/display_x11_utf8.lo: \
	display_x11_utf8.c display_x11_mod.h
	$(CC) $(LIBCFLAGS) -o $@ -c display_x11_utf8.c

$(LIBDIR)/libdisplay_x11.a: \
	$(OBJDIR)/display_x11_mod.lo $(OBJDIR)/display_x11_api.lo \
	$(OBJDIR)/display_x11_inst.lo $(OBJDIR)/display_x11_font.lo \
	$(OBJDIR)/display_x11_utf8.lo
	$(AR) $@ $?

###############################################################################

libs: $(LIBDIR) $(OBJDIR) $(LIBS)

clean: FORCE
	-$(RM) $(MODS) $(TOOLS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
