# prefix of test score and output file
M = m1
S = s1

TESTSND = num.snd

DIRT = $(M).snd $(M)rt.snd $(M).aif $(M)rt.aif $(M).wav $(M)rt.wav \
       $(M).sf $(M)rt.sf $(M).out \
       $(S).snd $(S)rt.snd $(S).aif $(S)rt.aif $(S).wav $(S)rt.wav \
       $(S).sf $(S)rt.sf $(S).out \
       $(TESTSND)

COMPARE = cmp -lc


all: mix stereo

mix: $(TESTSND)
	@echo MIX/mix to next format
	@CMIX 1 0 < $(M).sco 2>&1 >  $(M).out    # next
	@CMIX 0 0 < $(M).sco 2>&1 >> $(M).out
	@echo MIX/mix to aiff format
	@CMIX 1 1 < $(M).sco 2>&1 >> $(M).out    # aiff
	@CMIX 0 1 < $(M).sco 2>&1 >> $(M).out
	@echo MIX/mix to wav format
	@CMIX 1 2 < $(M).sco 2>&1 >> $(M).out    # wav
	@CMIX 0 2 < $(M).sco 2>&1 >> $(M).out
	@echo MIX/mix to ircam format
	@CMIX 1 3 < $(M).sco 2>&1 >> $(M).out    # ircam
	@CMIX 0 3 < $(M).sco 2>&1 >> $(M).out

stereo: $(TESTSND)
	@echo STEREO/stereo to next format
	@CMIX 1 0 < $(S).sco 2>&1 >  $(S).out    # next
	@CMIX 0 0 < $(S).sco 2>&1 >> $(S).out
	@echo STEREO/stereo to aiff format
	@CMIX 1 1 < $(S).sco 2>&1 >> $(S).out    # aiff
	@CMIX 0 1 < $(S).sco 2>&1 >> $(S).out
	@echo STEREO/stereo to wav format
	@CMIX 1 2 < $(S).sco 2>&1 >> $(S).out    # wav
	@CMIX 0 2 < $(S).sco 2>&1 >> $(S).out
	@echo STEREO/stereo to ircam format
	@CMIX 1 3 < $(S).sco 2>&1 >> $(S).out    # ircam
	@CMIX 0 3 < $(S).sco 2>&1 >> $(S).out

comparemix:
	@echo differences between disk-based and real-time output...
	$(COMPARE) $(M).snd $(M)rt.snd
	$(COMPARE) $(M).aif $(M)rt.aif
	$(COMPARE) $(M).wav $(M)rt.wav
	$(COMPARE) $(M).sf $(M)rt.sf

comparestereo:
	@echo differences between disk-based and real-time output...
	$(COMPARE) $(S).snd $(S)rt.snd
	$(COMPARE) $(S).aif $(S)rt.aif
	$(COMPARE) $(S).wav $(S)rt.wav
	$(COMPARE) $(S).sf $(S)rt.sf

$(TESTSND):
	@echo writing test snd...
	@CMIX < num.sco >& /dev/null

.PHONY: clean
clean:
	$(RM) $(DIRT)

