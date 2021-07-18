include ../makefile.conf

DIRS = 

.PHONY: all progs imbed install uninstall clean cleanall

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

imbed: do_package
	cd imbed; $(MAKE) $(MFLAGS) all;
		
install: do_package
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making install in $$DIR..."; \
	  $(MAKE) $(MFLAGS) install; ); \
	done

uninstall:
	@for DIR in $(DIRS); \
	do \
	( cd $$DIR; echo "making uninstall in $$DIR..."; \
	  $(MAKE) $(MFLAGS) uninstall; ); \
	done

clean:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making clean in $$DIR..."; \
		$(MAKE) $(MFLAGS) clean ); \
	done

cleanall:
	@for DIR in $(DIRS); \
	do \
	  ( cd $$DIR; echo "making cleanall in $$DIR..."; \
		$(MAKE) $(MFLAGS) cleanall ); \
	done

distclean: cleanall
	@for DIR in $(DIRS); \
	do \
	  $(RM) $$DIR/package.conf; \
	done

