################################################################################
# You shouldn't need to edit this file. Edit makefile.conf instead.
################################################################################

include makefile.conf

MAKEFILE_CONF = $(CMIXDIR)/makefile.conf

SNDLIB_DIR = sndlib

DIRS = $(SNDLIB_DIR) H rtstuff Minc sys lib head cmd utils docs
ifeq ($(PERL_SUPPORT), TRUE)
	DIRS += Perl
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	DIRS += Python
endif

#################################################################  make all  ###

all: $(DIRS) insts packages

$(SNDLIB_DIR)::
	@echo "making sndlib..."
	@cd $(SNDLIB_DIR); $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

H::
	@echo "making H..."
	@cd H; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

heap::
	@echo "making rtstuff/heap ..."
	@cd rtstuff/heap; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

rtstuff::
	@echo "making rtstuff/heap..."
	@cd rtstuff/heap; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""
	@echo "making rtstuff..."
	@cd rtstuff; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

Minc::
	@echo "making Minc..."
	@cd Minc; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

sys::
	@echo "making sys..."
	@cd sys; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

lib::
ifeq ($(ARCH),SGI)   # do only for SGI, which needs this for so_locations file
	@if test ! -d $(LIBDESTDIR); then mkdir $(LIBDESTDIR); fi;
endif
	@echo "making lib..."
	@cd lib; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

head::
	@echo "making head..."
	@cd head; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

cmd::
	@echo "making cmd..."
	@cd cmd; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

utils::
	@echo "making utils..."
	@cd utils; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

docs::
	@echo "making docs ..."
	@cd docs/pod; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

Perl::
	@echo "making Perl..."
	@cd Perl; $(MAKE) $(MFLAGS) all
	@echo "done."; echo ""

insts::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   echo "include $(MAKEFILE_CONF)" > package.conf; \
	   $(MAKE) $(MFLAGS) all; echo "done."; echo "" ); \
	done

packages::
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; echo "making $$DIR..."; \
		echo "include $(MAKEFILE_CONF)" > package.conf; \
		$(MAKE) $(MFLAGS) all; echo "done."; echo "" ); \
	done
endif

dsos::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) $(MFLAGS) all; echo "done."; echo "" ); \
	done

standalone::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone $$DIR..."; \
	   echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) $(MFLAGS) standalone; echo "done."; echo "" ); \
	done

#############################################################  make install  ###

install::
	@echo "beginning install..."
	@cd head; $(MAKE) $(MFLAGS) install;
	@cd cmd; $(MAKE) $(MFLAGS) install;
	@cd utils; $(MAKE) $(MFLAGS) install;
	@cd docs/pod; $(MAKE) $(MFLAGS) install
ifeq ($(PERL_SUPPORT), TRUE)
	@cd Perl; $(MAKE) $(MFLAGS) install;
endif
	@if test ! -d $(LIBDESTDIR); then mkdir $(LIBDESTDIR); fi;
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) install ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) install ); \
	done
endif
	@echo "install done."; echo ""

dso_install: 
	@echo "beginning dso_install..."
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) dso_install ); \
	done
	@echo "dso_install done."; echo ""

standalone_install::
	@echo "beginning standalone_install..."
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) standalone_install ); \
	done
	@echo "standalone_install done."; echo ""

###########################################################  make uninstall  ###

uninstall::
	@echo "beginning uninstall..."
	@cd head; $(MAKE) $(MFLAGS) uninstall;
	@cd cmd; $(MAKE) $(MFLAGS) uninstall;
	@cd utils; $(MAKE) $(MFLAGS) uninstall;
	@cd docs/pod; $(MAKE) $(MFLAGS) uninstall
ifeq ($(PERL_SUPPORT), TRUE)
	@cd Perl; $(MAKE) $(MFLAGS) uninstall;
endif
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) uninstall ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; $(MAKE) $(MFLAGS) uninstall ); \
	done
endif
	@echo "uninstall done."; echo ""

dso_uninstall: 
	@echo "beginning dso_uninstall..."
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) dso_uninstall ); \
	done
	@echo "dso_uninstall done."; echo ""

standalone_uninstall::
	@echo "beginning standalone_uninstall..."
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) $(MFLAGS) standalone_uninstall ); \
	done
	@echo "standalone_uninstall done."; echo ""

###############################################################  make depend  ##
depend::
	@for DIR in Minc sys; \
	do \
	  ( cd $$DIR; echo "making depend in $$DIR..."; \
	  $(RM) depend; \
	  $(MAKE) $(MFLAGS) depend ); \
	done
###############################################################  make clean  ###

# NB: leave docs/pod alone, so we don't have to rebuild docs all the time
clean::
	@for DIR in $(DIRS) $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
	  $(MAKE) $(MFLAGS) clean ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean ); \
	done
endif

cleanall::
	@echo "making cleanall in docs"
	@cd docs/pod; $(MAKE) $(MFLAGS) clean uninstall
	@for DIR in $(DIRS) $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making cleanall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) cleanall ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) cleanall ); \
	done
endif

# Make it clean for distribution or for moving to another system
distclean: cleanall
	$(RM) Minc/depend
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(RM) package.conf );  \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
	  ( cd $$DIR; $(RM) package.conf );  \
	done
endif

