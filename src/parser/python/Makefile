include ../../../makefile.conf

CFLAGS += -I$(INCLUDEDIR) -I../../rtcmix
CXXFLAGS += -I$(INCLUDEDIR) -I../../rtcmix

MODULE = rtcmix
MODULE_SRC = $(MODULE)module.cpp
FUNCLIST = funclist
FUNCEXCEPT = funcexcept
FIND_FLAGS = -follow

.PHONY: all clean cleanall

all: $(MODULE)

# This dependency prevents making unless the funcexcept file has
# changed.  Not really what we want, but it's annoying to have
# to remake this stuff every time you make at the top level...
$(FUNCLIST): make_funclist.pl $(FUNCEXCEPT)
	$(PERL) -w make_funclist.pl $(CMIXDIR) $(FIND_FLAGS)

$(MODULE_SRC): make_python_ext.pl $(MODULE_SRC).in $(FUNCLIST)
	$(PERL) -w make_python_ext.pl $(MODULE) $(MODULE_SRC).in > $(MODULE_SRC)

$(MODULE): $(MODULE_SRC)
	$(PYTHON) setup.py build
	cp build/lib.linux-i686-2.2/$(MODULE).so .
#FIXME: this is platform-specific -- use distutils for install

clean:
	$(RM) $(FUNCLIST) $(MODULE).so $(MODULE)module.o $(MODULE_SRC)
	$(RM) -r build

cleanall: clean

