# for CMIXDIR...
include ../makefile.conf

EXT_NAME = RT
XS_DIR = $(CMIXDIR)/Perl/$(EXT_NAME)
XS_FILE = $(EXT_NAME).xs
PM_FILE = $(EXT_NAME).pm
FUNCLIST = funclist
FUNCEXCEPT = funcexcept
ifeq ($(ARCH),MACOSX)
  XT_SUFFIX = .bundle
  FIND_FLAGS = -follow
else
  XT_SUFFIX = .so
  FIND_FLAGS = -follow
endif

all: RT_extension

# This dependency prevents making unless the funcexcept file has
# changed.  Not really what we want, but it's annoying to have
# to remake this stuff every time you make at the top level...
$(FUNCLIST): $(FUNCEXCEPT)
	perl -w make_funclist.pl $(CMIXDIR) $(FIND_FLAGS)

$(XS_DIR)/$(XS_FILE): $(FUNCLIST)
	perl -w make_rtxs.pl $(CMIXDIR) $(EXT_NAME) > $(XS_DIR)/$(XS_FILE)

$(XS_DIR)/$(PM_FILE): $(FUNCLIST)
	perl -w make_pmfile.pl $(EXT_NAME) > $(XS_DIR)/$(PM_FILE)

.PHONY: RT_extension install clean

RT_extension: $(XS_DIR)/$(XS_FILE) $(XS_DIR)/$(PM_FILE)
	( cd $(XS_DIR); perl Makefile.PL; $(MAKE) )

install:
	@$(INSTALL) $(XS_DIR)/$(PM_FILE) $(LIBDESTDIR)
	@$(INSTALL) $(XS_DIR)/blib/arch/auto/$(EXT_NAME)/$(EXT_NAME)$(XT_SUFFIX) \
               $(LIBDESTDIR)

clean:
	@if (test -f $(XS_DIR)/Makefile) then \
		(cd $(XS_DIR); $(MAKE) clean; $(RM) $(XS_FILE) $(PM_FILE) Makefile.old) \
	fi
	@$(RM) $(FUNCLIST)
	@$(RM) $(LIBDESTDIR)/$(PM_FILE) $(LIBDESTDIR)/$(EXT_NAME)$(XT_SUFFIX)

