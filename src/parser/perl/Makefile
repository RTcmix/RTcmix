# for CMIXDIR...
include ../makefile.conf

EXT_NAME = RT
XS_DIR = $(EXT_NAME)
XS_FILE = $(EXT_NAME).xs
PM_FILE = $(EXT_NAME).pm
FUNCLIST = funclist
FUNCEXCEPT = funcexcept

all: RT_extension

# This dependency prevents making unless the funcexcept file has
# changed.  Not really what we want, though it is annoying to have
# to remake this stuff every time you make at the top level...
#$(FUNCLIST): $(FUNCEXCEPT)

$(FUNCLIST):
	perl -w make_funclist.pl $(CMIXDIR)

$(XS_FILE): $(FUNCLIST)
	perl -w make_rtxs.pl $(CMIXDIR) $(EXT_NAME) > $(XS_DIR)/$(XS_FILE)

$(PM_FILE): $(FUNCLIST)
	perl -w make_pmfile.pl $(EXT_NAME) > $(XS_DIR)/$(PM_FILE)

.PHONY: RT_extension install clean

RT_extension: $(XS_FILE) $(PM_FILE)
	( cd $(XS_DIR); perl Makefile.PL; $(MAKE) )

install:
	@$(INSTALL) $(XS_DIR)/$(PM_FILE) $(LIBDESTDIR)
	@$(INSTALL) $(XS_DIR)/blib/arch/auto/$(EXT_NAME)/$(EXT_NAME).so $(LIBDESTDIR)

clean:
	@if (test -f $(XS_DIR)/Makefile) then \
		(cd $(XS_DIR); $(MAKE) clean; $(RM) $(XS_FILE) $(PM_FILE) Makefile.old) \
	fi
	@$(RM) $(FUNCLIST)
	@$(RM) $(LIBDESTDIR)/$(PM_FILE) $(LIBDESTDIR)/$(EXT_NAME).so

