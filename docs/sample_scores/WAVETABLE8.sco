// This hideous-sounding score is a workout for tables, pfield arithmetic
// and Minc arrays.        -JGG, 7/13/04

rtsetparams(44100, 2)
load("WAVETABLE")

notes = {8.00, 8.04, 8.07, 8.10}
numnotes = len(notes)

dur = 10
increment = 2

amp = 12000
env = maketable("line", 1000, 0,0, 1,1, 3,1, 4,0)

penv = maketable("line", 1000, 0,1, 1,1, 2,2, 3,2, 8,.15)

// NOTE: This is the way to fake an LFO with a wave table, but 
// there is also a makeLFO function that could be used
partial = dur * 10
vib = maketable("wave3", "nonorm", 1000, partial, lfoamp = 4, 0)

pan = maketable("line", 100, 0,0, 1,1, 2,0.5)

wavt = maketable("wave", 4000, 1, 0.5, 0.3, 0.2, 0.1, 0.1)

start = 0
for (n = 0; n < numnotes; n += 1) {
   pitch = notes[n]
   WAVETABLE(start, dur, amp * env, (cpspch(pitch) * penv) + vib, pan, wavt)
   pan = makefilter(pan, "invert", 0.5)
   start += increment
}

