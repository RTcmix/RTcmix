include ../../makefile.conf

MANDIR = ../man

# WARNING: do not use a dir here that contains anything else!
# The install process in this script is not yet refined.
HTMLDIR = ../html

# RTcmix instruments
INST = ELL PAN REVERBIT REVMIX STEREO

# Commands invoked from the command line
CMD = sfcreate sffixsize sfprint sfhedit pchcps

# Commands invoked from within an RTcmix script (in Minc, Perl, Python)
SCRIPT = makegen setline setline_size setexp addgens multgens fdump fplot \
gen1 gen2 gen3 gen4 gen5 gen7

# Programmer documentation
API = 


################################################################################
# No need to change any of this when adding new documentation.

# construct names with extensions
INST_MAN = $(INST:=.man)
INST_HTML = $(INST:=.html)
INST_PS = $(INST:=.ps)
INST_PDF = $(INST:=.pdf)
CMD_MAN = $(CMD:=.man)
CMD_HTML = $(CMD:=.html)
CMD_PS = $(CMD:=.ps)
CMD_PDF = $(CMD:=.pdf)
SCRIPT_MAN = $(SCRIPT:=.man)
SCRIPT_HTML = $(SCRIPT:=.html)
SCRIPT_PS = $(SCRIPT:=.ps)
SCRIPT_PDF = $(SCRIPT:=.pdf)
API_MAN = $(API:=.man)
API_HTML = $(API:=.html)
API_PS = $(API:=.ps)
API_PDF = $(API:=.pdf)

MAN = $(INST_MAN) $(CMD_MAN) $(SCRIPT_MAN) $(API_MAN)
HTML = $(INST_HTML) $(CMD_HTML) $(SCRIPT_HTML) $(API_HTML)
PS = $(INST_PS) $(CMD_PS) $(SCRIPT_PS) $(API_PS)
PDF = $(INST_PDF) $(CMD_PDF) $(SCRIPT_PDF) $(API_PDF)

INST_TITLE = "RTcmix Instrument Documentation"
CMD_TITLE = "RTcmix Command Documentation"
SCRIPT_TITLE = "RTcmix Script Command Documentation"
API_TITLE = "RTcmix Programmer Documentation"

VERS = "$(shell perl print_rtcmix_vers.pl $(CMIXDIR))"
DATE = "$(shell date +'%d %b, %Y')"

all: $(MAN) $(HTML)

.PHONY: man html ps clean installman uninstallman

man: $(INST_MAN) $(CMD_MAN) $(SCRIPT_MAN) $(API_MAN)

html: $(HTML)

ps: $(MAN) $(PS)

pdf: $(MAN) $(PS) $(PDF)

$(INST_MAN): %.man: %.pod
	pod2man --center=$(INST_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(CMD_MAN): %.man: %.pod
	pod2man --center=$(CMD_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(SCRIPT_MAN): %.man: %.pod
	pod2man --center=$(SCRIPT_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(API_MAN): %.man: %.pod
	pod2man --center=$(API_TITLE) -s 3 --release=$(VERS) --date=$(DATE) $< > $@

$(HTML): %.html: %.pod
	pod2html --htmlroot=. --podpath=. --noindex $< > $@
	@sed -e 's/<BODY>/<body bgcolor="white">/' $@ > $@.tmp
	@mv -f $@.tmp $@

# there's probably a more portable way to do this...
$(PS): %.ps: %.man
	groff -S -Tps -mandoc $< > $@

$(PDF): %.pdf: %.ps
	ps2pdf $<

clean:
	$(RM) *.man *.html *.ps *.pdf pod2htm*

installman: $(MAN)
	mkinstalldirs $(MANDIR)/man1
	mkinstalldirs $(MANDIR)/man3
	$(INSTALL) $(INST_MAN) $(CMD_MAN) $(SCRIPT_MAN) $(MANDIR)/man1
ifneq ($(strip $(API_MAN)),)    # do only if API_MAN is nonempty
	$(INSTALL) $(API_MAN) $(MANDIR)/man3
endif

uninstallman:
	( cd $(MANDIR)/man1; $(RM) $(INST_MAN) $(CMD_MAN) $(SCRIPT_MAN) )
	( cd $(MANDIR)/man3; $(RM) $(API_MAN) )

installhtml: $(HTML)
	mkinstalldirs $(HTMLDIR)
	$(INSTALL) $(HTML) $(HTMLDIR)
	mkinstalldirs $(HTMLDIR)/images
	$(INSTALL) images/* $(HTMLDIR)/images

uninstallhtml:
	( cd $(HTMLDIR); $(RM) $(HTML) )
	$(RM) -rf $(HTMLDIR)/images/*

install: installman installhtml
uninstall: uninstallman uninstallhtml

