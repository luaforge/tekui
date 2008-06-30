
BASEDIR ?= ../..
include $(BASEDIR)/config

###############################################################################

LIBS = \
	$(LIBDIR)/libexec.a

$(OBJDIR)/exec_mod.lo: \
	exec_mod.c exec_mod.h $(INCDIR)/tek/mod/exec.h
	$(CC) $(LIBCFLAGS) -o $@ -c exec_mod.c
$(OBJDIR)/exec_api.lo: \
	exec_api.c exec_mod.h $(INCDIR)/tek/mod/exec.h
	$(CC) $(LIBCFLAGS) -o $@ -c exec_api.c
$(OBJDIR)/exec_doexec.lo: \
	exec_doexec.c exec_mod.h $(INCDIR)/tek/mod/exec.h
	$(CC) $(LIBCFLAGS) -o $@ -c exec_doexec.c
$(OBJDIR)/exec_memory.lo: \
	exec_memory.c exec_mod.h $(INCDIR)/tek/mod/exec.h
	$(CC) $(LIBCFLAGS) -o $@ -c exec_memory.c

$(LIBDIR)/libexec.a: \
	$(OBJDIR)/exec_mod.lo $(OBJDIR)/exec_api.lo $(OBJDIR)/exec_doexec.lo \
	$(OBJDIR)/exec_memory.lo
	$(AR) $@ $?

###############################################################################

libs: $(LIBDIR) $(OBJDIR) $(LIBS)

clean: FORCE
	-$(RM) $(MODS) $(TOOLS) $(LIBS)
	-$(RMDIR) $(OBJDIR)
