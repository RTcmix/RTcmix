include package.conf

RTLIBDIRS = GVERB HALFWAVE SYNC VWAVE PFSCHED LOCALIZE

.PHONY: all progs dsos standalone install dso_install standalone_install\
uninstall dso_uninstall standalone_uninstall clean cleanall $(RTLIBDIRS)

all: progs

progs: $(RTLIBDIRS)

$(RTLIBDIRS)::
	echo "making all in bgg..."
	$(MAKE) $(MFLAGS) -C $@ all

dsos: all

standalone:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making standalone in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR"; fi ) \
	done

install:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; fi ) \
	done

dso_install:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making dso_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_install; fi ) \
	done

standalone_install:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_install; fi ) \
	done

uninstall:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; fi ) \
	done

dso_uninstall:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; \
	  echo "making dso_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_uninstall; fi ) \
	done

standalone_uninstall:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_uninstall; fi ) \
	done

clean:
	@for DIR in $(RTLIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean; fi ) \
	done

cleanall: clean uninstall

