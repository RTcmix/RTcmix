include ../package.conf

OBJS = JGFilter.o JGOnePole.o TwoPole.o JGOneZero.o TwoZero.o NZero.o \
JGBiQuad.o Butter.o DCBlock.o RMS.o Balance.o DLineN.o DLineL.o DLineA.o \
Reverb.o PRCRev.o JCRev.o NRev.o Comb.o ZComb.o Notch.o ZNotch.o Allpass.o \
Envelope.o ADSR.o Oscil.o OscilN.o OscilL.o KOscilN.o TableN.o TableL.o \
JGNoise.o SubNoise.o SubNoiseL.o WavShape.o ZAllpass.o Equalizer.o
#SoundIn.o

all: objlib.a

ifeq ($(BUILDTYPE), STANDALONE)
objlib.a: $(OBJS)
	$(RM) objlib.a
	ld -r $(OBJS) -o objlib.a
else
objlib.a: $(OBJS)
	$(RM) $@
	ar q $@ $(OBJS)
	$(RANLIB) $@
endif

clean:
	$(RM) $(OBJS) objlib.a

cleanall: clean
	$(RM) depend

