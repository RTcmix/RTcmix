include package.conf

RTLIBDIRS = BROWN CRACKLE DUST HENON LATOOCARFIAN MULTIFM PINK

LIBDIRS = $(RTLIBDIRS)

.PHONY: all progs dsos standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall $(LIBDIRS)


all: $(LIBDIRS)

$(LIBDIRS)::
	@echo "making all in neil..."
	$(MAKE) $(MFLAGS) -C $@

progs: all

dsos: all

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making standalone in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR"; fi ) \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; fi ) \
	done

dso_install:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making dso_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_install; fi ) \
	done

standalone_install:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_install; fi ) \
	done

uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; fi ) \
	done

dso_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; echo "making dso_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_uninstall; fi ) \
	done

standalone_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( if test -d $$DIR; then cd $$DIR; \
	  echo "making standalone_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_uninstall; fi ) \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( if test -d $$DIR; then cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean; fi ) \
	done

cleanall: clean uninstall

