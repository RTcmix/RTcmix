/* REV - some reverberators from the STK package (by Perry Cook, Gary Scavone,
         and Tim Stilson)

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = reverb type (1: PRCRev, 2: JCRev, 3: NRev ... see below)
   p5 = reverb time (seconds)
   p6 = reverb percent (0: dry --> 1: wet)
   p7 = input channel  [optional; default is 0]

   Assumes function table 1 is an amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses a
   flat amplitude curve. This curve, combined with the amplitude multiplier,
   affect the signal BEFORE it enters the reverb.

   Reverb types:

     (1) PRCRev (Perry R. Cook)
           2 allpass units in series followed by 2 comb filters in parallel.

     (2) JCRev (John Chowning)
           3 allpass filters in series, followed by 4 comb filters in
           parallel, a lowpass filter, and two decorrelation delay lines
           in parallel at the output.

     (3) NRev (Michael McNabb)
           6 comb filters in parallel, followed by 3 allpass filters, a
           lowpass filter, and another allpass in series, followed by 2
           allpass filters in parallel with corresponding right and left
           outputs.

   John Gibson (jgg9c@virginia.edu), 7/19/99.
*/
rtsetparams(44100, 2)
load("REV")

rtinput("foo.aif")

outskip = 0
inskip = 0
dur = DUR()
amp = 1.0
rvbtype = 3
rvbtime = 0.3
rvbpct = 0.3
inchan = 0

setline(0,0, 1,1, dur-1,1, dur,0)

REV(outskip, inskip, dur, amp, rvbtype, rvbtime, rvbpct, inchan)
