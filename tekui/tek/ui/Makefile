
BASEDIR ?= ../..
include $(BASEDIR)/config

EXTRADEFS += $(LUA_DEFS)

LUACLASSES = \
	class/application.lua class/area.lua class/border.lua class/canvas.lua \
	class/checkmark.lua class/dirlist.lua class/display.lua \
	class/drawable.lua class/element.lua class/family.lua class/floattext.lua \
	class/frame.lua class/gadget.lua class/gauge.lua class/group.lua \
	class/handle.lua class/image.lua class/listgadget.lua class/listview.lua \
	class/menuitem.lua class/numeric.lua class/popitem.lua class/poplist.lua \
	class/popupwindow.lua class/radiobutton.lua class/scrollbar.lua \
	class/scrollgroup.lua class/slider.lua class/spacer.lua \
	class/pagegroup.lua class/text.lua class/textinput.lua class/theme.lua \
	class/vectorimage.lua class/window.lua
LUABORDERS = \
	border/blank.lua border/button.lua border/group.lua \
	border/recess.lua border/socket.lua border/tab.lua \
	border/cursor.lua

###############################################################################

MODS = layout/default.so

layout/default.so: $(OBJDIR)/default.lo
	$(CC) $(MODCFLAGS) -o $@ $(OBJDIR)/default.lo

$(OBJDIR)/default.lo: layout/default.c
	$(CC) $(LIBCFLAGS) -o $@ -c layout/default.c

###############################################################################

FONTS = font/Vera.ttf font/VeraMono.ttf
CURSORS = cursor/cursor-green.png

###############################################################################

modules: $(OBJDIR) $(MODS)

install:
	$(INSTALL) -d $(LUA_LIB)/tek/ui/layout
	$(INSTALL) -d $(LUA_SHARE)/tek/ui/layout
	$(INSTALL) -d $(LUA_SHARE)/tek/ui/border
	$(INSTALL) -d $(LUA_SHARE)/tek/ui/class
	$(INSTALL) -d $(LUA_SHARE)/tek/ui/font
	$(INSTALL) -d $(LUA_SHARE)/tek/ui/cursor
	$(INSTALL) -s layout/default.so $(LUA_LIB)/tek/ui/layout
	$(INSTALL) $(LUACLASSES) $(LUA_SHARE)/tek/ui/class
	$(INSTALL) $(LUABORDERS) $(LUA_SHARE)/tek/ui/border
	$(INSTALL) $(FONTS) $(LUA_SHARE)/tek/ui/font
	$(INSTALL) $(CURSORS) $(LUA_SHARE)/tek/ui/cursor

clean: FORCE
	-$(RM) $(MODS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
