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
       insts.base $(PACKAGE_DIRS)

all: $(SNDLIB) H heap rtstuff Minc sys lib head cmd utils \
       insts.base $(PACKAGE_DIRS)

install:
	@echo "beginning install..."
	@cd cmd; $(MAKE) install;
	@cd head; $(MAKE) install;
	@cd utils; $(MAKE) install;
	@cd insts.base; $(MAKE) install;
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

clean:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
	  $(MAKE) clean ); \
	done

# JG: this might be a better idea for packages.
packages::
	@for DIR in $(PACKAGE_DIRS); \
	do \
	  ( cd $$DIR; echo "making $$DIR..."; \
	   @echo "include $(MAKEFILE_CONF)" > package.conf 	
	   $(MAKE) all; echo "done.";echo"" ); \
	done

