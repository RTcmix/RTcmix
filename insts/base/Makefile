# change these if needed
include insts.conf
INSTDIR = insts.base

RTLIBDIRS = MIX WAVETABLE

CMIXLIBDIRS = mix wavetable

LIBDIRS = $(RTLIBDIRS)  $(CMIXLIBDIRS) 

all:
	@for DIR in $(LIBDIRS) MIX; \
	do \
	( cd $$DIR; echo "making in $$DIR..."; \
	$(MAKE) $(MFLAGS); echo "done in $$DIR" ); \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	ln -fs $(CMIXDIR)/$(INSTDIR)/$$DIR/lib$$DIR.so $(LIBDESTDIR); ) \
	done
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	rm -f $(DESTDIR)/$$DIR; \
	ln -fs $(CMIXDIR)/$(INSTDIR)/$$DIR/$$DIR $(DESTDIR); ) \
	done
	ln -fs $(CMIXDIR)/$(INSTDIR)/MIX/MIX ../bin/CMIX

clean:

	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; make clean; \
	rm -f $(DESTDIR)/$$DIR; \
	rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

