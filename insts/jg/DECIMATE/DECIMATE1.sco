/* DECIMATE - reduce number of bits used to represent sound

   NOTE: Pfield order has changed since v3.8!  (p4 is new)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = pre-amp multiplier (before decimation)
   p4 = post-amp multiplier (after decimation)
   p5 = number of bits to use (1 to 16)
   p6 = low-pass filter cutoff frequency (or 0 to bypass)
         [optional, default is 0]
   p7 = input channel [optional, default is 0]
   p8 = percent of signal to left output channel [optional, default is .5]

   p3 (pre-amp), p4 (post-amp), p5 (bits), p6 (cutoff) and p8 (pan) can
   receive dynamic updates from a table or real-time control source.

   If an old-style gen table 1 is present, its values will be multiplied
   by the p4 post-amp multiplier, even if the latter is dynamic.

   JGG <johgibso at indiana dot edu>, 3 Jan 2002, rev for v4, 7/11/04
*/
rtsetparams(44100, 2)
load("DECIMATE")
rtinput("mynicecleansound.wav")
inchan = 0
dur = DUR()

bits = 3
preamp = 1
postamp = maketable("line", 1000, 0,0, 5,1, 9,1, 10,0)
cutoff = maketable("line", "nonorm", 1000, 0,1, 1,10000, 2,800)
pctleft = maketable("line", 100, 0,0, 1,1)

DECIMATE(0, 0, dur, preamp, postamp, bits, cutoff, inchan, pctleft)

