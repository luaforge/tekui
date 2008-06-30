
BASEDIR ?= ../..
include $(BASEDIR)/config

EXTRADEFS += $(LUA_DEFS) -DDISPLAY_DRIVER=\"$(DISPLAY_DRIVER)\"

###############################################################################

MODS = region.so exec.so visual.so
DISPLAYMODS = display/x11.so # display/dfb.so

EXECLIBS = $(LIBDIR)/libhal.a $(LIBDIR)/libexec.a $(LIBDIR)/libtime.a $(LIBDIR)/libtekc.a $(LIBDIR)/libtekdebug.a
VISUALLIBS = $(LIBDIR)/libvisual.a $(LIBDIR)/libtek.a $(LIBDIR)/libtekdebug.a
DISPLAYX11LIBS = $(LIBDIR)/libdisplay_x11.a $(LIBDIR)/libtek.a $(LIBDIR)/libtekdebug.a
DISPLAYDFBLIBS = $(LIBDIR)/libdisplay_dfb.a $(LIBDIR)/libtek.a $(LIBDIR)/libtekdebug.a

region.so: $(OBJDIR)/region.lo
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/region.lo $(PLATFORM_LIBS)

exec.so: $(OBJDIR)/exec_lua.lo $(EXECLIBS)
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/exec_lua.lo -L$(LIBDIR) -lhal -lexec -ltime -ltekc -ltekdebug $(PLATFORM_LIBS)

visual.so: $(OBJDIR)/visual_lua.lo $(OBJDIR)/visual_api.lo $(VISUALLIBS)
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/visual_lua.lo $(OBJDIR)/visual_api.lo -L$(LIBDIR) -lvisual -ltek -ltekdebug

display/x11.so: $(OBJDIR)/x11_lua.lo $(DISPLAYX11LIBS)
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/x11_lua.lo -L$(LIBDIR) -ldisplay_x11 -ltek -ltekdebug $(X11_LIBS)

display/dfb.so: $(OBJDIR)/dfb_lua.lo $(DISPLAYDFBLIBS)
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/dfb_lua.lo -L$(LIBDIR) -ldisplay_dfb -ltek -ltekdebug $(DFB_LIBS)

$(OBJDIR)/region.lo: region.c
	$(CC) $(LIBCFLAGS) -o $@ -c region.c

$(OBJDIR)/exec_lua.lo: exec_lua.c
	$(CC) $(LIBCFLAGS) -o $@ -c exec_lua.c

$(OBJDIR)/visual_lua.lo: visual_lua.c
	$(CC) $(LIBCFLAGS) -o $@ -c visual_lua.c
$(OBJDIR)/visual_api.lo: visual_api.c
	$(CC) $(LIBCFLAGS) -o $@ -c visual_api.c

$(OBJDIR)/x11_lua.lo: display/x11_lua.c
	$(CC) $(LIBCFLAGS) -o $@ -c display/x11_lua.c

$(OBJDIR)/dfb_lua.lo: display/dfb_lua.c
	$(CC) $(LIBCFLAGS) -o $@ -c display/dfb_lua.c

###############################################################################

modules: $(OBJDIR) $(MODS) $(DISPLAYMODS)

install:
	$(INSTALL) -d $(LUA_LIB)/tek/lib/display
	$(INSTALL) -s $(MODS) $(LUA_LIB)/tek/lib
	$(INSTALL) -s $(DISPLAYMODS) $(LUA_LIB)/tek/lib/display
	$(INSTALL) -d $(LUA_SHARE)/tek/lib
	$(INSTALL) debug.lua $(LUA_SHARE)/tek/lib

clean: FORCE
	-$(RM) $(MODS) $(DISPLAYMODS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
