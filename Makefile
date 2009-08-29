################################################################################
# You shouldn't need to edit this file. Edit site.conf instead.
################################################################################

include makefile.conf

BASE = insts/base
DIRS = include genlib src insts utils apps docs snd

all:	install_dirs
	@echo "making all ..."
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) all ); \
	done
	@echo "done"

# Individual make targets.  Note that these are not necessarily equivalent
# to the subdir with the same name.

include::
	@echo "making and installing include..."
	@cd include; $(MAKE) $(MFLAGS) install
	@echo "done."; echo ""

src::
	@echo "making all in src..."
	@cd src; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

genlib::
	@echo "making and installing genlib..."
	@cd genlib; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

utils::
	@echo "making utils..."
	@cd utils; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

docs::
	@echo "making docs ..."
	@cd docs; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

snd::
	@echo "making snd ..."
	@cd snd; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

insts::
	@echo "making insts ..."
	@cd insts; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

base::
	@echo "making base insts ..."
	@cd insts; $(MAKE) $(MFLAGS) base
	@echo "done."; echo ""

imbed::
	@echo "making imbed apps ..."
	@cd apps; $(MAKE) $(MFLAGS) imbed
	@echo "done."; echo ""

apps::
	@echo "making apps ..."
	@cd apps; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

dsos:: insts

standalone::
	@echo "making standalone ..."
	@cd insts; $(MAKE) $(MFLAGS) standalone
	@echo "done."; echo ""

#############################################################  make install  ###

install:	install_dirs
	@echo "beginning install..."
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) install ); \
	done
	@echo "install done."; echo ""

base_install:
	@echo "beginning base_install..."
	@cd $(BASE); $(MAKE) $(MFLAGS) install;
	@echo "base_install done."; echo ""

dso_install: 
	@echo "beginning dso_install..."
	cd insts; $(MAKE) $(MFLAGS) dso_install;
	@echo "dso_install done."; echo ""

standalone_install::
	@echo "beginning standalone_install..."
	@cd insts; $(MAKE) $(MFLAGS) standalone_install;
	@echo "standalone_install done."; echo ""

install_dirs::
	@if test ! -d $(LIBDIR); then mkdir $(LIBDIR); fi;
	@if test ! -d $(LIBDESTDIR); then mkdir $(LIBDESTDIR); fi;

###########################################################  make uninstall  ###

uninstall::
	@echo "beginning uninstall..."
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) uninstall ); \
	done
	@echo "uninstall done."; echo ""

dso_uninstall: 
	@echo "beginning dso_uninstall..."
	@cd insts; $(MAKE) $(MFLAGS) dso_uninstall; 
	@echo "dso_uninstall done."; echo ""

standalone_uninstall::
	@echo "beginning standalone_uninstall..."
	@cd insts; $(MAKE) $(MFLAGS) standalone_uninstall; 
	@echo "standalone_uninstall done."; echo ""

###############################################################  make depend  ##

depend::
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making depend in $$DIR..."; \
	  $(RM) depend; \
	  $(MAKE) $(MFLAGS) depend ); \
	done

###############################################################  make clean  ###

clean::
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
	  $(MAKE) $(MFLAGS) clean ); \
	done
	( cd pkg/osx; echo "making clean in pkg/osx..." )

cleanall::
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making cleanall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) cleanall ); \
	done
	( cd pkg/osx; echo "making clean in pkg/osx..." )

# Make it clean for distribution or for moving to another system
distclean: cleanall cleanac
	@cd insts; $(MAKE) $(MFLAGS) distclean; 
	@cd apps; $(MAKE) $(MFLAGS) distclean; 
	@find . -name depend -exec rm -f '{}' ';'
	@$(RM) config.h
	@$(RM) defs.conf
	@$(RM) makefile.conf

####################################################### for maintainers only ###

configure: configure.ac
	touch defs.conf
	aclocal
	autoconf

cleanac:
	@$(RM) -r autom4te.cache config.log config.status config.cache

