################################################################################
# You shouldn't need to edit this file. Edit makefile.conf instead.
################################################################################

include makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
  SNDLIB = sndlib
  SNDLIB_DIR = $(SNDLIB)-5.5
else
  SNDLIB = 
endif

MAKEFILE_CONF = $(CMIXDIR)/makefile.conf

DIRS = $(SNDLIB) H rtstuff/heap rtstuff Minc sys lib head cmd utils \
       $(PACKAGE_DIRS) $(INST_DIRS)

all: $(DIRS)

install:
	@echo "beginning install..."
	@cd cmd; $(MAKE) install;
	@cd head; $(MAKE) install;
	@cd utils; $(MAKE) install;
	@cd insts.base; $(MAKE) install;
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) install ); \
	done
	@for DIR in $(PACKAGE_DIRS); \
	do \
	  ( cd $$DIR; $(MAKE) install ); \
	done
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

sndlib::
	@echo "making sndlib..."
	@ln -sf $(SNDLIB_DIR) $(SNDLIB);
	@cd sndlib; $(MAKE) all
	@echo "done.";echo""

lib::
	@echo "making lib..."
	@cd lib; $(MAKE) all
	@echo "done.";echo""

rtstuff::
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

dsos::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) all; echo "done.";echo"" ); \
	done

dso_clean::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making dso_clean $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) dso_clean; echo "done.";echo"" ); \
	done

standalone::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) standalone; echo "done.";echo"" ); \
	done

standalone_clean::
	@for DIR in $(INST_DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone_clean $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) standalone_clean; echo "done.";echo"" ); \
	done

packages::
	@for DIR in $(PACKAGE_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) all; echo "done.";echo"" ); \
	done

clean::
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
	  $(MAKE) clean ); \
	done
