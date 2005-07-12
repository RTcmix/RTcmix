// This shows how to add global reverb to any score.
rtsetparams(44100, 2)
load("WAVETABLE")
load("REVERBIT")

// WAVETABLE output enters aux 0-1 buses; REVERBIT reads from those,
// and sends output to the audio hardware.

bus_config("WAVETABLE", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in", "out 0-1")

// Controls duration of WAVETABLE loop and length of REVERBIT call.
totdur = 6

control_rate(2000)   // otherwise, the short WAVETABLE notes click

//----------------------------------------------------------------------------
dur = 1
amp = 15000
amp = maketable("line", 1000, 0,0, 1,1, 5,0) * amp
freq = 9.00
wavet = maketable("wave", 10000, 1, 0.5, 0.3, 0.1)

// Seed random number generator that controls panning.
srand(3284)

dur = 0.08
for (st = 0; st < totdur; st = st + .3)
   WAVETABLE(st, dur, amp, freq, random(), wavet)

freq = 7.10
for (st = 0; st < totdur; st = st + .45)
   WAVETABLE(st, dur, amp, freq, random(), wavet)

//----------------------------------------------------------------------------
amp = 1
revtime = 1.5

// vary the wet/dry mix over time
wetpct = maketable("line", "nonorm", 1000, 0,0.2, 1,0.01, 3,1.2)

rtchandel = 0.15
cf = 200

// inskip MUST be zero when reading from aux bus
REVERBIT(st=0, insk=0, totdur, amp, revtime, wetpct, rtchandel, cf)

