/* JFIR - FIR filter, designed from a frequency response curve

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = filter order (higher order allows steeper slope)
   p5 = input channel [optional, default is 0]
   p6 = stereo spread (0 - 1) [optional, default is .5 for stereo output]

   Can only process 1 channel at a time. To process stereo, call twice --
   once with inchan=0 and spread=1, again with inchan=1 and spread=0.
   
   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the desired frequency response curve, described
   by freq,amp pairs. Frequency is in Hz, from 0 to Nyquist; amp is from
   0 to 1. Ideally, frequencies with amplitude of 1 are passed without
   attenuation; those with amplitude of 0 are attenuated totally. But
   this behavior depends on the order of the filter. Try an order of 200,
   and increase that as needed. (I've gotten an order of 600 in mono in
   real time on a PII266.)

   Example:

      nyquist = 44100 / 2
      makegen(2, 24, 5000, 0,0, 200,0, 300,1, 2000,1, 4000,0, nyquist,0)

   With a high order, this should attenuate everything below 200 Hz
   and above 4000 Hz.

   John Gibson (jgg9c@virginia.edu), 7/3/99.
   Filter design code adapted from Bill Schottstaedt's Snd.
*/
rtsetparams(44100, 2)
load("JFIR")

writeit = 0
if (writeit) {
   set_option("audio_off", "clobber_on")
   rtoutput("foo.snd", "sun", "float")
}
rtinput("/tmp/1stmove.snd")

reset(4000)
setline_size(12000)
setline(0,0, 1,1, 29,1, 30,0)

inchan = 0
amp = 2.0
inskip = 13
totaldur = 5.5
dur = .05

nyq = 44100 / 2

numentries = 5
makegen(3, 2, numentries, 0)
500 2000 1000 3000 1200

half_bandwidth_percent = .50

order = 300

for (st = 0; st < totaldur - dur; st = st + dur) {
   n = (st / totaldur) * (numentries - 1)
   cf = sampfunci(3, n)
   low = cf - (cf * half_bandwidth_percent)
   high = cf + (cf * half_bandwidth_percent)
   makegen(2, 24, 5000, 0,0, low,0, cf,1, high,0, nyq,0)
   JFIR(st, inskip, dur, amp, order, inchan)
   inskip = inskip + dur
}


