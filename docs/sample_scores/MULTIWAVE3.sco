rtsetparams(44100, 2)
load("MULTIWAVE")

dur = 60
masteramp = 30000

// in linear octaves
minpitch = 6.00
maxpitch = 10.00

glide = 50

wave = maketable("wave", 5000, 1, 0, .02)
line = maketable("line", 1000, 0,0, 1,1, 8,1, 10,0)

pitchtable = maketable("literal", "nonorm", 0,
   6.00,
   7.00,
   octpch(7.05),
   octpch(7.07),
   8.00,
   octpch(8.07),
   octpch(8.08),
   9.00,
   octpch(9.07)
)

numwaves = 10
freq = {}
amp = {}
pan = {}
for (i = 0; i < numwaves; i += 1) {
   lfofreq = 0.007 + (i * 1.4)
   rfreq = makeLFO("sine", lfofreq, min = 0.2 + (i * 0.03), min * 3.5)
   min = minpitch + (i * octpch(0.03))
   max = maxpitch - (i * octpch(0.02))
   freq[i] = makerandom("low", rfreq, min, max, seed = i + 2)
   freq[i] = makefilter(freq[i], "constrain", pitchtable, 0.95)
   freq[i] = makefilter(freq[i], "smooth", glide)
   freq[i] = makeconverter(freq[i], "cpsoct")
   min = i % 2
   if (min == 0)
      max = 1
   else
      max = 0
   amp[i] = makeLFO("sine", 0.06 + (i * 0.04), 0.2, 1)
   pan[i] = makeLFO("sine", 0.007 + (i * 0.026), min, max)
}

phase = 0

MULTIWAVE(0, dur, masteramp * line, wave,
   freq[0], amp[0], phase, pan[0],
   freq[1], amp[1], phase, pan[1],
   freq[2], amp[2], phase, pan[2],
   freq[3], amp[3], phase, pan[3],
   freq[4], amp[4], phase, pan[4],
   freq[5], amp[5], phase, pan[5],
   freq[6], amp[6], phase, pan[6],
   freq[7], amp[7], phase, pan[7],
   freq[8], amp[8], phase, pan[8],
   freq[9], amp[9], phase, pan[9])


// JGG, 3/10/05

