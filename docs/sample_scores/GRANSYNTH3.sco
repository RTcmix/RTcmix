// This score demonstrates the "transposition collection" feature of
// GRANSYNTH, as well as the technique of changing grain density and
// pitch using mouse input.   -JGG

rtsetparams(44100, 2, 128)
load("GRANSYNTH")

dur = 120

amp = ampdb(68)

wavetab = maketable("wave", 2000, 1, .5, .3, .2, .1)

envtab = maketable("window", 2000, "hanning")

outjitter = 0.000

density = makeconnection("mouse", "x", 1, 3000, 20, 70, "density")
hoptime = 1 / density

// scaling grain duration by hoptime keeps CPU util under control
mindur = hoptime * 20
maxdur = mindur // + .02

minamp = maxamp = 1

pitch = makeconnection("mouse", "y", 6, 12, 8, 10, "pitch", "linoct")

transpcoll = maketable("literal", "nonorm", 0, 0, .02, .03, .05, .07, .10)
pitchjitter = 1

seed = 1

GRANSYNTH(st=0, dur, amp, wavetab, envtab, hoptime, outjitter,
   mindur, maxdur, minamp, maxamp, pitch, transpcoll, pitchjitter, seed)

