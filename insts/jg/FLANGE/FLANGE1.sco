/* FLANGE - flanger, using either notch or comb filter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = amplitude multiplier (pre-processing)
   p4  = resonance (can be negative)
   p5  = maximum delay time (determines lowest pitch; try: 1.0 / cpspch(8.00)
   p6  = modulation depth (0 - 100%)
   p7  = modulation speed (Hz)
   p8  = wet/dry mix (0: dry --> 1: wet)  [optional; default is 0.5]
   p9  = flanger type (0 is IIR comb, 1 is FIR notch)  [optional; default is 0]
   p10 = input channel  [optional; default is 0]
   p11 = stereo spread (0 - 1)  [optional; default is 0.5]

   Assumes function table 1 is an amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses a
   flat amplitude curve. This curve, combined with the amplitude multiplier,
   affect the signal BEFORE processing.

   Assumes function table 2 holds one cycle of the modulation waveform.
   Don't let the amplitude of this waveform exceed 1 (absolute value)!
   When in doubt, use "makegen(2, 10, 2000, 1)".

   When using the FIR notch flanger, resonance needs to be > 0.1 for the 
   flanger to have much of an effect.
   
   (For kicks, try max delay of 1/x, where x is < 20 Hz. First increase the
   size of your waveform makegen to > 20000. Otherwise, the non-interpolating
   oscillator used in this instrument will create some noise.)

   John Gibson (jgg9c@virginia.edu), 7/21/99.
*/
rtsetparams(44100, 2)
load("FLANGE")

rtinput("any.snd")

outskip = 0
inskip = 0
dur = DUR()
amp = 1.0
inchan = 0
spread = 0.5

resonance = 0.06
lowpitch = 7.00
moddepth = 70
modspeed = 2.5
wetdrymix = 0.5
flangetype = 0

gensize = 2000
makegen(2,10,gensize, 1)

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, spread)
