include insts.conf

RTLIBDIRS = ELL JDELAY JCHOR JFIR REV REVERBIT ROOM MROOM SROOM VOCODE
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
	  $(MAKE) $(MFLAGS); echo "done in $$DIR" ); \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making install in $$DIR..."; \
	  ln -fs `pwd`/lib$$DIR.so $(LIBDESTDIR); \
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
	( cd objlib; make clean; )

onlyDSO:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "removing standalone instrument in $$DIR..."; \
	  rm -f $$DIR; ) \
	done


.PHONY: objlib

