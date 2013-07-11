include package.conf

LIBDIRS = MAXBANG MAXMESSAGE

.PHONY: all progs dsos standalone install dso_install standalone_install \
uninstall dso_uninstall standalone_uninstall clean cleanall

all: progs

progs:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making in $$DIR..."; \
	  $(MAKE) $(MFLAGS); echo "done in $$DIR"; ) \
	done

dsos: all

standalone:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making standalone in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR" ); \
	done

install:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; ); \
	done

dso_install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making dso_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_install; ); \
	done

standalone_install:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making standalone_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_install; ); \
	done

uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; ); \
	done

dso_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making dso_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_uninstall; ); \
	done

standalone_uninstall:
	@for DIR in $(LIBDIRS); \
	do \
	( cd $$DIR; echo "making standalone_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_uninstall; ); \
	done

clean:
	@for DIR in $(LIBDIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean; ); \
	done

cleanall: clean uninstall

