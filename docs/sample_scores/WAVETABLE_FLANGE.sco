rtsetparams(44100, 2)
load("WAVETABLE")
load("FLANGE")

bus_config("WAVETABLE", "aux 0-1 out")
bus_config("FLANGE", "aux 0-1 in", "out 0-1")

wavet = maketable("wave", 10000, "buzz20")

start = 0
dur = 20

maxamp = 12000
amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 19,maxamp, 20,0)
reset(5000)

WAVETABLE(start, dur, amp, 6.00, pan=1.0, wavet)
WAVETABLE(start, dur, amp, 7.00, pan=0.5, wavet)
WAVETABLE(start, dur, amp, 7.07, pan=0.0, wavet)


//----------------------------------------------------------------------------
start = 0
inskip = 0      // must be zero when reading from aux bus!

amp = 1.0

resonance = 0.2
lowpitch = 5.00
moddepth = 90
modspeed = 0.05
wetdrymix = 0.5
flangetype = "IIR"

// If table size is too small, you'll hear zipper noise.  (Try 1000 to see.)
tabsize = 100000
waveform = maketable("wave", tabsize, "sine")

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=0, pan=1, ringdur=0, waveform)

lowpitch += 0.07
maxdelay = 1.0 / cpspch(lowpitch)

waveform = maketable("wave3", tabsize, 1,1,-180)

FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=1, pan=0, ringdur=0, waveform)

