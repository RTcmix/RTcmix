/* DECIMATE - reduce number of bits used to represent sound

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier (before decimation)
   p4 = number of bits to use (1 to 16)
   p5 = low-pass filter cutoff frequency (or 0 to bypass)
         [optional, default is 0]
   p6 = input channel [optional, default is 0]
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.  This is applied AFTER the decimation and filter.

   JGG <johgibso@indiana.edu>, 3 Jan 2002
*/
rtsetparams(44100, 1)
load("DECIMATE")
rtinput("/snd/motorclip.snd")
bits = 2
cutoff = 4000
dur = DUR()
amp = 1
setline(0,0, 1,1, 9,1, 10,0)
DECIMATE(0, 0, dur, amp, bits, cutoff)
