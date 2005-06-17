/* FLANGE - flanger, using either notch or comb filter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = amplitude multiplier (pre-processing) *
   p4  = resonance (can be negative)
   p5  = maximum delay time (determines lowest pitch; try: 1.0 / cpspch(8.00)
   p6  = modulation depth (0 - 100%)
   p7  = modulation rate (Hz)
   p8  = wet/dry mix (0: dry --> 1: wet)  [optional; default is 0.5]
   p9  = flanger type ("IIR" is IIR comb, "FIR" is FIR notch)
         [optional; default is "iir"] **
   p10 = input channel  [optional; default is 0]
   p11 = pan (in percent-to-left form: 0-1) [optional; default is 0.5]
   p12 = ring-down duration [optional; default is resonance value]
   p13 = reference to mod. wavetable [optional; if missing, must use gen 2 ***]
         Don't let the amplitude of this waveform exceed 1 (absolute value)!

   p3 (amplitude), p4 (resonance), p6 (modulation depth), p7 (modulation rate),
   p8 (wet/dry mix), p9 (flanger type) and p11 (pan) can receive dynamic updates
   from a table or real-time control source.  p9 (flanger type) can be updated
   only when using numeric codes. **

   The point of the ring-down duration parameter (p12) is to let you control
   how long the flanger will ring after the input has stopped.  If you set p12
   to zero, then FLANGE will try to figure out the correct ring-down duration
   for you.  This will almost always be fine.  However, if resonance is dynamic,
   there are cases where FLANGE's estimate of the ring duration will be too
   short, and your sound will cut off prematurely.  Use p12 to extend the
   duration.

   ----

   Notes about backward compatibility with pre-v4 scores:

   * If an old-style gen table 1 is present, its values will be multiplied
   by p3 (amplitude), even if the latter is dynamic.

   ** You can also give numeric codes for the flanger type (0: iir, 1: fir).
   These can be changed during a note.  If you give the string version, you
   can't change types during a note.

   *** If p13 is missing, you must use an old-style gen table 2 for the
   modulator waveform.


   John Gibson (johgibso at indiana dot edu), 7/21/99; rev for v4, JGG, 7/24/04
*/
rtsetparams(44100, 2)
load("FLANGE")

rtinput("../../../snd/huhh.wav")
inchan = 0
inskip = 0
dur = DUR()
amp = 5.0

resonance = 0.06
lowpitch = 7.00
moddepth = 70
modspeed = maketable("line", "nonorm", 100, 0,4, 1,.1)
wetdrymix = 0.5
flangetype = "IIR"
pan = 0.5

waveform = maketable("wave", 1000, "sine")

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(outskip=0, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, pan, ringdur=0, waveform)

