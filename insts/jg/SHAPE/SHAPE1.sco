/* SHAPE - waveshaping instrument

   This differs from insts.std/WAVESHAPE in that it accepts input from
   a file or bus, and it offers a different way of performing amplitude
   normalization.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = minimum distortion index
   p5 = maximum distortion index
   p6 = table number of amplitude normalization function, or 0 for no norm.
   p7 = input channel [optional, default is 0]
   p8 = percent of signal to left output channel [optional, default is .5]

   Function tables:
      1  amplitude curve (or use setline)
         If no setline or function table 1, uses flat amplitude curve.
      2  transfer function
      3  distortion index curve

   JGG <johgibso@indiana.edu>, 3 Jan 2002
*/
rtsetparams(44100, 1)
load("WAVETABLE")
load("SHAPE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("SHAPE", "aux 0 in", "out 0")

dur = 10
amp = 10000
freq = 200
makegen(2, 10, 2000, 1)
start = 0
WAVETABLE(start, dur, amp, freq)

setline(0,0, 9,1, 10,0)
makegen(2, 17, 1000, 0.9, 0.3, -0.2, 0.6, -0.7, 0.9, -0.1)
makegen(3, 25, 1000, 1)    /* bell curve */

min = 0; max = 3.0
SHAPE(start, inskip = 0, dur, amp = 0.5, min, max, 0)

