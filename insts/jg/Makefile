include package.conf

RTLIBDIRS = ELL JDELAY JCHOR JFIR REV REVERBIT ROOM MROOM SROOM VOCODE FLANGE
CMIXLIBDIRS = bender denoise splice
LIBDIRS = $(RTLIBDIRS) $(CMIXLIBDIRS) 

all: objlib progs

objlib:
	@( cd objlib; echo "making in objlib..."; \
	  $(MAKE) $(MFLAGS); echo "done in objlib" ); \

progs:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; ) \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  ln -fs `pwd`/lib$$DIR.so $(LIBDESTDIR); ) \
	done

dsos:  all

dso_install:  install

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR" ); \
	done

standalone_install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	  rm -f $(DESTDIR)/$$DIR; \
	  ln -fs `pwd`/$$DIR $(DESTDIR); ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(DESTDIR)/$$DIR; \
	  rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	rm -f $(DESTDIR)/CMIX

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; make clean; \
	  rm -f $(DESTDIR)/$$DIR; \
	done

.PHONY: objlib

