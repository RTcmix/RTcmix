include makefile.conf

ifeq ($(USE_SNDLIB),TRUE)
  SNDLIB = sndlib
  SNDLIB_DIR = $(SNDLIB)-5.5
else
  SNDLIB = 
endif

DIRS = H rtstuff/heap rtstuff $(SNDLIB) Minc sys lib head cmd utils insts.base

# Add these to DIRS and all: as needed
#  insts.std insts.dev

all: $(SNDLIB) H heap rtstuff Minc sys lib head cmd utils insts.base

install:
	@echo "making install..."
	@cd cmd; $(MAKE) install;
	@cd head; $(MAKE) install;
	@cd utils; $(MAKE) install;
	@cd insts.base; $(MAKE) install;
	@cd insts.std; $(MAKE) install;
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

insts.base::
	@echo "making insts..."
	@cd insts.base; $(MAKE) all
	@echo "done.";echo""

insts.dev::
	@echo "making insts.dev..."
	@cd insts.dev; $(MAKE) all
	@echo "done.";echo""

insts.std::
	@echo "making insts.std..."
	@cd insts.std; $(MAKE) all
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

