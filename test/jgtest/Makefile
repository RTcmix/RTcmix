TESTSND = num
TESTSND = sinefloat
TESTSND = sine

TESTSCO = $(TESTSND).sco

DIRT = *.snd *.aif *.aiff *.aifc *.wav *.sf *.out *.rescale test.snd

COMPARE = cmp -lc


all: m1 s1

.PHONY: m1 m2 m3 s1 cmp_m1 cmp_m2 cmp_m3 cmp_s1 $(TESTSND) clean

m1: $(TESTSND)
	@echo MIX/mix to next format
	@CMIX 1 0 < m1.sco 2>&1 >  m1.out    # next
	@CMIX 0 0 < m1.sco 2>&1 >> m1.out
	@echo MIX/mix to aiff format
	@CMIX 1 1 < m1.sco 2>&1 >> m1.out    # aiff
	@CMIX 0 1 < m1.sco 2>&1 >> m1.out
	@echo MIX/mix to aifc format
	@CMIX 1 2 < m1.sco 2>&1 >> m1.out    # aifc
	@CMIX 0 2 < m1.sco 2>&1 >> m1.out
	@echo MIX/mix to wav format
	@CMIX 1 3 < m1.sco 2>&1 >> m1.out    # wav
	@CMIX 0 3 < m1.sco 2>&1 >> m1.out
	@echo MIX/mix to ircam format
	@CMIX 1 4 < m1.sco 2>&1 >> m1.out    # ircam
	@CMIX 0 4 < m1.sco 2>&1 >> m1.out

m2: $(TESTSND)
	@echo MIX/mix to next format
	@CMIX 1 0 < m2.sco 2>&1 >  m2.out    # next
	@CMIX 0 0 < m2.sco 2>&1 >> m2.out
	@echo MIX/mix to aiff format
	@CMIX 1 1 < m2.sco 2>&1 >> m2.out    # aiff
	@CMIX 0 1 < m2.sco 2>&1 >> m2.out
	@echo MIX/mix to aifc format
	@CMIX 1 2 < m2.sco 2>&1 >> m2.out    # aifc
	@CMIX 0 2 < m2.sco 2>&1 >> m2.out
	@echo MIX/mix to wav format
	@CMIX 1 3 < m2.sco 2>&1 >> m2.out    # wav
	@CMIX 0 3 < m2.sco 2>&1 >> m2.out
	@echo MIX/mix to ircam format
	@CMIX 1 4 < m2.sco 2>&1 >> m2.out    # ircam
	@CMIX 0 4 < m2.sco 2>&1 >> m2.out

m3: $(TESTSND)
	@echo MIX/mix to aiff format
	@CMIX 1 < m3.sco 2>&1 >> m3.out
	@CMIX 0 < m3.sco 2>&1 >> m3.out

s1: $(TESTSND)
	@echo STEREO/stereo to next format
	@CMIX 1 0 < s1.sco 2>&1 >  s1.out    # next
	@CMIX 0 0 < s1.sco 2>&1 >> s1.out
	@echo STEREO/stereo to aiff format
	@CMIX 1 1 < s1.sco 2>&1 >> s1.out    # aiff
	@CMIX 0 1 < s1.sco 2>&1 >> s1.out
	@echo STEREO/stereo to aifc format
	@CMIX 1 2 < s1.sco 2>&1 >> s1.out    # aifc
	@CMIX 0 2 < s1.sco 2>&1 >> s1.out
	@echo STEREO/stereo to wav format
	@CMIX 1 3 < s1.sco 2>&1 >> s1.out    # wav
	@CMIX 0 3 < s1.sco 2>&1 >> s1.out
	@echo STEREO/stereo to ircam format
	@CMIX 1 4 < s1.sco 2>&1 >> s1.out    # ircam
	@CMIX 0 4 < s1.sco 2>&1 >> s1.out

cmp_m1:
	@echo differences between disk-based and real-time output...
	$(COMPARE) m1.snd m1rt.snd
	$(COMPARE) m1.aiff m1rt.aiff
	$(COMPARE) m1.aifc m1rt.aifc
	$(COMPARE) m1.wav m1rt.wav
	$(COMPARE) m1.sf m1rt.sf

cmp_m2:
	@echo differences between disk-based and real-time output...
	$(COMPARE) m2.snd m2rt.snd
	$(COMPARE) m2.aiff m2rt.aiff
	$(COMPARE) m2.aifc m2rt.aifc
	$(COMPARE) m2.wav m2rt.wav
	$(COMPARE) m2.sf m2rt.sf

cmp_m3:
	@echo differences between disk-based and real-time output...
	$(COMPARE) m3.aiff m3rt.aiff

cmp_s1:
	@echo differences between disk-based and real-time output...
	$(COMPARE) s1.snd s1rt.snd
	$(COMPARE) s1.aiff s1rt.aiff
	$(COMPARE) s1.aifc s1rt.aifc
	$(COMPARE) s1.wav s1rt.wav
	$(COMPARE) s1.sf s1rt.sf

$(TESTSND):
	@echo writing test.snd...
	@CMIX < $(TESTSCO) >& /dev/null

clean:
	$(RM) $(DIRT)

