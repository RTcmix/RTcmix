# for CMIXDIR...
include ../makefile.conf

EXT_NAME = RT
XS_DIR = $(CMIXDIR)/Perl/$(EXT_NAME)
XS_FILE = $(EXT_NAME).xs
PM_FILE = $(EXT_NAME).pm
ifeq ($(ARCH),MACOSX)
  XT_SUFFIX = .bundle
else
  XT_SUFFIX = .so
endif
EXT_FILE = $(XS_DIR)/blib/arch/auto/$(EXT_NAME)/$(EXT_NAME)$(XT_SUFFIX)

all: $(EXT_FILE)

.PHONY: install clean

$(EXT_FILE): $(XS_DIR)/$(XS_FILE) $(XS_DIR)/$(PM_FILE)
	( cd $(XS_DIR); perl Makefile.PL; $(MAKE) )

install:
	@echo "making install..."	
	@$(INSTALL) $(XS_DIR)/$(PM_FILE) $(LIBDESTDIR)
	@$(INSTALL) $(EXT_FILE) $(LIBDESTDIR)

clean:
	@if (test -f $(XS_DIR)/Makefile) then \
		(cd $(XS_DIR); $(MAKE) clean; $(RM) Makefile.old) \
	fi
	@$(RM) $(LIBDESTDIR)/$(PM_FILE) $(LIBDESTDIR)/$(EXT_NAME)$(XT_SUFFIX)

