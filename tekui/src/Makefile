
BASEDIR ?= ..
include $(BASEDIR)/config

all: libs
clean libs:
	cd teklib && $(MAKE) $@
	cd hal && $(MAKE) $@
	cd exec && $(MAKE) $@
	cd time && $(MAKE) $@
	cd visual && $(MAKE) $@
	cd display_x11 && $(MAKE) $@
#	cd display_dfb && $(MAKE) $@
