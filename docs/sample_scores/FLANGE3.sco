/* This script imposes a trill of a major 2nd onto the input file. */

rtsetparams(44100, 2)
load("FLANGE")

rtinput("../../snd/huhh.wav")
inchan = 0
inskip = 0
dur = DUR()
amp = 1.8

resonance = 1.0           /* how "ringy" are trill pitches? */
lowpitch = 8.00           /* lower pitch of major 2nd */
moddepth = 11.5           /* somehow makes a major 2nd above low pitch  ;-) */
modspeed = 6.0            /* speed of trill */
wetdrymix = 0.4           /* how prominent is trill? */

// make an "ideal" square wave
waveform = maketable("wave", 1000, "square")

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(0, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, "IIR", inchan, pan=1, ringdur=0, waveform)

FLANGE(.1, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, "IIR", inchan, pan=0, ringdur=0, waveform)
