include ../makefile.conf

EXT_NAME = rtcmix
EXT_FILE = rtcmix_python_ext
FUNCLIST = funclist
FUNCEXCEPT = funcexcept
ifeq ($(ARCH),MACOSX)
  XT_SUFFIX = .bundle
  FIND_FLAGS = -follow
else
  XT_SUFFIX = .so
  FIND_FLAGS = -follow
endif

PYTHON_INCLUDE_DIR = `$(PYTHON) print_python_includedir.py`

# Note: any optimization makes huge object file, due to extensive inlining
CFLAGS = -I$(PYTHON_INCLUDE_DIR) $(OPT)

all: $(EXT_FILE).o

# This dependency prevents making unless the funcexcept file has
# changed.  Not really what we want, but it's annoying to have
# to remake this stuff every time you make at the top level...
$(FUNCLIST): make_funclist.pl $(FUNCEXCEPT)
	perl -w make_funclist.pl $(CMIXDIR) $(FIND_FLAGS)

$(EXT_FILE).c: make_python_ext.pl $(FUNCLIST)
	perl -w make_python_ext.pl $(CMIXDIR) $(EXT_NAME) > $(EXT_FILE).c

$(EXT_FILE).o: $(EXT_FILE).c $(EXT_FILE).h
	$(CC) $(CFLAGS) -c $(EXT_FILE).c

.PHONY: clean
clean:
	@$(RM) $(FUNCLIST) $(EXT_FILE).o $(EXT_FILE).c

