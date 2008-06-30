
include config

.PHONY: all libs modules install clean help distclean docs release

all: libs modules

libs:
	cd src && $(MAKE) $@

modules: libs
	cd tek/lib && $(MAKE) $@
	cd tek/ui && $(MAKE) $@

install:
	cd tek && $(MAKE) $@
	cd tek/lib && $(MAKE) $@
	cd tek/ui && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
	cd tek/lib && $(MAKE) $@
	cd tek/ui && $(MAKE) $@

help: default-help
	@echo "Extra build targets for this Makefile:"
	@echo "-------------------------------------------------------------------------------"
	@echo "docs .................... (re-)generate documentation"
	@echo "distclean ............... remove all temporary files and directories"
	@echo "==============================================================================="

distclean: clean
	-$(RMDIR) lib
	-find src tek -type d -name build | xargs $(RMDIR)

docs:
	bin/gendoc.lua README -i 32 -n tekUI > doc/index.html
	bin/gendoc.lua COPYRIGHT -i 32 -n tekUI Copyright > doc/copyright.html
	bin/gendoc.lua TODO -i 32 -n tekUI TODO > doc/todo.html
	bin/gendoc.lua tek/ -n tekUI Reference manual > doc/manual.html

kdiff:
	-(a=$$(mktemp -du) && hg clone $$PWD $$a && kdiff3 $$a $$PWD; rm -rf $$a)
