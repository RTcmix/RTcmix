PROG = coreaudioprobe

CXXFLAGS = -g
LDFLAGS = -framework CoreAudio -lm

all: $(PROG)

$(PROG): $(PROG).cpp
	$(CXX) $(CXXFLAGS) -o $(PROG) $(PROG).cpp $(LDFLAGS)

clean:
	$(RM) *.o $(PROG)
