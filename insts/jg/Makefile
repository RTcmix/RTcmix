include package.conf

RTLIBDIRS = JDELAY JCHOR JFIR REV REVERBIT ROOM MROOM SROOM VOCODE FLANGE ELL
CMIXLIBDIRS = bender denoise splice
LIBDIRS = $(CMIXLIBDIRS) $(RTLIBDIRS)

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
	  $(INSTALL) `pwd`/lib$$DIR.so $(LIBDESTDIR); ) \
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
	  $(INSTALL) `pwd`/$$DIR $(DESTDIR); ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(DESTDIR)/$$DIR; \
	  $(RM) $(LIBDESTDIR)/lib$$DIR.so; ) \
	done
	( cd objlib; $(MAKE) clean; )

dso_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(LIBDESTDIR)/lib$$DIR.so; ) \
	done

standalone_clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; $(MAKE) clean; \
	  $(RM) $(DESTDIR)/$$DIR; \
	done

.PHONY: objlib

