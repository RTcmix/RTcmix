include ../makefile.conf

# This finds all dirs in this directory (except for CVS/).
DIRS = `find . -type d -mindepth 1 -maxdepth 1 | sed -e '/CVS/d'`

.PHONY: all progs base std dsos standalone install dso_install \
	standalone_install uninstall dso_uninstall standalone_uninstall \
	clean cleanall

all: do_package
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making all in $$DIR..."; \
	  $(MAKE) $(MFLAGS) all; echo "done in $$DIR" ); \
	done

do_package:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "include $(MAKEFILE_CONF)" > package.conf; )\
	done

base: do_package
	cd base; $(MAKE) $(MFLAGS) all;
		
std: do_package
	cd std; $(MAKE) $(MFLAGS) all;
		
dsos: all

standalone: do_package
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making standalone in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone; echo "done in $$DIR" ); \
	done

install: do_package
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; ); \
	done

dso_install: do_package
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making dso_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_install; ); \
	done

standalone_install: standalone
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making standalone_install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_install; ); \
	done

uninstall:
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; ); \
	done

dso_uninstall:
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making dso_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) dso_uninstall; ); \
	done

standalone_uninstall:
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making standalone_uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) standalone_uninstall; ); \
	done

clean:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean ); \
	done

cleanall: clean uninstall

distclean: cleanall
	@for DIR in $(DIRS); \
	do \
	  $(RM) $$DIR/package.conf; \
	done

