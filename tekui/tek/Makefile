
BASEDIR ?= ..
include $(BASEDIR)/config

###############################################################################

LUACLASSES = \
	class/object.lua class/list.lua class/utf8string.lua

install:
	$(INSTALL) -d $(LUA_SHARE)/tek/class
	$(INSTALL) class.lua ui.lua $(LUA_SHARE)/tek
	$(INSTALL) $(LUACLASSES) $(LUA_SHARE)/tek/class
