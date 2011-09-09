// Risset bell, after Dodge 2nd ed, p. 111, 2nd example  -JG
rtsetparams(44100, 2)
load("WAVETABLE")

// exponential amplitude envelope ("F2" in Dodge)
env = maketable("expbrk", 1000,  1, 1000, .0000000002)

durf = 3    // scale duration
freqf = 3   // scale frequencies
ampf = 400 * env
start = 0
WAVETABLE(start, durf * 15,  ampf * 6,  freqf * 35,   pan=1)
WAVETABLE(start, durf * 20,  ampf * 20, freqf * 82,   pan=0)
WAVETABLE(start, durf * 17,  ampf * 15, freqf * 82.4, pan=.9)
WAVETABLE(start, durf * 20,  ampf * 20, freqf * 165,  pan=.1)
WAVETABLE(start, durf * 15,  ampf * 30, freqf * 200,  pan=.8)
WAVETABLE(start, durf * 6,   ampf * 20, freqf * 342,  pan=.2)
WAVETABLE(start, durf * 5,   ampf * 15, freqf * 425,  pan=.7)
WAVETABLE(start, durf * 7,   ampf * 20, freqf * 500,  pan=.3)
WAVETABLE(start, durf * 4,   ampf * 5,  freqf * 895,  pan=.6)
WAVETABLE(start, durf * 2,   ampf * 4,  freqf * 1303, pan=.4)
WAVETABLE(start, durf * 1,   ampf * 5,  freqf * 1501, pan=.5)
WAVETABLE(start, durf * 4,   ampf * 6,  freqf * 1700, pan=.5)
WAVETABLE(start, durf * 1.5, ampf * 4,  freqf * 2200, pan=.5)

