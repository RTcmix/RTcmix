# RTcmix  - Copyright (C) 2000  The RTcmix Development Team
# See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
# the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
#
# RTcmix help system makefile by John Gibson

include ../../makefile.conf

# RTcmix instruments
INST = AM ELL PAN REVERBIT REVMIX SHAPE STEREO TRANS TRANS3 WAVETABLE

# Commands invoked from within an RTcmix script (in Minc, Perl, Python)
SCRIPT = addgens bus_config copygen fdump fplot \
gen1 gen2 gen3 gen4 gen5 gen6 gen7 gen9 gen10 gen17 gen18 gen20 gen24 gen25 \
get_spray \
infile invertgen irand makegen multgens offsetgen pickrand pickwrand \
quantizegen rand random reversegen rtinput rtoutput rtsetparams \
sampfunc sampfunci scalegen set_option setexp setline setline_size \
shiftgen spray_init srand system

# Commands invoked from the command line
CMD = pchcps rescale sfcreate sffixsize sfhedit sfprint

# Programmer documentation
API = 

IMAGES = addgens.png fplot.png gen4a.png gen4b.png gen4c.png gen4d.png \
gen5a.png gen5b.png gen6.png gen7.png gen9a.png gen9b.png gen9c.png \
gen9d.png gen9e.png gen17.png gen20.png multgens.png


################################################################################
# No need to change any of this when adding new documentation.

# construct names with extensions
# NB: have to use ".1" and ".3" suffixes, instead of ".man," for man pages,
# because OS X seems to insist on this.
INST_MAN = $(INST:=.1)
INST_HTML = $(INST:=.html)
INST_PS = $(INST:=.ps)
INST_PDF = $(INST:=.pdf)
SCRIPT_MAN = $(SCRIPT:=.1)
SCRIPT_HTML = $(SCRIPT:=.html)
SCRIPT_PS = $(SCRIPT:=.ps)
SCRIPT_PDF = $(SCRIPT:=.pdf)
CMD_MAN = $(CMD:=.1)
CMD_HTML = $(CMD:=.html)
CMD_PS = $(CMD:=.ps)
CMD_PDF = $(CMD:=.pdf)
API_MAN = $(API:=.3)
API_HTML = $(API:=.html)
API_PS = $(API:=.ps)
API_PDF = $(API:=.pdf)

MAN = $(INST_MAN) $(SCRIPT_MAN) $(CMD_MAN) $(API_MAN)
HTML = $(INST_HTML) $(SCRIPT_HTML) $(CMD_HTML) $(API_HTML)
PS = $(INST_PS) $(SCRIPT_PS) $(CMD_PS) $(API_PS)
PDF = $(INST_PDF) $(SCRIPT_PDF) $(CMD_PDF) $(API_PDF)
SYN = syn

HTML_INDEX = index.html

INST_TITLE = "RTcmix Instrument Documentation"
SCRIPT_TITLE = "RTcmix Script Command Documentation"
CMD_TITLE = "RTcmix Command Documentation"
API_TITLE = "RTcmix Programmer Documentation"

VERS = "$(shell perl print_rtcmix_vers.pl $(CMIXDIR))"
DATE = "$(shell date +'%d %b, %Y')"
CMDIR = "$(CMIXDIR)"

mkinstalldirs = $(SHELL) $(CMIXDIR)/docs/pod/mkinstalldirs

all: $(MAN) $(HTML) $(HTML_INDEX)

.PHONY: man html ps pdf clean install uninstall installman uninstallman \
installhtml uninstallhtml installsyn uninstallsyn

man: $(INST_MAN) $(CMD_MAN) $(SCRIPT_MAN) $(API_MAN)

html: $(HTML) $(HTML_INDEX)

ps: $(MAN) $(PS)

pdf: $(MAN) $(PS) $(PDF)

$(INST_MAN): %.1: %.pod
	pod2man --center=$(INST_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(SCRIPT_MAN): %.1: %.pod
	pod2man --center=$(SCRIPT_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(CMD_MAN): %.1: %.pod
	pod2man --center=$(CMD_TITLE) --release=$(VERS) --date=$(DATE) $< > $@

$(API_MAN): %.3: %.pod
	pod2man --center=$(API_TITLE) -s 3 --release=$(VERS) --date=$(DATE) $< > $@

$(HTML): %.html: %.pod
	pod2html --css=style.css --htmlroot=. --podpath=. --noindex $< > $@
	@sh fix_html.sh $@ > $@.tmp
	@mv -f $@.tmp $@

$(HTML_INDEX): make_html_index.pl
	@echo $(INST_HTML) > inst_html.tmp
	@echo $(SCRIPT_HTML) > script_html.tmp
	@echo $(CMD_HTML) > cmd_html.tmp
	@echo $(API_HTML) > api_html.tmp
	@perl -w make_html_index.pl > $(HTML_INDEX)
	@$(RM) inst_html.tmp script_html.tmp cmd_html.tmp api_html.tmp

# FIXME: this won't build the API docs!
$(PS): %.ps: %.1
	groff -S -Tps -mandoc $< > $@

$(PDF): %.pdf: %.ps
	ps2pdf $<

# Edit syn.in script to insert RTcmix src dir.  Use 'Z' instead of '/' as regexp
# delimiter, since cmixdir contains path slashes.  We might be screwed without
# GNU sed.
$(SYN): syn.in
	@sed s\Z_CMIXDIR_Z'$(CMDIR)'Z syn.in > $(SYN)
	@chmod 755 $(SYN)

clean:
	$(RM) *.1 *.3 *.html *.ps *.pdf *.tmp pod2htm* $(SYN)

installman: $(MAN)
ifneq ($(strip $(MANDESTDIR)),)    # do only if MANDESTDIR is nonempty
	$(mkinstalldirs) $(MANDESTDIR)/man1
	$(mkinstalldirs) $(MANDESTDIR)/man3
	$(INSTALL) $(INST_MAN) $(SCRIPT_MAN) $(CMD_MAN) $(MANDESTDIR)/man1
ifneq ($(strip $(API_MAN)),)       # do only if API_MAN is nonempty
	$(INSTALL) $(API_MAN) $(MANDESTDIR)/man3
endif
endif

uninstallman:
ifneq ($(strip $(MANDESTDIR)),)    # do only if MANDESTDIR is nonempty
	( cd $(MANDESTDIR)/man1; $(RM) $(INST_MAN) $(SCRIPT_MAN) $(CMD_MAN) )
ifneq ($(strip $(API_MAN)),)       # do only if API_MAN is nonempty
	( cd $(MANDESTDIR)/man3; $(RM) $(API_MAN) )
endif
endif

installhtml: $(HTML) $(HTML_INDEX)
ifneq ($(strip $(HTMLDESTDIR)),)    # do only if HTMLDESTDIR is nonempty
	$(mkinstalldirs) $(HTMLDESTDIR)
	$(INSTALL) $(HTML) $(HTMLDESTDIR)
	$(INSTALL) $(HTML_INDEX) $(HTMLDESTDIR)
	$(INSTALL) style.css $(HTMLDESTDIR)
	$(mkinstalldirs) $(HTMLDESTDIR)/images
	( cd images; $(INSTALL) $(IMAGES) $(HTMLDESTDIR)/images )
endif

uninstallhtml:
ifneq ($(strip $(HTMLDESTDIR)),)    # do only if HTMLDESTDIR is nonempty
	( if test -d $(HTMLDESTDIR); then cd $(HTMLDESTDIR); \
		$(RM) $(HTML) $(HTML_INDEX) style.css; fi )
	( cd $(HTMLDESTDIR)/images; $(RM) $(IMAGES) )
endif

installsyn: $(SYN)
	$(INSTALL) $(SYN) $(BINDIR)

uninstallsyn:
	( cd $(BINDIR); $(RM) $(SYN) )

install: installman installhtml installsyn
uninstall: uninstallman uninstallhtml uninstallsyn

