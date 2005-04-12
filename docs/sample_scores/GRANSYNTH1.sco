rtsetparams(44100, 2, 128)
load("GRANSYNTH")

dur = 60

amp = makeconnection("mouse", "y", 20, 90, 10, 50, "amp", "dB")
amp = makeconverter(amp, "ampdb")

wavetab = maketable("wave", 2000, 1, .5, .3, .2, .1)

envtab = maketable("window", 2000, "hanning")

outjitter = 0.0001

density = makeconnection("mouse", "x", 1, 500, 1, 10, "density")
hoptime = 1.0 / density

mindur = .05
maxdur = mindur
minamp = maxamp = 1

pitch = makeconnection("mouse", "y", 6, 8, 6, 10, "pitch", "linoct")

transpcoll = maketable("literal", "nonorm", 0, 0, .02, .03, .05, .07, .10)
pitchjitter = 1

seed = 1

st = 0
bus_config("GRANSYNTH", "out 0")
GRANSYNTH(st, dur, amp, wavetab, envtab, hoptime, outjitter,
   mindur, maxdur, minamp, maxamp, pitch, transpcoll, pitchjitter, seed)

bus_config("GRANSYNTH", "out 1")
st += 0.01
seed += 1
pitch = pitch + 0.002
GRANSYNTH(st, dur, amp, wavetab, envtab, hoptime, outjitter,
   mindur, maxdur, minamp, maxamp, pitch, transpcoll, pitchjitter, seed)

