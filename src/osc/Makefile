include ../../makefile.conf

OSCLIB = libosclstnr.a
OBJECTS = RTOSCListener.o
SRCS = RTOSCListener.cpp
SCRIPT = osc_server_init.sco
SCRIPT_PATH = $(LIBDIR)/$(SCRIPT)

ifeq ($(USE_INTERNAL_LIBLO), TRUE)
CURDIR = $(CMIXDIR)/src/osc
LIBLO_DIR = $(CURDIR)/liblo
LIBLO_INCLUDEDIR = $(LIBLO_DIR)/lo
LIBLO_HEADERS = $(LIBLO_INCLUDEDIR)/*.h
LIBLO_LIBDIR = $(LIBLO_DIR)/src/.libs
LIBLO_LIB = liblo.$(SHLIB_SUFFIX)
LIBLO_LIB7 = liblo.7.$(SHLIB_SUFFIX)
# location of wrapper tool scripts
LIBLO_TOOLDIR = $(LIBLO_DIR)/src/tools
# location of "real" tool binaries
LIBLO_TOOL_LIBDIR = $(LIBLO_TOOLDIR)/.libs
LIBLO_TOOL = oscsend
# We rename these on install to avoid pulling in outside liblo
LIBLO_INSTALLED_LIB = libloextended.$(SHLIB_SUFFIX)
LIBLO_INSTALLED_LIB7 = libloextended.7.$(SHLIB_SUFFIX)

.PHONY: liblo liblo_install liblo_uninstall liblo_clean
else
liblo:
liblo_install:
liblo_uninstall:
liblo_clean:
endif

CXXFLAGS += -I. -I../parser $(LIBLO_CFLAGS)

all: $(OSCLIB) liblo_install		# the custom liblo needs to be installed as part of 'all'

$(OSCLIB): $(OBJECTS)
	$(RM) $@
	ar q $@ $(OBJECTS)
	$(RANLIB) $@

RTOSCListener.o: RTOSCListener.cpp RTOSCListener.h
	$(CXX) $(CXXFLAGS) -DSCRIPT_PATH=\"$(SCRIPT_PATH)\" -c -o $@ RTOSCListener.cpp

install: all
	$(INSTALL) $(SCRIPT) $(LIBDIR)

uninstall: liblo_uninstall
	$(RM) $(LIBDIR)/$(SCRIPT)

clean: liblo_clean
	$(RM) *.o $(OSCLIB)

cleanall: clean
	$(RM) depend

ifeq ($(USE_INTERNAL_LIBLO), TRUE)

liblo:
	@echo "making in $(LIBLO_DIR)..."
	$(MAKE) -C "$(LIBLO_DIR)" all

liblo_install: liblo
	@echo "installing liblo headers..."
	@mkdir -p $(INCLUDEDIR)/lo
	$(INSTALL) $(LIBLO_HEADERS) $(INCLUDEDIR)/lo
	@echo "installing libloextended library..."
	@$(RM)  $(LIBDIR)/$(LIBLO_INSTALLED_LIB) $(LIBDIR)/$(LIBLO_INSTALLED_LIB7)
	$(INSTALL) $(LIBLO_LIBDIR)/$(LIBLO_LIB) $(LIBDIR)/$(LIBLO_INSTALLED_LIB)
	ln -s $(LIBDIR)/$(LIBLO_INSTALLED_LIB) $(LIBDIR)/$(LIBLO_INSTALLED_LIB7)
	# Do a hand-copy of the script to its new location
	@echo "installing liblo tool(s)..."
	$(INSTALL) $(LIBLO_TOOL_LIBDIR)/$(LIBLO_TOOL) $(DESTDIR)/$(LIBLO_TOOL)

	# Now modify the actual binary to point to the modified liblo
ifeq ($(ARCH),MACOSX)
	install_name_tool -id @rpath/$(LIBLO_INSTALLED_LIB) $(LIBDIR)/$(LIBLO_INSTALLED_LIB)
	install_name_tool -change /usr/local/lib/liblo.7.dylib @rpath/$(LIBLO_INSTALLED_LIB) $(DESTDIR)/$(LIBLO_TOOL)
	install_name_tool -add_rpath @executable_path/../lib $(DESTDIR)/$(LIBLO_TOOL)
endif
ifeq ($(ARCH),LINUX)
	patchelf --replace-needed liblo.so.7 $(LIBLO_INSTALLED_LIB) $(DESTDIR)/$(LIBLO_TOOL)
endif

liblo_clean:
	@echo "cleaning in $(LIBLO_DIR)..."
	$(MAKE) -C "$(LIBLO_DIR)" clean

liblo_uninstall:
	$(RM) -rf $(INCLUDEDIR)/lo
	$(RM) $(LIBDIR)/$(LIBLO_INSTALLED_LIB7) $(LIBDIR)/$(LIBLO_INSTALLED_LIB)

endif
