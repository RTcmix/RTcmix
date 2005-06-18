// See "WIGGLE1.sco" for a description of parameters

rtsetparams(44100, 1)
load("WIGGLE")

dur = 10
two_layers = true
amp = 800
env = maketable("line", 1000, 0,1, dur-.1,1, dur,0)
pitch = 11.00

carwav = maketable("wave", 2000, 1, .3, .1)    // carrier waveform

// --------------------------------------------------------------------------
/* This shows how to create random vibrato.  Note that RTcmix doesn't
   interpolate between values of the random oscillator, so you get a clear
   stair-step effect, like an analog sequencer.  If you want smooth transitions
   between the steps, run the random lfo through a smoothing filter, which is
   commented out below.
*/
vib_speed = 10 // in Hz

// down as much as a perfect fifth, up as much as an octave
min = -0.07    // in oct.pc
max = 1.00

lfo = makerandom("even", vib_speed, octpch(min), octpch(max), seed = 1)
//lfo = makefilter(lfo, "smooth", lag=40)
freq = makeconverter(lfo + octpch(pitch), "cpsoct")

WIGGLE(st=0, dur, amp * env, freq, 0, 0, 0, 0, carwav)

// --------------------------------------------------------------------------
if (two_layers) {
   // same as above, but different seed and base pitch
   lfo = makerandom("even", vib_speed, octpch(min), octpch(max), seed = 2)
   //lfo = makefilter(lfo, "smooth", lag=30)
   freq = makeconverter(lfo + octpch(pitch - 3), "cpsoct")
   WIGGLE(st=0, dur, amp * env, freq, 0, 0, 0, 0, carwav)
}

