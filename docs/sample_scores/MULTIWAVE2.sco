rtsetparams(44100, 2)
load("MULTIWAVE")

dur = 60
masteramp = 20000

minfreq = 50
maxfreq = 1500
glide = 50

// quantize freqs to this number (in Hz); set to zero for no quantum
//quantum = 100
quantum = 0

wave = maketable("wave", 5000, 1)
line = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

numwaves = 10
freq = {}
pan = {}
for (i = 0; i < numwaves; i += 1) {
   lfofreq = 0.007 + (i * 1.4)
   rfreq = makeLFO("sine", lfofreq, min = 0.2 + (i * 0.03), min * 3.5)
   min = minfreq + (i * 10)
   max = maxfreq - (i * 70)
   rand = makerandom("linear", rfreq, min, max, seed = i + 1)
   freq[i] = makefilter(rand, "smooth", glide)
   if (quantum)
      freq[i] = makefilter(freq[i], "quantize", quantum)
   min = mod(i, 2)
   if (min == 0)
      max = 1
   else
      max = 0
   pan[i] = makeLFO("sine", 0.007 + (i * 0.026), min, max)
}

amp = 1
phase = 0

MULTIWAVE(0, dur, masteramp * line, wave,
   freq[0], amp, phase, pan[0],
   freq[1], amp, phase, pan[1],
   freq[2], amp, phase, pan[2],
   freq[3], amp, phase, pan[3],
   freq[4], amp, phase, pan[4],
   freq[5], amp, phase, pan[5],
   freq[6], amp, phase, pan[6],
   freq[7], amp, phase, pan[7],
   freq[8], amp, phase, pan[8],
   freq[9], amp, phase, pan[9])


// JGG, 3/10/05

