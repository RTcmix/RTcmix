include ../package.conf

OBJS = Brass.o DelayA.o BiQuad.o Filter.o Stk.o PoleZero.o Instrmnt.o \
	Delay.o DelayL.o Noise.o BowTabl.o BandedWG.o JetTabl.o \
	BlowBotl.o ReedTabl.o OneZero.o BlowHole.o OnePole.o \
	Bowed.o Clarinet.o Flute.o Modal.o ModalBar.o Mesh2D.o \
	Saxofony.o Shakers.o Sitar.o

#CXXFLAGS += -Wall -O3

all: stklib.a

stklib.a: $(OBJS)
	$(RM) stklib.a
	ld -r $(OBJS) -o stklib.a

clean:
	$(RM) $(OBJS) stklib.a

