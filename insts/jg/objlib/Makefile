include ../package.conf

OBJS = Filter.o OnePole.o TwoPole.o OneZero.o TwoZero.o NZero.o BiQuad.o \
Butter.o DCBlock.o RMS.o Balance.o DLineN.o DLineL.o DLineA.o Reverb.o \
PRCRev.o JCRev.o NRev.o Comb.o ZComb.o Notch.o ZNotch.o Allpass.o \
Envelope.o ADSR.o Oscil.o OscilN.o OscilL.o KOscilN.o TableN.o TableL.o \
Noise.o SubNoise.o SubNoiseL.o SoundIn.o WavShape.o ZAllpass.o Equalizer.o

all: objlib.a

objlib.a: $(OBJS)
	$(RM) objlib.a
	ld -r $(OBJS) -o objlib.a

clean:
	$(RM) $(OBJS) objlib.a

