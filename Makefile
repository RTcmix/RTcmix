################################################################################
# You shouldn't need to edit this file. Edit makefile.conf instead.
################################################################################

include makefile.conf

MAKEFILE_CONF = $(CMIXDIR)/makefile.conf

SNDLIB_DIR = sndlib

DIRS = $(SNDLIB_DIR) H rtstuff Minc sys lib head cmd utils 
ifeq ($(PERL_SUPPORT), TRUE)
	DIRS += Perl
endif
ifeq ($(PYTHON_SUPPORT), TRUE)
	DIRS += Python
endif

all: $(DIRS) insts packages

install::
	@echo "beginning install..."
	@cd cmd; $(MAKE) install;
	@cd head; $(MAKE) install;
	@cd utils; $(MAKE) install;
	@if test ! -d $(LIBDESTDIR); then mkdir $(LIBDESTDIR); fi;
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) install ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; $(MAKE) install ); \
	done
endif
ifeq ($(PERL_SUPPORT), TRUE)
	@cd Perl; $(MAKE) install;
endif
	@echo "install done."; echo ""

H::
	@echo "making H..."
	@cd H; $(MAKE) all
	@echo "done."; echo""

heap::
	@echo "making heap ..."
	@cd rtstuff/heap; $(MAKE) all
	@echo "done."; echo""

Minc::
	@echo "making Minc..."
	@cd Minc; $(MAKE) all
	@echo "done.";echo""

sys::
	@echo "making sys..."
	@cd sys; $(MAKE) all
	@echo "done.";echo""

$(SNDLIB_DIR)::
	@echo "making sndlib..."
	@cd $(SNDLIB_DIR); $(MAKE) all
	@echo "done.";echo""

lib::
ifeq ($(ARCH),SGI)   # do only for SGI, which needs this for so_locations file
	@if test ! -d $(LIBDESTDIR); then mkdir $(LIBDESTDIR); fi;
endif
	@echo "making lib..."
	@cd lib; $(MAKE) all
	@echo "done.";echo""

rtstuff::
	@echo "making rtstuff/heap..."
	@cd rtstuff/heap; $(MAKE) all
	@echo "done.";echo""
	@echo "making rtstuff..."
	@cd rtstuff; $(MAKE) all
	@echo "done.";echo""

head::
	@echo "making head..."
	@cd head; $(MAKE) all
	@echo "done.";echo""

cmd::
	@echo "making cmd..."
	@cd cmd; $(MAKE) all
	@echo "done.";echo""

utils::
	@echo "making utils..."
	@cd utils; $(MAKE) all
	@echo "done.";echo""

Perl::
	@echo "making Perl..."
	@cd Perl; $(MAKE) all
	@echo "done."; echo ""

dsos::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) all; echo "done.";echo"" ); \
	done

dso_install: 
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making dso_clean $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) install; echo "done.";echo"" ); \
	done

dso_clean::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making dso_clean $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) dso_clean; echo "done.";echo"" ); \
	done

standalone::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) standalone; echo "done.";echo"" ); \
	done

standalone_install::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) standalone_install; echo "done.";echo"" ); \
	done

standalone_clean::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone_clean $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf \
	   $(MAKE) standalone_clean; echo "done.";echo"" ); \
	done

insts::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   echo "include $(MAKEFILE_CONF)" > package.conf; \
	   $(MAKE) all; echo "done.";echo"" ); \
	done

packages::
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; echo "making $$DIR..."; \
		echo "include $(MAKEFILE_CONF)" > package.conf; \
		$(MAKE) all; echo "done.";echo"" ); \
	done
endif

clean::
	@for DIR in $(DIRS) $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
	  $(MAKE) clean ); \
	done
ifneq ($(strip $(PACKAGE_DIRS)),)    # do only if PACKAGE_DIRS is nonempty
	@for DIR in $(PACKAGE_DIRS); \
	do \
		( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) clean ); \
	done
endif

